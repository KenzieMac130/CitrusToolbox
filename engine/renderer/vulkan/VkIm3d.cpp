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

#include "VkBackend.hpp"
#include "VkIm3d.hpp"

#define CT_MAX_IM3D_VERTS 300000

struct vkim3dPushConstant {
   ctMat4 viewProj;
   ctVec2 viewSize;
   uint32_t vertBuffBindIdx;
   uint32_t primType;
};

ctResults ctVkIm3d::Startup(ctFileSystem& fileSystem,
                            ctVkBackend* pBackend,
                            VkRenderPass guiRenderpass,
                            uint32_t subpass) {
   ZoneScoped;
   _pBackend = pBackend;
   for (int i = 0; i < CT_MAX_INFLIGHT_FRAMES; i++) {
      pBackend->CreateCompleteBuffer(vertexBuffer[i],
                                     VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                     0,
                                     sizeof(Im3d::VertexData) * CT_MAX_IM3D_VERTS,
                                     VMA_MEMORY_USAGE_CPU_TO_GPU);
      vmaMapMemory(pBackend->vmaAllocator, vertexBuffer[i].alloc, (void**)&vertexData[i]);
      _pBackend->ExposeBindlessStorageBuffer(vertexBuffBindIdx[i],
                                             vertexBuffer[i].buffer);
   }
   VkPushConstantRange pushConstRange = {0};
   pushConstRange.offset = 0;
   pushConstRange.size = sizeof(vkim3dPushConstant);
   pushConstRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
   _pBackend->CreateBindlessPipelineLayout(pipelineLayout, 1, &pushConstRange);
   LoadShaders(fileSystem, guiRenderpass, subpass);
   return CT_SUCCESS;
}

ctResults ctVkIm3d::Shutdown() {
   ZoneScoped;
   for (int i = 0; i < CT_MAX_INFLIGHT_FRAMES; i++) {
      _pBackend->ReleaseBindlessStorageBuffer(vertexBuffBindIdx[i]);
      vmaUnmapMemory(_pBackend->vmaAllocator, vertexBuffer[i].alloc);
      _pBackend->TryDestroyCompleteBuffer(vertexBuffer[i]);
   }
   vkDestroyPipelineLayout(
     _pBackend->vkDevice, pipelineLayout, &_pBackend->vkAllocCallback);
   if (pipeline != VK_NULL_HANDLE) {
      vkDestroyPipeline(_pBackend->vkDevice, pipeline, &_pBackend->vkAllocCallback);
   }
   return CT_SUCCESS;
}

#include "formats/wad/WADCore.h"
#include "formats/wad/prototypes/MarkersAndBlobs.h"
ctResults ctVkIm3d::LoadShaders(ctFileSystem& fileSystem,
                                VkRenderPass guiRenderpass,
                                uint32_t subpass) {
   ZoneScoped;
   VkShaderModule vertShader;
   VkShaderModule fragShader;

   ctFile file;
   fileSystem.OpenAssetFileNamed(file, "shaders/im3d.wad");
   ctDynamicArray<uint8_t> bytes;
   file.GetBytes(bytes);
   ctWADReader wad;
   ctWADReaderBind(&wad, bytes.Data(), bytes.Count());
   _pBackend->CreateShaderModuleFromWad(
     vertShader, wad, 0, CT_WADBLOB_NAME_SHADER_VERT_SPIRV);
   _pBackend->CreateShaderModuleFromWad(
     fragShader, wad, 0, CT_WADBLOB_NAME_SHADER_FRAG_SPIRV);

   if (pipeline != VK_NULL_HANDLE) {
      vkDestroyPipeline(_pBackend->vkDevice, pipeline, &_pBackend->vkAllocCallback);
   }
   _pBackend->CreateGraphicsPipeline(pipeline,
                                     pipelineLayout,
                                     guiRenderpass,
                                     0,
                                     vertShader,
                                     fragShader,
                                     VK_NULL_HANDLE,
                                     VK_NULL_HANDLE,
                                     VK_NULL_HANDLE,
                                     VK_NULL_HANDLE,
                                     true,
                                     true,
                                     true,
                                     1,
                                     NULL,
                                     VK_FRONT_FACE_CLOCKWISE,
                                     VK_CULL_MODE_NONE);

   vkDestroyShaderModule(_pBackend->vkDevice, vertShader, &_pBackend->vkAllocCallback);
   vkDestroyShaderModule(_pBackend->vkDevice, fragShader, &_pBackend->vkAllocCallback);

   return CT_SUCCESS;
}

void ctVkIm3d::BuildDrawLists() {
   ZoneScoped;
   const int currentFrame = _pBackend->currentFrame;
   Im3d::EndFrame();
   uint32_t drawListCount = Im3d::GetDrawListCount();
   const Im3d::DrawList* drawLists = Im3d::GetDrawLists();
   size_t nextVertex = 0;
   for (uint32_t i = 0; i < drawListCount; i++) {
      const uint32_t vertexCount = drawLists[i].m_vertexCount;
      if (nextVertex + vertexCount > CT_MAX_IM3D_VERTS) {
         ctDebugError("Debug draw vertex budget blown");
         break;
      }
      memcpy(vertexData[currentFrame] + nextVertex,
             drawLists[i].m_vertexData,
             vertexCount * sizeof(Im3d::VertexData));
      nextVertex += vertexCount;
   }
   if (nextVertex != 0) {
      vmaFlushAllocation(
        _pBackend->vmaAllocator, vertexBuffer[currentFrame].alloc, 0, VK_WHOLE_SIZE);
   }
}

void ctVkIm3d::RenderCommands(VkCommandBuffer cmd,
                              ctVec2 viewSize,
                              ctMat4 view,
                              ctMat4 projection) {
   ZoneScoped;
   uint32_t drawListCount = Im3d::GetDrawListCount();
   const Im3d::DrawList* drawLists = Im3d::GetDrawLists();
   if (!drawListCount) { return; }

   vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
   vkCmdBindDescriptorSets(cmd,
                           VK_PIPELINE_BIND_POINT_GRAPHICS,
                           pipelineLayout,
                           0,
                           1,
                           &_pBackend->vkGlobalDescriptorSet,
                           0,
                           NULL);

   vkim3dPushConstant pushConstant = {0};
   pushConstant.vertBuffBindIdx = vertexBuffBindIdx[_pBackend->currentFrame];
   pushConstant.viewProj = projection * view;
   pushConstant.viewSize = viewSize;
   uint32_t nextVerts = 0;
   for (uint32_t i = 0; i < drawListCount; i++) {
      const Im3d::DrawList drawList = drawLists[i];
      pushConstant.primType = drawList.m_primType;
      vkCmdPushConstants(cmd,
                         pipelineLayout,
                         VK_SHADER_STAGE_VERTEX_BIT,
                         0,
                         sizeof(pushConstant),
                         &pushConstant);
      /* Draw Tris */
      if (drawList.m_primType == Im3d::DrawPrimitive_Triangles) {
         vkCmdDraw(cmd, drawList.m_vertexCount, 1, nextVerts, 0);
         nextVerts += drawList.m_vertexCount;
      }
      /* Draw Lines */
      else if (drawList.m_primType == Im3d::DrawPrimitive_Lines) {
         vkCmdDraw(cmd, 6, drawList.m_vertexCount / 2, 0, nextVerts);
         nextVerts += drawList.m_vertexCount;
      }
      /* Draw Points */
      else if (drawList.m_primType == Im3d::DrawPrimitive_Points) {
         vkCmdDraw(cmd, 6, drawList.m_vertexCount, 0, nextVerts);
         nextVerts += drawList.m_vertexCount;
      }
   }
}
