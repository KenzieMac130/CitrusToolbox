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

#include "core/Application.hpp"
#include "core/Translation.hpp"
#include "im3d/im3d.h"
#include "imgui/imgui.h"

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

ctResults TestApp::OnStartup() {
   ctDebugLog(CT_NC("Translation Test"));
   Engine->FileSystem->MakePreferencesDirectory("Test");
   ctDebugLog("---------------------- Dumping paths ----------------------");
   Engine->Interact->Directory.LogContents();
   ctDebugLog("-----------------------------------------------------------");
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

ctResults TestApp::OnUIUpdate() {
   ImGui::ShowDemoWindow();
   ImGui::Begin("Interact");
   Engine->Interact->DebugImGui();
   ImGui::End();

   phase += Engine->FrameTime.GetDeltaTimeFloat();
   diskPos[0] = ctSin(phase) * rad;
   diskPos[1] = ctCos(phase) * rad;

   // clang-format off
   Im3d::Text(Im3d::Vec3(0, 1, 0), 1.0f, Im3d::Color_Red, Im3d::TextFlags_Default, "Red");
   Im3d::Text(Im3d::Vec3(1, -1, 0), 1.0f, Im3d::Color_Green, Im3d::TextFlags_Default, "Green");
   Im3d::Text(Im3d::Vec3(-1, -1, 0), 1.0f, Im3d::Color_Blue, Im3d::TextFlags_Default, "Blue");

   Im3d::Text(Im3d::Vec3(-1, -1, 1), 2.0f, Im3d::Color_White, Im3d::TextFlags_Default, "0");
   Im3d::Text(Im3d::Vec3(1, -1, 1), 2.0f, Im3d::Color_White, Im3d::TextFlags_Default, "1");
   Im3d::Text(Im3d::Vec3(-1, -1, -1), 2.0f, Im3d::Color_White, Im3d::TextFlags_Default, "2");
   Im3d::Text(Im3d::Vec3(1, -1, -1), 2.0f, Im3d::Color_White, Im3d::TextFlags_Default, "3");
   Im3d::Text(Im3d::Vec3(1, 1, 1), 2.0f, Im3d::Color_White, Im3d::TextFlags_Default, "4");
   Im3d::Text(Im3d::Vec3(-1, 1, 1), 2.0f, Im3d::Color_White, Im3d::TextFlags_Default, "5");
   Im3d::Text(Im3d::Vec3(1, 1, -1), 2.0f, Im3d::Color_White, Im3d::TextFlags_Default, "6");
   Im3d::Text(Im3d::Vec3(-1, 1, -1), 2.0f, Im3d::Color_White, Im3d::TextFlags_Default, "7");

   Im3d::PushColor(Im3d::Color_Blue);
   Im3d::PushAlpha(1.0f);
   Im3d::DrawCircleFilled(Im3d::Vec3(diskPos[0], 0, diskPos[1]), Im3d::Vec3(0, 1, 0), 1.0f);
   Im3d::DrawCircle(Im3d::Vec3(-diskPos[0], 0, -diskPos[1]), Im3d::Vec3(0, 1, 0), 1.0f);
   Im3d::PopAlpha();
   Im3d::PopColor();

   Im3d::PushColor(Im3d::Color_Red);
   Im3d::PushAlpha(1.0f);
   Im3d::DrawAlignedBoxFilled(Im3d::Vec3(-0.5f, -0.5f, -0.5f), Im3d::Vec3(0.5f, 0.5f, 0.5f));
   Im3d::DrawPoint(Im3d::Vec3(-5.0, 0, 0), 16.0f, Im3d::Color_Green);
   Im3d::DrawPoint(Im3d::Vec3(-5.0, 1, 0), 16.0f, Im3d::Color_Green);
   Im3d::DrawPoint(Im3d::Vec3(-5.0, 2, 0), 16.0f, Im3d::Color_Green);
   Im3d::DrawPoint(Im3d::Vec3(-5.0, 3, 0), 16.0f, Im3d::Color_Green);
   Im3d::PopAlpha();
   Im3d::PopColor();

   Im3d::PushColor(Im3d::Color_Red);
   Im3d::PushAlpha(1.0f);
   Im3d::PushSize(16.0f);
   Im3d::PushMatrix();
   Im3d::Translate(Im3d::Vec3(5.0, 0, 0));
   Im3d::DrawXyzAxes();
   Im3d::PopMatrix();
   Im3d::PopSize();
   Im3d::PopAlpha();
   Im3d::PopColor();

   // clang-format on
   return CT_SUCCESS;
}

ctResults TestApp::OnShutdown() {
   return CT_SUCCESS;
}

int main(int argc, char* argv[]) {
   TestApp* pApplication = new TestApp();
   return pApplication->Execute(argc, argv);
}