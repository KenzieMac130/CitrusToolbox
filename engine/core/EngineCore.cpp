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

#include "EngineCore.hpp"

ctResults ctEngineCore::Ignite(ctApplication* pApp) {
   App = pApp;
   /* Create Modules */
   FileSystem = new ctFileSystem(App->GetAppName(), "CitrusToolbox");
   Debug = new ctDebugSystem(32);
   WindowManager = new ctWindowManager();
   Renderer = new ctRenderer();

   /* Startup Modules */
   FileSystem->ModuleStartup(this);
   Debug->ModuleStartup(this);
   WindowManager->ModuleStartup(this);
   App->InitialWindowSetup();
   Renderer->ModuleStartup(this);

   /* Run User Code */
   App->OnStartup();
   return CT_SUCCESS;
}

ctResults ctEngineCore::EnterLoop() {
   _isRunning = true;
   while (_isRunning) {
      LoopSingleShot(1.0f / 60.0f);
   }
   Shutdown();
   return CT_SUCCESS;
}

void ctEngineCore::Exit() {
   _isRunning = false;
}

bool ctEngineCore::isExitRequested() {
   return !_isRunning;
}

ctResults ctEngineCore::LoopSingleShot(const float deltatime) {
   /*Update modules*/
   App->OnTick(deltatime);
   return CT_SUCCESS;
}

ctResults ctEngineCore::Shutdown() {
   App->OnShutdown();
   /*Shutdown modules*/
   Renderer->ModuleShutdown();
   WindowManager->ModuleShutdown();
   Debug->ModuleShutdown();
   FileSystem->ModuleShutdown();
   delete WindowManager;
   delete Renderer;
   delete Debug;
   delete FileSystem;
   return CT_SUCCESS;
}