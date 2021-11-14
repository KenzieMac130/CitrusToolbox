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

#include "ArchitectVulkan.hpp"

/* Stubs for structures in vulkan still under development :P */
// Provided by VK_KHR_dynamic_rendering
typedef enum VkRenderingFlagBitsKHR {
   VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT_KHR = 0x00000001,
   VK_RENDERING_SUSPENDING_BIT_KHR = 0x00000002,
   VK_RENDERING_RESUMING_BIT_KHR = 0x00000004,
} VkRenderingFlagBitsKHR;
typedef VkFlags VkRenderingFlagsKHR;

typedef struct VkRenderingAttachmentInfoKHR {
   VkStructureType sType;
   const void* pNext;
   VkImageView imageView;
   VkImageLayout imageLayout;
   VkResolveModeFlagBits resolveMode;
   VkImageView resolveImageView;
   VkImageLayout resolveImageLayout;
   VkAttachmentLoadOp loadOp;
   VkAttachmentStoreOp storeOp;
   VkClearValue clearValue;
} VkRenderingAttachmentInfoKHR;

// Provided by VK_KHR_dynamic_rendering
typedef struct VkRenderingInfoKHR {
   VkStructureType sType;
   const void* pNext;
   VkRenderingFlagsKHR flags;
   VkRect2D renderArea;
   uint32_t layerCount;
   uint32_t viewMask;
   uint32_t colorAttachmentCount;
   const VkRenderingAttachmentInfoKHR* pColorAttachments;
   const VkRenderingAttachmentInfoKHR* pDepthAttachment;
   const VkRenderingAttachmentInfoKHR* pStencilAttachment;
} VkRenderingInfoKHR;

// Provided by VK_KHR_dynamic_rendering
void vkCmdBeginRenderingKHR(VkCommandBuffer commandBuffer,
                            const VkRenderingInfoKHR* pRenderingInfo) {
   /* stub... */
}
void vkCmdEndRenderingKHR(VkCommandBuffer commandBuffer) {
}
/* --------------------------------------------------------- */

ctGPUArchitectBackend* ctGPUNewArchitectBackend(ctGPUDevice* pDevice) {
   return new ctGPUArchitectBackendVulkan();
}

ctResults ctGPUArchitectBackendVulkan::Startup(struct ctGPUDevice* pDeviceIn,
                                               struct ctGPUArchitect* pArchitectIn) {
   ZoneScoped;
   pDevice = pDeviceIn;
   pArchitect = pArchitectIn;
   return CT_SUCCESS;
}

ctResults ctGPUArchitectBackendVulkan::Shutdown() {
   ZoneScoped;
   DereferenceAll();
   GarbageCollect();
   return CT_SUCCESS;
}

ctResults ctGPUArchitectBackendVulkan::BuildInternal() {
   ZoneScoped;
   screenWidth = pDevice->mainScreenResources.extent.width;
   screenHeight = pDevice->mainScreenResources.extent.height;

   isRenderable = false;
   DereferenceAll();
   for (auto it = pArchitect->cpLogicalBufferPayloads.GetIterator(); it; it++) {
      MapToBuffer(it.Value());
   }
   for (auto it = pArchitect->cpLogicalImagePayloads.GetIterator(); it; it++) {
      MapToImage(it.Value());
   }
   GarbageCollect();
   isRenderable = true;
   return CT_SUCCESS;
}

struct ExecutionInternalData {
   ctGPUArchitectBackendVulkan* pBackend;
   ctGPUArchitectTaskInternal* pTask;
};
ctResults ctGPUArchitectBackendVulkan::ExecuteInternal() {
   ZoneScoped;
   if (!isRenderable) { return CT_FAILURE_DEPENDENCY_NOT_MET; }
   for (auto current = pArchitect->GetFinalTaskIterator(); current; current++) {
      ctGPUArchitectTaskInternal& task = current.Task();
      if (task.category != CT_GPU_TASK_RASTER) { continue; }

      /* Build render info */
      VkRenderingAttachmentInfoKHR colorInfos[8];
      VkRenderingAttachmentInfoKHR depthStencilInfo;
      bool hasStencil = false;
      bool hasDepth = false;
      VkRenderingInfoKHR renderInfo = {VK_STRUCTURE_TYPE_MAX_ENUM /* todo: struct type*/};
      renderInfo.flags = 0;
      renderInfo.renderArea = {0};
      renderInfo.layerCount = 0;
      renderInfo.pColorAttachments = colorInfos;
      renderInfo.viewMask = 0;

      for (int i = 0; i < task.dependencies.Count(); i++) {
         ctGPUArchitectDependencyEntry& dependency = task.dependencies[i];
         if (dependency.type == CT_GPU_ARCH_COLOR_TARGET ||
             dependency.type == CT_GPU_ARCH_DEPTH_TARGET) {
            ctGPUArchitectImagePayload* pLPayload =
              pArchitect->GetImagePayloadPtrForDependency(dependency.resourceId);
            ctAssert(pLPayload);
            PhysicalImage* image = (PhysicalImage*)pLPayload->apiData;
            ctAssert(image);

            uint32_t width = image->widthPixels;
            uint32_t height = image->heightPixels;
            uint32_t layers = image->layers;
            if (renderInfo.layerCount == 0) { renderInfo.layerCount = layers; }
            if (renderInfo.renderArea.extent.width == 0) {
               renderInfo.renderArea.extent.width = width;
               renderInfo.renderArea.extent.height = height;
            }
            ctAssert(renderInfo.layerCount == layers);
            ctAssert(renderInfo.renderArea.extent.width == width);
            ctAssert(renderInfo.renderArea.extent.height == height);

            if (dependency.type == CT_GPU_ARCH_COLOR_TARGET) {
               if (renderInfo.colorAttachmentCount <= dependency.slot) {
                  renderInfo.colorAttachmentCount = dependency.slot + 1;
                  colorInfos[dependency.slot] =
                    RenderingAttachmentInfoFromImage(image, dependency);
               }
            }
            // clang-format off
            if (dependency.type == CT_GPU_ARCH_DEPTH_TARGET) {
               depthStencilInfo = RenderingAttachmentInfoFromImage(image, dependency);
               if (image->hasStencil) { renderInfo.pStencilAttachment = &depthStencilInfo; }
               if (image->hasDepth) { renderInfo.pDepthAttachment = &depthStencilInfo; }
            }
            // clang-format on
         }
      }
      VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
      vkCmdBeginRenderingKHR(commandBuffer, &renderInfo);
      ctGPUArchitectExecutionContext ctx = {};
      ExecutionInternalData internalData = {};
      internalData.pBackend = this;
      internalData.pTask = &task;
      ctx.raster.width = renderInfo.renderArea.extent.width;
      ctx.raster.height = renderInfo.renderArea.extent.height;
      ctx.raster.layerCount = renderInfo.layerCount;
      ctx.cmd = commandBuffer;
      ctx._internalData = &internalData;
      if (task.fpExecution) { task.fpExecution(&ctx, task.pUserData); }
      vkCmdEndRenderingKHR(commandBuffer);
   }
   return CT_SUCCESS;
}

ctResults ctGPUArchitectBackendVulkan::ResetInternal() {
   ZoneScoped;
   DereferenceAll();
   GarbageCollect();
   pPhysicalBuffers.Clear();
   pPhysicalImages.Clear();
   isRenderable = false;
   return CT_SUCCESS;
}

ctResults
ctGPUArchitectBackendVulkan::MapToBuffer(ctGPUArchitectBufferPayload* pPayloadData) {
   ZoneScoped;
   if (ctCFlagCheck(pPayloadData->flags, CT_GPU_PAYLOAD_FEEDBACK)) {
      return CreateNewBuffer(pPayloadData);
   }
   for (size_t i = 0; i < pPhysicalBuffers.Count(); i++) {
      if (isBufferAliasable(*pPhysicalBuffers[i], *pPayloadData)) {
         pPhysicalBuffers[i]->refCount++;
         pPhysicalBuffers[i]->mappings.Append(pPayloadData->identifier);
         pPayloadData->apiData = pPhysicalBuffers[i];
         return CT_SUCCESS;
      }
   }
   return CreateNewBuffer(pPayloadData);
}

ctResults
ctGPUArchitectBackendVulkan::MapToImage(ctGPUArchitectImagePayload* pPayloadData) {
   ZoneScoped;
   if (ctCFlagCheck(pPayloadData->flags, CT_GPU_PAYLOAD_FEEDBACK)) {
      return CreateNewImage(pPayloadData);
   }
   for (size_t i = 0; i < pPhysicalImages.Count(); i++) {
      if (isImageAliasable(*pPhysicalImages[i], *pPayloadData)) {
         pPhysicalImages[i]->refCount++;
         pPhysicalImages[i]->mappings.Append(pPayloadData->identifier);
         pPayloadData->apiData = pPhysicalImages[i];
         return CT_SUCCESS;
      }
   }
   return CreateNewImage(pPayloadData);
}

ctResults ctGPUArchitectBackendVulkan::GarbageCollect() {
   ZoneScoped;
   for (size_t i = 0; i < pPhysicalBuffers.Count(); i++) {
      if (pPhysicalBuffers[i]->refCount <= 0) {
         pDevice->TryDestroyCompleteBuffer(pPhysicalBuffers[i]->data);
         delete pPhysicalBuffers[i];
         pPhysicalBuffers[i] = NULL;
      }
   }
   for (size_t i = 0; i < pPhysicalImages.Count(); i++) {
      if (pPhysicalImages[i]->refCount <= 0) {
         pDevice->TryDestroyCompleteImage(pPhysicalImages[i]->data);
         delete pPhysicalImages[i];
         pPhysicalImages[i] = NULL;
      }
   }
   pPhysicalImages.RemoveAllOf(NULL);
   pPhysicalBuffers.RemoveAllOf(NULL);
   return CT_SUCCESS;
}

ctResults ctGPUArchitectBackendVulkan::DereferenceAll() {
   ZoneScoped;
   for (size_t i = 0; i < pPhysicalBuffers.Count(); i++) {
      pPhysicalBuffers[i]->refCount = 0;
   }
   for (size_t i = 0; i < pPhysicalImages.Count(); i++) {
      pPhysicalImages[i]->refCount = 0;
   }
   return CT_SUCCESS;
}

bool ctGPUArchitectBackendVulkan::isBufferAliasable(
  const PhysicalBuffer& physical, const ctGPUArchitectBufferPayload& desired) {
   ZoneScoped;
   /* Dont allow feedback to be aliased */
   if (ctCFlagCheck(physical.flags, CT_GPU_PAYLOAD_FEEDBACK)) { return false; }
   /* Check for settings overlap */
   if (physical.size < desired.size) { return false; }
   /* Check for lifetime overlap */
   ctGPUArchitectDependencyRange desiredLifetime =
     pArchitect->GetDependencyLifetime(desired.identifier);
   for (size_t i = 0; i < physical.mappings.Count(); i++) {
      ctGPUArchitectDependencyRange currentLifetime =
        pArchitect->GetDependencyLifetime(physical.mappings[i]);
      if (desiredLifetime.isOverlapping(currentLifetime)) { return false; }
   }
   return true;
}

bool ctGPUArchitectBackendVulkan::isImageAliasable(
  const PhysicalImage& physical, const ctGPUArchitectImagePayload& desired) {
   ZoneScoped;
   /* Dont allow feedback to be aliased */
   if (ctCFlagCheck(physical.flags, CT_GPU_PAYLOAD_FEEDBACK)) { return false; }
   if (physical.flags != desired.flags) { return false; }
   if (physical.widthPixels != GetPhysicalImageWidth(desired.flags, desired.width)) {
      return false;
   }
   if (physical.heightPixels != GetPhysicalImageHeight(desired.flags, desired.height)) {
      return false;
   }
   if (physical.layers != desired.layers) { return false; }
   if (physical.miplevels != desired.miplevels) { return false; }
   if (physical.format != (VkFormat)TinyImageFormat_ToVkFormat(desired.format)) {
      return false;
   }
   /* Check for lifetime overlap */
   ctGPUArchitectDependencyRange desiredLifetime =
     pArchitect->GetDependencyLifetime(desired.identifier);
   for (size_t i = 0; i < physical.mappings.Count(); i++) {
      ctGPUArchitectDependencyRange currentLifetime =
        pArchitect->GetDependencyLifetime(physical.mappings[i]);
      if (desiredLifetime.isOverlapping(currentLifetime)) { return false; }
   }
   return true;
}

ctResults
ctGPUArchitectBackendVulkan::CreateNewBuffer(ctGPUArchitectBufferPayload* pPayloadData) {
   ZoneScoped;
   PhysicalBuffer* pPhysicalBuffer = new PhysicalBuffer();
   pPhysicalBuffer->refCount = 1;
   pPhysicalBuffer->flags = pPayloadData->flags;
   pPhysicalBuffer->size = pPayloadData->size;
   pPhysicalBuffer->mappings.Append(pPayloadData->identifier);
   pPhysicalBuffers.Append(pPhysicalBuffer);

   VkBufferUsageFlags knownUsage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                   VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                                   VK_BUFFER_USAGE_TRANSFER_DST_BIT;
   if (pDevice->CreateCompleteBuffer(pPhysicalBuffer->data,
                                     knownUsage,
                                     VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
                                     pPayloadData->size) != VK_SUCCESS) {
      ctAssert(0);
      return CT_FAILURE_UNKNOWN;
   }
   pPayloadData->apiData = pPhysicalBuffer;
   return CT_SUCCESS;
}

ctResults
ctGPUArchitectBackendVulkan::CreateNewImage(ctGPUArchitectImagePayload* pPayloadData) {
   ZoneScoped;
   PhysicalImage* pPhysicalImage = new PhysicalImage();
   pPhysicalImage->refCount = 1;
   pPhysicalImage->flags = pPayloadData->flags;

   pPhysicalImage->widthPixels =
     GetPhysicalImageWidth(pPayloadData->flags, pPayloadData->width);
   pPhysicalImage->heightPixels =
     GetPhysicalImageHeight(pPayloadData->flags, pPayloadData->height);
   pPhysicalImage->mappings.Append(pPayloadData->identifier);
   pPhysicalImages.Append(pPhysicalImage);

   VkImageUsageFlags knownUsage = VK_IMAGE_USAGE_SAMPLED_BIT |
                                  VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                                  VK_BUFFER_USAGE_TRANSFER_DST_BIT;
   VkImageAspectFlags aspect;
   if (TinyImageFormat_IsDepthOnly(pPayloadData->format)) {
      pPhysicalImage->hasDepth = true;
      pPhysicalImage->hasStencil = false;
      knownUsage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
      aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
   } else if (TinyImageFormat_IsStencilOnly(pPayloadData->format)) {
      pPhysicalImage->hasDepth = false;
      pPhysicalImage->hasStencil = true;
      knownUsage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
      aspect = VK_IMAGE_ASPECT_STENCIL_BIT;
   } else if (TinyImageFormat_IsDepthAndStencil(pPayloadData->format)) {
      pPhysicalImage->hasDepth = true;
      pPhysicalImage->hasStencil = true;
      knownUsage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
      aspect = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
   } else {
      pPhysicalImage->hasDepth = false;
      pPhysicalImage->hasStencil = false;
      knownUsage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
      aspect = VK_IMAGE_ASPECT_COLOR_BIT;
   }

   pPhysicalImage->useClear = (bool)pPayloadData->pClearDesc;
   // clang-format off
   if (pPhysicalImage->useClear) {
       if (pPhysicalImage->hasDepth || pPhysicalImage->hasStencil) {
           pPhysicalImage->clearValue.depthStencil.depth = pPayloadData->pClearDesc->depth;
           pPhysicalImage->clearValue.depthStencil.stencil = pPayloadData->pClearDesc->stencil;
       }
       else if (TinyImageFormat_IsFloat(pPayloadData->format)) {
           pPhysicalImage->clearValue.color.float32[0] = pPayloadData->pClearDesc->rgba[0];
           pPhysicalImage->clearValue.color.float32[1] = pPayloadData->pClearDesc->rgba[1];
           pPhysicalImage->clearValue.color.float32[2] = pPayloadData->pClearDesc->rgba[2];
           pPhysicalImage->clearValue.color.float32[3] = pPayloadData->pClearDesc->rgba[3];
       }
       else if (TinyImageFormat_IsSigned(pPayloadData->format)) {
           pPhysicalImage->clearValue.color.int32[0] = (int32_t)pPayloadData->pClearDesc->rgba[0];
           pPhysicalImage->clearValue.color.int32[1] = (int32_t)pPayloadData->pClearDesc->rgba[1];
           pPhysicalImage->clearValue.color.int32[2] = (int32_t)pPayloadData->pClearDesc->rgba[2];
           pPhysicalImage->clearValue.color.int32[3] = (int32_t)pPayloadData->pClearDesc->rgba[3];
       }
       else {
           pPhysicalImage->clearValue.color.uint32[0] = (uint32_t)pPayloadData->pClearDesc->rgba[0];
           pPhysicalImage->clearValue.color.uint32[1] = (uint32_t)pPayloadData->pClearDesc->rgba[1];
           pPhysicalImage->clearValue.color.uint32[2] = (uint32_t)pPayloadData->pClearDesc->rgba[2];
           pPhysicalImage->clearValue.color.uint32[3] = (uint32_t)pPayloadData->pClearDesc->rgba[3];
       }
   }
   // clang-format on

   VkImageType imageType = VK_IMAGE_TYPE_2D;
   VkImageViewType viewType;
   if (ctCFlagCheck(pPayloadData->flags, CT_GPU_PAYLOAD_IMAGE_3D)) {
      imageType = VK_IMAGE_TYPE_3D;
      viewType = VK_IMAGE_VIEW_TYPE_3D;
   } else if (ctCFlagCheck(pPayloadData->flags, CT_GPU_PAYLOAD_IMAGE_CUBEMAP)) {
      if (pPayloadData->layers == 1) {
         viewType = VK_IMAGE_VIEW_TYPE_CUBE;
      } else {
         viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
      }
   } else {
      if (pPayloadData->layers == 1) {
         viewType = VK_IMAGE_VIEW_TYPE_2D;
      } else {
         viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
      }
   }

   pPhysicalImage->format = (VkFormat)TinyImageFormat_ToVkFormat(pPayloadData->format);
   if (pDevice->CreateCompleteImage(
         pPhysicalImage->data,
         pPhysicalImage->format,
         knownUsage,
         VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
         aspect,
         pPhysicalImage->widthPixels,
         pPhysicalImage->heightPixels,
         ctCFlagCheck(pPayloadData->flags, CT_GPU_PAYLOAD_IMAGE_3D) ? pPayloadData->layers
                                                                    : 1,
         pPayloadData->miplevels,
         !ctCFlagCheck(pPayloadData->flags, CT_GPU_PAYLOAD_IMAGE_3D)
           ? pPayloadData->layers
           : 1,
         VK_SAMPLE_COUNT_1_BIT,
         imageType,
         viewType)) {
      ctAssert(0);
      return CT_FAILURE_UNKNOWN;
   }
   pPayloadData->apiData = pPhysicalImage;
   return CT_SUCCESS;
}

VkRenderingAttachmentInfoKHR
ctGPUArchitectBackendVulkan::RenderingAttachmentInfoFromImage(
  PhysicalImage* image, ctGPUArchitectDependencyEntry currentDep) {
   ZoneScoped;
   VkRenderingAttachmentInfoKHR info = {
     VK_STRUCTURE_TYPE_MAX_ENUM /* todo: struct type*/};
   if (image->hasDepth && !image->hasStencil) {
      info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
   } else if (!image->hasDepth && image->hasStencil) {
      info.imageLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
   } else if (image->hasDepth && image->hasStencil) {
      info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
   } else {
      info.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
   }
   if (ctCFlagCheck(currentDep.access, CT_GPU_ACCESS_READ)) {
      info.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
   } else {
      if (image->useClear) {
         info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
         info.clearValue = image->clearValue;
      } else {
         info.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      }
   }
   info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
   info.imageView = image->data.view;
   /* todo: msaa support */
   info.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
   info.resolveMode = VK_RESOLVE_MODE_NONE;
   info.resolveImageView = VK_NULL_HANDLE;
   return info;
}
