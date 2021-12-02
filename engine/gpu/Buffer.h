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

/*
Buffer manages Structured Buffers and Constant Buffers
Vertex/index buffers can't be fetched for manually and arent supported.
*/

struct ctGPUExternalBufferPool;
struct ctGPUExternalBuffer;

enum ctGPUExternalBufferType {
   CT_GPU_EXTERN_BUFFER_TYPE_STORAGE,
   CT_GPU_EXTERN_BUFFER_TYPE_UNIFORM,
   CT_GPU_EXTERN_BUFFER_TYPE_INDIRECT,
   CT_GPU_EXTERN_BUFFER_TYPE_COUNT
};

typedef void (*ctGPUBufferGenerateFn)(uint8_t* dest, size_t size, void* userData);

struct ctGPUExternalBufferCreateInfo {
   const char* debugName;
   ctGPUExternalBufferType type;
   ctGPUExternalUpdateMode updateMode;

   ctGPUExternalSource source;
   union {
      struct {
         size_t size;
         ctGPUBufferGenerateFn fpGenerationFunction;
         void* pUserData;
      } generate;
      struct {
         ctGPUAssetIdentifier assetIdentifier;
      } load;
   };
};

struct ctGPUExternalBufferPoolCreateInfo {
   int32_t flags;
   size_t reserve;
};

CT_API ctResults ctGPUExternalBufferPoolCreate(struct ctGPUDevice* pDevice,
                                               ctGPUExternalBufferPool** ppPool,
                                               ctGPUExternalBufferPoolCreateInfo* pInfo);
CT_API ctResults ctGPUExternalBufferPoolFlush(struct ctGPUDevice* pDevice,
                                              ctGPUExternalBufferPool* pPool);
CT_API ctResults ctGPUExternalBufferPoolDestroy(struct ctGPUDevice* pDevice,
                                                ctGPUExternalBufferPool* pPool);

CT_API ctResults ctGPUExternalBufferCreate(struct ctGPUDevice* pDevice,
                                           ctGPUExternalBufferPool* pPool,
                                           size_t count,
                                           ctGPUExternalBuffer** ppBuffers,
                                           ctGPUExternalBufferCreateInfo* pInfos);

CT_API ctResults ctGPUExternalBufferRelease(struct ctGPUDevice* pDevice,
                                            ctGPUExternalBufferPool* pPool,
                                            size_t count,
                                            ctGPUExternalBuffer** ppBuffers);

CT_API ctResults ctGPUExternalBufferRequestUpdate(struct ctGPUDevice* pDevice,
                                                  ctGPUExternalBufferPool* pPool,
                                                  size_t count,
                                                  ctGPUExternalBuffer** ppBuffers);

CT_API ctResults ctGPUExternalBufferGetCurrentAccessor(struct ctGPUDevice* pDevice,
                                                       ctGPUExternalBufferPool* pPool,
                                                       ctGPUExternalBuffer* pBuffer,
                                                       ctGPUBufferAccessor* pAccessor);

CT_API ctResults ctGPUExternalBufferGetBindlessIndex(struct ctGPUDevice* pDevice,
                                                     ctGPUExternalBufferPool* pPool,
                                                     ctGPUExternalBuffer* pBuffer,
                                                     int32_t* pIndex);