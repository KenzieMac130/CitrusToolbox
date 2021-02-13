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

#include "renderer/lowlevel/GfxBase.hpp"
#include "utilities/Common.h"

#include "vulkan/vulkan.h"
#include "SDL_vulkan.h"

class ctGfxCanvasVk : public ctGfxCanvasBase {
public:
   ctGfxCanvasVk(ctWindow* pWindow);
   ctGfxCanvasVk(ctGfxTextureBase* pTexture);

   ctGfxBackend GetBackendId() final;

   ctResults Resize(uint32_t width, uint32_t height) final;

   ctResults CreateSwapchain();
};

class ctGfxCanvasManagerVk : public ctGfxCanvasManagerBase {
public:
    ctGfxBackend GetBackendId() final;

    void AddWindowEvent(const SDL_WindowEvent* event) final;

    /* Canvas */
    ctGfxCanvasBase* CreateWindowCanvas(ctWindow* pWindow) final;
    ctResults DestroyWindowCanvas(ctGfxCanvasBase* pCanvas) final;
};

struct ctVkQueueFamilyIndices {
   uint32_t graphicsIdx;
   uint32_t presentIdx;
   uint32_t computeIdx;
   uint32_t transferIdx;
};

class ctGfxCoreVk : public ctGfxCoreBase {
public:
   ctGfxCoreVk(bool validate, const char* appName, int appVersion[3]);
   ~ctGfxCoreVk();

   ctResults LoadConfig(ctJSONReader::Entry& json) final;
   ctResults SaveConfig(ctJSONWriter& writer) final;
   ctResults Startup(ctEngineCore* pEngine) final;
   ctResults Shutdown() final;

   ctResults RenderFrame() final;
   ctGfxBackend GetBackendId() final;

   /* User Config */
   bool validationEnabled;
   int32_t preferredDevice;

   /* Validation */
   bool isValidationLayersAvailible();
   VkAllocationCallbacks* pVkAllocCallback;
   ctDynamicArray<const char*> validationLayers;
   ctDynamicArray<const char*> instanceExtensions;
   VkApplicationInfo vkAppInfo;
   VkInstance vkInstance;
   VkPhysicalDevice vkPhysicalDevice;
   VkDevice vkDevice;
};