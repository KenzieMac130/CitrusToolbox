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
#include "gpu/Architect.h"

class ctGPUArchitectBackend {
public:
   virtual ctResults Startup(struct ctGPUDevice* pDevice) = 0;
   virtual ctResults Shutdown(struct ctGPUDevice* pDevice) = 0;
   virtual ctResults BuildInternal(struct ctGPUDevice* pDevice,
                                   struct ctGPUArchitect* pArchitect) = 0;
   virtual ctResults ExecuteInternal(struct ctGPUDevice* pDevice,
                                     struct ctGPUArchitect* pArchitect) = 0;
   virtual ctResults ResetInternal(struct ctGPUDevice* pDevice,
                                   struct ctGPUArchitect* pArchitect) = 0;
   virtual ctResults OptimizeOrder(ctGPUDevice* pDevice,
                                   struct ctGPUArchitect* pArchitect) = 0;
};
ctGPUArchitectBackend* ctGPUNewArchitectBackend(ctGPUDevice* pDevice);

struct ctGPUArchitectImagePayload {
   ctGPUArchitectImagePayload();
   ctGPUArchitectImagePayload(const ctGPUArchitectImagePayloadDesc& desc,
                              ctGPUDependencyID id);
   ctGPUDependencyID identifier;
   int32_t bindSlot;
   char debugName[32];
   int32_t flags;
   bool fixedSize;
   float height;
   float width;
   TinyImageFormat format;
};

struct ctGPUArchitectBufferPayload {
   ctGPUArchitectBufferPayload();
   ctGPUArchitectBufferPayload(const ctGPUArchitectBufferPayloadDesc& desc,
                               ctGPUDependencyID depIdx);
   ctGPUDependencyID identifier;
   int32_t bindSlot;
   char debugName[32];
   int32_t flags;
   size_t size;
};

struct ctGPUArchitectBarrierPayload {
   ctGPUArchitectBarrierPayload();
   ctGPUArchitectBarrierPayload(const char* inDebugName,
                                int32_t flags,
                                ctGPUDependencyID id);
   ctGPUDependencyID identifier;
   char debugName[32];
   int32_t flags;
};

enum ctGPUArchitectDependencyType {
   CT_GPU_ARCH_BARRIER,
   CT_GPU_ARCH_DEPTH_TARGET,
   CT_GPU_ARCH_COLOR_TARGET,
   CT_GPU_ARCH_STORAGE_BUFFER,
   CT_GPU_ARCH_TEXTURE,
   CT_GPU_ARCH_STORAGE_MAX = UINT8_MAX
};

struct ctGPUArchitectDependencyEntry {
   ctGPUDependencyID resourceId;
   ctGPUArchitectResourceAccess access;
   ctGPUArchitectDependencyType type;
};

struct ctGPUArchitectTaskInternal {
   char debugName[32];

   ctGPUArchitectTaskCategory category;
   ctGPUArchitectTaskDefinitionFn fpDefinition;
   ctGPUArchitectTaskExecutionFn fpExecution;
   void* pUserData;

   ctStaticArray<ctGPUArchitectImagePayload, CT_MAX_GFX_TASK_BUFFERS> images;
   ctStaticArray<ctGPUArchitectBufferPayload, CT_MAX_GFX_TASK_IMAGES> buffers;
   ctStaticArray<ctGPUArchitectBarrierPayload, CT_MAX_GFX_TASK_IMAGES> barriers;
   ctStaticArray<ctGPUArchitectDependencyEntry, CT_MAX_GFX_TASK_DEPENDENCIES>
     dependencies;
};

struct ctGPUArchitectDefinitionContext {
   ctGPUArchitectTaskInternal* pInternal;
};
struct ctGPUArchitectExecutionContext {};
#define OUTPUT_TASK_ID INT32_MAX

struct ctGPUArchitect {
   ctGPUArchitectBackend* pBackend;
   ctDynamicArray<ctGPUArchitectTaskInternal> tasks;

   /* Main User Functions */
   ctResults Validate();
   ctResults DumpGraphVis(const char* path, bool generateImage, bool showImage);
   ctResults Build(ctGPUDevice* pDevice);

   /* Fetching Functions */
   inline class ctGPUArchitectTaskIterator
   GetFinalTaskIterator(ctGPUArchitectTaskCategory cat);

   /* Caches */
   ctResults ResetCache(ctGPUDevice* pDevice);
   bool isCacheBuilt;
   ctDynamicArray<int32_t> cTaskFinalCutList;
   ctHashTable<bool, ctGPUDependencyID> cDependenciesSafeToRead;
   ctDynamicArray<int32_t> cFinalOrderList;

   /* Build Phases */
   int32_t InitialFindNextWithDependencyMet();
   bool InitialAreDependenciesMet(const ctGPUArchitectTaskInternal& task);
   void InitialMarkDependencyWrites(const ctGPUArchitectTaskInternal& task);
};

/* Iterator over final tasks for a queue */
class ctGPUArchitectTaskIterator {
public:
   inline ctGPUArchitectTaskIterator(ctGPUArchitect* pArch,
                                     ctGPUArchitectTaskCategory cat) {
      pArchitect = pArch;
      category = cat;
      orderedIdx = 0;
   }
   inline ctGPUArchitectTaskIterator(const ctGPUArchitectTaskIterator& orig) {
      pArchitect = orig.pArchitect;
      category = orig.category;
      orderedIdx = orig.orderedIdx;
   }
   inline ctGPUArchitectTaskInternal& Task() const {
      pArchitect->tasks[pArchitect->cFinalOrderList[orderedIdx]];
   }
   inline ctGPUArchitectTaskIterator& operator++() {
      ctGPUArchitectTaskInternal& task = Task();
      while (task.category != category &&
             orderedIdx < pArchitect->cFinalOrderList.Count()) {
         orderedIdx++;
      }
      return *this;
   }
   inline ctGPUArchitectTaskIterator& operator--() {
      ctGPUArchitectTaskInternal& task = Task();
      while (task.category != category && orderedIdx >= 0) {
         orderedIdx--;
      }
      return *this;
   }
   inline operator bool() const {
      return orderedIdx < pArchitect->cFinalOrderList.Count() && orderedIdx >= 0;
   }

private:
   ctGPUArchitect* pArchitect;
   ctGPUArchitectTaskCategory category;
   int32_t orderedIdx;
};

inline class ctGPUArchitectTaskIterator
ctGPUArchitect::GetFinalTaskIterator(ctGPUArchitectTaskCategory cat) {
   return ctGPUArchitectTaskIterator(this, cat);
}