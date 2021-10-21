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
#endif

enum ctPackageCompression {
   CT_PACKAGE_COMPRESSION_NONE = 0,
   CT_PACKAGE_COMPRESSION_LZ4_BLOCK = 1,
};

#define CT_PACKAGE_MAGIC   0x4b505443
#define CT_PACKAGE_VERSION 1

struct ctPackageHeader {
   uint32_t magic;
   uint32_t version;
   uint64_t sectionCount;
   int64_t sectionOffset;
};

struct ctPackageSection {
   uint64_t pathHash;
   char guidData[16];
   int64_t blobOffset;
   uint64_t blobSize;
   uint32_t compressionMode;
   int32_t compressionMetadata[5];
};

/* ------------- Write API ------------- */
typedef void* ctPackageWriteContext;
CT_API ctPackageWriteContext ctPackageWriteContextCreate(const char* filePath);
CT_API enum ctResults ctPackageWriteSection(ctPackageWriteContext ctx,
                                            const char* path,
                                            const void* guidPtr,
                                            size_t size,
                                            const void* data,
                                            enum ctPackageCompression compressionMode);
CT_API enum ctResults ctPackageWriteFinish(ctPackageWriteContext ctx);
CT_API void ctPackageWriteDestroy(ctPackageWriteContext ctx);

/* ------------- Read API ------------- */
typedef void* ctPackageReadManager;
CT_API ctPackageReadManager ctPackageReadManagerCreate(size_t packageCount,
                                                       const char** filePaths);
CT_API void ctPackageReadManagerDestroy(ctPackageReadManager ctx);

enum ctPackageReadSeekMode {
   CT_PACKAGE_SEEK_SET = SEEK_SET,
   CT_PACKAGE_SEEK_CUR = SEEK_CUR,
   CT_PACKAGE_SEEK_END = SEEK_END
};

typedef void* ctPackageReadStream;
CT_API ctPackageReadStream ctPackageReadOpenStreamByPath(const ctPackageReadManager ctx,
                                                         const char* path);
CT_API ctPackageReadStream ctPackageReadOpenStreamByGUID(const ctPackageReadManager ctx,
                                                         const void* guidPtr);
CT_API void ctPackageReadClose(ctPackageReadStream stream);

CT_API uint64_t ctPackageReadStreamGetDecompressedSize(const ctPackageReadStream stream);
CT_API uint64_t ctPackageReadStreamSeek(ctPackageReadStream stream,
                                        int64_t position,
                                        enum ctPackageReadSeekMode mode);
CT_API size_t ctPackageReadStreamGetBytes(ctPackageReadStream stream,
                                          void* dest,
                                          size_t byteCount);

#ifdef __cplusplus
}
#endif