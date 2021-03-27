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

#ifdef _WIN32
#include <Windows.h>
#endif
#define VMA_IMPLEMENTATION
#include "vma/vk_mem_alloc.h"

const char* deviceReqExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

/* Debug Callback */
VKAPI_ATTR VkBool32 VKAPI_CALL
vDebugCallback(VkDebugReportFlagsEXT flags,
               VkDebugReportObjectTypeEXT objType,
               uint64_t srcObject,
               size_t location,
               int32_t msgCode,
               const char* pLayerPrefix,
               const char* pMsg,
               void* pUserData) {
   ctDebugSystem* pDebug = (ctDebugSystem*)pUserData;
   pDebug->Warning(
     "VK Validation Layer: [%s] Code %u : %s", pLayerPrefix, msgCode, pMsg);
   return VK_FALSE;
}
VkDebugReportCallbackEXT vDbgCallback;

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
   return (
     indices.graphicsIdx != UINT32_MAX && indices.transferIdx != UINT32_MAX &&
     indices.presentIdx != UINT32_MAX && indices.computeIdx != UINT32_MAX);
}

ctVkQueueFamilyIndices
ctVkBackend::FindQueueFamilyIndices(VkPhysicalDevice gpu) {
   ctVkQueueFamilyIndices result = {
     UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX};
   uint32_t queueFamilyCount;
   vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, NULL);
   VkQueueFamilyProperties* queueFamilyProps =
     (VkQueueFamilyProperties*)ctMalloc(sizeof(VkQueueFamilyProperties) *
                                          queueFamilyCount,
                                        "Vulkan Queue Family Search");
   vkGetPhysicalDeviceQueueFamilyProperties(
     gpu, &queueFamilyCount, queueFamilyProps);
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
      vkGetPhysicalDeviceSurfaceSupportKHR(
        gpu, i, mainScreenResources.surface, &present);
      if (queueFamilyProps[i].queueCount > 0 && present) result.presentIdx = i;

      if (vIsQueueFamilyComplete(result)) break;
   }
   ctFree(queueFamilyProps);
   return result;
}

bool vDeviceHasRequiredExtensions(VkPhysicalDevice gpu) {
   uint32_t extCount;
   vkEnumerateDeviceExtensionProperties(gpu, NULL, &extCount, NULL);
   VkExtensionProperties* availible = (VkExtensionProperties*)ctMalloc(
     sizeof(VkExtensionProperties) * extCount, "Extension List");
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

bool vDeviceHasRequiredFeatures(VkPhysicalDeviceFeatures* pFeatures) {
   if (!pFeatures->imageCubeArray || /*Cubemap arrays must be supported*/
       !pFeatures->shaderFloat64 ||  /*Float 64 for Shaders must be supported*/
       !pFeatures->fillModeNonSolid || !pFeatures->depthClamp ||
       !pFeatures->depthBounds || !pFeatures->wideLines ||
       !pFeatures->shaderSampledImageArrayDynamicIndexing ||
       !pFeatures->shaderStorageBufferArrayDynamicIndexing)
      return false;
   return true;
}

VkPhysicalDevice ctVkBackend::PickBestDevice(VkPhysicalDevice* pGpus,
                                             uint32_t count) {
   int64_t bestGPUIdx = -1;
   int64_t bestGPUScore = -1;
   int64_t currentGPUScore;
   for (uint32_t i = 0; i < count; i++) {
      currentGPUScore = 0;
      VkPhysicalDeviceProperties deviceProperties;
      VkPhysicalDeviceFeatures deviceFeatures;
      vkGetPhysicalDeviceProperties(pGpus[i], &deviceProperties);
      vkGetPhysicalDeviceFeatures(pGpus[i], &deviceFeatures);

      /*Disqualifications*/
      if (!vDeviceHasRequiredFeatures(
            &deviceFeatures)) /*Device doesn't meet the minimum features spec*/
         continue;
      if (!vIsQueueFamilyComplete(
            FindQueueFamilyIndices(pGpus[i]))) /*Queue families are incomplete*/
         continue;
      if (!vDeviceHasRequiredExtensions(
            pGpus[i])) /*Doesn't have the required extensions*/
         continue;

      /*Benifits*/
      if (deviceProperties.deviceType ==
          VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) /*Not an integrated GPU*/
         currentGPUScore += 10000;
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

ctVkBackend::ctVkBackend() {
   pVkAllocCallback = NULL;
   preferredDevice = -1;
#ifndef NDEBUG
   validationEnabled = 1;
#else
   validationEnabled = 0;
#endif
   vkAppInfo = VkApplicationInfo();
   vkInstance = VkInstance();
   vkPhysicalDevice = VkPhysicalDevice();
   vkDevice = VkDevice();
   mainScreenResources = ctVkScreenResources();
}

ctResults ctVkBackend::Startup() {
   ZoneScoped;
   if (!Engine->WindowManager->isStarted()) {
      return CT_FAILURE_DEPENDENCY_NOT_MET;
   }
   if (!Engine->OSEventManager->isStarted()) {
      return CT_FAILURE_DEPENDENCY_NOT_MET;
   }
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
      pVkAllocCallback = NULL;

      validationLayers.Append("VK_LAYER_KHRONOS_validation");
   }
   /* Create Instance */
   {
      if (validationEnabled && !isValidationLayersAvailible()) {
         ctFatalError(
           -1, CT_NC("Vulkan Validation layers requested but not avalible"));
      }

      unsigned int sdlExtCount;
      unsigned int extraExtCount = 1;
      if (!SDL_Vulkan_GetInstanceExtensions(
            Engine->WindowManager->mainWindow.pSDLWindow, &sdlExtCount, NULL)) {
         ctFatalError(-1,
                      CT_NC("SDL_Vulkan_GetInstanceExtensions() Failed to get "
                            "instance extensions"));
      }
      instanceExtensions.Resize(sdlExtCount + extraExtCount);
      instanceExtensions[0] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
      SDL_Vulkan_GetInstanceExtensions(
        NULL, &sdlExtCount, &instanceExtensions[extraExtCount]);
      for (uint32_t i = 0; i < sdlExtCount + extraExtCount; i++) {
         ctDebugLog("VK Extension: \"%s\" found", instanceExtensions[i]);
      }

      VkInstanceCreateInfo instanceInfo = {
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
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
      CT_VK_CHECK(
        vkCreateInstance(&instanceInfo, pVkAllocCallback, &vkInstance),
        CT_NC("vkCreateInstance() Failed to create vulkan instance"));
   }
   /* Initialize a first surface to check for support */
   {
      mainScreenResources.Create(this,
                                 Engine->WindowManager->mainWindow.pSDLWindow);
   } /* Pick a GPU */
   {
      uint32_t gpuCount;
      CT_VK_CHECK(
        vkEnumeratePhysicalDevices(vkInstance, &gpuCount, NULL),
        CT_NC("Failed to find devices with vkEnumeratePhysicalDevices()"));
      if (!gpuCount) {
         ctFatalError(-1, CT_NC("No supported Vulkan compatible GPU found"));
      }
      ctDynamicArray<VkPhysicalDevice> gpus;
      gpus.Resize(gpuCount);
      vkEnumeratePhysicalDevices(vkInstance, &gpuCount, gpus.Data());
      if (preferredDevice >= 0 && preferredDevice < (int32_t)gpuCount) {
         vkPhysicalDevice = gpus[preferredDevice];

         VkPhysicalDeviceFeatures deviceFeatures;
         vkGetPhysicalDeviceFeatures(vkPhysicalDevice, &deviceFeatures);
         if (!vDeviceHasRequiredFeatures(&deviceFeatures) ||
             !vDeviceHasRequiredExtensions(vkPhysicalDevice)) {
            ctFatalError(-1, CT_NC("Graphics card doesn't meet requirements"));
         }
      } else {
         vkPhysicalDevice = PickBestDevice(gpus.Data(), gpuCount);
         if (vkPhysicalDevice == VK_NULL_HANDLE) {
            ctFatalError(-1, CT_NC("Could not find suitable graphics card"));
         }
      }
   }
   /* Queues and Device */
   {
      VkDeviceCreateInfo deviceInfo = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
      /* Queue Creation */
      queueFamilyIndices = FindQueueFamilyIndices(vkPhysicalDevice);
      if (!vIsQueueFamilyComplete(queueFamilyIndices)) {
         ctFatalError(-1, CT_NC("Device doesn't have the necessary queues"));
      }

      uint32_t uniqueIdxCount = 0;
      uint32_t uniqueIndices[4] = {
        UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX};

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
                 ii++) { /*Search previous items*/
               if (uniqueIndices[ii] ==
                   nonUniqueIndices[i]) { /*Only add if unique*/
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

      deviceInfo.enabledExtensionCount = 1;
      deviceInfo.ppEnabledExtensionNames = deviceReqExtensions;
      CT_VK_CHECK(vkCreateDevice(
                    vkPhysicalDevice, &deviceInfo, pVkAllocCallback, &vkDevice),
                  CT_NC("vkCreateDevice() failed to create the device"));

      /* Get Queues */
      vkGetDeviceQueue(
        vkDevice, queueFamilyIndices.graphicsIdx, 0, &graphicsQueue);
      vkGetDeviceQueue(
        vkDevice, queueFamilyIndices.presentIdx, 0, &presentQueue);
      vkGetDeviceQueue(
        vkDevice, queueFamilyIndices.computeIdx, 0, &computeQueue);
      vkGetDeviceQueue(
        vkDevice, queueFamilyIndices.transferIdx, 0, &transferQueue);
   }
   /* Memory Allocator */
   {
      VmaAllocatorCreateInfo allocatorInfo = {};
      /* Current version won't go further than Vulkan 1.1 */
      allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_1;
      allocatorInfo.physicalDevice = vkPhysicalDevice;
      allocatorInfo.device = vkDevice;
      allocatorInfo.instance = vkInstance;

      CT_VK_CHECK(vmaCreateAllocator(&allocatorInfo, &vmaAllocator),
                  CT_NC("vmaCreateAllocator() failed to create allocator"));
   }
   /* Pipeline Factory */
   {
     // Todo: :)
     // https://gpuopen.com/performance/#pso
     // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#pipelines-cache
     // Compile uber-shaders
     //
   } /* Bindless System */
   {
      // https://ourmachinery.com/post/moving-the-machinery-to-bindless/
      // https://roar11.com/2019/06/vulkan-textures-unbound/
   }
   Engine->WindowManager->ShowMainWindow();
   return CT_SUCCESS;
}

ctResults ctVkBackend::Shutdown() {
   mainScreenResources.Destroy(this);
   vmaDestroyAllocator(vmaAllocator);
   vkDestroyDevice(vkDevice, pVkAllocCallback);
   vkDestroyInstance(vkInstance, pVkAllocCallback);
   return CT_SUCCESS;
}

ctResults ctVkScreenResources::Create(ctVkBackend* pBackend,
                                      SDL_Window* pWindow) {
   SDL_Vulkan_CreateSurface(pWindow, pBackend->vkInstance, &surface);
   return CT_SUCCESS;
}

ctResults ctVkScreenResources::Destroy(ctVkBackend* pBackend) {
   vkDestroySurfaceKHR(
     pBackend->vkInstance, surface, pBackend->pVkAllocCallback);
   return CT_SUCCESS;
}
