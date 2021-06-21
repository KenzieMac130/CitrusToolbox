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

ctResults ctHoneybellSceneEngine::Startup() {
   CurrentCamera.fov = 0.785f;
   CurrentCamera.position = {0.0f, 0.0f, -5.0f};
   lookAt = {0.0f, 0.0f, 0.0f};
   return ctResults();
}

ctResults ctHoneybellSceneEngine::Shutdown() {
   return ctResults();
}

ctResults ctHoneybellSceneEngine::NextFrame() {
   /* Todo: Move to it's own object when scene engine is developed */
   /* Temporary Debug Camera */
   {
      ctCameraInfo debugCamera = CurrentCamera;
      ImGui::Begin("Debug Camera");
      ImGui::DragFloat("Fov", &debugCamera.fov, 0.01f, 0.01f, CT_PI / 2.0f);
      ImGui::InputFloat3("Position", debugCamera.position.data);
      ImGui::InputFloat3("LookAt", lookAt.data);
      ImGui::End();

      ctVec3 fwd = CT_VEC3_FORWARD;
      ctVec3 up = CT_VEC3_UP;
      glm_quat_for(lookAt.data, fwd.data, up.data, debugCamera.rotation.data);
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
