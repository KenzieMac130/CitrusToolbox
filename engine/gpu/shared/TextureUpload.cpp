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
#include "formats/image/Image.hpp"

void ctGPUTextureUploadFnQuickMemcpy(uint8_t* dest,
                                     ctGPUExternalGenerateContext* pCtx,
                                     void* userData) {
   ctAssert(userData);
   ctAssert(pCtx->currentLayer == 0 && pCtx->currentMipLevel == 0);
   ctAssert(TinyImageFormat_IsHomogenous(pCtx->format));
   const size_t bytesPerPixel = (size_t)TinyImageFormat_BitSizeOfBlock(pCtx->format) / 8;
   const size_t byteCount = bytesPerPixel * pCtx->width * pCtx->height * pCtx->depth;
   memcpy(dest, userData, byteCount);
}

void ctGPUTextureUploadFnImage(uint8_t* dest,
                               ctGPUExternalGenerateContext* pCtx,
                               void* userData) {
   ctAssert(userData);
   const ctImage& image = *(const ctImage*)userData;
   const uint32_t mipLevel = pCtx->currentMipLevel;
   const size_t arrayLayer = pCtx->currentLayer;
   const size_t arrayLayerCount =
     image.GetType() == CT_IMAGE_TYPE_3D ? 1 : image.GetDepth();
   ctAssert(image.GetByteCount(mipLevel) % arrayLayerCount == 0);
   const size_t sliceSize = image.GetByteCount(mipLevel) / arrayLayerCount;
   const size_t sliceOffset = arrayLayer * sliceSize;
   memcpy(dest, (uint8_t*)image.GetData(mipLevel) + sliceOffset, sliceSize);
}

CT_API enum ctResults
ctGPUExternalTextureFromImage(struct ctGPUDevice* pDevice,
                              struct ctGPUExternalTexturePool* pPool,
                              struct ctGPUExternalTexture** ppTexture,
                              const char* debugName,
                              const class ctImage* pImage) {
   ctGPUExternalTextureCreateInfo createInfo = {};
   createInfo.debugName = debugName;
   createInfo.type = (ctGPUExternalTextureType)pImage->GetType();
   createInfo.updateMode = CT_GPU_UPDATE_STATIC;
   createInfo.format = pImage->GetFormat();
   createInfo.height = pImage->GetHeight();
   createInfo.width = pImage->GetWidth();
   createInfo.depth = pImage->GetDepth();
   createInfo.mips = pImage->GetMipCount();
   createInfo.fpUploadSlice = ctGPUTextureUploadFnImage;
   createInfo.uploadData = (void*)pImage;
   return ctGPUExternalTextureCreate(pDevice, pPool, ppTexture, &createInfo);
}