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

#ifdef __cplusplus
extern "C" {
#endif

enum ctTextureLoadSrc {
   CT_TEXTURELOAD_TINYKTX,
   CT_TEXTURELOAD_TINYDDS,
   CT_TEXTURELOAD_CUBE,
   CT_TEXTURELOAD_STB,
   CT_TEXTURELOAD_CUSTOM
};

/* Contex that allows loading textures from various file sources */
struct ctTextureLoadCtx {
   enum ctTextureLoadSrc src;
   struct {
      int width;
      int height;
      int depth;
      int mips;
      int format;
      void* data;
      void* userData;
   } memory;
   bool availible;
   size_t progress;
};

enum ctResults ctLoadTextureFromFile(const char* path, struct ctTextureLoadCtx* pCtx);
void ctTextureLoadCtxRelease(struct ctTextureLoadCtx* pCtx);

#ifdef __cplusplus
}
#endif