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

#include "BufferVulkan.hpp"
#include "vulkan/vulkan.h"

CT_API ctResults ctGPUExternalBufferPoolCreate(ctGPUDevice* pDevice,
                                               ctGPUExternalBufferPool** ppPool,
                                               ctGPUExternalBufferPoolCreateInfo* pInfo) {
   ctGPUExternalBufferPool* pPool = new ctGPUExternalBufferPool(pInfo);
   *ppPool = pPool;
   return CT_SUCCESS;
}

CT_API ctResults ctGPUExternalBufferPoolDestroy(ctGPUDevice* pDevice,
                                                ctGPUExternalBufferPool* pPool) {
   while (!pPool->garbageList.isEmpty()) {
      pPool->GarbageCollect(pDevice);
   }
   delete pPool;
   return CT_SUCCESS;
}

CT_API ctResults ctGPUExternalBufferPoolGarbageCollect(ctGPUDevice* pDevice,
                                                       ctGPUExternalBufferPool* pPool) {
   pPool->GarbageCollect(pDevice);
   return CT_SUCCESS;
}

CT_API bool ctGPUExternalBufferPoolNeedsDispatch(ctGPUDevice* pDevice,
                                                 ctGPUExternalBufferPool* pPool) {
   ctSpinLockEnterCritical(pPool->uploadListLock);
   bool needsUpdate = !pPool->gpuCmdUpdateListHot.isEmpty();
   ctSpinLockExitCritical(pPool->uploadListLock);
   return needsUpdate;
}

CT_API ctResults ctGPUExternalBufferPoolDispatch(ctGPUDevice* pDevice,
                                                 ctGPUExternalBufferPool* pPool,
                                                 ctGPUCommandBuffer cmd) {
   VkCommandBuffer vkcmd = (VkCommandBuffer)cmd;
   pPool->CommitCmdHotList();
   for (size_t i = 0; i < pPool->gpuCmdUpdateList.Count(); i++) {
      pPool->gpuCmdUpdateList[i]->ExecuteCommands(vkcmd);
   }
   return CT_SUCCESS;
}

CT_API ctResults ctGPUExternalBufferCreate(ctGPUDevice* pDevice,
                                           ctGPUExternalBufferPool* pPool,
                                           ctGPUExternalBuffer** ppBuffer,
                                           ctGPUExternalBufferCreateInfo* pInfo) {
   if (pInfo->size == 0) { return CT_FAILURE_INVALID_PARAMETER; }
   ctGPUExternalBuffer* pBuffer = new ctGPUExternalBuffer();
   *ppBuffer = pBuffer;
   pBuffer->pPool = pPool;
   pBuffer->size = pInfo->size;
   pBuffer->type = pInfo->type;
   pBuffer->updateMode = pInfo->updateMode;
   pBuffer->currentFrame = 0;
   pBuffer->frameCount =
     pInfo->updateMode == CT_GPU_UPDATE_STATIC ? 1 : CT_MAX_INFLIGHT_FRAMES;

   VkBufferUsageFlags usage = 0;
   switch (pInfo->type) {
      case CT_GPU_EXTERN_BUFFER_TYPE_STORAGE:
         usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
         break;
      case CT_GPU_EXTERN_BUFFER_TYPE_INDIRECT:
         usage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
         break;
      case CT_GPU_EXTERN_BUFFER_TYPE_UNIFORM:
         usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
         break;
      case CT_GPU_EXTERN_BUFFER_TYPE_INDEX:
         usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
         break;
      case CT_GPU_EXTERN_BUFFER_TYPE_VERTEX:
         usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
         break;
      default: break;
   }
   VmaMemoryUsage memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
   if (pInfo->updateMode == CT_GPU_UPDATE_STREAM) {
      memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
   } else {
      usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
   }
   pBuffer->AllocateContents(pDevice, usage, memUsage, pInfo->debugName);
   if (pInfo->updateMode != CT_GPU_UPDATE_STREAM) { pBuffer->AquireStaging(pDevice); }
   pBuffer->GenMappings(pDevice);
   if (pInfo->data) {
      memcpy(pBuffer->mappings[pBuffer->currentFrame], pInfo->data, pInfo->size);
   }
   return CT_SUCCESS;
}

CT_API ctResults ctGPUExternalBufferRelease(ctGPUDevice* pDevice,
                                            ctGPUExternalBufferPool* pPool,
                                            ctGPUExternalBuffer* pBuffer) {
   pPool->garbageList.Append(pBuffer);
   return CT_SUCCESS;
}

CT_API ctResults ctGPUExternalBufferGetCurrentAccessor(ctGPUDevice* pDevice,
                                                       ctGPUExternalBuffer* pBuffer,
                                                       ctGPUBufferAccessor* pAccessor) {
   *pAccessor = pBuffer->contents[pBuffer->currentFrame].buffer;
   return CT_SUCCESS;
}

ctGPUExternalBufferPool::ctGPUExternalBufferPool(
  ctGPUExternalBufferPoolCreateInfo* pInfo) {
   ctSpinLockInit(uploadListLock);
}

void ctGPUExternalBufferPool::GarbageCollect(ctGPUDevice* pDevice) {
   for (size_t i = 0; i < garbageList.Count(); i++) {
      ctGPUExternalBuffer* pBuffer = garbageList[i];
      /* Release internals */
      pBuffer->FreeMappings(pDevice);
      pBuffer->ReleaseStaging(pDevice);
      pBuffer->DestroyContents(pDevice);
   }
   garbageList.Resize(incompleteGarbageList.Count());
   memcpy(garbageList.Data(),
          incompleteGarbageList.Data(),
          sizeof(incompleteGarbageList[0]) * incompleteGarbageList.Count());
   incompleteGarbageList.Clear();
}

void ctGPUExternalBufferPool::CommitCmdHotList() {
   ctSpinLockEnterCritical(uploadListLock);
   gpuCmdUpdateList.Resize(gpuCmdUpdateListHot.Count());
   memcpy(gpuCmdUpdateList.Data(),
          gpuCmdUpdateListHot.Data(),
          sizeof(ctGPUExternalBuffer*) * gpuCmdUpdateListHot.Count());
   gpuCmdUpdateListHot.Clear();
   ctSpinLockExitCritical(uploadListLock);
}

void ctGPUExternalBufferPool::AddToUpload(ctGPUExternalBuffer* pBuffer) {
   ctSpinLockEnterCritical(uploadListLock);
   gpuCmdUpdateListHot.Append(pBuffer);
   ctSpinLockExitCritical(uploadListLock);
}

void ctGPUExternalBuffer::AllocateContents(ctGPUDevice* pDevice,
                                           VkBufferUsageFlags usage,
                                           VmaMemoryUsage memUsage,
                                           const char* name) {
   for (uint32_t i = 0; i < frameCount; i++) {
      char nameData[32];
      memset(nameData, 0, 32);
      snprintf(nameData, 31, "%s: %i", name, i);
      pDevice->CreateCompleteBuffer(nameData,
                                    contents[i],
                                    usage,
                                    VMA_ALLOCATION_CREATE_STRATEGY_MIN_FRAGMENTATION_BIT,
                                    size,
                                    memUsage);
   }
}

void ctGPUExternalBuffer::DestroyContents(ctGPUDevice* pDevice) {
   for (uint32_t i = 0; i < frameCount; i++) {
      pDevice->TryDestroyCompleteBuffer(contents[i]);
   }
}

void ctGPUExternalBuffer::AquireStaging(ctGPUDevice* pDevice) {
   for (uint32_t i = 0; i < frameCount; i++) {
      pDevice->GetStagingBuffer(staging[i], size);
   }
}

void ctGPUExternalBuffer::ReleaseStaging(ctGPUDevice* pDevice) {
   for (uint32_t i = 0; i < frameCount; i++) {
      pDevice->ReleaseStagingBuffer(staging[i]);
   }
}

void ctGPUExternalBuffer::GenMappings(ctGPUDevice* pDevice) {
   if (staging[0].buffer != VK_NULL_HANDLE) {
      for (uint32_t i = 0; i < frameCount; i++) {
         vmaMapMemory(pDevice->vmaAllocator, staging[i].alloc, (void**)&mappings[i]);
      }
   } else {
      for (uint32_t i = 0; i < frameCount; i++) {
         vmaMapMemory(pDevice->vmaAllocator, contents[i].alloc, (void**)&mappings[i]);
      }
   }
}

void ctGPUExternalBuffer::FreeMappings(ctGPUDevice* pDevice) {
   if (staging[0].buffer != VK_NULL_HANDLE) {
      for (uint32_t i = 0; i < frameCount; i++) {
         vmaUnmapMemory(pDevice->vmaAllocator, staging[i].alloc);
      }
   } else {
      for (uint32_t i = 0; i < frameCount; i++) {
         vmaUnmapMemory(pDevice->vmaAllocator, contents[i].alloc);
      }
   }
}

CT_API ctResults ctGPUExternalBufferUploadMap(ctGPUDevice* pDevice,
                                              ctGPUExternalBufferPool* pPool,
                                              ctGPUExternalBuffer* pBuffer,
                                              void** ppDest) {
   ctAssert(ppDest);
   pBuffer->NextFrame();
   *ppDest = pBuffer->mappings[pBuffer->currentFrame];
   return CT_SUCCESS;
}

CT_API ctResults ctGPUExternalBufferUploadFlush(ctGPUDevice* pDevice,
                                                ctGPUExternalBufferPool* pPool,
                                                ctGPUExternalBuffer* pBuffer) {
   if (pBuffer->updateMode != CT_GPU_UPDATE_STREAM) { pPool->AddToUpload(pBuffer); }
   return CT_SUCCESS;
}

void ctGPUExternalBuffer::ExecuteCommands(VkCommandBuffer cmd) {
   if (staging[0].buffer == VK_NULL_HANDLE) { return; }
   VkBufferCopy copy = {};
   copy.size = size;
   vkCmdCopyBuffer(
     cmd, staging[currentFrame].buffer, contents[currentFrame].buffer, 1, &copy);
}