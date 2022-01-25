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

#include "CommandsVulkan.hpp"

#include "vulkan/vulkan.h"
#include "..\Commands.h"

/* The intention of the abstraction is to be thin to promote inlining */

CT_API void ctGPUCmdDraw(ctGPUCommandBuffer commandBuffer,
                         uint32_t vertexCount,
                         uint32_t instanceCount,
                         uint32_t firstVertex,
                         uint32_t firstInstance) {
   vkCmdDraw((VkCommandBuffer)commandBuffer,
             vertexCount,
             instanceCount,
             firstVertex,
             firstInstance);
}

CT_API void ctGPUCmdDrawIndirect(ctGPUCommandBuffer commandBuffer,
                                 ctGPUBufferAccessor buffer,
                                 size_t bufferOffset,
                                 uint32_t drawCount,
                                 uint32_t stride) {
   vkCmdDrawIndirect(
     (VkCommandBuffer)commandBuffer, (VkBuffer)buffer, bufferOffset, drawCount, stride);
}

CT_API void ctGPUCmdDrawIndirectRobust(ctGPUCommandBuffer commandBuffer,
                                       ctGPUBufferAccessor buffer,
                                       size_t bufferOffset,
                                       ctGPUBufferAccessor countBuffer,
                                       size_t countBufferOffset,
                                       uint32_t maxDrawCount,
                                       uint32_t stride) {
   vkCmdDrawIndirectCount((VkCommandBuffer)commandBuffer,
                          (VkBuffer)buffer,
                          bufferOffset,
                          (VkBuffer)countBuffer,
                          countBufferOffset,
                          maxDrawCount,
                          stride);
}

CT_API void ctGPUCmdDispatch(ctGPUCommandBuffer commandBuffer,
                             uint32_t groupCountX,
                             uint32_t groupCountY,
                             uint32_t groupCountZ) {
   vkCmdDispatch((VkCommandBuffer)commandBuffer, groupCountX, groupCountY, groupCountZ);
}

CT_API void ctGPUCmdBlit(ctGPUCommandBuffer commandBuffer,
                         ctGPUImageAccessor source,
                         ctGPUImageAccessor dest,
                         bool filter,
                         uint32_t regionCount,
                         ctGPUBlitRegion* pRegions) {
   ctAssert(regionCount <= 64);
   VkImageBlit* pVkRegions =
     (VkImageBlit*)ctStackAlloc(regionCount * sizeof(VkImageBlit));
   ctAssert(pVkRegions);
   for (uint32_t i = 0; i < regionCount; i++) {
      // clang-format off
      pVkRegions[i].srcOffsets[0].x = (int32_t)pRegions[i].sourceOffset[0];
      pVkRegions[i].srcOffsets[0].y = (int32_t)pRegions[i].sourceOffset[1];
      pVkRegions[i].srcOffsets[0].z = (int32_t)pRegions[i].sourceOffset[2];

      pVkRegions[i].srcOffsets[1].x = (int32_t)pRegions[i].sourceSize[0];
      pVkRegions[i].srcOffsets[1].y = (int32_t)pRegions[i].sourceSize[1];
      pVkRegions[i].srcOffsets[1].z = (int32_t)pRegions[i].sourceSize[2];

      pVkRegions[i].dstOffsets[0].x = (int32_t)pRegions[i].destOffset[0];
      pVkRegions[i].dstOffsets[0].y = (int32_t)pRegions[i].destOffset[1];
      pVkRegions[i].dstOffsets[0].z = (int32_t)pRegions[i].destOffset[2];

      pVkRegions[i].dstOffsets[1].x = (int32_t)pRegions[i].destSize[0];
      pVkRegions[i].dstOffsets[1].y = (int32_t)pRegions[i].destSize[1];
      pVkRegions[i].dstOffsets[1].z = (int32_t)pRegions[i].destSize[2];

      pVkRegions[i].srcSubresource.aspectMask = pRegions[i].isSourceDepth ? 
          VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
      pVkRegions[i].dstSubresource.aspectMask = pRegions[i].isDestDepth ?
          VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

      pVkRegions[i].srcSubresource.baseArrayLayer = pRegions[i].sourceLayerBase;
      pVkRegions[i].dstSubresource.baseArrayLayer = pRegions[i].destLayerBase;

      pVkRegions[i].srcSubresource.layerCount = pRegions[i].sourceLayerCount;
      pVkRegions[i].dstSubresource.layerCount = pRegions[i].destLayerCount;

      pVkRegions[i].srcSubresource.mipLevel = pRegions[i].sourceMipLevel;
      pVkRegions[i].dstSubresource.mipLevel = pRegions[i].destMipLevel;
      // clang-format on
   }
   vkCmdBlitImage((VkCommandBuffer)commandBuffer,
                  // pSource->image,
                  // pSource->layout,
                  // pDest->image,
                  // pDest->layout,
                  VK_NULL_HANDLE,
                  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                  VK_NULL_HANDLE,
                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                  regionCount,
                  pVkRegions,
                  filter ? VK_FILTER_NEAREST : VK_FILTER_LINEAR);
}

CT_API void ctGPUCmdSetViewport(ctGPUCommandBuffer commandBuffer,
                                float offsetX,
                                float offsetY,
                                float width,
                                float height,
                                float beginDepth,
                                float endDepth) {
   VkViewport viewport;
   viewport.x = offsetX;
   viewport.y = offsetY;
   viewport.width = width;
   viewport.height = height;
   viewport.minDepth = beginDepth;
   viewport.maxDepth = endDepth;
   vkCmdSetViewport((VkCommandBuffer)commandBuffer, 0, 1, &viewport);
}

CT_API void ctGPUCmdSetScissor(ctGPUCommandBuffer commandBuffer,
                               uint32_t offsetX,
                               uint32_t offsetY,
                               uint32_t width,
                               uint32_t height) {
   VkRect2D scissor;
   scissor.extent.width = width;
   scissor.extent.height = height;
   scissor.offset.x = offsetX;
   scissor.offset.y = offsetY;
   vkCmdSetScissor((VkCommandBuffer)commandBuffer, 0, 1, &scissor);
}

CT_API void ctGPUCmdSetDepthBias(ctGPUCommandBuffer commandBuffer,
                                 float constantFactor,
                                 float slopeFactor,
                                 float clamp) {
   vkCmdSetDepthBias((VkCommandBuffer)commandBuffer, constantFactor, clamp, slopeFactor);
}

CT_API void ctGPUCmdSetDepthBounds(ctGPUCommandBuffer commandBuffer,
                                   float beginDepth,
                                   float endDepth) {
   vkCmdSetDepthBounds((VkCommandBuffer)commandBuffer, beginDepth, endDepth);
}

CT_API void ctGPUCmdSetStencilCompare(ctGPUCommandBuffer commandBuffer,
                                      ctGPUFaceMask faceMask,
                                      uint32_t value) {
   vkCmdSetStencilCompareMask(
     (VkCommandBuffer)commandBuffer, (VkStencilFaceFlagBits)faceMask, value);
}

CT_API void ctGPUCmdSetStencilReference(ctGPUCommandBuffer commandBuffer,
                                        ctGPUFaceMask faceMask,
                                        uint32_t value) {
   vkCmdSetStencilReference(
     (VkCommandBuffer)commandBuffer, (VkStencilFaceFlagBits)faceMask, value);
}

CT_API void ctGPUCmdSetStencilWrite(ctGPUCommandBuffer commandBuffer,
                                    ctGPUFaceMask faceMask,
                                    uint32_t value) {
   vkCmdSetStencilWriteMask(
     (VkCommandBuffer)commandBuffer, (VkStencilFaceFlagBits)faceMask, value);
}

CT_API void ctGPUCmdSetLineWidth(ctGPUCommandBuffer commandBuffer, float width) {
   vkCmdSetLineWidth((VkCommandBuffer)commandBuffer, width);
}

#include "DeviceVulkan.hpp"
CT_API void ctGPUCmdSetDynamicInteger(ctGPUCommandBuffer commandBuffer,
                                      ctGPUDevice* pDevice,
                                      uint32_t index,
                                      int32_t value) {
   ctAssert(pDevice);
   ctAssert(index < CT_MAX_GFX_DYNAMIC_INTS);
   vkCmdPushConstants((VkCommandBuffer)commandBuffer,
                      pDevice->vkGlobalPipelineLayout,
                      VK_SHADER_STAGE_ALL,
                      sizeof(int32_t) * index,
                      sizeof(int32_t),
                      &value);
}

CT_API void ctGPUCmdSetGraphicsPipeline(ctGPUCommandBuffer commandBuffer,
                                        ctGPUPipeline pipeline) {
   vkCmdBindPipeline((VkCommandBuffer)commandBuffer,
                     VK_PIPELINE_BIND_POINT_GRAPHICS,
                     (VkPipeline)pipeline);
}

CT_API void ctGPUCmdSetComputePipeline(ctGPUCommandBuffer commandBuffer,
                                       ctGPUPipeline pipeline) {
   vkCmdBindPipeline((VkCommandBuffer)commandBuffer,
                     VK_PIPELINE_BIND_POINT_COMPUTE,
                     (VkPipeline)pipeline);
}