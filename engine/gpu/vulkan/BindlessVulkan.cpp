/*
   Copyright 2022 MacKenzie Strand

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
#include "BufferVulkan.hpp"
#include "TextureVulkan.hpp"
#include "ArchitectVulkan.hpp"

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
   return pBindless->Startup(pDevice, pCreateInfo);
}

CT_API ctResults ctGPUBindlessManagerShutdown(ctGPUDevice* pDevice,
                                              ctGPUBindlessManager* pBindless) {
   return pBindless->Shutdown(pDevice);
}

ctResults ctGPUBindlessManager::Startup(ctGPUDevice* pDevice,
                                        ctGPUBindlessManagerCreateInfo* pCreateInfo) {
   VkDescriptorSetLayoutBinding descriptorSetLayouts[4] = {};
   VkDescriptorPoolSize descriptorPoolSizes[4] = {};
   VkDescriptorBindingFlags bindFlags[4] = {};
   /* Sampler */
   descriptorSetLayouts[0].binding = GLOBAL_BIND_SAMPLER;
   descriptorSetLayouts[0].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
   descriptorSetLayouts[0].stageFlags = VK_SHADER_STAGE_ALL;
   descriptorSetLayouts[0].descriptorCount = maxSamplers;
   descriptorPoolSizes[0].type = VK_DESCRIPTOR_TYPE_SAMPLER;
   descriptorPoolSizes[0].descriptorCount = maxSamplers;
   bindFlags[0] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
   slotManagerSamplers = ctVkBindlessSlotManager(0, maxSamplers);
   /* Sampled Image */
   descriptorSetLayouts[1].binding = GLOBAL_BIND_SAMPLED_IMAGE;
   descriptorSetLayouts[1].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
   descriptorSetLayouts[1].stageFlags = VK_SHADER_STAGE_ALL;
   descriptorSetLayouts[1].descriptorCount = maxSampledImages;
   descriptorPoolSizes[1].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
   descriptorPoolSizes[1].descriptorCount = maxSampledImages;
   bindFlags[1] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
                  VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
   slotManagerSampledImage =
     ctVkBindlessSlotManager(pCreateInfo->fixedTextureBindUpperBound, maxSampledImages);
   /* Storage Image */
   descriptorSetLayouts[2].binding = GLOBAL_BIND_STORAGE_IMAGE;
   descriptorSetLayouts[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
   descriptorSetLayouts[2].stageFlags = VK_SHADER_STAGE_ALL;
   descriptorSetLayouts[2].descriptorCount = maxStorageImages;
   descriptorPoolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
   descriptorPoolSizes[2].descriptorCount = maxStorageImages;
   bindFlags[2] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
                  VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
   slotManagerStorageImage =
     ctVkBindlessSlotManager(pCreateInfo->fixedTextureBindUpperBound, maxStorageImages);
   /* Storage Buffer */
   descriptorSetLayouts[3].binding = GLOBAL_BIND_STORAGE_BUFFER;
   descriptorSetLayouts[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
   descriptorSetLayouts[3].stageFlags = VK_SHADER_STAGE_ALL;
   descriptorSetLayouts[3].descriptorCount = maxStorageBuffers;
   descriptorPoolSizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
   descriptorPoolSizes[3].descriptorCount = maxStorageBuffers;
   bindFlags[3] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
                  VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
   slotManagerStorageBuffer = ctVkBindlessSlotManager(
     pCreateInfo->fixedStorageBufferBindUpperBound, maxStorageBuffers);

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
                                 pDevice->GetAllocCallback(),
                                 &vkGlobalDescriptorSetLayout),
     CT_NCT("FAIL:vkCreateDescriptorSetLayout",
            "vkCreateDescriptorSetLayout() failed to create descriptor set layout."));

   VkDescriptorPoolCreateInfo poolInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
   poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT |
                    VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
   poolInfo.poolSizeCount = ctCStaticArrayLen(descriptorPoolSizes);
   poolInfo.pPoolSizes = descriptorPoolSizes;
   poolInfo.maxSets = 3;
   CT_VK_CHECK(
     vkCreateDescriptorPool(
       pDevice->vkDevice, &poolInfo, pDevice->GetAllocCallback(), &vkDescriptorPool),
     CT_NCT("FAIL:vkCreateDescriptorPool",
            "vkCreateDescriptorPool() failed to create descriptor pool."));

   VkDescriptorSetAllocateInfo allocInfo = {
     VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
   allocInfo.descriptorSetCount = 1;
   allocInfo.descriptorPool = vkDescriptorPool;
   allocInfo.pSetLayouts = &vkGlobalDescriptorSetLayout;
   for (int i = 0; i < CT_MAX_INFLIGHT_FRAMES; i++) {
      CT_VK_CHECK(
        vkAllocateDescriptorSets(
          pDevice->vkDevice, &allocInfo, &vkGlobalDescriptorSet[i]),
        CT_NCT("FAIL:vkAllocateDescriptorSets",
               "vkAllocateDescriptorSets() failed to allocate global descriptor set."));
   }

   VkPushConstantRange range = {};
   range.stageFlags = VK_SHADER_STAGE_ALL;
   range.size = sizeof(int32_t) * CT_MAX_GFX_DYNAMIC_INTS;
   range.offset = 0;
   VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
     VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
   pipelineLayoutInfo.setLayoutCount = 1;
   pipelineLayoutInfo.pSetLayouts = &vkGlobalDescriptorSetLayout;
   pipelineLayoutInfo.pushConstantRangeCount = 1;
   pipelineLayoutInfo.pPushConstantRanges = &range;
   CT_VK_CHECK(
     vkCreatePipelineLayout(pDevice->vkDevice,
                            &pipelineLayoutInfo,
                            pDevice->GetAllocCallback(),
                            &vkGlobalPipelineLayout),
     CT_NCT("FAIL:vkCreatePipelineLayout",
            "vkCreatePipelineLayout() failed to allocate global pipeline layout."));

   for (int i = 0; i < CT_MAX_INFLIGHT_FRAMES; i++) {
      buffInfos[i].Reserve((size_t)maxStorageBuffers + 1000);
      imageInfos[i].Reserve((size_t)maxSampledImages + maxStorageImages + 1000);
   }
   return CT_SUCCESS;
}

ctResults ctGPUBindlessManager::Shutdown(ctGPUDevice* pDevice) {
   vkDestroyPipelineLayout(
     pDevice->vkDevice, vkGlobalPipelineLayout, pDevice->GetAllocCallback());
   vkDestroyDescriptorPool(
     pDevice->vkDevice, vkDescriptorPool, pDevice->GetAllocCallback());
   vkDestroyDescriptorSetLayout(
     pDevice->vkDevice, vkGlobalDescriptorSetLayout, pDevice->GetAllocCallback());
   return CT_SUCCESS;
}

CT_API ctResults
ctGPUBindlessManagerPrepareFrame(ctGPUDevice* pDevice,
                                 ctGPUBindlessManager* pBindless,
                                 uint32_t bufferPoolCount,
                                 ctGPUExternalBufferPool** ppBufferPools,
                                 uint32_t texturePoolCount,
                                 ctGPUExternalTexturePool** ppTexturePools,
                                 uint32_t architectCount,
                                 struct ctGPUArchitect** ppArchitects,
                                 void* pNext) {
   return pBindless->PrepareFrame(
     pDevice, architectCount, (struct ctGPUArchitectVulkan**)ppArchitects);
}

ctResults ctGPUBindlessManager::PrepareFrame(ctGPUDevice* pDevice,
                                             uint32_t architectCount,
                                             struct ctGPUArchitectVulkan** ppArchitect) {
   for (size_t i = 0; i < trackedDynamicBufferIndices.Count(); i++) {
      if (trackedDynamicBuffers[i]->currentFrame != trackedDynamicBufferLastFrames[i]) {
         trackedDynamicBufferLastFrames[i] = trackedDynamicBuffers[i]->currentFrame;
         /* flush all upcoming frames with current frame */
         ctGPUExternalBuffer* pBuffer = trackedDynamicBuffers[i];
         CT_RETURN_FAIL(AddBufferWrite(currentFrame,
                                       trackedDynamicBufferIndices[i],
                                       pBuffer->contents[pBuffer->currentFrame].buffer));
      }
   }
   for (size_t i = 0; i < trackedDynamicTextureIndices.Count(); i++) {
      if (trackedDynamicTextures[i]->currentFrame != trackedDynamicTextureLastFrames[i]) {
         trackedDynamicTextureLastFrames[i] = trackedDynamicTextures[i]->currentFrame;
         /* flush all upcoming frames with current frame */
         ctGPUExternalTexture* pTexture = trackedDynamicTextures[i];
         CT_RETURN_FAIL(AddTextureWrite(currentFrame,
                                        trackedDynamicTextureIndices[i],
                                        pTexture->contents[currentFrame].view));
      }
   }
   for (size_t i = 0; i < architectCount; i++) {
      if (ppArchitect[i]->needsBindUpdate) {
         for (int j = 0; j < ppArchitect[i]->bufferBindings.Count(); j++) {
            for (int k = 0; k < CT_MAX_INFLIGHT_FRAMES; k++) {
               AddBufferWrite(k,
                              ppArchitect[i]->bufferBindings[j].idx,
                              ppArchitect[i]->bufferBindings[j].buff);
            }
         }
         for (int j = 0; j < ppArchitect[i]->imageBindings.Count(); j++) {
            for (int k = 0; k < CT_MAX_INFLIGHT_FRAMES; k++) {
               AddTextureWrite(k,
                               ppArchitect[i]->imageBindings[j].idx,
                               ppArchitect[i]->imageBindings[j].view);
            }
         }
         ppArchitect[i]->needsBindUpdate = false;
      }
   }
   if (!descriptorSetWrites[currentFrame].isEmpty()) {
      vkUpdateDescriptorSets(pDevice->vkDevice,
                             (uint32_t)descriptorSetWrites[currentFrame].Count(),
                             descriptorSetWrites[currentFrame].Data(),
                             0,
                             NULL);
   }
   buffInfos[currentFrame].Clear();
   imageInfos[currentFrame].Clear();
   descriptorSetWrites[currentFrame].Clear();
   return CT_SUCCESS;
}

CT_API int32_t ctGPUBindlessManagerMapTexture(ctGPUDevice* pDevice,
                                              ctGPUBindlessManager* pBindless,
                                              int32_t desiredIdx,
                                              ctGPUExternalTexture* pTexture) {
   if (desiredIdx < 0) { desiredIdx = pBindless->slotManagerSampledImage.AllocateSlot(); }
   /* Handle Dynamics On the Fly*/
   if (pTexture->updateMode == CT_GPU_UPDATE_DYNAMIC) {
      pBindless->trackedDynamicTextureIndices.Append(desiredIdx);
      pBindless->trackedDynamicTextureLastFrames.Append(-1);
      pBindless->trackedDynamicTextures.Append(pTexture);
   } else {
      VkWriteDescriptorSet writes[CT_MAX_INFLIGHT_FRAMES] = {};
      VkDescriptorBufferInfo buffInfos[CT_MAX_INFLIGHT_FRAMES] = {};
      int srcFrame = 0;
      int dstFrame = 0;
      for (int i = 0; i < CT_MAX_INFLIGHT_FRAMES; i++) {
         if (pTexture->updateMode == CT_GPU_UPDATE_STREAM) {
            dstFrame = (pBindless->currentFrame + i) % CT_MAX_INFLIGHT_FRAMES;
            srcFrame = ((int)pTexture->currentFrame + i) % CT_MAX_INFLIGHT_FRAMES;
         } else {
            dstFrame = i;
         }
         CT_RETURN_ON_FAIL(pBindless->AddTextureWrite(
                             dstFrame, desiredIdx, pTexture->contents[srcFrame].view),
                           -1);
      }
   }
   return desiredIdx;
}

CT_API void ctGPUBindlessManagerUnmapTexture(ctGPUDevice* pDevice,
                                             ctGPUBindlessManager* pBindless,
                                             int32_t index,
                                             ctGPUExternalTexture* pTexture) {
   pBindless->slotManagerSampledImage.ReleaseSlot(index);
   if (pTexture->updateMode == CT_GPU_UPDATE_DYNAMIC) {
      const int64_t trackSlot = pBindless->trackedDynamicTextureIndices.FindIndex(index);
      if (trackSlot >= 0) {
         pBindless->trackedDynamicTextureIndices.RemoveAt(trackSlot);
         pBindless->trackedDynamicTextureLastFrames.RemoveAt(trackSlot);
         pBindless->trackedDynamicTextures.RemoveAt(trackSlot);
      }
   }
}

CT_API int32_t ctGPUBindlessManagerMapStorageBuffer(ctGPUDevice* pDevice,
                                                    ctGPUBindlessManager* pBindless,
                                                    int32_t desiredIdx,
                                                    ctGPUExternalBuffer* pBuffer) {
   if (desiredIdx < 0) {
      desiredIdx = pBindless->slotManagerStorageBuffer.AllocateSlot();
   }
   if (pBuffer->updateMode == CT_GPU_UPDATE_DYNAMIC) {
      pBindless->trackedDynamicBufferIndices.Append(desiredIdx);
      pBindless->trackedDynamicBufferLastFrames.Append(-1);
      pBindless->trackedDynamicBuffers.Append(pBuffer);
   } else {
      VkWriteDescriptorSet writes[CT_MAX_INFLIGHT_FRAMES] = {};
      VkDescriptorBufferInfo buffInfos[CT_MAX_INFLIGHT_FRAMES] = {};
      int srcFrame = 0;
      int dstFrame = 0;
      for (int i = 0; i < CT_MAX_INFLIGHT_FRAMES; i++) {
         if (pBuffer->updateMode == CT_GPU_UPDATE_STREAM) {
            dstFrame = (pBindless->currentFrame + i) % CT_MAX_INFLIGHT_FRAMES;
            srcFrame = ((int)pBuffer->currentFrame + i) % CT_MAX_INFLIGHT_FRAMES;
         } else {
            dstFrame = i;
         }
         CT_RETURN_ON_FAIL(pBindless->AddBufferWrite(
                             dstFrame, desiredIdx, pBuffer->contents[srcFrame].buffer),
                           -1);
      }
   }
   return desiredIdx;
}

CT_API void ctGPUBindlessManagerUnmapStorageBuffer(ctGPUDevice* pDevice,
                                                   ctGPUBindlessManager* pBindless,
                                                   int32_t index,
                                                   ctGPUExternalBuffer* pBuffer) {
   pBindless->slotManagerStorageBuffer.ReleaseSlot(index);
   if (pBuffer->updateMode == CT_GPU_UPDATE_DYNAMIC) {
      const int64_t trackSlot = pBindless->trackedDynamicBufferIndices.FindIndex(index);
      if (trackSlot >= 0) {
         pBindless->trackedDynamicBufferIndices.RemoveAt(trackSlot);
         pBindless->trackedDynamicBufferLastFrames.RemoveAt(trackSlot);
         pBindless->trackedDynamicBuffers.RemoveAt(trackSlot);
      }
   }
}