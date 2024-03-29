/*
   Copyright 2022 MacKenzie Strand

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
   appInfo.applicationVersion = VK_MAKE_VERSION(
     pCreateInfo->version[0], pCreateInfo->version[1], pCreateInfo->version[2]);
   pDevice->validationEnabled = pCreateInfo->validationEnabled;
   pDevice->pMainWindow = (SDL_Window*)pCreateInfo->pMainWindow;
   pDevice->nextFrameTimeout = 10000;
   pDevice->preferredDevice = -1;
   pDevice->fpOpenCacheFileCallback = pCreateInfo->fpOpenCacheFileCallback;
   pDevice->pCacheCallbackCustomData = pCreateInfo->pCacheCallbackCustomData;
   pDevice->fpOpenAssetFileCallback = pCreateInfo->fpOpenAssetFileCallback;
   pDevice->pAssetCallbackCustomData = pCreateInfo->pAssetCallbackCustomData;
   ctResults result = pDevice->Startup();
   if (pCapabilitiesOut) { memset(pCapabilitiesOut, 0, sizeof(*pCapabilitiesOut)); }
   return result;
}

CT_API void ctGPUDeviceWaitForIdle(ctGPUDevice* pDevice) {
   vkDeviceWaitIdle(pDevice->vkDevice);
}

CT_API ctResults ctGPUDeviceShutdown(ctGPUDevice* pDevice) {
   ZoneScoped;
   ctAssert(pDevice);
   ctResults result = pDevice->Shutdown();
   delete pDevice;
   return result;
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
   ctDebugLog("Checking Device Extensions...");
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

void ctGPUDevice::ApplyOptionalDeviceExtensions(VkPhysicalDevice gpu) {
   uint32_t extCount;
   vkEnumerateDeviceExtensionProperties(gpu, NULL, &extCount, NULL);
   VkExtensionProperties* extensions =
     (VkExtensionProperties*)ctMalloc(sizeof(VkExtensionProperties) * extCount);
   vkEnumerateDeviceExtensionProperties(gpu, NULL, &extCount, extensions);
   for (uint32_t i = 0; i < extCount; i++) {
      if (validationEnabled &&
          ctCStrEql(extensions[i].extensionName, VK_EXT_DEBUG_MARKER_EXTENSION_NAME)) {
         useMarkers = true;
         deviceExtensions.Append(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
      }
   }
   ctFree(extensions);
}

bool vDeviceHasSwapChainSupport(VkPhysicalDevice gpu, VkSurfaceKHR surface) {
   ZoneScoped;
   ctVkSwapchainSupport support;
   support.GetSupport(gpu, surface);
   return !support.presentModes.isEmpty() && !support.surfaceFormats.isEmpty();
}

bool vDeviceHasRequiredFeatures(const VkPhysicalDeviceFeatures& features,
                                const VkPhysicalDeviceVulkan11Features& features_1_1,
                                const VkPhysicalDeviceVulkan12Features& features_1_2,
                                const VkPhysicalDeviceVulkan13Features& features_1_3) {
   if (!features.shaderFloat64 || !features.depthClamp || !features.depthBounds ||
       !features.fillModeNonSolid || !features.shaderSampledImageArrayDynamicIndexing ||
       !features.shaderStorageBufferArrayDynamicIndexing ||
       !features_1_2.descriptorBindingPartiallyBound ||
       !features_1_2.runtimeDescriptorArray ||
       !features_1_2.descriptorBindingSampledImageUpdateAfterBind ||
       !features_1_2.descriptorBindingStorageBufferUpdateAfterBind ||
       !features_1_2.descriptorBindingStorageImageUpdateAfterBind ||
       !features_1_2.shaderSampledImageArrayNonUniformIndexing ||
       !features_1_2.shaderStorageBufferArrayNonUniformIndexing ||
       !features_1_2.shaderStorageImageArrayNonUniformIndexing ||
       !features_1_3.dynamicRendering) {
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
      ctDebugLog("Checking Device %i...", i);
      currentGPUScore = 0;

      VkPhysicalDeviceProperties deviceProperties;
      VkPhysicalDeviceFeatures deviceFeatures;

      VkPhysicalDeviceVulkan11Features vk11Features {
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES};
      VkPhysicalDeviceVulkan12Features vk12Features {
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
      VkPhysicalDeviceVulkan13Features vk13Features {
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
      VkPhysicalDeviceFeatures2 deviceFeatures2 = {
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
      deviceFeatures2.pNext = &vk11Features;
      vk11Features.pNext = &vk12Features;
      vk12Features.pNext = &vk13Features;

      ctDebugLog("Getting Device Features 1/2...");
      vkGetPhysicalDeviceProperties(pGpus[i], &deviceProperties);
      vkGetPhysicalDeviceFeatures(pGpus[i], &deviceFeatures);
      ctDebugLog("Getting Device Features 2/2...");
      vkGetPhysicalDeviceFeatures2(pGpus[i], &deviceFeatures2);  // render-doc crash

      /*Disqualifications*/
      ctDebugLog("Checking Disqualifications...");
      if (!vDeviceHasRequiredFeatures(
            deviceFeatures, vk11Features, vk12Features, vk13Features))
         continue; /*Device doesn't meet the minimum features spec*/
      if (!vIsQueueFamilyComplete(FindQueueFamilyIndices(pGpus[i])))
         continue; /*Queue families are incomplete*/
      if (!DeviceHasRequiredExtensions(pGpus[i]))
         continue; /*Doesn't have the required extensions*/
      if ((surface != VK_NULL_HANDLE) && !vDeviceHasSwapChainSupport(pGpus[i], surface))
         continue; /*Doesn't have swap chain support*/

      /*Benifits*/
      if (deviceProperties.deviceType ==
          VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) /* Discrete GPU */
         currentGPUScore += 10000;
      /* Favor well known vendors who have reliable Vulkan drivers */
      if (deviceProperties.vendorID == 0x10DE || /* NVIDIA */
          deviceProperties.vendorID == 0x1002 || /* AMD */
          deviceProperties.vendorID == 0x8086)   /* Intel */
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
      if (vkInitialSurface != VK_NULL_HANDLE) {
         vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i, vkInitialSurface, &present);
      } else {
         present = VK_TRUE;
      }
      if (queueFamilyProps[i].queueCount > 0 && present) result.presentIdx = i;

      if (vIsQueueFamilyComplete(result)) break;
   }
   ctFree(queueFamilyProps);
   return result;
}

/* ------------- Init and Shutdown ------------- */

ctResults ctGPUDevice::Startup() {
   ZoneScoped;
   ctDebugLog("Starting Vulkan Backend...");
   SDL_Vulkan_LoadLibrary(NULL);
   /* Fill in AppInfo */
   {
      vkAppInfo.engineVersion = VK_MAKE_VERSION(CITRUS_ENGINE_VERSION_MAJOR,
                                                CITRUS_ENGINE_VERSION_MINOR,
                                                CITRUS_ENGINE_VERSION_PATCH);
      vkAppInfo.pEngineName = "Citrus Toolbox";
      vkAppInfo.apiVersion = VK_API_VERSION_1_3;

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
        vkCreateInstance(&instanceInfo, GetAllocCallback(), &vkInstance),
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
           vkInstance, &createInfo, GetAllocCallback(), &vkDebugCallback);
      } else {
         ctFatalError(-1,
                      CT_NCT("FAIL:vkCreateDebugReportCallbackEXT",
                             "Failed to find vkCreateDebugReportCallbackEXT()."));
      }
   }
   /* Initialize a surface for the main window (we do this to check capabilities) */
   {
      if (pMainWindow) {
         ctDebugLog("Creating Surface...");
         SDL_Vulkan_CreateSurface(pMainWindow, vkInstance, &vkInitialSurface);
      } else {
         vkInitialSurface = VK_NULL_HANDLE;
      }
   }
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
         ctDebugLog("Manual GPU Selected: %i...", preferredDevice);
         vkPhysicalDevice = gpus[preferredDevice];

         VkPhysicalDeviceVulkan11Features vk11Features {
           VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES};
         VkPhysicalDeviceVulkan12Features vk12Features {
           VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
         VkPhysicalDeviceVulkan13Features vk13Features {
           VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
         VkPhysicalDeviceFeatures2 deviceFeatures2 = {
           VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
         deviceFeatures2.pNext = &vk11Features;
         vk11Features.pNext = &vk12Features;
         vk12Features.pNext = &vk13Features;

         vkGetPhysicalDeviceProperties(vkPhysicalDevice, &vPhysicalDeviceProperties);
         vkGetPhysicalDeviceFeatures(vkPhysicalDevice, &vPhysicalDeviceFeatures);
         vkGetPhysicalDeviceFeatures2(vkPhysicalDevice, &vPhysicalDeviceFeatures2);

         if (!vDeviceHasRequiredFeatures(
               vPhysicalDeviceFeatures, vk11Features, vk12Features, vk13Features) ||
             !DeviceHasRequiredExtensions(vkPhysicalDevice) ||
             !(vkInitialSurface != VK_NULL_HANDLE
                 ? vDeviceHasSwapChainSupport(vkPhysicalDevice, vkInitialSurface)
                 : true)) {
            ctFatalError(-1,
                         CT_NCT("FAIL:GPUReqNotMet",
                                "Rendering device does not meet requirements."));
         }
      } else {
         vkPhysicalDevice = PickBestDevice(gpus.Data(), gpuCount, vkInitialSurface);
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
      features.fillModeNonSolid = VK_TRUE;
      features.wideLines = VK_TRUE;
      features.largePoints = VK_TRUE;
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

      VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures = {
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES};
      dynamicRenderingFeatures.dynamicRendering = true;
      indexingFeatures.pNext = &dynamicRenderingFeatures;

      ApplyOptionalDeviceExtensions(vkPhysicalDevice);

      deviceInfo.enabledExtensionCount = (uint32_t)deviceExtensions.Count();
      deviceInfo.ppEnabledExtensionNames = deviceExtensions.Data();
      ctDebugLog("Creating Device...");
      CT_VK_CHECK(
        vkCreateDevice(vkPhysicalDevice, &deviceInfo, GetAllocCallback(), &vkDevice),
        CT_NCT("FAIL:vkCreateDevice", "vkCreateDevice() failed to create the device."));

      /* Debug Markers*/
      if (useMarkers) { SetupMarkers(); }

      /* Get Queues */
      ctDebugLog("Getting Queues...");
      vkGetDeviceQueue(vkDevice, queueFamilyIndices.graphicsIdx, 0, &graphicsQueue);
      vkGetDeviceQueue(vkDevice, queueFamilyIndices.presentIdx, 0, &presentQueue);
      vkGetDeviceQueue(vkDevice, queueFamilyIndices.computeIdx, 0, &computeQueue);
      vkGetDeviceQueue(vkDevice, queueFamilyIndices.transferIdx, 0, &transferQueue);
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
   /* JIT Renderpass and Framebuffers */
   {
      ctSpinLockInit(jitPipelineRenderpassLock);
      ctSpinLockInit(jitUsableRenderInfoLock);
   }
   /* Pipeline Cache */
   {
      vkPipelineCache = VK_NULL_HANDLE;
      if (fpOpenCacheFileCallback) {
         ctDebugLog("Loading Pipeline Cache...");
         ctFile cacheFile = OpenCacheFile(PIPELINE_CACHE_FILE_PATH, CT_FILE_OPEN_READ);
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
                       vkDevice, &cacheInfo, GetAllocCallback(), &vkPipelineCache),
                     CT_NCT("FAIL:vkCreatePipelineCache",
                            "vkCreatePipelineCache() failed to create cache."));
         if (cacheData) { ctFree(cacheData); }
      }
   }
   ctDebugLog("Vulkan Backend has Started!");
   return CT_SUCCESS;
}

ctResults ctGPUDevice::Shutdown() {
   ZoneScoped;
   ctDebugLog("Vulkan Backend is Shutting Down...");
   vkDeviceWaitIdle(vkDevice);

   DestroyAllStagingBuffers();
   DestroyJITRenderpasses();

   /* Save Pipeline Cache */
   if (fpOpenCacheFileCallback) {
      size_t cacheSize = 0;
      void* cacheData = NULL;
      vkGetPipelineCacheData(vkDevice, vkPipelineCache, &cacheSize, NULL);
      if (cacheSize) {
         cacheData = ctMalloc(cacheSize);
         vkGetPipelineCacheData(vkDevice, vkPipelineCache, &cacheSize, cacheData);
         ctFile cacheFile = OpenCacheFile(PIPELINE_CACHE_FILE_PATH, CT_FILE_OPEN_WRITE);
         if (cacheFile.isOpen()) {
            cacheFile.WriteRaw(cacheData, cacheSize, 1);
            cacheFile.Close();
         }
         ctFree(cacheData);
      }
   }
   if (vkPipelineCache) {
      vkDestroyPipelineCache(vkDevice, vkPipelineCache, GetAllocCallback());
   }

   if (vkInitialSurface) { vkDestroySurfaceKHR(vkInstance, vkInitialSurface, NULL); }
   vmaDestroyAllocator(vmaAllocator);
   vkDestroyDevice(vkDevice, GetAllocCallback());

   if (vkDebugCallback != VK_NULL_HANDLE) {
      PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallback =
        (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(
          vkInstance, "vkDestroyDebugReportCallbackEXT");
      vkDestroyDebugReportCallback(vkInstance, vkDebugCallback, GetAllocCallback());
   }

   vkDestroyInstance(vkInstance, GetAllocCallback());
   return CT_SUCCESS;
}

VkAllocationCallbacks* ctGPUDevice::GetAllocCallback() {
   return &vkAllocCallback;
}

VkResult ctGPUDevice::CreateCompleteImage(const char* name,
                                          ctVkCompleteImage& fullImage,
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
   char fullName[40];
   memset(fullName, 0, 40);
   snprintf(fullName, 39, "%s - Image", name);
   SetObjectMarker(
     fullName, (uint64_t)fullImage.image, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT);

   VkImageViewCreateInfo viewInfo {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
   viewInfo.image = fullImage.image;
   viewInfo.components = {VK_COMPONENT_SWIZZLE_IDENTITY,
                          VK_COMPONENT_SWIZZLE_IDENTITY,
                          VK_COMPONENT_SWIZZLE_IDENTITY,
                          VK_COMPONENT_SWIZZLE_IDENTITY};
   viewInfo.viewType = viewType;
   viewInfo.format = format;
   viewInfo.subresourceRange.aspectMask = aspect;
   viewInfo.subresourceRange.baseArrayLayer = 0;
   viewInfo.subresourceRange.layerCount = layers;
   viewInfo.subresourceRange.baseMipLevel = 0;
   viewInfo.subresourceRange.levelCount = mip;
   result = vkCreateImageView(vkDevice, &viewInfo, GetAllocCallback(), &fullImage.view);
   if (result != VK_SUCCESS) { return result; }
   memset(fullName, 0, 40);
   snprintf(fullName, 39, "%s - View", name);
   SetObjectMarker(
     fullName, (uint64_t)fullImage.view, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT);
   return result;
}

VkResult ctGPUDevice::CreateCompleteBuffer(const char* name,
                                           ctVkCompleteBuffer& fullBuffer,
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
   VkResult result = vmaCreateBuffer(
     vmaAllocator, &bufferInfo, &allocInfo, &fullBuffer.buffer, &fullBuffer.alloc, NULL);
   if (result != VK_SUCCESS) { return result; }
   SetObjectMarker(
     name, (uint64_t)fullBuffer.buffer, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT);
   return result;
}

void ctGPUDevice::TryDestroyCompleteImage(ctVkCompleteImage& fullImage) {
   ZoneScoped;
   /* todo: deferred deletion */
   if (fullImage.view == VK_NULL_HANDLE) { return; }
   vkDestroyImageView(vkDevice, fullImage.view, GetAllocCallback());
   if (fullImage.image == VK_NULL_HANDLE || fullImage.alloc == VK_NULL_HANDLE) { return; }
   vmaDestroyImage(vmaAllocator, fullImage.image, fullImage.alloc);
}

void ctGPUDevice::TryDestroyCompleteBuffer(ctVkCompleteBuffer& fullBuffer) {
   ZoneScoped;
   if (fullBuffer.buffer == VK_NULL_HANDLE || fullBuffer.alloc == VK_NULL_HANDLE) {
      return;
   }
   /* todo: deferred deletion */
   vmaDestroyBuffer(vmaAllocator, fullBuffer.buffer, fullBuffer.alloc);
}

ctResults ctGPUDevice::GetStagingBuffer(ctVkCompleteBuffer& fullBuffer,
                                        size_t sizeRequest) {
   /* Find existing */
   const size_t count = stagingBufferCooldown.Count();
   for (size_t i = 0; i < count; i++) {
      if (stagingBufferCooldown[i] == 0) {
         if (stagingBuffers[i].size >= sizeRequest) {
            fullBuffer = stagingBuffers[i].buffer;
            stagingBufferCooldown[i] = -1;
            return CT_SUCCESS;
         }
      }
   }

   /* Create new */
   StagingEntry entry;
   entry.size = sizeRequest;
   CreateCompleteBuffer("Staging",
                        entry.buffer,
                        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VMA_ALLOCATION_CREATE_STRATEGY_MIN_FRAGMENTATION_BIT,
                        sizeRequest,
                        VMA_MEMORY_USAGE_CPU_TO_GPU);
   stagingBufferCooldown.Append(-1);
   stagingBuffers.Append(entry);
   fullBuffer = entry.buffer;
   return CT_SUCCESS;
}

ctResults ctGPUDevice::ReleaseStagingBuffer(ctVkCompleteBuffer& fullBuffer) {
   const size_t count = stagingBuffers.Count();
   for (size_t i = 0; i < count; i++) {
      if (stagingBuffers[i].buffer.buffer == fullBuffer.buffer) {
         stagingBufferCooldown[i] = CT_MAX_INFLIGHT_FRAMES;
      }
   }
   return CT_SUCCESS;
}

void ctGPUDevice::AdvanceStagingCooldownTimers() {
   const size_t count = stagingBufferCooldown.Count();
   for (size_t i = 0; i < count; i++) {
      if (stagingBufferCooldown[i] > 0) { stagingBufferCooldown[i]--; }
   }
}

void ctGPUDevice::DestroyAllStagingBuffers() {
   for (size_t i = 0; i < stagingBuffers.Count(); i++) {
      TryDestroyCompleteBuffer(stagingBuffers[i].buffer);
   }
   stagingBufferCooldown.Clear();
   stagingBuffers.Clear();
}

void ctGPUDevice::DestroyJITRenderpasses() {
   for (auto it = pipelineRenderpasses.GetIterator(); it; it++) {
      vkDestroyRenderPass(vkDevice, it.Value(), GetAllocCallback());
   }
   for (auto it = usableRenderInfo.GetIterator(); it; it++) {
      vkDestroyRenderPass(vkDevice, it.Value().renderpass, GetAllocCallback());
      vkDestroyFramebuffer(vkDevice, it.Value().framebuffer, GetAllocCallback());
   }
}

uint32_t HashPipelineInfo(const VkPipelineRenderingCreateInfoKHR* pInfo) {
   uint32_t h1 = pInfo->colorAttachmentCount
                   ? ctXXHash32(pInfo->pColorAttachmentFormats,
                                sizeof(VkFormat) * pInfo->colorAttachmentCount)
                   : 0;
   return ctXXHash32(pInfo, sizeof(VkPipelineRenderingCreateInfoKHR), h1);
}

/* This is VERY hacky but Vulkan's renderpass system is not very good... */
VkRenderPass
ctGPUDevice::GetJITPipelineRenderpass(VkPipelineRenderingCreateInfoKHR& info) {
   /* Fetch existing */
   uint32_t hash = HashPipelineInfo(&info);
   ctSpinLockEnterCritical(jitPipelineRenderpassLock);
   VkRenderPass* pExistingPass = pipelineRenderpasses.FindPtr(hash);
   ctSpinLockExitCritical(jitPipelineRenderpassLock);
   if (pExistingPass) { return *pExistingPass; }

   VkAttachmentReference depthAttachmentRef;
   ctStaticArray<VkAttachmentReference, 8> colorAttachmentRefs;
   ctStaticArray<VkAttachmentDescription, 12> attachments;

   /* Configure depth stencil */
   VkFormat depthStencilFormat = VK_FORMAT_UNDEFINED;
   if (info.depthAttachmentFormat != VK_FORMAT_UNDEFINED) {
      depthStencilFormat = info.depthAttachmentFormat;
   } else {
      depthStencilFormat = info.stencilAttachmentFormat;
   }
   if (depthStencilFormat != VK_FORMAT_UNDEFINED) {
      depthAttachmentRef = {(uint32_t)attachments.Count(),
                            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
      VkAttachmentDescription desc = {};
      desc.format = depthStencilFormat;
      desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
      desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
      desc.flags = 0;
      desc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
      desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
      desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
      desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
      desc.samples = VK_SAMPLE_COUNT_1_BIT; /* doesn't support MSAA, don't cate */
      attachments.Append(desc);
   }

   /* Configure colors */
   for (uint32_t i = 0; i < info.colorAttachmentCount; i++) {
      colorAttachmentRefs.Append(
        {(uint32_t)attachments.Count(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
      VkAttachmentDescription desc = {};
      desc.format = info.pColorAttachmentFormats[i];
      desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
      desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
      desc.flags = 0;
      desc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
      desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
      desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
      desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
      desc.samples = VK_SAMPLE_COUNT_1_BIT; /* doesn't support MSAA, don't cate */
      attachments.Append(desc);
   }

   /* Make dummy subpass */
   VkSubpassDescription subpassInfo = {};
   subpassInfo.colorAttachmentCount = (uint32_t)colorAttachmentRefs.Count();
   subpassInfo.pColorAttachments = colorAttachmentRefs.Data();
   subpassInfo.pDepthStencilAttachment =
     depthStencilFormat != VK_FORMAT_UNDEFINED ? &depthAttachmentRef : NULL;
   subpassInfo.inputAttachmentCount = 0;
   subpassInfo.pInputAttachments = NULL;
   subpassInfo.preserveAttachmentCount = 0;
   subpassInfo.pPreserveAttachments = NULL;
   subpassInfo.pResolveAttachments = NULL;
   subpassInfo.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

   /* Create renderpass */
   VkRenderPass newRenderpass;
   VkRenderPassCreateInfo renderpassInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
   renderpassInfo.attachmentCount = (uint32_t)attachments.Count();
   renderpassInfo.pAttachments = attachments.Data();
   renderpassInfo.subpassCount = 1;
   renderpassInfo.pSubpasses = &subpassInfo;
   CT_VK_CHECK(
     vkCreateRenderPass(vkDevice, &renderpassInfo, GetAllocCallback(), &newRenderpass),
     CT_NC("vkCreateRenderPass() failed to create temporary pipeline renderpass."));

   /* Insert new */
   ctSpinLockEnterCritical(jitPipelineRenderpassLock);
   pipelineRenderpasses.Insert(hash, newRenderpass);
   ctSpinLockExitCritical(jitPipelineRenderpassLock);
   return newRenderpass;
}

ctGPUDevice::CompleteRenderInfo
ctGPUDevice::CreateCompleteRenderInfo(const VkRenderingInfoKHR* pRenderingInfo,
                                      VkFormat depthStencilFormat,
                                      VkFormat* pColorFormats,
                                      VkImageLayout lastDepthLayout,
                                      VkImageLayout* pLastColorLayouts) {
   CompleteRenderInfo newRenderInfo = CompleteRenderInfo();

   VkAttachmentReference depthAttachmentRef;
   ctStaticArray<VkAttachmentReference, 8> colorAttachmentRefs;
   ctStaticArray<VkAttachmentDescription, 12> attachments;

   /* Configure Depth */
   if (depthStencilFormat != VK_FORMAT_UNDEFINED) {
      depthAttachmentRef = {(uint32_t)attachments.Count(),
                            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
      VkAttachmentDescription desc = {};
      desc.format = depthStencilFormat;
      desc.initialLayout = lastDepthLayout;
      desc.finalLayout = pRenderingInfo->pDepthAttachment->imageLayout;
      desc.flags = 0;
      desc.loadOp = pRenderingInfo->pDepthAttachment->loadOp;
      desc.storeOp = pRenderingInfo->pDepthAttachment->storeOp;
      desc.stencilLoadOp = pRenderingInfo->pDepthAttachment->loadOp;
      desc.stencilStoreOp = pRenderingInfo->pDepthAttachment->storeOp;
      desc.samples = VK_SAMPLE_COUNT_1_BIT; /* doesn't support MSAA, don't cate */
      attachments.Append(desc);
   }

   /* Configure colors */
   for (uint32_t i = 0; i < pRenderingInfo->colorAttachmentCount; i++) {
      colorAttachmentRefs.Append(
        {(uint32_t)attachments.Count(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
      VkAttachmentDescription desc = {};
      desc.format = pColorFormats[i];
      desc.initialLayout = pLastColorLayouts[i];
      desc.finalLayout = pRenderingInfo->pColorAttachments[i].imageLayout;
      desc.flags = 0;
      desc.loadOp = pRenderingInfo->pColorAttachments[i].loadOp;
      desc.storeOp = pRenderingInfo->pColorAttachments[i].storeOp;
      desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      desc.samples = VK_SAMPLE_COUNT_1_BIT; /* doesn't support MSAA, don't cate */
      attachments.Append(desc);
   }

   /* Make dummy subpass */
   VkSubpassDescription subpassInfo = {};
   subpassInfo.colorAttachmentCount = (uint32_t)colorAttachmentRefs.Count();
   subpassInfo.pColorAttachments = colorAttachmentRefs.Data();
   subpassInfo.pDepthStencilAttachment =
     depthStencilFormat != VK_FORMAT_UNDEFINED ? &depthAttachmentRef : NULL;
   subpassInfo.inputAttachmentCount = 0;
   subpassInfo.pInputAttachments = NULL;
   subpassInfo.preserveAttachmentCount = 0;
   subpassInfo.pPreserveAttachments = NULL;
   subpassInfo.pResolveAttachments = NULL;
   subpassInfo.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

   /* Create renderpass */
   VkRenderPassCreateInfo renderpassInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
   renderpassInfo.attachmentCount = (uint32_t)attachments.Count();
   renderpassInfo.pAttachments = attachments.Data();
   renderpassInfo.subpassCount = 1;
   renderpassInfo.pSubpasses = &subpassInfo;
   CT_VK_CHECK(
     vkCreateRenderPass(
       vkDevice, &renderpassInfo, GetAllocCallback(), &newRenderInfo.renderpass),
     CT_NC("vkCreateRenderPass() failed to create just-in-time renderpass."));

   /* Create framebuffer */
   ctStaticArray<VkImageView, 9> imageViews;
   if (pRenderingInfo->pDepthAttachment) {
      imageViews.Append(pRenderingInfo->pDepthAttachment->imageView);
   }
   for (uint32_t i = 0; i < pRenderingInfo->colorAttachmentCount; i++) {
      imageViews.Append(pRenderingInfo->pColorAttachments[i].imageView);
   }
   VkFramebufferCreateInfo framebufferInfo = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
   framebufferInfo.width = pRenderingInfo->renderArea.extent.width;
   framebufferInfo.height = pRenderingInfo->renderArea.extent.height;
   framebufferInfo.layers = pRenderingInfo->layerCount;
   framebufferInfo.renderPass = newRenderInfo.renderpass;
   framebufferInfo.attachmentCount = (uint32_t)imageViews.Count();
   framebufferInfo.pAttachments = imageViews.Data();
   CT_VK_CHECK(
     vkCreateFramebuffer(
       vkDevice, &framebufferInfo, GetAllocCallback(), &newRenderInfo.framebuffer),
     CT_NC("vkCreateFramebuffer() failed to create just-in-time framebuffer."));
   return newRenderInfo;
}

void ctGPUDevice::SetupMarkers() {
   ctDebugLog("Getting Debug Markers...");
   fpDebugMarkerSetObjectTag = (PFN_vkDebugMarkerSetObjectTagEXT)vkGetDeviceProcAddr(
     vkDevice, "vkDebugMarkerSetObjectTagEXT");
   fpDebugMarkerSetObjectName = (PFN_vkDebugMarkerSetObjectNameEXT)vkGetDeviceProcAddr(
     vkDevice, "vkDebugMarkerSetObjectNameEXT");
   fpCmdDebugMarkerBegin = (PFN_vkCmdDebugMarkerBeginEXT)vkGetDeviceProcAddr(
     vkDevice, "vkCmdDebugMarkerBeginEXT");
   fpCmdDebugMarkerEnd =
     (PFN_vkCmdDebugMarkerEndEXT)vkGetDeviceProcAddr(vkDevice, "vkCmdDebugMarkerEndEXT");
   fpCmdDebugMarkerInsert = (PFN_vkCmdDebugMarkerInsertEXT)vkGetDeviceProcAddr(
     vkDevice, "vkCmdDebugMarkerInsertEXT");
}

void ctGPUDevice::SetObjectMarker(const char* name,
                                  uint64_t object,
                                  VkDebugReportObjectTypeEXT type) {
   if (useMarkers) {
      VkDebugMarkerObjectNameInfoEXT nameInfo = {};
      nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
      nameInfo.objectType = type;
      nameInfo.object = object;
      nameInfo.pObjectName = name;
      fpDebugMarkerSetObjectName(vkDevice, &nameInfo);
   }
}

void ctGPUDevice::MarkBeginRegion(VkCommandBuffer cmd, const char* name) {
   if (!useMarkers) { return; }
   VkDebugMarkerMarkerInfoEXT info = {VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT};
   info.pMarkerName = name;
   ctRandomGenerator rgen = ctXXHash32(name);
   const ctVec4 randomColor = rgen.GetColor();
   memcpy(info.color, randomColor.data, sizeof(float) * 4);
   fpCmdDebugMarkerBegin(cmd, &info);
}

void ctGPUDevice::MarkEndRegion(VkCommandBuffer cmd) {
   if (!useMarkers) { return; }
   fpCmdDebugMarkerEnd(cmd);
}

uint32_t HashRenderingInfo(const VkRenderingInfoKHR* pRenderingInfo) {
   uint32_t h1 = pRenderingInfo->pDepthAttachment
                   ? ctXXHash32(pRenderingInfo->pDepthAttachment,
                                sizeof(VkRenderingAttachmentInfoKHR))
                   : 0;
   uint32_t h2 = pRenderingInfo->pStencilAttachment
                   ? ctXXHash32(pRenderingInfo->pStencilAttachment,
                                sizeof(VkRenderingAttachmentInfoKHR),
                                h1)
                   : h1;
   uint32_t h3 = pRenderingInfo->colorAttachmentCount
                   ? ctXXHash32(pRenderingInfo->pColorAttachments,
                                sizeof(VkRenderingAttachmentInfoKHR) *
                                  pRenderingInfo->colorAttachmentCount,
                                h2)
                   : h2;
   return ctXXHash32(pRenderingInfo, sizeof(VkRenderingInfoKHR), h3);
}

void ctGPUDevice::BeginJITRenderPass(VkCommandBuffer commandBuffer,
                                     const VkRenderingInfoKHR* pRenderingInfo,
                                     VkFormat depthStencilFormat,
                                     VkFormat* pColorFormats,
                                     VkImageLayout lastDepthLayout,
                                     VkImageLayout* pLastColorLayouts) {
   if (isDynamicRenderingSupported()) {
      vkCmdBeginRendering(commandBuffer, pRenderingInfo);
   } else {
      uint32_t hash = HashRenderingInfo(pRenderingInfo);
      ctXXHash32(&depthStencilFormat, sizeof(depthStencilFormat), hash);
      ctXXHash32(pColorFormats,
                 sizeof(pColorFormats[0]) * pRenderingInfo->colorAttachmentCount,
                 hash);
      ctXXHash32(&lastDepthLayout, sizeof(lastDepthLayout), hash);
      ctXXHash32(pLastColorLayouts,
                 sizeof(pLastColorLayouts[0]) * pRenderingInfo->colorAttachmentCount,
                 hash);
      ctSpinLockEnterCritical(jitUsableRenderInfoLock);
      CompleteRenderInfo* pCompleteRenderInfo = usableRenderInfo.FindPtr(hash);
      ctSpinLockExitCritical(jitUsableRenderInfoLock);
      if (!pCompleteRenderInfo) {
         CompleteRenderInfo newRenderInfo = CreateCompleteRenderInfo(pRenderingInfo,
                                                                     depthStencilFormat,
                                                                     pColorFormats,
                                                                     lastDepthLayout,
                                                                     pLastColorLayouts);
         ctSpinLockEnterCritical(jitUsableRenderInfoLock);
         pCompleteRenderInfo = usableRenderInfo.Insert(hash, newRenderInfo);
         ctSpinLockExitCritical(jitUsableRenderInfoLock);
      }

      ctStaticArray<VkClearValue, 9> clears;
      if (pRenderingInfo->pDepthAttachment) {
         clears.Append(pRenderingInfo->pDepthAttachment->clearValue);
      } else {
      }
      for (uint32_t i = 0; i < pRenderingInfo->colorAttachmentCount; i++) {
         clears.Append(pRenderingInfo->pColorAttachments[i].clearValue);
      }
      VkRenderPassBeginInfo begin = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
      begin.framebuffer = pCompleteRenderInfo->framebuffer;
      begin.renderArea = pRenderingInfo->renderArea;
      begin.renderPass = pCompleteRenderInfo->renderpass;
      begin.clearValueCount = (uint32_t)clears.Count();
      begin.pClearValues = clears.Data();
      vkCmdBeginRenderPass(commandBuffer, &begin, VK_SUBPASS_CONTENTS_INLINE);
   }
}

void ctGPUDevice::EndJITRenderpass(VkCommandBuffer commandBuffer) {
   if (isDynamicRenderingSupported()) {
      vkCmdEndRendering(commandBuffer);
   } else {
      vkCmdEndRenderPass(commandBuffer);
   }
}
