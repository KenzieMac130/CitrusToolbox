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

#include "utilities/Common.h"
#include "renderer/Renderer.hpp"
#include "core/EngineCore.hpp"
#include "renderer/vulkan/VkBackend.hpp"

ctResults ctRenderer::Startup() {
   ZoneScoped;
   vkBackend = new ctVkBackend();

   ctSettingsSection* vkSettings =
     Engine->Settings->CreateSection("VulkanBackend", 2);
   vkSettings->BindInteger(
     &vkBackend->preferredDevice,
     false,
     true,
     "PreferredDevice",
     "Index of the preferred device to use for rendering.");
   vkSettings->BindInteger(&vkBackend->validationEnabled,
                           false,
                           true,
                           "ValidationEnabled",
                           "Use validation layers for debug testing.");

   /* Check for power usage */
   int percent;
   SDL_PowerState power = SDL_GetPowerInfo(NULL, &percent);
   if (power == SDL_POWERSTATE_ON_BATTERY) {
      ctDebugWarning("!!!USER IS ON BATTERY!!! Expect performance issues!");
      if (percent <= 5) {
         Engine->WindowManager->ShowErrorMessage(
           "Battery Warning!", CT_NC("Battery is almost dead! Plug in ASAP!"));
      }
   }

   return vkBackend->ModuleStartup(Engine);
};

ctResults ctRenderer::Shutdown() {
   const ctResults results = vkBackend->ModuleShutdown();
   delete vkBackend;
   return results;
};