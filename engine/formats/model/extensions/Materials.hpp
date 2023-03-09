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

struct ctModelTextureInfo {
   int32_t textureIdx;
   int32_t uvChannel;
   float offset[2];
   float scale[2];
   float scroll[2];
};

struct ctModelMaterial {
   char materialName[32];
   char shaderName[32];
   char surfaceName[32];

   float baseColorTint[3];
   float alphaModifier;

   float roughnessModifier;
   float indexOfRefraction;
   float metalModifier;
   float normalModifier;
   float emissionColor[3];
   float occlusionModifier;

   float auxProperties[16];

   TextureInfo baseColorTexture;
   TextureInfo pbrTexture;
   TextureInfo normalTexture;
   TextureInfo auxTexture1;
   TextureInfo auxTexture2;
   TextureInfo auxTexture3;
   TextureInfo auxTexture4;
};

/* reading */


/* authoring */