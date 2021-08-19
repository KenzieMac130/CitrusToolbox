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

struct CT_API ctKeyLimeStreamSubmesh {
   int32_t idxOffset;
   int32_t idxCount;
   int32_t matIdx;
};

struct CT_API ctKeyLimeStreamPosition {
   float position[3];
};

struct CT_API ctKeyLimeStreamNormalTangent {
   int32_t normal;
   int32_t tangent;
};

struct CT_API ctKeyLimeStreamUV {
   float uv[2];
};

struct CT_API ctKeyLimeStreamColor {
   uint8_t color[4];
};

struct CT_API ctKeyLimeStreamSkin {
   uint16_t weight[4];
   uint16_t boneIdx[4];
};

struct CT_API ctKeyLimeStreamTransform {
   float position[3];
   float orientation[4];
   float scale[3];
};