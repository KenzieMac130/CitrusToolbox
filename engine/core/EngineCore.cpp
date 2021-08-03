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
   Settings = new ctSettings();
   AssetManager = new ctAssetManager(FileSystem);
   WindowManager = new ctWindowManager(App->GetAppName(), Settings);
   Debug = new ctDebugSystem(FileSystem, 32, true, Settings, WindowManager);
#if CITRUS_INCLUDE_AUDITION
   HotReload = new ctHotReloadDetection(FileSystem, Settings);
#endif
   Translation = new ctTranslation(true);
   JobSystem = new ctJobSystem(2, Settings);
   OSEventManager = new ctOSEventManager();
   Interact = new ctInteractionEngine(OSEventManager);
   ImguiIntegration = new ctImguiIntegration();
   Im3dIntegration = new ctIm3dIntegration();
   Renderer = new ctKeyLimeRenderer();
   FrameTime = ctStopwatch();
   SceneEngine = new CITRUS_SCENE_ENGINE_CLASS(this);
   PhysXIntegration = new ctPhysXIntegration();

   /* Startup Modules */
   Settings->ModuleStartup();
   FileSystem->ModuleStartup();
   Debug->ModuleStartup();
#if CITRUS_INCLUDE_AUDITION
   HotReload->ModuleStartup();
#endif
   FileSystem->LogPaths();
   AssetManager->ModuleStartup();
   Translation->ModuleStartup();
   JobSystem->ModuleStartup();
   OSEventManager->ModuleStartup();
#if !CITRUS_HEADLESS
   WindowManager->ModuleStartup();
#endif
   Interact->ModuleStartup();
   ImguiIntegration->ModuleStartup();
   Im3dIntegration->ModuleStartup();
   Renderer->ModuleStartup();
   PhysXIntegration->ModuleStartup();
   SceneEngine->ModuleStartup();
   ctDebugLog("Citrus Toolbox has Started!");

   /* Run User Code */
   ctGetGameLayer().ModuleStartup();
   App->OnStartup();
   ctDebugLog("Application has Started!");
   return CT_SUCCESS;
}

ctResults ctEngineCore::EnterLoop() {
   while (!OSEventManager->wantsExit) {
      FrameTime.NextLap();
      LoopSingleShot(FrameTime.GetDeltaTimeFloat());
   }
   Shutdown();

   return CT_SUCCESS;
}

ctResults ctEngineCore::LoopSingleShot(const float deltatime) {
   ZoneScoped;
   App->OnTick(deltatime);
   SceneEngine->NextFrame();
   App->OnUIUpdate();
   Renderer->RenderFrame();
   OSEventManager->PollOSEvents();
   Interact->PumpInput(deltatime);
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
   AssetManager->Shutdown();
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