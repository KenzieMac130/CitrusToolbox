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

#include "Image.hpp"

enum ctImageSource {
   CT_IMAGE_SOURCE_NONE,
   CT_IMAGE_SOURCE_CUSTOM,
   CT_IMAGE_SOURCE_TINYKTX,
   CT_IMAGE_SOURCE_TINYDDS,
   CT_IMAGE_SOURCE_STB
};

#define STBI_ASSERT(x)         ctAssert(x)
#define STBI_MALLOC(sz)        ctMalloc(sz)
#define STBI_REALLOC(p, newsz) ctRealloc(p, newsz)
#define STBI_FREE(p)           ctFree(p)
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include "tiny_imageFormat/tinyimageformat.h"

#define TINYKTX_IMPLEMENTATION
#define TINYDDS_IMPLEMENTATION
#include "tiny_ktx/tinyktx.h"
#include "tiny_dds/tinydds.h"

struct tinyUserData {
   ctFile* pFile;
   bool* pErrorOccured;
   const char* errorPrefix;
};
void* ctTinyAlloc(void* user, size_t size) {
   return ctMalloc(size);
}
void ctTinyFree(void* user, void* memory) {
   ctFree(memory);
}
size_t ctTinyRead(void* user, void* buffer, size_t byteCount) {
   tinyUserData* pCtx = (tinyUserData*)user;
   return pCtx->pFile->ReadRaw(buffer, byteCount, 1);
}
bool ctTinySeek(void* user, int64_t offset) {
   tinyUserData* pCtx = (tinyUserData*)user;
   return pCtx->pFile->Seek(offset, CT_FILE_SEEK_SET) == CT_SUCCESS;
}
int64_t ctTinyTell(void* user) {
   tinyUserData* pCtx = (tinyUserData*)user;
   return pCtx->pFile->Tell();
}
void ctTinyError(void* user, char const* msg) {
   tinyUserData* pCtx = (tinyUserData*)user;
   *pCtx->pErrorOccured = true;
   ctDebugError("%s%s", pCtx->errorPrefix, msg);
}

TinyKtx_Callbacks tinyKtxCbs = {
  ctTinyError, ctTinyAlloc, ctTinyFree, ctTinyRead, ctTinySeek, ctTinyTell};

TinyDDS_Callbacks tinyDdsCbs = {
  ctTinyError, ctTinyAlloc, ctTinyFree, ctTinyRead, ctTinySeek, ctTinyTell};

struct stbUserData {
   ctFile* pFile;
};

int ctStbRead(void* user, char* data, int size) {
   stbUserData* pCtx = (stbUserData*)user;
   return (int)pCtx->pFile->ReadRaw(data, size, 1);
}
void ctStbSkip(void* user, int n) {
   stbUserData* pCtx = (stbUserData*)user;
   pCtx->pFile->Seek(n, CT_FILE_SEEK_CUR);
}
int ctStbEof(void* user) {
   stbUserData* pCtx = (stbUserData*)user;
   return pCtx->pFile->isEndOfFile();
}

stbi_io_callbacks stbCallbacks = {ctStbRead, ctStbSkip, ctStbEof};

ctResults ctImage::LoadKTX(ctFile& file) {
   source = CT_IMAGE_SOURCE_TINYKTX;
   bool errorOccured = false;
   tinyUserData ud = {&file, &errorOccured, "[TinyKTX] "};
   TinyKtx_ContextHandle tinyKtx = TinyKtx_CreateContext(&tinyKtxCbs, &ud);
   loaderdata = tinyKtx;
   if (!TinyKtx_ReadHeader(tinyKtx)) {
      TinyKtx_DestroyContext(tinyKtx);
      return CT_FAILURE_CORRUPTED_CONTENTS;
   }

   /* only support native endian */
   if (TinyKtx_NeedsEndianCorrecting(tinyKtx)) {
      Invalidate();
      return CT_FAILURE_CORRUPTED_CONTENTS;
   }

   /* get format */
   format = TinyImageFormat_FromTinyKtxFormat(TinyKtx_GetFormat(tinyKtx));

   /* get dimensions */
   width = TinyKtx_Width(tinyKtx);
   height = TinyKtx_Height(tinyKtx);

   /* mipmaps */
   if (TinyKtx_NeedsGenerationOfMipmaps(tinyKtx)) {
      mips = 1; /* nope: not doing it here */
   } else {
      mips = TinyKtx_NumberOfMipmaps(tinyKtx);
   }

   /* get type */
   if (TinyKtx_Is1D(tinyKtx)) {
      type = CT_IMAGE_TYPE_1D;
      depth = TinyKtx_ArraySlices(tinyKtx);
   } else if (TinyKtx_Is2D(tinyKtx)) {
      type = CT_IMAGE_TYPE_2D;
      depth = TinyKtx_ArraySlices(tinyKtx);
   } else if (TinyKtx_Is3D(tinyKtx)) {
      type = CT_IMAGE_TYPE_3D;
      depth = TinyKtx_Depth(tinyKtx);
   } else if (TinyKtx_IsCubemap(tinyKtx)) {
      type = CT_IMAGE_TYPE_CUBE;
      depth = TinyKtx_ArraySlices(tinyKtx);
   }

   /* load data */
   for (uint32_t mip = 0; mip < mips; mip++) {
      uint32_t size = TinyKtx_ImageSize(tinyKtx, mip);
      if (TinyKtx_IsMipMapLevelUnpacked(tinyKtx, mip)) {
         /* todo read with TinyKtx_UnpackedRowStride stride */
         ctAssert(0);
         Invalidate();
         return CT_FAILURE_CORRUPTED_CONTENTS;
      } else {
         levels[mip] = (void*)TinyKtx_ImageRawData(tinyKtx, mip);
      }
   }
   if (errorOccured) {
      Invalidate();
      return CT_FAILURE_CORRUPTED_CONTENTS;
   }
   return CT_SUCCESS;
}

/* mostly mirrors KTX code */
ctResults ctImage::LoadDDS(ctFile& file) {
   source = CT_IMAGE_SOURCE_TINYDDS;
   bool errorOccured = false;
   tinyUserData ud = {&file, &errorOccured, "[TinyDDS] "};
   TinyDDS_ContextHandle tinyDDS = TinyDDS_CreateContext(&tinyDdsCbs, &ud);
   loaderdata = tinyDDS;
   if (!TinyDDS_ReadHeader(tinyDDS)) {
      TinyDDS_DestroyContext(tinyDDS);
      return CT_FAILURE_CORRUPTED_CONTENTS;
   }

   /* only support native endian */
   if (TinyDDS_NeedsEndianCorrecting(tinyDDS)) {
      Invalidate();
      return CT_FAILURE_CORRUPTED_CONTENTS;
   }

   /* get format */
   format = TinyImageFormat_FromTinyDDSFormat(TinyDDS_GetFormat(tinyDDS));

   /* get dimensions */
   width = TinyDDS_Width(tinyDDS);
   height = TinyDDS_Height(tinyDDS);

   /* mipmaps */
   if (TinyDDS_NeedsGenerationOfMipmaps(tinyDDS)) {
      mips = 1; /* nope: not doing it here */
   } else {
      mips = TinyDDS_NumberOfMipmaps(tinyDDS);
   }

   /* get type */
   if (TinyDDS_Is1D(tinyDDS)) {
      type = CT_IMAGE_TYPE_1D;
      depth = TinyDDS_ArraySlices(tinyDDS);
   } else if (TinyDDS_Is2D(tinyDDS)) {
      type = CT_IMAGE_TYPE_2D;
      depth = TinyDDS_ArraySlices(tinyDDS);
   } else if (TinyDDS_Is3D(tinyDDS)) {
      type = CT_IMAGE_TYPE_3D;
      depth = TinyDDS_Depth(tinyDDS);
   } else if (TinyDDS_IsCubemap(tinyDDS)) {
      type = CT_IMAGE_TYPE_CUBE;
      depth = TinyDDS_ArraySlices(tinyDDS);
   }

   /* load data */
   for (uint32_t mip = 0; mip < mips; mip++) {
      levelSizes[mip] = TinyDDS_ImageSize(tinyDDS, mip);
      levels[mip] = (void*)TinyDDS_ImageRawData(tinyDDS, mip);
   }

   if (errorOccured) {
      Invalidate();
      return CT_FAILURE_CORRUPTED_CONTENTS;
   }
   return CT_SUCCESS;
}

ctResults ctImage::LoadMisc(ctFile& file) {
   source = CT_IMAGE_SOURCE_STB;
   stbUserData ud = {&file};
   int channels = 0;
   int iwidth, iheight;

   loaderdata =
     stbi_load_from_callbacks(&stbCallbacks, &ud, &iwidth, &iheight, &channels, 4);
   if (!loaderdata) {
      Invalidate();
      return CT_FAILURE_UNKNOWN_FORMAT;
   }
   width = (uint32_t)iwidth;
   height = (uint32_t)iheight;
   depth = 1;
   mips = 1;
   format = TinyImageFormat_R8G8B8A8_UNORM;
   type = CT_IMAGE_TYPE_2D;
   levels[0] = loaderdata;
   return CT_SUCCESS;
}

ctResults ctImage::Load(ctFile& file) {
   Release();
   char peek[6];
   memset(peek, 0, 6);
   int64_t read = (int64_t)file.ReadRaw(peek, 1, 5);
   file.Seek(-read, CT_FILE_SEEK_CUR);
   uint8_t ktxId[5] = {0xAB, 0x4B, 0x54, 0x58, 0x00};
   if (ctCStrNEql(peek, (const char*)ktxId, 4)) {
      return LoadKTX(file);
   } else if (ctCStrNEql(peek, "DDS", 3)) {
      return LoadDDS(file);
   } else {
      return LoadMisc(file);
   }
}

void ctImage::Invalidate() {
   width = 0;
   height = 0;
   depth = 0;
   mips = 0;
   format = TinyImageFormat_UNDEFINED;
   type = CT_IMAGE_TYPE_2D;
   source = CT_IMAGE_SOURCE_NONE;
}

void ctImage::MakeCustom() {
if (source == CT_IMAGE_SOURCE_CUSTOM) {
      return; /* already editable */
   } else {
      for(uint32_t i = 0; i < GetMipCount(); i++){
         size_t size = GetByteCount(i);
         void* original = GetData(i);
         void* newData = ctMalloc(size);
         memcpy(newData, original, size);
         levels[i] = newData;
      }
      ReleaseLoader();
      source = CT_IMAGE_SOURCE_CUSTOM;
   }
}

void ctImage::Release() {
   ReleaseLoader();
   Invalidate();
}

void ctImage::ReleaseLoader() {
   if (source == CT_IMAGE_SOURCE_CUSTOM) {
      for(size_t i = 0; i < GetMipCount(); i++){
         ctFree(levels[i]);
      }
   } else if (source == CT_IMAGE_SOURCE_TINYKTX) {
      TinyKtx_DestroyContext((TinyKtx_ContextHandle)loaderdata);
   } else if (source == CT_IMAGE_SOURCE_TINYDDS) {
      TinyDDS_DestroyContext((TinyDDS_ContextHandle)loaderdata);
   } else if (source == CT_IMAGE_SOURCE_STB) {
      stbi_image_free(loaderdata);
   }
   loaderdata = NULL;
   source = CT_IMAGE_SOURCE_NONE;
}

ctImage::ctImage() {
   Release();
}

ctImage::ctImage(ctFile& file) {
   Load(file);
}

ctImage::~ctImage() {
   Release();
}

ctResults ctImage::Create(uint32_t iwidth,
                          uint32_t iheight,
                          uint32_t idepth,
                          uint32_t imips,
                          TinyImageFormat iformat,
                          ctImageType itype,
                          size_t* pLevelSizes) {
   width = iwidth;
   height = iheight;
   depth = idepth;
   mips = imips;
   format = iformat;
   type = itype;
   source = CT_IMAGE_SOURCE_CUSTOM;
   for(size_t i = 0; i < imips; i++){
      levels[i] = ctMalloc(pLevelSizes[i]);
   }
   return CT_SUCCESS;
}

bool ctImage::isValid() const {
   return true;
}