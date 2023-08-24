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

#include "SceneEngine.hpp"
#include "core/EngineCore.hpp"

#include "middleware/ImguiIntegration.hpp"
#include "middleware/Im3dIntegration.hpp"
#include "renderer/KeyLimeRenderer.hpp"
#include "core/WindowManager.hpp"
#include "core/Application.hpp"

ctResults ctSceneEngine::Startup() {
   return ctResults();
}

ctResults ctSceneEngine::Shutdown() {
   return ctResults();
}

ctResults ctSceneEngine::NextFrame(double deltaTime) {
   Engine->App->OnTick((float)deltaTime);

   /* debug camera */
   if (debugCameraEnabled) {
      debugCamera.FrameUpdate((float)deltaTime);
      mainCamera = debugCamera.camera;
   }
   PushCameraToRenderer();
   PushAndResetCursor();
   return CT_SUCCESS;
}

void ctSceneEngine::EnableDebugCamera() {
   debugCameraEnabled = true;
   debugCamera.camera = mainCamera;
}

void ctSceneEngine::DisableDebugCamera() {
   debugCameraEnabled = false;
}

void ctSceneEngine::UpdateCursor() {
   ctVec2 mousePosition = ctVec2(); /* todo: get from audition */
   cursorDirection = normalize(mainCamera.ScreenToWorld(mousePosition));
}

void ctSceneEngine::PushCameraToRenderer() {
   int32_t width, height;
   Engine->WindowManager->GetMainWindowDrawableSize(&width, &height);
   mainCamera.aspectRatio = (float)width / (float)height;
   Engine->Renderer->UpdateCamera(mainCamera);
}

void ctSceneEngine::PushAndResetCursor() {
   Engine->Im3dIntegration->SetSelection(mainCamera.position, cursorDirection);
   cursorDirection = ctVec3(0.0f); /* reset cursor each frame */
}

const char* ctSceneEngine::GetModuleName() {
   return "Scene Engine";
}
