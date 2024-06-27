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

enum ctArchiveEntryType {
   CT_ARCHIVE_NULL,       /* contains no data*/
   CT_ARCHIVE_TABLE,      /* data is an array of ctArchiveTables */
   CT_ARCHIVE_DICTIONARY, /* data an offset to two ctArchiveEntry as a key/value */
   CT_ARCHIVE_BYTES,      /* data is offset to raw bytes */
   CT_ARCHIVE_GPU_BYTES,  /* data is offset to raw bytes in the GPU arena  */
   CT_ARCHIVE_STRING,     /* data is a uint64_t offset into the string pool */
   CT_ARCHIVE_BOOL,
   CT_ARCHIVE_UINT8,
   CT_ARCHIVE_UINT16,
   CT_ARCHIVE_UINT32,
   CT_ARCHIVE_UINT64,
   CT_ARCHIVE_INT8,
   CT_ARCHIVE_INT16,
   CT_ARCHIVE_INT32,
   CT_ARCHIVE_INT64,
   CT_ARCHIVE_FLOAT,
   CT_ARCHIVE_DOUBLE,
   CT_ARCHIVE_VEC2,
   CT_ARCHIVE_VEC3,
   CT_ARCHIVE_VEC4,
   CT_ARCHIVE_QUAT,
   CT_ARCHIVE_MAT4,
   CT_ARCHIVE_GUID
};

struct ctArchiveTable {
   uint32_t entryCount;
   uint64_t entryHashStart;
   uint64_t entryOffsetStart;
};

struct ctArchiveEntry {
   uint32_t type;       /* ctArchiveEntryType */
   uint32_t byteCount;  /* byte count, multiples of expected size are a flat array */
   uint64_t byteOffset; /* offset to data */
};

struct ctArchiveHeader {
   uint32_t magic;
   uint32_t version;
   ctArchiveTable root;
   uint64_t stringPoolStart;
   uint64_t cpuDataStart;
   uint64_t cpuDataByteCount;
   uint64_t gpuDataStart;
   uint64_t gpuDataByteCount;
};

class CT_API ctArchiveWriter {
public:
   void PushTable(const char* name);
   void PopTable();
   void BeginTableArray();
   void EndTableArray();
   void Write(const char* name, size_t count, ctArchiveEntryType valueType, void* value);
   inline void Write(const char* name, ctArchiveEntryType valueType, void* value) {
      Write(name, 1, valueType, value);
   }
   void WriteDict(const char* name,
                  size_t count,
                  ctArchiveEntryType keyType,
                  void* keys,
                  ctArchiveEntryType valueType,
                  void* values) {
      PushTable(name);
      Write("key", count, keyType, keys);
      Write("values", count, valueType, values);
      PopTable();
   }

private:
   ctDynamicArray<uint32_t> entryHashList;
   ctDynamicArray<ctArchiveEntry> entryList;
   ctDynamicArray<ctArchiveTable> tables;

   ctDynamicArray<uint8_t> cpuData;
   ctDynamicArray<uint8_t> gpuData;
};

class ctArchiveReaderTable;
class CT_API ctArchiveReaderEntry {
   /* check first before loading */
   bool isType(ctArchiveEntryType type) const;
   ctResults GetTable(ctArchiveReaderTable& out, size_t arrayIndex = 0) const;
   ctResults GetArrayCount() const;
   ctResults GetData(ctArchiveEntryType type, void* dest);
};

class CT_API ctArchiveReaderTable {
   ctResults GetEntry(const char* name, ctArchiveReaderEntry& entry) const;
};

class CT_API ctArchiveReader {
public:
   ctArchiveReaderTable GetRoot();
};