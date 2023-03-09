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
#include "formats/wad/WADCore.h"

#include "../extensions/Skeleton.hpp"
#include "../extensions/RenderMesh.hpp"

struct ctModelHeader {
   uint64_t wadOffset;
   uint64_t wadSize;
   uint64_t gpuOffset;
   uint64_t gpuSize;
};

struct ctModelT {
   ctModelHeader header;

   uint64_t inMemoryWadDataSize;
   uint8_t* inMemoryWadData;

   bool stringDataMapped;
   uint64_t stringPoolSize;
   char* stringPool;

   ctModelSkeleton skeleton;
   ctModelMeshData meshData;
};