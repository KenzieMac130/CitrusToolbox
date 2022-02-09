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

#pragma once

#include "utilities/Common.h"
#include "DeviceVulkan.hpp"
#include "gpu/Bindless.h"

class CT_API ctVkDescriptorManager {
public:
   ctVkDescriptorManager() {
      nextNewIdx = 0;
      _max = 0;
   }
   ctVkDescriptorManager(int32_t max) {
      nextNewIdx = 0;
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
   /* Bindless System */
   VkDescriptorSetLayout vkGlobalDescriptorSetLayout;
   VkDescriptorPool vkDescriptorPool;
   VkDescriptorSet vkGlobalDescriptorSet;
   VkPipelineLayout vkGlobalPipelineLayout;
   ctVkDescriptorManager descriptorsSamplers;
   ctVkDescriptorManager descriptorsSampledImage;
   ctVkDescriptorManager descriptorsStorageImage;
   ctVkDescriptorManager descriptorsStorageBuffer;
   int32_t maxSamplers = CT_MAX_GFX_SAMPLERS;
   int32_t maxSampledImages = CT_MAX_GFX_SAMPLED_IMAGES;
   int32_t maxStorageImages = CT_MAX_GFX_STORAGE_IMAGES;
   int32_t maxStorageBuffers = CT_MAX_GFX_STORAGE_BUFFERS;
   int32_t maxUniformBuffers = CT_MAX_GFX_UNIFORM_BUFFERS;

   // todo: better api for filling in descriptors
   void ExposeBindlessStorageBuffer(ctGPUDevice* pDevice,
                                    int32_t& outIdx,
                                    VkBuffer buffer,
                                    VkDeviceSize range = VK_WHOLE_SIZE,
                                    VkDeviceSize offset = 0);
   void ReleaseBindlessStorageBuffer(ctGPUDevice* pDevice, int32_t idx);
   void ExposeBindlessSampledImage(
     ctGPUDevice* pDevice,
     int32_t& outIdx,
     VkImageView view,
     VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
     VkSampler sampler = VK_NULL_HANDLE);
   void ReleaseBindlessSampledImage(int32_t idx);
};