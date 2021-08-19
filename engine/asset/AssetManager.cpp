#include "AssetManager.hpp"
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

#include "core/EngineCore.hpp"

#define ASSETSYS_IMPLEMENTATION
#define STRPOOL_IMPLEMENTATION
#pragma warning(push)
#pragma warning(disable : 4996)
#undef _CRT_SECURE_NO_WARNINGS
#include "mattias/assetsys.h"
#pragma warning(pop)

#include <stdio.h>
#include <stdlib.h>

#include "types/WAD.hpp"
#include "types/Mesh.hpp"
#include "types/Texture.hpp"

ctAssetBase*
newAssetOfType(class ctAssetManager* manager, const char* path, const char* type) {
   if (ctCStrEql(type, "wad")) {
      return new ctWADAsset(manager, path);
   } else if (ctCStrEql(type, "mesh")) {
      return new ctMeshAsset(manager, path);
   } else if (ctCStrEql(type, "texture")) {
      return new ctTextureAsset(manager, path);
   }
   return NULL;
}

uint64_t hashAssetPathAndType(const char* assetPath, const char* assetType) {
   struct {
      uint64_t ah;
      uint64_t th;
   } combo;
   combo.ah = ctXXHash64(assetPath);
   combo.th = ctXXHash64(assetType);
   return ctXXHash64(&combo, sizeof(combo), CT_HASH_RESEED_ASSET_PATH);
}

ctAssetManager::ctAssetManager(ctFileSystem* _pFileSystem) {
   pFileSystem = _pFileSystem;
   gcAutoActive = true;
   assetsByHash.Reserve(1024);
}

ctResults ctAssetManager::Startup() {
   ZoneScoped;
   ctDebugLog("Starting Asset Manager...");
   gcAutoActive = true;
   return CT_SUCCESS;
}

ctResults ctAssetManager::Shutdown() {
   ZoneScoped;
   GarbageCollect(Engine->JobSystem);
   return CT_SUCCESS;
}

ctResults ctAssetManager::Update(ctJobSystem* jobs) {
   CT_RETURN_FAIL(LoadMoreData(jobs));
   if (gcAutoActive && !assetsToGC.isEmpty()) { CT_RETURN_FAIL(GarbageCollect(jobs)); }
   return ctResults();
}

ctResults ctAssetManager::LoadMoreData(ctJobSystem* jobs) {
   return CT_SUCCESS;
}

ctResults ctAssetManager::GarbageCollect(ctJobSystem* jobs) {
   for (int i = 0; i < assetsToGC.Count(); i++) {
      ctAssetBase* asset = assetsToGC[i];
      if (asset->isUnreferenced() && asset->isAvailible()) {
         asset->OnLogout();
         asset->OnRelease();
      }
   }
   assetsToGC.Clear();
   return CT_SUCCESS;
}

ctWADAsset* ctAssetManager::GetWADAsset(const char* path) {
   return (ctWADAsset*)FindOrReferenceAsset(path, "wad");
}

void ctAssetManager::_AddAssetToGC(ctAssetBase* pAsset) {
   assetsToGC.Append(pAsset);
}

bool ctAssetManager::_LoadSingleAsset(ctAssetBase* pAsset) {
   /* Immediately load and login the file (this will do for now) */
   ctFile file;
   if (Engine->FileSystem->OpenAssetFile(file, pAsset->GetRelativePath()) == CT_SUCCESS) {
      pAsset->OnLoad(file);
      pAsset->OnLogin();
      file.Close();
      return true;
   }
   ctDebugError("Failed to load asset: %s", pAsset->GetRelativePath());
   return false;
}

ctAssetBase* ctAssetManager::FindOrReferenceAsset(const char* path, const char* type) {
   const uint64_t combinedHash = hashAssetPathAndType(path, type);
   ctAssetBase** ppFoundEntry = assetsByHash.FindPtr(combinedHash);
   if (ppFoundEntry) {
      if (!ctCStrEql((*ppFoundEntry)->GetRelativePath(), path) ||
          !ctCStrEql((*ppFoundEntry)->GetAssetType(), type)) {
         ctFatalError(-1,
                      "ASSET RESOLVE HASH COLLISION: {%s %s} == {%s %s} (solution: "
                      "change name/rehash)",
                      path,
                      type,
                      (*ppFoundEntry)->GetRelativePath(),
                      (*ppFoundEntry)->GetAssetType());
      }
      (*ppFoundEntry)->Reference();
      return *ppFoundEntry;
   }
   ctAssetBase* newAsset = newAssetOfType(this, path, type);
   if (!newAsset) {
      ctDebugError("Failed to create asset \"%s\" of type: \"%s\"", path, type);
      return NULL;
   }
   newAsset->Reference();

   assetsByHash.Insert(combinedHash, newAsset);
   return newAsset;
}

ctAssetBase::ctAssetBase(class ctAssetManager* _manager, const char* _relativePath) {
   pManager = _manager;
   relativePath = _relativePath;
   references = 0;
}

ctAssetBase::~ctAssetBase() {
}

bool ctAssetBase::isIdeal() {
   return isAvailible();
}

bool ctAssetBase::isUnreferenced() {
   return references <= 0;
}

int32_t ctAssetBase::GetReferenceCount() {
   return references;
}

bool ctAssetBase::LoadAndWait() {
   if (isIdeal()) { return true; }
   if (isUnreferenced()) { return false; }
   if (!pManager) { return false; }
   return pManager->_LoadSingleAsset(this);
}

const char* ctAssetBase::GetRelativePath() {
   return relativePath.CStr();
}

ctResults ctAssetBase::OnLoad(ctFile& file) {
   return CT_SUCCESS;
}

ctResults ctAssetBase::OnLogin() {
   return CT_SUCCESS;
}

ctResults ctAssetBase::OnLogout() {
   return CT_SUCCESS;
}

ctResults ctAssetBase::OnRelease() {
   return CT_SUCCESS;
}

void ctAssetBase::Reference() {
   references++;
}

void ctAssetBase::Dereferene() {
   references--;
   if (references <= 0) { pManager->_AddAssetToGC(this); }
}
