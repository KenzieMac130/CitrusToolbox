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

CT_API ctResults ctGPUExternalBufferPoolCreate(struct ctGPUDevice* pDevice,
                                               ctGPUExternalBufferPool** ppPool,
                                               ctGPUExternalBufferPoolCreateInfo* pInfo);

CT_API ctResults ctGPUExternalBufferPoolDestroy(struct ctGPUDevice* pDevice,
                                                ctGPUExternalBufferPool* pPool);

CT_API ctResults ctGPUExternalBufferPoolGarbageCollect(struct ctGPUDevice* pDevice,
                                                       ctGPUExternalBufferPool* pPool);

CT_API bool ctGPUExternalBufferPoolNeedsDispatch(struct ctGPUDevice* pDevice,
                                                 ctGPUExternalBufferPool* pPool);

CT_API ctResults ctGPUExternalBufferPoolDispatch(struct ctGPUDevice* pDevice,
                                                 ctGPUExternalBufferPool* pPool,
                                                 ctGPUCommandBuffer cmd);

/* ------------------------------------------------------------------------------------ */

struct ctGPUExternalBufferCreateFuncInfo {
   const char* debugName;
   int32_t desiredBinding;
   bool async;
   ctGPUExternalBuffer* pPlaceholder;
   ctGPUExternalBufferType type;
   ctGPUExternalUpdateMode updateMode;
   size_t size;
   ctGPUBufferGenerateFn generationFunction;
   void* userData;
};

CT_API ctResults ctGPUExternalBufferCreateFunc(struct ctGPUDevice* pDevice,
                                               ctGPUExternalBufferPool* pPool,
                                               ctGPUExternalBuffer** ppBuffer,
                                               ctGPUExternalBufferCreateFuncInfo* pInfo);
struct ctGPUExternalBufferCreateLoadInfo {
   const char* debugName;
   int32_t desiredBinding;
   ctGPUExternalBuffer* pPlaceholder;
   ctGPUExternalBufferType type;
   ctGPUAssetIdentifier* identifier;
   size_t offset;
   size_t size;
};

CT_API ctResults ctGPUExternalBufferCreateLoad(struct ctGPUDevice* pDevice,
                                               ctGPUExternalBufferPool* pPool,
                                               ctGPUExternalBuffer** ppBuffer,
                                               ctGPUExternalBufferCreateLoadInfo* pInfo);

CT_API ctResults ctGPUExternalBufferRebuild(struct ctGPUDevice* pDevice,
                                            ctGPUExternalBufferPool* pPool,
                                            size_t bufferCount,
                                            ctGPUExternalBuffer** ppBuffers);
CT_API ctResults ctGPUExternalBufferRelease(struct ctGPUDevice* pDevice,
                                            ctGPUExternalBufferPool* pPool,
                                            ctGPUExternalBuffer* pBuffer);

CT_API bool ctGPUExternalBufferIsReady(struct ctGPUDevice* pDevice,
                                       ctGPUExternalBufferPool* pPool,
                                       ctGPUExternalBuffer* pBuffer);

/* NOT GUARANTEED TO EXIST BEFORE ctGPUExternalBufferIsReady()! */
CT_API ctResults ctGPUExternalBufferGetCurrentAccessor(struct ctGPUDevice* pDevice,
                                                       ctGPUExternalBufferPool* pPool,
                                                       ctGPUExternalBuffer* pBuffer,
                                                       ctGPUBufferAccessor* pAccessor);
/* NOT GUARANTEED TO EXIST BEFORE ctGPUExternalBufferIsReady()! */
CT_API ctResults ctGPUExternalBufferGetBindlessIndex(struct ctGPUDevice* pDevice,
                                                     ctGPUExternalBufferPool* pPool,
                                                     ctGPUExternalBuffer* pBuffer,
                                                     int32_t* pIndex);