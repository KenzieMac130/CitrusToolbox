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

#include "middleware/PhysXIntegration.hpp"

#include "gamelayer/GameLayer.hpp"

#include "JobSystem.hpp"
#include "FileSystem.hpp"
#include "Logging.hpp"
#include "Settings.hpp"
#include "WindowManager.hpp"
#include "OSEvents.hpp"
#include "Translation.hpp"

#include "middleware/ImguiIntegration.hpp"
#include "middleware/Im3dIntegration.hpp"

#if CITRUS_INCLUDE_AUDITION
#include "audition/HotReloadDetection.hpp"
#endif

#include "interact/InteractionEngine.hpp"
#include "renderer/KeyLime.hpp"
#include "scene/SceneEngineBase.hpp"
#include "asset/AssetManager.hpp"

#include CITRUS_SCENE_ENGINE_HEADER

ctResults ctEngineCore::Ignite(ctApplication* pApp) {
   ZoneScoped;
   App = pApp;

   /*SDL*/
#if !CITRUS_HEADLESS
   SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
#else
   SDL_Init(SDL_INIT_TIMER);
#endif

   /* Create Modules */
   FileSystem = new ctFileSystem(App->GetAppName(), App->GetAppDeveloperName());
   AssetManager = new ctAssetManager(FileSystem);
   Settings = new ctSettings();
   Debug = new ctDebugSystem(32, true);
#if CITRUS_INCLUDE_AUDITION
   HotReload = new ctHotReloadDetection();
#endif
   Translation = new ctTranslation(true);
   JobSystem = new ctJobSystem(2);
   OSEventManager = new ctOSEventManager();
   WindowManager = new ctWindowManager();
   Interact = new ctInteractionEngine();
   ImguiIntegration = new ctImguiIntegration();
   Im3dIntegration = new ctIm3dIntegration();
   Renderer = new ctKeyLimeRenderer();
   FrameTime = ctStopwatch();
   SceneEngine = new CITRUS_SCENE_ENGINE_CLASS();
   PhysXIntegration = new ctPhysXIntegration();

   /* Startup Modules */
   Settings->ModuleStartup(this);
   FileSystem->ModuleStartup(this);
   Debug->ModuleStartup(this);
#if CITRUS_INCLUDE_AUDITION
   HotReload->ModuleStartup(this);
#endif
   FileSystem->LogPaths();
   AssetManager->ModuleStartup(this);
   Translation->ModuleStartup(this);
   JobSystem->ModuleStartup(this);
   OSEventManager->ModuleStartup(this);
#if !CITRUS_HEADLESS
   WindowManager->ModuleStartup(this);
#endif
   Interact->ModuleStartup(this);
   ImguiIntegration->ModuleStartup(this);
   Im3dIntegration->ModuleStartup(this);
   Renderer->ModuleStartup(this);
   PhysXIntegration->ModuleStartup(this);
   SceneEngine->ModuleStartup(this);
   ctDebugLog("Citrus Toolbox has Started!");

   /* Run User Code */
   ctGetGameLayer().ModuleStartup(this);
   App->OnStartup();
   ctDebugLog("Application has Started!");
   return CT_SUCCESS;
}

ctResults ctEngineCore::EnterLoop() {
   _isRunning = true;
   while (_isRunning) {
      FrameTime.NextLap();
      LoopSingleShot(FrameTime.GetDeltaTimeFloat());
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
   AssetManager->Update(JobSystem);
   App->OnTick(deltatime);
   SceneEngine->NextFrame();
   App->OnUIUpdate();
   Renderer->RenderFrame();
   OSEventManager->PollOSEvents();
   Interact->PumpInput();
   Im3dIntegration->NextFrame();
   ImguiIntegration->NextFrame();
   FrameMark;
   return CT_SUCCESS;
}

ctResults ctEngineCore::Shutdown() {
   ZoneScoped;
   /*Shutdown application*/
   ctDebugLog("Application is Shutting Down...");
   App->OnShutdown();
   ctGetGameLayer().ModuleShutdown();

   /*Shutdown modules*/
   ctDebugLog("Citrus Toolbox is Shutting Down...");
   SceneEngine->ModuleShutdown();
   PhysXIntegration->ModuleShutdown();
   Renderer->ModuleShutdown();
   Im3dIntegration->ModuleShutdown();
   ImguiIntegration->ModuleShutdown();
   Interact->ModuleShutdown();
#if !CITRUS_HEADLESS
   WindowManager->ModuleShutdown();
#endif
   OSEventManager->ModuleShutdown();
   AssetManager->ModuleShutdown();
   JobSystem->ModuleShutdown();
   Translation->ModuleShutdown();
#if CITRUS_INCLUDE_AUDITION
   HotReload->ModuleShutdown();
#endif
   Debug->ModuleShutdown();
   Settings->ModuleShutdown();
   FileSystem->ModuleShutdown();
   delete Renderer;
   delete ImguiIntegration;
   delete WindowManager;
   delete Debug;
   delete AssetManager;
   delete FileSystem;
   delete Settings;
   delete OSEventManager;
   delete Translation;
   delete JobSystem;

   /*SDL*/
   SDL_Quit();
   return CT_SUCCESS;
}