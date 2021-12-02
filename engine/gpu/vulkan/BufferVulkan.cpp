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

#include "BufferVulkan.hpp"

#include "vulkan/vulkan.h"

CT_API ctResults ctGPUExternalBufferPoolCreate(ctGPUDevice* pDevice,
                                               ctGPUExternalBufferPool** ppPool,
                                               ctGPUExternalBufferPoolCreateInfo* pInfo) {
   ctGPUExternalBufferPool* pPool = new ctGPUExternalBufferPool(pInfo);
   *ppPool = pPool;
   return CT_SUCCESS;
}

CT_API ctResults ctGPUExternalBufferPoolFlush(ctGPUDevice* pDevice,
                                              ctGPUExternalBufferPool* pPool) {
   return pPool->Flush(pDevice, pDevice->GetNextSafeReleaseConveyor());
}

CT_API ctResults ctGPUExternalBufferPoolDestroy(ctGPUDevice* pDevice,
                                                ctGPUExternalBufferPool* pPool) {
   pPool->Shutdown(pDevice);
   delete pPool;
   return CT_SUCCESS;
}

CT_API ctResults ctGPUExternalBufferCreate(struct ctGPUDevice* pDevice,
                                           ctGPUExternalBufferPool* pPool,
                                           size_t count,
                                           ctGPUExternalBuffer** ppBuffers,
                                           ctGPUExternalBufferCreateInfo* pInfos) {
   ctAssert(pDevice);
   ctAssert(pPool);
   ctAssert(ppBuffers);
   ctAssert(pInfos);
   for (size_t i = 0; i < count; i++) {
      ctAssert(ppBuffers[i]);
      ppBuffers[i] = new ctGPUExternalBuffer();
      ppBuffers[i]->createInfo = pInfos[i];
      pPool->Add(ppBuffers[i]);
   }
   return CT_SUCCESS;
}

CT_API ctResults ctGPUExternalBufferRelease(struct ctGPUDevice* pDevice,
                                            ctGPUExternalBufferPool* pPool,
                                            size_t count,
                                            ctGPUExternalBuffer** ppBuffers) {
   const int32_t currentRelease = pDevice->conveyorFrame;
   for (size_t i = 0; i < count; i++) {
      pPool->Release(ppBuffers[i], currentRelease);
   }
   return CT_SUCCESS;
}

CT_API ctResults ctGPUExternalBufferRequestUpdate(struct ctGPUDevice* pDevice,
                                                  ctGPUExternalBufferPool* pPool,
                                                  size_t count,
                                                  ctGPUExternalBuffer** ppBuffers) {
   for (size_t i = 0; i < count; i++) {
      pPool->Update(ppBuffers[i]);
   }
   return CT_SUCCESS;
}

CT_API ctResults ctGPUExternalBufferGetCurrentAccessor(struct ctGPUDevice* pDevice,
                                                       ctGPUExternalBufferPool* pPool,
                                                       ctGPUExternalBuffer* pBuffer,
                                                       ctGPUBufferAccessor* pAccessor) {
   *pAccessor = &pBuffer->data[pBuffer->currentDynamicFrame];
   return CT_SUCCESS;
}

CT_API ctResults ctGPUExternalBufferGetBindlessIndex(ctGPUDevice* pDevice,
                                                     ctGPUExternalBufferPool* pPool,
                                                     ctGPUExternalBuffer* pBuffer,
                                                     int32_t* pIndex) {
   return CT_API ctResults();
}

ctGPUExternalBufferPool::ctGPUExternalBufferPool(
  ctGPUExternalBufferPoolCreateInfo* pInfo) {
}

ctResults ctGPUExternalBuffer::Create(ctGPUDevice* pDevice) {
   if (createInfo.source == CT_GPU_EXTERN_SOURCE_GENERATE) {
      bufferSize = createInfo.generate.size;
   } else {
      file = pDevice->OpenAssetFile(&createInfo.load.assetIdentifier);
      bufferSize = file.GetFileSize();
   }
   VkBufferUsageFlags usage = 0;
   switch (createInfo.type) {
      case CT_GPU_EXTERN_BUFFER_TYPE_STORAGE:
         usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
         break;
      case CT_GPU_EXTERN_BUFFER_TYPE_UNIFORM:
         usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
         break;
      case CT_GPU_EXTERN_BUFFER_TYPE_INDIRECT:
         usage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
         break;
      default: break;
   }
   if (createInfo.updateMode != CT_GPU_UPDATE_STREAM) {
      usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
   }
   VmaMemoryUsage memUseage = VMA_MEMORY_USAGE_GPU_ONLY;
   switch (createInfo.updateMode) {
      case CT_GPU_UPDATE_STREAM: memUseage = VMA_MEMORY_USAGE_CPU_TO_GPU;
      default: break;
   }
   /* Create buffer for all necessary slots */
   const int32_t frameCount =
     createInfo.updateMode == CT_GPU_UPDATE_STATIC ? 1 : CT_MAX_INFLIGHT_FRAMES;
   for (int32_t i = 0; i < frameCount; i++) {
      /* Create backing buffer */
      pDevice->CreateCompleteBuffer(data[i],
                                    usage,
                                    VMA_ALLOCATION_CREATE_STRATEGY_MIN_FRAGMENTATION_BIT,
                                    bufferSize,
                                    memUseage);
      /* Persistently map memory */
      if (createInfo.updateMode == CT_GPU_UPDATE_STREAM) {
         vmaMapMemory(pDevice->vmaAllocator, data[i].alloc, (void**)&mappings[i]);
      }
   }
   PopulateContents(pDevice);
   /* Cleanup */
   if (createInfo.source == CT_GPU_EXTERN_SOURCE_LOAD) { file.Close(); }
   return CT_SUCCESS;
}

ctResults ctGPUExternalBuffer::Update(ctGPUDevice* pDevice) {
   if (createInfo.updateMode == CT_GPU_UPDATE_STATIC) { return CT_FAILURE_NOT_UPDATABLE; }
   currentDynamicFrame = (currentDynamicFrame + 1) % CT_MAX_INFLIGHT_FRAMES;
   PopulateContents(pDevice);
   return CT_SUCCESS;
}

ctResults ctGPUExternalBuffer::Free(ctGPUDevice* pDevice) {
   const int32_t frameCount =
     createInfo.updateMode == CT_GPU_UPDATE_STATIC ? 1 : CT_MAX_INFLIGHT_FRAMES;
   for (int32_t i = 0; i < frameCount; i++) {
      if (createInfo.updateMode == CT_GPU_UPDATE_STREAM) {
         vmaUnmapMemory(pDevice->vmaAllocator, data[i].alloc);
      }
      pDevice->TryDestroyCompleteBuffer(data[i]);
   }
   return CT_SUCCESS;
}

void ctGPUExternalBuffer::PopulateContents(ctGPUDevice* pDevice) {
   if (createInfo.updateMode != CT_GPU_UPDATE_STREAM) {
      ctVkCompleteBuffer stagingBuffer;
      size_t stagingOffset = 0;
      uint8_t* dest = NULL;
      pDevice->GetStagingBuffer(stagingBuffer, stagingOffset, dest, bufferSize);
      ctAssert(dest);
      createInfo.generate.fpGenerationFunction(
        dest, bufferSize, createInfo.generate.pUserData);
      vmaFlushAllocation(pDevice->vmaAllocator, stagingBuffer.alloc, 0, VK_WHOLE_SIZE);
      VkBufferCopy copyInfo;
      copyInfo.size = bufferSize;
      copyInfo.srcOffset = stagingOffset;
      copyInfo.dstOffset = 0;
      vkCmdCopyBuffer(pDevice->transferCommands.cmd[pDevice->currentFrame],
                      stagingBuffer.buffer,
                      data[currentDynamicFrame].buffer,
                      1,
                      &copyInfo);
      // todo: tell device to fill in descriptors
   } else {
      createInfo.generate.fpGenerationFunction(
        mappings[currentDynamicFrame], bufferSize, createInfo.generate.pUserData);
      vmaFlushAllocation(
        pDevice->vmaAllocator, data[currentDynamicFrame].alloc, 0, VK_WHOLE_SIZE);
   }
}