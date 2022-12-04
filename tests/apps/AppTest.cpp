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
#include "core/Translation.hpp"
#include "middleware/Im3dIntegration.hpp"
#include "middleware/ImguiIntegration.hpp"
#include "core/FileSystem.hpp"
#include "interact/InteractionEngine.hpp"
#include "renderer/KeyLimeRenderer.hpp"
#include "core/AsyncTasks.hpp"
#include "core/JobSystem.hpp"

class TestApp : public ctApplication {
   virtual const char* GetAppName();
   virtual const char* GetAppDeveloperName();
   virtual ctAppVersion GetAppVersion();
   virtual ctResults OnStartup();
   virtual ctResults OnTick(const float deltatime);
   virtual ctResults OnUIUpdate();
   virtual ctResults OnShutdown();

   float rad = 5.0f;
   float phase = 0;
   float diskPos[2] = {0};

   int gausPts = 0;
   int gausSeed = 0;

   int spherePts = 0;
   int sphereSeed = 0;
   float sphereRad = 1.0f;

   ctAsyncTaskHandle tasks[32];
};

const char* TestApp::GetAppName() {
   return "AppTest";
}

const char* TestApp::GetAppDeveloperName() {
   return "CitrusToolbox";
}

ctAppVersion TestApp::GetAppVersion() {
   return {1, 0, 0};
}

ctResults testTask(void*) {
   ZoneScoped;
   ctDebugLog("Testing Async...");
   ctWait(500);
   return CT_SUCCESS;
}

void testJob(void*) {
   ZoneScoped;
   for (int i = 0; i < 10000; i++) {
      float j = 23.0f * i;
   }
}

ctResults TestApp::OnStartup() {
   ctDebugLog(CT_NC("Translation Test"));
   ctDebugLog("---------------------- Dumping paths ----------------------");
   Engine->Interact->Directory.LogContents();
   ctDebugLog("-----------------------------------------------------------");
   /* Test Geometry Load */
   {} /* Test Texture Load */
   {}

   for (int i = 31; i > 0; i--) {
      ctStringUtf8 str = ctStringUtf8();
      str.Printf(32, "Test Task: %d", i);
      tasks[i] = ctGetAsyncManager()->CreateTask(str.CStr(), testTask, NULL, i);
   }

   return CT_SUCCESS;
}

int loopvar = 0;
ctResults TestApp::OnTick(const float deltatime) {
   ZoneScoped;
   /*if (loopvar <= 5000) {
      Engine->Debug->Log("This Message %d", loopvar);
      loopvar++;
      if (loopvar == 5001) { ctDebugLog("Finished"); }
   }*/
   return CT_SUCCESS;
}

void (*pfpFunction[1024])(void*);
void* datas[1024];

ctResults TestApp::OnUIUpdate() {
   ImGui::ShowDemoWindow();
   if (ImGui::Begin("Interact")) { Engine->Interact->DebugImGui(); }
   ImGui::End();

   if (ImGui::Begin("Async")) {
      Engine->JobSystem->DebugImGui();
      Engine->AsyncTasks->DebugImGui();
   }
   ImGui::End();

   phase += Engine->FrameTime.GetDeltaTimeFloat();
   diskPos[0] = ctSin(phase) * rad;
   diskPos[1] = ctCos(phase) * rad;

   for (int i = 0; i < 1024; i++) {
      pfpFunction[i] = testJob;
      datas[i] = NULL;
   }

   ctGetJobSystem()->PushJobs(1024, pfpFunction, datas);
   ctGetJobSystem()->WaitBarrier();

   // clang-format off
   //Im3d::Text(Im3d::Vec3(0, 1, 0), 1.0f, Im3d::Color_Red, Im3d::TextFlags_Default, "Red");
   //Im3d::Text(Im3d::Vec3(1, -1, 0), 1.0f, Im3d::Color_Green, Im3d::TextFlags_Default, "Green");
   //Im3d::Text(Im3d::Vec3(-1, -1, 0), 1.0f, Im3d::Color_Blue, Im3d::TextFlags_Default, "Blue");

   //Im3d::Text(Im3d::Vec3(-1, -1, 1), 2.0f, Im3d::Color_White, Im3d::TextFlags_Default, "0");
   //Im3d::Text(Im3d::Vec3(1, -1, 1), 2.0f, Im3d::Color_White, Im3d::TextFlags_Default, "1");
   //Im3d::Text(Im3d::Vec3(-1, -1, -1), 2.0f, Im3d::Color_White, Im3d::TextFlags_Default, "2");
   //Im3d::Text(Im3d::Vec3(1, -1, -1), 2.0f, Im3d::Color_White, Im3d::TextFlags_Default, "3");
   //Im3d::Text(Im3d::Vec3(1, 1, 1), 2.0f, Im3d::Color_White, Im3d::TextFlags_Default, "4");
   //Im3d::Text(Im3d::Vec3(-1, 1, 1), 2.0f, Im3d::Color_White, Im3d::TextFlags_Default, "5");
   //Im3d::Text(Im3d::Vec3(1, 1, -1), 2.0f, Im3d::Color_White, Im3d::TextFlags_Default, "6");
   //Im3d::Text(Im3d::Vec3(-1, 1, -1), 2.0f, Im3d::Color_White, Im3d::TextFlags_Default, "7");

   //Im3d::PushColor(Im3d::Color_Blue);
   //Im3d::PushAlpha(1.0f);
   //Im3d::DrawCircleFilled(Im3d::Vec3(diskPos[0], 0, diskPos[1]), Im3d::Vec3(0, 1, 0), 1.0f);
   //Im3d::DrawCircle(Im3d::Vec3(-diskPos[0], 0, -diskPos[1]), Im3d::Vec3(0, 1, 0), 1.0f);
   //Im3d::PopAlpha();
   //Im3d::PopColor();

   //Im3d::PushColor(Im3d::Color_Red);
   //Im3d::PushAlpha(1.0f);
   //Im3d::DrawAlignedBoxFilled(Im3d::Vec3(-0.5f, -0.5f, -0.5f), Im3d::Vec3(0.5f, 0.5f, 0.5f));
   //Im3d::DrawPoint(Im3d::Vec3(-5.0, 0, 0), 16.0f, Im3d::Color_Green);
   //Im3d::DrawPoint(Im3d::Vec3(-5.0, 1, 0), 16.0f, Im3d::Color_Green);
   //Im3d::DrawPoint(Im3d::Vec3(-5.0, 2, 0), 16.0f, Im3d::Color_Green);
   //Im3d::DrawPoint(Im3d::Vec3(-5.0, 3, 0), 16.0f, Im3d::Color_Green);
   //Im3d::PopAlpha();
   //Im3d::PopColor();

   //Im3d::PushColor(Im3d::Color_Red);
   //Im3d::PushAlpha(1.0f);
   //Im3d::PushSize(16.0f);
   //Im3d::PushMatrix();
   //Im3d::Translate(Im3d::Vec3(5.0, 0, 0));
   //Im3d::DrawXyzAxes();
   //Im3d::PopMatrix();
   //Im3d::PopSize();
   //Im3d::PopAlpha();
   //Im3d::PopColor();
   // clang-format on

   if (ImGui::Begin("3D Math")) {
      ImGui::DragInt("Gaussian Points", &gausPts, 1.0f, 0, 4096);
      ImGui::DragInt("Gaussian Seed", &gausSeed);
      ImGui::Separator();
      ImGui::DragInt("Sphere Points", &spherePts, 1.0f, 0, 4096);
      ImGui::DragFloat("Sphere Radius", &sphereRad, 0.05f, 0.0f);
      ImGui::DragInt("Sphere Seed", &sphereSeed);
   }
   ImGui::End();

   Im3d::PushColor(Im3d::Color_Pink);
   Im3d::PushAlpha(0.9f);
   Im3d::DrawAlignedBox(Im3d::Vec3(-1.0f), Im3d::Vec3(1.0f));
   Im3d::PopAlpha();
   Im3d::PopColor();

   Im3d::PushLayerId(CT_IM3D_LAYER_XRAY);
   Im3d::BeginTriangles();
   Im3d::Vertex(Im3d::Vec3(-0.5f, -0.5f, 0.0f), Im3d::Color_Red);
   Im3d::Vertex(Im3d::Vec3(0.5f, -0.5f, 0.0f), Im3d::Color_Green);
   Im3d::Vertex(Im3d::Vec3(0.0f, 0.5f, 0.0f), Im3d::Color_Blue);
   Im3d::End();
   Im3d::SetColor(Im3d::Color_Red);
   Im3d::Text(Im3d::Vec3(-0.5f, -0.5f, 0.0f), Im3d::TextFlags_Default, "Red");
   Im3d::SetColor(Im3d::Color_Green);
   Im3d::Text(Im3d::Vec3(0.5f, -0.5f, 0.0f), Im3d::TextFlags_Default, "Green");
   Im3d::SetColor(Im3d::Color_Blue);
   Im3d::Text(Im3d::Vec3(0.0f, 0.5f, 0.0f), Im3d::TextFlags_Default, "Blue");
   Im3d::PopLayerId();

   ctRandomGenerator rng = ctRandomGenerator(gausSeed);
   for (int i = 0; i < gausPts; i++) {
      ctVec3 pt = CT_VEC3_FROM_2D_GROUND(rng.GetGaussian2D(ctVec2(0.0f), 1));
      Im3d::DrawPoint(ctVec3ToIm3d(pt), 8.0f, ctVec4ToIm3dColor(rng.GetColor()));
   }

   rng.SetSeed(sphereSeed);
   for (int i = 0; i < spherePts; i++) {
      ctVec3 pt = rng.GetOnSphere(sphereRad);
      ctVec4 color = ctVec4(saturate(normalize(pt)), 1.0f);
      Im3d::DrawPoint(ctVec3ToIm3d(pt), 8.0f, ctVec4ToIm3dColor(color));
   }

   return CT_SUCCESS;
}

ctResults TestApp::OnShutdown() {
   return CT_SUCCESS;
}

int main(int argc, char* argv[]) {
   TestApp* pApplication = new TestApp();
   return pApplication->Execute(argc, argv);
}