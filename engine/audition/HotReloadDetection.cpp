#include "HotReloadDetection.hpp"
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

#include "HotReloadDetection.hpp"
#include "core/EngineCore.hpp"
#include "core/Settings.hpp"

#define DMON_IMPL
#include "dmon/dmon.h"

void ctHotReloadDetection::WatchCallback(uint32_t watch_id,
                                         uint32_t action,
                                         const char* rootdir,
                                         const char* filepath,
                                         const char* oldfilepath,
                                         void* user) {
   ctHotReloadDetection* pDetector = (ctHotReloadDetection*)user;
   if (action == DMON_ACTION_MODIFY) {
      ctMutexLock(pDetector->callbackLock);
      pDetector->PushPathUpdate(filepath);
      ctMutexUnlock(pDetector->callbackLock);
   }
}

typedef void (*dmon_callback)(dmon_watch_id watch_id,
                              dmon_action action,
                              const char* dirname,
                              const char* filename,
                              const char* oldname,
                              void* user);

ctResults ctHotReloadDetection::Startup() {
   ZoneScoped;
   ctSettingsSection* settings = Engine->Settings->GetOrCreateSection("Audition", 1);
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
      callbackLock = ctMutexCreate();
      const char* dataPath = Engine->FileSystem->GetDataPath();
      if (!dataPath) { return CT_FAILURE_INACCESSIBLE; }
      dmon_init();
      dmon_watch(dataPath,
                 (dmon_callback)ctHotReloadDetection::WatchCallback,
                 DMON_WATCHFLAGS_RECURSIVE,
                 this);
   }
   return CT_SUCCESS;
}

ctResults ctHotReloadDetection::Shutdown() {
   ZoneScoped;
   if (watchIsRunning) {
      ctDebugLog("Hot Reload Watch is Shutting Down...");
      ctMutexLock(callbackLock);
      ctMutexDestroy(callbackLock);
      dmon_deinit();
   }
   return CT_SUCCESS;
}

const char* ctHotReloadDetection::GetModuleName() {
   return "Hot Reload";
}

void ctHotReloadDetection::DebugUI(bool useGizmos)
{
}

ctResults ctHotReloadDetection::RegisterDataCategory(ctHotReloadCategory* pCategory) {
   pCategory->_pOwner = this;
   return hotReloads.Append(pCategory);
}

void ctHotReloadDetection::PushPathUpdate(const char* path) {
   ZoneScoped;
   ctDebugLog("Hot Reload: Modified %s", path);
   for (int i = 0; i < hotReloads.Count(); i++) {
      hotReloads[i]->_AddFileUpdate(path);
   }
}

void ctHotReloadCategory::RegisterData(const ctGUID& guid) {
   char path[33];
   memset(path, 0, 33);
   guid.ToHex(path);
   RegisterPath(path);
}

void ctHotReloadCategory::UnregisterData(const ctGUID& guid) {
   char path[33];
   memset(path, 0, 33);
   guid.ToHex(path);
   UnregisterPath(path);
}

void ctHotReloadCategory::RegisterPath(const char* relativePath) {
   ZoneScoped;
   uint64_t hash = XXH64(relativePath, strlen(relativePath), 0);
   if (watchedPathHashes.FindIndex(hash) >= 0) { return; }
   watchedPathHashes.Append(hash);
}

void ctHotReloadCategory::UnregisterPath(const char* relativePath) {
   ZoneScoped;
   uint64_t hash = XXH64(relativePath, strlen(relativePath), 0);
   watchedPathHashes.Remove(hash);
}

void ctHotReloadCategory::Reset() {
   watchedPathHashes.Clear();
   updatedPaths.Clear();
}

bool ctHotReloadCategory::isContentUpdated() {
   return !updatedPaths.isEmpty();
}

void ctHotReloadCategory::BeginReadingChanges() {
   ZoneScoped;
   ctMutexLock(_pOwner->callbackLock);
}

const ctDynamicArray<ctStringUtf8>& ctHotReloadCategory::GetUpdatedPaths() const {
   return updatedPaths;
}

void ctHotReloadCategory::EndReadingChanges() {
   ZoneScoped;
   updatedPaths.Clear();
   ctMutexUnlock(_pOwner->callbackLock);
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
