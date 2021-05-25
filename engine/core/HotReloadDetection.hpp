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

#pragma once

#include "utilities/Common.h"
#include "ModuleBase.hpp"

class ctHotReloadCategory {
public:
   void RegisterPath(const char* relativePath);
   void UnregisterPath(const char* relativePath);

   bool isContentUpdated();
   void BeginReadingChanges();
   const ctDynamicArray<ctStringUtf8>& GetUpdatedPaths() const;
   void EndReadingChanges();
   void ClearChanges();

   void _AddFileUpdate(const char* path);
   class ctHotReloadDetection* _pOwner;
protected: 

private:
   ctDynamicArray<uint64_t> watchedPathHashes;
   ctDynamicArray<ctStringUtf8> updatedPaths;
};

class ctHotReloadDetection : public ctModuleBase {
public:
   ctResults Startup() final;
   ctResults Shutdown() final;

   ctResults RegisterAssetCategory(ctHotReloadCategory* pCategory);

   ctMutex _callbackLock;
   void _PushPathUpdate(const char* path);

private:
   ctDynamicArray<ctHotReloadCategory*> hotReloads;
   int32_t watchEnable;
   bool watchIsRunning;
};