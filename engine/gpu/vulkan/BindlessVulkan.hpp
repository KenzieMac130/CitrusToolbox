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

#pragma once

#include "utilities/Common.h"
#include "DeviceVulkan.hpp"
#include "gpu/Bindless.h"

class CT_API ctVkBindlessSlotManager {
public:
   ctVkBindlessSlotManager() {
      nextNewIdx = 0;
      _max = 0;
   }
   ctVkBindlessSlotManager(int32_t begin, int32_t max) {
      nextNewIdx = begin;
      _max = max;
   }

   /* Get the next open slot to place a resource in the bindless system */
   inline int32_t AllocateSlot() {
      int32_t result = nextNewIdx;
      if (!freedIdx.isEmpty()) {
         result = freedIdx.Last();
         freedIdx.RemoveLast();
      } else {
         nextNewIdx++;
      }
      return result;
   }
   /* Only call once the resource is not in-flight! */
   void ReleaseSlot(const int32_t idx) {
      freedIdx.Append(idx);
   }

private:
   int32_t _max;
   ctDynamicArray<int32_t> freedIdx;
   int32_t nextNewIdx;
};

struct ctGPUBindlessManager {
   ctResults Startup(ctGPUDevice* pDevice, ctGPUBindlessManagerCreateInfo* pCreateInfo);
   ctResults Shutdown(ctGPUDevice* pDevice);
   ctResults PrepareFrame(ctGPUDevice* pDevice,
                          uint32_t architectCount,
                          struct ctGPUArchitectVulkan** ppArchitects);
   /* Automatics */
   int32_t fixedTextureBindUpperBound;
   int32_t fixedStorageBufferBindUpperBound;
   /* Bindless System */
   VkDescriptorSetLayout vkGlobalDescriptorSetLayout;
   VkDescriptorPool vkDescriptorPool;
   int32_t currentFrame = 0;
   VkDescriptorSet vkGlobalDescriptorSet[CT_MAX_INFLIGHT_FRAMES];
   VkPipelineLayout vkGlobalPipelineLayout;
   ctVkBindlessSlotManager slotManagerSamplers;
   ctVkBindlessSlotManager slotManagerSampledImage;
   ctVkBindlessSlotManager slotManagerStorageImage;
   ctVkBindlessSlotManager slotManagerStorageBuffer;
   int32_t maxSamplers = CT_MAX_GFX_SAMPLERS;
   int32_t maxSampledImages = CT_MAX_GFX_SAMPLED_IMAGES;
   int32_t maxStorageImages = CT_MAX_GFX_STORAGE_IMAGES;
   int32_t maxStorageBuffers = CT_MAX_GFX_STORAGE_BUFFERS;
   int32_t maxUniformBuffers = CT_MAX_GFX_UNIFORM_BUFFERS;

   inline ctResults AddBufferWrite(int32_t dstFrame, int32_t idx, VkBuffer buffer) {
      VkWriteDescriptorSet write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
      write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
      write.dstBinding = GLOBAL_BIND_STORAGE_BUFFER;
      write.dstSet = vkGlobalDescriptorSet[dstFrame];
      write.descriptorCount = 1;
      write.dstArrayElement = idx;

      VkDescriptorBufferInfo buffInfo = {};
      buffInfo.buffer = buffer;
      buffInfo.offset = 0;
      buffInfo.range = VK_WHOLE_SIZE;
      if (buffInfos[dstFrame].Count() >= buffInfos[dstFrame].Capacity()) {
         return CT_FAILURE_OUT_OF_MEMORY;
      }
      buffInfos[dstFrame].Append(buffInfo);
      write.pBufferInfo = &buffInfos[dstFrame].Last();
      descriptorSetWrites[dstFrame].Append(write);
      return CT_SUCCESS;
   }
   inline ctResults AddTextureWrite(int32_t dstFrame, int32_t idx, VkImageView view) {
      VkWriteDescriptorSet write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
      write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
      write.dstBinding = GLOBAL_BIND_SAMPLED_IMAGE;
      write.dstSet = vkGlobalDescriptorSet[dstFrame];
      write.descriptorCount = 1;
      write.dstArrayElement = idx;

      VkDescriptorImageInfo imageInfo = {};
      imageInfo.imageView = view;
      imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      if (imageInfos[dstFrame].Count() >= imageInfos[dstFrame].Capacity()) {
         return CT_FAILURE_OUT_OF_MEMORY;
      }
      imageInfos[dstFrame].Append(imageInfo);
      write.pImageInfo = &imageInfos[dstFrame].Last();
      descriptorSetWrites[dstFrame].Append(write);
      return CT_SUCCESS;
   }
   ctDynamicArray<VkDescriptorBufferInfo>
     buffInfos[CT_MAX_INFLIGHT_FRAMES]; /* don't resize at runtime */
   ctDynamicArray<VkDescriptorImageInfo>
     imageInfos[CT_MAX_INFLIGHT_FRAMES]; /* don't resize at runtime */
   ctDynamicArray<VkWriteDescriptorSet> descriptorSetWrites[CT_MAX_INFLIGHT_FRAMES];

   ctDynamicArray<int32_t> trackedDynamicBufferIndices;
   ctDynamicArray<int32_t> trackedDynamicBufferLastFrames;
   ctDynamicArray<ctGPUExternalBuffer*> trackedDynamicBuffers;

   ctDynamicArray<int32_t> trackedDynamicTextureIndices;
   ctDynamicArray<int32_t> trackedDynamicTextureLastFrames;
   ctDynamicArray<ctGPUExternalTexture*> trackedDynamicTextures;
};