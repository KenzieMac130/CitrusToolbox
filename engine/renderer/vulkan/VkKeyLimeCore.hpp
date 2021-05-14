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
#include "VkImgui.hpp"

class ctVkKeyLimeCore : public ctModuleBase {
public:
    ctResults Startup() final;
    ctResults Shutdown() final;

    ctResults Render();

    VkFormat compositeFormat;
    VkFormat depthFormat;
    ctVkCompleteImage compositeBuffer;
    ctVkCompleteImage depthBuffer;

    uint32_t internalResolutionWidth;
    uint32_t internalResolutionHeight;

    ctVkBackend vkBackend;
    ctVkImgui vkImgui;

    VkCommandPool gfxCommandPool;
    VkCommandPool transferCommandPool;
    VkCommandBuffer gfxCommandBuffers[CT_MAX_INFLIGHT_FRAMES];
    VkCommandBuffer frameInitialUploadCommandBuffers[CT_MAX_INFLIGHT_FRAMES];
    VkSemaphore renderFinished[CT_MAX_INFLIGHT_FRAMES];

    VkRenderPass guiRenderPass;
    VkFramebuffer guiFramebuffer;
};