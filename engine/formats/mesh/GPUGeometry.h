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

struct ctGeometryFormatHeader {
   char magic[4];
   int32_t flags;
   int32_t alignment;

   float cener[3];
   float radius;

   uint32_t submeshCount;
   uint32_t indexCount;
   uint32_t vertexCount;
   uint32_t uvChannelCount;
   uint32_t colorChannelCount;

   uint64_t submeshOffset;
   uint64_t indexOffset;
   uint64_t positionOffset;
   uint64_t tangentNormalOffset;
   uint64_t skinOffset;
   uint64_t uvOffsets[4];
   uint64_t colorOffsets[4];
};