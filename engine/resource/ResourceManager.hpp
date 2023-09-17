/*
   Copyright 2023 MacKenzie Strand

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
#if CITRUS_INCLUDE_AUDITION
#include "audition/HotReloadDetection.hpp"
#endif

enum ctResourcePriority {
   CT_RESOURCE_PRIORITY_BACKGROUND, /* scenery models, textures, level chunks, etc */
   CT_RESOURCE_PRIORITY_FOREGROUND, /* hud models, cutscene content, etc */
   CT_RESOURCE_PRIORITY_HIGHEST,    /* highest priority gameplay config data */
};

#define ctGetResource(_TYPE, _KEY)                                                       \
   (_TYPE) ctGetResourceManager()->GetOrLoad(                                            \
     ##_TYPE, _KEY, CT_RESOURCE_PRIORITY_BACKGROUND)
#define ctGetResourceWithPriority(_TYPE, _KEY, _PRIORITY)                                \
   (_TYPE) ctGetResourceManager()->GetOrLoad(##_TYPE, _KEY, _PRIORITY)

class ctResourceManager : public ctModuleBase {
   ctResourceManager(bool shared);
   virtual ctResults Startup();
   virtual ctResults Shutdown();
   virtual const char* GetModuleName();

   void Poll();

   void StartupServers();
   void ReloadNicknames();

   class ctResourceBase*
   GetOrLoad(const char* className, ctGUID guid, ctResourcePriority priority);
   class ctResourceBase*
   GetOrLoad(const char* className, const char* nickname, ctResourcePriority priority);
   ctResults GetGUIDForNickname(ctGUID& result, const char* nickname);

private:
   ctHashTable<class ctResourceServerBase*, size_t> resourceServers;
   inline void RegisterServer(const char* resourceClassName,
                              class ctResourceServerBase* server) {
      resourceServers.Insert(ctHornerHash(resourceClassName), server);
   }
   inline class ctResourceServerBase* GetServer(const char* className) {
   }
   ctHashTable<ctGUID, uint64_t> nicknameToGUIDs;
#if CITRUS_INCLUDE_AUDITION
   ctHotReloadCategory hotReloadCategory;
   void ProcessHotReload();
#endif
};

ctResourceManager* ctGetResourceManager();