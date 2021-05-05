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
#include "Application.hpp"

ctResults ctEngineCore::Ignite(ctApplication* pApp) {
   ZoneScoped;
   App = pApp;
   /*SDL*/
   SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_TIMER | SDL_INIT_HAPTIC);

   /* Create Modules */
   FileSystem = new ctFileSystem(App->GetAppName(), App->GetAppPublisher());
   Settings = new ctSettings();
   Debug = new ctDebugSystem(32, true);
   Translation = new ctTranslation(true);
   JobSystem = new ctJobSystem(2);
   OSEventManager = new ctOSEventManager();
   WindowManager = new ctWindowManager();
   ImguiIntegration = new ctImguiIntegration();
   Renderer = new ctKeyLimeRenderer();

   /* Startup Modules */
   Settings->ModuleStartup(this);
   FileSystem->ModuleStartup(this);
   Debug->ModuleStartup(this);
   FileSystem->LogPaths();
   Translation->ModuleStartup(this);
   JobSystem->ModuleStartup(this);
   OSEventManager->ModuleStartup(this);
   WindowManager->ModuleStartup(this);
   ImguiIntegration->ModuleStartup(this);
   Renderer->ModuleStartup(this);
   ctDebugLog("Citrus Toolbox has Started!");

   /* Run User Code */
   App->OnStartup();
   ctDebugLog("Application has Started!");
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
   ZoneScoped;
   App->OnTick(deltatime);
   App->OnUIUpdate();
   /*Update modules*/
   OSEventManager->PollOSEvents();
   Renderer->RenderFrame();
   ImguiIntegration->NextFrame();
   FrameMark;
   return CT_SUCCESS;
}

ctResults ctEngineCore::Shutdown() {
   ZoneScoped;
   /*Shutdown application*/
   ctDebugLog("Application is Shutting Down...");
   App->OnShutdown();

   /*Shutdown modules*/
   ctDebugLog("Citrus Toolbox is Shutting Down...");
   Renderer->ModuleShutdown();
   ImguiIntegration->ModuleShutdown();
   WindowManager->ModuleShutdown();
   OSEventManager->ModuleShutdown();
   JobSystem->ModuleShutdown();
   Translation->ModuleShutdown();
   Debug->ModuleShutdown();
   Settings->ModuleShutdown();
   FileSystem->ModuleShutdown();
   delete Renderer;
   delete ImguiIntegration;
   delete WindowManager;
   delete Debug;
   delete FileSystem;
   delete Settings;
   delete OSEventManager;
   delete Translation;
   delete JobSystem;

   /*SDL*/
   SDL_Quit();
   return CT_SUCCESS;
}