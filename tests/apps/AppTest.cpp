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
#include "imgui/Im3dIntegration.hpp"
#include "imgui/ImguiIntegration.hpp"
#include "core/FileSystem.hpp"
#include "interact/InteractionEngine.hpp"
#include "renderer/KeyLimeRenderer.hpp"
#include "core/AsyncTasks.hpp"
#include "core/JobSystem.hpp"
#include "resource/TextureResource.hpp"

class TestApp : public ctApplication {
   virtual const char* GetAppName();
   virtual const char* GetAppDeveloperName();
   virtual ctAppVersion GetAppVersion();
   virtual ctResults OnStartup();
   virtual ctResults OnTick(const float deltatime);
   virtual ctResults OnUIUpdate();
   virtual ctResults OnShutdown();
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
   ctHandlePtr<ctResourceTexture> testTexture =
     ctGetResource(ctResourceTexture, "TEST_IMAGE_PERLIN_NOISE");
   return CT_SUCCESS;
}

ctResults TestApp::OnTick(const float deltatime) {
   return CT_SUCCESS;
}

ctResults TestApp::OnUIUpdate() {
   return CT_SUCCESS;
}

ctResults TestApp::OnShutdown() {
   return CT_SUCCESS;
}

int main(int argc, char* argv[]) {
   TestApp* pApplication = new TestApp();
   return pApplication->Execute(argc, argv);
}