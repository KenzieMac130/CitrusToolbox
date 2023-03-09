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

#define CT_MODEL_EXT_SPAWNER 0

enum ctModelSpawnerOpCodes {
   CT_MODEL_SPAWN_OP_NOOP = 0,
   CT_MODEL_SPAWN_OP_NEXT_OBJ = 1,
   CT_MODEL_SPAWN_OP_SET_FLAG = 2,
   CT_MODEL_SPAWN_OP_SET_INT32 = 3,
   CT_MODEL_SPAWN_OP_SET_UINT32 = 4,
   CT_MODEL_SPAWN_OP_SET_INT64 = 5,
   CT_MODEL_SPAWN_OP_SET_UINT64 = 6,
   CT_MODEL_SPAWN_OP_SET_FLOAT = 7,
   CT_MODEL_SPAWN_OP_SET_STRING = 8,
   CT_MODEL_SPAWN_OP_SET_VEC4F = 11
   CT_MODEL_SPAWN_OP_SET_GUID = 12
};

struct ctModelSpawnerOpSetFlag {
   uint32_t hash;
};

struct ctModelSpawnerOpSetInt32 {
   uint32_t hash;
   int32_t value;
};

struct ctModelSpawnerOpSetUInt32 {
   uint32_t hash;
   uint32_t value;
};

struct ctModelSpawnerOpSetInt64 {
   uint32_t hash;
   int64_t value;
};

struct ctModelSpawnerOpSetUInt64 {
   uint32_t hash;
   uint64_t value;
};

struct ctModelSpawnerOpSetFloat {
   uint32_t hash;
   float value;
};

struct ctModelSpawnerOpSetString {
   uint32_t hash;
   int32_t stringOffset;
};

struct ctModelSpawnerOpSetVec4f {
   uint32_t hash;
   float value[4];
};

struct ctModelSpawnerOpSetGUID {
   uint32_t hash;
   uint8_t value[16];
};

size_t ctModelSpawnerOpCodesSize[] = {
   4, /* NOOP */
   4, /* NEXT_OBJ */
   sizeof(ctModelSpawnerOpSetFlag), 
   sizeof(ctModelSpawnerOpSetInt32),
   sizeof(ctModelSpawnerOpSetUInt32),
   sizeof(ctModelSpawnerOpSetInt64),
   sizeof(ctModelSpawnerOpSetUInt64),
   sizeof(ctModelSpawnerOpSetFloat),
   sizeof(ctModelSpawnerOpSetString),
   sizeof(ctModelSpawnerOpSetVec4f),
   sizeof(ctModelSpawnerOpSetGUID)
};

/* reading */


/* authoring */