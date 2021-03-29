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
#include "TracyVulkan.hpp"
#endif

#include "core/EngineCore.hpp"

#define CT_VK_CHECK(_args, _msg)                                               \
   {                                                                           \
      VkResult _tmpvresult = _args;                                            \
      if (_tmpvresult != VK_SUCCESS) {                                         \
         ctFatalError((int)_tmpvresult, #_msg);                                \
      }                                                                        \
   }

struct ctVkQueueFamilyIndices {
   uint32_t graphicsIdx;
   uint32_t presentIdx;
   uint32_t computeIdx;
   uint32_t transferIdx;
};

class ctVkScreenResources {
public:
    VkSurfaceKHR surface;

    ctResults Create(class ctVkBackend* pBackend, SDL_Window* pWindow);
    ctResults Destroy(class ctVkBackend* pBackend);
};

class ctVkBackend : public ctModuleBase {
public:
   ctVkBackend();

   ctResults Startup() final;
   ctResults Shutdown() final;

   bool isValidationLayersAvailible();
   VkPhysicalDevice PickBestDevice(VkPhysicalDevice* pGpus, uint32_t count);
   ctVkQueueFamilyIndices FindQueueFamilyIndices(VkPhysicalDevice gpu);

   /* Vulkan Objects */
   VkAllocationCallbacks vkAllocCallback;
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

   ctVkScreenResources mainScreenResources;

   /* Settings */
   int32_t preferredDevice;
   int32_t validationEnabled;
};