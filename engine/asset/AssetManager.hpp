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
#include "core/ModuleBase.hpp"
#include "core/FileSystem.hpp"

class CT_API ctAssetBase {
public:
   /* Do NOT attempt to load at construction */
   ctAssetBase(class ctAssetManager* manager, const char* relativePath);
   ~ctAssetBase();

   virtual bool isAvailible() = 0; /* Is availible (loaded) */
   virtual bool isIdeal();         /* Is ideal (fully loaded) */

   virtual const char* GetRelativePath();
   virtual const char* GetAssetType() = 0;

   virtual ctResults OnLoad(ctFile& file);
   virtual ctResults OnLogin();
   virtual ctResults OnLogout();
   virtual ctResults OnRelease();

   void Reference();
   void Dereferene();
   bool isUnreferenced();
   int32_t GetReferenceCount();

   bool LoadAndWait();

private:
   class ctAssetManager* pManager;
   ctStringUtf8 relativePath;
   int32_t references;
};

class CT_API ctAssetManager : public ctModuleBase {
public:
   ctAssetManager(ctFileSystem* _pFileSystem);
   virtual ctResults Startup();
   virtual ctResults Shutdown();

   ctResults Update(class ctJobSystem* jobs);
   ctResults LoadMoreData(class ctJobSystem* jobs);
   ctResults GarbageCollect(class ctJobSystem* jobs);

   // ctResults MountDirectory(const char* path, const char* virtualPath);
   // ctResults DismountDirectory(const char* path, const char* virtualPath);

   class ctWADAsset* GetWADAsset(const char* path);

   void _AddAssetToGC(ctAssetBase* pAsset);
   bool _LoadSingleAsset(ctAssetBase* pAsset);

   int32_t gcAutoActive;

private:
   ctAssetBase* ctAssetManager::FindOrReferenceAsset(const char* path, const char* type);
   ctFileSystem* pFileSystem;
   // struct assetsys_t* pAssetSystem;
   ctHashTable<ctAssetBase*, uint64_t> assetsByHash;
   ctDynamicArray<ctAssetBase*> assetsToGC;
};