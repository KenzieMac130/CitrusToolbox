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

class ctGPUArchitectBackendVulkan : public ctGPUArchitectBackend {
public:
   virtual ctResults Startup(ctGPUDevice* pDevice, struct ctGPUArchitect* pArchitect);
   virtual ctResults Shutdown();
   virtual ctResults BuildInternal();
   virtual ctResults ExecuteInternal();
   virtual ctResults ResetInternal();

   ctResults MapToBuffer(ctGPUArchitectBufferPayload* pPayloadData);
   ctResults MapToImage(ctGPUArchitectImagePayload* pPayloadData);
   ctResults GarbageCollect();
   ctResults DereferenceAll();

private:
   struct ctGPUDevice* pDevice;
   struct ctGPUArchitect* pArchitect;

   uint32_t screenWidth;
   uint32_t screenHeight;

   struct PhysicalBuffer {
      int32_t flags;
      size_t size;

      int32_t refCount;
      ctDynamicArray<ctGPUDependencyID> mappings;

      /* for now we just alias the whole buffer */
      ctVkCompleteBuffer data;
   };
   struct PhysicalImage {
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
   };
   ctDynamicArray<PhysicalBuffer*> pPhysicalBuffers;
   ctDynamicArray<PhysicalImage*> pPhysicalImages;

   bool isRenderable = false;

   bool isBufferAliasable(const PhysicalBuffer& physical,
                          const ctGPUArchitectBufferPayload& desired);
   bool isImageAliasable(const PhysicalImage& physical,
                         const ctGPUArchitectImagePayload& desired);
   ctResults CreateNewBuffer(ctGPUArchitectBufferPayload* pPayloadData);
   ctResults CreateNewImage(ctGPUArchitectImagePayload* pPayloadData);
   struct VkRenderingAttachmentInfoKHR
   RenderingAttachmentInfoFromImage(PhysicalImage* image,
                                    ctGPUArchitectDependencyEntry currentDep);

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
};