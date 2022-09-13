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

#ifdef __cplusplus
extern "C" {
#endif

struct ctGPUBindlessManagerCreateInfo {
   int32_t fixedTextureBindUpperBound;
   int32_t fixedStorageBufferBindUpperBound;
};

/* Sets up the bindless manager */
CT_API enum ctResults
ctGPUBindlessManagerStartup(struct ctGPUDevice* pDevice,
                            struct ctGPUBindlessManager** ppBindless,
                            struct ctGPUBindlessManagerCreateInfo* pCreateInfo);

/* Shutdowns the bindless manager */
CT_API enum ctResults
ctGPUBindlessManagerShutdown(struct ctGPUDevice* pDevice,
                             struct ctGPUBindlessManager* pBindless);

/* Updates the mappings for the frame after build but before execution */
CT_API enum ctResults
ctGPUBindlessManagerPrepareFrame(struct ctGPUDevice* pDevice,
                                 struct ctGPUBindlessManager* pBindless,
                                 uint32_t bufferPoolCount,
                                 struct ctGPUExternalBufferPool** ppBufferPools,
                                 uint32_t texturePoolCount,
                                 struct ctGPUExternalTexturePool** ppTexturePools,
                                 uint32_t architectCount,
                                 struct ctGPUArchitect** ppArchitects,
                                 void* pNext);

/* Texture must be ready, desired index -1: auto */
CT_API int32_t ctGPUBindlessManagerMapTexture(struct ctGPUDevice* pDevice,
                                              struct ctGPUBindlessManager* pBindless,
                                              int32_t desiredIdx,
                                              struct ctGPUExternalTexture* pTexture);

CT_API void ctGPUBindlessManagerUnmapTexture(struct ctGPUDevice* pDevice,
                                             struct ctGPUBindlessManager* pBindless,
                                             int32_t index,
                                             struct ctGPUExternalTexture* pTexture);

/* Buffer must be ready, desired index -1: auto */
CT_API int32_t
ctGPUBindlessManagerMapStorageBuffer(struct ctGPUDevice* pDevice,
                                     struct ctGPUBindlessManager* pBindless,
                                     int32_t desiredIdx,
                                     struct ctGPUExternalBuffer* pBuffer);

CT_API void ctGPUBindlessManagerUnmapStorageBuffer(struct ctGPUDevice* pDevice,
                                                   struct ctGPUBindlessManager* pBindless,
                                                   int32_t index,
                                                   struct ctGPUExternalBuffer* pBuffer);

#ifdef __cplusplus
}
#endif