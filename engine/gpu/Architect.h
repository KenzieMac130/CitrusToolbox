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

#ifdef __cplusplus
extern "C" {
#endif

/* Architect allows the high level gfx programmer
 to lay out the frame and let the backend build
 the required transitions without torturous detail. */

struct ctGPUArchitect;
struct ctGPUArchitectDefinitionContext;

enum ctGPUArchitectTaskCategory {
   CT_GPU_TASK_UNDEFINED,
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

   struct ctGPUArchitect* pArchitect;
   struct ctGPUDevice* pDevice;
   void* _internalData;
};

struct ctGPUArchitectCreateInfo {
   int tmp;
};

CT_API enum ctResults ctGPUArchitectStartup(struct ctGPUDevice* pDevice,
                                            struct ctGPUArchitect** ppArchitect,
                                            struct ctGPUArchitectCreateInfo* pCreateInfo);
CT_API enum ctResults ctGPUArchitectShutdown(struct ctGPUDevice* pDevice,
                                             struct ctGPUArchitect* pArchitect);

/* Requires https://graphviz.org/ to generate to an image */
CT_API ctResults ctGPUArchitectDumpGraphVis(struct ctGPUArchitect* pArchitect,
                                            const char* path,
                                            bool generateImage,
                                            bool showImage);

typedef enum ctResults (*ctGPUArchitectTaskDefinitionFn)(
  struct ctGPUArchitectDefinitionContext* pCtx, void* pUserData);
typedef enum ctResults (*ctGPUArchitectTaskExecutionFn)(
  struct ctGPUArchitectExecutionContext* pCtx, void* pUserData);

enum ctGPUArchitectTaskFlags { CT_GPU_ARCH_TASK_NO_EXTERNAL_RESOURCES = 0x01 };
struct ctGPUArchitectTaskInfo {
   const char* name;
   int32_t flags;
   enum ctGPUArchitectTaskCategory category;
   ctGPUArchitectTaskDefinitionFn fpDefinition;
   ctGPUArchitectTaskExecutionFn fpExecution;
   void* pUserData;
};

// Todo: look at other ways than explicit build (register with device)
struct ctGPUArchitectBuildInfo {
   uint32_t width;
   uint32_t height;
};

CT_API enum ctResults ctGPUArchitectBuild(struct ctGPUDevice* pDevice,
                                          struct ctGPUArchitect* pArchitect,
                                          struct ctGPUArchitectBuildInfo* pInfo);
CT_API enum ctResults ctGPUArchitectReset(struct ctGPUDevice* pDevice,
                                          struct ctGPUArchitect* pArchitect);
CT_API enum ctResults ctGPUArchitectAddTask(struct ctGPUDevice* pDevice,
                                            struct ctGPUArchitect* pArchitect,
                                            struct ctGPUArchitectTaskInfo* pTaskInfo);
CT_API enum ctResults ctGPUArchitectExecute(struct ctGPUDevice* pDevice,
                                            struct ctGPUArchitect* pArchitect);
CT_API enum ctResults ctGPUArchitectSetOutput(struct ctGPUDevice* pDevice,
                                              struct ctGPUArchitect* pArchitect,
                                              ctGPUDependencyID dependency,
                                              uint32_t socket);

#ifdef __cplusplus
/* Use following format:
S_: structured buffer
C_: color target
D_: depth target
B_: barrier */
#define CT_ARCH_ID(_NAME)         (uint32_t)(CT_COMPILE_HORNER_HASH(_NAME) % UINT32_MAX)
#define CT_ARCH_DYNAMIC_ID(_NAME) (uint32_t)(ctHornerHash(_NAME) % UINT32_MAX)
#endif

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
   enum TinyImageFormat format;
   struct ctGPUArchitectClearContents* pClearDesc; /* pointer must remain alive */
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
   CT_GPU_ACCESS_NONE = 0
};
typedef uint8_t ctGPUArchitectResourceAccess;

// clang-format off

CT_API enum ctResults ctGPUTaskDeclareDepthTarget(struct ctGPUArchitectDefinitionContext* pCtx,
                                             ctGPUDependencyID id,
                                             struct ctGPUArchitectImagePayloadDesc* pDesc);

CT_API enum ctResults ctGPUTaskDeclareColorTarget(struct ctGPUArchitectDefinitionContext* pCtx,
                                             ctGPUDependencyID id,
                                             struct ctGPUArchitectImagePayloadDesc* pDesc);

CT_API enum ctResults ctGPUTaskDeclareStorageBuffer(struct ctGPUArchitectDefinitionContext* pCtx,
                              ctGPUDependencyID id,
                              struct ctGPUArchitectBufferPayloadDesc* pDesc);

CT_API enum ctResults ctGPUTaskCreateBarrier(struct ctGPUArchitectDefinitionContext* pCtx,
                                        ctGPUDependencyID id,
                                        const char* debugName,
                                        int32_t flags);

CT_API enum ctResults ctGPUTaskUseDepthTarget(struct ctGPUArchitectDefinitionContext* pCtx,
                                         ctGPUDependencyID id,
                                         ctGPUArchitectResourceAccess access);

CT_API enum ctResults ctGPUTaskUseColorTarget(struct ctGPUArchitectDefinitionContext* pCtx,
                                         ctGPUDependencyID id,
                                         ctGPUArchitectResourceAccess access,
                                         uint32_t slot);

CT_API enum ctResults ctGPUTaskUseTexture(ctGPUArchitectDefinitionContext* pCtx,
                                          ctGPUDependencyID id);

CT_API enum ctResults ctGPUTaskUseStorageBuffer(struct ctGPUArchitectDefinitionContext* pCtx,
                                                ctGPUDependencyID id,
                                                ctGPUArchitectResourceAccess access);

CT_API enum ctResults ctGPUTaskWaitBarrier(struct ctGPUArchitectDefinitionContext* pCtx,
                                           ctGPUDependencyID id);

CT_API enum ctResults ctGPUTaskSignalBarrier(struct ctGPUArchitectDefinitionContext* pCtx,
                                             ctGPUDependencyID id);

/* Accessors */
CT_API enum ctResults ctGPUTaskGetImageAccessor(struct ctGPUArchitectExecutionContext* pCtx,
                                                ctGPUDependencyID id,
                                                ctGPUImageAccessor* pAccessorOut);
CT_API enum ctResults ctGPUTaskGetBufferAccessor(struct ctGPUArchitectExecutionContext* pCtx,
                                                 ctGPUDependencyID id,
                                                 ctGPUBufferAccessor* pAccessorOut);
// clang-format on
#ifdef __cplusplus
}
#endif