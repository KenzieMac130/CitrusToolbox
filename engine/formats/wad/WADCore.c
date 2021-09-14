#include "WADCore.h"
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

enum ctResults ctWADReaderBind(struct ctWADReader* pReader, uint8_t* blob, size_t size) {
   if (!pReader || !blob || !size) { return CT_FAILURE_INVALID_PARAMETER; }
   if (size < sizeof(struct ctWADInfo)) { return CT_FAILURE_OUT_OF_BOUNDS; }
   pReader->blob = blob;
   pReader->blobSize = size;
   pReader->pInfo = (struct ctWADInfo*)blob;
   const int32_t tableOffset = pReader->pInfo->infotableofs;
   const int32_t numLumps = pReader->pInfo->numlumps;
   if (size < tableOffset + sizeof(struct ctWADLump) * numLumps) {
      return CT_FAILURE_OUT_OF_BOUNDS;
   }
   pReader->pLumps = (struct ctWADLump*)(blob + tableOffset);
   return CT_SUCCESS;
}

enum ctResults ctWADFindLump(struct ctWADReader* pReader,
                             const char* name,
                             void** ppDataOut,
                             int32_t* ppSizeOut) {
   if (!pReader->pInfo) { return CT_FAILURE_INACCESSIBLE; }
   const int32_t numlumps = pReader->pInfo->numlumps;
   for (int32_t i = 0; i < numlumps; i++) {
      const struct ctWADLump lump = pReader->pLumps[i];
      if (ctCStrNEql(lump.name, name, 8)) {
         if (ppDataOut) { *ppDataOut = pReader->blob + lump.filepos; }
         if (ppSizeOut) { *ppSizeOut = lump.size; }
         return CT_SUCCESS;
      }
   }
   return CT_FAILURE_NOT_FOUND;
}

enum ctResults ctWADFindLumpInMarker(struct ctWADReader* pReader,
                                     int32_t occurrence,
                                     const char* beginName,
                                     const char* endName,
                                     const char* name,
                                     void** ppDataOut,
                                     int32_t* ppSizeOut) {
   if (!pReader->pInfo) { return CT_FAILURE_INACCESSIBLE; }
   const int32_t numlumps = pReader->pInfo->numlumps;
   int32_t currentOccurence = 0;
   for (int32_t i = 0; i < numlumps; i++) {
      const struct ctWADLump lump = pReader->pLumps[i];
      if (ctCStrNEql(beginName, name, 8)) { currentOccurence++; }
      if (currentOccurence == occurrence) {
         if (ctCStrNEql(lump.name, name, 8)) {
            if (ppDataOut) { *ppDataOut = pReader->blob + lump.filepos; }
            if (ppSizeOut) { *ppSizeOut = lump.size; }
            return CT_SUCCESS;
         }
         if (ctCStrNEql(endName, name, 8)) { return CT_FAILURE_NOT_FOUND; }
      }
   }
   return CT_FAILURE_NOT_FOUND;
}

const char* ctWADGetStringExt(struct ctWADReader* pReader, int32_t offset) {
   if (!pReader) { return NULL; }
   if (!pReader->pInfo) { return NULL; }
   if (!pReader->pInfo->numlumps) { return NULL; }
   const struct ctWADLump lump = pReader->pLumps[pReader->pInfo->numlumps - 1];
   if (!ctCStrNEql(lump.name, "STRINGS", 8)) { return NULL; }
   return pReader->blob + lump.filepos + offset;
}
