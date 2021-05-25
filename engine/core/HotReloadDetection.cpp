#include "HotReloadDetection.hpp"
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

#include "HotReloadDetection.hpp"
#include "EngineCore.hpp"

#define DMON_IMPL
#include "dmon/dmon.h"

static void watch_callback(dmon_watch_id watch_id,
                           dmon_action action,
                           const char* rootdir,
                           const char* filepath,
                           const char* oldfilepath,
                           void* user) {
   ctHotReloadDetection* pDetector = (ctHotReloadDetection*)user;
   if (action == DMON_ACTION_MODIFY) {
       ctMutexLock(pDetector->_callbackLock);
       pDetector->_PushPathUpdate(filepath);
       ctMutexUnlock(pDetector->_callbackLock);
   }
}

ctResults ctHotReloadDetection::Startup() {
   ZoneScoped;
   ctSettingsSection* settings = Engine->Settings->CreateSection("HotReload", 32);
   watchEnable = 1;
   settings->BindInteger(&watchEnable,
                         false,
                         true,
                         "WatchEnable",
                         "Watch for file updates in the background.",
                         CT_SETTINGS_BOUNDS_BOOL);
   watchIsRunning = watchEnable;
   if (watchIsRunning) {
      ctDebugLog("Hot Reload Watch Enabled...");
      _callbackLock = ctMutexCreate();
      ctStringUtf8 assetPath = Engine->FileSystem->GetAssetPath();
      dmon_init();
      dmon_watch(assetPath.CStr(), watch_callback, DMON_WATCHFLAGS_RECURSIVE, this);
   }
   return CT_SUCCESS;
}

ctResults ctHotReloadDetection::Shutdown() {
   ZoneScoped;
   if (watchIsRunning) {
      ctDebugLog("Hot Reload Watch is Shutting Down...");
      ctMutexLock(_callbackLock);
      ctMutexDestroy(_callbackLock);
      dmon_deinit();
   }
   return CT_SUCCESS;
}

ctResults ctHotReloadDetection::RegisterAssetCategory(ctHotReloadCategory* pCategory) {
   pCategory->_pOwner = this;
   return hotReloads.Append(pCategory);
}

void ctHotReloadDetection::_PushPathUpdate(const char* path) {
   ZoneScoped;
   ctDebugLog("Hot Reload: Modified %s", path);
   for (int i = 0; i < hotReloads.Count(); i++) {
      hotReloads[i]->_AddFileUpdate(path);
   }
}

void ctHotReloadCategory::RegisterPath(const char* relativePath) {
   ZoneScoped;
   uint64_t hash = XXH64(relativePath, strlen(relativePath), 0);
   watchedPathHashes.Append(hash);
}

void ctHotReloadCategory::UnregisterPath(const char* relativePath) {
   ZoneScoped;
   uint64_t hash = XXH64(relativePath, strlen(relativePath), 0);
   watchedPathHashes.Remove(hash);
}

bool ctHotReloadCategory::isContentUpdated() {
   return !updatedPaths.isEmpty();
}

void ctHotReloadCategory::BeginReadingChanges() {
   ZoneScoped;
   ctMutexLock(_pOwner->_callbackLock);
}

const ctDynamicArray<ctStringUtf8>& ctHotReloadCategory::GetUpdatedPaths() const {
   return updatedPaths;
}

void ctHotReloadCategory::EndReadingChanges() {
   ZoneScoped;
   updatedPaths.Clear();
   ctMutexUnlock(_pOwner->_callbackLock);
}

void ctHotReloadCategory::ClearChanges() {
   BeginReadingChanges();
   EndReadingChanges();
}

void ctHotReloadCategory::_AddFileUpdate(const char* path) {
   ZoneScoped;
   uint64_t hash = XXH64(path, strlen(path), 0);
   if (watchedPathHashes.Exists(hash)) { updatedPaths.Append(path); }
}
