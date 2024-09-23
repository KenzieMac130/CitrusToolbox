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
#include "Device.h"
#include "Commands.h"
#include "tiny_imageFormat/tinyimageformat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Handles all textures */

struct ctGPUExternalTexturePool;
struct ctGPUExternalTexture;

enum ctGPUExternalTextureType {
   CT_GPU_EXTERN_TEXTURE_TYPE_1D,
   CT_GPU_EXTERN_TEXTURE_TYPE_2D,
   CT_GPU_EXTERN_TEXTURE_TYPE_3D,
   CT_GPU_EXTERN_TEXTURE_TYPE_CUBE
};

struct ctGPUExternalGenerateContext {
   enum TinyImageFormat format;
   uint32_t width;
   uint32_t height;
   uint32_t depth;
   uint32_t currentLayer;
   uint32_t currentMipLevel;
};

/* ------------------------------------------------------------------------------------ */

struct ctGPUExternalTexturePoolCreateInfo {
   int32_t flags;
   size_t baseStagingSize;
};

CT_API enum ctResults
ctGPUExternalTexturePoolCreate(struct ctGPUDevice* pDevice,
                               struct ctGPUExternalTexturePool** ppPool,
                               struct ctGPUExternalTexturePoolCreateInfo* pInfo);

CT_API enum ctResults
ctGPUExternalTexturePoolDestroy(struct ctGPUDevice* pDevice,
                                struct ctGPUExternalTexturePool* pPool);

CT_API enum ctResults
ctGPUExternalTexturePoolGarbageCollect(struct ctGPUDevice* pDevice,
                                       struct ctGPUExternalTexturePool* pPool);

CT_API bool ctGPUExternalTexturePoolNeedsDispatch(struct ctGPUDevice* pDevice,
                                                  struct ctGPUExternalTexturePool* pPool);

CT_API enum ctResults
ctGPUExternalTexturePoolDispatch(struct ctGPUDevice* pDevice,
                                 struct ctGPUExternalTexturePool* pPool,
                                 ctGPUCommandBuffer cmd);

/* ------------------------------------------------------------------------------------ */

typedef void (*ctGPUTextureUploadFn)(uint8_t* dest,
                                     struct ctGPUExternalGenerateContext* pCtx,
                                     void* userData);

/* Assumes userdata fits the whole size of the first slice/mip of the texture */
void ctGPUTextureUploadFnQuickMemcpy(uint8_t* dest,
                                     struct ctGPUExternalGenerateContext* pCtx,
                                     void* userData);

struct ctGPUExternalTextureCreateInfo {
   const char* debugName;
   enum ctGPUExternalTextureType type;
   enum ctGPUExternalUpdateMode updateMode;
   enum TinyImageFormat format;
   uint32_t height;
   uint32_t width;
   uint32_t depth;
   uint32_t mips;
   ctGPUTextureUploadFn fpUploadSlice;
   void* uploadData;
};

CT_API enum ctResults
ctGPUExternalTextureCreate(struct ctGPUDevice* pDevice,
                           struct ctGPUExternalTexturePool* pPool,
                           struct ctGPUExternalTexture** ppTexture,
                           struct ctGPUExternalTextureCreateInfo* pInfo);

#ifdef __cplusplus
CT_API enum ctResults
ctGPUExternalTextureFromImage(struct ctGPUDevice* pDevice,
                           struct ctGPUExternalTexturePool* pPool,
                           struct ctGPUExternalTexture** ppTexture,
                           const char* debugName,
                           const class ctImage* pImage);
#endif

CT_API enum ctResults ctGPUExternalTextureUpload(struct ctGPUDevice* pDevice,
                                                 struct ctGPUExternalTexturePool* pPool,
                                                 struct ctGPUExternalTexture* ppTexture,
                                                 ctGPUTextureUploadFn fpUploadSlice,
                                                 void* uploadData);

CT_API enum ctResults ctGPUExternalTextureRelease(struct ctGPUDevice* pDevice,
                                                  struct ctGPUExternalTexturePool* pPool,
                                                  struct ctGPUExternalTexture* pTexture);

CT_API enum ctResults
ctGPUExternalTextureGetCurrentAccessor(struct ctGPUDevice* pDevice,
                                       struct ctGPUExternalTexture* pTexture,
                                       ctGPUImageAccessor* pAccessor);

#ifdef __cplusplus
}
#endif