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

#include "KinnowSceneEngine.hpp"
#include "core/EngineCore.hpp"

#include "middleware/ImguiIntegration.hpp"
#include "renderer/KeyLimeRenderer.hpp"
#include "core/WindowManager.hpp"
#include "core/Application.hpp"

ctResults ctKinnowSceneEngine::Startup() {
   return ctResults();
}

ctResults ctKinnowSceneEngine::Shutdown() {
   return ctResults();
}

ctResults ctKinnowSceneEngine::OnNextFrame(double deltaTime) {
   Engine->App->OnTick((float)deltaTime);
   PushCameraToRenderer();
   return CT_SUCCESS;
}

void ctKinnowSceneEngine::PushCameraToRenderer() {
   int32_t width, height;
   Engine->WindowManager->GetMainWindowDrawableSize(&width, &height);
   mainCamera.aspectRatio = (float)width / (float)height;
   Engine->Renderer->UpdateCamera(mainCamera);
}
