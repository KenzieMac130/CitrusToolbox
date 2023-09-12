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
#include "physics/Module.hpp"

ctResults ctSceneEngine::Startup() {
   return ctResults();
}

ctResults ctSceneEngine::Shutdown() {
   return ctResults();
}

ctResults ctSceneEngine::NextFrame(double deltaTime) {
   /* update the time accumulator */
   timeAccumulator += deltaTime;
   int32_t tickNumber = 0;
   float frameTime = 0.0f;
   while (timeAccumulator >= timeStep) {
      ctStopwatch tickTimer = ctStopwatch();

      /* update subsystems */
      Engine->App->OnTick((float)timeStep);
      ctPhysicsEngineUpdate(Engine->Physics->GetPhysicsEngine(), timeStep);

      /* finalize tick loop info */
      tickTimer.NextLap();
      frameTime += tickTimer.GetDeltaTimeFloat();
      if (frameTime > maxFrameTime) {
         ctDebugWarning("TICK LOOP EXCEEDED %f SECONDS, MAX IS %f, BAILING OUT!",
                        frameTime,
                        maxFrameTime);
         timeAccumulator = 0.0f;
      } else {
         timeAccumulator -= timeStep;
      }
      tickNumber++;
   }

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
