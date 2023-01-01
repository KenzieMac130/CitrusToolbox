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

#include "LiveSync.hpp"
#include "core/EngineCore.hpp"
#include "core/Settings.hpp"

ctResults ctAuditionLiveSync::Startup() {
   ctSettingsSection* settings = Engine->Settings->GetOrCreateSection("Audition", 3);
   settings->BindInteger(&stagedPort,
                         true,
                         true,
                         "LiveSyncPort",
                         "Port to use for a live sync session.",
                         CT_SETTINGS_BOUNDS_UINT);
   lock = ctMutexCreate();
   return CT_SUCCESS;
}

ctResults ctAuditionLiveSync::Shutdown() {
   StopServer();
   ctThreadWaitForExit(serverThread);
   return CT_SUCCESS;
}

const char* ctAuditionLiveSync::GetModuleName() {
   return "Live Sync";
}

void ctAuditionLiveSync::Dispatch(ctAuditionLiveSyncCategory category, bool onlyChanged) {
   ctMutexLock(lock);
   uint64_t lastTime = locked.lastDispatchTime[category];
   for (size_t i = 0; i < locked.properties[category].Count(); i++) {
      ctAuditionLiveSyncProp& prop = locked.properties[category][i];
      if (prop.timestamp > lastTime || !onlyChanged) {
         locked.callbacks[category].fpCallback(prop, locked.callbacks[category].pData);
      }
   }
   locked.lastDispatchTime[category] = ctGetTimestamp();
   ctMutexUnlock(lock);
}