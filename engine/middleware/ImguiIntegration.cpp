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

#include "ImguiIntegration.hpp"

#include "imgui/backends/imgui_impl_sdl.h"
#include "../core/EngineCore.hpp"

void processImguiEvent(SDL_Event* event, void* pData) {
   ctImguiIntegration* pIntegration = (ctImguiIntegration*)pData;
   ImGui_ImplSDL2_ProcessEvent(event);
}

ctResults ctImguiIntegration::Startup() {
   ctDebugLog("Starting DearImgui...");
   ImGui::CreateContext();
#ifdef CITRUS_GFX_VULKAN
   ImGui_ImplSDL2_InitForVulkan(Engine->WindowManager->mainWindow.pSDLWindow);
#endif
   Engine->OSEventManager->MiscEventHandlers.Append({processImguiEvent, this});
   Engine->OSEventManager->WindowEventHandlers.Append({processImguiEvent, this});
   return CT_SUCCESS;
}

ctResults ctImguiIntegration::Shutdown() {
   ImGui_ImplSDL2_Shutdown();
   ImGui::DestroyContext();
   return CT_SUCCESS;
}

ctResults ctImguiIntegration::NextFrame() {
   ImGui_ImplSDL2_NewFrame(Engine->WindowManager->mainWindow.pSDLWindow);
   ImGui::NewFrame();
   return CT_SUCCESS;
}
