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

struct ctGPUExternalBuffer {
   size_t size;
   ctGPUExternalBufferType type;
   ctGPUExternalUpdateMode updateMode;

   uint32_t frameCount;
   uint32_t currentFrame;

   /* Contents */
   ctVkCompleteBuffer contents[CT_MAX_INFLIGHT_FRAMES];
   void AllocateContents(ctGPUDevice* pDevice,
                         VkBufferUsageFlags usage,
                         VmaMemoryUsage memUsage,
                         const char* name);
   void DestroyContents(ctGPUDevice* pDevice);

   /* Staging */
   ctVkCompleteBuffer staging[CT_MAX_INFLIGHT_FRAMES];
   void AquireStaging(ctGPUDevice* pDevice);
   void ReleaseStaging(ctGPUDevice* pDevice);

   /* Mappings */
   uint8_t* mappings[CT_MAX_INFLIGHT_FRAMES];
   void GenMappings(ctGPUDevice* pDevice);
   void FreeMappings(ctGPUDevice* pDevice);

   /* Bindless */
   int32_t bindlessIndices[CT_MAX_INFLIGHT_FRAMES];
   void GenBindless(ctGPUDevice* pDevice);
   void InvalidateBindless();
   void FreeBindless(ctGPUDevice* pDevice);

   /* Generation */
   ctGPUExternalBufferPool* pPool;
   ctGPUBufferGenerateFn generationFunction;
   void* userData;
   void GenerateContents();

   /* Async features */
   bool wantsAsync;
   ctAtomic contentsReady;
   inline bool isReady() {
      return ctAtomicGet(contentsReady);
   }
   inline void MakeReady(bool val) {
      ctAtomicSet(contentsReady, val);
   }
   inline void NextFrame() {
      currentFrame = (currentFrame + 1) % frameCount;
   }

   /* Commands */
   void ExecuteCommands(VkCommandBuffer cmd);
};

struct ctGPUExternalBufferPool {
   ctGPUExternalBufferPool(ctGPUExternalBufferPoolCreateInfo* pInfo);

   void GarbageCollect(ctGPUDevice* pDevice);

   ctGPUAsyncSchedulerFn fpAsyncScheduler;
   void* pAsyncUserData;

   /* Hot list is protected by the spinlock and access is in critical section */
   ctSpinLock uploadListLock;
   ctDynamicArray<ctGPUExternalBuffer*> gpuCmdUpdateListHot;
   void CommitHotList();
   void AddToUpload(ctGPUExternalBuffer* pBuffer);
   /* This list will be commited from the hot list and is threadsafe */
   ctDynamicArray<ctGPUExternalBuffer*> gpuCmdUpdateList;
   ctDynamicArray<ctGPUExternalBuffer*> incompleteGarbageList;
   ctDynamicArray<ctGPUExternalBuffer*> garbageList;
};