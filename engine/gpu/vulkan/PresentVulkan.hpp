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

#include "gpu/Present.h"
#include "DeviceVulkan.hpp"

struct CT_API ctVkScreenResizeCallback {
   void (*callback)(uint32_t width, uint32_t height, void*);
   void* userData;
};

struct ctGPUPresenter {
   bool wantsVsync;
   SDL_Window* pWindow;

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

   int32_t currentFrame;
   VkSemaphore imageAvailible[CT_MAX_INFLIGHT_FRAMES];

   ctResults CreateSurface(struct ctGPUDevice* pDevice);
   ctResults CreateSwapchain(struct ctGPUDevice* pDevice);
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

   VkCommandPool blitCommandPool;
   VkCommandBuffer blitCommands[CT_MAX_INFLIGHT_FRAMES];
   uint32_t frameIdx;
};