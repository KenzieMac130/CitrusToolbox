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

#ifdef __cplusplus
extern "C" {
#endif

/*
Manages Buffers
*/

struct ctGPUExternalBufferPool;
struct ctGPUExternalBuffer;

enum ctGPUExternalBufferType {
   CT_GPU_EXTERN_BUFFER_TYPE_STORAGE,
   CT_GPU_EXTERN_BUFFER_TYPE_UNIFORM,
   CT_GPU_EXTERN_BUFFER_TYPE_INDIRECT,
   CT_GPU_EXTERN_BUFFER_TYPE_VERTEX,
   CT_GPU_EXTERN_BUFFER_TYPE_INDEX,
   CT_GPU_EXTERN_BUFFER_TYPE_COUNT
};

typedef void (*ctGPUBufferGenerateFn)(uint8_t* dest, size_t size, void* userData);

/* ------------------------------------------------------------------------------------ */

struct ctGPUExternalBufferPoolCreateInfo {
   int32_t flags;
   size_t baseStagingSize;
};

CT_API enum ctResults
ctGPUExternalBufferPoolCreate(struct ctGPUDevice* pDevice,
                              struct ctGPUExternalBufferPool** ppPool,
                              struct ctGPUExternalBufferPoolCreateInfo* pInfo);

CT_API enum ctResults
ctGPUExternalBufferPoolDestroy(struct ctGPUDevice* pDevice,
                               struct ctGPUExternalBufferPool* pPool);

CT_API enum ctResults
ctGPUExternalBufferPoolGarbageCollect(struct ctGPUDevice* pDevice,
                                      struct ctGPUExternalBufferPool* pPool);

CT_API bool ctGPUExternalBufferPoolNeedsDispatch(ctGPUDevice* pDevice,
                                                 ctGPUExternalBufferPool* pPool);

CT_API enum ctResults
ctGPUExternalBufferPoolDispatch(struct ctGPUDevice* pDevice,
                                struct ctGPUExternalBufferPool* pPool,
                                ctGPUCommandBuffer cmd);

/* ------------------------------------------------------------------------------------ */

struct ctGPUExternalBufferCreateInfo {
   const char* debugName;
   enum ctGPUExternalBufferType type;
   enum ctGPUExternalUpdateMode updateMode;
   size_t size;
   uint8_t* data;
};

// clang-format off

CT_API enum ctResults ctGPUExternalBufferCreate(struct ctGPUDevice* pDevice,
                                                    struct ctGPUExternalBufferPool* pPool,
                                                    struct ctGPUExternalBuffer** ppBuffer,
                                                    struct ctGPUExternalBufferCreateInfo* pInfo);
CT_API enum ctResults ctGPUExternalBufferUploadMap(struct ctGPUDevice* pDevice,
                                                struct ctGPUExternalBufferPool* pPool,
                                                struct ctGPUExternalBuffer* pBuffer,
                                                void** ppDest);
CT_API enum ctResults ctGPUExternalBufferUploadFlush(struct ctGPUDevice* pDevice,
                                                   struct ctGPUExternalBufferPool* pPool,
                                                   struct ctGPUExternalBuffer* pBuffer);

CT_API enum ctResults ctGPUExternalBufferRelease(struct ctGPUDevice* pDevice,
                                            struct ctGPUExternalBufferPool* pPool,
                                            struct ctGPUExternalBuffer* pBuffer);

CT_API enum ctResults ctGPUExternalBufferGetCurrentAccessor(struct ctGPUDevice* pDevice,
                                                       struct ctGPUExternalBuffer* pBuffer,
                                                       ctGPUBufferAccessor* pAccessor);

// clang-format on

#ifdef __cplusplus
}
#endif
