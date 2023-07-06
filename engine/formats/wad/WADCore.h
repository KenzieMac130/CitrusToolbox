/*
   Copyright 2022 MacKenzie Strand

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

struct CT_API ctWADInfo {
   char identification[4];
   int32_t numlumps;
   int32_t infotableofs;
};

struct CT_API ctWADLump {
   int32_t filepos;
   int32_t size;
   char name[8];
};

struct CT_API ctWADReader {
   struct ctWADInfo* pInfo;
   struct ctWADLump* pLumps;
   uint8_t* blob;
   size_t blobSize;
};

enum ctResults ctWADReaderBind(struct ctWADReader* pReader, uint8_t* blob, size_t size);
enum ctResults ctWADFindLump(struct ctWADReader* pReader,
                             const char* name,
                             void** ppDataOut,
                             int32_t* ppSizeOut);
enum ctResults ctWADFindLumpInMarker(struct ctWADReader* pReader,
                                     int32_t occurrence,
                                     const char* beginName,
                                     const char* endName,
                                     const char* name,
                                     void** ppDataOut,
                                     int32_t* ppSizeOut);

#define CT_WADBLOB_NAME_STRINGS "STRINGS"
const char* ctWADGetStringExt(struct ctWADReader* pReader, int32_t offset);

enum ctResults ctWADSetupWrite(struct ctWADReader* pReader);
enum ctResults ctWADWriteSection(struct ctWADReader* pReader,
                                 const char name[8],
                                 uint8_t* data,
                                 size_t size);
enum ctResults ctWADToBuffer(struct ctWADReader* pReader, uint8_t* data, size_t* pSize);
void ctWADWriteFree(struct ctWADReader* pReader);

#ifdef __cplusplus
}
#endif