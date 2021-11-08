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

typedef ctResults (*ctGPUOpenCacheFileFn)(void* pTargetCtFile,
                                          const char* path,
                                          int fileMode,
                                          void* pUserData);

struct ctGPUDeviceCreateInfo {
   void* pMainWindow;

   const char* appName;
   int32_t version[3];
   bool validationEnabled;
   bool useVSync;

   int32_t fixedTextureBindUpperBound;
   int32_t fixedBufferBindUpperBound;

   ctGPUOpenCacheFileFn fpOpenCacheFileCallback;
   void* pCacheCallbackCustomData;
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