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

#include "core/Application.hpp"
#include "scene/SceneEngineBase.hpp"
#include "interact/InteractionEngine.hpp"
#include "middleware/ImguiIntegration.hpp"
#include "middleware/Im3dIntegration.hpp"
#include "scene/kinnow/KinnowSceneEngine.hpp"

#include "formats/model/Model.hpp"

class ModelViewer {
public:
   void UpdateUI();
   ctResults LoadModel(ctFile file);

private:
   ctModel model;
};

class TestApp : public ctApplication {
   virtual const char* GetAppName();
   virtual const char* GetAppDeveloperName();
   virtual ctAppVersion GetAppVersion();
   virtual ctResults OnStartup();
   virtual ctResults OnTick(const float deltatime);
   virtual ctResults OnUIUpdate();
   virtual ctResults OnShutdown();

   ModelViewer viewer;
   ctKinnowSceneEngine* Scene;
   float cameraDistance = 12.0f;
   float cameraSpeed = 1.0f;
   float cameraHeight = 2.0f;
   float cameraPhase = 0.0f;
   ctCameraInfo camera;
};

const char* TestApp::GetAppName() {
   return "ModelTest";
}

const char* TestApp::GetAppDeveloperName() {
   return "CitrusToolbox";
}

ctAppVersion TestApp::GetAppVersion() {
   return {1, 0, 0};
}

ctResults TestApp::OnStartup() {
   Scene = (ctKinnowSceneEngine*)Engine->SceneEngine;
   Scene->EnableCameraOverride();

   /* load known model */

   return CT_SUCCESS;
}

ctResults TestApp::OnTick(const float deltatime) {
   cameraPhase += cameraSpeed * deltatime;

   camera.position = CT_VEC3_FROM_2D_GROUND(
     ctVec2(ctSin(cameraPhase) * cameraDistance, ctCos(cameraPhase) * cameraDistance));
   camera.position.CT_AXIS_UP = cameraHeight;
   ctVec3 vertpos = ctVec3(camera.position.x, 0.0f, camera.position.z);
   camera.rotation = ctQuatLookTowards(normalize(-vertpos));
   Scene->SetCameraOverride(camera);
   return CT_SUCCESS;
}

ctResults TestApp::OnUIUpdate() {
   Im3d::PushLayerId(CT_IM3D_LAYER_XRAY);
   Im3d::BeginTriangles();
   Im3d::SetColor(ctVec4ToIm3dColor(CT_COLOR_RED));
   Im3d::Vertex(ctVec3ToIm3d(ctVec3(-0.5f, -0.5f, 0.0f)));
   Im3d::SetColor(ctVec4ToIm3dColor(CT_COLOR_GREEN));
   Im3d::Vertex(ctVec3ToIm3d(ctVec3(0.5f, -0.5f, 0.0f)));
   Im3d::SetColor(ctVec4ToIm3dColor(CT_COLOR_BLUE));
   Im3d::Vertex(ctVec3ToIm3d(ctVec3(0.0f, 0.5f, 0.0f)));
   Im3d::End();
   Im3d::PopLayerId();

   Im3d::SetColor(ctVec4ToIm3dColor(CT_COLOR_PINK));
   Im3d::DrawAlignedBoxFilled(Im3d::Vec3(-1.0f), Im3d::Vec3(1.0f));

   Im3d::SetColor(ctVec4ToIm3dColor(CT_COLOR_WHITE));
   Im3d::Text(Im3d::Vec3(0.0f), Im3d::TextFlags_Default, "Hello World!!!");

   if (ImGui::Begin("Quick Camera")) {
      ImGui::DragFloat("Camera Distance", &cameraDistance);
      ImGui::DragFloat("Camera Speed", &cameraSpeed);
      ImGui::DragFloat("Camera Height", &cameraHeight);
      viewer.UpdateUI();
   }

   ImGui::End();

   return CT_SUCCESS;
}

ctResults TestApp::OnShutdown() {
   return CT_SUCCESS;
}

int main(int argc, char* argv[]) {
   TestApp* pApplication = new TestApp();
   return pApplication->Execute(argc, argv);
}

/* ---------------------------- Im3d Model Viewer ---------------------------- */

void ModelViewer::UpdateUI() {
}

ctResults ModelViewer::LoadModel(ctFile file) {
   return ctModelLoad(model, &file);
}
