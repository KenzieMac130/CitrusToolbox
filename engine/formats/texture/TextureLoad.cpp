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
   ctDebugError(msg);
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

ctResults ctLoadTextureFromFile(const char* path, ctTextureLoadCtx* ctx) {
   ctFile file = ctFile(path, CT_FILE_OPEN_READ);
   if (!file.isOpen()) { return CT_FAILURE_INACCESSIBLE; }

   if (1/* check ktx magic number*/) {
      /* Load Tiny KTX */
      tinyUserData ud = {&file};
      TinyKtx_ContextHandle tinyKtx = TinyKtx_CreateContext(&tinyKtxCbs, &ud);
      if (!TinyKtx_ReadHeader(tinyKtx)) {
         TinyKtx_DestroyContext(tinyKtx);
         return CT_FAILURE_CORRUPTED_CONTENTS;
      }
   } else if (1/* check dds magic*/) {
      /* Load Tiny DDS */
   } else if(1/* Wants 3D texture & Check for CUBE file keywords (#**, TIT, LUT, DOM*/) {
	   /* https://wwwimages2.adobe.com/content/dam/acom/en/products/speedgrade/cc/pdfs/cube-lut-specification-1.0.pdf */
   } else if(1/* Wants 2D texture */) {
      /* Attempt to Load STB */
      int channels = 0;
      stbUserData ud = {&file};
      ctx->memory.data = stbi_load_from_callbacks(
        &stbCallbacks, &ud, &ctx->memory.width, &ctx->memory.height, &channels, 4);
      ctx->memory.format = TinyImageFormat_R8G8B8A8_UNORM;
      if (!ctx->memory.data) { return CT_FAILURE_CORRUPTED_CONTENTS; }
   } else {
	   return CT_FAILURE_CORRUPTED_CONTENTS;
   }
   return CT_SUCCESS;
}

void ctTextureLoadCtxRelease(ctTextureLoadCtx* pCtx) {
   if (!pCtx) { return; }
   ctFree(pCtx->memory.data);
}