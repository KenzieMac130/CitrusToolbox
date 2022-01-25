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
#include "ArchitectGraphBuilder.hpp"
#include "system/System.h"

CT_API ctResults ctGPUArchitectDumpGraphVis(ctGPUArchitect* pArchitect,
                                            const char* path,
                                            bool generateImage,
                                            bool showImage) {
   ZoneScoped;
   return pArchitect->DumpGraphVis(path, generateImage, showImage);
}

CT_API ctResults ctGPUArchitectBuild(ctGPUDevice* pDevice,
                                     ctGPUArchitect* pArchitect,
                                     ctGPUArchitectBuildInfo* pInfo) {
   ZoneScoped;
   return pArchitect->Build(pDevice, pInfo->width, pInfo->height);
}

CT_API ctResults ctGPUArchitectReset(ctGPUDevice* pDevice, ctGPUArchitect* pArchitect) {
   ZoneScoped;
   return pArchitect->ResetCache(pDevice);
}

CT_API ctResults ctGPUArchitectAddTask(ctGPUDevice* pDevice,
                                       ctGPUArchitect* pArchitect,
                                       ctGPUArchitectTaskInfo* pTaskInfo) {
   ZoneScoped;
   return pArchitect->AddTask(pTaskInfo);
}

CT_API ctResults ctGPUArchitectExecute(ctGPUDevice* pDevice, ctGPUArchitect* pArchitect) {
   ZoneScoped;
   return pArchitect->Execute(pDevice);
}

CT_API ctResults ctGPUArchitectSetOutput(ctGPUDevice* pDevice,
                                         ctGPUArchitect* pArchitect,
                                         ctGPUDependencyID dependency,
                                         uint32_t socket) {
   ZoneScoped;
   return pArchitect->SetOutput(dependency, socket);
}

ctResults ctGPUArchitect::AddTask(ctGPUArchitectTaskInfo* pTaskInfo) {
   ZoneScoped;
   if (isCacheBuilt()) { return CT_FAILURE_DEPENDENCY_NOT_MET; }
   ctGPUArchitectTaskInternal task = ctGPUArchitectTaskInternal();
   task.category = pTaskInfo->category;
   task.fpDefinition = pTaskInfo->fpDefinition;
   task.fpExecution = pTaskInfo->fpExecution;
   task.pUserData = pTaskInfo->pUserData;
   task.debugIdx = (int32_t)tasks.Count();
   strncpy(task.debugName, pTaskInfo->name, 31);
   tasks.Append(task);
   cacheBuilt = false;
   return CT_SUCCESS;
}

ctResults ctGPUArchitect::SetOutput(ctGPUDependencyID dep, uint32_t socket) {
   outputDependency = dep;
   return CT_SUCCESS;
}

ctResults ctGPUArchitect::Validate() {
   ZoneScoped;
   /* Has output and tasks */
   if (tasks.isEmpty()) {
      ctDebugError("ctGPUArchitect: Graph has no tasks!");
      return CT_FAILURE_INVALID_PARAMETER;
   }
   /* Check undeclared/mistyped resource use */
   for (int i = 0; i < tasks.Count(); i++) {
      ctGPUArchitectTaskInternal& task = tasks[i];
      for (int j = 0; j < task.dependencies.Count(); j++) {
         if (!cDependenciesSafeToReadNextTask.FindPtr(task.dependencies[j].resourceId)) {
            ctDebugError("ctGPUArchitect: Resource of ID %u is undeclared!",
                         task.dependencies[i].resourceId);
            return CT_FAILURE_INVALID_PARAMETER;
         }
      }
   }
   return CT_SUCCESS;
}

void GetWireStyle(ctGPUArchitectDependencyEntry entry,
                  const char** ppStr,
                  int* pColorIdx) {
   switch (entry.type) {
      case CT_GPU_ARCH_BARRIER:
         *ppStr = "B";
         *pColorIdx = 9;
         break;
      case CT_GPU_ARCH_COLOR_TARGET:
         switch (entry.slot) {
            case 0: *ppStr = "C0"; break;
            case 1: *ppStr = "C1"; break;
            case 2: *ppStr = "C2"; break;
            case 3: *ppStr = "C3"; break;
            case 4: *ppStr = "C4"; break;
            case 5: *ppStr = "C5"; break;
            case 6: *ppStr = "C6"; break;
            case 7: *ppStr = "C7"; break;
            default: *ppStr = "C#"; break;
         }
         *pColorIdx = 8;
         break;
      case CT_GPU_ARCH_DEPTH_TARGET:
         *ppStr = "D";
         *pColorIdx = 2;
         break;
      case CT_GPU_ARCH_TEXTURE:
         *ppStr = "T";
         *pColorIdx = 4;
         break;
      case CT_GPU_ARCH_STORAGE_BUFFER:
         *ppStr = "S";
         *pColorIdx = 6;
         break;
      default: break;
   }
}

void MakeDotWire(ctFile& file,
                 ctGPUArchitectDependencyEntry curDep,
                 ctGPUArchitectDependencyEntry lastDep,
                 int32_t lastTaskIdx,
                 int32_t curentTaskIdx,
                 const char* depName) {
   /* Create wire markup */
   const char* curTypeStr = "?";
   int lineBeginColor = 0;
   int lineEndColor = 0;
   const char* lastTypeStr = "?";
   const char* curAccess = "?";
   GetWireStyle(curDep, &curTypeStr, &lineEndColor);
   GetWireStyle(lastDep, &lastTypeStr, &lineBeginColor);
   switch (curDep.access) {
      case CT_GPU_ACCESS_READ: curAccess = "R"; break;
      case CT_GPU_ACCESS_WRITE: curAccess = "W"; break;
      case CT_GPU_ACCESS_READ_WRITE: curAccess = "RW"; break;
      default: break;
   }
   const char* lastAccess = "?";
   switch (lastDep.access) {
      case CT_GPU_ACCESS_READ: lastAccess = "R"; break;
      case CT_GPU_ACCESS_WRITE: lastAccess = "W"; break;
      case CT_GPU_ACCESS_READ_WRITE: lastAccess = "RW"; break;
      default: break;
   }

   file.Printf("%i -> %i"
               "[label=\"%s\" fontsize=\"10\"]"
               "[taillabel=\"%s:%s\"]"
               "[headlabel=\"%s:%s\"]"
               "[color=\"%i:%i;0.5\"]\n",
               lastTaskIdx,
               curentTaskIdx,
               depName,
               lastTypeStr,
               lastAccess,
               curTypeStr,
               curAccess,
               lineBeginColor,
               lineEndColor);
}

ctResults
ctGPUArchitect::DumpGraphVis(const char* path, bool generateImage, bool showImage) {
   ZoneScoped;
   CT_RETURN_FAIL(Validate());
   if (!isCacheBuilt()) { return CT_FAILURE_DEPENDENCY_NOT_MET; }
   ctStringUtf8 dotPath = path;
   dotPath += ".dot";
   ctFile file = ctFile(dotPath.CStr(), CT_FILE_OPEN_WRITE_TEXT);
   if (!file.isOpen()) { return CT_FAILURE_INACCESSIBLE; }

   /* Write header */
   file.Printf("digraph rendergraph {\n"
               "node [colorscheme=paired9]\n"
               "edge [colorscheme=paired9]\n");

   /* Create feedback output */
   if (!cpDependencyFeedbacks.isEmpty()) {
      file.Printf("%i [label=\"Feedback\"]", INT_MIN);
   }

   /* Iterat all tasks */
   for (auto it = GetFinalTaskIterator(); it; it++) {
      ctGPUArchitectTaskInternal& task = it.Task();

      /* Create Node Markup */
      int taskColorIdx = 2;
      const char* queueText = "?";
      switch (task.category) {
         case CT_GPU_TASK_RASTER:
            queueText = "Raster";
            taskColorIdx = 9;
            break;
         case CT_GPU_TASK_COMPUTE:
            queueText = "Compute";
            taskColorIdx = 7;
            break;
         case CT_GPU_TASK_TRANSFER:
            queueText = "Transfer";
            taskColorIdx = 2;
            break;
         case CT_GPU_TASK_RAYTRACE:
            queueText = "Raytrace";
            taskColorIdx = 6;
            break;
         default: break;
      }
      file.Printf("%i "
                  "[label=\"%s:%s\"]"
                  "[fillcolor=\"%i\"]"
                  "[shape=box style=filled]\n",
                  task.debugIdx,
                  queueText,
                  task.debugName,
                  taskColorIdx);

      /* Iterate all dependencies */
      for (int32_t i = 0; i < task.dependencies.Count(); i++) {
         /* Get current and last use */
         ctGPUArchitectDependencyEntry curDep = task.dependencies[i];
         ctGPUArchitectDependencyUseData lastUse =
           it.GetLastDependencyUse(curDep.resourceId);
         ctGPUArchitectDependencyEntry& lastDep = lastUse.entry;
         int curentTaskIdx = INT_MAX;
         int lastTaskIdx = INT_MIN;
         /* Last task exists */
         if (lastUse.pTask) {
            lastTaskIdx = lastUse.pTask->debugIdx;
         }
         /* Last task does not exist */
         else {
            /* Create wire to feedback as needed */
            if (cpDependencyFeedbacks.FindIndex(curDep.resourceId) < 0) { continue; }
            lastUse.entry = curDep;
            lastUse.entry.access = 0;
         }
         const char** ppDepName = cpDependencyNames.FindPtr(curDep.resourceId);
         const char* depName = "UNKNOWN";
         if (ppDepName) { depName = *ppDepName; }
         curentTaskIdx = task.debugIdx;

         MakeDotWire(file, curDep, lastDep, lastTaskIdx, curentTaskIdx, depName);
      }
   }

   /* Write output node */
   if (outputDependency) {
      file.Printf("%i [label=\"Output\"]", INT_MAX);
      ctGPUArchitectDependencyEntry outDepEntry = ctGPUArchitectDependencyEntry();
      outDepEntry.access = CT_GPU_ACCESS_READ;
      outDepEntry.resourceId = outputDependency;
      outDepEntry.type = CT_GPU_ARCH_TEXTURE;
      ctGPUArchitectDependencyUseData lastTouched =
        GetTaskLastTouchedDependency(outputDependency);
      if (lastTouched.pTask) {
         const char** ppDepName = cpDependencyNames.FindPtr(outputDependency);
         const char* depName = "UNKNOWN";
         if (ppDepName) { depName = *ppDepName; }
         MakeDotWire(file,
                     outDepEntry,
                     lastTouched.entry,
                     lastTouched.pTask->debugIdx,
                     INT_MAX,
                     depName);
      }
   }
   file.Printf("}");
   file.Close();

   /* Build the image using graphvis via command line */
   if (generateImage) {
      ctStringUtf8 imagePath = path;
      imagePath += ".png";
      const char* argArray[] = {"-Tpng", dotPath.CStr(), "-o", imagePath.CStr()};
      ctSystemExecuteCommand("dot", ctCStaticArrayLen(argArray), argArray);
      if (showImage) { ctSystemShowFileToDeveloper(imagePath.CStr()); }
   }
   return CT_SUCCESS;
}

ctResults ctGPUArchitect::Build(ctGPUDevice* pDevice, uint32_t width, uint32_t height) {
   ZoneScoped;
   isRenderable = false;
   screenWidth = width;
   screenHeight = height;

   /* Define all nodes */
   if (isCacheBuilt()) { ResetCache(pDevice); }
   for (size_t i = 0; i < tasks.Count(); i++) {
      ctGPUArchitectTaskInternal& task = tasks[i];
      ctGPUArchitectDefinitionContext ctx = {&task};
      if (!task.fpDefinition) { return CT_FAILURE_INVALID_PARAMETER; }
      if (task.fpDefinition(&ctx, tasks[i].pUserData) == CT_SUCCESS) {
         cTaskFinalCutList.Append((int32_t)i);
         for (size_t j = 0; j < task.dependencies.Count(); j++) {
            cDependenciesLifespan.InsertOrReplace(task.dependencies[j].resourceId,
                                                  ctGPUArchitectDependencyRange());
         }
         for (size_t j = 0; j < task.images.Count(); j++) {
            cDependenciesSafeToReadNextTask.Insert(
              task.images[j].identifier,
              ctCFlagCheck(task.images[j].flags, CT_GPU_PAYLOAD_FEEDBACK));
         }
         for (size_t j = 0; j < task.buffers.Count(); j++) {
            cDependenciesSafeToReadNextTask.Insert(
              task.buffers[j].identifier,
              ctCFlagCheck(task.buffers[j].flags, CT_GPU_PAYLOAD_FEEDBACK));
         }
         for (size_t j = 0; j < task.barriers.Count(); j++) {
            cDependenciesSafeToReadNextTask.Insert(task.barriers[j].identifier, false);
         }
      }
   }
   CT_RETURN_FAIL_CLEAN(Validate(), ResetCache(pDevice));
   while (!cTaskFinalCutList.isEmpty()) {
      /* find with all dependencies safe to read */
      int32_t nextAvailibleTaskIdx = InitialFindNextWithDependencyMet();
      ctAssert((nextAvailibleTaskIdx >= 0)); /* Invalid dependency graph! */
      if (nextAvailibleTaskIdx < 0) {
         ctDebugError("ctGPUArchitect: Cyclic or invalid dependency!");
         ResetCache(pDevice);
         return CT_FAILURE_INVALID_PARAMETER;
      }
      /* make writes safe to read */
      InitialMarkDependencyWrites(tasks[nextAvailibleTaskIdx]);
      InitialAddPayloads(&tasks[nextAvailibleTaskIdx]);
      InitialTrackResourceLifespan(tasks[nextAvailibleTaskIdx],
                                   (int)cFinalOrderList.Count());
      /* add to initial order list */
      cFinalOrderList.Append(nextAvailibleTaskIdx);
      cTaskFinalCutList.Remove(nextAvailibleTaskIdx);
   }
   cacheBuilt = true;
   CT_RETURN_FAIL(BackendBuild(pDevice));
   isRenderable = true;
   return CT_SUCCESS;
}

ctResults ctGPUArchitect::Execute(ctGPUDevice* pDevice) {
   ZoneScoped;
   return BackendExecute(pDevice);
}

ctResults ctGPUArchitect::ResetCache(ctGPUDevice* pDevice) {
   ZoneScoped;
   cacheBuilt = false;
   cTaskFinalCutList.Clear();
   cDependenciesSafeToReadNextTask.Clear();
   cFinalOrderList.Clear();
   cpLogicalImagePayloads.Clear();
   cpLogicalBufferPayloads.Clear();
   cDependenciesLifespan.Clear();
   cpDependencyNames.Clear();
   cpDependencyFeedbacks.Clear();
   return BackendReset(pDevice);
}

int32_t ctGPUArchitect::InitialFindNextWithDependencyMet() {
   ZoneScoped;
   for (int32_t i = 0; i < (int32_t)cTaskFinalCutList.Count(); i++) {
      ctGPUArchitectTaskInternal& task = tasks[cTaskFinalCutList[i]];
      if (InitialAreDependenciesMet(task)) { return cTaskFinalCutList[i]; }
   }
   return -1;
}

bool ctGPUArchitect::InitialAreDependenciesMet(const ctGPUArchitectTaskInternal& task) {
   ZoneScoped;
   for (int32_t i = 0; i < task.dependencies.Count(); i++) {
      const ctGPUArchitectDependencyEntry& dependency = task.dependencies[i];
      if (ctCFlagCheck(dependency.access, CT_GPU_ACCESS_READ)) {
         bool* pSafeRead = cDependenciesSafeToReadNextTask.FindPtr(dependency.resourceId);
         ctAssert(pSafeRead); /* validation will have caught this */
         if (!*pSafeRead) { return false; }
      }
   }
   return true;
}

void ctGPUArchitect::InitialMarkDependencyWrites(const ctGPUArchitectTaskInternal& task) {
   ZoneScoped;
   for (int32_t i = 0; i < task.dependencies.Count(); i++) {
      const ctGPUArchitectDependencyEntry& dependency = task.dependencies[i];
      if (ctCFlagCheck(dependency.access, CT_GPU_ACCESS_WRITE)) {
         bool* pSafeRead = cDependenciesSafeToReadNextTask.FindPtr(dependency.resourceId);
         ctAssert(pSafeRead); /* validation will have caught this */
         *pSafeRead = true;
      }
   }
}

void ctGPUArchitect::InitialAddPayloads(ctGPUArchitectTaskInternal* pTask) {
   ZoneScoped;
   for (int32_t i = 0; i < pTask->images.Count(); i++) {
      cpLogicalImagePayloads.Insert(pTask->images[i].identifier, &pTask->images[i]);
      cpDependencyNames.Insert(pTask->images[i].identifier, pTask->images[i].debugName);
      if (ctCFlagCheck(pTask->images[i].flags, CT_GPU_PAYLOAD_FEEDBACK)) {
         cpDependencyFeedbacks.Append(pTask->images[i].identifier);
      }
   }
   for (int32_t i = 0; i < pTask->buffers.Count(); i++) {
      cpLogicalBufferPayloads.Insert(pTask->images[i].identifier, &pTask->buffers[i]);
      cpDependencyNames.Insert(pTask->buffers[i].identifier, pTask->buffers[i].debugName);
      if (ctCFlagCheck(pTask->buffers[i].flags, CT_GPU_PAYLOAD_FEEDBACK)) {
         cpDependencyFeedbacks.Append(pTask->buffers[i].identifier);
      }
   }
   for (int32_t i = 0; i < pTask->barriers.Count(); i++) {
      cpDependencyNames.Insert(pTask->barriers[i].identifier,
                               pTask->barriers[i].debugName);
   }
}

void ctGPUArchitect::InitialTrackResourceLifespan(const ctGPUArchitectTaskInternal& task,
                                                  int32_t orderedIdx) {
   ZoneScoped;
   for (int32_t i = 0; i < task.dependencies.Count(); i++) {
      ctGPUArchitectDependencyRange* pRange =
        cDependenciesLifespan.FindPtr(task.dependencies[i].resourceId);
      ctAssert(pRange); /* validation will have caught this */
      if (pRange->firstSeenIdx > orderedIdx) { pRange->firstSeenIdx = orderedIdx; }
      if (pRange->lastSeenIdx < orderedIdx) { pRange->lastSeenIdx = orderedIdx; }
   }
}

CT_API ctResults ctGPUTaskDeclareDepthTarget(ctGPUArchitectDefinitionContext* pCtx,
                                             ctGPUDependencyID id,
                                             ctGPUArchitectImagePayloadDesc* pDesc) {
   ZoneScoped;
   ctAssert(pCtx);
   ctAssert(pCtx->pInternal);
   return pCtx->pInternal->images.Append(ctGPUArchitectImagePayload(*pDesc, id));
}

CT_API ctResults ctGPUTaskDeclareColorTarget(ctGPUArchitectDefinitionContext* pCtx,
                                             ctGPUDependencyID id,
                                             ctGPUArchitectImagePayloadDesc* pDesc) {
   ZoneScoped;
   ctAssert(pCtx);
   ctAssert(pCtx->pInternal);
   return pCtx->pInternal->images.Append(ctGPUArchitectImagePayload(*pDesc, id));
}

CT_API ctResults ctGPUTaskDeclareStorageBuffer(ctGPUArchitectDefinitionContext* pCtx,
                                               ctGPUDependencyID id,
                                               ctGPUArchitectBufferPayloadDesc* pDesc) {
   ZoneScoped;
   ctAssert(pCtx);
   ctAssert(pCtx->pInternal);
   return pCtx->pInternal->buffers.Append(ctGPUArchitectBufferPayload(*pDesc, id));
}

CT_API ctResults ctGPUTaskCreateBarrier(ctGPUArchitectDefinitionContext* pCtx,
                                        ctGPUDependencyID id,
                                        const char* debugName,
                                        int32_t flags) {
   ZoneScoped;
   ctAssert(pCtx);
   ctAssert(pCtx->pInternal);
   return pCtx->pInternal->barriers.Append(
     ctGPUArchitectBarrierPayload(debugName, flags, id));
}

CT_API ctResults ctGPUTaskUseDepthTarget(ctGPUArchitectDefinitionContext* pCtx,
                                         ctGPUDependencyID id,
                                         ctGPUArchitectResourceAccess access) {
   ZoneScoped;
   ctAssert(pCtx);
   ctAssert(pCtx->pInternal);
   pCtx->pInternal->dependencies.Append({id, access, CT_GPU_ARCH_DEPTH_TARGET, 0});
   return CT_SUCCESS;
}

CT_API ctResults ctGPUTaskUseColorTarget(ctGPUArchitectDefinitionContext* pCtx,
                                         ctGPUDependencyID id,
                                         ctGPUArchitectResourceAccess access,
                                         uint32_t slot) {
   ZoneScoped;
   ctAssert(pCtx);
   ctAssert(pCtx->pInternal);
   pCtx->pInternal->dependencies.Append(
     {id, access, CT_GPU_ARCH_COLOR_TARGET, (uint8_t)slot});
   return CT_SUCCESS;
}

CT_API ctResults ctGPUTaskUseTexture(ctGPUArchitectDefinitionContext* pCtx,
                                     ctGPUDependencyID id) {
   ZoneScoped;
   ctAssert(pCtx);
   ctAssert(pCtx->pInternal);
   pCtx->pInternal->dependencies.Append({id, CT_GPU_ACCESS_READ, CT_GPU_ARCH_TEXTURE, 0});
   return CT_SUCCESS;
}

CT_API ctResults ctGPUTaskUseStorageBuffer(ctGPUArchitectDefinitionContext* pCtx,
                                           ctGPUDependencyID id,
                                           ctGPUArchitectResourceAccess access) {
   ZoneScoped;
   ctAssert(pCtx);
   ctAssert(pCtx->pInternal);
   pCtx->pInternal->dependencies.Append({id, access, CT_GPU_ARCH_STORAGE_BUFFER, 0});
   return CT_SUCCESS;
}

CT_API ctResults ctGPUTaskWaitBarrier(ctGPUArchitectDefinitionContext* pCtx,
                                      ctGPUDependencyID id) {
   ZoneScoped;
   ctAssert(pCtx);
   ctAssert(pCtx->pInternal);
   pCtx->pInternal->dependencies.Append({id, CT_GPU_ACCESS_READ, CT_GPU_ARCH_BARRIER, 0});
   return CT_SUCCESS;
}

CT_API ctResults ctGPUTaskSignalBarrier(ctGPUArchitectDefinitionContext* pCtx,
                                        ctGPUDependencyID id) {
   ZoneScoped;
   ctAssert(pCtx);
   ctAssert(pCtx->pInternal);
   pCtx->pInternal->dependencies.Append(
     {id, CT_GPU_ACCESS_WRITE, CT_GPU_ARCH_BARRIER, 0});
   return CT_SUCCESS;
}

ctGPUArchitectImagePayload::ctGPUArchitectImagePayload() {
   memset(this, 0, sizeof(*this));
}

ctGPUArchitectBufferPayload::ctGPUArchitectBufferPayload() {
   memset(this, 0, sizeof(*this));
}

ctGPUArchitectBarrierPayload::ctGPUArchitectBarrierPayload() {
   memset(this, 0, sizeof(*this));
}

ctGPUArchitectImagePayload::ctGPUArchitectImagePayload(
  const ctGPUArchitectImagePayloadDesc& desc, ctGPUDependencyID id) :
    ctGPUArchitectImagePayload() {
   strncpy(debugName, desc.debugName, 31);
   flags = desc.flags;
   height = desc.height;
   width = desc.width;
   layers = desc.layers;
   miplevels = desc.miplevels;
   format = desc.format;
   identifier = id;
   pClearDesc = desc.pClearDesc;
   apiData = NULL;
}

ctGPUArchitectBufferPayload::ctGPUArchitectBufferPayload(
  const ctGPUArchitectBufferPayloadDesc& desc, ctGPUDependencyID id) :
    ctGPUArchitectBufferPayload() {
   strncpy(debugName, desc.debugName, 31);
   flags = desc.flags;
   size = desc.size;
   identifier = id;
   apiData = NULL;
}

ctGPUArchitectBarrierPayload::ctGPUArchitectBarrierPayload(const char* inDebugName,
                                                           int32_t inFlags,
                                                           ctGPUDependencyID id) {
   strncpy(debugName, inDebugName, 31);
   flags = inFlags;
   identifier = id;
   apiData = NULL;
}