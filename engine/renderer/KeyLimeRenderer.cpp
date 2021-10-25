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

#include "KeyLimeRenderer.hpp"

#include "imgui/imgui.h"
#include "gpu/Device.h"

#include "core/EngineCore.hpp"
#include "core/Application.hpp"
#include "core/WindowManager.hpp"

ctResults ctKeyLimeRenderer::Startup() {
#if !CITRUS_HEADLESS
   ImGui::GetIO().Fonts->AddFontDefault();
   ImGui::GetIO().Fonts->Build();
   ImGui::GetIO().DisplaySize.x = 640.0f;
   ImGui::GetIO().DisplaySize.y = 480.0f;
   ImGui::GetIO().DeltaTime = 1.0f / 60.0f;
   ImGui::NewFrame();
   ctGPUDeviceCreateInfo deviceCreateInfo = {};
   deviceCreateInfo.appName = Engine->App->GetAppName();
   deviceCreateInfo.version[0] = Engine->App->GetAppVersion().major;
   deviceCreateInfo.version[1] = Engine->App->GetAppVersion().minor;
   deviceCreateInfo.version[2] = Engine->App->GetAppVersion().patch;
   deviceCreateInfo.mainWindowPtr = Engine->WindowManager->mainWindow;
   deviceCreateInfo.fileSystemModulePtr = Engine->FileSystem;
   deviceCreateInfo.useVSync = false;
   deviceCreateInfo.validationEnabled = true;
   ctGPUDeviceStartup(&pGPUDevice, &deviceCreateInfo, NULL);
   Engine->WindowManager->ShowMainWindow();
#endif
   return ctResults();
}

ctResults ctKeyLimeRenderer::Shutdown() {
#if !CITRUS_HEADLESS
   ctGPUDeviceShutdown(pGPUDevice);
#endif
   return ctResults();
}

ctResults ctKeyLimeRenderer::RenderFrame() {
#if !CITRUS_HEADLESS
   ImGui::Render();
#endif
   return ctResults();
}

ctResults ctKeyLimeRenderer::UpdateCamera(const ctCameraInfo& cameraInfo) {
   return ctResults();
}