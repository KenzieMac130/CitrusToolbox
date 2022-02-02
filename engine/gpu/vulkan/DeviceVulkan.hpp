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

#include "gpu/Device.h"
#include "gpu/shared/DeviceBase.hpp"
#include "utilities/Common.h"

#include "vulkan/vulkan.h"
#include "SDL_vulkan.h"
#include "vma/vk_mem_alloc.h"

#if CITRUS_TRACY
#include "tracy/TracyVulkan.hpp"
#endif

/* todo: replace need with generic fatal error callback */
#ifndef CITRUS_GPU_NO_TRANSLATION
#include "core/Translation.hpp"
#else
#define CT_NCT(_tag, _txt)
#define CT_NC(_txt)
#endif

#define CT_VK_CHECK(_func, _msg)                                                         \
   {                                                                                     \
      const VkResult _tmpvresult = _func;                                                \
      if (_tmpvresult != VK_SUCCESS) { ctFatalError((int)_tmpvresult, _msg); }           \
   }
#define PIPELINE_CACHE_FILE_PATH "VK_PIPELINE_CACHE"

#define GLOBAL_BIND_SAMPLER        0
#define GLOBAL_BIND_SAMPLED_IMAGE  1
#define GLOBAL_BIND_STORAGE_IMAGE  2
#define GLOBAL_BIND_STORAGE_BUFFER 3
#define GLOBAL_BIND_UNIFORM_BUFFER 4

struct ctVkQueueFamilyIndices {
   uint32_t graphicsIdx;
   uint32_t presentIdx;
   uint32_t computeIdx;
   uint32_t transferIdx;
};

class CT_API ctVkSwapchainSupport {
public:
   VkSurfaceCapabilitiesKHR surfaceCapabilities;
   ctDynamicArray<VkSurfaceFormatKHR> surfaceFormats;
   ctDynamicArray<VkPresentModeKHR> presentModes;
   void GetSupport(VkPhysicalDevice gpu, VkSurfaceKHR surface);
};

struct ctVkCompleteImage {
   VkImage image;
   VkImageView view;
   VmaAllocation alloc;
};

struct ctVkCompleteBuffer {
   VkBuffer buffer;
   VmaAllocation alloc;
};

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

/* -------- Define Structure -------- */
struct ctGPUDevice : ctGPUDeviceBase {
   ctResults Startup();
   ctResults Shutdown();

   /* Settings */
   bool validationEnabled;
   int32_t preferredDevice;
   int32_t nextFrameTimeout;

   /* Vulkan Objects */
   VkAllocationCallbacks vkAllocCallback;
   VkDebugReportCallbackEXT vkDebugCallback;
   ctDynamicArray<const char*> validationLayers;
   ctDynamicArray<const char*> instanceExtensions;
   ctDynamicArray<const char*> deviceExtensions;
   VkApplicationInfo vkAppInfo;
   VkInstance vkInstance;
   VkPhysicalDevice vkPhysicalDevice;
   VkDevice vkDevice;
   VmaAllocator vmaAllocator;
   VkPipelineCache vkPipelineCache;

   VkSurfaceKHR vkInitialSurface;

   VkPhysicalDeviceProperties vPhysicalDeviceProperties;
   VkPhysicalDeviceFeatures vPhysicalDeviceFeatures;
   VkPhysicalDeviceDescriptorIndexingFeatures vDescriptorIndexingFeatures;
   VkPhysicalDeviceFeatures2 vPhysicalDeviceFeatures2;

   ctVkQueueFamilyIndices queueFamilyIndices;
   VkQueue graphicsQueue;
   VkQueue presentQueue;
   VkQueue computeQueue;
   VkQueue transferQueue;

   /* Functions */
   bool isValidationLayersAvailible();
   VkPhysicalDevice
   PickBestDevice(VkPhysicalDevice* pGpus, uint32_t count, VkSurfaceKHR surface);
   ctVkQueueFamilyIndices FindQueueFamilyIndices(VkPhysicalDevice gpu);
   bool DeviceHasRequiredExtensions(VkPhysicalDevice gpu);
   void ApplyOptionalDeviceExtensions(VkPhysicalDevice gpu);

   /* Helpers */
   VkResult CreateCompleteImage(const char* name,
                                ctVkCompleteImage& fullImage,
                                VkFormat format,
                                VkImageUsageFlags usage,
                                VmaAllocationCreateFlags allocFlags,
                                VkImageAspectFlags aspect,
                                uint32_t width,
                                uint32_t height,
                                uint32_t depth = 1,
                                uint32_t mip = 1,
                                uint32_t layers = 1,
                                VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT,
                                VkImageType imageType = VK_IMAGE_TYPE_2D,
                                VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D,
                                VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL,
                                VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                                VmaMemoryUsage memUsage = VMA_MEMORY_USAGE_GPU_ONLY,
                                int32_t imageFlags = 0,
                                VkSharingMode sharing = VK_SHARING_MODE_EXCLUSIVE,
                                uint32_t queueFamilyIndexCount = 0,
                                uint32_t* pQueueFamilyIndices = NULL);
   VkResult CreateCompleteBuffer(const char* name,
                                 ctVkCompleteBuffer& fullBuffer,
                                 VkBufferUsageFlags usage,
                                 VmaAllocationCreateFlags allocFlags,
                                 size_t size,
                                 VmaMemoryUsage memUsage = VMA_MEMORY_USAGE_GPU_ONLY,
                                 int32_t bufferFlags = 0,
                                 VkSharingMode sharing = VK_SHARING_MODE_EXCLUSIVE,
                                 uint32_t queueFamilyIndexCount = 0,
                                 uint32_t* pQueueFamilyIndices = NULL);

   /* Todo Wait for right opportunity to garbage collect */
   void TryDestroyCompleteImage(ctVkCompleteImage& fullImage);
   void TryDestroyCompleteBuffer(ctVkCompleteBuffer& fullBuffer);

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
   void ExposeBindlessStorageBuffer(int32_t& outIdx,
                                    VkBuffer buffer,
                                    VkDeviceSize range = VK_WHOLE_SIZE,
                                    VkDeviceSize offset = 0);
   void ReleaseBindlessStorageBuffer(int32_t idx);
   void ExposeBindlessSampledImage(
     int32_t& outIdx,
     VkImageView view,
     VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
     VkSampler sampler = VK_NULL_HANDLE);
   void ReleaseBindlessSampledImage(int32_t idx);

   /* Staging */
   ctResults GetStagingBuffer(ctVkCompleteBuffer& fullBuffer, size_t sizeRequest);
   ctResults ReleaseStagingBuffer(ctVkCompleteBuffer& fullBuffer);
   void AdvanceStagingCooldownTimers();
   void DestroyAllStagingBuffers();
   struct StagingEntry {
      size_t size;
      uint8_t* mapping;
      ctVkCompleteBuffer buffer;
   };
   ctDynamicArray<int8_t> stagingBufferCooldown;
   ctDynamicArray<StagingEntry> stagingBuffers;

   /* Just in time renderpass (for lack of dynamic rendering) */
   inline bool isDynamicRenderingSupported() {
      return false;
   }
   void DestroyJITRenderpasses();
   VkRenderPass GetJITPipelineRenderpass(VkPipelineRenderingCreateInfoKHR& info);
   ctHashTable<VkRenderPass, uint32_t> pipelineRenderpasses;
   ctSpinLock jitPipelineRenderpassLock;

   void BeginJITRenderPass(VkCommandBuffer commandBuffer,
                           const VkRenderingInfoKHR* pRenderingInfo,
                           VkFormat depthStencilFormat,
                           VkFormat* pColorFormats,
                           VkImageLayout lastDepthLayout,
                           VkImageLayout* pLastColorLayouts);
   void EndJITRenderpass(VkCommandBuffer commandBuffer);
   struct CompleteRenderInfo {
      VkRenderPass renderpass;
      VkFramebuffer framebuffer;
   };
   CompleteRenderInfo CreateCompleteRenderInfo(const VkRenderingInfoKHR* pRenderingInfo,
                                               VkFormat depthStencilFormat,
                                               VkFormat* pColorFormats,
                                               VkImageLayout lastDepthLayout,
                                               VkImageLayout* pLastColorLayouts);
   ctHashTable<CompleteRenderInfo, uint32_t> usableRenderInfo;
   ctSpinLock jitUsableRenderInfoLock;

   /* Debug */
   bool useMarkers;
   void SetupMarkers();
   void
   SetObjectMarker(const char* name, uint64_t object, VkDebugReportObjectTypeEXT type);
   void MarkBeginRegion(VkCommandBuffer cmd, const char* name);
   void MarkEndRegion(VkCommandBuffer cmd);
   PFN_vkDebugMarkerSetObjectTagEXT fpDebugMarkerSetObjectTag;
   PFN_vkDebugMarkerSetObjectNameEXT fpDebugMarkerSetObjectName;
   PFN_vkCmdDebugMarkerBeginEXT fpCmdDebugMarkerBegin;
   PFN_vkCmdDebugMarkerEndEXT fpCmdDebugMarkerEnd;
   PFN_vkCmdDebugMarkerInsertEXT fpCmdDebugMarkerInsert;
};