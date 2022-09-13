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
#define CT_KINNOW_STRUCT(_NAME) _NAME();
#else
#define CT_KINNOW_STRUCT(_NAME)
#endif

typedef enum ctKinnowReflectDataType {
   CT_KINNOW_REFLECT_UNDEFINED = 0,
   CT_KINNOW_REFLECT_ENUM = 1,
   CT_KINNOW_REFLECT_STRUCT = 2,
   CT_KINNOW_REFLECT_INT8 = 3,
   CT_KINNOW_REFLECT_INT16 = 4,
   CT_KINNOW_REFLECT_INT32 = 5,
   CT_KINNOW_REFLECT_INT64 = 6,
   CT_KINNOW_REFLECT_UINT8 = 7,
   CT_KINNOW_REFLECT_UINT16 = 8,
   CT_KINNOW_REFLECT_UINT32 = 9,
   CT_KINNOW_REFLECT_UINT64 = 10,
   CT_KINNOW_REFLECT_CHAR8 = 11,
   CT_KINNOW_REFLECT_BOOL = 12,
   CT_KINNOW_REFLECT_FLOAT = 13,
   CT_KINNOW_REFLECT_DOUBLE = 14,
   CT_KINNOW_REFLECT_VEC2 = 15,
   CT_KINNOW_REFLECT_VEC3 = 16,
   CT_KINNOW_REFLECT_VEC4 = 17,
   CT_KINNOW_REFLECT_QUAT = 18,
   CT_KINNOW_REFLECT_GUID = 19,
   CT_KINNOW_REFLECT_ENTITY = 20,
   CT_KINNOW_REFLECT_MAX = UINT32_MAX
} ctKinnowReflectDataType;

typedef enum ctKinnowReflectFlagBits {
   CT_KINNOW_REFLECT_IS_POINTER = 0x0001,
   CT_KINNOW_REFLECT_IS_CONST = 0x0002,
   CT_KINNOW_REFLECT_IS_BITFIELD = 0x0004,
   CT_KINNOW_REFLECT_IS_HIDDEN = 0x0008,
   CT_KINNOW_REFLECT_IS_EDITOR_READ_ONLY = 0x0010,
   CT_KINNOW_REFLECT_IS_NO_LOAD = 0x0020,
   CT_KINNOW_REFLECT_IS_EMBED_BLOB = 0x0040,
   CT_KINNOW_REFLECT_IS_COPY_BLOB = 0x0080
} ctKinnowReflectFlagBits;
typedef int32_t ctKinnowReflectFlags;

typedef struct ctKinnowReflectEnumEntryField {
   const char* name;
   const char* label;
   const char** oldnames;
   ctKinnowReflectFlags flags;
   int64_t value;
} ctKinnowReflectEnumEntryField;

typedef struct ctKinnowReflectStructEntryField {
   const char* name;             /* Variable name */
   const char* utype;            /* Struct/enum type */
   const char* label;            /* CT_LABEL */
   const char* units;            /* CT_UNITS */
   const char* docs;             /* CT_DOCUMENTATION */
   const char* varlen;           /* CT_LENGTH_VAR */
   const char** oldnames;        /* Chain of CT_OLD_NAME ending in null */
   ctKinnowReflectDataType type; /* Variable type */
   ctKinnowReflectFlags flags;   /* CT_HIDDEN and CT_READ_ONLY */
   int32_t offset;               /* Offset from start for struct */
   int32_t arraymax;             /* Max size of the array */
   double min;                   /* CT_MIN */
   double max;                   /* CT_MAX */
   double softmin;               /* CT_SOFT_MIN */
   double softmax;               /* CT_SOFT_MAX */
} ctKinnowReflectStructEntryField;

typedef struct ctKinnowReflectEnumField {
   const char* name;
   const char** oldnames;
   ctKinnowReflectFlags flags;
   ctKinnowReflectEnumEntryField* fields;
} ctKinnowReflectEnumField;

typedef struct ctKinnowReflectStructField {
   const char* name;
   const char** oldnames;
   const char* componentName;
   ctKinnowReflectFlags flags;
   size_t size;
   ctKinnowReflectStructEntryField* fields;
} ctKinnowReflectStructField;

typedef struct ctKinnowReflectHeader {
   char magic[4];
   uint32_t version;
   ctKinnowReflectEnumField* pEnums;
   ctKinnowReflectStructField* pStructs;
};

#ifdef __cplusplus
}
#endif