#include "VkKeyLimeCore.hpp"
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

void sendResizeSignal(SDL_Event* event, void* pData) {
   ctVkKeyLimeCore* pCore = (ctVkKeyLimeCore*)pData;
   if (SDL_GetWindowFromID(event->window.windowID) ==
       pCore->vkBackend.mainScreenResources.window) {
      if (event->window.event == SDL_WINDOWEVENT_RESIZED) {
         ctDebugLog("Window Resize: %d:%dx%d",
                    event->window.windowID,
                    event->window.data1,
                    event->window.data2);
         pCore->vkBackend.mainScreenResources.resizeTriggered = true;
      }
   }
}

ctResults ctVkKeyLimeCore::Startup() {
   ZoneScoped;
   vkBackend.preferredDevice = -1;
   vkBackend.vsync = Engine->WindowManager->mainWindowVSync;
#ifndef NDEBUG
   vkBackend.validationEnabled = 1;
#else
   vkBackend.validationEnabled = 0;
#endif
   vkBackend.maxSamplers = CT_MAX_GFX_SAMPLERS;
   vkBackend.maxSampledImages = CT_MAX_GFX_SAMPLED_IMAGES;
   vkBackend.maxStorageImages = CT_MAX_GFX_STORAGE_IMAGES;
   vkBackend.maxStorageBuffers = CT_MAX_GFX_STORAGE_BUFFERS;
   vkBackend.maxGraphicsCommandBuffers = CT_MAX_GFX_GRAPHICS_COMMAND_BUFFERS;
   vkBackend.maxComputeCommandBuffers = CT_MAX_GFX_COMPUTE_COMMAND_BUFFERS;
   vkBackend.maxTransferCommandBuffers = CT_MAX_GFX_TRANSFER_COMMAND_BUFFERS;

   ctSettingsSection* vkSettings = Engine->Settings->CreateSection("VulkanBackend", 32);
   vkSettings->BindInteger(&vkBackend.preferredDevice,
                           false,
                           true,
                           "PreferredDevice",
                           "Index of the preferred device to use for rendering.");
   vkSettings->BindInteger(&vkBackend.validationEnabled,
                           false,
                           true,
                           "ValidationEnabled",
                           "Use validation layers for debug testing.",
                           CT_SETTINGS_BOUNDS_BOOL);

   vkSettings->BindInteger(&vkBackend.maxSamplers,
                           false,
                           true,
                           "MaxSamplers",
                           "Max number of samplers.",
                           CT_SETTINGS_BOUNDS_UINT);
   vkSettings->BindInteger(&vkBackend.maxSampledImages,
                           false,
                           true,
                           "MaxSampledImages",
                           "Max number of sampled images.",
                           CT_SETTINGS_BOUNDS_UINT);
   vkSettings->BindInteger(&vkBackend.maxStorageImages,
                           false,
                           true,
                           "MaxStorageImages",
                           "Max number of storage images.",
                           CT_SETTINGS_BOUNDS_UINT);
   vkSettings->BindInteger(&vkBackend.maxStorageBuffers,
                           false,
                           true,
                           "MaxStorageBuffers",
                           "Max number of storage buffers.",
                           CT_SETTINGS_BOUNDS_UINT);

   vkSettings->BindInteger(&vkBackend.maxGraphicsCommandBuffers,
                           false,
                           true,
                           "MaxGraphicsCommandBuffers",
                           "Max number of graphics command buffers per-frame.",
                           CT_SETTINGS_BOUNDS_UINT);
   vkSettings->BindInteger(&vkBackend.maxComputeCommandBuffers,
                           false,
                           true,
                           "MaxComputeCommandBuffers",
                           "Max number of compute command buffers per-frame.",
                           CT_SETTINGS_BOUNDS_UINT);
   vkSettings->BindInteger(&vkBackend.maxTransferCommandBuffers,
                           false,
                           true,
                           "MaxTransferCommandBuffers",
                           "Max number of transfer command buffers per-frame.",
                           CT_SETTINGS_BOUNDS_UINT);

   ctSettingsSection* settings = Engine->Settings->CreateSection("KeyLimeRenderer", 32);
   Engine->OSEventManager->WindowEventHandlers.Append({sendResizeSignal, this});

   return vkBackend.ModuleStartup(Engine);
}

ctResults ctVkKeyLimeCore::Shutdown() {
   return vkBackend.ModuleShutdown();
}

ctResults ctVkKeyLimeCore::Render() {
   return CT_SUCCESS;
}
