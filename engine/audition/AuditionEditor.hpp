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

class CT_API ctAuditionSpaceContext {
public:
   ctStringUtf8 assetBasePath = "";
   ctStringUtf8 selectedAssetPath = "";
   ctStringUtf8 selectedResourcePath = "";
   ctStringUtf8 hoveredResourcePath = "";
   ctStringUtf8 currentFolder = "";
   bool allowGizmo = true;
   class ctEngineCore* Engine;
   class ctAuditionEditor* Editor;
   class ctAssetCompilerBootstrap* Compiler;
};

class CT_API ctAuditionEditor : public ctModuleBase {
public:
   ctResults Startup() final;
   ctResults Shutdown() final;
   const char* GetModuleName() final;

   void UpdateEditor();
   ctResults TryOpenEditorForAsset(const char* path);
   ctResults TryOpenHexViewerForResource(ctGUID guid);
   ctResults TryOpenHexViewerForFile(const char* path);
   ctResults AddDynamicSpace(class ctAuditionSpaceBase* pSpace);
   static ctResults GetTypeForPath(const char* path, char dest[32]);

protected:
   class ctAuditionSpaceCompiler* pCompilerWindow;
   class ctAuditionSpaceAssetBrowser* pAssetBrowser;

   void RegisterModule(class ctModuleBase* pModule);
   ctDynamicArray<class ctAuditionModuleInspector*> pModuleInspectors;
   ctDynamicArray<class ctAuditionSpaceBase*> pDynamicSpaces;
   void GarbageCollectDynamicSpaces();
   ctResults AddAssetEditor(class ctAuditionSpaceAssetEditorBase*, const char* path);

private:
   ctAuditionSpaceContext ctx;
   int allowEditor = true;
   int isHidden = false;
   ctStringUtf8 assetPath = CT_AUDITION_ASSET_PATH;
};