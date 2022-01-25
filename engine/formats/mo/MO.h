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

/* https://www.gnu.org/software/gettext/manual/html_node/MO-Files.html */

#include "utilities/Common.h"

#define CT_MO_MAGIC 0xde120495
#define CT_MO_CUR_VER 0

struct ctMOHeader {
    int32_t magic;
    int32_t version;
    int32_t numStrings;
    int32_t originalOffset;
    int32_t translatedOffset;
    int32_t _unused1; /* baked hash table size */
    int32_t _unused2; /* baked hash table offset */
}

struct ctMOStringEntry {
    int32_t length;
    int32_t offset;
}

struct CT_API ctMOReader {
    struct ctMOHeader* pHeader;
    struct ctMOStringEntry* pOriginal;
    struct ctMOStringEntry* pTranslated;
    uint8_t* blob;
    size_t blobSize;
    
    void* searchStructure;
};

enum ctResults ctMOReaderInitialize(struct ctWADReader* pReader, uint8_t* blob, size_t size);
enum ctResults ctMOReaderRelease(struct ctWADReader* pReader, uint8_t* blob, size_t size);

const char* ctMOFindTranslation(struct ctWADReader* pReader, const char* original);
const char* ctMOFindTranslationPlural(struct ctWADReader* pReader, const char* original, uint32_t number);