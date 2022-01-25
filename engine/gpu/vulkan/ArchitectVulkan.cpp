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

CT_API ctResults ctGPUArchitectStartup(ctGPUDevice* pDevice,
                                       ctGPUArchitect** ppArchitect,
                                       ctGPUArchitectCreateInfo* pCreateInfo) {
   ZoneScoped;
   *ppArchitect = new ctGPUArchitectVulkan();
   return (*ppArchitect)->BackendStartup(pDevice);
}

CT_API ctResults ctGPUArchitectShutdown(ctGPUDevice* pDevice,
                                        ctGPUArchitect* pArchitect) {
   ZoneScoped;
   return pArchitect->BackendShutdown(pDevice);
   delete (ctGPUArchitectVulkan*)pArchitect;
}

ctResults ctGPUArchitectVulkan::BackendStartup(ctGPUDevice* pDevice) {
   ZoneScoped;
   VkSemaphoreCreateInfo semaphoreInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};

   /* Create command buffer managers for each queue */
   struct queuePairEntry {
      uint32_t idx;
      VkQueue queue;
      int operator==(queuePairEntry b) {
         return idx == b.idx && queue == b.queue;
      }
   };
   queuePairEntry graphicsEntry = {pDevice->queueFamilyIndices.graphicsIdx,
                                   pDevice->graphicsQueue};
   queuePairEntry computeEntry = {pDevice->queueFamilyIndices.computeIdx,
                                  pDevice->computeQueue};
   queuePairEntry transferEntry = {pDevice->queueFamilyIndices.transferIdx,
                                   pDevice->transferQueue};
   ctStaticArray<queuePairEntry, 3> uniqueQueueFamilies;
   uniqueQueueFamilies.InsertUnique(graphicsEntry);
   uniqueQueueFamilies.InsertUnique(computeEntry);
   uniqueQueueFamilies.InsertUnique(transferEntry);
   uniqueManagers.Resize(uniqueQueueFamilies.Count());
   for (int i = 0; i < uniqueQueueFamilies.Count(); i++) {
      uniqueManagers[i].Startup(
        pDevice, uniqueQueueFamilies[i].idx, uniqueQueueFamilies[i].queue);
   }
   pGraphicsCommands = &uniqueManagers[uniqueQueueFamilies.FindIndex(graphicsEntry)];
   pComputeCommands = &uniqueManagers[uniqueQueueFamilies.FindIndex(computeEntry)];
   pTransferCommands = &uniqueManagers[uniqueQueueFamilies.FindIndex(transferEntry)];

   VkFenceCreateInfo fenceInfo = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
   fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
   for (int i = 0; i < CT_MAX_INFLIGHT_FRAMES; i++) {
      vkCreateSemaphore(
        pDevice->vkDevice, &semaphoreInfo, &pDevice->vkAllocCallback, &outputReady[i]);
      vkCreateFence(
        pDevice->vkDevice, &fenceInfo, &pDevice->vkAllocCallback, &finishedFence[i]);
   }
   return CT_SUCCESS;
}

ctResults ctGPUArchitectVulkan::BackendShutdown(ctGPUDevice* pDevice) {
   ZoneScoped;
   vkDeviceWaitIdle(pDevice->vkDevice);
   for (int i = 0; i < CT_MAX_INFLIGHT_FRAMES; i++) {
      vkDestroyFence(pDevice->vkDevice, finishedFence[i], &pDevice->vkAllocCallback);
      vkDestroySemaphore(pDevice->vkDevice, outputReady[i], &pDevice->vkAllocCallback);
   }
   DereferenceAll();
   GarbageCollect(pDevice);
   for (int i = 0; i < uniqueManagers.Count(); i++) {
      uniqueManagers[i].Shutdown(pDevice);
   }
   return CT_SUCCESS;
}

ctResults ctGPUArchitectVulkan::BackendBuild(ctGPUDevice* pDevice) {
   ZoneScoped;
   DereferenceAll();
   for (auto it = cpLogicalBufferPayloads.GetIterator(); it; it++) {
      MapToBuffer(pDevice, it.Value());
   }
   for (auto it = cpLogicalImagePayloads.GetIterator(); it; it++) {
      MapToImage(pDevice, it.Value());
   }
   GarbageCollect(pDevice);
   isRenderable = true;
   return CT_SUCCESS;
}

void ctGPUArchitectVulkan::CommandBufferManager::Startup(ctGPUDevice* pDevice,
                                                         uint32_t queueFamilyIdx,
                                                         VkQueue targetQueue) {
   VkCommandPoolCreateInfo poolInfo = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
   poolInfo.queueFamilyIndex = queueFamilyIdx;
   poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
   vkCreateCommandPool(pDevice->vkDevice, &poolInfo, &pDevice->vkAllocCallback, &pool);
   VkCommandBufferAllocateInfo allocInfo = {
     VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
   allocInfo.commandBufferCount = CT_MAX_INFLIGHT_FRAMES;
   allocInfo.commandPool = pool;
   allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
   vkAllocateCommandBuffers(pDevice->vkDevice, &allocInfo, cmd);
   frame = -1;
   queue = targetQueue;
}

void ctGPUArchitectVulkan::CommandBufferManager::Shutdown(ctGPUDevice* pDevice) {
   vkFreeCommandBuffers(pDevice->vkDevice, pool, CT_MAX_INFLIGHT_FRAMES, cmd);
   vkDestroyCommandPool(pDevice->vkDevice, pool, &pDevice->vkAllocCallback);
}

VkCommandBuffer ctGPUArchitectVulkan::CommandBufferManager::GetCmd() {
   return cmd[frame];
}

void ctGPUArchitectVulkan::CommandBufferManager::Submit(ctGPUDevice* pDevice) {
   if (frame < 0) { return; }
   vkEndCommandBuffer(cmd[frame]);
   VkSubmitInfo info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
   info.commandBufferCount = 1;
   info.pCommandBuffers = &cmd[frame];
   info.signalSemaphoreCount = activeSemaphore == VK_NULL_HANDLE ? 0 : 1;
   info.pSignalSemaphores = &activeSemaphore;
   vkQueueSubmit(queue, 1, &info, activeFence);
}

void ctGPUArchitectVulkan::CommandBufferManager::NextFrame() {
   frame = (frame + 1) % CT_MAX_INFLIGHT_FRAMES;
   ctAssert(frame >= 0);
   VkCommandBuffer result = cmd[frame];
   VkCommandBufferBeginInfo begin = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
   vkBeginCommandBuffer(result, &begin);
}

struct ExecutionInternalData {
   ctGPUArchitectVulkan* pBackend;
   ctGPUArchitectTaskInternal* pTask;
};

VkPipelineStageFlags GetStageForCategory(ctGPUArchitectTaskCategory category) {
   switch (category) {
      case CT_GPU_TASK_RASTER: return VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
      case CT_GPU_TASK_COMPUTE: return VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
      case CT_GPU_TASK_TRANSFER: return VK_PIPELINE_STAGE_TRANSFER_BIT;
      case CT_GPU_TASK_RAYTRACE: return VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
      default: return 0;
   }
}

ctResults ctGPUArchitectVulkan::BackendExecute(ctGPUDevice* pDevice) {
   ZoneScoped;
   if (!isRenderable) { return CT_FAILURE_DEPENDENCY_NOT_MET; }
   currentFrame = (currentFrame + 1) % CT_MAX_INFLIGHT_FRAMES;
   vkWaitForFences(
     pDevice->vkDevice, 1, &finishedFence[currentFrame], VK_TRUE, UINT64_MAX);
   vkResetFences(pDevice->vkDevice, 1, &finishedFence[currentFrame]);
   for (int i = 0; i < uniqueManagers.Count(); i++) {
      uniqueManagers[i].NextFrame();
   }

   ctGPUArchitectTaskCategory lastTaskCategory = CT_GPU_TASK_UNDEFINED;
   ctGPUArchitectTaskCategory lastOutputTouchCategory = CT_GPU_TASK_UNDEFINED;
   for (auto current = GetFinalTaskIterator(); current; current++) {
      ctGPUArchitectTaskInternal& task = current.Task();
      if (lastTaskCategory == CT_GPU_TASK_UNDEFINED) { lastTaskCategory = task.category; }
      /* Do Raster Commands */
      if (task.category == CT_GPU_TASK_RASTER) {
         VkCommandBuffer cmd = pGraphicsCommands->GetCmd();

         /* Build render info */
         VkRenderingAttachmentInfoKHR colorInfos[8];
         VkRenderingAttachmentInfoKHR depthStencilInfo;
         bool hasStencil = false;
         bool hasDepth = false;
         VkRenderingInfoKHR renderInfo = {VK_STRUCTURE_TYPE_RENDERING_INFO_KHR};
         renderInfo.flags = 0;
         renderInfo.renderArea = {0};
         renderInfo.layerCount = 1;
         renderInfo.pColorAttachments = colorInfos;
         renderInfo.viewMask = 0;

         /* Setup dependencies */
         VkFormat depthStencilFormat = VK_FORMAT_UNDEFINED;
         ctStaticArray<VkFormat, 8> colorFormats;
         VkImageLayout depthLayout = VK_IMAGE_LAYOUT_UNDEFINED;
         ctStaticArray<VkImageLayout, 8> colorLayouts;
         ctGPUArchVkPhysicalImage* pDepthImage = NULL;
         ctStaticArray<ctGPUArchVkPhysicalImage*, 8> pColorImages;
         for (int i = 0; i < task.dependencies.Count(); i++) {
            ctGPUArchitectDependencyEntry& dependency = task.dependencies[i];
            if (dependency.resourceId == outputDependency) {
               lastOutputTouchCategory = CT_GPU_TASK_RASTER;
            }
            /* Depth and color targets become a part of the renderpass info */
            if (dependency.type == CT_GPU_ARCH_COLOR_TARGET ||
                dependency.type == CT_GPU_ARCH_DEPTH_TARGET) {
               ctGPUArchitectImagePayload* pLPayload =
                 GetImagePayloadPtrForDependency(dependency.resourceId);
               ctAssert(pLPayload);
               ctGPUArchVkPhysicalImage* image =
                 (ctGPUArchVkPhysicalImage*)pLPayload->apiData;
               ctAssert(image);

               /* Coming from another queue family */
               if (image->lastQueueFamilyIdx != pDevice->queueFamilyIndices.graphicsIdx) {
                  DoResourceTransition(
                    cmd, dependency, pDevice->queueFamilyIndices.graphicsIdx);
               }

               uint32_t width = image->widthPixels;
               uint32_t height = image->heightPixels;
               uint32_t layers = image->layers;
               if (renderInfo.layerCount == 0) { renderInfo.layerCount = layers; }
               if (renderInfo.renderArea.extent.width == 0) {
                  renderInfo.renderArea.extent.width = width;
                  renderInfo.renderArea.extent.height = height;
               }
               /* One of the attachments has a mismatched resolution */
               ctAssert(renderInfo.layerCount == layers);
               ctAssert(renderInfo.renderArea.extent.width == width);
               ctAssert(renderInfo.renderArea.extent.height == height);

               if (dependency.type == CT_GPU_ARCH_COLOR_TARGET) {
                  if (renderInfo.colorAttachmentCount <= dependency.slot) {
                     renderInfo.colorAttachmentCount = dependency.slot + 1;
                     colorInfos[dependency.slot] =
                       RenderingAttachmentInfoFromImage(image, dependency);
                     colorFormats.Append(image->format);
                     colorLayouts.Append(image->currentLayout);
                     pColorImages.Append(image);
                  }
               }
               // clang-format off
                  if (dependency.type == CT_GPU_ARCH_DEPTH_TARGET) {
                      depthStencilInfo = RenderingAttachmentInfoFromImage(image, dependency);
                      if (image->hasStencil) { renderInfo.pStencilAttachment = &depthStencilInfo; }
                      if (image->hasDepth) { renderInfo.pDepthAttachment = &depthStencilInfo; }
                      depthStencilFormat = image->format;
                      depthLayout = image->currentLayout;
                      pDepthImage = image;
                  }
               // clang-format on
            } else {
               DoResourceTransition(
                 cmd, dependency, pDevice->queueFamilyIndices.graphicsIdx);
            }
         }
         /* Handle Pipeline Barrier */
         if (lastTaskCategory != CT_GPU_TASK_RASTER) {
            vkCmdPipelineBarrier(cmd,
                                 GetStageForCategory(task.category),
                                 GetStageForCategory(lastTaskCategory),
                                 0,
                                 0,
                                 NULL,
                                 0,
                                 NULL,  // todo: buffer barriers
                                 0,
                                 NULL);
         }
         pDevice->MarkBeginRegion(cmd, task.debugName);
         pDevice->BeginJITRenderPass(cmd,
                                     &renderInfo,
                                     depthStencilFormat,
                                     colorFormats.Data(),
                                     depthLayout,
                                     colorLayouts.Data());
         ctGPUArchitectExecutionContext ctx = {};
         ExecutionInternalData internalData = {};
         internalData.pBackend = this;
         internalData.pTask = &task;
         ctx.category = CT_GPU_TASK_RASTER;
         ctx.raster.width = renderInfo.renderArea.extent.width;
         ctx.raster.height = renderInfo.renderArea.extent.height;
         ctx.raster.layerCount = renderInfo.layerCount;
         ctx.cmd = cmd;
         ctx.pArchitect = this;
         ctx.pDevice = pDevice;
         ctx._internalData = &internalData;
         if (task.fpExecution) { task.fpExecution(&ctx, task.pUserData); }
         pDevice->EndJITRenderpass(cmd);
         pDevice->MarkEndRegion(cmd);

         /* Keep track of layout changes */
         if (pDepthImage) {
            pDepthImage->currentLayout = renderInfo.pDepthAttachment->imageLayout;
         }
         for (size_t i = 0; i < pColorImages.Count(); i++) {
            pColorImages[i]->currentLayout = renderInfo.pColorAttachments->imageLayout;
         }
      } /* end raster commands */
      /* Do Compute Commands */
      else if (task.category == CT_GPU_TASK_COMPUTE) {
         VkCommandBuffer cmd = pComputeCommands->GetCmd();

         /* Handle Dependencies */
         for (int i = 0; i < task.dependencies.Count(); i++) {
            ctGPUArchitectDependencyEntry& dependency = task.dependencies[i];
            if (dependency.resourceId == outputDependency) {
               lastOutputTouchCategory = CT_GPU_TASK_COMPUTE;
            }
            DoResourceTransition(cmd, dependency, pDevice->queueFamilyIndices.computeIdx);
         }

         /* Handle Pipeline Barrier */
         if (lastTaskCategory != CT_GPU_TASK_RASTER) {
            vkCmdPipelineBarrier(cmd,
                                 GetStageForCategory(task.category),
                                 GetStageForCategory(lastTaskCategory),
                                 0,
                                 0,
                                 NULL,
                                 0,
                                 NULL,  // todo: buffer barriers
                                 0,
                                 NULL);  // todo: image barriers
         }

         pDevice->MarkBeginRegion(cmd, task.debugName);
         ctGPUArchitectExecutionContext ctx = {};
         ExecutionInternalData internalData = {};
         internalData.pBackend = this;
         internalData.pTask = &task;
         ctx.category = CT_GPU_TASK_COMPUTE;
         ctx.raster = {};
         ctx.cmd = cmd;
         ctx.pArchitect = this;
         ctx.pDevice = pDevice;
         ctx._internalData = &internalData;
         if (task.fpExecution) { task.fpExecution(&ctx, task.pUserData); }
         pDevice->MarkEndRegion(cmd);
      }
      /* Do Transfer Commands */
      else if (task.category == CT_GPU_TASK_TRANSFER) {
         VkCommandBuffer cmd = pTransferCommands->GetCmd();

         /* Handle Dependencies */
         for (int i = 0; i < task.dependencies.Count(); i++) {
            ctGPUArchitectDependencyEntry& dependency = task.dependencies[i];
            if (dependency.resourceId == outputDependency) {
               lastOutputTouchCategory = CT_GPU_TASK_TRANSFER;
            }
            DoResourceTransition(
              cmd, dependency, pDevice->queueFamilyIndices.transferIdx);
         }

         pDevice->MarkBeginRegion(cmd, task.debugName);
         ctGPUArchitectExecutionContext ctx = {};
         ExecutionInternalData internalData = {};
         internalData.pBackend = this;
         internalData.pTask = &task;
         ctx.category = CT_GPU_TASK_TRANSFER;
         ctx.raster = {};
         ctx.cmd = cmd;
         ctx.pArchitect = this;
         ctx.pDevice = pDevice;
         ctx._internalData = &internalData;
         if (task.fpExecution) { task.fpExecution(&ctx, task.pUserData); }
         pDevice->MarkEndRegion(cmd);
      }
      lastTaskCategory = task.category;
   }

   /* Setup sync */
   switch (lastTaskCategory) {
      case CT_GPU_TASK_RASTER:
         pGraphicsCommands->activeFence = finishedFence[currentFrame];
         break;
      case CT_GPU_TASK_COMPUTE:
         pComputeCommands->activeFence = finishedFence[currentFrame];
         break;
      case CT_GPU_TASK_TRANSFER:
         pTransferCommands->activeFence = finishedFence[currentFrame];
         break;
      case CT_GPU_TASK_RAYTRACE: break;
      default: ctAssert(0); break;
   }
   switch (lastOutputTouchCategory) {
      case CT_GPU_TASK_RASTER:
         pGraphicsCommands->activeSemaphore = outputReady[currentFrame];
         break;
      case CT_GPU_TASK_COMPUTE:
         pComputeCommands->activeSemaphore = outputReady[currentFrame];
         break;
      case CT_GPU_TASK_TRANSFER:
         pTransferCommands->activeSemaphore = outputReady[currentFrame];
         break;
      case CT_GPU_TASK_RAYTRACE: break;
      default: ctAssert(0); break;
   }

   for (int i = 0; i < uniqueManagers.Count(); i++) {
      uniqueManagers[i].Submit(pDevice);
   }
   return CT_SUCCESS;
}

ctResults ctGPUArchitectVulkan::BackendReset(ctGPUDevice* pDevice) {
   ZoneScoped;
   DereferenceAll();
   GarbageCollect(pDevice);
   pPhysicalBuffers.Clear();
   pPhysicalImages.Clear();
   isRenderable = false;
   return CT_SUCCESS;
}

ctResults ctGPUArchitectVulkan::MapToBuffer(ctGPUDevice* pDevice,
                                            ctGPUArchitectBufferPayload* pPayloadData) {
   ZoneScoped;
   if (ctCFlagCheck(pPayloadData->flags, CT_GPU_PAYLOAD_FEEDBACK)) {
      return CreateNewBuffer(pDevice, pPayloadData);
   }
   for (size_t i = 0; i < pPhysicalBuffers.Count(); i++) {
      if (isBufferAliasable(*pPhysicalBuffers[i], *pPayloadData)) {
         pPhysicalBuffers[i]->refCount++;
         pPhysicalBuffers[i]->mappings.Append(pPayloadData->identifier);
         pPayloadData->apiData = pPhysicalBuffers[i];
         return CT_SUCCESS;
      }
   }
   return CreateNewBuffer(pDevice, pPayloadData);
}

ctResults ctGPUArchitectVulkan::MapToImage(ctGPUDevice* pDevice,
                                           ctGPUArchitectImagePayload* pPayloadData) {
   ZoneScoped;
   if (ctCFlagCheck(pPayloadData->flags, CT_GPU_PAYLOAD_FEEDBACK)) {
      return CreateNewImage(pDevice, pPayloadData);
   }
   for (size_t i = 0; i < pPhysicalImages.Count(); i++) {
      if (isImageAliasable(*pPhysicalImages[i], *pPayloadData)) {
         pPhysicalImages[i]->refCount++;
         pPhysicalImages[i]->mappings.Append(pPayloadData->identifier);
         pPayloadData->apiData = pPhysicalImages[i];
         return CT_SUCCESS;
      }
   }
   return CreateNewImage(pDevice, pPayloadData);
}

ctResults ctGPUArchitectVulkan::GarbageCollect(ctGPUDevice* pDevice) {
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

ctResults ctGPUArchitectVulkan::DereferenceAll() {
   ZoneScoped;
   for (size_t i = 0; i < pPhysicalBuffers.Count(); i++) {
      pPhysicalBuffers[i]->refCount = 0;
   }
   for (size_t i = 0; i < pPhysicalImages.Count(); i++) {
      pPhysicalImages[i]->refCount = 0;
   }
   return CT_SUCCESS;
}

bool ctGPUArchitectVulkan::GetOutput(ctGPUDevice* pDevice,
                                     uint32_t& width,
                                     uint32_t& height,
                                     uint32_t& queueFamily,
                                     VkImageLayout& layout,
                                     VkAccessFlags& access,
                                     VkPipelineStageFlags& stage,
                                     VkImage& image) {
   ctGPUArchitectImagePayload* pPayload =
     GetImagePayloadPtrForDependency(outputDependency);
   if (!pPayload) { return false; }
   image = ((ctGPUArchVkPhysicalImage*)pPayload->apiData)->data.image;
   queueFamily = pDevice->queueFamilyIndices.graphicsIdx;
   width = GetPhysicalImageWidth(pPayload->flags, pPayload->width);
   height = GetPhysicalImageHeight(pPayload->flags, pPayload->height);
   layout = ((ctGPUArchVkPhysicalImage*)pPayload->apiData)->currentLayout;
   access = 0;                                  // todo
   stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;  // todo
   return true;
}

bool ctGPUArchitectVulkan::isBufferAliasable(const ctGPUArchVkPhysicalBuffer& physical,
                                             const ctGPUArchitectBufferPayload& desired) {
   ZoneScoped;
   /* Dont allow feedback to be aliased */
   if (ctCFlagCheck(physical.flags, CT_GPU_PAYLOAD_FEEDBACK)) { return false; }
   /* Check for settings overlap */
   if (physical.size < desired.size) { return false; }
   /* Check for lifetime overlap */
   ctGPUArchitectDependencyRange desiredLifetime =
     GetDependencyLifetime(desired.identifier);
   for (size_t i = 0; i < physical.mappings.Count(); i++) {
      ctGPUArchitectDependencyRange currentLifetime =
        GetDependencyLifetime(physical.mappings[i]);
      if (desiredLifetime.isOverlapping(currentLifetime)) { return false; }
   }
   return true;
}

bool ctGPUArchitectVulkan::isImageAliasable(const ctGPUArchVkPhysicalImage& physical,
                                            const ctGPUArchitectImagePayload& desired) {
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
     GetDependencyLifetime(desired.identifier);
   for (size_t i = 0; i < physical.mappings.Count(); i++) {
      ctGPUArchitectDependencyRange currentLifetime =
        GetDependencyLifetime(physical.mappings[i]);
      if (desiredLifetime.isOverlapping(currentLifetime)) { return false; }
   }
   return true;
}

ctResults
ctGPUArchitectVulkan::CreateNewBuffer(ctGPUDevice* pDevice,
                                      ctGPUArchitectBufferPayload* pPayloadData) {
   ZoneScoped;
   ctGPUArchVkPhysicalBuffer* pPhysicalBuffer = new ctGPUArchVkPhysicalBuffer();
   pPhysicalBuffer->refCount = 1;
   pPhysicalBuffer->flags = pPayloadData->flags;
   pPhysicalBuffer->lastQueueFamilyIdx = -1; /* not owned yet */
   pPhysicalBuffer->size = pPayloadData->size;
   pPhysicalBuffer->mappings.Append(pPayloadData->identifier);
   pPhysicalBuffers.Append(pPhysicalBuffer);

   VkBufferUsageFlags knownUsage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                   VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                                   VK_BUFFER_USAGE_TRANSFER_DST_BIT;
   if (pDevice->CreateCompleteBuffer(pPayloadData->debugName,
                                     pPhysicalBuffer->data,
                                     knownUsage,
                                     VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
                                     pPayloadData->size) != VK_SUCCESS) {
      ctAssert(0);
      return CT_FAILURE_UNKNOWN;
   }
   pPayloadData->apiData = pPhysicalBuffer;
   return CT_SUCCESS;
}

ctResults ctGPUArchitectVulkan::CreateNewImage(ctGPUDevice* pDevice,
                                               ctGPUArchitectImagePayload* pPayloadData) {
   ZoneScoped;
   ctGPUArchVkPhysicalImage* pPhysicalImage = new ctGPUArchVkPhysicalImage();
   pPhysicalImage->refCount = 1;
   pPhysicalImage->flags = pPayloadData->flags;
   pPhysicalImage->currentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
   pPhysicalImage->lastQueueFamilyIdx = -1; /* not owned yet */

   pPhysicalImage->widthPixels =
     GetPhysicalImageWidth(pPayloadData->flags, pPayloadData->width);
   pPhysicalImage->heightPixels =
     GetPhysicalImageHeight(pPayloadData->flags, pPayloadData->height);
   pPhysicalImage->layers = pPayloadData->layers;
   pPhysicalImage->miplevels = pPayloadData->miplevels;
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
       else if (TinyImageFormat_IsNormalised(pPayloadData->format) || TinyImageFormat_IsFloat(pPayloadData->format)) {
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
         pPayloadData->debugName,
         pPhysicalImage->data,
         pPhysicalImage->format,
         knownUsage,
         VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
         aspect,
         pPhysicalImage->widthPixels,
         pPhysicalImage->heightPixels,
         ctCFlagCheck(pPhysicalImage->flags, CT_GPU_PAYLOAD_IMAGE_3D)
           ? pPhysicalImage->layers
           : 1,
         pPhysicalImage->miplevels,
         !ctCFlagCheck(pPhysicalImage->flags, CT_GPU_PAYLOAD_IMAGE_3D)
           ? pPhysicalImage->layers
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

VkRenderingAttachmentInfoKHR ctGPUArchitectVulkan::RenderingAttachmentInfoFromImage(
  ctGPUArchVkPhysicalImage* image, ctGPUArchitectDependencyEntry currentDep) {
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

void ctGPUArchitectVulkan::DoResourceTransition(ctGPUCommandBuffer cmd,
                                                ctGPUArchitectDependencyEntry& dependency,
                                                int32_t queueIdx) {
}

CT_API ctResults ctGPUTaskGetImageAccessor(ctGPUArchitectExecutionContext* pCtx,
                                           ctGPUDependencyID id,
                                           ctGPUImageAccessor* pAccessorOut) {
   ctGPUArchitectImagePayload** ppPayload =
     pCtx->pArchitect->cpLogicalImagePayloads.FindPtr(id);
   if (!ppPayload) { return CT_FAILURE_DATA_DOES_NOT_EXIST; }
   ctGPUArchVkPhysicalImage* pPhysicalImage =
     (ctGPUArchVkPhysicalImage*)(*ppPayload)->apiData;
   ctAssert(pPhysicalImage);
   *pAccessorOut = pPhysicalImage->data.image;
   return CT_SUCCESS;
}

CT_API ctResults ctGPUTaskGetBufferAccessor(ctGPUArchitectExecutionContext* pCtx,
                                            ctGPUDependencyID id,
                                            ctGPUBufferAccessor* pAccessorOut) {
   ctGPUArchitectBufferPayload** ppPayload =
     pCtx->pArchitect->cpLogicalBufferPayloads.FindPtr(id);
   if (!ppPayload) { return CT_FAILURE_DATA_DOES_NOT_EXIST; }
   ctGPUArchVkPhysicalBuffer* pPhysicalBuffer =
     (ctGPUArchVkPhysicalBuffer*)(*ppPayload)->apiData;
   ctAssert(pPhysicalBuffer);
   *pAccessorOut = pPhysicalBuffer->data.buffer;
   return CT_SUCCESS;
}