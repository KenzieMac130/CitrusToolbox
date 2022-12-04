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

ctResults ctKinnowSceneEngine::Startup() {
   return ctResults();
}

ctResults ctKinnowSceneEngine::Shutdown() {
   return ctResults();
}

ctResults ctKinnowSceneEngine::OnNextFrame(double deltaTime) {
   rotationPhase += (float)deltaTime * rotationSpeed;
   /* todo: removeme!!! */
   if (ImGui::Begin("Debug Camera")) {
      ImGui::DragFloat3("Camera Position", cameraPos.data, 0.01f);
      ImGui::SliderFloat("FOV", &mainCamera.fov, 0.01f, CT_PI);
      ImGui::SliderFloat("Speed", &rotationSpeed, 0.0f, CT_PI);
      ImGui::DragFloat("rotationDistance", &rotationDistance);
      ImGui::End();

      cameraPos.x = ctSin(rotationPhase) * -rotationDistance;
      cameraPos.z = ctCos(rotationPhase) * -rotationDistance;
      mainCamera.position = cameraPos;
      mainCamera.rotation = ctQuat(CT_VEC3_UP, rotationPhase);

      int32_t width, height;
      Engine->WindowManager->GetMainWindowDrawableSize(&width, &height);
      mainCamera.aspectRatio = (float)width / (float)height;
      Engine->Renderer->UpdateCamera(mainCamera);
   }
   return CT_SUCCESS;
}