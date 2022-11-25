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

/* Hashes are expected to be horner with a prime of 31 */

enum ctLevelVMOp {
	CT_LEVEL_VM_NOOP = 0,
	CT_LEVEL_VM_BEGIN_CONDITIONAL = 1, /* value hash */
	CT_LEVEL_VM_END_CONDITIONAL = 2,
	CT_LEVEL_VM_BEGIN_TOY = 127, /* type, transform */
	CT_LEVEL_VM_END_TOY = 128,
	CT_LEVEL_VM_SET_PROP_INT = 129, /* name hash, value */
	CT_LEVEL_VM_SET_PROP_REAL = 130, /* name hash, value */
	CT_LEVEL_VM_SET_PROP_FLAG = 131, /* name hash, presence implies true */
	CT_LEVEL_VM_SET_PROP_STRING = 132, /* name hash, length, string */
	CT_LEVEL_VM_SET_PROP_VECTOR = 135, /* name hash, value */
	CT_LEVEL_VM_SET_PROP_GUID = 136, /* name hash, value */
	CT_LEVEL_VM_MAX = UINT8_MAX
};

struct ctLevelVMHeader {
	char magic[4]; /* LEVM */
	uint32_t version; /* 1 */
	uint64_t instructionStart;
	char levelName[32]; /* Level name */
};

struct ctLevelVMConditional {
	uint64_t hash;
};

struct ctLevelVMBeginToy {
	uint64_t hash;
	float location[3];
	float rotation[4];
	float scale[3];
};

struct ctLevelVMSetInt {
	uint64_t hash;
	int64_t value;
};

struct ctLevelVMSetReal {
	uint64_t hash;
	double value;
};

struct ctLevelVMSetFlag {
	uint64_t hash;
};

struct ctLevelVMSetString {
	uint64_t hash;
	uint32_t length;
	/* after this is a null terminated utf8 string */
};

struct ctLevelVMSetVector {
	uint64_t hash;
	float values[4];
};

struct ctLevelVMSetGUID {
	uint64_t hash;
	uint8_t data[16];
};

typedef bool (*ctLevelVMGetGlobalFlagCb)(uint64_t hash);
typedef bool (*ctLevelVMCreateToyCb)(uint64_t hash, float position[3], float rotation[4], float scale[3], struct ctToyParams* pParams);

struct ctLevelVMRunInfo {
	ctLevelVMGetGlobalFlagCb fpFlagCallback;
	ctLevelVMCreateToyCb fpCreateToyCallback;
};

ctResults ctLevelVMRun(uint8_t* pBytes, size_t size, ctLevelVMRunInfo* pRunInfo);

#ifdef __cplusplus
}
#endif