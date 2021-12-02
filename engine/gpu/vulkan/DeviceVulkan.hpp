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
#include "utilities/Common.h"

#include "vulkan/vulkan.h"
#include "SDL_vulkan.h"
#include "vma/vk_mem_alloc.h"

#if CITRUS_TRACY
#include "tracy/TracyVulkan.hpp"
#endif

#include "core/Translation.hpp"

#define CT_VK_CHECK(_func, _msg)                                                         \
   {                                                                                     \
      const VkResult _tmpvresult = _func;                                                \
      if (_tmpvresult != VK_SUCCESS) { ctFatalError((int)_tmpvresult, _msg); }           \
   }
#define PIPELINE_CACHE_FILE_PATH "VK_PIPELINE_CACHE"
#define CT_MAX_CONVEYOR_FRAMES   CT_MAX_INFLIGHT_FRAMES + 1

#define GLOBAL_BIND_SAMPLER        0
#define GLOBAL_BIND_SAMPLED_IMAGE  1
#define GLOBAL_BIND_STORAGE_IMAGE  2
#define GLOBAL_BIND_STORAGE_BUFFER 3

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

struct CT_API ctVkScreenResizeCallback {
   void (*callback)(uint32_t width, uint32_t height, void*);
   void* userData;
};

class CT_API ctVkScreenResources {
public:
   SDL_Window* window;
   VkSurfaceKHR surface;
   VkSwapchainKHR swapchain;
   VkSurfaceFormatKHR surfaceFormat;
   VkPresentModeKHR presentMode;

   ctVkSwapchainSupport swapChainSupport;

   VkExtent2D extent;
   uint32_t imageCount;
   ctDynamicArray<VkImage> swapImages;

   bool resizeTriggered;
   bool isMinimized;
   VkSemaphore imageAvailible[CT_MAX_INFLIGHT_FRAMES];

   ctResults CreateSurface(struct ctGPUDevice* pDevice, SDL_Window* pWindow);
   ctResults CreateSwapchain(struct ctGPUDevice* pDevice,
                             ctVkQueueFamilyIndices indices,
                             int32_t vsyncLevel,
                             VkSwapchainKHR oldSwapchain);
   ctResults CreatePresentResources(struct ctGPUDevice* pDevice);
   ctResults DestroySurface(struct ctGPUDevice* pDevice);
   ctResults DestroySwapchain(struct ctGPUDevice* pDevice);
   ctResults DestroyPresentResources(struct ctGPUDevice* pDevice);

   bool HandleResizeIfNeeded(struct ctGPUDevice* pDevice);
   bool ShouldSkip();

   VkResult BlitAndPresent(struct ctGPUDevice* pDevice,
                           uint32_t blitQueueIdx,
                           VkQueue blitQueue,
                           uint32_t semaphoreCount,
                           VkSemaphore* pWaitSemaphores,
                           VkImage srcImage,
                           VkImageLayout srcLayout,
                           VkPipelineStageFlags srcStageMask,
                           VkAccessFlags srcAccess,
                           uint32_t srcQueueFamily,
                           VkImageBlit blit);

   ctDynamicArray<ctVkScreenResizeCallback> screenResizeCallbacks;

private:
   VkCommandPool blitCommandPool;
   VkCommandBuffer blitCommands[CT_MAX_INFLIGHT_FRAMES];
   uint32_t frameIdx;
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
struct ctGPUDevice {
   ctResults Startup();
   ctResults Shutdown();

   /* Settings */
   bool validationEnabled;
   bool vsync;
   int32_t preferredDevice;
   int32_t nextFrameTimeout;

   /* Extern */
   SDL_Window* pMainWindow;
   ctGPUOpenCacheFileFn fpOpenCacheFileCallback;
   void* pCacheCallbackCustomData;
   ctGPUOpenAssetFileFn fpOpenAssetFileCallback;
   void* pAssetCallbackCustomData;
   ctFile OpenAssetFile(ctGPUAssetIdentifier* pAssetIdentifier);

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

   VkPhysicalDeviceProperties vPhysicalDeviceProperties;
   VkPhysicalDeviceFeatures vPhysicalDeviceFeatures;
   VkPhysicalDeviceDescriptorIndexingFeatures vDescriptorIndexingFeatures;
   VkPhysicalDeviceFeatures2 vPhysicalDeviceFeatures2;

   ctVkQueueFamilyIndices queueFamilyIndices;
   VkQueue graphicsQueue;
   VkQueue presentQueue;
   VkQueue computeQueue;
   VkQueue transferQueue;

   ctVkScreenResources mainScreenResources;

   int32_t conveyorFrame;
   int32_t currentFrame;
   VkFence frameAvailibleFences[CT_MAX_INFLIGHT_FRAMES];

   /* Functions */
   bool isValidationLayersAvailible();
   VkPhysicalDevice
   PickBestDevice(VkPhysicalDevice* pGpus, uint32_t count, VkSurfaceKHR surface);
   ctVkQueueFamilyIndices FindQueueFamilyIndices(VkPhysicalDevice gpu);
   bool DeviceHasRequiredExtensions(VkPhysicalDevice gpu);
   inline void AdvanceNextFrame() {
      currentFrame = (currentFrame + 1) % CT_MAX_INFLIGHT_FRAMES;
      conveyorFrame = (conveyorFrame + 1) % CT_MAX_CONVEYOR_FRAMES;
      AdvanceStagingCooldownTimers();
   };

   /* Used to find safe resources that have "fallen off the conveyor belt"*/
   inline int32_t GetNextSafeReleaseConveyor() {
      return (conveyorFrame + 1) % CT_MAX_CONVEYOR_FRAMES;
   }
   VkResult WaitForFrameAvailible();
   void RecreateSync();

   /* Helpers */
   VkResult CreateCompleteImage(ctVkCompleteImage& fullImage,
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
   VkResult CreateCompleteBuffer(ctVkCompleteBuffer& fullBuffer,
                                 VkBufferUsageFlags usage,
                                 VmaAllocationCreateFlags allocFlags,
                                 size_t size,
                                 VmaMemoryUsage memUsage = VMA_MEMORY_USAGE_GPU_ONLY,
                                 int32_t bufferFlags = 0,
                                 VkSharingMode sharing = VK_SHARING_MODE_EXCLUSIVE,
                                 uint32_t queueFamilyIndexCount = 0,
                                 uint32_t* pQueueFamilyIndices = NULL);
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

   // todo: better api for filling in descriptors
   void ExposeBindlessStorageBuffer(uint32_t& outIdx,
                                    VkBuffer buffer,
                                    VkDeviceSize range = VK_WHOLE_SIZE,
                                    VkDeviceSize offset = 0);
   void ReleaseBindlessStorageBuffer(uint32_t idx);
   void ExposeBindlessSampledImage(
     uint32_t& outIdx,
     VkImageView view,
     VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
     VkSampler sampler = VK_NULL_HANDLE);
   void ReleaseBindlessSampledImage(uint32_t idx);

   /* Staging */
   ctResults GetStagingBuffer(ctVkCompleteBuffer& fullBuffer,
                              size_t& offset,
                              uint8_t*& mapping,
                              size_t sizeRequest);
   void AdvanceStagingCooldownTimers();
   void DestroyAllStagingBuffers();
   struct StagingEntry {
      size_t size;
      uint8_t* mapping;
      ctVkCompleteBuffer buffer;
   };
   ctDynamicArray<int8_t> stagingBufferCooldown;
   ctDynamicArray<StagingEntry> stagingBuffers;

   /* Command buffers */
   struct CommandBufferManager {
      void Startup(ctGPUDevice* pDevice, uint32_t queueFamilyIdx);
      void Shutdown(ctGPUDevice* pDevice);
      void BeginFrame(ctGPUDevice* pDevice);
      void Submit(ctGPUDevice* pDevice, VkQueue queue);
      VkCommandBuffer cmd[CT_MAX_INFLIGHT_FRAMES];
      VkCommandPool pool;
   };
   CommandBufferManager graphicsCommands;
   CommandBufferManager computeCommands;
   CommandBufferManager transferCommands;
};

/* Conveyor belt resource handling */
struct ctVkConveyorBeltResource {
   virtual ctResults Create(ctGPUDevice* pDevice) = 0;
   virtual ctResults Update(ctGPUDevice* pDevice) = 0;
   virtual ctResults Free(ctGPUDevice* pDevice) = 0;
};

struct ctVkConveyorBeltPool {
   ctDynamicArray<ctVkConveyorBeltResource*> live;
   ctDynamicArray<ctVkConveyorBeltResource*> toAllocate;
   ctDynamicArray<ctVkConveyorBeltResource*> toUpdate;
   ctDynamicArray<ctVkConveyorBeltResource*> toFree[CT_MAX_CONVEYOR_FRAMES];

   inline void Add(ctVkConveyorBeltResource* pResource) {
      toAllocate.Append(pResource);
   }
   inline void Update(ctVkConveyorBeltResource* pResource) {
      toUpdate.Append(pResource);
   }
   inline void Release(ctVkConveyorBeltResource* pResource,
                       int32_t currentConveyorFrame) {
      toFree[currentConveyorFrame].Append(pResource);
      live.Remove(pResource);
   }
   inline void Shutdown(ctGPUDevice* pDevice) {
      for (size_t i = 0; i < live.Count(); i++) {
         live[i]->Free(pDevice);
      }
   }
   inline ctResults Flush(ctGPUDevice* pDevice, int32_t nextFreeFrame) {
      for (size_t i = 0; i < toAllocate.Count(); i++) {
         toAllocate[i]->Create(pDevice);
         live.Append(toAllocate[i]);
      }
      for (size_t i = 0; i < toUpdate.Count(); i++) {
         toUpdate[i]->Update(pDevice);
      }
      for (size_t i = 0; i < toFree[nextFreeFrame].Count(); i++) {
         toFree[nextFreeFrame][i]->Free(pDevice);
         delete toFree[nextFreeFrame][i];
      }
      toAllocate.Clear();
      toUpdate.Clear();
      toFree[nextFreeFrame].Clear();
      return CT_SUCCESS;
   }
};