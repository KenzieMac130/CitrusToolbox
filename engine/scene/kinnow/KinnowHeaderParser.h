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
#include "KinnowReflection.h"

typedef struct ctKinnowReflectParseContext {
   size_t enumCount;
   size_t structCount;
   size_t enumEntryCount;
   size_t structEntryCount;
   size_t arenaNext;
   size_t arenaCapacity;
   ctKinnowReflectEnumField* pEnumFields;
   ctKinnowReflectStructField* pStructFields;
   ctKinnowReflectEnumEntryField* pEnumEntries;
   ctKinnowReflectStructEntryField* pStructEntries;
   uint8_t* pArena;
};

enum ctResults ctKinnowReflectParseCHeader(ctKinnowReflectParseContext* pCtx,
                                           const char* header,
                                           size_t length,
                                           size_t capacity);
enum ctResults ctKinnowReflectParseSerialize(ctKinnowReflectParseContext* pCtx,
                                             uint8_t* pDest,
                                             size_t capacity);
enum ctResults ctKinnowReflectParseDeserialize(ctKinnowReflectParseContext* pCtx,
                                               uint8_t* pSrc,
                                               size_t capacity);
enum ctResults ctKinnowReflectParseContextFree(ctKinnowReflectParseContext* pCtx);