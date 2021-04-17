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

/* API Main Goal: Keep the interface simple for non-graphics programmers! */

#include "core/ModuleBase.hpp"
#include "utilities/Common.h"

typedef uint32_t ctKeyLimeMaterialFlags;
enum ctKeyLimeMaterialFlagBits {
   CT_KEYLIME_MATFLAG_NONE = 0x0000,
   CT_KEYLIME_MATFLAG_DOUBLE_SIDED = 0x0001,
   CT_KEYLIME_MATFLAG_BLEND = 0x0002,
   CT_KEYLIME_MATFLAG_CLIP = 0x0004,
   CT_KEYLIME_MATFLAG_AUX_ENABLE = 0x0008,
   CT_KEYLIME_MATFLAG_AUX_MATCAP = 0x0010,
   CT_KEYLIME_MATFLAG_AUX_DISTORT = 0x0020,
   CT_KEYLIME_MATFLAG_AUX_ADD = 0x0040,
   CT_KEYLIME_MATFLAG_AUX_UV2 = 0x0080,
   CT_KEYLIME_MATFLAG_AUX_FLIPBOOK = 0x0100,
   CT_KEYLIME_MATFLAG_BASE_ENABLE = 0x0200,
   CT_KEYLIME_MATFLAG_BASE_MATCAP = 0x0400,
   CT_KEYLIME_MATFLAG_BASE_ADD = 0x0800,
   CT_KEYLIME_MATFLAG_BASE_UV2 = 0x1000,
   CT_KEYLIME_MATFLAG_BASE_FLIPBOOK = 0x2000,
   CT_KEYLIME_MATFLAG_BASE_YCBCR = 0x4000,
   CT_KEYLIME_MATFLAG_LIT = 0x8000,
   CT_KEYLIME_MATFLAG_LIGHT_PALETTE = 0x10000,
   CT_KEYLIME_MATFLAG_GLOW = 0x20000,
   CT_KEYLIME_MATFLAG_REFRACT = 0x40000,
};

struct ctKeyLimeMaterialDesc {
   uint32_t baseTexture;
   uint32_t auxTexture;

   uint32_t lightPalette;
   ctKeyLimeMaterialFlags flags;

   ctVec4 baseTint;
   ctVec4 auxTint;
   ctVec3 specTint;
   float specPower;
   ctVec3 emissionColor;
   float glowStrength;

   ctVec2 baseUVOffset;
   ctVec2 baseUVScale;
   ctVec2 baseUVScroll;
   ctVec2 auxUVOffset;
   ctVec2 auxUVScale;
   ctVec2 auxUVScroll;

   float alphaClip;
};

typedef uint32_t ctKeyLimeGeometryFlags;
enum ctKeyLimeGeometryFlagBits {
   CT_KEYLIME_GEOFLAG_NONE = 0x0000,
   CT_KEYLIME_GEOFLAG_SKINNED = 0x0001,
};

struct ctKeyLimeGeometryDesc {
   uint32_t indexCount;

   uint32_t indexArray;
   uint32_t positionArray;
   uint32_t normalArray;
   uint32_t uvArray;
   uint32_t uv2Array;
   uint32_t colorArray;
   uint32_t skinningArray;

   uint32_t boneArray;

   ctKeyLimeGeometryFlags flags;
};

typedef uint32_t ctKeyLimeInstanceFlags;
enum ctKeyLimeInstanceFlagBits {
   CT_KEYLIME_INSTFLAG_NONE = 0x0000,
   CT_KEYLIME_INSTFLAG_CAST_SHADOW = 0x0001,
   CT_KEYLIME_INSTFLAG_TRANSPARENT_SORT = 0x0002,
   CT_KEYLIME_INSTFLAG_DEPTH_BIASED = 0x0004,
};

struct ctKeyLimeInstanceDesc {
   ctVec3 location;
   float sortBias;
   ctVec4 rotation;
   ctVec3 scale;
   float boundSphere;
   uint32_t layerVisibilityBits;
   ctKeyLimeInstanceFlagBits instanceFlags;
   uint64_t _reserved; /* Reserved for storing pipeline hash */
};

struct ctKeyLimeRenderableDesc {
   ctKeyLimeGeometryDesc* pGeometryDesc;
   ctKeyLimeMaterialDesc* pMatDesc;
};

class ctKeyLimeRenderer : public ctModuleBase {
public:
   ctResults Startup() final;
   ctResults Shutdown() final;

   ctResults RenderFrame();

#ifdef CITRUS_GFX_VULKAN
   class ctVkKeyLimeCore* vkKeyLime;
#endif
};