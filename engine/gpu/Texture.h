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
   TinyImageFormat format;
   uint32_t width;
   uint32_t height;
   uint32_t depth;
   uint32_t currentLayer;
   uint32_t currentMipLevel;
};

typedef void (*ctGPUTextureGenerateFn)(uint8_t* dest,
                                       ctGPUExternalGenerateContext* pCtx,
                                       void* userData);

/* Assumes userdata fits the whole size of the first slice/mip of the texture */
void ctGPUTextureGenerateFnQuickMemcpy(uint8_t* dest,
                                       ctGPUExternalGenerateContext* pCtx,
                                       void* userData);

/* ------------------------------------------------------------------------------------ */

struct ctGPUExternalTexturePoolCreateInfo {
   int32_t flags;
   size_t baseStagingSize;
   ctGPUAsyncSchedulerFn fpAsyncScheduler;
   void* pAsyncUserData;
};

CT_API ctResults
ctGPUExternalTexturePoolCreate(struct ctGPUDevice* pDevice,
                               ctGPUExternalTexturePool** ppPool,
                               ctGPUExternalTexturePoolCreateInfo* pInfo);

CT_API ctResults ctGPUExternalTexturePoolDestroy(struct ctGPUDevice* pDevice,
                                                 ctGPUExternalTexturePool* pPool);

CT_API ctResults ctGPUExternalTexturePoolGarbageCollect(struct ctGPUDevice* pDevice,
                                                        ctGPUExternalTexturePool* pPool);

CT_API bool ctGPUExternalTexturePoolNeedsDispatch(struct ctGPUDevice* pDevice,
                                                  ctGPUExternalTexturePool* pPool);

CT_API ctResults ctGPUExternalTexturePoolDispatch(struct ctGPUDevice* pDevice,
                                                  ctGPUExternalTexturePool* pPool,
                                                  ctGPUCommandBuffer cmd);

/* ------------------------------------------------------------------------------------ */

struct ctGPUExternalTextureCreateFuncInfo {
   const char* debugName;
   int32_t desiredBinding;
   bool async;
   ctGPUExternalTexture* pPlaceholder;
   ctGPUExternalTextureType type;
   ctGPUExternalUpdateMode updateMode;
   TinyImageFormat format;
   uint32_t height;
   uint32_t width;
   uint32_t depth;
   uint32_t mips;
   ctGPUTextureGenerateFn generationFunction;
   void* userData;
};

CT_API ctResults
ctGPUExternalTextureCreateFunc(struct ctGPUDevice* pDevice,
                               ctGPUExternalTexturePool* pPool,
                               ctGPUExternalTexture** ppTexture,
                               ctGPUExternalTextureCreateFuncInfo* pInfo);

struct ctGPUExternalTextureCreateLoadInfo {
   const char* debugName;
   int32_t desiredBinding;
   ctGPUExternalTexture* pPlaceholder;
   ctGPUExternalTextureType type;
   ctGPUAssetIdentifier* identifier;
};

CT_API ctResults
ctGPUExternalTextureCreateLoad(struct ctGPUDevice* pDevice,
                               ctGPUExternalTexturePool* pPool,
                               ctGPUExternalTexture** ppTexture,
                               ctGPUExternalTextureCreateLoadInfo* pInfo);

CT_API ctResults ctGPUExternalTextureRebuild(struct ctGPUDevice* pDevice,
                                             ctGPUExternalTexturePool* pPool,
                                             size_t textureCount,
                                             ctGPUExternalTexture** ppTextures);
CT_API ctResults ctGPUExternalTextureRelease(struct ctGPUDevice* pDevice,
                                             ctGPUExternalTexturePool* pPool,
                                             ctGPUExternalTexture* pTexture);

CT_API bool ctGPUExternalTextureIsReady(struct ctGPUDevice* pDevice,
                                        ctGPUExternalTexturePool* pPool,
                                        ctGPUExternalTexture* pTexture);

/* NOT GUARANTEED TO EXIST BEFORE ctGPUExternalTextureIsReady()! */
CT_API ctResults ctGPUExternalTextureGetCurrentAccessor(struct ctGPUDevice* pDevice,
                                                        ctGPUExternalTexturePool* pPool,
                                                        ctGPUExternalTexture* pTexture,
                                                        ctGPUImageAccessor* pAccessor);
/* NOT GUARANTEED TO EXIST BEFORE ctGPUExternalTextureIsReady()! */
CT_API ctResults ctGPUExternalTextureGetBindlessIndex(struct ctGPUDevice* pDevice,
                                                      ctGPUExternalTexturePool* pPool,
                                                      ctGPUExternalTexture* pTexture,
                                                      int32_t* pIndex);