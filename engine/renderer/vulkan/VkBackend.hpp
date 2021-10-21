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

#include "core/ModuleBase.hpp"
#include "core/Translation.hpp"

#define CT_VK_CHECK(_func, _msg)                                                         \
   {                                                                                     \
      const VkResult _tmpvresult = _func;                                                \
      if (_tmpvresult != VK_SUCCESS) { ctFatalError((int)_tmpvresult, _msg); }           \
   }

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

   ctResults CreateSurface(class ctVkBackend* pBackend, SDL_Window* pWindow);
   ctResults CreateSwapchain(class ctVkBackend* pBackend,
                             ctVkQueueFamilyIndices indices,
                             int32_t vsyncLevel,
                             VkSwapchainKHR oldSwapchain);
   ctResults CreatePresentResources(class ctVkBackend* pBackend);
   ctResults DestroySurface(class ctVkBackend* pBackend);
   ctResults DestroySwapchain(class ctVkBackend* pBackend);
   ctResults DestroyPresentResources(class ctVkBackend* pBackend);

   bool HandleResizeIfNeeded(class ctVkBackend* pBackend);
   bool ShouldSkip();

   VkResult BlitAndPresent(class ctVkBackend* pBackend,
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

class CT_API ctVkDescriptorManager {
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

struct ctVkCompleteImage {
   VkImage image;
   VkImageView view;
   VmaAllocation alloc;
};

struct ctVkCompleteBuffer {
   VkBuffer buffer;
   VmaAllocation alloc;
};

class CT_API ctVkBackend : public ctModuleBase {
public:
   ctResults Startup() final;
   ctResults Shutdown() final;

   bool isValidationLayersAvailible();
   VkPhysicalDevice
   PickBestDevice(VkPhysicalDevice* pGpus, uint32_t count, VkSurfaceKHR surface);
   ctVkQueueFamilyIndices FindQueueFamilyIndices(VkPhysicalDevice gpu);
   bool DeviceHasRequiredExtensions(VkPhysicalDevice gpu);

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

   VkResult CreateShaderModuleFromWad(VkShaderModule& shader,
                                      struct ctWADReader& reader,
                                      int32_t fxIdx,
                                      const char* name);
   VkResult CreateBindlessPipelineLayout(VkPipelineLayout& layout,
                                         uint32_t pushConstantRangeCount = 0,
                                         VkPushConstantRange* pPushConstantRanges = NULL);
   VkResult CreateGraphicsPipeline(
     VkPipeline& pipeline,
     VkPipelineLayout layout,
     VkRenderPass renderpass,
     uint32_t subpass,
     VkShaderModule vertexShader,
     VkShaderModule fragShader,
     VkShaderModule controlShader = VK_NULL_HANDLE,
     VkShaderModule evaluationShader = VK_NULL_HANDLE,
     VkShaderModule taskShader = VK_NULL_HANDLE,
     VkShaderModule meshShader = VK_NULL_HANDLE,
     bool depthTest = false,
     bool depthWrite = true,
     bool blendEnable = false, /* Enable blending on defaults */
     uint32_t colorBlendCount = 1,
     VkPipelineColorBlendAttachmentState* pCustomBlends = NULL,
     VkFrontFace winding = VK_FRONT_FACE_CLOCKWISE,
     VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT,
     VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
     VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT,
     uint32_t customDynamicCount = 0,
     VkDynamicState* pDynamicStates = NULL);

   void TryDestroyCompleteImage(ctVkCompleteImage& fullImage);
   void TryDestroyCompleteBuffer(ctVkCompleteBuffer& fullBuffer);

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

   VkPhysicalDeviceProperties vPhysicalDeviceProperties;
   VkPhysicalDeviceFeatures vPhysicalDeviceFeatures;
   VkPhysicalDeviceDescriptorIndexingFeatures vDescriptorIndexingFeatures;
   VkPhysicalDeviceFeatures2 vPhysicalDeviceFeatures2;

   ctVkQueueFamilyIndices queueFamilyIndices;
   VkQueue graphicsQueue;
   VkQueue presentQueue;
   VkQueue computeQueue;
   VkQueue transferQueue;

   int32_t currentFrame;
   inline void AdvanceNextFrame() {
      currentFrame = (currentFrame + 1) % CT_MAX_INFLIGHT_FRAMES;
   };
   VkFence frameAvailibleFences[CT_MAX_INFLIGHT_FRAMES];
   VkResult WaitForFrameAvailible();

   VkPipelineCache vkPipelineCache;

   void RecreateSync();

   /* Bindless System */
   VkDescriptorSetLayout vkDescriptorSetLayout;
   VkDescriptorPool vkDescriptorPool;
   VkDescriptorSet vkGlobalDescriptorSet;
   ctVkDescriptorManager descriptorsSamplers;
   ctVkDescriptorManager descriptorsSampledImage;
   ctVkDescriptorManager descriptorsStorageImage;
   ctVkDescriptorManager descriptorsStorageBuffer;

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
   uint64_t AddStageToBuffer(VkBuffer src, VkBuffer dst, VkBufferCopy& copyInfo);
   uint64_t AddStageToImage(VkBuffer src,
                            VkImage dst,
                            VkImageLayout layout,
                            VkBufferImageCopy& copyInfo);
   ctResults CheckAsyncStageProgress(uint64_t idx);
   uint64_t stagingPhase;
   VkCommandBuffer stagingCommands[CT_MAX_INFLIGHT_FRAMES];

   /* Screen */
   ctVkScreenResources mainScreenResources;

   /* Settings */
   int32_t preferredDevice;
   int32_t validationEnabled;

   int32_t maxSamplers;
   int32_t maxSampledImages;
   int32_t maxStorageImages;
   int32_t maxStorageBuffers;

   int32_t vsync;
   int32_t nextFrameTimeout;

   int32_t useMeshShaders;
   int32_t defaultTessAmount;
};