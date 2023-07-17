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

#include "utilities/Common.h"
#include "GameEntryPoint.h"
#include "core/EngineCore.hpp"
#include "core/GameLayer.hpp"

ctEngineCore* Engine;

GAME_LAYER_API int ctGameAPIStartup(void* pEngine) {
   Engine = (ctEngineCore*)pEngine;
#if !CITRUS_STATIC_GAME
   Engine->GameLayer->_StartupGameLayerAsSharedObject();
#endif

   ctDebugLog("TEST");
   return (int)CT_SUCCESS;
}

GAME_LAYER_API int ctGameAPIShutdown() {
#if !CITRUS_STATIC_GAME
   Engine->GameLayer->_ShutdownGameLayerAsSharedObject();
#endif
   return (int)CT_SUCCESS;
}