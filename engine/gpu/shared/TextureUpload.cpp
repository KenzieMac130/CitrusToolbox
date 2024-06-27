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

#include "../Texture.h"
#include "formats/texture/TextureLoad.h"

void ctGPUTextureUploadFnQuickMemcpy(uint8_t* dest,
                                     ctGPUExternalGenerateContext* pCtx,
                                     void* userData) {
   ctAssert(pCtx->currentLayer == 0 && pCtx->currentMipLevel == 0);
   ctAssert(TinyImageFormat_IsHomogenous(pCtx->format));
   const size_t bytesPerPixel = (size_t)TinyImageFormat_BitSizeOfBlock(pCtx->format) / 8;
   const size_t byteCount = bytesPerPixel * pCtx->width * pCtx->height * pCtx->depth;
   memcpy(dest, userData, byteCount);
}

void ctGPUTextureUploadFnTextureLoader(uint8_t* dest,
                                       ctGPUExternalGenerateContext* pCtx,
                                       void* userData) {
   const ctTextureLoadCtx* pTextureLoader = (ctTextureLoadCtx*)userData;
   const size_t mipLevel = pCtx->currentMipLevel;
   const size_t arrayLayer = pCtx->currentLayer;
   const size_t arrayLayerCount =
     pTextureLoader->type == CT_TEXTURELOAD_3D ? 1 : pTextureLoader->depth;
   ctAssert(pTextureLoader->levelSizes[mipLevel] % arrayLayerCount == 0);
   const size_t sliceSize = pTextureLoader->levelSizes[mipLevel] / arrayLayerCount;
   const size_t sliceOffset = arrayLayer * sliceSize;
   memcpy(dest, (uint8_t*)pTextureLoader->levels[mipLevel] + sliceOffset, sliceSize);
}