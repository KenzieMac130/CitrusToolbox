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

#include "PresentVulkan.hpp"
#include "ArchitectVulkan.hpp"

CT_API ctResults ctGPUPresenterStartup(ctGPUDevice* pDevice,
                                       ctGPUPresenter** ppPresenter,
                                       ctGPUPresenterCreateInfo* pCreateInfo) {
   ctGPUPresenter* pPresenter = new ctGPUPresenter();
   *ppPresenter = pPresenter;
   pPresenter->pWindow = (SDL_Window*)pCreateInfo->pWindow;
   pPresenter->wantsVsync = pCreateInfo->useVSync;
   CT_RETURN_FAIL(pPresenter->CreateSurface(pDevice));
   CT_RETURN_FAIL(pPresenter->CreateSwapchain(pDevice));
   CT_RETURN_FAIL(pPresenter->CreatePresentResources(pDevice));
   return CT_SUCCESS;
}

CT_API ctResults ctGPUPresenterShutdown(ctGPUDevice* pDevice,
                                        ctGPUPresenter* pPresenter) {
   pPresenter->DestroyPresentResources(pDevice);
   pPresenter->DestroySwapchain(pDevice);
   pPresenter->DestroySurface(pDevice);
   return CT_SUCCESS;
}

CT_API ctResults ctGPUPresenterExecute(ctGPUDevice* pDevice,
                                       struct ctGPUPresenter* pPresenter,
                                       ctGPUArchitect* pArchitect) {
   if (!pArchitect->isRenderable) { return CT_FAILURE_DEPENDENCY_NOT_MET; }
   if (pPresenter->ShouldSkip()) { return CT_SUCCESS; }
   ctGPUArchitectVulkan* pVkArch = (ctGPUArchitectVulkan*)pArchitect;
   uint32_t srcQueueIdx;
   VkImage srcImage;
   uint32_t width;
   uint32_t height;
   VkImageLayout layout;
   VkAccessFlags access;
   VkPipelineStageFlags stage;
   if (!pVkArch->GetOutput(
         pDevice, width, height, srcQueueIdx, layout, access, stage, srcImage)) {
      return CT_FAILURE_CORRUPTED_CONTENTS;
   }

   VkImageBlit blitInfo;
   blitInfo.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
   blitInfo.dstSubresource.baseArrayLayer = 0;
   blitInfo.dstSubresource.mipLevel = 0;
   blitInfo.dstSubresource.layerCount = 1;
   blitInfo.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
   blitInfo.srcSubresource.baseArrayLayer = 0;
   blitInfo.srcSubresource.mipLevel = 0;
   blitInfo.srcSubresource.layerCount = 1;
   blitInfo.srcOffsets[0] = {0};
   blitInfo.srcOffsets[1] = {(int32_t)width, (int32_t)height, 1};
   blitInfo.dstOffsets[0] = {0};
   blitInfo.dstOffsets[1] = {
     (int32_t)pPresenter->extent.width, (int32_t)pPresenter->extent.height, 1};
   if (pPresenter->BlitAndPresent(pDevice,
                                  pDevice->queueFamilyIndices.graphicsIdx,
                                  pDevice->graphicsQueue,
                                  1,
                                  &pVkArch->outputReady[pVkArch->currentFrame],
                                  srcImage,
                                  layout,
                                  stage,
                                  access,
                                  srcQueueIdx,
                                  blitInfo) != VK_SUCCESS) {
      return CT_FAILURE_RUNTIME_ERROR;
   }
   return CT_SUCCESS;
}

CT_API ctGPUPresenterState ctGPUPresenterHandleState(ctGPUDevice* pDevice,
                                                     ctGPUPresenter* pPresenter,
                                                     uint32_t* pWidth,
                                                     uint32_t* pHeight) {
   ctAssert(pDevice);
   ctAssert(pPresenter);
   bool resizeTriggered = pPresenter->resizeTriggered;
   pPresenter->HandleResizeIfNeeded(pDevice);
   if (pWidth) { *pWidth = pPresenter->extent.width; }
   if (pHeight) { *pHeight = pPresenter->extent.height; }
   if (pPresenter->ShouldSkip()) { return CT_GPU_PRESENTER_INVALID; }
   if (resizeTriggered) { return CT_GPU_PRESENTER_RESIZED; }
   return CT_GPU_PRESENTER_NORMAL;
}

CT_API void ctGPUPresenterSignalStateChange(ctGPUDevice* pDevice,
                                            ctGPUPresenter* pPresenter,
                                            ctGPUPresenterState state) {
   if (state == CT_GPU_PRESENTER_RESIZED) { pPresenter->resizeTriggered = true; }
}

ctResults ctGPUPresenter::CreateSurface(ctGPUDevice* pDevice) {
   ZoneScoped;
   if (pDevice->pMainWindow == pWindow && pDevice->vkInitialSurface != VK_NULL_HANDLE) {
      surface = pDevice->vkInitialSurface;
      pDevice->vkInitialSurface = VK_NULL_HANDLE; /* steal ownership */
      return CT_SUCCESS;
   } else {
      ctDebugLog("Creating Surface...");
      if (SDL_Vulkan_CreateSurface(pWindow, pDevice->vkInstance, &surface)) {
         return CT_SUCCESS;
      }
   }
   return CT_FAILURE_UNKNOWN;
}

VkSurfaceFormatKHR vPickBestSurfaceFormat(VkSurfaceFormatKHR* formats, size_t count) {
   ZoneScoped;
   for (uint32_t i = 0; i < count; i++) {
      const VkSurfaceFormatKHR fmt = formats[i];
      if (fmt.format == VK_FORMAT_B8G8R8A8_SRGB &&
          fmt.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
         return fmt;
      }
   }
   return formats[0];
}

VkPresentModeKHR vPickPresentMode(VkPresentModeKHR* modes, size_t count, bool vsync) {
   ZoneScoped;
   int64_t bestIdx = 0;
   int64_t bestScore = -1;
   int64_t currentScore;

   for (uint32_t i = 0; i < count; i++) {
      const VkPresentModeKHR mode = modes[i];
      currentScore = 0;

      /* Present to screen ASAP, no V-Sync */
      if (mode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
         if (vsync == false) {
            currentScore = 1000;
         } else {
            currentScore = 0;
         }
      }
      /* Present and swap to backbuffer. Needs to wait if full. */
      if (mode == VK_PRESENT_MODE_FIFO_KHR) {
         if (vsync == true) {
            currentScore = 500;
         } else {
            currentScore = 0;
         }
      }
      /* Present and swap to backbuffer. Can continue if full. */
      if (mode == VK_PRESENT_MODE_FIFO_RELAXED_KHR) {
         if (vsync == true) {
            currentScore = 1000;
         } else {
            currentScore = 0;
         }
      }

      if (currentScore > bestScore) {
         bestIdx = i;
         bestScore = currentScore;
      }
   }
   return modes[bestIdx];
}

ctResults ctGPUPresenter::CreateSwapchain(ctGPUDevice* pDevice) {
   ZoneScoped;

   swapChainSupport.GetSupport(pDevice->vkPhysicalDevice, surface);
   if (swapChainSupport.surfaceCapabilities.currentExtent.width == 0 ||
       swapChainSupport.surfaceCapabilities.currentExtent.height == 0) {
      return CT_FAILURE_NOT_UPDATABLE;
   }

   ctDebugLog("Creating Swapchain...");

   /* Select a Image and Color Format */
   surfaceFormat = vPickBestSurfaceFormat(swapChainSupport.surfaceFormats.Data(),
                                          swapChainSupport.surfaceFormats.Count());

   /* Select a Present Mode */
   presentMode = vPickPresentMode(swapChainSupport.presentModes.Data(),
                                  swapChainSupport.presentModes.Count(),
                                  wantsVsync);

   /* Get Size */
   int w, h;
   SDL_Vulkan_GetDrawableSize(pWindow, &w, &h);
   uint32_t flags = SDL_GetWindowFlags(pWindow);
   extent.width = w;
   extent.height = h;

   /* Get Image Count */
   imageCount = swapChainSupport.surfaceCapabilities.minImageCount + 1;
   if (swapChainSupport.surfaceCapabilities.maxImageCount > 0 &&
       imageCount > swapChainSupport.surfaceCapabilities.maxImageCount) {
      imageCount = swapChainSupport.surfaceCapabilities.maxImageCount;
   }

   /* Create swapchain */
   uint32_t sharedIndices[] {pDevice->queueFamilyIndices.graphicsIdx,
                             pDevice->queueFamilyIndices.presentIdx};
   VkSwapchainCreateInfoKHR swapChainInfo = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
   swapChainInfo.surface = surface;
   swapChainInfo.imageFormat = surfaceFormat.format;
   swapChainInfo.imageColorSpace = surfaceFormat.colorSpace;
   swapChainInfo.presentMode = presentMode;
   swapChainInfo.imageExtent = extent;
   swapChainInfo.imageArrayLayers = 1;
   swapChainInfo.minImageCount = imageCount;
   swapChainInfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
   swapChainInfo.preTransform = swapChainSupport.surfaceCapabilities.currentTransform;
   swapChainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
   swapChainInfo.clipped = VK_TRUE;
   swapChainInfo.oldSwapchain = VK_NULL_HANDLE;
   if (sharedIndices[0] == sharedIndices[1]) {
      swapChainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
      swapChainInfo.queueFamilyIndexCount = 0;
      swapChainInfo.pQueueFamilyIndices = NULL;
   } else {
      swapChainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
      swapChainInfo.queueFamilyIndexCount = 2;
      swapChainInfo.pQueueFamilyIndices = sharedIndices;
   }
   CT_VK_CHECK(
     vkCreateSwapchainKHR(
       pDevice->vkDevice, &swapChainInfo, &pDevice->vkAllocCallback, &swapchain),
     CT_NCT("FAIL:vkCreateSwapchainKHR",
            "vkCreateSwapchainKHR() failed to create swapchain."));

   /* Get images */
   vkGetSwapchainImagesKHR(pDevice->vkDevice, swapchain, &imageCount, NULL);
   swapImages.Resize(imageCount);
   vkGetSwapchainImagesKHR(pDevice->vkDevice, swapchain, &imageCount, swapImages.Data());

   return CT_SUCCESS;
}

ctResults ctGPUPresenter::CreatePresentResources(ctGPUDevice* pDevice) {
   ZoneScoped;
   /* Create image availible semaphore */
   VkSemaphoreCreateInfo semaphoreInfo {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
   for (int i = 0; i < CT_MAX_INFLIGHT_FRAMES; i++) {
      vkCreateSemaphore(
        pDevice->vkDevice, &semaphoreInfo, &pDevice->vkAllocCallback, &imageAvailible[i]);
   }
   /* Blitting command buffer */
   VkCommandPoolCreateInfo poolInfo {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
   poolInfo.queueFamilyIndex = pDevice->queueFamilyIndices.transferIdx;
   poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
   vkCreateCommandPool(
     pDevice->vkDevice, &poolInfo, &pDevice->vkAllocCallback, &blitCommandPool);
   VkCommandBufferAllocateInfo allocInfo {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
   allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
   allocInfo.commandBufferCount = CT_MAX_INFLIGHT_FRAMES;
   allocInfo.commandPool = blitCommandPool;
   vkAllocateCommandBuffers(pDevice->vkDevice, &allocInfo, blitCommands);
   return CT_SUCCESS;
}

ctResults ctGPUPresenter::DestroySwapchain(ctGPUDevice* pDevice) {
   ZoneScoped;
   if (swapchain == VK_NULL_HANDLE) { return CT_FAILURE_NOT_FOUND; }
   vkDestroySwapchainKHR(pDevice->vkDevice, swapchain, &pDevice->vkAllocCallback);
   swapchain = VK_NULL_HANDLE;
   return CT_SUCCESS;
}

ctResults ctGPUPresenter::DestroyPresentResources(ctGPUDevice* pDevice) {
   ZoneScoped;
   for (int i = 0; i < CT_MAX_INFLIGHT_FRAMES; i++) {
      vkDestroySemaphore(pDevice->vkDevice, imageAvailible[i], &pDevice->vkAllocCallback);
   }
   vkDestroyCommandPool(pDevice->vkDevice, blitCommandPool, &pDevice->vkAllocCallback);
   return CT_SUCCESS;
}

bool ctGPUPresenter::HandleResizeIfNeeded(ctGPUDevice* pDevice) {
   if (isMinimized) {
      swapChainSupport.GetSupport(pDevice->vkPhysicalDevice, surface);
      if (swapChainSupport.surfaceCapabilities.currentExtent.width != 0 ||
          swapChainSupport.surfaceCapabilities.currentExtent.height != 0) {
         resizeTriggered = true;
         isMinimized = false;
      }
   }
   if (resizeTriggered) {
      vkDeviceWaitIdle(pDevice->vkDevice);
      DestroyPresentResources(pDevice);
      CreatePresentResources(pDevice);
      DestroySwapchain(pDevice);
      if (CreateSwapchain(pDevice) != VK_SUCCESS) { isMinimized = true; }
      resizeTriggered = false;
      return true;
   }
   return false;
}

bool ctGPUPresenter::ShouldSkip() {
   return swapChainSupport.surfaceCapabilities.currentExtent.width == 0 ||
          swapChainSupport.surfaceCapabilities.currentExtent.height == 0;
}

ctResults ctGPUPresenter::DestroySurface(ctGPUDevice* pDevice) {
   ZoneScoped;
   vkDestroySurfaceKHR(pDevice->vkInstance, surface, NULL);
   return CT_SUCCESS;
}

VkResult ctGPUPresenter::BlitAndPresent(ctGPUDevice* pDevice,
                                        uint32_t blitQueueIdx,
                                        VkQueue blitQueue,
                                        uint32_t semaphoreCount,
                                        VkSemaphore* pWaitSemaphores,
                                        VkImage srcImage,
                                        VkImageLayout srcLayout,
                                        VkPipelineStageFlags srcStageMask,
                                        VkAccessFlags srcAccess,
                                        uint32_t srcQueueFamily,
                                        VkImageBlit blit) {
   ZoneScoped;
   currentFrame = (currentFrame + 1) % CT_MAX_INFLIGHT_FRAMES;
   uint32_t imageIndex = 0;
   VkResult result = vkAcquireNextImageKHR(pDevice->vkDevice,
                                           swapchain,
                                           pDevice->nextFrameTimeout,
                                           imageAvailible[currentFrame],
                                           VK_NULL_HANDLE,
                                           &imageIndex);
   if (resizeTriggered || result == VK_ERROR_OUT_OF_DATE_KHR) {
      resizeTriggered = true;
      return VK_ERROR_OUT_OF_DATE_KHR;
   } else if (result != VK_SUCCESS) {
      return result;
   }

   /* Blit */
   {
      VkCommandBuffer cmd = blitCommands[currentFrame];
      VkCommandBufferBeginInfo beginInfo {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
      vkResetCommandBuffer(cmd, 0);
      vkBeginCommandBuffer(cmd, &beginInfo);
      pDevice->MarkBeginRegion(cmd, "Blit Output");

      VkImageMemoryBarrier srcToTransfer {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
      srcToTransfer.image = srcImage;
      srcToTransfer.srcQueueFamilyIndex = srcQueueFamily;
      srcToTransfer.dstQueueFamilyIndex = blitQueueIdx;
      srcToTransfer.oldLayout = srcLayout;
      srcToTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
      srcToTransfer.srcAccessMask = srcAccess;
      srcToTransfer.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
      srcToTransfer.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      srcToTransfer.subresourceRange.baseArrayLayer = 0;
      srcToTransfer.subresourceRange.baseMipLevel = 0;
      srcToTransfer.subresourceRange.levelCount = 1;
      srcToTransfer.subresourceRange.layerCount = 1;
      vkCmdPipelineBarrier(cmd,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_TRANSFER_BIT,
                           VK_DEPENDENCY_BY_REGION_BIT,
                           0,
                           NULL,
                           0,
                           NULL,
                           1,
                           &srcToTransfer);

      VkImageMemoryBarrier dstToTransfer {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
      dstToTransfer.image = swapImages[imageIndex];
      dstToTransfer.srcQueueFamilyIndex = pDevice->queueFamilyIndices.presentIdx;
      dstToTransfer.dstQueueFamilyIndex = blitQueueIdx;
      dstToTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      dstToTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
      dstToTransfer.srcAccessMask = 0;
      dstToTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      dstToTransfer.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      dstToTransfer.subresourceRange.baseArrayLayer = 0;
      dstToTransfer.subresourceRange.baseMipLevel = 0;
      dstToTransfer.subresourceRange.levelCount = 1;
      dstToTransfer.subresourceRange.layerCount = 1;
      vkCmdPipelineBarrier(cmd,
                           VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                           VK_PIPELINE_STAGE_TRANSFER_BIT,
                           VK_DEPENDENCY_BY_REGION_BIT,
                           0,
                           NULL,
                           0,
                           NULL,
                           1,
                           &dstToTransfer);
      vkCmdBlitImage(cmd,
                     srcImage,
                     VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                     swapImages[imageIndex],
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                     1,
                     &blit,
                     VK_FILTER_LINEAR);
      VkImageMemoryBarrier dstToPresent {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
      dstToPresent.image = swapImages[imageIndex];
      dstToPresent.srcQueueFamilyIndex = blitQueueIdx;
      dstToPresent.dstQueueFamilyIndex = pDevice->queueFamilyIndices.presentIdx;
      dstToPresent.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
      dstToPresent.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
      dstToPresent.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      dstToPresent.dstAccessMask = 0; /* Won't touch it */
      dstToPresent.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      dstToPresent.subresourceRange.baseArrayLayer = 0;
      dstToPresent.subresourceRange.baseMipLevel = 0;
      dstToPresent.subresourceRange.levelCount = 1;
      dstToPresent.subresourceRange.layerCount = 1;
      vkCmdPipelineBarrier(cmd,
                           VK_PIPELINE_STAGE_TRANSFER_BIT,
                           VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                           VK_DEPENDENCY_BY_REGION_BIT,
                           0,
                           NULL,
                           0,
                           NULL,
                           1,
                           &dstToPresent);
      pDevice->MarkEndRegion(cmd);
      vkEndCommandBuffer(cmd);
      VkSubmitInfo submitInfo {VK_STRUCTURE_TYPE_SUBMIT_INFO};
      VkPipelineStageFlags waitForStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
      submitInfo.commandBufferCount = 1;
      submitInfo.pCommandBuffers = &cmd;
      submitInfo.waitSemaphoreCount = semaphoreCount;
      submitInfo.pWaitSemaphores = pWaitSemaphores;
      submitInfo.pWaitDstStageMask = &waitForStage;
      vkQueueSubmit(blitQueue, 1, &submitInfo, VK_NULL_HANDLE);
   }

   VkPresentInfoKHR presentInfo {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
   presentInfo.waitSemaphoreCount = 1;
   presentInfo.pWaitSemaphores = &imageAvailible[currentFrame];
   presentInfo.swapchainCount = 1;
   presentInfo.pSwapchains = &swapchain;
   presentInfo.pImageIndices = &imageIndex;
   return vkQueuePresentKHR(pDevice->presentQueue, &presentInfo);
}