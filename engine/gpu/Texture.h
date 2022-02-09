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

typedef void (*ctGPUTextureGenerateFn)(uint8_t* dest,
                                       struct ctGPUExternalGenerateContext* pCtx,
                                       void* userData);

/* Assumes userdata fits the whole size of the first slice/mip of the texture */
void ctGPUTextureGenerateFnQuickMemcpy(uint8_t* dest,
                                       struct ctGPUExternalGenerateContext* pCtx,
                                       void* userData);

/* ------------------------------------------------------------------------------------ */

struct ctGPUExternalTexturePoolCreateInfo {
   int32_t flags;
   size_t baseStagingSize;
   ctGPUAsyncSchedulerFn fpAsyncScheduler;
   void* pAsyncUserData;
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

CT_API bool ctGPUExternalTexturePoolNeedsRebind(struct ctGPUDevice* pDevice,
                                                struct ctGPUExternalTexturePool* pPool);

CT_API enum ctResults
ctGPUExternalTexturePoolRebind(struct ctGPUDevice* pDevice,
                               struct ctGPUExternalTexturePool* pPool,
                               ctGPUBindingModel* pBindingModel);

/* ------------------------------------------------------------------------------------ */

struct ctGPUExternalTextureCreateFuncInfo {
   const char* debugName;
   bool async;
   struct ctGPUExternalTexture* pPlaceholder;
   enum ctGPUExternalTextureType type;
   enum ctGPUExternalUpdateMode updateMode;
   enum TinyImageFormat format;
   uint32_t height;
   uint32_t width;
   uint32_t depth;
   uint32_t mips;
   ctGPUTextureGenerateFn generationFunction;
   void* userData;
};

CT_API enum ctResults
ctGPUExternalTextureCreateFunc(struct ctGPUDevice* pDevice,
                               struct ctGPUExternalTexturePool* pPool,
                               struct ctGPUExternalTexture** ppTexture,
                               struct ctGPUExternalTextureCreateFuncInfo* pInfo);

struct ctGPUExternalTextureCreateLoadInfo {
   const char* debugName;
   int32_t desiredBinding;
   struct ctGPUExternalTexture* pPlaceholder;
   enum ctGPUExternalTextureType type;
   struct ctGPUAssetIdentifier* identifier;
};

CT_API enum ctResults
ctGPUExternalTextureCreateLoad(struct ctGPUDevice* pDevice,
                               struct ctGPUExternalTexturePool* pPool,
                               struct ctGPUExternalTexture** ppTexture,
                               struct ctGPUExternalTextureCreateLoadInfo* pInfo);

CT_API enum ctResults
ctGPUExternalTextureRebuild(struct ctGPUDevice* pDevice,
                            struct ctGPUExternalTexturePool* pPool,
                            size_t textureCount,
                            struct ctGPUExternalTexture** ppTextures);
CT_API enum ctResults ctGPUExternalTextureRelease(struct ctGPUDevice* pDevice,
                                                  struct ctGPUExternalTexturePool* pPool,
                                                  struct ctGPUExternalTexture* pTexture);

CT_API bool ctGPUExternalTextureIsReady(struct ctGPUDevice* pDevice,
                                        struct ctGPUExternalTexturePool* pPool,
                                        struct ctGPUExternalTexture* pTexture);

/* NOT GUARANTEED TO EXIST BEFORE ctGPUExternalTextureIsReady()! */
CT_API enum ctResults
ctGPUExternalTextureGetCurrentAccessor(struct ctGPUDevice* pDevice,
                                       struct ctGPUExternalTexturePool* pPool,
                                       struct ctGPUExternalTexture* pTexture,
                                       ctGPUImageAccessor* pAccessor);
/* NOT GUARANTEED TO EXIST BEFORE ctGPUExternalTextureIsReady()! */
CT_API enum ctResults
ctGPUExternalTextureGetBindlessIndex(struct ctGPUDevice* pDevice,
                                     struct ctGPUExternalTexturePool* pPool,
                                     struct ctGPUExternalTexture* pTexture,
                                     int32_t* pIndex);

#ifdef __cplusplus
}
#endif