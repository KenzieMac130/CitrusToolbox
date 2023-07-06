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

#include "WADCore.h"

#ifdef __cplusplus
extern "C" {
#endif

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
   int32_t currentOccurence = -1;
   bool inMarker = false;
   for (int32_t i = 0; i < numlumps; i++) {
      const struct ctWADLump lump = pReader->pLumps[i];
      if (ctCStrNEql(beginName, lump.name, 8)) {
         currentOccurence++;
         inMarker = true;
      } else if (ctCStrNEql(endName, lump.name, 8)) {
         inMarker = false;
      }
      if (inMarker && currentOccurence == occurrence) {
         if (ctCStrNEql(lump.name, name, 8)) {
            if (ppDataOut) { *ppDataOut = pReader->blob + lump.filepos; }
            if (ppSizeOut) { *ppSizeOut = lump.size; }
            return CT_SUCCESS;
         }
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

enum ctResults ctWADSetupWrite(struct ctWADReader* pReader) {
   if (pReader->pInfo) {
      return CT_FAILURE_DUPLICATE_ENTRY;
   } else {
      pReader->blobSize = sizeof(struct ctWADInfo);
      pReader->blob = (uint8_t*)ctMalloc(pReader->blobSize);
      pReader->pLumps = (struct ctWADLump*)ctMalloc(sizeof(struct ctWADLump) * 1);
      pReader->pInfo = (struct ctWADInfo*)pReader->blob;

      pReader->pInfo->infotableofs = 0;
      pReader->pInfo->numlumps = 0;
      memcpy(pReader->pInfo->identification, "PWAD", 4);
   }
   return CT_SUCCESS;
}

enum ctResults ctWADWriteSection(struct ctWADReader* pReader,
                                 const char name[8],
                                 uint8_t* data,
                                 size_t size) {
   if (!data) { return CT_FAILURE_INVALID_PARAMETER; }
   size_t initialBlobSize = pReader->blobSize;
   pReader->blob = ctRealloc(pReader->blob, pReader->blobSize + size);
   pReader->pInfo = (struct ctWADInfo*)pReader->blob;
   pReader->blobSize += size;
   pReader->pInfo->numlumps++;
   pReader->pLumps =
     ctRealloc(pReader->pLumps, sizeof(struct ctWADLump) * pReader->pInfo->numlumps);
   struct ctWADLump* pLump = &pReader->pLumps[pReader->pInfo->numlumps - 1];
   pLump->filepos = (int32_t)initialBlobSize;
   pLump->size = (int32_t)size;
   memcpy(pLump->name, name, 8);
   memcpy(&pReader->blob[initialBlobSize], data, size);
   return CT_SUCCESS;
}

enum ctResults ctWADToBuffer(struct ctWADReader* pReader, uint8_t* data, size_t* pSize) {
   ctAssert(pSize);
   *pSize = pReader->blobSize + (sizeof(struct ctWADLump) * pReader->pInfo->numlumps);
   pReader->pInfo->infotableofs = (int32_t)pReader->blobSize;
   if (data) {
      memcpy(data, pReader->blob, pReader->blobSize);
      memcpy(&data[pReader->blobSize],
             pReader->pLumps,
             (sizeof(struct ctWADLump) * pReader->pInfo->numlumps));
   }
   return CT_SUCCESS;
}

void ctWADWriteFree(struct ctWADReader* pReader) {
   ctFree(pReader->blob);
   ctFree(pReader->pLumps);
}

#ifdef __cplusplus
}
#endif