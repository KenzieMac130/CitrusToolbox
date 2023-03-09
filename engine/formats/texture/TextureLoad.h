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

#ifdef __cplusplus
extern "C" {
#endif

enum ctTextureLoadSrc {
   CT_TEXTURELOAD_TINYKTX,
   CT_TEXTURELOAD_TINYDDS,
   CT_TEXTURELOAD_STB
};

enum ctTextureLoadType {
   CT_TEXTURELOAD_1D,
   CT_TEXTURELOAD_2D,
   CT_TEXTURELOAD_3D,
   CT_TEXTURELOAD_CUBEMAP
};

struct ctTextureLoadCtx {
   enum ctTextureLoadSrc src;
   uint32_t width;
   uint32_t height;
   uint32_t depth;
   uint32_t mips;
   enum TinyImageFormat format;
   enum ctTextureLoadType type;
   const void* levels[CT_MAX_MIP_LEVELS];
   void* loaderdata;
};

enum ctResults ctTextureLoadFromFile(ctFile& file, struct ctTextureLoadCtx* pCtx);
void ctTextureLoadCtxRelease(struct ctTextureLoadCtx* pCtx);

#ifdef __cplusplus
}
#endif