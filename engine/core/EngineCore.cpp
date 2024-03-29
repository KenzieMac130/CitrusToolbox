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

#include "EngineCore.hpp"
#include "Application.hpp"

#include "GameLayer.hpp"

#include "AsyncTasks.hpp"
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
#include "audition/AuditionEditor.hpp"
#include "audition/AssetCompilerBootstrap.hpp"
#include "audition/LiveSync.hpp"
#endif

#include "animation/AnimationSystem.hpp"
#include "interact/InteractionEngine.hpp"
#include "renderer/KeyLimeRenderer.hpp"
#include "scene/SceneEngine.hpp"

#include "physics/Module.hpp"

ctResults ctEngineCore::Ignite(ctApplication* pApp, int argc, char* argv[]) {
   ZoneScoped;
   App = pApp;

   /*SDL*/
#if !CITRUS_HEADLESS
   SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
#else
   SDL_Init(SDL_INIT_TIMER);
#endif
   if (SDL_GetCPUCacheLineSize() != CT_ALIGNMENT_CACHE) {
      ctDebugError("Cache line mismatch! Expected %d got %d",
                   CT_ALIGNMENT_CACHE,
                   SDL_GetCPUCacheLineSize())
   };

   /* Create Modules */
   FileSystem = new ctFileSystem(App->GetAppName(), App->GetAppDeveloperName());
   Settings = new ctSettingsManager(argc, argv);
   Debug = new ctDebugSystem(32, true);
#if CITRUS_INCLUDE_AUDITION
   HotReload = new ctHotReloadDetection();
#endif
   Translation = new ctTranslation(true);
   AsyncTasks = new ctAsyncManager(true);
   JobSystem = new ctJobSystem(2);
   OSEventManager = new ctOSEventManager();
   WindowManager = new ctWindowManager();
   Interact = new ctInteractionEngine(true);
   ImguiIntegration = new ctImguiIntegration();
   Im3dIntegration = new ctIm3dIntegration();
   Animation = new ctAnimationSystem();
   Renderer = new ctKeyLimeRenderer();
   Physics = new ctPhysicsModule();
   FrameTime = ctStopwatch();
   SceneEngine = new ctSceneEngine();
#if CITRUS_INCLUDE_AUDITION
   Editor = new ctAuditionEditor();
   AssetCompiler = new ctAssetCompilerBootstrap();
   LiveSync = new ctAuditionLiveSync();
#endif
   GameLayer = new ctGameLayerManager();

   /* Startup Modules */
   Settings->ModuleStartup(this);
   FileSystem->ModuleStartup(this);
   Debug->ModuleStartup(this);
#if CITRUS_INCLUDE_AUDITION
   HotReload->ModuleStartup(this);
   LiveSync->ModuleStartup(this);
#endif
   FileSystem->LogPaths();
   Translation->ModuleStartup(this);
   AsyncTasks->ModuleStartup(this);
   JobSystem->ModuleStartup(this);
   OSEventManager->ModuleStartup(this);
#if !CITRUS_HEADLESS
   WindowManager->ModuleStartup(this);
#endif
   Interact->ModuleStartup(this);
   ImguiIntegration->ModuleStartup(this);
   Im3dIntegration->ModuleStartup(this);
   Animation->ModuleStartup(this);
   Physics->ModuleStartup(this);
   SceneEngine->ModuleStartup(this);
   Renderer->ModuleStartup(this);
#if CITRUS_INCLUDE_AUDITION
   Editor->ModuleStartup(this);
   AssetCompiler->ModuleStartup(this);
#endif
   ctDebugLog("Citrus Toolbox has Started!");

   /* Run User Code */
   GameLayer->ModuleStartup(this);
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

bool ctEngineCore::isExitRequested() const {
   return !_isRunning;
}

ctResults ctEngineCore::LoopSingleShot(const float deltatime) {
   ZoneScoped;
   Translation->NextFrame();
   App->OnFrameAdvance(deltatime);
   SceneEngine->NextFrame(deltatime);
   App->OnUIUpdate();
#if CITRUS_INCLUDE_AUDITION
   Editor->UpdateEditor();
#endif
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
   /* Kill all dangling tasks first */
   AsyncTasks->ModuleShutdown();

   /*Shutdown application*/
   ctDebugLog("Application is Shutting Down...");
   App->OnShutdown();
   GameLayer->ModuleShutdown();

   /*Shutdown modules*/
   ctDebugLog("Citrus Toolbox is Shutting Down...");
#if CITRUS_INCLUDE_AUDITION
   LiveSync->ModuleShutdown();
   AssetCompiler->ModuleShutdown();
   Editor->ModuleShutdown();
#endif
   SceneEngine->ModuleShutdown();
   Physics->ModuleShutdown();
   Animation->ModuleShutdown();
   Renderer->ModuleShutdown();
   Im3dIntegration->ModuleShutdown();
   ImguiIntegration->ModuleShutdown();
   Interact->ModuleShutdown();
#if !CITRUS_HEADLESS
   WindowManager->ModuleShutdown();
#endif
   OSEventManager->ModuleShutdown();
   JobSystem->ModuleShutdown();
   Translation->ModuleShutdown();
#if CITRUS_INCLUDE_AUDITION
   HotReload->ModuleShutdown();
#endif
   Debug->ModuleShutdown();
   Settings->ModuleShutdown();
   FileSystem->ModuleShutdown();
   delete GameLayer;
#if CITRUS_INCLUDE_AUDITION
   delete AssetCompiler;
   delete Editor;
#endif
   delete Physics;
   delete Animation;
   delete Renderer;
   delete Im3dIntegration;
   delete ImguiIntegration;
   delete Interact;
   delete WindowManager;
   delete Debug;
   delete FileSystem;
   delete Settings;
   delete OSEventManager;
   delete Translation;
   delete JobSystem;
   delete AsyncTasks;
#if CITRUS_INCLUDE_AUDITION
   delete HotReload;
#endif

   /*SDL*/
   SDL_Quit();
   size_t leakedAllocations = ctGetAliveAllocations();
   if (leakedAllocations) {
      ctDebugWarning("POSSIBLE LEAKED ALLOCATIONS %lu!", leakedAllocations);
   }
   return CT_SUCCESS;
}