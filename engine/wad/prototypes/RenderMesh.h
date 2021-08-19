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

struct CT_API ctWADProtoRenderMeshInstance {
   int32_t boneIdx;
   int32_t meshIdx;
};

struct CT_API ctWADProtoRenderMesh {
   int32_t flags;

   uint32_t submeshCount;
   uint32_t vertexCount;
   uint32_t indexCount;

   int64_t submeshStreamOffset;
   int64_t indexStreamOffset;
   int64_t positionStreamOffset;
   int64_t normalTangentStreamOffset;
   int64_t skinStreamOffset;
   int64_t uvStreamOffsets[4];
   int64_t colorStreamOffsets[4];
};