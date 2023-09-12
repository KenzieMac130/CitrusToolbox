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

#include "PhysicsTest.hpp"
#include "core/Application.hpp"

#include "StackTest.hpp"
#include "Constraints.hpp"

class TestApp : public ctApplication {
   virtual const char* GetAppName();
   virtual const char* GetAppDeveloperName();
   virtual ctAppVersion GetAppVersion();
   virtual ctResults OnStartup();
   virtual ctResults OnTick(const float deltatime);
   virtual ctResults OnUIUpdate();
   virtual ctResults OnShutdown();

   int32_t scenarioIndex;
   PhysicsTestBase* activeScenario = NULL;
   ctDynamicArray<PhysicsTestBase*> scenarios;
   ctDynamicArray<const char*> scenariosText;
   void StartScenario();
};

const char* TestApp::GetAppName() {
   return "PhsicsTest";
}

const char* TestApp::GetAppDeveloperName() {
   return "CitrusToolbox";
}

ctAppVersion TestApp::GetAppVersion() {
   return {1, 0, 0};
}

ctResults TestApp::OnStartup() {
   /* setup camera */
   Engine->SceneEngine->EnableCameraOverride();
   ctCameraInfo camera;
   camera.position = ctVec3(0.0f, 3.0f, -25.0f);
   Engine->SceneEngine->SetCameraOverride(camera);
   Engine->SceneEngine->EnableDebugCamera();

   /* create scenarios */
   scenarios.Append(new StackTest());
   scenarios.Append(new ConstraintTest());

   for (size_t i = 0; i < scenarios.Count(); i++) {
      scenarios[i]->Engine = Engine;
      scenariosText.Append(scenarios[i]->GetName());
   }
   return CT_SUCCESS;
}

ctResults TestApp::OnTick(const float deltatime) {
   if (activeScenario) { activeScenario->OnTick(deltatime); }
   return CT_SUCCESS;
}

ctResults TestApp::OnUIUpdate() {
   if (ImGui::Begin("Physics Test")) {
      ImGui::Combo(
        "Scenario", &scenarioIndex, scenariosText.Data(), (int)scenariosText.Count(), -1);
      scenarios[scenarioIndex]->UIOptions();
      if (ImGui::Button("Start")) {
         if (activeScenario) { activeScenario->OnTestShutdown(); }
         activeScenario = scenarios[scenarioIndex];
         activeScenario->OnTestStartup();
      }
      if (activeScenario) { activeScenario->UIStatus(); }
   }
   ImGui::End();
   return CT_SUCCESS;
}

void TestApp::StartScenario() {
}

ctResults TestApp::OnShutdown() {
   return CT_SUCCESS;
}

int main(int argc, char* argv[]) {
   TestApp* pApplication = new TestApp();
   return pApplication->Execute(argc, argv);
}