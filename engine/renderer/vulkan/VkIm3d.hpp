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
#include "VkBackend.hpp"
#include "im3d/im3d.h"
#include "core/FileSystem.hpp"

class CT_API ctVkIm3d {
public:
   ctResults Startup(ctFileSystem& fileSystem,
                     ctVkBackend* pBackend,
                     VkRenderPass guiRenderpass,
                     uint32_t subpass);
   ctResults Shutdown();
   ctResults
   LoadShaders(ctFileSystem& fileSystem, VkRenderPass guiRenderpass, uint32_t subpass);
   void BuildDrawLists();
   void
   RenderCommands(VkCommandBuffer cmd, ctVec2 viewSize, ctMat4 view, ctMat4 projection);

private:
   Im3d::VertexData* vertexData[CT_MAX_INFLIGHT_FRAMES];
   ctVkCompleteBuffer vertexBuffer[CT_MAX_INFLIGHT_FRAMES];
   uint32_t vertexBuffBindIdx[CT_MAX_INFLIGHT_FRAMES];

   VkPipelineLayout pipelineLayout;
   VkPipeline pipeline;
   ctVkBackend* _pBackend;
};