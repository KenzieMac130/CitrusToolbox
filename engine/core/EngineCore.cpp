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
   Settings = new ctSettings();
   JobSystem = new ctJobSystem(1);
   Translation = new ctTranslation(App->GetNativeLanguage(), true);
   OSEventManager = new ctOSEventManager();
   FileSystem = new ctFileSystem(App->GetAppName(), App->GetAppPublisher());
   Debug = new ctDebugSystem(32, true);
   WindowManager = new ctWindowManager();
   Renderer = new ctRenderer();

   /* Startup Modules */
   Settings->ModuleStartup(this);
   JobSystem->ModuleStartup(this);
   Translation->ModuleStartup(this);
   OSEventManager->ModuleStartup(this);
   FileSystem->ModuleStartup(this);
   Debug->ModuleStartup(this);
   WindowManager->ModuleStartup(this);
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
   OSEventManager->PollOSEvents();
   return CT_SUCCESS;
}

ctResults ctEngineCore::Shutdown() {
   App->OnShutdown();
   /*Shutdown modules*/
   Renderer->ModuleShutdown();
   WindowManager->ModuleShutdown();
   Debug->ModuleShutdown();
   FileSystem->ModuleShutdown();
   OSEventManager->ModuleShutdown();
   Translation->ModuleShutdown();
   JobSystem->ModuleShutdown();
   Settings->ModuleShutdown();
   delete Renderer;
   delete WindowManager;
   delete Debug;
   delete FileSystem;
   delete Settings;
   delete OSEventManager;
   delete Translation;
   delete JobSystem;
   return CT_SUCCESS;
}