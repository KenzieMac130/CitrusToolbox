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

#pragma once

#include "utilities/Common.h"
#include "core/ModuleBase.hpp"

/* load the DLL at runtime */
#if !CITRUS_STATIC_GAME
#define CT_GAMEAPI_NAME_STARTUP  "ctGameAPIStartup"
#define CT_GAMEAPI_NAME_SHUTDOWN "ctGameAPIShutdown"

typedef int (*ctGameAPIStartupFn)(void* pEngine);
typedef int (*ctGameAPIShutdownFn)(void);

/* link directly and call the function */
#else
#include "../game/GameEntryPoint.h"
#endif

class CT_API ctGameLayerManager : public ctModuleBase {
public:
   virtual ctResults Startup();
   virtual ctResults Shutdown();
   const char* GetModuleName() final;

   ctResults StartupGameLayer();
   ctResults ShutdownGamelayer();

#if !CITRUS_STATIC_GAME
   /* internal for use by game entry point */
   void _StartupGameLayerAsSharedObject();
   /* internal for use by game entry point */
   void _ShutdownGameLayerAsSharedObject();
#endif

private:
#if !CITRUS_STATIC_GAME
   void* GameLayerHandle;
   ctGameAPIStartupFn ctGameAPIStartup;
   ctGameAPIShutdownFn ctGameAPIShutdown;
#endif
};