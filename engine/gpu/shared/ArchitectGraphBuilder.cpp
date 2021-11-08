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

CT_API ctResults ctGPUArchitectStartup(ctGPUDevice* pDevice,
                                       ctGPUArchitect** ppArchitect,
                                       ctGPUArchitectCreateInfo* pCreateInfo) {
   *ppArchitect = new ctGPUArchitect();
   ctGPUArchitect& architect = **ppArchitect;
   architect.pBackend = ctGPUNewArchitectBackend(pDevice);
   return architect.pBackend->Startup(pDevice);
}

CT_API ctResults ctGPUArchitectShutdown(ctGPUDevice* pDevice,
                                        ctGPUArchitect* pArchitect) {
   return pArchitect->pBackend->Shutdown(pDevice);
}

CT_API ctResults ctGPUArchitectDumpGraphVis(ctGPUArchitect* pArchitect,
                                            const char* path,
                                            bool generateImage,
                                            bool showImage) {
   return pArchitect->DumpGraphVis(path, generateImage, showImage);
}

CT_API ctResults ctGPUArchitectBuild(ctGPUDevice* pDevice, ctGPUArchitect* pArchitect) {
   return pArchitect->Build(pDevice);
}

CT_API ctResults ctGPUArchitectReset(ctGPUDevice* pDevice, ctGPUArchitect* pArchitect) {
   return pArchitect->ResetCache(pDevice);
}

CT_API ctResults ctGPUArchitectAddTask(ctGPUDevice* pDevice,
                                       ctGPUArchitect* pArchitect,
                                       ctGPUArchitectTaskInfo* pTaskInfo) {
   if (pArchitect->isCacheBuilt) { return CT_FAILURE_DEPENDENCY_NOT_MET; }
   ctGPUArchitectTaskInternal task = ctGPUArchitectTaskInternal();
   task.category = pTaskInfo->category;
   task.fpDefinition = pTaskInfo->fpDefinition;
   task.fpExecution = pTaskInfo->fpExecution;
   task.pUserData = pTaskInfo->pUserData;
   strncpy(task.debugName, pTaskInfo->name, 31);
   pArchitect->tasks.Append(task);
   pArchitect->isCacheBuilt = false;
   return CT_SUCCESS;
}

ctResults ctGPUArchitect::Validate() {
   /* Has output and tasks */
   if (tasks.isEmpty()) {
      ctDebugError("ctGPUArchitect: Graph has no tasks!");
      return CT_FAILURE_INVALID_PARAMETER;
   }
   /* Check undeclared/mistyped resource use */
   for (int i = 0; i < tasks.Count(); i++) {
      ctGPUArchitectTaskInternal& task = tasks[i];
      for (int j = 0; j < task.dependencies.Count(); j++) {
         if (!cDependenciesSafeToRead.FindPtr(task.dependencies[j].resourceId)) {
            ctDebugError("ctGPUArchitect: Resource of ID %u is undeclared!",
                         task.dependencies[i].resourceId);
            return CT_FAILURE_INVALID_PARAMETER;
         }
      }
   }
   return CT_SUCCESS;
}

ctResults
ctGPUArchitect::DumpGraphVis(const char* path, bool generateImage, bool showImage) {
   CT_RETURN_FAIL(Validate());
   if (!isCacheBuilt) { return CT_FAILURE_DEPENDENCY_NOT_MET; }
   ctStringUtf8 dotPath = path;
   dotPath += ".dot";
   ctFile file = ctFile(path, CT_FILE_OPEN_WRITE_TEXT);
   if (!file.isOpen()) { return CT_FAILURE_INACCESSIBLE; }
   file.Printf("digraph rendergraph {\n");
   // todo...
   file.Printf("}");
   file.Close();

   if (generateImage) {
      ctStringUtf8 imagePath = path;
      imagePath += ".png";
      const char* argArray[] = {"-Tpng", dotPath.CStr(), "-o", imagePath.CStr()};
      ctSystemExecuteCommand("dot", ctCStaticArrayLen(argArray), argArray);
      if (showImage) { ctSystemShowFileToDeveloper(imagePath.CStr()); }
   }
   return CT_SUCCESS;
}

ctResults ctGPUArchitect::Build(ctGPUDevice* pDevice) {
   /* Define all nodes */
   if (isCacheBuilt) { return CT_FAILURE_DEPENDENCY_NOT_MET; }
   for (size_t i = 0; i < tasks.Count(); i++) {
      ctGPUArchitectTaskInternal& task = tasks[i];
      ctGPUArchitectDefinitionContext ctx = {&task};
      if (!task.fpDefinition) { return CT_FAILURE_INVALID_PARAMETER; }
      if (task.fpDefinition(&ctx, tasks[i].pUserData) == CT_SUCCESS) {
         cTaskFinalCutList.Append((int32_t)i);
         for (size_t j = 0; j < task.images.Count(); j++) {
            cDependenciesSafeToRead.Insert(
              task.images[j].identifier,
              ctCFlagCheck(task.images[j].flags, CT_GPU_PAYLOAD_FEEDBACK));
         }
         for (size_t j = 0; j < task.buffers.Count(); j++) {
            cDependenciesSafeToRead.Insert(
              task.buffers[j].identifier,
              ctCFlagCheck(task.buffers[j].flags, CT_GPU_PAYLOAD_FEEDBACK));
         }
         for (size_t j = 0; j < task.barriers.Count(); j++) {
            cDependenciesSafeToRead.Insert(task.barriers[j].identifier, false);
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
      /* add to initial order list */
      cFinalOrderList.Append(nextAvailibleTaskIdx);
      /* make writes safe to read */
      InitialMarkDependencyWrites(tasks[nextAvailibleTaskIdx]);
      cTaskFinalCutList.Remove(nextAvailibleTaskIdx);
   }
   pBackend->OptimizeOrder(pDevice, this);
   CT_RETURN_FAIL_CLEAN(Validate(), ResetCache(pDevice));

   pBackend->BuildInternal(pDevice, this);
   isCacheBuilt = true;
   return CT_SUCCESS;
}

ctResults ctGPUArchitect::ResetCache(ctGPUDevice* pDevice) {
   isCacheBuilt = false;
   cTaskFinalCutList.Clear();
   cDependenciesSafeToRead.Clear();
   cFinalOrderList.Clear();
   return pBackend->ResetInternal(pDevice, this);
}

int32_t ctGPUArchitect::InitialFindNextWithDependencyMet() {
   for (int32_t i = 0; i < (int32_t)cTaskFinalCutList.Count(); i++) {
      ctGPUArchitectTaskInternal& task = tasks[cTaskFinalCutList[i]];
      if (InitialAreDependenciesMet(task)) { return cTaskFinalCutList[i]; }
   }
   return -1;
}

bool ctGPUArchitect::InitialAreDependenciesMet(const ctGPUArchitectTaskInternal& task) {
   for (int32_t i = 0; i < task.dependencies.Count(); i++) {
      const ctGPUArchitectDependencyEntry& dependency = task.dependencies[i];
      if (ctCFlagCheck(dependency.access, CT_GPU_ACCESS_READ)) {
         bool* pSafeRead = cDependenciesSafeToRead.FindPtr(dependency.resourceId);
         ctAssert(pSafeRead); /* validation will have caught this */
         if (!*pSafeRead) { return false; }
      }
   }
   return true;
}

void ctGPUArchitect::InitialMarkDependencyWrites(const ctGPUArchitectTaskInternal& task) {
   for (int32_t i = 0; i < task.dependencies.Count(); i++) {
      const ctGPUArchitectDependencyEntry& dependency = task.dependencies[i];
      if (ctCFlagCheck(dependency.access, CT_GPU_ACCESS_WRITE)) {
         bool* pSafeRead = cDependenciesSafeToRead.FindPtr(dependency.resourceId);
         ctAssert(pSafeRead); /* validation will have caught this */
         *pSafeRead = true;
      }
   }
}

CT_API ctResults ctGPUTaskDeclareDepthTarget(ctGPUArchitectDefinitionContext* pCtx,
                                             ctGPUDependencyID id,
                                             ctGPUArchitectImagePayloadDesc* pDesc) {
   ctAssert(pCtx);
   ctAssert(pCtx->pInternal);
   return pCtx->pInternal->images.Append(ctGPUArchitectImagePayload(*pDesc, id));
}

CT_API ctResults ctGPUTaskDeclareColorTarget(ctGPUArchitectDefinitionContext* pCtx,
                                             ctGPUDependencyID id,
                                             ctGPUArchitectImagePayloadDesc* pDesc) {
   ctAssert(pCtx);
   ctAssert(pCtx->pInternal);
   return pCtx->pInternal->images.Append(ctGPUArchitectImagePayload(*pDesc, id));
}

CT_API ctResults ctGPUTaskDeclareStorageBuffer(ctGPUArchitectDefinitionContext* pCtx,
                                               ctGPUDependencyID id,
                                               ctGPUArchitectBufferPayloadDesc* pDesc) {
   ctAssert(pCtx);
   ctAssert(pCtx->pInternal);
   return pCtx->pInternal->buffers.Append(ctGPUArchitectBufferPayload(*pDesc, id));
}

CT_API ctResults ctGPUTaskCreateBarrier(ctGPUArchitectDefinitionContext* pCtx,
                                        ctGPUDependencyID id,
                                        const char* debugName,
                                        int32_t flags) {
   ctAssert(pCtx);
   ctAssert(pCtx->pInternal);
   return pCtx->pInternal->barriers.Append(
     ctGPUArchitectBarrierPayload(debugName, flags, id));
}

CT_API ctResults ctGPUTaskUseDepthTarget(ctGPUArchitectDefinitionContext* pCtx,
                                         ctGPUDependencyID id,
                                         ctGPUArchitectResourceAccess access) {
   ctAssert(pCtx);
   ctAssert(pCtx->pInternal);
   pCtx->pInternal->dependencies.Append({id, access, CT_GPU_ARCH_DEPTH_TARGET});
   return CT_SUCCESS;
}

CT_API ctResults ctGPUTaskUseColorTarget(ctGPUArchitectDefinitionContext* pCtx,
                                         ctGPUDependencyID id,
                                         ctGPUArchitectResourceAccess access) {
   ctAssert(pCtx);
   ctAssert(pCtx->pInternal);
   pCtx->pInternal->dependencies.Append({id, access, CT_GPU_ARCH_COLOR_TARGET});
   return CT_SUCCESS;
}

CT_API ctResults ctGPUTaskUseTexture(ctGPUArchitectDefinitionContext* pCtx,
                                     ctGPUDependencyID id) {
   ctAssert(pCtx);
   ctAssert(pCtx->pInternal);
   pCtx->pInternal->dependencies.Append({id, CT_GPU_ACCESS_READ, CT_GPU_ARCH_TEXTURE});
   return CT_SUCCESS;
}

CT_API ctResults ctGPUTaskUseStorageBuffer(ctGPUArchitectDefinitionContext* pCtx,
                                           ctGPUDependencyID id,
                                           ctGPUArchitectResourceAccess access) {
   ctAssert(pCtx);
   ctAssert(pCtx->pInternal);
   pCtx->pInternal->dependencies.Append({id, access, CT_GPU_ARCH_STORAGE_BUFFER});
   return CT_SUCCESS;
}

CT_API ctResults ctGPUTaskWaitBarrier(ctGPUArchitectDefinitionContext* pCtx,
                                      ctGPUDependencyID id) {
   ctAssert(pCtx);
   ctAssert(pCtx->pInternal);
   pCtx->pInternal->dependencies.Append({id, CT_GPU_ACCESS_READ, CT_GPU_ARCH_BARRIER});
   return CT_SUCCESS;
}

CT_API ctResults ctGPUTaskSignalBarrier(ctGPUArchitectDefinitionContext* pCtx,
                                        ctGPUDependencyID id) {
   ctAssert(pCtx);
   ctAssert(pCtx->pInternal);
   pCtx->pInternal->dependencies.Append({id, CT_GPU_ACCESS_WRITE, CT_GPU_ARCH_BARRIER});
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
   format = desc.format;
   identifier = id;
}

ctGPUArchitectBufferPayload::ctGPUArchitectBufferPayload(
  const ctGPUArchitectBufferPayloadDesc& desc, ctGPUDependencyID id) :
    ctGPUArchitectBufferPayload() {
   strncpy(debugName, desc.debugName, 31);
   flags = desc.flags;
   size = desc.size;
   identifier = id;
}

ctGPUArchitectBarrierPayload::ctGPUArchitectBarrierPayload(const char* inDebugName,
                                                           int32_t inFlags,
                                                           ctGPUDependencyID id) {
   strncpy(debugName, inDebugName, 31);
   flags = inFlags;
   identifier = id;
}