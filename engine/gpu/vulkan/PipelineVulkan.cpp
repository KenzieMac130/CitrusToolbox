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

#include "PipelineVulkan.hpp"
#include "formats/wad/prototypes/MarkersAndBlobs.h"

ctGPUPipelineBuilder::ctGPUPipelineBuilder(ctGPUPipelineType pipelineType) {
   memset(this, 0, sizeof(this));
   type = pipelineType;
   if (type == CT_GPU_PIPELINE_RASTER) {
      // clang-format off
       raster.createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
       raster.inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
       raster.tessellation.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
       raster.viewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
       raster.rasterState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
       raster.msaa.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
       raster.depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
       raster.blendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
       raster.dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
       raster.dynamicRendering.sType = (VkStructureType)VK_STRUCTURE_TYPE_MAX_ENUM; /* todo: use final */
      // clang-format on
      raster.createInfo.pInputAssemblyState = &raster.inputAssembly;
      raster.createInfo.pTessellationState = &raster.tessellation;
      raster.createInfo.pViewportState = &raster.viewport;
      raster.createInfo.pRasterizationState = &raster.rasterState;
      raster.createInfo.pMultisampleState = &raster.msaa;
      raster.createInfo.pDepthStencilState = &raster.depthStencil;
      raster.createInfo.pColorBlendState = &raster.blendState;
      raster.createInfo.pDynamicState = &raster.dynamicState;
      raster.createInfo.pNext = &raster.dynamicRendering;

      raster.dynamicState.pDynamicStates = raster.dynamics;
      raster.blendState.pAttachments = raster.attachmentBlends;
      raster.createInfo.pStages = stages;

      raster.createInfo.renderPass = VK_NULL_HANDLE;
      raster.createInfo.subpass = 0;
      raster.dynamicRendering.pColorAttachmentFormats = raster.colorFormats;

      ctGPUPipelineBuilderEnableDynamicState(this, CT_GPU_DYNAMICSTATE_VIEWPORT);
      ctGPUPipelineBuilderEnableDynamicState(this, CT_GPU_DYNAMICSTATE_SCISSOR);
   }
}

CT_API ctGPUPipelineBuilder* ctGPUPipelineBuilderNew(ctGPUDevice* pDevice,
                                                     ctGPUPipelineType type) {
   ctAssert(pDevice);
   return new ctGPUPipelineBuilder(type);
}

CT_API void ctGPUPipelineBuilderDelete(ctGPUPipelineBuilder* pBuilder) {
   ctAssert(pBuilder);
   delete pBuilder;
}

CT_API void ctGPUPipelineBuilderReset(ctGPUPipelineBuilder* pBuilder) {
   ctAssert(pBuilder);
   memset(pBuilder, 0, sizeof(*pBuilder));
}

CT_API ctResults ctGPUShaderCreateFromWad(ctGPUDevice* pDevice,
                                          ctGPUShaderModule* pShaderOut,
                                          ctWADReader* pWad,
                                          int32_t fxSegment,
                                          ctGPUShaderType type) {
   ctAssert(pDevice);
   ctAssert(pWad);
   ctAssert(type >= 0 && type < CT_GPU_SHADER_COUNT);
   const char* lumpNamesByType[] {CT_WADBLOB_NAME_SHADER_VERT_SPIRV,
                                  CT_WADBLOB_NAME_SHADER_FRAG_SPIRV,
                                  CT_WADBLOB_NAME_SHADER_GEOMETRY_SPIRV,
                                  CT_WADBLOB_NAME_SHADER_COMPUTE_SPIRV,
                                  CT_WADBLOB_NAME_SHADER_TESS_CONTROL_SPIRV,
                                  CT_WADBLOB_NAME_SHADER_TESS_EVALUATION_SPIRV,
                                  CT_WADBLOB_NAME_SHADER_RAY_GENERATION_SPIRV,
                                  CT_WADBLOB_NAME_SHADER_RAY_ANY_HIT_SPIRV,
                                  CT_WADBLOB_NAME_SHADER_RAY_CLOSEST_HIT_SPIRV,
                                  CT_WADBLOB_NAME_SHADER_RAY_MISS_SPIRV,
                                  CT_WADBLOB_NAME_SHADER_RAY_INTERSECTION_SPIRV,
                                  CT_WADBLOB_NAME_SHADER_CALLABLE_SPIRV,
                                  CT_WADBLOB_NAME_SHADER_TASK_SPIRV,
                                  CT_WADBLOB_NAME_SHADER_MESH_SPIRV};
   void* pData;
   int32_t size;
   ctWADFindLumpInMarker(
     pWad, fxSegment, "FX_START", "FX_END", lumpNamesByType[type], &pData, &size);
   VkShaderModuleCreateInfo info = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
   if (!pData) { return CT_FAILURE_DATA_DOES_NOT_EXIST; }
   info.codeSize = size;
   info.pCode = (uint32_t*)pData;
   VkResult result = vkCreateShaderModule(
     pDevice->vkDevice, &info, &pDevice->vkAllocCallback, (VkShaderModule*)pShaderOut);
   return result == VK_SUCCESS ? CT_SUCCESS : CT_FAILURE_RUNTIME_ERROR;
}

CT_API void ctGPUShaderSoftRelease(ctGPUDevice* pDevice, ctGPUShaderModule shader) {
   ctAssert(pDevice);
   ctAssert(shader);
   vkDestroyShaderModule(
     pDevice->vkDevice, (VkShaderModule)shader, &pDevice->vkAllocCallback);
}

CT_API ctResults ctGPUPipelineBuilderSetShader(ctGPUPipelineBuilder* pBuilder,
                                               ctGPUShaderType type,
                                               ctGPUShaderModule shader) {
   ctAssert(type >= 0 && type < CT_GPU_SHADER_COUNT);
   if (pBuilder->stageCount >= ctCStaticArrayLen(pBuilder->stages)) {
      return CT_FAILURE_OUT_OF_BOUNDS;
   }
   VkShaderStageFlagBits stagesNative[] = {VK_SHADER_STAGE_VERTEX_BIT,
                                           VK_SHADER_STAGE_FRAGMENT_BIT,
                                           VK_SHADER_STAGE_GEOMETRY_BIT,
                                           VK_SHADER_STAGE_COMPUTE_BIT,
                                           VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                           VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
                                           VK_SHADER_STAGE_RAYGEN_BIT_KHR,
                                           VK_SHADER_STAGE_ANY_HIT_BIT_KHR,
                                           VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
                                           VK_SHADER_STAGE_MISS_BIT_KHR,
                                           VK_SHADER_STAGE_INTERSECTION_BIT_KHR,
                                           VK_SHADER_STAGE_CALLABLE_BIT_KHR,
                                           VK_SHADER_STAGE_TASK_BIT_NV,
                                           VK_SHADER_STAGE_MESH_BIT_NV};
   VkPipelineShaderStageCreateInfo& stageInfo = pBuilder->stages[pBuilder->stageCount];
   stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
   stageInfo.module = (VkShaderModule)shader;
   stageInfo.pName = "main";
   stageInfo.stage = stagesNative[type];
   pBuilder->stageCount++;
   return CT_SUCCESS;
}

CT_API ctResults ctGPUPipelineBuilderSetDepthTest(ctGPUPipelineBuilder* pBuilder,
                                                  ctGPUDepthTestMode mode) {
   ctAssert(pBuilder);
   ctAssert(pBuilder->type == CT_GPU_PIPELINE_RASTER);
   VkPipelineDepthStencilStateCreateInfo& depthStencil = pBuilder->raster.depthStencil;

   VkCompareOp nativeOp[] = {VK_COMPARE_OP_GREATER,
                             VK_COMPARE_OP_LESS,
                             VK_COMPARE_OP_EQUAL,
                             VK_COMPARE_OP_ALWAYS};
   depthStencil.depthTestEnable = mode != CT_GPU_DEPTHTEST_ALWAYS ? VK_TRUE : VK_FALSE;
   depthStencil.depthCompareOp = nativeOp[mode];
   return CT_SUCCESS;
}

CT_API ctResults ctGPUPipelineBuilderSetDepthWrite(ctGPUPipelineBuilder* pBuilder,
                                                   bool enabled) {
   ctAssert(pBuilder);
   ctAssert(pBuilder->type == CT_GPU_PIPELINE_RASTER);
   VkPipelineDepthStencilStateCreateInfo& depthStencil = pBuilder->raster.depthStencil;
   depthStencil.depthWriteEnable = enabled ? VK_TRUE : VK_FALSE;
   return CT_SUCCESS;
}

CT_API ctResults ctGPUPipelineBuilderSetDepthBias(ctGPUPipelineBuilder* pBuilder,
                                                  bool enabled,
                                                  float constantFactor,
                                                  float slopeFactor,
                                                  float clamp) {
   ctAssert(pBuilder);
   ctAssert(pBuilder->type == CT_GPU_PIPELINE_RASTER);
   VkPipelineRasterizationStateCreateInfo& rasterState = pBuilder->raster.rasterState;
   rasterState.depthBiasEnable = enabled ? VK_TRUE : VK_FALSE;
   rasterState.depthBiasConstantFactor = constantFactor;
   rasterState.depthBiasSlopeFactor = slopeFactor;
   rasterState.depthBiasClamp = clamp;
   return CT_SUCCESS;
}

CT_API ctResults ctGPUPipelineBuilderSetBlendMode(ctGPUPipelineBuilder* pBuilder,
                                                  uint32_t attachmentIndex,
                                                  bool enabled,
                                                  ctColorComponents writeMask,
                                                  ctGPUBlendingMode colorMode,
                                                  ctGPUBlendingMode alphaMode) {
   ctAssert(pBuilder);
   ctAssert(pBuilder->type == CT_GPU_PIPELINE_RASTER);
   VkPipelineColorBlendStateCreateInfo& blendState = pBuilder->raster.blendState;
   if (blendState.attachmentCount <= attachmentIndex) {
      blendState.attachmentCount = attachmentIndex + 1;
   }
   VkPipelineColorBlendAttachmentState& attachment =
     pBuilder->raster.attachmentBlends[attachmentIndex];
   attachment.blendEnable = enabled ? VK_TRUE : VK_FALSE;
   switch (colorMode) {
      case CT_GPU_BLEND_ADD:
         attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
         attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
         attachment.colorBlendOp = VK_BLEND_OP_ADD;
         break;
      case CT_GPU_BLEND_SUBTRACT:
         attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
         attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
         attachment.colorBlendOp = VK_BLEND_OP_REVERSE_SUBTRACT;
         break;
      case CT_GPU_BLEND_MULTIPLY:
         attachment.srcColorBlendFactor = VK_BLEND_FACTOR_DST_COLOR;
         attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
         attachment.colorBlendOp = VK_BLEND_OP_ADD;
         break;
      case CT_GPU_BLEND_MAX:
         attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
         attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
         attachment.colorBlendOp = VK_BLEND_OP_MAX;
         break;
      case CT_GPU_BLEND_MIN:
         attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
         attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
         attachment.colorBlendOp = VK_BLEND_OP_MIN;
         break;
      case CT_GPU_BLEND_LERP:
         attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
         attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
         attachment.colorBlendOp = VK_BLEND_OP_ADD;
         break;
      case CT_GPU_BLEND_OVERWRITE:
         attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
         attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
         attachment.colorBlendOp = VK_BLEND_OP_ADD;
         break;
      case CT_GPU_BLEND_DISCARD:
         attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
         attachment.dstColorBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
         attachment.colorBlendOp = VK_BLEND_OP_ADD;
         break;
      default: return CT_FAILURE_INVALID_PARAMETER;
   }
   switch (alphaMode) {
      case CT_GPU_BLEND_ADD:
         attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
         attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
         attachment.alphaBlendOp = VK_BLEND_OP_ADD;
         break;
      case CT_GPU_BLEND_SUBTRACT:
         attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
         attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
         attachment.alphaBlendOp = VK_BLEND_OP_REVERSE_SUBTRACT;
         break;
      case CT_GPU_BLEND_MULTIPLY:
         attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_DST_COLOR;
         attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
         attachment.alphaBlendOp = VK_BLEND_OP_ADD;
         break;
      case CT_GPU_BLEND_MAX:
         attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
         attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
         attachment.alphaBlendOp = VK_BLEND_OP_MAX;
         break;
      case CT_GPU_BLEND_MIN:
         attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
         attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
         attachment.alphaBlendOp = VK_BLEND_OP_MIN;
         break;
      case CT_GPU_BLEND_LERP:
         attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
         attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
         attachment.alphaBlendOp = VK_BLEND_OP_ADD;
         break;
      case CT_GPU_BLEND_OVERWRITE:
         attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
         attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
         attachment.alphaBlendOp = VK_BLEND_OP_ADD;
         break;
      case CT_GPU_BLEND_DISCARD:
         attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
         attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
         attachment.alphaBlendOp = VK_BLEND_OP_ADD;
         break;
      default: return CT_FAILURE_INVALID_PARAMETER;
   }
   return CT_SUCCESS;
}

CT_API ctResults ctGPUPipelineBuilderSetWinding(ctGPUPipelineBuilder* pBuilder,
                                                ctGPUWinding winding) {
   ctAssert(pBuilder);
   ctAssert(pBuilder->type == CT_GPU_PIPELINE_RASTER);
   VkPipelineRasterizationStateCreateInfo& rasterState = pBuilder->raster.rasterState;
   rasterState.frontFace = winding == CT_GPU_WIND_CLOCKWISE
                             ? VK_FRONT_FACE_CLOCKWISE
                             : VK_FRONT_FACE_COUNTER_CLOCKWISE;
   return CT_SUCCESS;
}

CT_API ctResults ctGPUPipelineBuilderSetFillMode(ctGPUPipelineBuilder* pBuilder,
                                                 ctGPUFillMode fill) {
   ctAssert(pBuilder);
   ctAssert(pBuilder->type == CT_GPU_PIPELINE_RASTER);
   VkPipelineRasterizationStateCreateInfo& rasterState = pBuilder->raster.rasterState;
   VkPolygonMode nativePolygonMode[] = {
     VK_POLYGON_MODE_FILL, VK_POLYGON_MODE_LINE, VK_POLYGON_MODE_POINT};
   rasterState.polygonMode = nativePolygonMode[fill];
   return CT_SUCCESS;
}

CT_API ctResults ctGPUPipelineBuilderSetFaceCull(ctGPUPipelineBuilder* pBuilder,
                                                 ctGPUFaceMask cull) {
   ctAssert(pBuilder);
   ctAssert(pBuilder->type == CT_GPU_PIPELINE_RASTER);
   VkPipelineRasterizationStateCreateInfo& rasterState = pBuilder->raster.rasterState;
   VkCullModeFlagBits nativeCull[] = {VK_CULL_MODE_FRONT_BIT,
                                      VK_CULL_MODE_BACK_BIT,
                                      VK_CULL_MODE_NONE,
                                      VK_CULL_MODE_FRONT_AND_BACK};
   rasterState.cullMode = nativeCull[cull];
   return CT_SUCCESS;
}

CT_API ctResults ctGPUPipelineBuilderSetTopology(ctGPUPipelineBuilder* pBuilder,
                                                 ctGPUTopology topology,
                                                 bool primativeRestart) {
   ctAssert(pBuilder);
   ctAssert(pBuilder->type == CT_GPU_PIPELINE_RASTER);
   VkPipelineInputAssemblyStateCreateInfo& inputAssembly = pBuilder->raster.inputAssembly;
   VkPrimitiveTopology nativeTopology[] = {VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                                           VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
                                           VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
                                           VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
                                           VK_PRIMITIVE_TOPOLOGY_LINE_STRIP};
   inputAssembly.topology = nativeTopology[topology];
   inputAssembly.primitiveRestartEnable = primativeRestart ? VK_TRUE : VK_FALSE;
   return CT_SUCCESS;
}

CT_API ctResults ctGPUPipelineBuilderSetTessPoints(ctGPUPipelineBuilder* pBuilder,
                                                   int32_t points) {
   ctAssert(pBuilder);
   ctAssert(pBuilder->type == CT_GPU_PIPELINE_RASTER);
   VkPipelineTessellationStateCreateInfo& tessellation = pBuilder->raster.tessellation;
   tessellation.patchControlPoints = points;
   return CT_SUCCESS;
}

CT_API ctResults ctGPUPipelineBuilderSetMSAA(ctGPUPipelineBuilder* pBuilder,
                                             ctGPUSampleCounts samples,
                                             bool sampleShading,
                                             bool alphaToCoverage,
                                             bool alphaToOne) {
   ctAssert(pBuilder);
   ctAssert(pBuilder->type == CT_GPU_PIPELINE_RASTER);
   VkPipelineMultisampleStateCreateInfo& msaa = pBuilder->raster.msaa;
   msaa.rasterizationSamples = (VkSampleCountFlagBits)samples;
   msaa.sampleShadingEnable = sampleShading ? VK_TRUE : VK_FALSE;
   msaa.alphaToCoverageEnable = alphaToCoverage ? VK_TRUE : VK_FALSE;
   msaa.alphaToOneEnable = alphaToOne ? VK_TRUE : VK_FALSE;
   return CT_SUCCESS;
}

CT_API ctResults ctGPUPipelineBuilderSetStencil(ctGPUPipelineBuilder* pBuilder,
                                                bool enable,
                                                ctGPUFaceMask face,
                                                ctGPUStencilOps failOp,
                                                ctGPUStencilOps passOp,
                                                ctGPUStencilOps depthFailOp,
                                                ctGPUStencilTest testMode,
                                                uint32_t compareMask,
                                                uint32_t writeMask,
                                                uint32_t referenceValue) {
   ctAssert(pBuilder);
   ctAssert(pBuilder->type == CT_GPU_PIPELINE_RASTER);
   VkPipelineDepthStencilStateCreateInfo& depthStencil = pBuilder->raster.depthStencil;
   depthStencil.stencilTestEnable = enable;
   VkStencilOpState state = {};
   state.failOp = (VkStencilOp)failOp;
   state.passOp = (VkStencilOp)passOp;
   state.depthFailOp = (VkStencilOp)depthFailOp;
   state.compareOp = (VkCompareOp)testMode;
   state.compareMask = compareMask;
   state.writeMask = writeMask;
   state.reference = referenceValue;

   if (ctCFlagCheck(face, CT_GPU_FACE_FRONT)) { depthStencil.back = state; }
   if (ctCFlagCheck(face, CT_GPU_FACE_BACK)) { depthStencil.back = state; }
   return CT_SUCCESS;
}

CT_API ctResults ctGPUPipelineBuilderEnableDynamicState(ctGPUPipelineBuilder* pBuilder,
                                                        ctGPUDynamicState state) {
   ctAssert(pBuilder);
   ctAssert(pBuilder->type == CT_GPU_PIPELINE_RASTER);
   VkPipelineDynamicStateCreateInfo& dynamicState = pBuilder->raster.dynamicState;
   if (dynamicState.dynamicStateCount >= ctCStaticArrayLen(pBuilder->raster.dynamics)) {
      return CT_FAILURE_OUT_OF_BOUNDS;
   }
   VkDynamicState nativeDynamicState[] = {VK_DYNAMIC_STATE_VIEWPORT,
                                          VK_DYNAMIC_STATE_SCISSOR,
                                          VK_DYNAMIC_STATE_LINE_WIDTH,
                                          VK_DYNAMIC_STATE_DEPTH_BIAS,
                                          VK_DYNAMIC_STATE_BLEND_CONSTANTS,
                                          VK_DYNAMIC_STATE_DEPTH_BOUNDS,
                                          VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK,
                                          VK_DYNAMIC_STATE_STENCIL_WRITE_MASK,
                                          VK_DYNAMIC_STATE_STENCIL_REFERENCE};
   pBuilder->raster.dynamics[dynamicState.dynamicStateCount] = nativeDynamicState[state];
   dynamicState.dynamicStateCount++;
   return CT_SUCCESS;
}

CT_API ctResults ctGPUPipelineBuilderSetAttachments(ctGPUPipelineBuilder* pBuilder,
                                                    TinyImageFormat depthFormat,
                                                    uint32_t colorCount,
                                                    TinyImageFormat* colorFormats) {
   ctAssert(pBuilder);
   ctAssert(pBuilder->type == CT_GPU_PIPELINE_RASTER);
   ctAssert(colorFormats);
   VkPipelineRenderingCreateInfoKHR& renderInfo = pBuilder->raster.dynamicRendering;
   renderInfo.colorAttachmentCount = colorCount;
   renderInfo.viewMask = 0;
   VkFormat vDepthFormat = (VkFormat)(TinyImageFormat_ToVkFormat(depthFormat));
   if (TinyImageFormat_IsDepthOnly(depthFormat)) {
      renderInfo.depthAttachmentFormat = vDepthFormat;
      renderInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;
   } else if (TinyImageFormat_IsStencilOnly(depthFormat)) {
      renderInfo.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
      renderInfo.stencilAttachmentFormat = vDepthFormat;
   } else {
      renderInfo.depthAttachmentFormat = vDepthFormat;
      renderInfo.stencilAttachmentFormat = vDepthFormat;
   }
   for (uint32_t i = 0; i < colorCount; i++) {
      pBuilder->raster.colorFormats[i] =
        (VkFormat)(TinyImageFormat_ToVkFormat(colorFormats[i]));
   }
   return CT_API CT_SUCCESS;
}

CT_API ctResults ctGPUPipelineCreate(ctGPUDevice* pDevice,
                                     ctGPUPipelineBuilder* pBuilder,
                                     ctGPUPipeline* pPipeline) {
   ctAssert(pDevice);
   ctAssert(pBuilder);
   if (pBuilder->type == CT_GPU_PIPELINE_RASTER) {
      pBuilder->raster.createInfo.stageCount = pBuilder->stageCount;
      pBuilder->raster.createInfo.layout = pDevice->vkGlobalPipelineLayout;
      if (vkCreateGraphicsPipelines(pDevice->vkDevice,
                                    pDevice->vkPipelineCache,
                                    1,
                                    &pBuilder->raster.createInfo,
                                    &pDevice->vkAllocCallback,
                                    (VkPipeline*)pPipeline) != VK_SUCCESS) {
         return CT_FAILURE_RUNTIME_ERROR;
      }
      return CT_SUCCESS;
   }
   return CT_FAILURE_UNKNOWN;
}

CT_API void ctGPUPipelineDestroy(ctGPUDevice* pDevice, ctGPUPipeline pipeline) {
   ctAssert(pDevice);
   ctAssert(pipeline);
   vkDestroyPipeline(pDevice->vkDevice, (VkPipeline)pipeline, &pDevice->vkAllocCallback);
}