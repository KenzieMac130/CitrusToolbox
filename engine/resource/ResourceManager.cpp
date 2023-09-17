#include "ResourceManager.hpp"
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

/* -------------- Register Servers Here -------------- */

#include "JSONResource.hpp"

void ctResourceManager::StartupServers() {
   RegisterServer("ctResourceJSON", new ctResourceServerJSON());
}

/* --------------------------------------------------- */

ctResourceManager* gResourceManager = NULL;

ctResourceManager::ctResourceManager(bool shared) {
   if (shared) { gResourceManager = this; }
}

ctResults ctResourceManager::Startup() {
   StartupServers();
   ReloadNicknames();
   return CT_SUCCESS;
}

ctResults ctResourceManager::Shutdown() {
   // todo
   return CT_SUCCESS;
}

const char* ctResourceManager::GetModuleName() {
   return "Resource Manager";
}

void ctResourceManager::Poll() {
   /* update servers garbage collection */
   for (auto it = resourceServers.GetIterator(); it; it++) {
      it.Value()->DoGarbageCollection();
   }

#if CITRUS_INCLUDE_AUDITION
   ProcessHotReload();
#endif
}

void ctResourceManager::ReloadNicknames() {
   ctResourceJSON* pResourceNicknames =
     (ctResourceJSON*)GetOrLoad("ctResourceJSON",
                                ctGUID("00000000000000000000000000000000"),
                                CT_RESOURCE_PRIORITY_HIGHEST);
   pResourceNicknames->WaitForReady();
   if (!pResourceNicknames->isValid()) {
      ctDebugError("FAILED TO LOAD RESOURCE NICKNAMES");
      return;
   }
   ctJSONReadEntry root = pResourceNicknames->rootEntry;
   for (size_t i = 0; i < root.GetObjectEntryCount(); i++) {
      ctStringUtf8 name;
      ctJSONReadEntry entry;
      root.GetObjectEntry(i, entry, &name);
      char guidStr[34];
      memset(guidStr, 34, 0);
      name.CopyToArray(guidStr, 34);
      ctGUID guid = ctGUID(guidStr);
      nicknameToGUIDs.Insert(name.xxHash64(), guid);
   }
   pResourceNicknames->Dereference();
}

ctResourceBase* ctResourceManager::GetOrLoad(const char* className,
                                             ctGUID guid,
                                             ctResourcePriority priority) {
   // todo
   return NULL;
}

ctResourceBase* ctResourceManager::GetOrLoad(const char* className,
                                             const char* nickname,
                                             ctResourcePriority priority) {
   ctGUID guid;
   if (GetGUIDForNickname(guid, nickname) != CT_SUCCESS) {
      ctDebugError("RESOURCE OF NICKNAME \"$s\" NOT FOUND!", nickname);
      return NULL;
   }
   return GetOrLoad(className, nickname, priority);
}

ctResults ctResourceManager::GetGUIDForNickname(ctGUID& result, const char* nickname) {
   ctGUID* pResult = nicknameToGUIDs.FindPtr(ctXXHash64(nickname));
   if (pResult) {
      result = *pResult;
      return CT_SUCCESS;
   }
   return CT_FAILURE_NOT_FOUND;
}

#if CITRUS_INCLUDE_AUDITION
void ctResourceManager::ProcessHotReload() {
   hotReloadCategory.BeginReadingChanges();

   /* check if content was updated (otherwise skip updates) */
   if (!hotReloadCategory.isContentUpdated()) {
      hotReloadCategory.EndReadingChanges();
      return;
   }

   /* drain the async system to ensure single threading */
   Engine->AsyncTasks->WaitForAllToExit();

   /* process updated paths */
   ctDynamicArray<ctStringUtf8> paths = hotReloadCategory.GetUpdatedPaths();
   for (size_t i = 0; i < paths.Count(); i++) {
      /* create GUID from file path */
      ctStringUtf8 name = paths[i].FilePathGetName();
      char guidStr[34];
      memset(guidStr, 34, 0);
      name.CopyToArray(guidStr, 34);
      ctGUID guid = ctGUID(guidStr);
      /* notify all servers */
      for (auto it = resourceServers.GetIterator(); it; it++) {
         it.Value()->NotifyHotReload(guid);
      }
   }

   /* reload nicknames as well */
   ReloadNicknames();

   hotReloadCategory.ClearChanges();
   hotReloadCategory.EndReadingChanges();
}
#endif

ctResourceManager* ctGetResourceManager() {
   return gResourceManager;
}
