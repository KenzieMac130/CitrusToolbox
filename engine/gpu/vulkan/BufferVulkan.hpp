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

#include "gpu/Buffer.h"
#include "DeviceVulkan.hpp"

struct ctGPUExternalBuffer : ctVkConveyorBeltResource {
   ctGPUExternalBufferCreateInfo createInfo;
   int32_t currentDynamicFrame;
   ctVkCompleteBuffer data[CT_MAX_INFLIGHT_FRAMES];
   uint8_t* mappings[CT_MAX_INFLIGHT_FRAMES];
   ctFile file;
   size_t bufferSize;

   virtual ctResults Create(ctGPUDevice* pDevice);
   virtual ctResults Update(ctGPUDevice* pDevice);
   virtual ctResults Free(ctGPUDevice* pDevice);

   void PopulateContents(ctGPUDevice* pDevice);
};

struct ctGPUExternalBufferPool : ctVkConveyorBeltPool {
   ctGPUExternalBufferPool(ctGPUExternalBufferPoolCreateInfo* pInfo);
};