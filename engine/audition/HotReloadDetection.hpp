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

class CT_API ctHotReloadCategory {
public:
   void RegisterData(const ctGUID& guid);
   void UnregisterData(const ctGUID& guid);
   void RegisterPath(const char* relativePath);
   void UnregisterPath(const char* relativePath);
   void Reset();

   bool isContentUpdated();
   void BeginReadingChanges();
   const ctDynamicArray<ctStringUtf8>& GetUpdatedPaths() const;
   void EndReadingChanges();
   void ClearChanges();

   void _AddFileUpdate(const char* path);
   class ctHotReloadDetection* _pOwner;

protected:
private:
   ctDynamicArray<uint64_t> watchedPathHashes; /* todo: hash set */
   ctDynamicArray<ctStringUtf8> updatedPaths;
};

class CT_API ctHotReloadDetection : public ctModuleBase {
public:
   ctResults Startup() final;
   ctResults Shutdown() final;
   const char* GetModuleName() final;
   virtual void DebugUI(bool useGizmos);

   ctResults RegisterDataCategory(ctHotReloadCategory* pCategory);

protected:
   friend class ctHotReloadCategory;
   static void WatchCallback(uint32_t watch_id,
                             uint32_t action,
                             const char* rootdir,
                             const char* filepath,
                             const char* oldfilepath,
                             void* user);
   ctMutex callbackLock;
   void PushPathUpdate(const char* path);

   ctDynamicArray<ctHotReloadCategory*> hotReloads;
   int32_t watchEnable;
   bool watchIsRunning;
};