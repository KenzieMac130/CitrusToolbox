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

/* Device pretty much manages all the api device, gpu, swapchain, etc. */
struct ctGPUDevice;

/* This is pretty opaque to the abstraction layer so change as needbe */
struct ctGPUAssetIdentifier {
   uint8_t guidData[16];
};

typedef ctResults (*ctGPUOpenCacheFileFn)(void* pTargetCtFile,
                                          const char* path,
                                          int fileMode,
                                          void* pUserData);
typedef ctResults (*ctGPUOpenAssetFileFn)(void* pTargetCtFile,
                                          ctGPUAssetIdentifier* pAssetIdentifier,
                                          void* pUserData);

struct ctGPUDeviceCreateInfo {
   void* pMainWindow; /* Expects an SDL window if applicable*/

   const char* appName;
   int32_t version[3];
   bool validationEnabled;
   bool useVSync;

   int32_t fixedTextureBindUpperBound;
   int32_t fixedBufferBindUpperBound;

   ctGPUOpenCacheFileFn fpOpenCacheFileCallback;
   void* pCacheCallbackCustomData;

   ctGPUOpenAssetFileFn fpOpenAssetFileCallback;
   void* pAssetCallbackCustomData;
};

struct ctGPUDeviceCapabilities {
   bool hasRobustIndirect;
   bool hasMeshShaders;
   bool hasRaytracing;
};

CT_API ctResults ctGPUDeviceStartup(struct ctGPUDevice** ppDevice,
                                    struct ctGPUDeviceCreateInfo* pCreateInfo,
                                    ctGPUDeviceCapabilities* pCapabilitiesOut);
CT_API ctResults ctGPUDeviceShutdown(struct ctGPUDevice* pDevice);
CT_API ctResults ctGPUDeviceNextFrame(struct ctGPUDevice* pDevice);

/* Common data types  */

enum ctGPUExternalUpdateMode {
   CT_GPU_UPDATE_STATIC,  /* Statics are only ever uploaded once */
   CT_GPU_UPDATE_DYNAMIC, /* Dynamics are updated infrequently and are costly to write */
   CT_GPU_UPDATE_STREAM   /* Streams are always expected to be updated before use */
};

enum ctGPUExternalSource { CT_GPU_EXTERN_SOURCE_GENERATE, CT_GPU_EXTERN_SOURCE_LOAD };

enum ctGPUSampleCounts {
   CT_GPU_SAMPLES_1 = 1,
   CT_GPU_SAMPLES_2 = 2,
   CT_GPU_SAMPLES_4 = 4,
   CT_GPU_SAMPLES_8 = 8,
   CT_GPU_SAMPLES_16 = 16,
   CT_GPU_SAMPLES_32 = 32,
   CT_GPU_SAMPLES_64 = 64
};

enum ctGPUFaceBits {
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