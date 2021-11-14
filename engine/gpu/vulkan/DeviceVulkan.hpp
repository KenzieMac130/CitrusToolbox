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
#include "core/FileSystem.hpp"

#define CT_VK_CHECK(_func, _msg)                                                         \
   {                                                                                     \
      const VkResult _tmpvresult = _func;                                                \
      if (_tmpvresult != VK_SUCCESS) { ctFatalError((int)_tmpvresult, _msg); }           \
   }
#define PIPELINE_CACHE_FILE_PATH "VK_PIPELINE_CACHE"

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
   };
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
};