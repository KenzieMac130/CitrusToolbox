#include "VkBackend.hpp"
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

#include "VkBackend.hpp"
#include "core/Application.hpp"

#include "tracy/Tracy.hpp"

#ifdef _WIN32
#include <Windows.h>
#endif
#define VMA_IMPLEMENTATION
#include "vma/vk_mem_alloc.h"

#define PIPELINE_CACHE_FILE_PATH "VK_PIPELINE_CACHE"

const char* deviceReqExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                                     "VK_EXT_descriptor_indexing"};

/* ------------- Debug Callback ------------- */

VKAPI_ATTR VkBool32 VKAPI_CALL vDebugCallback(VkDebugReportFlagsEXT flags,
                                              VkDebugReportObjectTypeEXT objType,
                                              uint64_t srcObject,
                                              size_t location,
                                              int32_t msgCode,
                                              const char* pLayerPrefix,
                                              const char* pMsg,
                                              void* pUserData) {
   ctDebugError("VK Validation Layer: [%s] Code %u : %s", pLayerPrefix, msgCode, pMsg);
   return VK_FALSE;
}

/* ------------- Helpers ------------- */

bool ctVkBackend::isValidationLayersAvailible() {
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

bool vIsQueueFamilyComplete(ctVkQueueFamilyIndices indices) {
   return (indices.graphicsIdx != UINT32_MAX && indices.transferIdx != UINT32_MAX &&
           indices.presentIdx != UINT32_MAX && indices.computeIdx != UINT32_MAX);
}

ctVkQueueFamilyIndices ctVkBackend::FindQueueFamilyIndices(VkPhysicalDevice gpu) {
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

VkResult ctVkBackend::CreateCompleteImage(ctVkCompleteImage& fullImage,
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

VkResult ctVkBackend::CreateCompleteBuffer(ctVkCompleteBuffer& fullBuffer,
                                           VkImageUsageFlags usage,
                                           VmaAllocationCreateFlags allocFlags,
                                           size_t size,
                                           VmaMemoryUsage memUsage,
                                           int32_t bufferFlags,
                                           VkSharingMode sharing,
                                           uint32_t queueFamilyIndexCount,
                                           uint32_t* pQueueFamilyIndices) {
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

void ctVkBackend::TryDestroyCompleteImage(ctVkCompleteImage& fullImage) {
   if (fullImage.view == VK_NULL_HANDLE) { return; }
   vkDestroyImageView(vkDevice, fullImage.view, &vkAllocCallback);
   if (fullImage.image == VK_NULL_HANDLE || fullImage.alloc == VK_NULL_HANDLE) { return; }
   vmaDestroyImage(vmaAllocator, fullImage.image, fullImage.alloc);
}

void ctVkBackend::TryDestroyCompleteBuffer(ctVkCompleteBuffer& fullBuffer) {
    if (fullBuffer.buffer == VK_NULL_HANDLE || fullBuffer.alloc == VK_NULL_HANDLE) { return; }
    vmaDestroyBuffer(vmaAllocator, fullBuffer.buffer, fullBuffer.alloc);
}

void ctVkSwapchainSupport::GetSupport(VkPhysicalDevice gpu, VkSurfaceKHR surface) {
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

bool vDeviceHasRequiredExtensions(VkPhysicalDevice gpu) {
   uint32_t extCount;
   vkEnumerateDeviceExtensionProperties(gpu, NULL, &extCount, NULL);
   VkExtensionProperties* availible =
     (VkExtensionProperties*)ctMalloc(sizeof(VkExtensionProperties) * extCount);
   vkEnumerateDeviceExtensionProperties(gpu, NULL, &extCount, availible);

   bool foundAll = true;
   bool foundThis;
   for (uint32_t i = 0; i < ctCStaticArrayLen(deviceReqExtensions); i++) {
      foundThis = false;
      for (uint32_t ii = 0; ii < extCount; ii++) {
         if (strcmp(deviceReqExtensions[i], availible[ii].extensionName) == 0) {
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
   ctVkSwapchainSupport support;
   support.GetSupport(gpu, surface);
   return !support.presentModes.isEmpty() && !support.surfaceFormats.isEmpty();
}

bool vDeviceHasRequiredFeatures(
  const VkPhysicalDeviceFeatures& features,
  const VkPhysicalDeviceDescriptorIndexingFeatures& descriptorIndexing) {
   if (!features.shaderFloat64 || !features.fillModeNonSolid || !features.depthClamp ||
       !features.depthBounds || !features.wideLines ||
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

VkPhysicalDevice ctVkBackend::PickBestDevice(VkPhysicalDevice* pGpus,
                                             uint32_t count,
                                             VkSurfaceKHR surface) {
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
      if (!vDeviceHasRequiredExtensions(pGpus[i]))
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

/* ------------- Alloc Functions ------------- */

void* vAllocFunction(void* pUserData,
                     size_t size,
                     size_t alignment,
                     VkSystemAllocationScope allocationScope) {
   return ctAlignedMalloc(size, alignment);
}

void* vReallocFunction(void* pUserData,
                       void* pOriginal,
                       size_t size,
                       size_t alignment,
                       VkSystemAllocationScope allocationScope) {
   return ctAlignedRealloc(pOriginal, size, alignment);
}

void vFreeFunction(void* pUserData, void* pMemory) {
   ctAlignedFree(pMemory);
}

/* ------------- Backend Core ------------- */

ctResults ctVkBackend::Startup() {
   ZoneScoped;
   if (!Engine->WindowManager->isStarted()) { return CT_FAILURE_DEPENDENCY_NOT_MET; }
   if (!Engine->OSEventManager->isStarted()) { return CT_FAILURE_DEPENDENCY_NOT_MET; }

   ctDebugLog("Starting Vulkan Backend...");
   /* Fill in AppInfo */
   {
      vkAppInfo = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
      vkAppInfo.apiVersion = VK_VERSION_1_2;
      vkAppInfo.pEngineName = "CitrusToolbox";
      vkAppInfo.engineVersion = VK_MAKE_VERSION(CITRUS_ENGINE_VERSION_MAJOR,
                                                CITRUS_ENGINE_VERSION_MINOR,
                                                CITRUS_ENGINE_VERSION_PATCH);
      vkAppInfo.pApplicationName = Engine->App->GetAppName();
      const ctAppVersion appVersion = Engine->App->GetAppVersion();
      ;
      vkAppInfo.applicationVersion =
        VK_MAKE_VERSION(appVersion.major, appVersion.minor, appVersion.patch);

      vkAllocCallback = VkAllocationCallbacks {};
      vkAllocCallback.pUserData = NULL;
      vkAllocCallback.pfnAllocation = vAllocFunction;
      vkAllocCallback.pfnReallocation = vReallocFunction;
      vkAllocCallback.pfnFree = vFreeFunction;

      validationLayers.Append("VK_LAYER_KHRONOS_validation");
   }
   /* Create Instance */
   {
      ctDebugLog("Getting Extensions...");
      if (validationEnabled && !isValidationLayersAvailible()) {
         ctFatalError(-1, CT_NC("Vulkan Validation layers requested but not avalible."));
      }

      unsigned int sdlExtCount;
      unsigned int extraExtCount = 1;
      if (!SDL_Vulkan_GetInstanceExtensions(
            Engine->WindowManager->mainWindow.pSDLWindow, &sdlExtCount, NULL)) {
         ctFatalError(-1,
                      CT_NC("SDL_Vulkan_GetInstanceExtensions() Failed to get "
                            "instance extensions."));
      }
      instanceExtensions.Resize(sdlExtCount + extraExtCount);
      instanceExtensions[0] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
      SDL_Vulkan_GetInstanceExtensions(
        NULL, &sdlExtCount, &instanceExtensions[extraExtCount]);
      for (uint32_t i = 0; i < sdlExtCount + extraExtCount; i++) {
         ctDebugLog("VK Extension: \"%s\" found", instanceExtensions[i]);
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
      CT_VK_CHECK(vkCreateInstance(&instanceInfo, &vkAllocCallback, &vkInstance),
                  CT_NC("vkCreateInstance() Failed to create vulkan instance."));
   }
   /*Setup Validation Debug Callback*/
   {
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
         ctFatalError(-1, CT_NC("Failed to find vkCreateDebugReportCallbackEXT()."));
      }
   }
   /* Initialize a first surface to check for support */
   {
      mainScreenResources.CreateSurface(this,
                                        Engine->WindowManager->mainWindow.pSDLWindow);
   } /* Pick a GPU */
   {
      ctDebugLog("Finding GPU...");
      uint32_t gpuCount;
      CT_VK_CHECK(vkEnumeratePhysicalDevices(vkInstance, &gpuCount, NULL),
                  CT_NC("Failed to find devices with vkEnumeratePhysicalDevices()."));
      if (!gpuCount) {
         ctFatalError(-1, CT_NC("No supported Vulkan compatible GPU found."));
      }
      ctDynamicArray<VkPhysicalDevice> gpus;
      gpus.Resize(gpuCount);
      vkEnumeratePhysicalDevices(vkInstance, &gpuCount, gpus.Data());
      if (preferredDevice >= 0 && preferredDevice < (int32_t)gpuCount) {
         vkPhysicalDevice = gpus[preferredDevice];

         VkPhysicalDeviceFeatures deviceFeatures;
         VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures {
           VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES};
         VkPhysicalDeviceFeatures2 deviceFeatures2 = {
           VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
         deviceFeatures2.pNext = &descriptorIndexingFeatures;

         vkGetPhysicalDeviceFeatures(vkPhysicalDevice, &deviceFeatures);
         vkGetPhysicalDeviceFeatures2(vkPhysicalDevice, &deviceFeatures2);

         if (!vDeviceHasRequiredFeatures(deviceFeatures, descriptorIndexingFeatures) ||
             !vDeviceHasRequiredExtensions(vkPhysicalDevice) ||
             !vDeviceHasSwapChainSupport(vkPhysicalDevice, mainScreenResources.surface)) {
            ctFatalError(-1, CT_NC("Graphics card doesn't meet requirements."));
         }
      } else {
         vkPhysicalDevice =
           PickBestDevice(gpus.Data(), gpuCount, mainScreenResources.surface);
         if (vkPhysicalDevice == VK_NULL_HANDLE) {
            ctFatalError(-1, CT_NC("Could not find suitable graphics card."));
         }
      }
   }
   /* Queues and Device */
   {
      ctDebugLog("Setting Queues...");
      VkDeviceCreateInfo deviceInfo = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
      /* Queue Creation */
      queueFamilyIndices = FindQueueFamilyIndices(vkPhysicalDevice);
      if (!vIsQueueFamilyComplete(queueFamilyIndices)) {
         ctFatalError(-1, CT_NC("Device doesn't have the necessary queues."));
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
      features.fillModeNonSolid = VK_TRUE;
      features.depthBounds = VK_TRUE;
      features.depthClamp = VK_TRUE;
      features.wideLines = VK_TRUE;
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

      deviceInfo.enabledExtensionCount = ctCStaticArrayLen(deviceReqExtensions);
      deviceInfo.ppEnabledExtensionNames = deviceReqExtensions;
      ctDebugLog("Creating Device...");
      CT_VK_CHECK(
        vkCreateDevice(vkPhysicalDevice, &deviceInfo, &vkAllocCallback, &vkDevice),
        CT_NC("vkCreateDevice() failed to create the device."));

      ctDebugLog("Getting Queues...");
      /* Get Queues */
      vkGetDeviceQueue(vkDevice, queueFamilyIndices.graphicsIdx, 0, &graphicsQueue);
      vkGetDeviceQueue(vkDevice, queueFamilyIndices.presentIdx, 0, &presentQueue);
      vkGetDeviceQueue(vkDevice, queueFamilyIndices.computeIdx, 0, &computeQueue);
      vkGetDeviceQueue(vkDevice, queueFamilyIndices.transferIdx, 0, &transferQueue);
   }
   /* Swapchain */
   {
      mainScreenResources.CreateSwapchain(
        this, queueFamilyIndices, vsync, VK_NULL_HANDLE);
   }
   /* Command Buffer Manager */
   {
      for (int i = 0; i < CT_MAX_INFLIGHT_FRAMES; i++) {
         graphicsCommands[i].Create(
           this, maxGraphicsCommandBuffers, queueFamilyIndices.graphicsIdx);
         computeCommands[i].Create(
           this, maxComputeCommandBuffers, queueFamilyIndices.computeIdx);
         transferCommands[i].Create(
           this, maxTransferCommandBuffers, queueFamilyIndices.transferIdx);
      }
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
                  CT_NC("vmaCreateAllocator() failed to create allocator."));
   }
   /* Pipeline Factory */
   {
      ctDebugLog("Loading Pipeline Cache...");
      ctFile cacheFile;
      Engine->FileSystem->OpenPreferencesFile(
        cacheFile, PIPELINE_CACHE_FILE_PATH, CT_FILE_OPEN_READ);
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
      CT_VK_CHECK(
        vkCreatePipelineCache(vkDevice, &cacheInfo, &vkAllocCallback, &vkPipelineCache),
        CT_NC("vkCreatePipelineCache() failed to create cache."));
      if (cacheData) { ctFree(cacheData); }

      // Todo: (maybe later inside of key lime) :)
      // https://gpuopen.com/performance/#pso
      // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#pipelines-cache
      // Compile uber-shaders
   }
   /* Bindless System */
   {
      ctDebugLog("Starting Bindless System...");
      VkDescriptorSetLayoutBinding descriptorSetLayouts[4] = {};
      VkDescriptorPoolSize descriptorPoolSizes[4] = {};
      VkDescriptorBindingFlags bindFlags[4] = {};
      /* Sampler */
      descriptorSetLayouts[0].binding = 0;
      descriptorSetLayouts[0].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
      descriptorSetLayouts[0].stageFlags = VK_SHADER_STAGE_ALL;
      descriptorSetLayouts[0].descriptorCount = maxSamplers;
      descriptorPoolSizes[0].type = VK_DESCRIPTOR_TYPE_SAMPLER;
      descriptorPoolSizes[0].descriptorCount = maxSamplers;
      bindFlags[0] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
      descriptorsSamplers = ctVkDescriptorManager(maxSamplers);
      /* Sampled Image */
      descriptorSetLayouts[1].binding = 1;
      descriptorSetLayouts[1].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
      descriptorSetLayouts[1].stageFlags = VK_SHADER_STAGE_ALL;
      descriptorSetLayouts[1].descriptorCount = maxSampledImages;
      descriptorPoolSizes[1].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
      descriptorPoolSizes[1].descriptorCount = maxSampledImages;
      bindFlags[1] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
                     VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
      descriptorsSampledImage = ctVkDescriptorManager(maxSampledImages);
      /* Storage Image */
      descriptorSetLayouts[2].binding = 2;
      descriptorSetLayouts[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
      descriptorSetLayouts[2].stageFlags = VK_SHADER_STAGE_ALL;
      descriptorSetLayouts[2].descriptorCount = maxStorageImages;
      descriptorPoolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
      descriptorPoolSizes[2].descriptorCount = maxStorageImages;
      bindFlags[2] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
                     VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
      descriptorsStorageImage = ctVkDescriptorManager(maxStorageImages);
      /* Storage Buffer */
      descriptorSetLayouts[3].binding = 3;
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
          vkDevice, &descSetLayoutInfo, &vkAllocCallback, &vkDescriptorSetLayout),
        CT_NC("vkCreateDescriptorSetLayout() failed to create descriptor set layout."));

      VkDescriptorPoolCreateInfo poolInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
      poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT |
                       VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
      poolInfo.poolSizeCount = ctCStaticArrayLen(descriptorPoolSizes);
      poolInfo.pPoolSizes = descriptorPoolSizes;
      poolInfo.maxSets = 1;
      CT_VK_CHECK(
        vkCreateDescriptorPool(vkDevice, &poolInfo, &vkAllocCallback, &vkDescriptorPool),
        CT_NC("vkCreateDescriptorPool() failed to create descriptor pool."));

      VkDescriptorSetAllocateInfo allocInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
      allocInfo.descriptorSetCount = 1;
      allocInfo.descriptorPool = vkDescriptorPool;
      allocInfo.pSetLayouts = &vkDescriptorSetLayout;
      CT_VK_CHECK(
        vkAllocateDescriptorSets(vkDevice, &allocInfo, &vkGlobalDescriptorSet),
        CT_NC("vkAllocateDescriptorSets() failed to allocate global descriptor set."));

      // https://ourmachinery.com/post/moving-the-machinery-to-bindless/
      // https://roar11.com/2019/06/vulkan-textures-unbound/
      // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VK_EXT_descriptor_indexing.html
      // https://gpuopen.com/performance/#descriptors
   }
   /* Make Window Visible */
   {
      ctDebugLog("Showing Window...");
      Engine->WindowManager->ShowMainWindow();
   }

   ctDebugLog("Vulkan Backend has Started!");
   return CT_SUCCESS;
}

ctResults ctVkBackend::Shutdown() {
   ctDebugLog("Vulkan Backend is Shutting Down...");
   vkDeviceWaitIdle(vkDevice);

   /* Save Pipeline Cache */
   {
      size_t cacheSize = 0;
      void* cacheData = NULL;
      vkGetPipelineCacheData(vkDevice, vkPipelineCache, &cacheSize, NULL);
      if (cacheSize) {
         cacheData = ctMalloc(cacheSize);
         vkGetPipelineCacheData(vkDevice, vkPipelineCache, &cacheSize, cacheData);
         ctFile cacheFile;
         Engine->FileSystem->OpenPreferencesFile(
           cacheFile, PIPELINE_CACHE_FILE_PATH, CT_FILE_OPEN_WRITE);
         if (cacheFile.isOpen()) {
            cacheFile.WriteRaw(cacheData, cacheSize, 1);
            cacheFile.Close();
         }
         ctFree(cacheData);
      }
      vkDestroyPipelineCache(vkDevice, vkPipelineCache, &vkAllocCallback);
   }

   for (int i = 0; i < CT_MAX_INFLIGHT_FRAMES; i++) {
      graphicsCommands[i].Destroy(this);
      computeCommands[i].Destroy(this);
      transferCommands[i].Destroy(this);
   }

   vkDestroyDescriptorPool(vkDevice, vkDescriptorPool, &vkAllocCallback);
   vkDestroyDescriptorSetLayout(vkDevice, vkDescriptorSetLayout, &vkAllocCallback);
   mainScreenResources.DestroySwapchain(this);
   mainScreenResources.DestroySurface(this);
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

/* ------------- Screen Resources ------------- */

ctResults ctVkScreenResources::CreateSurface(ctVkBackend* pBackend, SDL_Window* pWindow) {
   ctDebugLog("Creating Surface...");
   window = pWindow;
   if (SDL_Vulkan_CreateSurface(pWindow, pBackend->vkInstance, &surface)) {
      return CT_SUCCESS;
   }
   return CT_FAILURE_UNKNOWN;
}

VkSurfaceFormatKHR vPickBestSurfaceFormat(VkSurfaceFormatKHR* formats, size_t count) {
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

ctResults ctVkScreenResources::CreateSwapchain(ctVkBackend* pBackend,
                                               ctVkQueueFamilyIndices indices,
                                               int32_t vsync,
                                               VkSwapchainKHR oldSwapchain) {
   ctDebugLog("Creating Swapchain...");

   swapChainSupport.GetSupport(pBackend->vkPhysicalDevice, surface);

   /* Select a Image and Color Format */
   surfaceFormat = vPickBestSurfaceFormat(swapChainSupport.surfaceFormats.Data(),
                                          swapChainSupport.surfaceFormats.Count());

   /* Select a Present Mode */
   presentMode = vPickPresentMode(
     swapChainSupport.presentModes.Data(), swapChainSupport.presentModes.Count(), vsync);

   /* Get Size */
   int w, h;
   SDL_Vulkan_GetDrawableSize(window, &w, &h);
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
   swapChainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
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
       pBackend->vkDevice, &swapChainInfo, &pBackend->vkAllocCallback, &swapchain),
     CT_NC("vkCreateSwapchainKHR() failed to create swapchain."));

   /* Get images */
   vkGetSwapchainImagesKHR(pBackend->vkDevice, swapchain, &imageCount, NULL);
   swapImages.Resize(imageCount);
   swapImageViews.Resize(imageCount);
   vkGetSwapchainImagesKHR(pBackend->vkDevice, swapchain, &imageCount, swapImages.Data());

   /* Create image views */
   for (uint32_t i = 0; i < imageCount; i++) {
      VkImageViewCreateInfo viewInfo = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
      viewInfo.image = swapImages[i];
      viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
      viewInfo.format = surfaceFormat.format;
      viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
      viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
      viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
      viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
      viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      viewInfo.subresourceRange.baseMipLevel = 0;
      viewInfo.subresourceRange.levelCount = 1;
      viewInfo.subresourceRange.baseArrayLayer = 0;
      viewInfo.subresourceRange.layerCount = 1;
      CT_VK_CHECK(
        vkCreateImageView(
          pBackend->vkDevice, &viewInfo, &pBackend->vkAllocCallback, &swapImageViews[i]),
        CT_NC("vkCreateImageView() failed to create swapchain image view."));
   }
   return CT_SUCCESS;
}

ctResults ctVkScreenResources::DestroySwapchain(ctVkBackend* pBackend) {
   for (uint32_t i = 0; i < imageCount; i++) {
      vkDestroyImageView(
        pBackend->vkDevice, swapImageViews[i], &pBackend->vkAllocCallback);
   }
   vkDestroySwapchainKHR(pBackend->vkDevice, swapchain, &pBackend->vkAllocCallback);
   return CT_SUCCESS;
}

ctResults ctVkScreenResources::DestroySurface(ctVkBackend* pBackend) {
   vkDestroySurfaceKHR(pBackend->vkInstance, surface, NULL);
   return CT_SUCCESS;
}

/* ------------- Descriptor Manager ------------- */

ctVkDescriptorManager::ctVkDescriptorManager() {
   nextNewIdx = 0;
   _max = 0;
}

ctVkDescriptorManager::ctVkDescriptorManager(int32_t max) {
   nextNewIdx = 0;
   _max = max;
}

int32_t ctVkDescriptorManager::AllocateSlot() {
   int32_t result = nextNewIdx;
   if (!freedIdx.isEmpty()) {
      result = freedIdx.Last();
      freedIdx.RemoveLast();
   }
   return result;
}

void ctVkDescriptorManager::ReleaseSlot(const int32_t idx) {
   freedIdx.Append(idx);
}

/* ------------- Command Buffer Manager ------------- */

ctResults ctVkCommandBufferManager::Create(ctVkBackend* pBackend,
                                           uint32_t max,
                                           uint32_t familyIdx) {
   cmdBuffers.Resize(max);
   VkCommandPoolCreateInfo createInfo = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
   createInfo.queueFamilyIndex = familyIdx;
   CT_VK_CHECK(vkCreateCommandPool(
                 pBackend->vkDevice, &createInfo, &pBackend->vkAllocCallback, &pool),
               CT_NC("vkCreateCommandPool() failed to create command pool."));

   VkCommandBufferAllocateInfo allocInfo = {
     VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
   allocInfo.commandBufferCount = max;
   allocInfo.commandPool = pool;
   allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
   CT_VK_CHECK(
     vkAllocateCommandBuffers(pBackend->vkDevice, &allocInfo, cmdBuffers.Data()),
     CT_NC("vkAllocateCommandBuffers() failed to allocate command buffers."));
   return CT_SUCCESS;
}

ctResults ctVkCommandBufferManager::Destroy(ctVkBackend* pBackend) {
   vkFreeCommandBuffers(
     pBackend->vkDevice, pool, (uint32_t)cmdBuffers.Count(), cmdBuffers.Data());
   vkDestroyCommandPool(pBackend->vkDevice, pool, &pBackend->vkAllocCallback);
   return CT_SUCCESS;
}

VkCommandBuffer ctVkCommandBufferManager::GetNextCommandBuffer() {
   if (cmdBuffers.Count() <= activeBufferCount) {
      return VK_NULL_HANDLE;
   } else {
      const VkCommandBuffer result = cmdBuffers[activeBufferCount];
      activeBufferCount++;
      return result;
   }
}

VkResult
ctVkCommandBufferManager::SubmitCommands(VkQueue queue,
                                         uint32_t signalSemaphoreCount,
                                         VkSemaphore* pSignalSemaphores,
                                         uint32_t waitSemaphoreCount,
                                         VkSemaphore* pWaitSemaphores,
                                         VkFence fence,
                                         VkPipelineStageFlags* pCustomWaitStages) {
   VkPipelineStageFlags defaultWaitStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
   VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
   submitInfo.commandBufferCount = activeBufferCount;
   submitInfo.pCommandBuffers = cmdBuffers.Data();
   submitInfo.signalSemaphoreCount = signalSemaphoreCount;
   submitInfo.pSignalSemaphores = pSignalSemaphores;
   submitInfo.waitSemaphoreCount = waitSemaphoreCount;
   submitInfo.pWaitSemaphores = pWaitSemaphores;
   submitInfo.pWaitDstStageMask =
     pCustomWaitStages ? pCustomWaitStages : &defaultWaitStage;
   activeBufferCount = 0;
   return vkQueueSubmit(queue, 1, &submitInfo, fence);
}