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

#include "renderer/KeyLime.hpp"
#include "utilities/Common.h"
#include "VkBackend.hpp"
#include "VkImgui.hpp"
#include "VkIm3d.hpp"

#if CITRUS_INCLUDE_AUDITION
#include "audition/HotReloadDetection.hpp"
#endif

struct ctVkKeyLimeGlobalBufferData {
   float debug;
   float time;
   float deltaTime;
};

struct ctVkKeyLimeViewBufferData {
   ctMat4 viewMatrix;
   ctMat4 inverseViewMatrix;
   ctMat4 projectionMatrix;
   ctMat4 inverseProjectionMatrix;
};

class CT_API ctVkKeyLimeCore : public ctModuleBase {
public:
   ctResults Startup() final;
   ctResults Shutdown() final;

   ctResults CreateScreenResources();
   ctResults DestroyScreenResources();

   ctResults UpdateCamera(ctKeyLimeCameraDesc cameraDesc);
   ctResults Render();

   VkFormat compositeFormat;
   VkFormat depthFormat;
   ctVkCompleteImage compositeBuffer;
   ctVkCompleteImage depthBuffer;

   uint32_t internalResolutionWidth;
   uint32_t internalResolutionHeight;

   ctVkBackend vkBackend;
   ctVkImgui vkImgui;
   ctVkIm3d vkIm3d;

#if CITRUS_INCLUDE_AUDITION
   ctHotReloadCategory ShaderHotReload;
#endif

   VkCommandPool gfxCommandPool;
   VkCommandPool transferCommandPool;
   VkCommandBuffer gfxCommandBuffers[CT_MAX_INFLIGHT_FRAMES];
   VkCommandBuffer frameInitialUploadCommandBuffers[CT_MAX_INFLIGHT_FRAMES];
   VkSemaphore renderFinished[CT_MAX_INFLIGHT_FRAMES];

   VkRenderPass forwardRenderPass;
   VkFramebuffer forwardFramebuffer;

   /* Mapped buffer data, offsets into global upload */
   ctVkCompleteBuffer frameDataUploadBuffer[CT_MAX_INFLIGHT_FRAMES];
   void* frameDataUploadBaseMappings[CT_MAX_INFLIGHT_FRAMES];

   ctVkKeyLimeGlobalBufferData* pGlobalBufferData;

   size_t viewBufferCount = 1;
   ctVkKeyLimeViewBufferData* pViewBufferData;
};