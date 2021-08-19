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
#include "asset/AssetManager.hpp"

class CT_API ctTextureAsset : public ctAssetBase {
public:
    ctTextureAsset(class ctAssetManager* manager, const char* relativePath);
   virtual const char* GetAssetType();

   virtual bool isAvailible();

   virtual ctResults OnLoad(ctFile& file);
   virtual ctResults OnRelease();
};