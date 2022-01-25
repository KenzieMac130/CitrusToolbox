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

struct ctGPUArchitectImagePayload {
   ctGPUArchitectImagePayload();
   ctGPUArchitectImagePayload(const ctGPUArchitectImagePayloadDesc& desc,
                              ctGPUDependencyID id);
   ctGPUDependencyID identifier;
   int32_t bindSlot;
   char debugName[32];
   int32_t flags;
   float height;
   float width;
   int32_t layers;
   int32_t miplevels;
   TinyImageFormat format;
   ctGPUArchitectClearContents* pClearDesc;

   void* apiData;
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

   void* apiData;
};

struct ctGPUArchitectBarrierPayload {
   ctGPUArchitectBarrierPayload();
   ctGPUArchitectBarrierPayload(const char* inDebugName,
                                int32_t flags,
                                ctGPUDependencyID id);
   ctGPUDependencyID identifier;
   char debugName[32];
   int32_t flags;

   void* apiData;
};

enum ctGPUArchitectDependencyType {
   CT_GPU_ARCH_INVALID,
   CT_GPU_ARCH_BARRIER,
   CT_GPU_ARCH_DEPTH_TARGET,
   CT_GPU_ARCH_COLOR_TARGET,
   CT_GPU_ARCH_STORAGE_BUFFER,
   CT_GPU_ARCH_TEXTURE,
   CT_GPU_ARCH_STORAGE_MAX = UINT8_MAX
};

struct ctGPUArchitectDependencyEntry {
   inline bool isValid() {
      return resourceId != 0 && access != 0 && type != CT_GPU_ARCH_INVALID;
   }
   ctGPUDependencyID resourceId;
   ctGPUArchitectResourceAccess access;
   ctGPUArchitectDependencyType type;
   uint8_t slot;
};

struct ctGPUArchitectTaskInternal {
   char debugName[32];
   int32_t debugIdx;

   ctGPUArchitectTaskCategory category;
   ctGPUArchitectTaskDefinitionFn fpDefinition;
   ctGPUArchitectTaskExecutionFn fpExecution;
   void* pUserData;

   ctStaticArray<ctGPUArchitectImagePayload, CT_MAX_GFX_TASK_BUFFERS> images;
   ctStaticArray<ctGPUArchitectBufferPayload, CT_MAX_GFX_TASK_IMAGES> buffers;
   ctStaticArray<ctGPUArchitectBarrierPayload, CT_MAX_GFX_TASK_IMAGES> barriers;
   ctStaticArray<ctGPUArchitectDependencyEntry, CT_MAX_GFX_TASK_DEPENDENCIES>
     dependencies;

   void* apiData;

   inline void Reset() {
      images.Clear();
      buffers.Clear();
      barriers.Clear();
      dependencies.Clear();
   }
};

struct ctGPUArchitectDefinitionContext {
   ctGPUArchitectTaskInternal* pInternal;
};
#define OUTPUT_TASK_ID INT32_MAX

struct ctGPUArchitectDependencyRange {
   inline ctGPUArchitectDependencyRange() {
      firstSeenIdx = INT32_MAX;
      lastSeenIdx = INT32_MIN;
   }
   inline bool isValid() {
      return firstSeenIdx < lastSeenIdx;
   }
   inline bool isOverlapping(ctGPUArchitectDependencyRange target) {
      return firstSeenIdx >= target.firstSeenIdx && lastSeenIdx <= target.lastSeenIdx;
   }
   int32_t firstSeenIdx;
   int32_t lastSeenIdx;
};

struct ctGPUArchitectDependencyUseData {
   ctGPUArchitectTaskInternal* pTask;
   ctGPUArchitectDependencyEntry entry;
};

struct ctGPUArchitect {
   ctDynamicArray<ctGPUArchitectTaskInternal> tasks;

   virtual ctResults BackendStartup(ctGPUDevice* pDevice) = 0;
   virtual ctResults BackendShutdown(ctGPUDevice* pDevice) = 0;
   virtual ctResults BackendBuild(ctGPUDevice* pDevice) = 0;
   virtual ctResults BackendExecute(ctGPUDevice* pDevice) = 0;
   virtual ctResults BackendReset(ctGPUDevice* pDevice) = 0;

   /* Main User Functions */
   ctResults Validate();
   ctResults DumpGraphVis(const char* path, bool generateImage, bool showImage);
   ctResults Build(ctGPUDevice* pDevice, uint32_t width, uint32_t height);
   ctResults Execute(ctGPUDevice* pDevice);

   ctResults AddTask(ctGPUArchitectTaskInfo* pTaskInfo);
   ctResults SetOutput(ctGPUDependencyID dep, uint32_t socket);

   /* Fetching Functions */
   inline class ctGPUArchitectTaskIterator GetFinalTaskIterator();
   ctGPUArchitectDependencyUseData GetTaskLastTouchedDependency(ctGPUDependencyID id);
   ctGPUArchitectDependencyRange GetDependencyLifetime(ctGPUDependencyID id);
   ctGPUArchitectImagePayload* GetImagePayloadPtrForDependency(ctGPUDependencyID id);
   ctGPUArchitectBufferPayload* GetBufferPayloadPtrForDependency(ctGPUDependencyID id);

   ctResults ResetCache(ctGPUDevice* pDevice);
   inline bool isCacheBuilt() {
      return cacheBuilt;
   }

   /* Screen Data */
   bool isRenderable = false;
   uint32_t screenWidth;
   uint32_t screenHeight;

   inline uint32_t GetPhysicalImageWidth(int32_t flags, float input) {
      if (ctCFlagCheck(flags, CT_GPU_PAYLOAD_IMAGE_FIXED_SIZE)) {
         return (uint32_t)input;
      } else {
         return (uint32_t)(input * screenWidth);
      }
   }
   inline uint32_t GetPhysicalImageHeight(uint32_t flags, float input) {
      if (ctCFlagCheck(flags, CT_GPU_PAYLOAD_IMAGE_FIXED_SIZE)) {
         return (uint32_t)input;
      } else {
         return (uint32_t)(input * screenHeight);
      }
   }

   /* Caches */
   ctGPUDependencyID outputDependency = 0;
   bool cacheBuilt = false;
   ctDynamicArray<int32_t> cTaskFinalCutList;
   ctHashTable<bool, ctGPUDependencyID> cDependenciesSafeToReadNextTask;
   ctDynamicArray<int32_t> cFinalOrderList;
   ctHashTable<const char*, ctGPUDependencyID> cpDependencyNames;
   ctDynamicArray<ctGPUDependencyID> cpDependencyFeedbacks;
   ctHashTable<ctGPUArchitectImagePayload*, ctGPUDependencyID> cpLogicalImagePayloads;
   ctHashTable<ctGPUArchitectBufferPayload*, ctGPUDependencyID> cpLogicalBufferPayloads;
   ctHashTable<ctGPUArchitectDependencyRange, ctGPUDependencyID> cDependenciesLifespan;

   /* Build Phases */
   int32_t InitialFindNextWithDependencyMet();
   bool InitialAreDependenciesMet(const ctGPUArchitectTaskInternal& task);
   void InitialMarkDependencyWrites(const ctGPUArchitectTaskInternal& task);
   void InitialAddPayloads(ctGPUArchitectTaskInternal* pTask);
   void InitialTrackResourceLifespan(const ctGPUArchitectTaskInternal& task,
                                     int32_t orderedIdx);
};

/* Iterator over final tasks for a queue */
class ctGPUArchitectTaskIterator {
public:
   inline ctGPUArchitectTaskIterator(ctGPUArchitect* pArch) {
      pArchitect = pArch;
      orderedIdx = 0;
   }
   inline ctGPUArchitectTaskIterator(const ctGPUArchitectTaskIterator& orig) {
      pArchitect = orig.pArchitect;
      orderedIdx = orig.orderedIdx;
   }
   inline ctGPUArchitectTaskInternal& Task() const {
      return pArchitect->tasks[pArchitect->cFinalOrderList[orderedIdx]];
   }
   inline ctGPUArchitectDependencyUseData GetLastDependencyUse(ctGPUDependencyID id) {
      ctGPUArchitectTaskIterator it = (*this);
      for (it--; it; it--) {
         for (int i = 0; i < it.Task().dependencies.Count(); i++) {
            if (it.Task().dependencies[i].resourceId == id) {
               return {&it.Task(), it.Task().dependencies[i]};
            }
         }
      }
      return ctGPUArchitectDependencyUseData();
   }
   inline ctGPUArchitectDependencyUseData GetNextDependencyUse(ctGPUDependencyID id) {
      ctGPUArchitectTaskIterator it = (*this);
      for (it++; it; it++) {
         for (int i = 0; i < it.Task().dependencies.Count(); i++) {
            if (it.Task().dependencies[i].resourceId == id) {
               return {&it.Task(), it.Task().dependencies[i]};
            }
         }
      }
      return ctGPUArchitectDependencyUseData();
   }
   inline ctGPUArchitectTaskIterator operator++(int) {
      orderedIdx++;
      return *this;
   }
   inline ctGPUArchitectTaskIterator operator--(int) {
      orderedIdx--;
      return *this;
   }
   inline operator bool() const {
      return orderedIdx < pArchitect->cFinalOrderList.Count() && orderedIdx >= 0;
   }

private:
   ctGPUArchitect* pArchitect;
   int32_t orderedIdx;
};

inline class ctGPUArchitectTaskIterator ctGPUArchitect::GetFinalTaskIterator() {
   return ctGPUArchitectTaskIterator(this);
}

inline ctGPUArchitectDependencyUseData
ctGPUArchitect::GetTaskLastTouchedDependency(ctGPUDependencyID id) {
   ctGPUArchitectTaskInternal* pTask = NULL;
   ctGPUArchitectDependencyRange* pRange = cDependenciesLifespan.FindPtr(id);
   if (!pRange) { return ctGPUArchitectDependencyUseData(); }
   pTask = &tasks[cFinalOrderList[pRange->lastSeenIdx]];
   for (size_t i = 0; i < pTask->dependencies.Count(); i++) {
      if (pTask->dependencies[i].resourceId == id) {
         return {pTask, pTask->dependencies[i]};
      }
   }
   return ctGPUArchitectDependencyUseData();
}

inline ctGPUArchitectDependencyRange
ctGPUArchitect::GetDependencyLifetime(ctGPUDependencyID id) {
   if (!isCacheBuilt()) { return ctGPUArchitectDependencyRange(); }
   ctGPUArchitectDependencyRange* pRange = cDependenciesLifespan.FindPtr(id);
   if (pRange) { return *pRange; }
   return ctGPUArchitectDependencyRange();
}

inline ctGPUArchitectImagePayload*
ctGPUArchitect::GetImagePayloadPtrForDependency(ctGPUDependencyID id) {
   ctGPUArchitectImagePayload** ppPayload = cpLogicalImagePayloads.FindPtr(id);
   return ppPayload ? *ppPayload : NULL;
}

inline ctGPUArchitectBufferPayload*
ctGPUArchitect::GetBufferPayloadPtrForDependency(ctGPUDependencyID id) {
   ctGPUArchitectBufferPayload** ppPayload = cpLogicalBufferPayloads.FindPtr(id);
   return ppPayload ? *ppPayload : NULL;
}