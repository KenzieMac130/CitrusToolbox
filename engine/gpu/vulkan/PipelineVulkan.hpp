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

#include "gpu/Pipeline.h"
#include "DeviceVulkan.hpp"

/* not there yet :P */
// Provided by VK_KHR_dynamic_rendering
typedef struct VkPipelineRenderingCreateInfoKHR {
   VkStructureType sType;
   const void* pNext;
   uint32_t viewMask;
   uint32_t colorAttachmentCount;
   const VkFormat* pColorAttachmentFormats;
   VkFormat depthAttachmentFormat;
   VkFormat stencilAttachmentFormat;
} VkPipelineRenderingCreateInfoKHR;

/* -------- Define Structure -------- */
struct ctGPUPipelineBuilder {
   ctGPUPipelineBuilder(ctGPUPipelineType pipelineType);
   ctGPUPipelineType type;
   union {
      struct {
         VkGraphicsPipelineCreateInfo createInfo;
         VkPipelineInputAssemblyStateCreateInfo inputAssembly;
         VkPipelineTessellationStateCreateInfo tessellation;
         VkPipelineViewportStateCreateInfo viewport;
         VkPipelineRasterizationStateCreateInfo rasterState;
         VkPipelineMultisampleStateCreateInfo msaa;
         VkPipelineDepthStencilStateCreateInfo depthStencil;
         VkPipelineColorBlendStateCreateInfo blendState;
         VkPipelineColorBlendAttachmentState attachmentBlends[8];
         VkPipelineDynamicStateCreateInfo dynamicState;
         VkPipelineRenderingCreateInfoKHR dynamicRendering;
         VkDynamicState dynamics[12];
         VkFormat colorFormats[8];
      } raster;
      struct {
         VkComputePipelineCreateInfo createInfo;
      } compute;
      struct {
         VkRayTracingPipelineCreateInfoKHR createInfo;
      } raytrace;
   };
   int32_t stageCount;
   VkPipelineShaderStageCreateInfo stages[8];
};