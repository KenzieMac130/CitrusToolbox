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

#include "TextureVulkan.hpp"
#include "gpu/shared/ExternalLoading.hpp"

#include "vulkan/vulkan.h"

size_t GetPhysicalSizeOfChunk(TinyImageFormat universalFormat,
                              uint32_t w,
                              uint32_t h,
                              uint32_t d) {
   if (!w || !h) { return 0; }
   uint32_t blockCount = 0;
   const size_t maxBitsPerBlock = TinyImageFormat_BitSizeOfBlock(universalFormat);
   const size_t maxBytesPerBlock = (maxBitsPerBlock / 8) > 0 ? maxBitsPerBlock / 8 : 1;
   uint32_t blockWidth = TinyImageFormat_WidthOfBlock(universalFormat);
   uint32_t blockHeight = TinyImageFormat_HeightOfBlock(universalFormat);
   if (!blockWidth) { blockWidth = 1; }
   if (!blockHeight) { blockHeight = 1; }
   w = (w + (w % blockWidth)) / blockWidth;
   h = (h + (h % blockHeight)) / blockHeight;
   blockCount = w * h;
   if (!d) { return blockCount * maxBytesPerBlock; }
   uint32_t blockDepth = TinyImageFormat_DepthOfBlock(universalFormat);
   if (!blockDepth) { blockDepth = 1; }
   d = (d + (d % blockDepth)) / blockDepth;
   blockCount = blockCount * d;
   return blockCount * maxBytesPerBlock;
}

inline size_t AlignOffsetIntoMapping(size_t base) {
   return base;
}

void ctGPUTextureGenerateFnQuickMemcpy(uint8_t* dest,
                                       ctGPUExternalGenerateContext* pCtx,
                                       void* userData) {
   ctAssert(pCtx->currentLayer == 0 && pCtx->currentMipLevel == 0);
   ctAssert(TinyImageFormat_IsHomogenous(pCtx->format));
   const size_t bytesPerPixel = (size_t)TinyImageFormat_BitSizeOfBlock(pCtx->format) / 8;
   const size_t byteCount = bytesPerPixel * pCtx->width * pCtx->height * pCtx->depth;
   memcpy(dest, userData, byteCount);
}

CT_API ctResults
ctGPUExternalTexturePoolCreate(ctGPUDevice* pDevice,
                               ctGPUExternalTexturePool** ppPool,
                               ctGPUExternalTexturePoolCreateInfo* pInfo) {
   ctGPUExternalTexturePool* pPool = new ctGPUExternalTexturePool(pInfo);
   *ppPool = pPool;
   return CT_SUCCESS;
}

CT_API ctResults ctGPUExternalTexturePoolDestroy(ctGPUDevice* pDevice,
                                                 ctGPUExternalTexturePool* pPool) {
   while (!pPool->garbageList.isEmpty()) {
      pPool->GarbageCollect(pDevice);
   }
   delete pPool;
   return CT_SUCCESS;
}

CT_API ctResults ctGPUExternalTexturePoolGarbageCollect(ctGPUDevice* pDevice,
                                                        ctGPUExternalTexturePool* pPool) {
   pPool->GarbageCollect(pDevice);
   return CT_SUCCESS;
}

CT_API bool ctGPUExternalTexturePoolNeedsDispatch(ctGPUDevice* pDevice,
                                                  ctGPUExternalTexturePool* pPool) {
   ctSpinLockEnterCritical(pPool->uploadListLock);
   bool needsUpdate = !pPool->gpuCmdUpdateListHot.isEmpty();
   ctSpinLockExitCritical(pPool->uploadListLock);
   return needsUpdate;
}

CT_API ctResults ctGPUExternalTexturePoolDispatch(ctGPUDevice* pDevice,
                                                  ctGPUExternalTexturePool* pPool,
                                                  ctGPUCommandBuffer cmd) {
   VkCommandBuffer vkcmd = (VkCommandBuffer)cmd;
   pPool->CommitHotList();
   pPool->MakeLayoutTransitions(
     vkcmd, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
   for (size_t i = 0; i < pPool->gpuCmdUpdateList.Count(); i++) {
      pPool->gpuCmdUpdateList[i]->ExecuteCommands(vkcmd);
   }
   pPool->MakeLayoutTransitions(vkcmd,
                                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
   return CT_SUCCESS;
}

ctResults ctGPUAsyncTextureGenerateWork(ctGPUExternalTexture* pTexture) {
   pTexture->GenerateContents();
   return CT_SUCCESS;
}

CT_API ctResults
ctGPUExternalTextureCreateFunc(ctGPUDevice* pDevice,
                               ctGPUExternalTexturePool* pPool,
                               ctGPUExternalTexture** ppTexture,
                               ctGPUExternalTextureCreateFuncInfo* pInfo) {
   VkFormat nativeFormat = (VkFormat)TinyImageFormat_ToVkFormat(pInfo->format);
   if (pInfo->updateMode == CT_GPU_UPDATE_STREAM || pInfo->height == 0 ||
       pInfo->width == 0 || pInfo->depth == 0 || pInfo->mips == 0 ||
       nativeFormat == VK_FORMAT_UNDEFINED) {
      return CT_FAILURE_INVALID_PARAMETER;
   }
   ctGPUExternalTexture* pTexture = new ctGPUExternalTexture();
   *ppTexture = pTexture;
   pTexture->pPool = pPool;
   pTexture->universalFormat = pInfo->format;
   pTexture->nativeFormat = nativeFormat;
   pTexture->height = pInfo->height;
   pTexture->width = pInfo->width;
   pTexture->depth = pInfo->depth;
   pTexture->mips = pInfo->mips;
   pTexture->type = pInfo->type;
   pTexture->updateMode = pInfo->updateMode;
   pTexture->currentFrame = 0;
   pTexture->frameCount =
     pInfo->updateMode == CT_GPU_UPDATE_STATIC ? 1 : CT_MAX_INFLIGHT_FRAMES;
   pTexture->generationFunction = pInfo->generationFunction;
   pTexture->userData = pInfo->userData;

   /* Get image and view type */
   VkImageType imageType = VK_IMAGE_TYPE_2D;
   VkImageViewType imageViewType = VK_IMAGE_VIEW_TYPE_2D;
   switch (pInfo->type) {
      case CT_GPU_EXTERN_TEXTURE_TYPE_1D:
         imageType = VK_IMAGE_TYPE_1D;
         imageViewType =
           pInfo->depth == 1 ? VK_IMAGE_VIEW_TYPE_1D : VK_IMAGE_VIEW_TYPE_1D_ARRAY;
         break;
      case CT_GPU_EXTERN_TEXTURE_TYPE_2D:
         imageType = VK_IMAGE_TYPE_2D;
         imageViewType =
           pInfo->depth == 1 ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY;
         break;
      case CT_GPU_EXTERN_TEXTURE_TYPE_3D:
         imageType = VK_IMAGE_TYPE_3D;
         imageViewType = VK_IMAGE_VIEW_TYPE_3D;
         break;
      case CT_GPU_EXTERN_TEXTURE_TYPE_CUBE:
         imageType = VK_IMAGE_TYPE_2D;
         imageViewType =
           pInfo->depth == 1 ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
         break;
      default: break;
   }
   pTexture->AllocateContents(
     pDevice, imageType, imageViewType, VMA_MEMORY_USAGE_GPU_ONLY, pInfo->debugName);
   vkGetImageMemoryRequirements(
     pDevice->vkDevice, pTexture->contents[0].image, &pTexture->memreq);
   pTexture->AquireStaging(pDevice);
   pTexture->GenMappings(pDevice);

   /* Add to async list */
   if (pInfo->async && pPool->fpAsyncScheduler) {
      pTexture->wantsAsync = true;
      pPool->fpAsyncScheduler(
        (ctGPUAsyncWorkFn)ctGPUAsyncTextureGenerateWork, pTexture, pPool->pAsyncUserData);
   } else {
      pTexture->GenerateContents();
   }

   /* Generate bindless as needed */
   pTexture->GenBindless(pDevice);
   return CT_SUCCESS;
}

CT_API ctResults ctGPUExternalTextureCreateLoad(ctGPUDevice* pDevice,
                                                ctGPUExternalTexturePool* pPool,
                                                ctGPUExternalTexture** ppTexture,
                                                const char* debugName,
                                                int32_t desiredBinding,
                                                ctGPUExternalTexture* pPlaceholder,
                                                ctGPUExternalTextureType type,
                                                ctGPUAssetIdentifier* identifier) {
   return ctGPUExternalTextureCreateLoadCPU(pDevice,
                                            pPool,
                                            ppTexture,
                                            debugName,
                                            desiredBinding,
                                            pPlaceholder,
                                            type,
                                            identifier);
}

CT_API ctResults ctGPUExternalTextureRebuild(ctGPUDevice* pDevice,
                                             ctGPUExternalTexturePool* pPool,
                                             size_t textureCount,
                                             ctGPUExternalTexture** ppTextures) {
   for (size_t i = 0; i < textureCount; i++) {
      ppTextures[i]->MakeReady(false);
      ppTextures[i]->NextFrame();
      if (ppTextures[i]->wantsAsync && pPool->fpAsyncScheduler) {
         pPool->fpAsyncScheduler((ctGPUAsyncWorkFn)ctGPUAsyncTextureGenerateWork,
                                 ppTextures[i],
                                 pPool->pAsyncUserData);
      } else {
         ppTextures[i]->GenerateContents();
      }
   }
   return CT_SUCCESS;
}

CT_API ctResults ctGPUExternalTextureRelease(ctGPUDevice* pDevice,
                                             ctGPUExternalTexturePool* pPool,
                                             ctGPUExternalTexture* pTexture) {
   pPool->garbageList.Append(pTexture);
   return CT_SUCCESS;
}

CT_API bool ctGPUExternalTextureIsReady(ctGPUDevice* pDevice,
                                        ctGPUExternalTexturePool* pPool,
                                        ctGPUExternalTexture* pTexture) {
   return pTexture->isReady();
}

CT_API ctResults ctGPUExternalTextureGetCurrentAccessor(ctGPUDevice* pDevice,
                                                        ctGPUExternalTexturePool* pPool,
                                                        ctGPUExternalTexture* pTexture,
                                                        ctGPUImageAccessor* pAccessor) {
   *pAccessor = &pTexture->contents[pTexture->currentFrame];
   return CT_SUCCESS;
}

CT_API ctResults ctGPUExternalTextureGetBindlessIndex(ctGPUDevice* pDevice,
                                                      ctGPUExternalTexturePool* pPool,
                                                      ctGPUExternalTexture* pTexture,
                                                      int32_t* pIndex) {
   *pIndex = pTexture->bindlessIndices[pTexture->currentFrame];
   return CT_SUCCESS;
}

ctGPUExternalTexturePool::ctGPUExternalTexturePool(
  ctGPUExternalTexturePoolCreateInfo* pInfo) {
   fpAsyncScheduler = pInfo->fpAsyncScheduler;
   pAsyncUserData = pInfo->pAsyncUserData;
   ctSpinLockInit(uploadListLock);
}

void ctGPUExternalTexturePool::GarbageCollect(ctGPUDevice* pDevice) {
   for (size_t i = 0; i < garbageList.Count(); i++) {
      ctGPUExternalTexture* pTexture = garbageList[i];
      /* Don't release if it is still in use */
      if (!pTexture->isReady()) { incompleteGarbageList.Append(pTexture); }

      /* Release internals */
      pTexture->FreeMappings(pDevice);
      pTexture->FreeBindless(pDevice);
      pTexture->ReleaseStaging(pDevice);
      pTexture->DestroyContents(pDevice);
   }
   garbageList.Resize(incompleteGarbageList.Count());
   memcpy(garbageList.Data(),
          incompleteGarbageList.Data(),
          sizeof(incompleteGarbageList[0]) * incompleteGarbageList.Count());
   incompleteGarbageList.Clear();
}

void ctGPUExternalTexturePool::CommitHotList() {
   ctSpinLockEnterCritical(uploadListLock);
   gpuCmdUpdateListHot.Resize(gpuCmdUpdateListHot.Count());
   memcpy(gpuCmdUpdateList.Data(),
          gpuCmdUpdateListHot.Data(),
          sizeof(ctGPUExternalBuffer*) * gpuCmdUpdateListHot.Count());
   gpuCmdUpdateListHot.Clear();
   ctSpinLockExitCritical(uploadListLock);
}

void ctGPUExternalTexturePool::AddToUpload(ctGPUExternalTexture* pTexture) {
   ctSpinLockEnterCritical(uploadListLock);
   gpuCmdUpdateListHot.Append(pTexture);
   ctSpinLockExitCritical(uploadListLock);
}

void ctGPUExternalTexturePool::MakeLayoutTransitions(VkCommandBuffer cmd,
                                                     VkImageLayout src,
                                                     VkImageLayout dst) {
   for (size_t i = 0; i < gpuCmdUpdateList.Count(); i++) {
      VkImageMemoryBarrier barrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
      ctGPUExternalTexture& texture = *gpuCmdUpdateList[i];
      barrier.srcAccessMask = 0;
      barrier.dstAccessMask = 0;
      barrier.image = texture.contents[texture.currentFrame].image;
      barrier.oldLayout = src;
      barrier.newLayout = dst;
      barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      barrier.subresourceRange.baseArrayLayer = 0;
      barrier.subresourceRange.baseMipLevel = 0;
      barrier.subresourceRange.layerCount =
        texture.type == CT_GPU_EXTERN_TEXTURE_TYPE_3D ? 1 : texture.depth;
      barrier.subresourceRange.levelCount = texture.mips;
      barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      memBarrierScratch.Append(barrier);
   }
   vkCmdPipelineBarrier(cmd,
                        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                        VK_DEPENDENCY_BY_REGION_BIT,
                        0,
                        NULL,
                        0,
                        NULL,
                        (uint32_t)memBarrierScratch.Count(),
                        memBarrierScratch.Data());
   memBarrierScratch.Clear();
}

void ctGPUExternalTexture::AllocateContents(ctGPUDevice* pDevice,
                                            VkImageType imageType,
                                            VkImageViewType imageViewType,
                                            VmaMemoryUsage memUsage,
                                            const char* name) {
   for (uint32_t i = 0; i < frameCount; i++) {
      char nameData[32];
      memset(nameData, 0, 32);
      snprintf(nameData, 31, "%s: %i", name, i);
      pDevice->CreateCompleteImage(nameData,
                                   contents[i],
                                   nativeFormat,
                                   VK_IMAGE_USAGE_SAMPLED_BIT |
                                     VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                   VMA_ALLOCATION_CREATE_STRATEGY_MIN_FRAGMENTATION_BIT,
                                   VK_IMAGE_ASPECT_COLOR_BIT, /* color only for now */
                                   width,
                                   height,
                                   imageType == VK_IMAGE_TYPE_3D ? depth : 1,
                                   mips,
                                   imageType == VK_IMAGE_TYPE_3D ? 1 : depth);
   }
}

void ctGPUExternalTexture::DestroyContents(ctGPUDevice* pDevice) {
   for (uint32_t i = 0; i < frameCount; i++) {
      pDevice->TryDestroyCompleteImage(contents[i]);
   }
}
void ctGPUExternalTexture::AquireStaging(ctGPUDevice* pDevice) {
   for (uint32_t i = 0; i < frameCount; i++) {
      pDevice->GetStagingBuffer(staging[i], memreq.size);
   }
}

void ctGPUExternalTexture::ReleaseStaging(ctGPUDevice* pDevice) {
   for (uint32_t i = 0; i < frameCount; i++) {
      pDevice->ReleaseStagingBuffer(staging[i]);
   }
}

void ctGPUExternalTexture::GenMappings(ctGPUDevice* pDevice) {
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

void ctGPUExternalTexture::FreeMappings(ctGPUDevice* pDevice) {
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

void ctGPUExternalTexture::GenBindless(ctGPUDevice* pDevice) {
   if (type == CT_GPU_EXTERN_BUFFER_TYPE_STORAGE) {
      for (uint32_t i = 0; i < frameCount; i++) {
         pDevice->ExposeBindlessSampledImage(bindlessIndices[i], contents[i].view);
         // todo: handle inflight frames and preferred bindings
      }
   } else {
      InvalidateBindless();
   }
}

void ctGPUExternalTexture::InvalidateBindless() {
   for (uint32_t i = 0; i < frameCount; i++) {
      bindlessIndices[i] = -1;
   }
}

void ctGPUExternalTexture::FreeBindless(ctGPUDevice* pDevice) {
   if (type == CT_GPU_EXTERN_BUFFER_TYPE_STORAGE) {
      for (uint32_t i = 0; i < frameCount; i++) {
         if (bindlessIndices[i] >= 0) {
            pDevice->ReleaseBindlessSampledImage(bindlessIndices[i]);
         }
      }
   }
}

void ctGPUExternalTexture::GenSlices() {
   size_t currentSeekIntoChunk = 0;
   ctGPUExternalGenerateContext ctx;
   ctx.format = universalFormat;
   ctx.depth = 1;
   uint32_t mipWidth = width;
   uint32_t mipHeight = height;
   for (uint32_t i = 0; i < depth; i++) {
      ctx.currentLayer = i;
      for (uint32_t j = 0; j < mips; j++) {
         mipWidth = mipWidth > 1 ? mipWidth / 2 : 1;
         mipHeight = mipHeight > 1 ? mipHeight / 2 : 1;

         ctx.currentMipLevel = j;
         ctx.width = mipWidth;
         ctx.height = mipHeight;
         generationFunction(
           mappings[currentFrame] + currentSeekIntoChunk, &ctx, userData);
         currentSeekIntoChunk = AlignOffsetIntoMapping(
           currentSeekIntoChunk +
           GetPhysicalSizeOfChunk(universalFormat, mipWidth, mipHeight, 0));
         VkBufferImageCopy copy = {};
         copy.bufferOffset = 0;
         copy.bufferRowLength = 0;
         copy.bufferImageHeight = 0;
         copy.imageSubresource.baseArrayLayer = i;
         copy.imageSubresource.layerCount = 1;
         copy.imageSubresource.mipLevel = j;
         copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
         copy.imageExtent.width = mipWidth;
         copy.imageExtent.height = mipHeight;
         copy.imageExtent.depth = 1;
         copyCommands.Append(copy);
      }
   }
}

void ctGPUExternalTexture::GenVolume() {
   size_t currentSeekIntoChunk = 0;
   ctGPUExternalGenerateContext ctx;
   ctx.format = universalFormat;
   ctx.currentLayer = 0;
   uint32_t mipWidth = width;
   uint32_t mipHeight = height;
   uint32_t mipDepth = depth;
   for (uint32_t i = 0; i < mips; i++) {
      mipWidth = mipWidth > 1 ? mipWidth / 2 : 1;
      mipHeight = mipHeight > 1 ? mipHeight / 2 : 1;
      mipDepth = mipDepth > 1 ? mipDepth / 2 : 1;

      ctx.currentMipLevel = i;
      ctx.width = mipWidth;
      ctx.height = mipHeight;
      ctx.depth = mipDepth;
      generationFunction(mappings[currentFrame] + currentSeekIntoChunk, &ctx, userData);
      currentSeekIntoChunk = AlignOffsetIntoMapping(
        currentSeekIntoChunk +
        GetPhysicalSizeOfChunk(universalFormat, mipWidth, mipHeight, 0));
      VkBufferImageCopy copy = {};
      copy.bufferOffset = 0;
      copy.bufferRowLength = 0;
      copy.bufferImageHeight = 0;
      copy.imageSubresource.baseArrayLayer = 0;
      copy.imageSubresource.layerCount = 1;
      copy.imageSubresource.mipLevel = i;
      copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      copy.imageExtent.width = mipWidth;
      copy.imageExtent.height = mipHeight;
      copy.imageExtent.depth = mipDepth;
      copyCommands.Append(copy);
   }
}

void ctGPUExternalTexture::GenerateContents() {
   if (type == CT_GPU_EXTERN_TEXTURE_TYPE_3D) {
      GenVolume();
   } else {
      GenSlices();
   }
   MakeReady(true);
   pPool->AddToUpload(this);
}

void ctGPUExternalTexture::ExecuteCommands(VkCommandBuffer cmd) {
   if (staging[0].buffer == VK_NULL_HANDLE) { return; }
   vkCmdCopyBufferToImage(cmd,
                          staging[currentFrame].buffer,
                          contents[currentFrame].image,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          (uint32_t)copyCommands.Count(),
                          copyCommands.Data());
   copyCommands.Clear();
}