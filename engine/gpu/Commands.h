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

#pragma once

#include "utilities/Common.h"
#include "tiny_imageFormat/tinyimageformat.h"
#include "Pipeline.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void* ctGPUCommandBuffer;

/* ------------------ Drawing Commands ------------------ */

CT_API void ctGPUCmdDraw(ctGPUCommandBuffer commandBuffer,
                         uint32_t vertexCount,
                         uint32_t instanceCount,
                         uint32_t firstVertex,
                         uint32_t firstInstance);
CT_API void ctGPUCmdDrawIndexed(ctGPUCommandBuffer commandBuffer,
                                uint32_t indexCount,
                                uint32_t instanceCount,
                                uint32_t firstIndex,
                                int32_t vertexOffset,
                                uint32_t firstInstance);
CT_API void ctGPUCmdDrawIndirect(ctGPUCommandBuffer commandBuffer,
                                 ctGPUBufferAccessor buffer,
                                 size_t bufferOffset,
                                 uint32_t drawCount,
                                 uint32_t stride);
CT_API void ctGPUCmdDrawIndirectRobust(ctGPUCommandBuffer commandBuffer,
                                       ctGPUBufferAccessor puffer,
                                       size_t bufferOffset,
                                       ctGPUBufferAccessor countBuffer,
                                       size_t countBufferOffset,
                                       uint32_t maxDrawCount,
                                       uint32_t stride);

/* ------------------ Compute Commands ------------------ */

CT_API void ctGPUCmdDispatch(ctGPUCommandBuffer commandBuffer,
                             uint32_t groupCountX,
                             uint32_t groupCountY,
                             uint32_t groupCountZ);

/* ------------------ Transfer Commands ------------------ */

struct ctGPUBlitRegion {
   uint32_t sourceOffset[3];
   uint32_t sourceSize[3];
   uint32_t sourceMipLevel;
   uint32_t sourceLayerBase;
   uint32_t sourceLayerCount;
   bool isSourceDepth;

   uint32_t destOffset[3];
   uint32_t destSize[3];
   uint32_t destMipLevel;
   uint32_t destLayerBase;
   uint32_t destLayerCount;
   bool isDestDepth;
};

CT_API void ctGPUCmdBlit(ctGPUCommandBuffer commandBuffer,
                         ctGPUImageAccessor source,
                         ctGPUImageAccessor dest,
                         bool filter,
                         uint32_t regionCount,
                         struct ctGPUBlitRegion* pRegions);

/* ------------------ Draw State Commands ------------------ */

/* Viewport and Scissor */

CT_API void ctGPUCmdSetViewport(ctGPUCommandBuffer commandBuffer,
                                float offsetX,
                                float offsetY,
                                float width,
                                float height,
                                float beginDepth,
                                float endDepth);
CT_API void ctGPUCmdSetScissor(ctGPUCommandBuffer commandBuffer,
                               uint32_t offsetX,
                               uint32_t offsetY,
                               uint32_t width,
                               uint32_t height);

/* Depth settings */

CT_API void ctGPUCmdSetDepthBias(ctGPUCommandBuffer commandBuffer,
                                 float constantFactor,
                                 float slopeFactor,
                                 float clamp);
CT_API void ctGPUCmdSetDepthBounds(ctGPUCommandBuffer commandBuffer,
                                   float beginDepth,
                                   float endDepth);

/* Stencil Settings */

CT_API void ctGPUCmdSetStencilCompare(ctGPUCommandBuffer commandBuffer,
                                      enum ctGPUFaceMask faceMask,
                                      uint32_t value);
CT_API void ctGPUCmdSetStencilReference(ctGPUCommandBuffer commandBuffer,
                                        enum ctGPUFaceMask faceMask,
                                        uint32_t value);
CT_API void ctGPUCmdSetStencilWrite(ctGPUCommandBuffer commandBuffer,
                                    enum ctGPUFaceMask faceMask,
                                    uint32_t value);

/* Misc. Settings */

CT_API void ctGPUCmdSetLineWidth(ctGPUCommandBuffer commandBuffer, float width);

/* ------------------ Input Data ------------------ */

CT_API void ctGPUCmdSetDynamicInteger(ctGPUCommandBuffer commandBuffer,
                                      ctGPUBindingModel* pBindingModel,
                                      uint32_t index,
                                      int32_t value);
CT_API void ctGPUCmdSetGraphicsPipeline(ctGPUCommandBuffer commandBuffer,
                                        ctGPUPipeline pipeline);
CT_API void ctGPUCmdSetComputePipeline(ctGPUCommandBuffer commandBuffer,
                                       ctGPUPipeline pipeline);

enum ctGPUIndexType { CT_GPU_INDEX_TYPE_UINT16, CT_GPU_INDEX_TYPE_UINT32 };
CT_API void ctGPUCmdSetIndexBuffer(ctGPUCommandBuffer commandBuffer,
                                   ctGPUBufferAccessor indexBuffer,
                                   size_t offset,
                                   ctGPUIndexType type);
CT_API void ctGPUCmdSetVertexBuffers(ctGPUCommandBuffer commandBuffer,
                                     size_t vertexBufferCount,
                                     ctGPUBufferAccessor* pVertexBuffers,
                                     size_t* pOffsets);

#ifdef __cplusplus
}
#endif