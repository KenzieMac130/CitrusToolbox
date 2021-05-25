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

class CT_API ctVkIm3d {
public:
   ctResults Startup(ctVkBackend* pBackend,
                     VkCommandBuffer textureUploadCmd,
                     VkRenderPass guiRenderpass,
                     uint32_t subpass);
   ctResults Shutdown();
   void BuildDrawLists();
   void SetDisplaySize(int32_t windowWidth,
                       int32_t windowHeight,
                       int32_t internalWidth,
                       int32_t internalHeight);
   void RenderCommands(VkCommandBuffer cmd);

private:
   ctVkBackend* _pBackend;
};