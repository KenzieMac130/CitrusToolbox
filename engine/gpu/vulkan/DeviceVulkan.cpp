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

#include "DeviceVulkan.hpp"

#ifdef _WIN32
#include <Windows.h>
#endif
#define VMA_IMPLEMENTATION
#include "vma/vk_mem_alloc.h"

/* -------- Expose to API -------- */
CT_API ctResults ctGPUDeviceStartup(ctGPUDevice** ppDevice,
                                    ctGPUDeviceCreateInfo* pCreateInfo,
                                    ctGPUDeviceCapabilities* pCapabilitiesOut) {
   ZoneScoped;
   ctAssert(ppDevice);
   ctAssert(pCreateInfo);
   *ppDevice = new ctGPUDevice();
   ctGPUDevice* pDevice = *ppDevice;
   VkApplicationInfo& appInfo = pDevice->vkAppInfo;
   appInfo.pApplicationName = pCreateInfo->appName;
   pDevice->validationEnabled = pCreateInfo->validationEnabled;
   pDevice->pMainWindow = (SDL_Window*)pCreateInfo->pMainWindow;
   pDevice->vsync = pCreateInfo->useVSync;
   pDevice->nextFrameTimeout = 10000;
   pDevice->fpOpenCacheFileCallback = pCreateInfo->fpOpenCacheFileCallback;
   pDevice->pCacheCallbackCustomData = pCreateInfo->pCacheCallbackCustomData;
   pDevice->fpOpenAssetFileCallback = pCreateInfo->fpOpenAssetFileCallback;
   pDevice->pAssetCallbackCustomData = pCreateInfo->pAssetCallbackCustomData;
   ctResults result = pDevice->Startup();
   if (pCapabilitiesOut) { memset(pCapabilitiesOut, 0, sizeof(*pCapabilitiesOut)); }
   return result;
}

CT_API ctResults ctGPUDeviceShutdown(ctGPUDevice* pDevice) {
   ZoneScoped;
   ctAssert(pDevice);
   ctResults result = pDevice->Shutdown();
   delete pDevice;
   return result;
}

CT_API ctResults ctGPUDeviceNextFrame(ctGPUDevice* pDevice) {
   return CT_SUCCESS;
}

/* -------- Implementation -------- */
VKAPI_ATTR VkBool32 VKAPI_CALL vDebugCallback(VkDebugReportFlagsEXT flags,
                                              VkDebugReportObjectTypeEXT objType,
                                              uint64_t srcObject,
                                              size_t location,
                                              int32_t msgCode,
                                              const char* pLayerPrefix,
                                              const char* pMsg,
                                              void* pUserData) {
   ZoneScoped;
   if ((flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) != 0) {
      ctDebugLog("VK Information: %s", pLayerPrefix, pMsg);
   } else {
      ctDebugError("VK Validation Layer: [%s] Code %u : %s", pLayerPrefix, msgCode, pMsg);
   }
   return VK_FALSE;
}

/* ------------- Alloc Functions ------------- */

void* vAllocFunction(void* pUserData,
                     size_t size,
                     size_t alignment,
                     VkSystemAllocationScope allocationScope) {
   ZoneScoped;
   return ctAlignedMalloc(size, alignment);
}

void* vReallocFunction(void* pUserData,
                       void* pOriginal,
                       size_t size,
                       size_t alignment,
                       VkSystemAllocationScope allocationScope) {
   ZoneScoped;
   return ctAlignedRealloc(pOriginal, size, alignment);
}

void vFreeFunction(void* pUserData, void* pMemory) {
   ZoneScoped;
   ctAlignedFree(pMemory);
}

bool ctGPUDevice::isValidationLayersAvailible() {
   ZoneScoped;
   uint32_t layerCount = 0;
   vkEnumerateInstanceLayerProperties(&layerCount, NULL);
   ctDynamicArray<VkLayerProperties> layerProps;
   layerProps.Resize(layerCount);
   vkEnumerateInstanceLayerProperties(&layerCount, layerProps.Data());
   for (uint32_t i = 0; i < layerCount; i++) {
      ctDebugLog("VK Layer: \"%s\" found", layerProps[i].layerName);
   }
   bool found;
   for (uint32_t i = 0; i < validationLayers.Count(); i++) {
      found = false;
      for (uint32_t j = 0; j < layerCount; j++) {
         if (strcmp(validationLayers[i], layerProps[j].layerName)) {
            found = true;
            break;
         }
      }
      if (!found) { return false; }
   }
   return true;
}

void ctVkSwapchainSupport::GetSupport(VkPhysicalDevice gpu, VkSurfaceKHR surface) {
   ZoneScoped;
   vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &surfaceCapabilities);

   uint32_t formatCount;
   vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, NULL);
   if (formatCount) {
      surfaceFormats.Resize(formatCount);
      vkGetPhysicalDeviceSurfaceFormatsKHR(
        gpu, surface, &formatCount, surfaceFormats.Data());
   }

   uint32_t presentModeCount;
   vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &presentModeCount, NULL);
   if (presentModeCount) {
      presentModes.Resize(presentModeCount);
      vkGetPhysicalDeviceSurfacePresentModesKHR(
        gpu, surface, &presentModeCount, presentModes.Data());
   }
}

/* ------------- Device Query ------------- */

bool vIsQueueFamilyComplete(ctVkQueueFamilyIndices indices) {
   return (indices.graphicsIdx != UINT32_MAX && indices.transferIdx != UINT32_MAX &&
           indices.presentIdx != UINT32_MAX && indices.computeIdx != UINT32_MAX);
}

bool ctGPUDevice::DeviceHasRequiredExtensions(VkPhysicalDevice gpu) {
   ZoneScoped;
   uint32_t extCount;
   vkEnumerateDeviceExtensionProperties(gpu, NULL, &extCount, NULL);
   VkExtensionProperties* availible =
     (VkExtensionProperties*)ctMalloc(sizeof(VkExtensionProperties) * extCount);
   vkEnumerateDeviceExtensionProperties(gpu, NULL, &extCount, availible);

   bool foundAll = true;
   bool foundThis;
   for (uint32_t i = 0; i < deviceExtensions.Count(); i++) {
      foundThis = false;
      for (uint32_t ii = 0; ii < extCount; ii++) {
         if (strcmp(deviceExtensions[i], availible[ii].extensionName) == 0) {
            foundThis = true;
            break;
         }
      }
      if (!foundThis) foundAll = false;
   }

   ctFree(availible);
   return foundAll;
}

bool vDeviceHasSwapChainSupport(VkPhysicalDevice gpu, VkSurfaceKHR surface) {
   ZoneScoped;
   ctVkSwapchainSupport support;
   support.GetSupport(gpu, surface);
   return !support.presentModes.isEmpty() && !support.surfaceFormats.isEmpty();
}

bool vDeviceHasRequiredFeatures(
  const VkPhysicalDeviceFeatures& features,
  const VkPhysicalDeviceDescriptorIndexingFeatures& descriptorIndexing) {
   if (!features.shaderFloat64 || !features.depthClamp || !features.depthBounds ||
       !features.shaderSampledImageArrayDynamicIndexing ||
       !features.shaderStorageBufferArrayDynamicIndexing ||
       !descriptorIndexing.descriptorBindingPartiallyBound ||
       !descriptorIndexing.runtimeDescriptorArray ||
       !descriptorIndexing.descriptorBindingSampledImageUpdateAfterBind ||
       !descriptorIndexing.descriptorBindingStorageBufferUpdateAfterBind ||
       !descriptorIndexing.descriptorBindingStorageImageUpdateAfterBind ||
       !descriptorIndexing.shaderSampledImageArrayNonUniformIndexing ||
       !descriptorIndexing.shaderStorageBufferArrayNonUniformIndexing ||
       !descriptorIndexing.shaderStorageImageArrayNonUniformIndexing) {
      return false;
   }
   return true;
}

VkPhysicalDevice ctGPUDevice::PickBestDevice(VkPhysicalDevice* pGpus,
                                             uint32_t count,
                                             VkSurfaceKHR surface) {
   ZoneScoped;
   int64_t bestGPUIdx = -1;
   int64_t bestGPUScore = -1;
   int64_t currentGPUScore;
   for (uint32_t i = 0; i < count; i++) {
      currentGPUScore = 0;
      VkPhysicalDeviceProperties deviceProperties;
      VkPhysicalDeviceFeatures deviceFeatures;

      VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures {
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES};
      VkPhysicalDeviceFeatures2 deviceFeatures2 = {
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
      deviceFeatures2.pNext = &descriptorIndexingFeatures;

      vkGetPhysicalDeviceProperties(pGpus[i], &deviceProperties);
      vkGetPhysicalDeviceFeatures(pGpus[i], &deviceFeatures);
      vkGetPhysicalDeviceFeatures2(pGpus[i], &deviceFeatures2);

      /*Disqualifications*/
      if (!vDeviceHasRequiredFeatures(deviceFeatures, descriptorIndexingFeatures))
         continue; /*Device doesn't meet the minimum features spec*/
      if (!vIsQueueFamilyComplete(FindQueueFamilyIndices(pGpus[i])))
         continue; /*Queue families are incomplete*/
      if (!DeviceHasRequiredExtensions(pGpus[i]))
         continue; /*Doesn't have the required extensions*/
      if (!vDeviceHasSwapChainSupport(pGpus[i], surface))
         continue; /*Doesn't have swap chain support*/

      /*Benifits*/
      if (deviceProperties.deviceType ==
          VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) /* Discrete GPU */
         currentGPUScore += 10000;
      /* Favor well known vendors who have reliable drivers */
      if (deviceProperties.vendorID == 0x10DE || /* NVIDIA */
          deviceProperties.vendorID == 0x1002)   /* AMD */
         currentGPUScore += 5000;

      /*Set as GPU if its the best*/
      if (currentGPUScore > bestGPUScore) {
         bestGPUScore = currentGPUScore;
         bestGPUIdx = i;
      }
   }
   if (bestGPUIdx < 0) return VK_NULL_HANDLE;
   return pGpus[bestGPUIdx];
}

ctVkQueueFamilyIndices ctGPUDevice::FindQueueFamilyIndices(VkPhysicalDevice gpu) {
   ZoneScoped;
   ctVkQueueFamilyIndices result = {UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX};
   uint32_t queueFamilyCount;
   vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, NULL);
   VkQueueFamilyProperties* queueFamilyProps = (VkQueueFamilyProperties*)ctMalloc(
     sizeof(VkQueueFamilyProperties) * queueFamilyCount);
   vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, queueFamilyProps);
   for (uint32_t i = 0; i < queueFamilyCount; i++) {
      if (queueFamilyProps[i].queueCount > 0 &&
          queueFamilyProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
         result.graphicsIdx = i;
      if (queueFamilyProps[i].queueCount > 0 &&
          queueFamilyProps[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
         result.computeIdx = i;
      if (queueFamilyProps[i].queueCount > 0 &&
          queueFamilyProps[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
         result.transferIdx = i;

      VkBool32 present = VK_FALSE;
      vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i, mainScreenResources.surface, &present);
      if (queueFamilyProps[i].queueCount > 0 && present) result.presentIdx = i;

      if (vIsQueueFamilyComplete(result)) break;
   }
   ctFree(queueFamilyProps);
   return result;
}

/* ------------- Screen Resources ------------- */

ctResults ctVkScreenResources::CreateSurface(ctGPUDevice* pDevice, SDL_Window* pWindow) {
   ZoneScoped;
   ctDebugLog("Creating Surface...");
   window = pWindow;
   if (SDL_Vulkan_CreateSurface(pWindow, pDevice->vkInstance, &surface)) {
      return CT_SUCCESS;
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

ctResults ctVkScreenResources::CreateSwapchain(ctGPUDevice* pDevice,
                                               ctVkQueueFamilyIndices indices,
                                               int32_t vsync,
                                               VkSwapchainKHR oldSwapchain) {
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
   presentMode = vPickPresentMode(
     swapChainSupport.presentModes.Data(), swapChainSupport.presentModes.Count(), vsync);

   /* Get Size */
   int w, h;
   SDL_Vulkan_GetDrawableSize(window, &w, &h);
   uint32_t flags = SDL_GetWindowFlags(window);
   extent.width = w;
   extent.height = h;

   /* Get Image Count */
   imageCount = swapChainSupport.surfaceCapabilities.minImageCount + 1;
   if (swapChainSupport.surfaceCapabilities.maxImageCount > 0 &&
       imageCount > swapChainSupport.surfaceCapabilities.maxImageCount) {
      imageCount = swapChainSupport.surfaceCapabilities.maxImageCount;
   }

   /* Create swapchain */
   uint32_t sharedIndices[] {indices.graphicsIdx, indices.presentIdx};
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
   swapChainInfo.oldSwapchain = oldSwapchain;
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

ctResults ctVkScreenResources::CreatePresentResources(ctGPUDevice* pDevice) {
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

ctResults ctVkScreenResources::DestroySwapchain(ctGPUDevice* pDevice) {
   ZoneScoped;
   if (swapchain == VK_NULL_HANDLE) { return CT_FAILURE_NOT_FOUND; }
   vkDestroySwapchainKHR(pDevice->vkDevice, swapchain, &pDevice->vkAllocCallback);
   swapchain = VK_NULL_HANDLE;
   return CT_SUCCESS;
}

ctResults ctVkScreenResources::DestroyPresentResources(ctGPUDevice* pDevice) {
   ZoneScoped;
   for (int i = 0; i < CT_MAX_INFLIGHT_FRAMES; i++) {
      vkDestroySemaphore(pDevice->vkDevice, imageAvailible[i], &pDevice->vkAllocCallback);
   }
   vkDestroyCommandPool(pDevice->vkDevice, blitCommandPool, &pDevice->vkAllocCallback);
   return CT_SUCCESS;
}

bool ctVkScreenResources::HandleResizeIfNeeded(ctGPUDevice* pDevice) {
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
      if (CreateSwapchain(
            pDevice, pDevice->queueFamilyIndices, pDevice->vsync, VK_NULL_HANDLE) !=
          VK_SUCCESS) {
         isMinimized = true;
      }
      resizeTriggered = false;
      return true;
   }
   return false;
}

bool ctVkScreenResources::ShouldSkip() {
   return swapChainSupport.surfaceCapabilities.currentExtent.width == 0 ||
          swapChainSupport.surfaceCapabilities.currentExtent.height == 0;
}

ctResults ctVkScreenResources::DestroySurface(ctGPUDevice* pDevice) {
   ZoneScoped;
   vkDestroySurfaceKHR(pDevice->vkInstance, surface, NULL);
   return CT_SUCCESS;
}

VkResult ctVkScreenResources::BlitAndPresent(ctGPUDevice* pDevice,
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
   uint32_t imageIndex = 0;
   VkResult result = vkAcquireNextImageKHR(pDevice->vkDevice,
                                           swapchain,
                                           pDevice->nextFrameTimeout,
                                           imageAvailible[pDevice->currentFrame],
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
      VkCommandBuffer cmd = blitCommands[pDevice->currentFrame];
      VkCommandBufferBeginInfo beginInfo {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
      vkResetCommandBuffer(cmd, 0);
      vkBeginCommandBuffer(cmd, &beginInfo);

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
      dstToTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED; /* Don't care about previous */
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
      vkEndCommandBuffer(cmd);
      VkSubmitInfo submitInfo {VK_STRUCTURE_TYPE_SUBMIT_INFO};
      VkPipelineStageFlags waitForStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
      submitInfo.commandBufferCount = 1;
      submitInfo.pCommandBuffers = &cmd;
      submitInfo.waitSemaphoreCount = semaphoreCount;
      submitInfo.pWaitSemaphores = pWaitSemaphores;
      submitInfo.pWaitDstStageMask = &waitForStage;
      vkQueueSubmit(
        blitQueue, 1, &submitInfo, pDevice->frameAvailibleFences[pDevice->currentFrame]);
   }

   VkPresentInfoKHR presentInfo {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
   presentInfo.waitSemaphoreCount = 1;
   presentInfo.pWaitSemaphores = &imageAvailible[pDevice->currentFrame];
   presentInfo.swapchainCount = 1;
   presentInfo.pSwapchains = &swapchain;
   presentInfo.pImageIndices = &imageIndex;
   return vkQueuePresentKHR(pDevice->presentQueue, &presentInfo);
}

/* ------------- Init and Shutdown ------------- */

void ctGPUDevice::RecreateSync() {
   VkFenceCreateInfo fenceInfo {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
   fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
   for (int i = 0; i < CT_MAX_INFLIGHT_FRAMES; i++) {
      if (frameAvailibleFences[i] != VK_NULL_HANDLE) {
         vkDestroyFence(vkDevice, frameAvailibleFences[i], &vkAllocCallback);
      }
      vkCreateFence(vkDevice, &fenceInfo, &vkAllocCallback, &frameAvailibleFences[i]);
   }
}

ctResults ctGPUDevice::Startup() {
   ZoneScoped;
   ctDebugLog("Starting Vulkan Backend...");
   /* Fill in AppInfo */
   {
      vkAllocCallback = VkAllocationCallbacks {};
      vkAllocCallback.pUserData = NULL;
      vkAllocCallback.pfnAllocation = (PFN_vkAllocationFunction)vAllocFunction;
      vkAllocCallback.pfnReallocation = (PFN_vkReallocationFunction)vReallocFunction;
      vkAllocCallback.pfnFree = (PFN_vkFreeFunction)vFreeFunction;

      validationLayers.Append("VK_LAYER_KHRONOS_validation");
      validationLayers.Append("VK_LAYER_KHRONOS_synchronization2");
   }
   /* Create Instance */
   {
      ctDebugLog("Getting Extensions...");
      if (validationEnabled && !isValidationLayersAvailible()) {
         ctFatalError(-1,
                      CT_NCT("FAIL:NoValidation",
                             "Vulkan Validation layers requested but not avalible."));
      }

      unsigned int sdlExtCount;
      if (!SDL_Vulkan_GetInstanceExtensions(pMainWindow, &sdlExtCount, NULL)) {
         ctFatalError(-1,
                      CT_NCT("FAIL:SDL_Vulkan_GetInstanceExtensions",
                             "SDL_Vulkan_GetInstanceExtensions() Failed to get "
                             "instance extensions."));
      }
      instanceExtensions.Resize(sdlExtCount);
      SDL_Vulkan_GetInstanceExtensions(NULL, &sdlExtCount, instanceExtensions.Data());

      if (validationEnabled) {
         instanceExtensions.Append(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
      }

      deviceExtensions.Append(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
      deviceExtensions.Append("VK_EXT_descriptor_indexing");
      deviceExtensions.Append("VK_KHR_relaxed_block_layout");
      if (validationEnabled) {
         deviceExtensions.Append(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
      }

      for (uint32_t i = 0; i < instanceExtensions.Count(); i++) {
         ctDebugLog("VK Instance Extension Requested: \"%s\"", instanceExtensions[i]);
      }
      for (uint32_t i = 0; i < deviceExtensions.Count(); i++) {
         ctDebugLog("VK Device Extension Requested: \"%s\"", deviceExtensions[i]);
      }

      VkInstanceCreateInfo instanceInfo = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
      instanceInfo.pApplicationInfo = &vkAppInfo;
      if (validationEnabled) {
         instanceInfo.ppEnabledLayerNames = validationLayers.Data();
         instanceInfo.enabledLayerCount = (uint32_t)validationLayers.Count();
      } else {
         instanceInfo.enabledLayerCount = 0;
         instanceInfo.ppEnabledLayerNames = NULL;
      }

      instanceInfo.enabledExtensionCount = (uint32_t)instanceExtensions.Count();
      instanceInfo.ppEnabledExtensionNames = instanceExtensions.Data();
      ctDebugLog("Creating Instance...");
      CT_VK_CHECK(
        vkCreateInstance(&instanceInfo, &vkAllocCallback, &vkInstance),
        CT_NCT("FAIL:vkCreateInstance",
               "vkCreateInstance() Failed to create vulkan instance.\n"
               "Please update the graphics drivers to the latest version available.\n"
               "If this or other issues persist then upgrade the hardware or contact "
               "support."));
   }
   /*Setup Validation Debug Callback*/
   if (validationEnabled) {
      PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallback =
        (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(
          vkInstance, "vkCreateDebugReportCallbackEXT");

      VkDebugReportCallbackCreateInfoEXT createInfo = {
        VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT};
      createInfo.pfnCallback = (PFN_vkDebugReportCallbackEXT)vDebugCallback;
      createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;

      vkDebugCallback = VK_NULL_HANDLE;
      if (vkCreateDebugReportCallback) {
         vkCreateDebugReportCallback(
           vkInstance, &createInfo, &vkAllocCallback, &vkDebugCallback);
      } else {
         ctFatalError(-1,
                      CT_NCT("FAIL:vkCreateDebugReportCallbackEXT",
                             "Failed to find vkCreateDebugReportCallbackEXT()."));
      }
   }
   /* Initialize a first surface to check for support */
   { mainScreenResources.CreateSurface(this, pMainWindow); }
   /* Pick a GPU */
   {
      ctDebugLog("Finding GPU...");
      uint32_t gpuCount;
      CT_VK_CHECK(vkEnumeratePhysicalDevices(vkInstance, &gpuCount, NULL),
                  CT_NCT("FAIL:vkEnumeratePhysicalDevices",
                         "Failed to find devices with vkEnumeratePhysicalDevices()."));
      if (!gpuCount) {
         ctFatalError(-1,
                      CT_NCT("FAIL:VkNoGPU",
                             "No supported Vulkan compatible rendering device found.\n"
                             "Please upgrade the hardware."));
      }
      ctDynamicArray<VkPhysicalDevice> gpus;
      gpus.Resize(gpuCount);
      vkEnumeratePhysicalDevices(vkInstance, &gpuCount, gpus.Data());
      if (preferredDevice >= 0 && preferredDevice < (int32_t)gpuCount) {
         vkPhysicalDevice = gpus[preferredDevice];

         vDescriptorIndexingFeatures = {
           VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES};
         vPhysicalDeviceFeatures2 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
         vPhysicalDeviceFeatures2.pNext = &vDescriptorIndexingFeatures;

         vkGetPhysicalDeviceProperties(vkPhysicalDevice, &vPhysicalDeviceProperties);
         vkGetPhysicalDeviceFeatures(vkPhysicalDevice, &vPhysicalDeviceFeatures);
         vkGetPhysicalDeviceFeatures2(vkPhysicalDevice, &vPhysicalDeviceFeatures2);

         if (!vDeviceHasRequiredFeatures(vPhysicalDeviceFeatures,
                                         vDescriptorIndexingFeatures) ||
             !DeviceHasRequiredExtensions(vkPhysicalDevice) ||
             !vDeviceHasSwapChainSupport(vkPhysicalDevice, mainScreenResources.surface)) {
            ctFatalError(-1,
                         CT_NCT("FAIL:GPUReqNotMet",
                                "Rendering device does not meet requirements."));
         }
      } else {
         vkPhysicalDevice =
           PickBestDevice(gpus.Data(), gpuCount, mainScreenResources.surface);
         if (vkPhysicalDevice == VK_NULL_HANDLE) {
            ctFatalError(
              -1,
              CT_NCT(
                "FAIL:CouldNotFindGoodGPU",
                "Could not find suitable rendering device.\n"
                "Please update the graphics drivers to the latest version available.\n"
                "If this or other issues persist then upgrade the hardware or contact "
                "support."));
         }

         /* Get Feature Support */
         vDescriptorIndexingFeatures = {
           VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES};
         vPhysicalDeviceFeatures2 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
         vPhysicalDeviceFeatures2.pNext = &vDescriptorIndexingFeatures;

         vkGetPhysicalDeviceProperties(vkPhysicalDevice, &vPhysicalDeviceProperties);
         vkGetPhysicalDeviceFeatures(vkPhysicalDevice, &vPhysicalDeviceFeatures);
         vkGetPhysicalDeviceFeatures2(vkPhysicalDevice, &vPhysicalDeviceFeatures2);
      }
   }
   /* Queues and Device */
   {
      ctDebugLog("Setting Queues...");
      VkDeviceCreateInfo deviceInfo = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
      /* Queue Creation */
      queueFamilyIndices = FindQueueFamilyIndices(vkPhysicalDevice);
      if (!vIsQueueFamilyComplete(queueFamilyIndices)) {
         ctFatalError(-1,
                      CT_NCT("FAIL:VkQueueFamilyIncomplete",
                             "Device doesn't have the necessary queues."));
      }

      uint32_t uniqueIdxCount = 0;
      uint32_t uniqueIndices[4] = {UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX};

      /*ONLY create a list of unique indices*/
      {
         uint32_t nonUniqueIndices[4] = {
           queueFamilyIndices.graphicsIdx,
           queueFamilyIndices.presentIdx,
           queueFamilyIndices.computeIdx,
           queueFamilyIndices.transferIdx,
         };
         bool found;
         for (uint32_t i = 0; i < 4; i++) { /*Add items*/
            found = false;
            for (uint32_t ii = 0; ii < uniqueIdxCount + 1;
                 ii++) {                                       /*Search previous items*/
               if (uniqueIndices[ii] == nonUniqueIndices[i]) { /*Only add if unique*/
                  found = true;
                  break;
               }
            }
            if (!found) {
               uniqueIndices[uniqueIdxCount] = nonUniqueIndices[i];
               uniqueIdxCount++;
            }
         }
      }

      VkDeviceQueueCreateInfo queueCreateInfos[4];
      float defaultPriority = 1.0f;
      for (uint32_t i = 0; i < uniqueIdxCount; i++) {
         queueCreateInfos[i] = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
         queueCreateInfos[i].queueCount = 1;
         queueCreateInfos[i].queueFamilyIndex = uniqueIndices[i];
         queueCreateInfos[i].pQueuePriorities = &defaultPriority;
      }
      deviceInfo.queueCreateInfoCount = uniqueIdxCount;
      deviceInfo.pQueueCreateInfos = queueCreateInfos;

      /* Validation */
      if (validationEnabled) {
         deviceInfo.ppEnabledLayerNames = validationLayers.Data();
         deviceInfo.enabledLayerCount = (uint32_t)validationLayers.Count();
      } else {
         deviceInfo.enabledLayerCount = 0;
         deviceInfo.ppEnabledLayerNames = NULL;
      }

      /* Enable Features */
      VkPhysicalDeviceFeatures features = VkPhysicalDeviceFeatures();
      features.imageCubeArray = VK_TRUE;
      features.shaderFloat64 = VK_TRUE;
      features.depthBounds = VK_TRUE;
      features.depthClamp = VK_TRUE;
      features.samplerAnisotropy = VK_TRUE;
      deviceInfo.pEnabledFeatures = &features;

      VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures = {
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES};
      indexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
      indexingFeatures.runtimeDescriptorArray = VK_TRUE;
      indexingFeatures.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
      indexingFeatures.shaderStorageBufferArrayNonUniformIndexing = VK_TRUE;
      indexingFeatures.shaderStorageImageArrayNonUniformIndexing = VK_TRUE;
      indexingFeatures.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
      indexingFeatures.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;
      indexingFeatures.descriptorBindingStorageImageUpdateAfterBind = VK_TRUE;
      deviceInfo.pNext = &indexingFeatures;

      deviceInfo.enabledExtensionCount = (uint32_t)deviceExtensions.Count();
      deviceInfo.ppEnabledExtensionNames = deviceExtensions.Data();
      ctDebugLog("Creating Device...");
      CT_VK_CHECK(
        vkCreateDevice(vkPhysicalDevice, &deviceInfo, &vkAllocCallback, &vkDevice),
        CT_NCT("FAIL:vkCreateDevice", "vkCreateDevice() failed to create the device."));

      ctDebugLog("Getting Queues...");
      /* Get Queues */
      vkGetDeviceQueue(vkDevice, queueFamilyIndices.graphicsIdx, 0, &graphicsQueue);
      vkGetDeviceQueue(vkDevice, queueFamilyIndices.presentIdx, 0, &presentQueue);
      vkGetDeviceQueue(vkDevice, queueFamilyIndices.computeIdx, 0, &computeQueue);
      vkGetDeviceQueue(vkDevice, queueFamilyIndices.transferIdx, 0, &transferQueue);
   }
   /* Swapchain and Present Resources */
   {
      mainScreenResources.CreatePresentResources(this);
      mainScreenResources.CreateSwapchain(
        this, queueFamilyIndices, vsync, VK_NULL_HANDLE);
   }
   /* Memory Allocator */
   {
      ctDebugLog("Creating Vulkan Memory Allocator...");
      VmaAllocatorCreateInfo allocatorInfo = {};
      /* Current version won't go further than Vulkan 1.1 */
      allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_1;
      allocatorInfo.physicalDevice = vkPhysicalDevice;
      allocatorInfo.device = vkDevice;
      allocatorInfo.instance = vkInstance;

      CT_VK_CHECK(vmaCreateAllocator(&allocatorInfo, &vmaAllocator),
                  CT_NCT("FAIL:vmaCreateAllocator",
                         "vmaCreateAllocator() failed to create allocator."));
   }
   /* Frame Sync */
   { RecreateSync(); }
   /* Pipeline Cache */
   {
      vkPipelineCache = VK_NULL_HANDLE;
      if (fpOpenCacheFileCallback) {
         ctDebugLog("Loading Pipeline Cache...");
         ctFile cacheFile;
         fpOpenCacheFileCallback(&cacheFile,
                                 PIPELINE_CACHE_FILE_PATH,
                                 CT_FILE_OPEN_READ,
                                 pCacheCallbackCustomData);
         size_t cacheSize = 0;
         void* cacheData = NULL;
         if (cacheFile.isOpen()) {
            cacheSize = (size_t)cacheFile.GetFileSize();
            cacheData = ctMalloc(cacheSize);
            cacheFile.ReadRaw(cacheData, cacheSize, 1);
            cacheFile.Close();
         }
         VkPipelineCacheCreateInfo cacheInfo = {
           VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO};
         cacheInfo.pInitialData = cacheData;
         cacheInfo.initialDataSize = cacheSize;
         CT_VK_CHECK(vkCreatePipelineCache(
                       vkDevice, &cacheInfo, &vkAllocCallback, &vkPipelineCache),
                     CT_NCT("FAIL:vkCreatePipelineCache",
                            "vkCreatePipelineCache() failed to create cache."));
         if (cacheData) { ctFree(cacheData); }
      }
   }
   {
      ctDebugLog("Starting Bindless System...");
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
      descriptorsSamplers = ctVkDescriptorManager(maxSamplers);
      /* Sampled Image */
      descriptorSetLayouts[1].binding = GLOBAL_BIND_SAMPLED_IMAGE;
      descriptorSetLayouts[1].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
      descriptorSetLayouts[1].stageFlags = VK_SHADER_STAGE_ALL;
      descriptorSetLayouts[1].descriptorCount = maxSampledImages;
      descriptorPoolSizes[1].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
      descriptorPoolSizes[1].descriptorCount = maxSampledImages;
      bindFlags[1] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
                     VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
      descriptorsSampledImage = ctVkDescriptorManager(maxSampledImages);
      /* Storage Image */
      descriptorSetLayouts[2].binding = GLOBAL_BIND_STORAGE_IMAGE;
      descriptorSetLayouts[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
      descriptorSetLayouts[2].stageFlags = VK_SHADER_STAGE_ALL;
      descriptorSetLayouts[2].descriptorCount = maxStorageImages;
      descriptorPoolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
      descriptorPoolSizes[2].descriptorCount = maxStorageImages;
      bindFlags[2] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
                     VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
      descriptorsStorageImage = ctVkDescriptorManager(maxStorageImages);
      /* Storage Buffer */
      descriptorSetLayouts[3].binding = GLOBAL_BIND_STORAGE_BUFFER;
      descriptorSetLayouts[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
      descriptorSetLayouts[3].stageFlags = VK_SHADER_STAGE_ALL;
      descriptorSetLayouts[3].descriptorCount = maxStorageBuffers;
      descriptorPoolSizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
      descriptorPoolSizes[3].descriptorCount = maxStorageBuffers;
      bindFlags[3] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
                     VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
      descriptorsStorageBuffer = ctVkDescriptorManager(maxStorageBuffers);

      VkDescriptorSetLayoutBindingFlagsCreateInfo bindingExtInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO};
      bindingExtInfo.pBindingFlags = bindFlags;
      bindingExtInfo.bindingCount = ctCStaticArrayLen(bindFlags);

      VkDescriptorSetLayoutCreateInfo descSetLayoutInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
      descSetLayoutInfo.pNext = &bindingExtInfo;
      descSetLayoutInfo.flags =
        VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
      descSetLayoutInfo.bindingCount = ctCStaticArrayLen(descriptorSetLayouts);
      descSetLayoutInfo.pBindings = descriptorSetLayouts;
      CT_VK_CHECK(
        vkCreateDescriptorSetLayout(
          vkDevice, &descSetLayoutInfo, &vkAllocCallback, &vkGlobalDescriptorSetLayout),
        CT_NCT("FAIL:vkCreateDescriptorSetLayout",
               "vkCreateDescriptorSetLayout() failed to create descriptor set layout."));

      VkDescriptorPoolCreateInfo poolInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
      poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT |
                       VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
      poolInfo.poolSizeCount = ctCStaticArrayLen(descriptorPoolSizes);
      poolInfo.pPoolSizes = descriptorPoolSizes;
      poolInfo.maxSets = 1;
      CT_VK_CHECK(
        vkCreateDescriptorPool(vkDevice, &poolInfo, &vkAllocCallback, &vkDescriptorPool),
        CT_NCT("FAIL:vkCreateDescriptorPool",
               "vkCreateDescriptorPool() failed to create descriptor pool."));

      VkDescriptorSetAllocateInfo allocInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
      allocInfo.descriptorSetCount = 1;
      allocInfo.descriptorPool = vkDescriptorPool;
      allocInfo.pSetLayouts = &vkGlobalDescriptorSetLayout;
      CT_VK_CHECK(
        vkAllocateDescriptorSets(vkDevice, &allocInfo, &vkGlobalDescriptorSet),
        CT_NCT("FAIL:vkAllocateDescriptorSets",
               "vkAllocateDescriptorSets() failed to allocate global descriptor set."));

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
        vkCreatePipelineLayout(
          vkDevice, &pipelineLayoutInfo, &vkAllocCallback, &vkGlobalPipelineLayout),
        CT_NCT("FAIL:vkCreatePipelineLayout",
               "vkCreatePipelineLayout() failed to allocate global pipeline layout."));

      /* https://ourmachinery.com/post/moving-the-machinery-to-bindless/
       * https://roar11.com/2019/06/vulkan-textures-unbound/
       * https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VK_EXT_descriptor_indexing.html
       * https://gpuopen.com/performance/#descriptors */
   }
   graphicsCommands.Startup(this, queueFamilyIndices.graphicsIdx);
   computeCommands.Startup(this, queueFamilyIndices.computeIdx);
   transferCommands.Startup(this, queueFamilyIndices.transferIdx);
   ctDebugLog("Vulkan Backend has Started!");
   return CT_SUCCESS;
}

ctResults ctGPUDevice::Shutdown() {
   ZoneScoped;
   ctDebugLog("Vulkan Backend is Shutting Down...");
   vkDeviceWaitIdle(vkDevice);

   graphicsCommands.Shutdown(this);
   computeCommands.Shutdown(this);
   transferCommands.Shutdown(this);
   DestroyAllStagingBuffers();
   vkDestroyPipelineLayout(vkDevice, vkGlobalPipelineLayout, &vkAllocCallback);
   vkDestroyDescriptorPool(vkDevice, vkDescriptorPool, &vkAllocCallback);
   vkDestroyDescriptorSetLayout(vkDevice, vkGlobalDescriptorSetLayout, &vkAllocCallback);

   /* Save Pipeline Cache */
   if (fpOpenCacheFileCallback) {
      size_t cacheSize = 0;
      void* cacheData = NULL;
      vkGetPipelineCacheData(vkDevice, vkPipelineCache, &cacheSize, NULL);
      if (cacheSize) {
         cacheData = ctMalloc(cacheSize);
         vkGetPipelineCacheData(vkDevice, vkPipelineCache, &cacheSize, cacheData);
         ctFile cacheFile;
         fpOpenCacheFileCallback(&cacheFile,
                                 PIPELINE_CACHE_FILE_PATH,
                                 CT_FILE_OPEN_WRITE,
                                 pCacheCallbackCustomData);
         if (cacheFile.isOpen()) {
            cacheFile.WriteRaw(cacheData, cacheSize, 1);
            cacheFile.Close();
         }
         ctFree(cacheData);
      }
   }
   if (vkPipelineCache) {
      vkDestroyPipelineCache(vkDevice, vkPipelineCache, &vkAllocCallback);
   }

   for (int i = 0; i < CT_MAX_INFLIGHT_FRAMES; i++) {
      vkDestroyFence(vkDevice, frameAvailibleFences[i], &vkAllocCallback);
   }

   mainScreenResources.DestroySwapchain(this);
   mainScreenResources.DestroySurface(this);
   mainScreenResources.DestroyPresentResources(this);
   vmaDestroyAllocator(vmaAllocator);
   vkDestroyDevice(vkDevice, &vkAllocCallback);

   if (vkDebugCallback != VK_NULL_HANDLE) {
      PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallback =
        (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(
          vkInstance, "vkDestroyDebugReportCallbackEXT");
      vkDestroyDebugReportCallback(vkInstance, vkDebugCallback, &vkAllocCallback);
   }

   vkDestroyInstance(vkInstance, &vkAllocCallback);
   return CT_SUCCESS;
}

VkResult ctGPUDevice::CreateCompleteImage(ctVkCompleteImage& fullImage,
                                          VkFormat format,
                                          VkImageUsageFlags usage,
                                          VmaAllocationCreateFlags allocFlags,
                                          VkImageAspectFlags aspect,
                                          uint32_t width,
                                          uint32_t height,
                                          uint32_t depth,
                                          uint32_t mip,
                                          uint32_t layers,
                                          VkSampleCountFlagBits samples,
                                          VkImageType imageType,
                                          VkImageViewType viewType,
                                          VkImageTiling tiling,
                                          VkImageLayout initialLayout,
                                          VmaMemoryUsage memUsage,
                                          int32_t imageFlags,
                                          VkSharingMode sharing,
                                          uint32_t queueFamilyIndexCount,
                                          uint32_t* pQueueFamilyIndices) {
   ZoneScoped;
   VkImageCreateInfo imageInfo {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
   imageInfo.format = format;
   imageInfo.usage = usage;
   imageInfo.extent.width = width;
   imageInfo.extent.height = height;
   imageInfo.extent.depth = depth;
   imageInfo.mipLevels = mip;
   imageInfo.samples = samples;
   imageInfo.imageType = imageType;
   imageInfo.sharingMode = sharing;
   imageInfo.tiling = tiling;
   imageInfo.initialLayout = initialLayout;
   imageInfo.flags = imageFlags;
   imageInfo.arrayLayers = layers;
   imageInfo.queueFamilyIndexCount = queueFamilyIndexCount;
   imageInfo.pQueueFamilyIndices = pQueueFamilyIndices;

   VmaAllocationCreateInfo allocInfo {};
   allocInfo.flags = allocFlags;
   allocInfo.usage = memUsage;

   VkResult result = vmaCreateImage(
     vmaAllocator, &imageInfo, &allocInfo, &fullImage.image, &fullImage.alloc, NULL);
   if (result != VK_SUCCESS) { return result; }

   VkImageViewCreateInfo viewInfo {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
   viewInfo.image = fullImage.image;
   viewInfo.viewType = viewType;
   viewInfo.format = format;
   viewInfo.subresourceRange.aspectMask = aspect;
   viewInfo.subresourceRange.baseArrayLayer = 0;
   viewInfo.subresourceRange.layerCount = layers;
   viewInfo.subresourceRange.baseMipLevel = 0;
   viewInfo.subresourceRange.levelCount = mip;
   return vkCreateImageView(vkDevice, &viewInfo, &vkAllocCallback, &fullImage.view);
}

VkResult ctGPUDevice::CreateCompleteBuffer(ctVkCompleteBuffer& fullBuffer,
                                           VkBufferUsageFlags usage,
                                           VmaAllocationCreateFlags allocFlags,
                                           size_t size,
                                           VmaMemoryUsage memUsage,
                                           int32_t bufferFlags,
                                           VkSharingMode sharing,
                                           uint32_t queueFamilyIndexCount,
                                           uint32_t* pQueueFamilyIndices) {
   ZoneScoped;
   VkBufferCreateInfo bufferInfo {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
   bufferInfo.usage = usage;
   bufferInfo.size = size;
   bufferInfo.sharingMode = sharing;
   bufferInfo.queueFamilyIndexCount = queueFamilyIndexCount;
   bufferInfo.pQueueFamilyIndices = pQueueFamilyIndices;

   VmaAllocationCreateInfo allocInfo {};
   allocInfo.flags = allocFlags;
   allocInfo.usage = memUsage;
   return vmaCreateBuffer(
     vmaAllocator, &bufferInfo, &allocInfo, &fullBuffer.buffer, &fullBuffer.alloc, NULL);
}

void ctGPUDevice::TryDestroyCompleteImage(ctVkCompleteImage& fullImage) {
   ZoneScoped;
   if (fullImage.view == VK_NULL_HANDLE) { return; }
   vkDestroyImageView(vkDevice, fullImage.view, &vkAllocCallback);
   if (fullImage.image == VK_NULL_HANDLE || fullImage.alloc == VK_NULL_HANDLE) { return; }
   vmaDestroyImage(vmaAllocator, fullImage.image, fullImage.alloc);
}

void ctGPUDevice::TryDestroyCompleteBuffer(ctVkCompleteBuffer& fullBuffer) {
   ZoneScoped;
   if (fullBuffer.buffer == VK_NULL_HANDLE || fullBuffer.alloc == VK_NULL_HANDLE) {
      return;
   }
   vmaDestroyBuffer(vmaAllocator, fullBuffer.buffer, fullBuffer.alloc);
}

ctResults ctGPUDevice::GetStagingBuffer(ctVkCompleteBuffer& fullBuffer,
                                        size_t& offset,
                                        uint8_t*& mapping,
                                        size_t sizeRequest) {
   /* Find existing */
   const size_t count = stagingBufferCooldown.Count();
   for (size_t i = 0; i < count; i++) {
      if (stagingBufferCooldown[i] == 0) {
         if (stagingBuffers[i].size >= sizeRequest) {
            offset = 0;
            fullBuffer = stagingBuffers[i].buffer;
            mapping = stagingBuffers[i].mapping;
            stagingBufferCooldown[i] = -1;
            return CT_SUCCESS;
         }
      }
   }

   /* Create new */
   StagingEntry entry = {};
   entry.size = sizeRequest;
   ctAssert(CreateCompleteBuffer(entry.buffer,
                                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                 VMA_ALLOCATION_CREATE_STRATEGY_MIN_FRAGMENTATION_BIT,
                                 sizeRequest,
                                 VMA_MEMORY_USAGE_CPU_TO_GPU) == VK_SUCCESS);
   vmaMapMemory(vmaAllocator, entry.buffer.alloc, (void**)&entry.mapping);
   stagingBufferCooldown.Append(-1);
   stagingBuffers.Append(entry);
   offset = 0;
   mapping = entry.mapping;
   fullBuffer = entry.buffer;
   return CT_SUCCESS;
}

void ctGPUDevice::AdvanceStagingCooldownTimers() {
   const size_t count = stagingBufferCooldown.Count();
   for (size_t i = 0; i < count; i++) {
      if (stagingBufferCooldown[i] > 0) {
         stagingBufferCooldown[i]--;
      } else if (stagingBufferCooldown[i] < 0) {
         stagingBufferCooldown[i] = CT_MAX_CONVEYOR_FRAMES;
      }
   }
}

void ctGPUDevice::DestroyAllStagingBuffers() {
   for (size_t i = 0; i < stagingBuffers.Count(); i++) {
      vmaUnmapMemory(vmaAllocator, stagingBuffers[i].buffer.alloc);
      TryDestroyCompleteBuffer(stagingBuffers[i].buffer);
   }
   stagingBufferCooldown.Clear();
   stagingBuffers.Clear();
}

ctFile ctGPUDevice::OpenAssetFile(ctGPUAssetIdentifier* pAssetIdentifier) {
   ctFile result = ctFile();
   fpOpenAssetFileCallback(&result, pAssetIdentifier, pAssetCallbackCustomData);
   return result;
}

void ctGPUDevice::CommandBufferManager::Startup(ctGPUDevice* pDevice,
                                                uint32_t queueFamilyIdx) {
   VkCommandPoolCreateInfo poolInfo = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
   poolInfo.queueFamilyIndex = queueFamilyIdx;
   poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
   vkCreateCommandPool(pDevice->vkDevice, &poolInfo, &pDevice->vkAllocCallback, &pool);
   VkCommandBufferAllocateInfo allocInfo = {
     VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
   allocInfo.commandBufferCount = CT_MAX_INFLIGHT_FRAMES;
   allocInfo.commandPool = pool;
   allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
   vkAllocateCommandBuffers(pDevice->vkDevice, &allocInfo, cmd);
   BeginFrame(pDevice);
}

void ctGPUDevice::CommandBufferManager::Shutdown(ctGPUDevice* pDevice) {
   vkFreeCommandBuffers(pDevice->vkDevice, pool, CT_MAX_INFLIGHT_FRAMES, cmd);
   vkDestroyCommandPool(pDevice->vkDevice, pool, &pDevice->vkAllocCallback);
}

void ctGPUDevice::CommandBufferManager::BeginFrame(ctGPUDevice* pDevice) {
   VkCommandBufferBeginInfo info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
   vkBeginCommandBuffer(cmd[pDevice->currentFrame], &info);
}

void ctGPUDevice::CommandBufferManager::Submit(ctGPUDevice* pDevice, VkQueue queue) {
   VkSubmitInfo info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
   info.commandBufferCount = 1;
   info.pCommandBuffers = &cmd[pDevice->currentFrame];
   // todo: semaphores and fences
   vkQueueSubmit(queue, 1, &info, VK_NULL_HANDLE);
}

/* Bindless */
void ctGPUDevice::ExposeBindlessStorageBuffer(uint32_t& outIdx,
                                              VkBuffer buffer,
                                              VkDeviceSize range,
                                              VkDeviceSize offset) {
   ZoneScoped;
   outIdx = descriptorsStorageBuffer.AllocateSlot();
   VkDescriptorBufferInfo buffInfo = {0};
   buffInfo.buffer = buffer;
   buffInfo.offset = offset;
   buffInfo.range = range;
   VkWriteDescriptorSet write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
   write.descriptorCount = 1;
   write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
   write.dstSet = vkGlobalDescriptorSet;
   write.dstBinding = GLOBAL_BIND_STORAGE_BUFFER;
   write.dstArrayElement = outIdx;
   write.pBufferInfo = &buffInfo;
   vkUpdateDescriptorSets(vkDevice, 1, &write, 0, NULL);
}

void ctGPUDevice::ReleaseBindlessStorageBuffer(uint32_t idx) {
   descriptorsStorageBuffer.ReleaseSlot(idx);
}

void ctGPUDevice::ExposeBindlessSampledImage(uint32_t& outIdx,
                                             VkImageView view,
                                             VkImageLayout layout,
                                             VkSampler sampler) {
   outIdx = descriptorsSampledImage.AllocateSlot();
   VkDescriptorImageInfo imageInfo = {0};
   imageInfo.imageView = view;
   imageInfo.imageLayout = layout;
   imageInfo.sampler = sampler;
   VkWriteDescriptorSet write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
   write.descriptorCount = 1;
   write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
   write.dstSet = vkGlobalDescriptorSet;
   write.dstBinding = GLOBAL_BIND_SAMPLED_IMAGE;
   write.dstArrayElement = outIdx;
   write.pImageInfo = &imageInfo;
   vkUpdateDescriptorSets(vkDevice, 1, &write, 0, NULL);
}

void ctGPUDevice::ReleaseBindlessSampledImage(uint32_t idx) {
   descriptorsSampledImage.ReleaseSlot(idx);
}