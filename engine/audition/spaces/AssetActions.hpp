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
#include "SpaceBase.hpp"
#include "../AssetCompilerBootstrap.hpp"

class CT_API ctAuditionSpaceActionNewAsset : public ctAuditionSpaceBase {
public:
   ctAuditionSpaceActionNewAsset(const char* relativePath);
   ~ctAuditionSpaceActionNewAsset();

   virtual const char* GetTabName();
   virtual const char* GetWindowName();
   virtual void OnGui(ctAuditionSpaceContext& ctx);

private:
   ctStringUtf8 relativePath;
   ctStringUtf8 name = "";
   int typeId = 0;
};

class CT_API ctAuditionSpaceActionNewFolder : public ctAuditionSpaceBase {
public:
   ctAuditionSpaceActionNewFolder(const char* relativePath);
   ~ctAuditionSpaceActionNewFolder();

   virtual const char* GetTabName();
   virtual const char* GetWindowName();
   virtual void OnGui(ctAuditionSpaceContext& ctx);

private:
   ctStringUtf8 relativePath;
   ctStringUtf8 name;
};

class CT_API ctAuditionSpaceActionMoveAsset : public ctAuditionSpaceBase {
public:
   ctAuditionSpaceActionMoveAsset(const char* srcPath);
   ~ctAuditionSpaceActionMoveAsset();

   virtual const char* GetTabName();
   virtual const char* GetWindowName();
   virtual void OnGui(ctAuditionSpaceContext& ctx);

private:
   ctStringUtf8 srcPath;
};

class CT_API ctAuditionSpaceActionDeleteAsset : public ctAuditionSpaceBase {
public:
   ctAuditionSpaceActionDeleteAsset(const char* path);
   ~ctAuditionSpaceActionDeleteAsset();

   virtual const char* GetTabName();
   virtual const char* GetWindowName();
   virtual void OnGui(ctAuditionSpaceContext& ctx);

private:
   ctStringUtf8 path;
};

class CT_API ctAuditionSpaceActionDuplicateAsset : public ctAuditionSpaceBase {
public:
   ctAuditionSpaceActionDuplicateAsset(const char* path);
   ~ctAuditionSpaceActionDuplicateAsset();

   virtual const char* GetTabName();
   virtual const char* GetWindowName();
   virtual void OnGui(ctAuditionSpaceContext& ctx);

private:
   ctStringUtf8 path;
};

class CT_API ctAuditionSpaceActionRenameAsset : public ctAuditionSpaceBase {
public:
   ctAuditionSpaceActionRenameAsset(const char* path);
   ~ctAuditionSpaceActionRenameAsset();

   virtual const char* GetTabName();
   virtual const char* GetWindowName();
   virtual void OnGui(ctAuditionSpaceContext& ctx);

private:
   ctStringUtf8 path;
};