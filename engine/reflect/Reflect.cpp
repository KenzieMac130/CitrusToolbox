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

#include "Reflect.hpp"

ctReflector::~ctReflector() {
   for (size_t i = 0; i < children.Count(); i++) {
      delete children[i];
   }
}

#define CT_REFLECT_SAVE_VERSION 1
struct ctReflectHeader {
   uint32_t version;
   uint32_t reflectorCount;
};

struct ctReflectorSectionHeader {
   uint32_t seqId;
   uint32_t parentSeqId;
   uint32_t propCount;
   uint32_t type;
};

/* Format: */
/* ctReflectorHeader */
/* HASHES (propCount * 4) */
/* Null terminated string list (propCount) */

ctResults ctReflector::LoadBinaryContents(ctFile& file) {
   uint32_t version = 0;
   ctReflectorSectionHeader header = {};
   file.ReadRaw(&header, sizeof(header), 1);
   seqId = header.seqId;
   parentSeqId = header.parentSeqId;
   type = (ctReflectorType)type;

   uint32_t* pHashes = (uint32_t*)ctMalloc(sizeof(uint32_t) * header.propCount);
   file.ReadRaw(pHashes, sizeof(uint32_t), header.propCount);

   for (uint32_t i = 0; i < header.propCount; i++) {
      ctStringUtf8 str = file.ReadLine(0);
      properties.Insert(pHashes[i], str);
   }
   ctFree(pHashes);
   return CT_SUCCESS;
}

ctResults ctReflector::LoadBinaryContentsTree(ctFile& file) {
   /* reset */
   *this = ctReflector();

   /* load header */
   ctReflectHeader header = {};
   file.Seek(-(int64_t)sizeof(ctReflectHeader), CT_FILE_SEEK_END);
   file.ReadRaw(&header, sizeof(ctReflectHeader), 1);
   file.Seek(0, CT_FILE_SEEK_SET);

   /* setup parent lookup table */
   ctHashTable<ctReflector*, uint32_t> storage;
   storage.Reserve(header.reflectorCount);

   /* load contents for each reflector */
   for (uint32_t i = 0; i < header.reflectorCount; i++) {
      ctReflector* pReflector;
      if (i == 0) {
         pReflector = this;
      } else {
         pReflector = new ctReflector();
      }
      pReflector->LoadBinaryContents(file);
      storage.Insert((uint32_t)pReflector->seqId + 1, pReflector);
      ctReflector** ppParent = storage.FindPtr(pReflector->parentSeqId + 1);
      if (ppParent) {
         pReflector->SetParent(*ppParent);
      } else {
         pReflector->pParent = NULL;
      }
   }
   return CT_SUCCESS;
}

ctResults ctReflector::SaveBinaryContents(ctFile& file) {
   ctReflectorSectionHeader header = {};
   header.seqId = seqId;
   header.parentSeqId = UINT32_MAX;
   if (pParent) {
      header.parentSeqId = pParent->seqId;
      parentSeqId = header.parentSeqId;
   }
   for (auto it = properties.GetIterator(); it; it++) {
      file.WriteRaw(&it.Key(), sizeof(uint32_t), 1);
   }
   for (auto it = properties.GetIterator(); it; it++) {
      file.WriteRaw(it.Value().CStr(), sizeof(char), it.Value().ByteLength() + 1);
   }
   return CT_SUCCESS;
}

ctResults ctReflector::SaveBinaryContentsTree(ctFile& file, uint32_t depth) {
   uint32_t internalCounter = 0;
   if (depth == 0) { GenSeqIds(internalCounter); }
   SaveBinaryContents(file);
   for (size_t i = 0; i < children.Count(); i++) {
      children[i]->SaveBinaryContentsTree(file, depth + 1);
   }
   if (depth == 0) {
      ctReflectHeader header = {};
      header.version = CT_REFLECT_SAVE_VERSION;
      header.reflectorCount = internalCounter;
      file.WriteRaw(&header, sizeof(header), 1);
   }
   return CT_SUCCESS;
}