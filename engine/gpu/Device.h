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

/* Central hub to interact with the graphics api/gpu. */
struct ctGPUDevice;

/* This is pretty opaque to the abstraction layer so change as needbe */
struct ctGPUAssetIdentifier {
   uint8_t guidData[16];
};

typedef enum ctResults (*ctGPUOpenCacheFileFn)(void* pTargetCtFile,
                                               const char* path,
                                               int fileMode,
                                               void* pUserData);
typedef enum ctResults (*ctGPUOpenAssetFileFn)(
  void* pTargetCtFile, struct ctGPUAssetIdentifier* pAssetIdentifier, void* pUserData);

struct ctGPUDeviceCreateInfo {
   void* pMainWindow; /* Expects an SDL window if applicable */

   const char* appName;
   int32_t version[3];
   bool validationEnabled;

   ctGPUOpenCacheFileFn fpOpenCacheFileCallback;
   void* pCacheCallbackCustomData;

   ctGPUOpenAssetFileFn fpOpenAssetFileCallback;
   void* pAssetCallbackCustomData;
};

enum ctGPUFeatureLevel {
   CT_GPU_FEATURE_LEVEL_INCOMPATIBLE,
   CT_GPU_FEATURE_LEVEL_DESKTOP_2020_Q1 /* Assumed on current-gen desktop/console */
};

struct ctGPUDeviceCapabilities {
   enum ctGPUFeatureLevel featureLevel;

   bool hasRobustIndirect;
   bool hasMeshShaders;
   bool hasRaytracing;

   bool hasMultiWindow;
};

/* todo: format support */

CT_API enum ctResults
ctGPUDeviceStartup(struct ctGPUDevice** ppDevice,
                   struct ctGPUDeviceCreateInfo* pCreateInfo,
                   struct ctGPUDeviceCapabilities* pCapabilitiesOut);
CT_API void ctGPUDeviceWaitForIdle(struct ctGPUDevice* pDevice);
CT_API enum ctResults ctGPUDeviceShutdown(struct ctGPUDevice* pDevice);

/* Common data types  */
typedef enum ctResults (*ctGPUAsyncWorkFn)(void*);
typedef void (*ctGPUAsyncSchedulerFn)(ctGPUAsyncWorkFn, void* pData, void* pUserData);

enum ctGPUExternalUpdateMode {
   CT_GPU_UPDATE_STATIC,  /* Statics are only ever uploaded once */
   CT_GPU_UPDATE_DYNAMIC, /* Dynamics are updated infrequently and are costly to write */
   CT_GPU_UPDATE_STREAM   /* Streams are always expected to be updated before use */
};

enum ctGPUSampleCounts {
   CT_GPU_SAMPLES_1 = 1,
   CT_GPU_SAMPLES_2 = 2,
   CT_GPU_SAMPLES_4 = 4,
   CT_GPU_SAMPLES_8 = 8,
   CT_GPU_SAMPLES_16 = 16,
   CT_GPU_SAMPLES_32 = 32,
   CT_GPU_SAMPLES_64 = 64
};

enum ctGPUFaceMask {
   CT_GPU_FACE_NONE = 0,
   CT_GPU_FACE_FRONT = 0x01,
   CT_GPU_FACE_BACK = 0x02,
   CT_GPU_FACE_BOTH = CT_GPU_FACE_FRONT | CT_GPU_FACE_BACK
};

enum ctGPUBlendingMode {
   CT_GPU_BLEND_ADD,
   CT_GPU_BLEND_SUBTRACT,
   CT_GPU_BLEND_MULTIPLY,
   CT_GPU_BLEND_MAX,
   CT_GPU_BLEND_MIN,
   CT_GPU_BLEND_LERP,
   CT_GPU_BLEND_OVERWRITE,
   CT_GPU_BLEND_DISCARD
};

typedef void* ctGPUBufferAccessor;
typedef void* ctGPUImageAccessor;
typedef struct ctGPUBindlessManager ctGPUBindingModel;

#ifdef __cplusplus
}
#endif