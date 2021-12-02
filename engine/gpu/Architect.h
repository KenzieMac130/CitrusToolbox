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

#include "Commands.h"
#include "utilities/Common.h"
#include "tiny_imageFormat/tinyimageformat.h"

/* Architect allows the high level gfx programmer
 to lay out the frame and let the backend build
 the required transitions without torturous detail. */

struct ctGPUArchitect;
struct ctGPUArchitectDefinitionContext;

enum ctGPUArchitectTaskCategory {
   CT_GPU_TASK_RASTER,
   CT_GPU_TASK_COMPUTE,
   CT_GPU_TASK_TRANSFER,
   CT_GPU_TASK_RAYTRACE
};

/* Describes the concept of a dependency */
typedef uint32_t ctGPUDependencyID;

struct ctGPUArchitectExecutionContext {
   ctGPUArchitectTaskCategory category;
   union {
      struct {
         uint32_t width;
         uint32_t height;
         uint32_t layerCount;
      } raster;
   };

   ctGPUCommandBuffer cmd;

   void* _internalData;
};

struct ctGPUArchitectCreateInfo {
   int tmp;
};

CT_API ctResults ctGPUArchitectStartup(struct ctGPUDevice* pDevice,
                                       struct ctGPUArchitect** ppArchitect,
                                       struct ctGPUArchitectCreateInfo* pCreateInfo);
CT_API ctResults ctGPUArchitectShutdown(struct ctGPUDevice* pDevice,
                                        struct ctGPUArchitect* pArchitect);

/* Requires https://graphviz.org/ to view or render to an image */
CT_API ctResults ctGPUArchitectDumpGraphVis(struct ctGPUArchitect* pArchitect,
                                            const char* path,
                                            bool generateImage,
                                            bool showImage);

typedef ctResults (*ctGPUArchitectTaskDefinitionFn)(ctGPUArchitectDefinitionContext* pCtx,
                                                    void* pUserData);
typedef ctResults (*ctGPUArchitectTaskExecutionFn)(ctGPUArchitectExecutionContext* pCtx,
                                                   void* pUserData);

enum ctGPUArchitectTaskFlags { CT_GPU_ARCH_TASK_NO_EXTERNAL_RESOURCES = 0x01 };
struct ctGPUArchitectTaskInfo {
   const char* name;
   int32_t flags;
   ctGPUArchitectTaskCategory category;
   ctGPUArchitectTaskDefinitionFn fpDefinition;
   ctGPUArchitectTaskExecutionFn fpExecution;
   void* pUserData;
};

// Todo: look at other ways than explicit build (register with device)
CT_API ctResults ctGPUArchitectBuild(struct ctGPUDevice* pDevice,
                                     struct ctGPUArchitect* pArchitect);
CT_API ctResults ctGPUArchitectReset(struct ctGPUDevice* pDevice,
                                     struct ctGPUArchitect* pArchitect);
CT_API ctResults ctGPUArchitectAddTask(struct ctGPUDevice* pDevice,
                                       struct ctGPUArchitect* pArchitect,
                                       struct ctGPUArchitectTaskInfo* pTaskInfo);
CT_API ctResults ctGPUArchitectExecute(struct ctGPUDevice* pDevice,
                                       struct ctGPUArchitect* pArchitect);
CT_API ctResults ctGPUArchitectSetOutput(struct ctGPUDevice* pDevice,
                                         struct ctGPUArchitect* pArchitect,
                                         ctGPUDependencyID dependency,
                                         uint32_t socket);

/* Use following format:
S_: structured buffer
C_: color target
D_: depth target
B_: barrier */
#define CT_ARCH_ID(_NAME) (uint32_t)(CT_COMPILE_HORNER_HASH(_NAME) % UINT32_MAX)

enum ctGPUArchitectPayloadFlags {
   /* Feedback payloads will be preserved between frames (TAA/scatter/paintmaps) */
   CT_GPU_PAYLOAD_FEEDBACK = 0x01,
   /* Fixed size image payloads are not dependent on the graphs size */
   CT_GPU_PAYLOAD_IMAGE_FIXED_SIZE = 0x02,
   /* 3D textures, uses layers as the depth (froxels/luts/etc) */
   CT_GPU_PAYLOAD_IMAGE_3D = 0x04,
   /* Cubemap textures (ambient/skys/etc) */
   CT_GPU_PAYLOAD_IMAGE_CUBEMAP = 0x08
};

struct ctGPUArchitectClearContents {
   float rgba[4];
   float depth;
   uint32_t stencil;
};

struct ctGPUArchitectImagePayloadDesc {
   int32_t globalBindSlot; /* must be unique, below fixedTextureBindUpperBound */
   const char* debugName;
   int32_t flags;
   float height;
   float width;
   int32_t layers;
   int32_t miplevels;
   TinyImageFormat format;
   ctGPUArchitectClearContents* pClearDesc; /* pointer must remain alive */
};

struct ctGPUArchitectBufferPayloadDesc {
   int32_t globalBindSlot; /* must be unique, below fixedBufferBindUpperBound */
   const char* debugName;
   int32_t flags;
   size_t size;
};

enum ctGPUArchitectResourceAccessFlags {
   /* preserve incoming to read */
   CT_GPU_ACCESS_READ = 0x01,
   /* write as raster attachment or buffer */
   CT_GPU_ACCESS_WRITE = 0x02,
   /* preserve incoming and write to attachment or buffer */
   CT_GPU_ACCESS_READ_WRITE = CT_GPU_ACCESS_READ | CT_GPU_ACCESS_WRITE,
};
typedef uint8_t ctGPUArchitectResourceAccess;

CT_API ctResults ctGPUTaskDeclareDepthTarget(struct ctGPUArchitectDefinitionContext* pCtx,
                                             ctGPUDependencyID id,
                                             ctGPUArchitectImagePayloadDesc* pDesc);
CT_API ctResults ctGPUTaskDeclareColorTarget(struct ctGPUArchitectDefinitionContext* pCtx,
                                             ctGPUDependencyID id,
                                             ctGPUArchitectImagePayloadDesc* pDesc);
CT_API ctResults
ctGPUTaskDeclareStorageBuffer(struct ctGPUArchitectDefinitionContext* pCtx,
                              ctGPUDependencyID id,
                              ctGPUArchitectBufferPayloadDesc* pDesc);

CT_API ctResults ctGPUTaskCreateBarrier(struct ctGPUArchitectDefinitionContext* pCtx,
                                        ctGPUDependencyID id,
                                        const char* debugName,
                                        int32_t flags);

CT_API ctResults ctGPUTaskUseDepthTarget(struct ctGPUArchitectDefinitionContext* pCtx,
                                         ctGPUDependencyID id,
                                         ctGPUArchitectResourceAccess access);
CT_API ctResults ctGPUTaskUseColorTarget(struct ctGPUArchitectDefinitionContext* pCtx,
                                         ctGPUDependencyID id,
                                         ctGPUArchitectResourceAccess access,
                                         uint32_t slot);
CT_API ctResults ctGPUTaskUseTexture(ctGPUArchitectDefinitionContext* pCtx,
                                     ctGPUDependencyID id);
CT_API ctResults ctGPUTaskUseStorageBuffer(struct ctGPUArchitectDefinitionContext* pCtx,
                                           ctGPUDependencyID id,
                                           ctGPUArchitectResourceAccess access);

CT_API ctResults ctGPUTaskWaitBarrier(struct ctGPUArchitectDefinitionContext* pCtx,
                                      ctGPUDependencyID id);
CT_API ctResults ctGPUTaskSignalBarrier(struct ctGPUArchitectDefinitionContext* pCtx,
                                        ctGPUDependencyID id);

/* Accessors */

CT_API ctResults ctGPUTaskGetImageAccessor(struct ctGPUArchitectDefinitionContext* pCtx,
                                           ctGPUDependencyID id,
                                           struct ctGPUImageAccessor* pAccessorOut);
CT_API ctResults ctGPUTaskGetBufferAccessor(struct ctGPUArchitectDefinitionContext* pCtx,
                                            ctGPUDependencyID id,
                                            struct ctGPUBufferAccessor* pAccessorOut);