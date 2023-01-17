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

#include "MO.h"

#include "utilities/BloomFilter.hpp"

struct ctMOReaderSearchStructure {
   ctBloomFilter<uint32_t, 64, 3> bloom;
   ctHashTable<int32_t, uint32_t> table;
};

ctResults ctMOReaderInitialize(ctMOReader* pReader, uint8_t* blob, size_t size) {
   pReader->blob = (uint8_t*)ctMalloc(size);
   memcpy(pReader->blob, blob, size);
   pReader->blobSize = size;
   pReader->searchStructure = NULL;
   pReader->pHeader = (ctMOHeader*)pReader->blob;
   if (pReader->pHeader->magic != CT_MO_MAGIC) { return CT_FAILURE_CORRUPTED_CONTENTS; }
   if (pReader->pHeader->version != CT_MO_CUR_VER) { return CT_FAILURE_UNKNOWN_FORMAT; }
   pReader->pOriginal =
     (ctMOStringEntry*)&pReader->blob[pReader->pHeader->originalOffset];
   pReader->pTranslated =
     (ctMOStringEntry*)&pReader->blob[pReader->pHeader->translatedOffset];
   pReader->searchStructure = new ctMOReaderSearchStructure();

   /* build table */
   ctMOReaderSearchStructure* pSearchStruct =
     (ctMOReaderSearchStructure*)pReader->searchStructure;
   for (int32_t i = 0; i < pReader->pHeader->numStrings; i++) {
      uint32_t hash =
        ctXXHash32((const char*)&pReader->blob[pReader->pOriginal[i].offset]);
      pSearchStruct->bloom.Insert(hash);
      pSearchStruct->table.Insert(hash, i);
   }
   return CT_SUCCESS;
}

ctResults ctMOReaderRelease(ctMOReader* pReader) {
   ctFree(pReader->blob);
   pReader->blob = NULL;
   pReader->blobSize = 0;
   pReader->searchStructure = NULL;
   delete (ctMOReaderSearchStructure*)pReader->searchStructure;
   return ctResults();
}

const char* ctMOFindTranslation(ctMOReader* pReader, const char* original) {
   if (!pReader->searchStructure) { return NULL; }
   uint32_t hash = ctXXHash32(original);
   ctMOReaderSearchStructure* pSearchStruct =
     (ctMOReaderSearchStructure*)pReader->searchStructure;
   if (!pSearchStruct->bloom.MightExist(hash)) { return NULL; }
   ctMOHeader* pHeader = pReader->pHeader;
   ctMOStringEntry* pOriginal = pReader->pOriginal;
   ctMOStringEntry* pTranslated = pReader->pTranslated;
   int32_t* pIdx = pSearchStruct->table.FindPtr(hash);
   if (!pIdx) { return NULL; }
   return (const char*)&pReader->blob[pTranslated[*pIdx].offset];
}