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

class TestApp : public ctApplication {
   virtual const char* GetAppName();
   virtual const char* GetAppPublisher();
   virtual const char* GetNativeLanguage();
   virtual ctAppVersion GetAppVersion();
   virtual ctResults OnStartup();
   virtual ctResults OnTick(const float deltatime);
   virtual ctResults OnShutdown();
};

const char* TestApp::GetAppName() {
   return "AppTest";
}

const char* TestApp::GetAppPublisher() {
    return "CitrusToolbox";
}

const char* TestApp::GetNativeLanguage() {
    return "English";
}

ctAppVersion TestApp::GetAppVersion() {
   return {1, 0, 0};
}

ctResults TestApp::OnStartup() {
   return CT_SUCCESS;
}

int loopvar = 0;
ctResults TestApp::OnTick(const float deltatime) {
   if (loopvar <= 5000) {
      Engine->Debug->Log("This Message %d", loopvar);
      loopvar++;
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