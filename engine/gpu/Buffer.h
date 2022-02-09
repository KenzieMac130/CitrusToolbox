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

#ifdef __cplusplus
extern "C" {
#endif

/*
Buffer manages Structured Buffers and Constant Buffers
Vertex/index buffers can't be fetched for manually and arent supported.
*/

struct ctGPUExternalBufferPool;
struct ctGPUExternalBuffer;

enum ctGPUExternalBufferType {
   CT_GPU_EXTERN_BUFFER_TYPE_STORAGE,
   CT_GPU_EXTERN_BUFFER_TYPE_INDIRECT,
   CT_GPU_EXTERN_BUFFER_TYPE_COUNT
};

typedef void (*ctGPUBufferGenerateFn)(uint8_t* dest, size_t size, void* userData);

/* ------------------------------------------------------------------------------------ */

struct ctGPUExternalBufferPoolCreateInfo {
   int32_t flags;
   size_t baseStagingSize;
   ctGPUAsyncSchedulerFn fpAsyncScheduler;
   void* pAsyncUserData;
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

CT_API bool ctGPUExternalBufferPoolNeedsDispatch(struct ctGPUDevice* pDevice,
                                                 struct ctGPUExternalBufferPool* pPool);

CT_API bool ctGPUExternalBufferPoolNeedsRebind(struct ctGPUDevice* pDevice,
                                               struct ctGPUExternalBufferPool* pPool);

CT_API enum ctResults
ctGPUExternalBufferPoolRebind(struct ctGPUDevice* pDevice,
                              struct ctGPUExternalTexturePool* pPool,
                              ctGPUBindingModel* pBindingModel);

CT_API enum ctResults
ctGPUExternalBufferPoolDispatch(struct ctGPUDevice* pDevice,
                                struct ctGPUExternalBufferPool* pPool,
                                ctGPUCommandBuffer cmd);

/* ------------------------------------------------------------------------------------ */

struct ctGPUExternalBufferCreateFuncInfo {
   const char* debugName;
   bool async;
   struct ctGPUExternalBuffer* pPlaceholder;
   enum ctGPUExternalBufferType type;
   enum ctGPUExternalUpdateMode updateMode;
   size_t size;
   ctGPUBufferGenerateFn generationFunction;
   void* userData;
};

// clang-format off

CT_API enum ctResults ctGPUExternalBufferCreateFunc(struct ctGPUDevice* pDevice,
                                                    struct ctGPUExternalBufferPool* pPool,
                                                    struct ctGPUExternalBuffer** ppBuffer,
                                                    struct ctGPUExternalBufferCreateFuncInfo* pInfo);
struct ctGPUExternalBufferCreateLoadInfo {
   const char* debugName;
   struct ctGPUExternalBuffer* pPlaceholder;
   enum ctGPUExternalBufferType type;
   struct ctGPUAssetIdentifier* identifier;
   size_t offset;
   size_t size;
};

CT_API enum ctResults ctGPUExternalBufferCreateLoad(struct ctGPUDevice* pDevice,
                                                    struct ctGPUExternalBufferPool* pPool,
                                                    struct ctGPUExternalBuffer** ppBuffer,
                                                    struct ctGPUExternalBufferCreateLoadInfo* pInfo);

CT_API enum ctResults ctGPUExternalBufferRebuild(struct ctGPUDevice* pDevice,
                                            struct ctGPUExternalBufferPool* pPool,
                                            size_t bufferCount,
                                            struct ctGPUExternalBuffer** ppBuffers);
CT_API enum ctResults ctGPUExternalBufferRelease(struct ctGPUDevice* pDevice,
                                            struct ctGPUExternalBufferPool* pPool,
                                            struct ctGPUExternalBuffer* pBuffer);

CT_API bool ctGPUExternalBufferIsReady(struct ctGPUDevice* pDevice,
                                       struct ctGPUExternalBufferPool* pPool,
                                       struct ctGPUExternalBuffer* pBuffer);

/* NOT GUARANTEED TO EXIST BEFORE ctGPUExternalBufferIsReady()! */
CT_API enum ctResults ctGPUExternalBufferGetCurrentAccessor(struct ctGPUDevice* pDevice,
                                                       struct ctGPUExternalBufferPool* pPool,
                                                       struct ctGPUExternalBuffer* pBuffer,
                                                       ctGPUBufferAccessor* pAccessor);
/* NOT GUARANTEED TO EXIST BEFORE ctGPUExternalBufferIsReady()! */
CT_API enum ctResults ctGPUExternalBufferGetBindlessIndex(struct ctGPUDevice* pDevice,
                                                     struct ctGPUExternalBufferPool* pPool,
                                                     struct ctGPUExternalBuffer* pBuffer,
                                                     int32_t* pIndex);

// clang-format on

#ifdef __cplusplus
}
#endif