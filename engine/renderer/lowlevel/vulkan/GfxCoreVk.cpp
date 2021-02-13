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

#include "GfxCoreVk.hpp"
#include "core/EngineCore.hpp"

#define CT_VK_CHECK(_args, _msg)                                               \
   {                                                                           \
      VkResult _tmpvresult = _args;                                            \
      if (_tmpvresult != VK_SUCCESS) {                                         \
         ctFatalError((int)_tmpvresult, #_msg);                                \
      }                                                                        \
   }

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

bool ctGfxCoreVk::isValidationLayersAvailible() {
   uint32_t layerCount = 0;
   vkEnumerateInstanceLayerProperties(&layerCount, NULL);
   ctDynamicArray<VkLayerProperties> layerProps;
   layerProps.Resize(layerCount);
   vkEnumerateInstanceLayerProperties(&layerCount, layerProps.Data());
   for (uint32_t i = 0; i < layerCount; i++) {
      Engine->Debug->Log("VK Layer: \"%s\" found", layerProps[i].layerName);
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

ctVkQueueFamilyIndices vFindQueueFamilyIndices(VkPhysicalDevice gpu) {
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
      // vkGetPhysicalDeviceSurfaceSupportKHR(
      //  gpu, i, vMainScreen.surface, &present);
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

VkPhysicalDevice vPickBestDevice(VkPhysicalDevice* pGpus, uint32_t count) {
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
            &deviceFeatures)) /*Device doesn't meet the minimum feature
                                 requirements*/
         continue;
      if (!vIsQueueFamilyComplete(vFindQueueFamilyIndices(
            pGpus[i]))) /*Queue families are incomplete*/
         continue;
      if (!vDeviceHasRequiredExtensions(
            pGpus[i])) /*Doesn't have the required extensions*/
         continue;

      /*Benifits*/
      if (deviceProperties.deviceType ==
          VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) /*Not an integrated GPU*/
         currentGPUScore += 10000;

      /*Set as GPU if its the best*/
      if (currentGPUScore > bestGPUScore) {
         bestGPUScore = currentGPUScore;
         bestGPUIdx = i;
      }
   }
   if (bestGPUIdx < 0) return VK_NULL_HANDLE;
   return pGpus[bestGPUIdx];
}

ctGfxCoreVk::ctGfxCoreVk(bool validate,
                         const char* appName,
                         int appVersion[3]) {
   preferredDevice = -1;
   validationEnabled = validate;
   validationLayers.Append("VK_LAYER_KHRONOS_validation");

   vkAppInfo = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
   vkAppInfo.apiVersion = VK_VERSION_1_2;
   vkAppInfo.pEngineName = "CitrusToolbox";
   vkAppInfo.engineVersion = VK_MAKE_VERSION(CITRUS_ENGINE_VERSION_MAJOR,
                                             CITRUS_ENGINE_VERSION_MINOR,
                                             CITRUS_ENGINE_VERSION_PATCH);
   vkAppInfo.pApplicationName = appName;
   vkAppInfo.applicationVersion =
     VK_MAKE_VERSION(appVersion[0], appVersion[1], appVersion[2]);
   pVkAllocCallback = NULL;

   CanvasManager = new ctGfxCanvasManagerVk();
}

ctGfxCoreVk::~ctGfxCoreVk() {
   Shutdown();
}

ctResults ctGfxCoreVk::LoadConfig(ctJSONReader::Entry& json) {
   return CT_SUCCESS;
}

ctResults ctGfxCoreVk::SaveConfig(ctJSONWriter& writer) {
   return CT_SUCCESS;
}

ctResults ctGfxCoreVk::Startup(ctEngineCore* pEngine) {
   Engine = pEngine;
   Engine->Debug->Log("Starting Vulkan Backend...");
   /* Create Instance */
   {
      if (validationEnabled && !isValidationLayersAvailible()) {
         ctFatalError(-1,
                      "Vulkan Validation layers requested but not avalible");
      }

      unsigned int sdlExtCount;
      unsigned int extraExtCount = 1;
      if (!SDL_Vulkan_GetInstanceExtensions(NULL, &sdlExtCount, NULL)) {
         ctFatalError(-1,
                      "SDL_Vulkan_GetInstanceExtensions() Failed to get "
                      "instance extensions");
      }
      instanceExtensions.Resize(sdlExtCount + extraExtCount);
      instanceExtensions[0] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
      SDL_Vulkan_GetInstanceExtensions(
        NULL, &sdlExtCount, &instanceExtensions[extraExtCount]);
      for (uint32_t i = 0; i < sdlExtCount + extraExtCount; i++)
         Engine->Debug->Log("VK Extension: \"%s\" found",
                            instanceExtensions[i]);

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
        "vkCreateInstance() Failed to create vulkan instance");
   }
   /* Initialize a first surface to check for support */
   {

   } /* Pick a GPU */
   {
      uint32_t gpuCount;
      CT_VK_CHECK(vkEnumeratePhysicalDevices(vkInstance, &gpuCount, NULL),
                  "Failed to find devices with vkEnumeratePhysicalDevices()");
      if (!gpuCount) {
         ctFatalError(-1, "No Supported Vulkan Compatible GPU found!");
      }
      ctDynamicArray<VkPhysicalDevice> gpus;
      gpus.Resize(gpuCount);
      vkEnumeratePhysicalDevices(vkInstance, &gpuCount, gpus.Data());
      if (preferredDevice >= 0 && preferredDevice < (int32_t)gpuCount) {
         vkPhysicalDevice = gpus[preferredDevice];
      } else {
         vkPhysicalDevice = vPickBestDevice(gpus.Data(), gpuCount);
         if (vkPhysicalDevice == VK_NULL_HANDLE)
            ctFatalError(-1,
                         "Could not automatically find suitable graphics card");
      }
   }

   return CT_SUCCESS;
}

ctResults ctGfxCoreVk::Shutdown() {
   return CT_SUCCESS;
}

ctResults ctGfxCoreVk::RenderFrame() {
   return CT_SUCCESS;
}

ctGfxBackend ctGfxCoreVk::GetBackendId() {
   return CT_GFX_VULKAN;
}

void ctGfxCoreVk::AddWindowEvent(const SDL_WindowEvent* event) {
}

ctGfxCanvasBase* ctGfxCanvasManagerBase::CreateWindowCanvas(ctWindow* pWindow) {
   ctGfxCanvasVk* canvas = new ctGfxCanvasVk(pWindow);
   Canvases.Append(canvas);
   return canvas;
}

ctGfxCanvasVk::ctGfxCanvasVk(ctWindow* pWindow) {
   this->pWindow = pWindow;
   this->pTexture = NULL;
}

ctGfxCanvasVk::ctGfxCanvasVk(ctGfxTextureBase* pTexture) {
   this->pWindow = NULL;
   this->pTexture = pTexture;
}

ctGfxBackend ctGfxCanvasVk::GetBackendId() {
   return CT_GFX_VULKAN;
}

ctResults ctGfxCanvasVk::Resize(uint32_t width, uint32_t height) {
   return CT_SUCCESS;
}

ctResults ctGfxCanvasVk::CreateSwapchain() {
   return CT_SUCCESS;
}

ctGfxBackend ctGfxCanvasManagerVk::GetBackendId() {
   return CT_GFX_VULKAN;
}
