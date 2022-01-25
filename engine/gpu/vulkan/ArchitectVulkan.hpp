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

#include "gpu/shared/ArchitectGraphBuilder.hpp"
#include "DeviceVulkan.hpp"

struct ctGPUArchVkPhysicalBuffer {
   int32_t flags;
   size_t size;

   int32_t refCount;
   ctDynamicArray<ctGPUDependencyID> mappings;

   /* for now we just alias the whole buffer */
   ctVkCompleteBuffer data;

   int32_t lastQueueFamilyIdx;
};

struct ctGPUArchVkPhysicalImage {
   int32_t flags;
   uint32_t heightPixels;
   uint32_t widthPixels;
   uint32_t layers;
   uint32_t miplevels;

   bool hasDepth;
   bool hasStencil;
   VkFormat format;

   bool useClear;
   VkClearValue clearValue;

   int32_t refCount;
   ctDynamicArray<ctGPUDependencyID> mappings;

   /* for now we just alias the whole image */
   ctVkCompleteImage data;

   VkImageLayout currentLayout;
   int32_t lastQueueFamilyIdx;
};

struct ctGPUArchitectVulkan : ctGPUArchitect {
   virtual ctResults BackendStartup(ctGPUDevice* pDevice);
   virtual ctResults BackendShutdown(ctGPUDevice* pDevice);
   virtual ctResults BackendBuild(ctGPUDevice* pDevice);
   virtual ctResults BackendExecute(ctGPUDevice* pDevice);
   virtual ctResults BackendReset(ctGPUDevice* pDevice);

   ctResults MapToBuffer(ctGPUDevice* pDevice, ctGPUArchitectBufferPayload* pPayloadData);
   ctResults MapToImage(ctGPUDevice* pDevice, ctGPUArchitectImagePayload* pPayloadData);
   ctResults GarbageCollect(ctGPUDevice* pDevice);
   ctResults DereferenceAll();

   ctDynamicArray<ctGPUArchVkPhysicalBuffer*> pPhysicalBuffers;
   ctDynamicArray<ctGPUArchVkPhysicalImage*> pPhysicalImages;

   bool GetOutput(ctGPUDevice* pDevice,
                  uint32_t& width,
                  uint32_t& height,
                  uint32_t& queueFamily,
                  VkImageLayout& layout,
                  VkAccessFlags& access,
                  VkPipelineStageFlags& stage,
                  VkImage& image);

   int32_t currentFrame;
   VkSemaphore outputReady[CT_MAX_INFLIGHT_FRAMES];
   VkFence finishedFence[CT_MAX_INFLIGHT_FRAMES];

   bool isBufferAliasable(const ctGPUArchVkPhysicalBuffer& physical,
                          const ctGPUArchitectBufferPayload& desired);
   bool isImageAliasable(const ctGPUArchVkPhysicalImage& physical,
                         const ctGPUArchitectImagePayload& desired);
   ctResults CreateNewBuffer(ctGPUDevice* pDevice,
                             ctGPUArchitectBufferPayload* pPayloadData);
   ctResults CreateNewImage(ctGPUDevice* pDevice,
                            ctGPUArchitectImagePayload* pPayloadData);
   struct VkRenderingAttachmentInfoKHR
   RenderingAttachmentInfoFromImage(ctGPUArchVkPhysicalImage* image,
                                    ctGPUArchitectDependencyEntry currentDep);
   void DoResourceTransition(ctGPUCommandBuffer cmd,
                             ctGPUArchitectDependencyEntry& dependency,
                             int32_t queueIdx);

   /* Command buffers */
   struct CommandBufferManager {
      void Startup(ctGPUDevice* pDevice, uint32_t queueFamilyIdx, VkQueue targetQueue);
      void Shutdown(ctGPUDevice* pDevice);
      void NextFrame();
      VkCommandBuffer GetCmd();
      void Submit(ctGPUDevice* pDevice);

      int frame;
      VkCommandBuffer cmd[CT_MAX_INFLIGHT_FRAMES];
      VkQueue queue;
      VkCommandPool pool;
      VkSemaphore activeSemaphore;
      VkFence activeFence;
   };
   CommandBufferManager* pGraphicsCommands;
   CommandBufferManager* pComputeCommands;
   CommandBufferManager* pTransferCommands;
   ctStaticArray<CommandBufferManager, 3> uniqueManagers;
};