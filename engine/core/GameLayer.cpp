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

#include "GameLayer.hpp"
#include "system/System.h"

bool gGameGlobalInit = false;

ctResults ctGameLayerManager::Startup() {
   return StartupGameLayer();
}

ctResults ctGameLayerManager::Shutdown() {
   return ShutdownGamelayer();
}

const char* ctGameLayerManager::GetModuleName() {
   return "Game Layer";
}

ctResults ctGameLayerManager::StartupGameLayer() {
   /* check global instance */
   if (gGameGlobalInit) {
      ctDebugError("ATTEMPTED TO START ALREADY RUNNING GAME LAYER!");
      return CT_FAILURE_NOT_FINISHED;
   }

#if !CITRUS_STATIC_GAME

   /* load the library */
   GameLayerHandle = SDL_LoadObject(ctSystemGetGameLayerLibName());
   if (!GameLayerHandle) {
      ctFatalError(-100, "COULD NOT LOAD GAME LAYER");
      return CT_FAILURE_NOT_FOUND;
   }

   /* load the functions */
   ctGameAPIStartup =
     (ctGameAPIStartupFn)SDL_LoadFunction(GameLayerHandle, CT_GAMEAPI_NAME_STARTUP);
   ctGameAPIShutdown =
     (ctGameAPIShutdownFn)SDL_LoadFunction(GameLayerHandle, CT_GAMEAPI_NAME_SHUTDOWN);
#endif

   ctGameAPIStartup(Engine);
   gGameGlobalInit = true;
   return CT_SUCCESS;
}

ctResults ctGameLayerManager::ShutdownGamelayer() {
   /* check global instance */
   if (!gGameGlobalInit) {
      ctDebugError("ATTEMPTED TO SHUTDOWN NOT RUNNING GAME LAYER!");
      return CT_FAILURE_MODULE_NOT_INITIALIZED;
   }

   /* shutdown */
   ctGameAPIShutdown();

   /* unload if needed */
#if !CITRUS_STATIC_GAME
   SDL_UnloadObject(GameLayerHandle);
#endif
   gGameGlobalInit = false;
   return CT_SUCCESS;
}

/* ------------------- Global Variable Sharing ------------------- */

#if !CITRUS_STATIC_GAME
#include "core/EngineCore.hpp"
#include "core/Logging.hpp"
#include "core/Translation.hpp"
#include "middleware/ImguiIntegration.hpp"
#include "middleware/Im3dIntegration.hpp"

void ctGameLayerManager::_StartupGameLayerAsSharedObject() {
   /* todo: ensure shared logging, imgui, im3d and etc are shared */
}

void ctGameLayerManager::_ShutdownGameLayerAsSharedObject() {
   /* todo: log out potential leak counts */
}
#endif