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
#include "vulkan/vulkan.h"
#include "SDL_vulkan.h"
#include "vma/vk_mem_alloc.h"

#if CITRUS_TRACY
#include "tracy/TracyVulkan.hpp"
#endif

#include "core/EngineCore.hpp"

#define CT_VK_CHECK(_func, _msg)                                                         \
   {                                                                                     \
      const VkResult _tmpvresult = _func;                                                \
      if (_tmpvresult != VK_SUCCESS) { ctFatalError((int)_tmpvresult, #_msg); }          \
   }

struct ctVkQueueFamilyIndices {
   uint32_t graphicsIdx;
   uint32_t presentIdx;
   uint32_t computeIdx;
   uint32_t transferIdx;
};

class ctVkSwapchainSupport {
public:
   VkSurfaceCapabilitiesKHR surfaceCapabilities;
   ctDynamicArray<VkSurfaceFormatKHR> surfaceFormats;
   ctDynamicArray<VkPresentModeKHR> presentModes;
   void GetSupport(VkPhysicalDevice gpu, VkSurfaceKHR surface);
};

struct ctVkScreenResizeCallback {
   void (*callback)(uint32_t width, uint32_t height, void*);
   void* userData;
};

class ctVkScreenResources {
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
   ctDynamicArray<VkImageView> swapImageViews;

   bool resizeTriggered;

   ctResults CreateSurface(class ctVkBackend* pBackend, SDL_Window* pWindow);
   ctResults CreateSwapchain(class ctVkBackend* pBackend,
                             ctVkQueueFamilyIndices indices,
                             int32_t vsyncLevel,
                             VkSwapchainKHR oldSwapchain);
   ctResults DestroySurface(class ctVkBackend* pBackend);
   ctResults DestroySwapchain(class ctVkBackend* pBackend);

   ctDynamicArray<ctVkScreenResizeCallback> screenResizeCallbacks;
};

class ctVkDescriptorManager {
public:
   ctVkDescriptorManager();
   ctVkDescriptorManager(int32_t max);

   /* Get the next open slot to place a resource in the bindless system */
   int32_t AllocateSlot();
   /* Only call once the resource is not in-flight! */
   void ReleaseSlot(const int32_t idx);

private:
   int32_t _max;
   ctDynamicArray<int32_t> freedIdx;
   int32_t nextNewIdx;
};

class ctVkCommandBufferManager {
public:
   ctResults Create(class ctVkBackend* pBackend, uint32_t max, uint32_t familyIdx);
   ctResults Destroy(class ctVkBackend* pBackend);

   VkCommandBuffer GetNextCommandBuffer();
   VkResult SubmitCommands(VkQueue queue,
                           uint32_t signalSemaphoreCount,
                           VkSemaphore* pSignalSemaphores,
                           uint32_t waitSemaphoreCount,
                           VkSemaphore* pWaitSemaphores,
                           VkFence fence = VK_NULL_HANDLE,
                           VkPipelineStageFlags* pCustomWaitStages = NULL);

private:
   VkCommandPool pool;
   ctDynamicArray<VkCommandBuffer> cmdBuffers;
   uint32_t activeBufferCount;
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

class ctVkBackend : public ctModuleBase {
public:
   ctResults Startup() final;
   ctResults Shutdown() final;

   bool isValidationLayersAvailible();
   VkPhysicalDevice
   PickBestDevice(VkPhysicalDevice* pGpus, uint32_t count, VkSurfaceKHR surface);
   ctVkQueueFamilyIndices FindQueueFamilyIndices(VkPhysicalDevice gpu);

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
                                 VkImageUsageFlags usage,
                                 VmaAllocationCreateFlags allocFlags,
                                 size_t size,
                                 VmaMemoryUsage memUsage = VMA_MEMORY_USAGE_GPU_ONLY,
                                 int32_t bufferFlags = 0,
                                 VkSharingMode sharing = VK_SHARING_MODE_EXCLUSIVE,
                                 uint32_t queueFamilyIndexCount = 0,
                                 uint32_t* pQueueFamilyIndices = NULL);

   void TryDestroyCompleteImage(ctVkCompleteImage& fullImage);
   void TryDestroyCompleteBuffer(ctVkCompleteBuffer& fullBuffer);

   /* Vulkan Objects */
   VkAllocationCallbacks vkAllocCallback;
   VkDebugReportCallbackEXT vkDebugCallback;
   ctDynamicArray<const char*> validationLayers;
   ctDynamicArray<const char*> instanceExtensions;
   VkApplicationInfo vkAppInfo;
   VkInstance vkInstance;
   VkPhysicalDevice vkPhysicalDevice;
   VkDevice vkDevice;
   VmaAllocator vmaAllocator;

   ctVkQueueFamilyIndices queueFamilyIndices;
   VkQueue graphicsQueue;
   VkQueue presentQueue;
   VkQueue computeQueue;
   VkQueue transferQueue;

   int32_t currentFrame;
   ctVkCommandBufferManager graphicsCommands[CT_MAX_INFLIGHT_FRAMES];
   ctVkCommandBufferManager computeCommands[CT_MAX_INFLIGHT_FRAMES];
   ctVkCommandBufferManager transferCommands[CT_MAX_INFLIGHT_FRAMES];

   VkPipelineCache vkPipelineCache;

   VkDescriptorSetLayout vkDescriptorSetLayout;
   VkDescriptorPool vkDescriptorPool;
   VkDescriptorSet vkGlobalDescriptorSet;
   ctVkDescriptorManager descriptorsSamplers;
   ctVkDescriptorManager descriptorsSampledImage;
   ctVkDescriptorManager descriptorsStorageImage;
   ctVkDescriptorManager descriptorsStorageBuffer;

   ctVkScreenResources mainScreenResources;

   /* Settings */
   int32_t preferredDevice;
   int32_t validationEnabled;

   int32_t maxSamplers;
   int32_t maxSampledImages;
   int32_t maxStorageImages;
   int32_t maxStorageBuffers;

   int32_t maxGraphicsCommandBuffers;
   int32_t maxComputeCommandBuffers;
   int32_t maxTransferCommandBuffers;

   int32_t vsync;
};