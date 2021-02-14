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

#include "Application.hpp"

const ctStringUtf8& ctApplication::GetAppName() {
   return _appName;
}

int ctApplication::GetAppVersionMajor() {
   return _appVersionMajor;
}

int ctApplication::GetAppVersionMinor() {
   return _appVersionMinor;
}

int ctApplication::GetAppVersionPatch() {
   return _appVersionPatch;
}

int ctApplication::Execute(int argc, char* argv[])
{
    Engine = new ctEngineCore();
    Engine->Ignite(this);
    Engine->EnterLoop();
    delete Engine;
    return 0;
}

ctResults ctApplication::InitialWindowSetup() {
   Engine->WindowManager->CreateWindow(
     NULL, "Test", 0, 640, 480, CT_WINDOWMODE_WINDOWED_RESIZABLE);
   return CT_SUCCESS;
}

ctResults ctApplication::OnStartup() {
   return ctResults();
}

ctResults ctApplication::OnTick(const float deltatime) {
   return ctResults();
}

ctResults ctApplication::OnShutdown() {
   return ctResults();
}