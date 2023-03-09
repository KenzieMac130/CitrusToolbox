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

#include "TextureLoad.h"

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

ctResults LoadKTX(ctFile& file, ctTextureLoadCtx* ctx) {
   ctx->src = CT_TEXTURELOAD_TINYKTX;
   bool errorOccured = false;
   tinyUserData ud = {&file, &errorOccured, "[TinyKTX] "};
   TinyKtx_ContextHandle tinyKtx = TinyKtx_CreateContext(&tinyKtxCbs, &ud);
   ctx->loaderdata = tinyKtx;
   if (!TinyKtx_ReadHeader(tinyKtx)) {
      TinyKtx_DestroyContext(tinyKtx);
      return CT_FAILURE_CORRUPTED_CONTENTS;
   }

   /* only support native endian */
   if (TinyKtx_NeedsEndianCorrecting(tinyKtx)) { return CT_FAILURE_CORRUPTED_CONTENTS; }

   /* get format */
   ctx->format = TinyImageFormat_FromTinyKtxFormat(TinyKtx_GetFormat(tinyKtx));

   /* get dimensions */
   ctx->width = TinyKtx_Width(tinyKtx);
   ctx->height = TinyKtx_Height(tinyKtx);

   /* mipmaps */
   if (TinyKtx_NeedsGenerationOfMipmaps(tinyKtx)) {
      ctx->mips = 1; /* nope: not doing it here */
   } else {
      ctx->mips = TinyKtx_NumberOfMipmaps(tinyKtx);
   }

   /* get type */
   if (TinyKtx_Is1D(tinyKtx)) {
      ctx->type = CT_TEXTURELOAD_1D;
      ctx->depth = TinyKtx_ArraySlices(tinyKtx);
   } else if (TinyKtx_Is2D(tinyKtx)) {
      ctx->type = CT_TEXTURELOAD_2D;
      ctx->depth = TinyKtx_ArraySlices(tinyKtx);
   } else if (TinyKtx_Is3D(tinyKtx)) {
      ctx->type = CT_TEXTURELOAD_3D;
      ctx->depth = TinyKtx_Depth(tinyKtx);
   } else if (TinyKtx_IsCubemap(tinyKtx)) {
      ctx->type = CT_TEXTURELOAD_CUBEMAP;
      ctx->depth = TinyKtx_ArraySlices(tinyKtx);
   }

   /* load data */
   for (uint32_t mip = 0; mip < ctx->mips; mip++) {
      uint32_t size = TinyKtx_ImageSize(tinyKtx, mip);
      if (TinyKtx_IsMipMapLevelUnpacked(tinyKtx, mip)) {
         /* todo read with TinyKtx_UnpackedRowStride stride */
         ctAssert(0);
         return CT_FAILURE_CORRUPTED_CONTENTS;
      } else {
         ctx->levels[mip] = TinyKtx_ImageRawData(tinyKtx, mip);
      }
   }
   if (errorOccured) { return CT_FAILURE_CORRUPTED_CONTENTS; }
   return CT_SUCCESS;
}

/* mostly mirrors KTX code */
ctResults LoadDDS(ctFile& file, ctTextureLoadCtx* ctx) {
   ctx->src = CT_TEXTURELOAD_TINYDDS;
   bool errorOccured = false;
   tinyUserData ud = {&file, &errorOccured, "[TinyDDS] "};
   TinyDDS_ContextHandle tinyDDS = TinyDDS_CreateContext(&tinyDdsCbs, &ud);
   ctx->loaderdata = tinyDDS;
   if (!TinyDDS_ReadHeader(tinyDDS)) {
      TinyDDS_DestroyContext(tinyDDS);
      return CT_FAILURE_CORRUPTED_CONTENTS;
   }

   /* only support native endian */
   if (TinyDDS_NeedsEndianCorrecting(tinyDDS)) { return CT_FAILURE_CORRUPTED_CONTENTS; }

   /* get format */
   ctx->format = TinyImageFormat_FromTinyDDSFormat(TinyDDS_GetFormat(tinyDDS));

   /* get dimensions */
   ctx->width = TinyDDS_Width(tinyDDS);
   ctx->height = TinyDDS_Height(tinyDDS);

   /* mipmaps */
   if (TinyDDS_NeedsGenerationOfMipmaps(tinyDDS)) {
      ctx->mips = 1; /* nope: not doing it here */
   } else {
      ctx->mips = TinyDDS_NumberOfMipmaps(tinyDDS);
   }

   /* get type */
   if (TinyDDS_Is1D(tinyDDS)) {
      ctx->type = CT_TEXTURELOAD_1D;
      ctx->depth = TinyDDS_ArraySlices(tinyDDS);
   } else if (TinyDDS_Is2D(tinyDDS)) {
      ctx->type = CT_TEXTURELOAD_2D;
      ctx->depth = TinyDDS_ArraySlices(tinyDDS);
   } else if (TinyDDS_Is3D(tinyDDS)) {
      ctx->type = CT_TEXTURELOAD_3D;
      ctx->depth = TinyDDS_Depth(tinyDDS);
   } else if (TinyDDS_IsCubemap(tinyDDS)) {
      ctx->type = CT_TEXTURELOAD_CUBEMAP;
      ctx->depth = TinyDDS_ArraySlices(tinyDDS);
   }

   /* load data */
   for (uint32_t mip = 0; mip < ctx->mips; mip++) {
      uint32_t size = TinyDDS_ImageSize(tinyDDS, mip);
      ctx->levels[mip] = TinyDDS_ImageRawData(tinyDDS, mip);
   }

   if (errorOccured) { return CT_FAILURE_CORRUPTED_CONTENTS; }
   return CT_SUCCESS;
}

ctResults LoadMisc(ctFile& file, ctTextureLoadCtx* ctx) {
   ctx->src = CT_TEXTURELOAD_STB;
   stbUserData ud = {&file};
   int channels = 0;
   int iwidth, iheight;

   ctx->loaderdata =
     stbi_load_from_callbacks(&stbCallbacks, &ud, &iwidth, &iheight, &channels, 4);
   if (!ctx->loaderdata) { return CT_FAILURE_CORRUPTED_CONTENTS; }
   ctx->width = (uint32_t)iwidth;
   ctx->height = (uint32_t)iheight;
   ctx->depth = 1;
   ctx->mips = 1;
   ctx->format = TinyImageFormat_R8G8B8A8_UNORM;
   ctx->type = CT_TEXTURELOAD_2D;
   ctx->levels[0] = ctx->loaderdata;
   return CT_SUCCESS;
}

ctResults ctTextureLoadFromFile(ctFile& file, ctTextureLoadCtx* ctx) {
   *ctx = ctTextureLoadCtx();
   char peek[6];
   memset(peek, 0, 6);
   int64_t read = (int64_t)file.ReadRaw(peek, 1, 5);
   file.Seek(-read, CT_FILE_SEEK_CUR);
   uint8_t ktxId[5] = {0xAB, 0x4B, 0x54, 0x58, 0x00};
   if (ctCStrNEql(peek, (const char*)ktxId, 4)) {
      return LoadKTX(file, ctx);
   } else if (ctCStrNEql(peek, "DDS", 3)) {
      return LoadDDS(file, ctx);
   } else {
      return LoadMisc(file, ctx);
   }
}

void ctTextureLoadCtxRelease(ctTextureLoadCtx* pCtx) {
   if (!pCtx) { return; }
   if (pCtx->src == CT_TEXTURELOAD_TINYKTX) {
      TinyKtx_DestroyContext((TinyKtx_ContextHandle)pCtx->loaderdata);
   } else if (pCtx->src == CT_TEXTURELOAD_TINYDDS) {
      TinyDDS_DestroyContext((TinyDDS_ContextHandle)pCtx->loaderdata);
   } else if (pCtx->src == CT_TEXTURELOAD_STB) {
      stbi_image_free(pCtx->loaderdata);
   }
}