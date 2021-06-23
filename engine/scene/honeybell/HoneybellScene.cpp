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

#include "HoneybellScene.hpp"
#include "imgui/imgui.h"

#include "core/EngineCore.hpp"

ctResults ctHoneybellSceneEngine::Startup() {
   CurrentCamera.fov = 0.785f;
   CurrentCamera.position = {0.0f, 0.0f, -5.0f};
   return ctResults();
}

ctResults ctHoneybellSceneEngine::Shutdown() {
   return ctResults();
}

ctResults ctHoneybellSceneEngine::NextFrame() {
   /* Todo: Move to it's own object when scene engine is developed */
   /* Temporary Debug Camera */
   float deltaTime = Engine->FrameTime.GetDeltaTimeFloat();
   {
      ctCameraInfo debugCamera = CurrentCamera;
      ImGui::Begin("Debug Camera");
      ImGui::DragFloat("Fov", &debugCamera.fov, 0.01f, 0.01f, CT_PI / 2.0f);
      ImGui::InputFloat3("Position", debugCamera.position.data);
      ImGui::DragFloat("Yaw", &camYaw, 0.01f, -CT_PI / 2.0f, CT_PI / 2.0f);
      ImGui::DragFloat("Pitch", &camPitch, 0.01f, -CT_PI / 2.0f, CT_PI / 2.0f);
      ImGui::End();

      float speed = 1.0f * deltaTime;

      /* Look */
      if (Engine->Interact->GetSignal(ctInteractPath("/dev/mouse/input/button/right"))) {
         float horizontalMove = Engine->Interact->GetSignal(
           ctInteractPath("/dev/mouse/input/relative_move/x"));
         float verticalMove = Engine->Interact->GetSignal(
           ctInteractPath("/dev/mouse/input/relative_move/y"));
         camYaw += horizontalMove * 2.0f;
         camPitch += verticalMove * 2.0f;
         if (camPitch < -CT_PI / 2.0f + 0.05f) { camPitch = -CT_PI / 2.0f + 0.05f; }
         if (camPitch > CT_PI / 2.0f - 0.05f) { camPitch = CT_PI / 2.0f - 0.05f; }
      }
      debugCamera.rotation = ctQuat(CT_VEC3_UP, camYaw) * ctQuat(CT_VEC3_RIGHT, camPitch);

      /* Speedup */
      if (Engine->Interact->GetSignal(
            ctInteractPath("/dev/keyboard/input/scancode/225"))) {
         speed *= 4.0f;
      }

      /* Forward */
      if (Engine->Interact->GetSignal(
            ctInteractPath("/dev/keyboard/input/scancode/26"))) {
         debugCamera.position =
           debugCamera.position + (debugCamera.rotation.getForward() * speed);
      }
      /* Back */
      if (Engine->Interact->GetSignal(
            ctInteractPath("/dev/keyboard/input/scancode/22"))) {
         debugCamera.position =
           debugCamera.position + (debugCamera.rotation.getBack() * speed);
      }
      /* Left */
      if (Engine->Interact->GetSignal(ctInteractPath("/dev/keyboard/input/scancode/4"))) {
         debugCamera.position =
           debugCamera.position + (debugCamera.rotation.getLeft() * speed);
      }
      /* Right */
      if (Engine->Interact->GetSignal(ctInteractPath("/dev/keyboard/input/scancode/7"))) {
         debugCamera.position =
           debugCamera.position + (debugCamera.rotation.getRight() * speed);
      }
      /* Up */
      if (Engine->Interact->GetSignal(ctInteractPath("/dev/keyboard/input/scancode/8"))) {
         debugCamera.position =
           debugCamera.position + (debugCamera.rotation.getUp() * speed);
      }
      /* Down */
      if (Engine->Interact->GetSignal(
            ctInteractPath("/dev/keyboard/input/scancode/20"))) {
         debugCamera.position =
           debugCamera.position + (debugCamera.rotation.getDown() * speed);
      }

      SetCameraInfo(debugCamera);
   }

   LastCamera = CurrentCamera;
   return CT_SUCCESS;
}

void ctHoneybellSceneEngine::SetCameraInfo(ctCameraInfo camera, const char* cameraId) {
   CurrentCamera = camera;
}

ctCameraInfo ctHoneybellSceneEngine::GetCameraInfo(const char* cameraId) {
   return CurrentCamera;
}

ctCameraInfo ctHoneybellSceneEngine::GetCameraInfoLastFrame(const char* cameraId) {
   return LastCamera;
}
