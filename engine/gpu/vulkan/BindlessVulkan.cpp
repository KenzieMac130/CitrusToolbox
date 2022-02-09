/*
   Copyright 2021 MacKenzie Strand

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include "BindlessVulkan.hpp"

/* https://ourmachinery.com/post/moving-the-machinery-to-bindless/
 * https://roar11.com/2019/06/vulkan-textures-unbound/
 * https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VK_EXT_descriptor_indexing.html
 * https://gpuopen.com/performance/#descriptors
 * https://anki3d.org/resource-uniformity-bindless-access-in-vulkan/
 */

CT_API ctResults
ctGPUBindlessManagerStartup(ctGPUDevice* pDevice,
                            ctGPUBindlessManager** ppBindless,
                            ctGPUBindlessManagerCreateInfo* pCreateInfo) {
   ctDebugLog("Starting Bindless System...");
   *ppBindless = new ctGPUBindlessManager();
   ctGPUBindlessManager* pBindless = *ppBindless;
   VkDescriptorSetLayoutBinding descriptorSetLayouts[4] = {};
   VkDescriptorPoolSize descriptorPoolSizes[4] = {};
   VkDescriptorBindingFlags bindFlags[4] = {};
   /* Sampler */
   descriptorSetLayouts[0].binding = GLOBAL_BIND_SAMPLER;
   descriptorSetLayouts[0].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
   descriptorSetLayouts[0].stageFlags = VK_SHADER_STAGE_ALL;
   descriptorSetLayouts[0].descriptorCount = pBindless->maxSamplers;
   descriptorPoolSizes[0].type = VK_DESCRIPTOR_TYPE_SAMPLER;
   descriptorPoolSizes[0].descriptorCount = pBindless->maxSamplers;
   bindFlags[0] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
   pBindless->descriptorsSamplers = ctVkDescriptorManager(pBindless->maxSamplers);
   /* Sampled Image */
   descriptorSetLayouts[1].binding = GLOBAL_BIND_SAMPLED_IMAGE;
   descriptorSetLayouts[1].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
   descriptorSetLayouts[1].stageFlags = VK_SHADER_STAGE_ALL;
   descriptorSetLayouts[1].descriptorCount = pBindless->maxSampledImages;
   descriptorPoolSizes[1].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
   descriptorPoolSizes[1].descriptorCount = pBindless->maxSampledImages;
   bindFlags[1] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
                  VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
   pBindless->descriptorsSampledImage =
     ctVkDescriptorManager(pBindless->maxSampledImages);
   /* Storage Image */
   descriptorSetLayouts[2].binding = GLOBAL_BIND_STORAGE_IMAGE;
   descriptorSetLayouts[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
   descriptorSetLayouts[2].stageFlags = VK_SHADER_STAGE_ALL;
   descriptorSetLayouts[2].descriptorCount = pBindless->maxStorageImages;
   descriptorPoolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
   descriptorPoolSizes[2].descriptorCount = pBindless->maxStorageImages;
   bindFlags[2] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
                  VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
   pBindless->descriptorsStorageImage =
     ctVkDescriptorManager(pBindless->maxStorageImages);
   /* Storage Buffer */
   descriptorSetLayouts[3].binding = GLOBAL_BIND_STORAGE_BUFFER;
   descriptorSetLayouts[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
   descriptorSetLayouts[3].stageFlags = VK_SHADER_STAGE_ALL;
   descriptorSetLayouts[3].descriptorCount = pBindless->maxStorageBuffers;
   descriptorPoolSizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
   descriptorPoolSizes[3].descriptorCount = pBindless->maxStorageBuffers;
   bindFlags[3] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
                  VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
   pBindless->descriptorsStorageBuffer =
     ctVkDescriptorManager(pBindless->maxStorageBuffers);

   VkDescriptorSetLayoutBindingFlagsCreateInfo bindingExtInfo = {
     VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO};
   bindingExtInfo.pBindingFlags = bindFlags;
   bindingExtInfo.bindingCount = ctCStaticArrayLen(bindFlags);

   VkDescriptorSetLayoutCreateInfo descSetLayoutInfo = {
     VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
   descSetLayoutInfo.pNext = &bindingExtInfo;
   descSetLayoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
   descSetLayoutInfo.bindingCount = ctCStaticArrayLen(descriptorSetLayouts);
   descSetLayoutInfo.pBindings = descriptorSetLayouts;
   CT_VK_CHECK(
     vkCreateDescriptorSetLayout(pDevice->vkDevice,
                                 &descSetLayoutInfo,
                                 &pDevice->vkAllocCallback,
                                 &pBindless->vkGlobalDescriptorSetLayout),
     CT_NCT("FAIL:vkCreateDescriptorSetLayout",
            "vkCreateDescriptorSetLayout() failed to create descriptor set layout."));

   VkDescriptorPoolCreateInfo poolInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
   poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT |
                    VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
   poolInfo.poolSizeCount = ctCStaticArrayLen(descriptorPoolSizes);
   poolInfo.pPoolSizes = descriptorPoolSizes;
   poolInfo.maxSets = 1;
   CT_VK_CHECK(vkCreateDescriptorPool(pDevice->vkDevice,
                                      &poolInfo,
                                      &pDevice->vkAllocCallback,
                                      &pBindless->vkDescriptorPool),
               CT_NCT("FAIL:vkCreateDescriptorPool",
                      "vkCreateDescriptorPool() failed to create descriptor pool."));

   VkDescriptorSetAllocateInfo allocInfo = {
     VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
   allocInfo.descriptorSetCount = 1;
   allocInfo.descriptorPool = pBindless->vkDescriptorPool;
   allocInfo.pSetLayouts = &pBindless->vkGlobalDescriptorSetLayout;
   CT_VK_CHECK(
     vkAllocateDescriptorSets(
       pDevice->vkDevice, &allocInfo, &pBindless->vkGlobalDescriptorSet),
     CT_NCT("FAIL:vkAllocateDescriptorSets",
            "vkAllocateDescriptorSets() failed to allocate global descriptor set."));

   VkPushConstantRange range = {};
   range.stageFlags = VK_SHADER_STAGE_ALL;
   range.size = sizeof(int32_t) * CT_MAX_GFX_DYNAMIC_INTS;
   range.offset = 0;
   VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
     VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
   pipelineLayoutInfo.setLayoutCount = 1;
   pipelineLayoutInfo.pSetLayouts = &pBindless->vkGlobalDescriptorSetLayout;
   pipelineLayoutInfo.pushConstantRangeCount = 1;
   pipelineLayoutInfo.pPushConstantRanges = &range;
   CT_VK_CHECK(
     vkCreatePipelineLayout(pDevice->vkDevice,
                            &pipelineLayoutInfo,
                            &pDevice->vkAllocCallback,
                            &pBindless->vkGlobalPipelineLayout),
     CT_NCT("FAIL:vkCreatePipelineLayout",
            "vkCreatePipelineLayout() failed to allocate global pipeline layout."));
   return CT_SUCCESS;
}

CT_API ctResults ctGPUBindlessManagerShutdown(ctGPUDevice* pDevice,
                                              ctGPUBindlessManager* pBindless) {
   vkDestroyPipelineLayout(
     pDevice->vkDevice, pBindless->vkGlobalPipelineLayout, &pDevice->vkAllocCallback);
   vkDestroyDescriptorPool(
     pDevice->vkDevice, pBindless->vkDescriptorPool, &pDevice->vkAllocCallback);
   vkDestroyDescriptorSetLayout(pDevice->vkDevice,
                                pBindless->vkGlobalDescriptorSetLayout,
                                &pDevice->vkAllocCallback);
   return CT_SUCCESS;
}

void ctGPUBindlessManager::ExposeBindlessStorageBuffer(ctGPUDevice* pDevice,
                                                       int32_t& outIdx,
                                                       VkBuffer buffer,
                                                       VkDeviceSize range,
                                                       VkDeviceSize offset) {
   ZoneScoped;
   outIdx = descriptorsStorageBuffer.AllocateSlot();
   VkDescriptorBufferInfo buffInfo = {0};
   buffInfo.buffer = buffer;
   buffInfo.offset = offset;
   buffInfo.range = range;
   VkWriteDescriptorSet write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
   write.descriptorCount = 1;
   write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
   write.dstSet = vkGlobalDescriptorSet;
   write.dstBinding = GLOBAL_BIND_STORAGE_BUFFER;
   write.dstArrayElement = outIdx;
   write.pBufferInfo = &buffInfo;
   vkUpdateDescriptorSets(pDevice->vkDevice, 1, &write, 0, NULL);
}

void ctGPUBindlessManager::ReleaseBindlessStorageBuffer(ctGPUDevice* pDevice,
                                                        int32_t idx) {
   descriptorsStorageBuffer.ReleaseSlot(idx);
}

void ctGPUBindlessManager::ExposeBindlessSampledImage(ctGPUDevice* pDevice,
                                                      int32_t& outIdx,
                                                      VkImageView view,
                                                      VkImageLayout layout,
                                                      VkSampler sampler) {
   outIdx = descriptorsSampledImage.AllocateSlot();
   VkDescriptorImageInfo imageInfo = {0};
   imageInfo.imageView = view;
   imageInfo.imageLayout = layout;
   imageInfo.sampler = sampler;
   VkWriteDescriptorSet write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
   write.descriptorCount = 1;
   write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
   write.dstSet = vkGlobalDescriptorSet;
   write.dstBinding = GLOBAL_BIND_SAMPLED_IMAGE;
   write.dstArrayElement = outIdx;
   write.pImageInfo = &imageInfo;
   vkUpdateDescriptorSets(pDevice->vkDevice, 1, &write, 0, NULL);
}

void ctGPUBindlessManager::ReleaseBindlessSampledImage(int32_t idx) {
   descriptorsSampledImage.ReleaseSlot(idx);
}