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

#include "CitrusPackage.h"

/* ------------- Write Internals ------------- */

class ctPackageWriterContextInternal {
public:
   ctPackageWriterContextInternal(const char* path) {
      file.Open(path, CT_FILE_OPEN_WRITE);
      file.Seek(sizeof(ctPackageHeader), CT_FILE_SEEK_SET);
   }
   ctResults WriteSection(const char* path,
                          const ctGUID* guidPtr,
                          size_t size,
                          const void* data,
                          ctPackageCompression compressionMode);
   ctResults Finish();

private:
   ctFile file;
   ctDynamicArray<ctPackageSection> sections;
};

ctResults
ctPackageWriterContextInternal::WriteSection(const char* path,
                                             const ctGUID* guidPtr,
                                             size_t size,
                                             const void* data,
                                             ctPackageCompression compressionMode) {
   if (!file.isOpen()) { return CT_FAILURE_INACCESSIBLE; }
   ctPackageSection section = {0};
   if (path) { section.pathHash = ctXXHash64(path); }
   if (guidPtr) { memcpy(section.guidData, guidPtr->data, 16); }
   section.blobOffset = (uint64_t)file.Tell();
   section.compressionMode = (int32_t)compressionMode;
   if (compressionMode == CT_PACKAGE_COMPRESSION_NONE) {
      section.blobSize = (uint64_t)size;
      memset(section.compressionMetadata, 0, sizeof(section.compressionMetadata));
      file.WriteRaw(data, size, 1);
   } else if (compressionMode == CT_PACKAGE_COMPRESSION_LZ4_BLOCK) {
      // Todo
   }
   sections.Append(section);
   return CT_SUCCESS;
}

ctResults ctPackageWriterContextInternal::Finish() {
   file.WriteRaw(sections.Data(), sizeof(ctPackageSection), sections.Count());
   file.Seek(0, CT_FILE_SEEK_SET);
   ctPackageHeader header = {0};
   header.magic = CT_PACKAGE_MAGIC;
   header.version = CT_PACKAGE_VERSION;
   header.sectionCount = (uint64_t)sections.Count();
   header.sectionOffset = (uint64_t)file.Tell();
   file.WriteRaw(&header, sizeof(header), 1);
   file.Close();
   return CT_SUCCESS;
}

/* ------------- Read Internals ------------- */

struct ctPackageReadMetadata {
public:
   ctPackageReadMetadata() {
      memset(fullPath, 0, sizeof(CT_MAX_FILE_PATH_LENGTH));
   };
   ctPackageReadMetadata(const char* path) : ctPackageReadMetadata() {
      strncpy(fullPath, path, CT_MAX_FILE_PATH_LENGTH - 1);
   }
   char fullPath[CT_MAX_FILE_PATH_LENGTH];
};

class ctPackageReadStreamInternalBase {
public:
   virtual uint64_t GetDecompressedSize() = 0;
   virtual uint64_t Seek(int64_t position, enum ctPackageReadSeekMode mode) = 0;
   virtual size_t ctPackageReadStreamGetBytes(void* dest, size_t byteCount) = 0;
   ~ctPackageReadStreamInternalBase() {};
};

class ctPackageReadStreamInternalRaw {
public:
   ctPackageReadStreamInternalRaw(ctPackageReadMetadata& packMeta,
                                  ctPackageSection& section) {
      file = ctFile(packMeta.fullPath, CT_FILE_OPEN_READ);
      file.Seek(section.blobOffset, CT_FILE_SEEK_SET);
      blobOffset = section.blobOffset;
      currentOffset = 0;
      size = section.blobSize;
   }
   virtual uint64_t GetDecompressedSize() {
      return size;
   };
   virtual uint64_t Seek(int64_t position, enum ctPackageReadSeekMode mode) {
      switch (mode) {
         case CT_PACKAGE_SEEK_SET: currentOffset = blobOffset + position; break;
         case CT_PACKAGE_SEEK_CUR: currentOffset = currentOffset + position;
         case CT_PACKAGE_SEEK_END: currentOffset = (currentOffset + size) - position;
         default: break;
      }
      return file.Seek(currentOffset, CT_FILE_SEEK_SET);
   }
   virtual size_t ctPackageReadStreamGetBytes(void* dest, size_t byteCount) {
      return file.ReadRaw(dest, byteCount, 1);
   }

private:
   int64_t blobOffset;
   int64_t currentOffset;
   size_t size;
   ctFile file;
};

class ctPackageReadManagerInternal {
public:
   ctPackageReadManagerInternal(size_t pathCount, const char** paths) {
      for (size_t i = 0; i < pathCount; i++) {
         LoadPackage(paths[i]);
      }
   };
   ctResults LoadPackage(const char* path);
   ctPackageReadStreamInternalBase* NewReadStreamForSectionIdx(uint32_t idx);
   ctPackageReadStreamInternalBase* NewReadStreamForSectionPath(const char* path);
   ctPackageReadStreamInternalBase* NewReadStreamForSectionGUID(const ctGUID* pGuid);

private:
   ctDynamicArray<ctPackageReadMetadata> packages;
   ctDynamicArray<uint32_t> sectionPackageIndex;
   ctDynamicArray<ctPackageSection> sections;

   ctHashTable<uint32_t, uint64_t> sectionIndexByPathHash;
   ctHashTable<uint32_t, uint64_t> sectionIndexByGUIDHash;
};

ctResults ctPackageReadManagerInternal::LoadPackage(const char* path) {
   /* Open File */
   ctFile scrubFile = ctFile(path, CT_FILE_OPEN_READ);
   if (!scrubFile.isOpen()) { return CT_FAILURE_INACCESSIBLE; }
   /* Read Header */
   ctPackageHeader header = {0};
   scrubFile.ReadRaw(&header, sizeof(header), 1);
   /* Check header info */
   if (header.magic != CT_PACKAGE_MAGIC) { return CT_FAILURE_CORRUPTED_CONTENTS; }
   if (header.version != CT_PACKAGE_VERSION) { return CT_FAILURE_UNKNOWN_FORMAT; }
   packages.Append(ctPackageReadMetadata(path));
   /* Seek */
   scrubFile.Seek(header.sectionOffset, CT_FILE_SEEK_SET);
   /* Load sections */
   const size_t sectionBegin = sections.Count();
   sections.Resize(sectionBegin + header.sectionCount);
   scrubFile.ReadRaw(
     sections.Data() + sectionBegin, sizeof(ctPackageSection), header.sectionCount);
   /* Build section indices */
   sectionPackageIndex.Append((uint32_t)packages.Count() - 1, header.sectionCount);
   for (uint32_t i = 0; i < (uint32_t)sections.Count(); i++) {
      const uint64_t hash = sections[i].pathHash;
      sectionIndexByPathHash.InsertOrReplace(hash, i);
   }
   for (uint32_t i = 0; i < (uint32_t)sections.Count(); i++) {
      const uint64_t hash = ctXXHash64(sections[i].guidData, 16);
      sectionIndexByGUIDHash.InsertOrReplace(hash, i);
   }
   return CT_SUCCESS;
}

ctPackageReadStreamInternalBase*
ctPackageReadManagerInternal::NewReadStreamForSectionIdx(uint32_t idx) {
   ctPackageReadMetadata meta = packages[sectionPackageIndex[idx]];
   ctPackageSection section = sections[idx];
   if (section.compressionMode == CT_PACKAGE_COMPRESSION_NONE) {
      return (ctPackageReadStreamInternalBase*)new ctPackageReadStreamInternalRaw(
        meta, section);
   }
   return NULL;
}

ctPackageReadStreamInternalBase*
ctPackageReadManagerInternal::NewReadStreamForSectionPath(const char* path) {
   uint32_t* pIdx = sectionIndexByPathHash.FindPtr(ctXXHash64(path));
   if (!pIdx) { return NULL; }
   return NewReadStreamForSectionIdx(*pIdx);
}

ctPackageReadStreamInternalBase*
ctPackageReadManagerInternal::NewReadStreamForSectionGUID(const ctGUID* pGuid) {
   uint32_t* pIdx = sectionIndexByGUIDHash.FindPtr(ctXXHash64(pGuid->data, 16));
   if (!pIdx) { return NULL; }
   return NewReadStreamForSectionIdx(*pIdx);
}

/* ------------- Write API ------------- */

CT_API ctPackageWriteContext ctPackageWriteContextCreate(const char* filePath) {
   return new ctPackageWriterContextInternal(filePath);
}

CT_API ctResults ctPackageWriteFinish(ctPackageWriteContext ctx) {
   return ((ctPackageWriterContextInternal*)ctx)->Finish();
}

CT_API void ctPackageWriteDestroy(ctPackageWriteContext ctx) {
   delete (ctPackageWriterContextInternal*)ctx;
}

CT_API ctResults ctPackageWriteSection(ctPackageWriteContext ctx,
                                       const char* path,
                                       const void* guidPtr,
                                       size_t size,
                                       const void* data,
                                       enum ctPackageCompression compressionMode) {
   return ((ctPackageWriterContextInternal*)ctx)
     ->WriteSection(path, (const ctGUID*)guidPtr, size, data, compressionMode);
}

/* ------------- Read API ------------- */

CT_API ctPackageReadManager ctPackageReadManagerCreate(size_t packageCount,
                                                       const char** filePaths) {
   return new ctPackageReadManagerInternal(packageCount, filePaths);
}

CT_API void ctPackageReadManagerDestroy(ctPackageReadManager ctx) {
   return delete (ctPackageReadManagerInternal*)ctx;
}

CT_API ctPackageReadStream ctPackageReadOpenStreamByPath(const ctPackageReadManager ctx,
                                                         const char* path) {
   return CT_API ctPackageReadStream();
}

CT_API ctPackageReadStream ctPackageReadOpenStreamByGUID(const ctPackageReadManager ctx,
                                                         const void* guidPtr) {
   return CT_API ctPackageReadStream();
}

CT_API void ctPackageReadClose(ctPackageReadStream stream) {
   delete (ctPackageReadStreamInternalBase*)stream;
}

CT_API uint64_t ctPackageReadStreamGetDecompressedSize(const ctPackageReadStream stream) {
   return ((ctPackageReadStreamInternalBase*)stream)->GetDecompressedSize();
}

CT_API uint64_t ctPackageReadStreamSeek(ctPackageReadStream stream,
                                        int64_t position,
                                        enum ctPackageReadSeekMode mode) {
   return ((ctPackageReadStreamInternalBase*)stream)->Seek(position, mode);
}

CT_API size_t ctPackageReadStreamGetBytes(ctPackageReadStream stream,
                                          void* dest,
                                          size_t byteCount) {
   return ((ctPackageReadStreamInternalBase*)stream)
     ->ctPackageReadStreamGetBytes(dest, byteCount);
}
