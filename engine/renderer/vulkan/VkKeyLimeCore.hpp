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

struct ctVkKeyLimeTexture {
   ctVkCompleteImage image;
};

struct ctVkKeyLimeGeometry {
   ctVkCompleteBuffer buffer;
   uint32_t streamSubmeshIdx;
   uint32_t streamIndexIdx;
   uint32_t streamPositionIdx;
   uint32_t streamTangentNormalIdx;
   uint32_t streamSkinIdx;
   uint32_t streamUVIdx[4];
   uint32_t streamColorIdx[4];
};

class CT_API ctVkKeyLimeCore : public ctModuleBase {
public:
   ctResults Startup() final;
   ctResults Shutdown() final;

   ctResults CreateGeometry(ctHandle* pHandleOut, const ctKeyLimeCreateGeometryDesc& desc);
   ctResults UpdateGeometry(ctHandle handle);
   ctResults DestroyGeometry(ctHandle handle);

   ctResults CreateMaterial(ctHandle* pHandleOut, const ctKeyLimeMaterialDesc& desc);
   ctResults UpdateMaterial(ctHandle handle, const ctKeyLimeMaterialDesc& desc);
   ctResults DestroyMaterial(ctHandle handle);

   ctResults CreateTransforms(ctHandle* pHandleOut, const ctKeyLimeTransformsDesc& pDesc);
   ctResults UpdateTransforms(ctHandle handle, const ctKeyLimeTransformsDesc& desc);
   ctResults DestroyTransforms(ctHandle handle);

   ctResults CreateGeoInstance(ctHandle* pHandleOut, const ctKeyLimeInstanceDesc& desc);
   ctResults UpdateGeoInstance(ctHandle handle, const ctKeyLimeInstanceDesc& desc);
   ctResults DestroyGeoInstance(ctHandle handle);

   ctResults LoadTextureKtx(ctHandle* pHandleOut, const char* resolvedPath);
   ctResults DestroyTexture(ctHandle handle);

   ctResults CreateScreenResources();
   ctResults DestroyScreenResources();

   ctResults UpdateCamera(const ctKeyLimeCameraDesc cameraDesc);
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

   ctHandleManager textureHandleManager;
   ctHashTable<ctVkKeyLimeTexture, ctHandle> textures;
   ctHandleManager geometryHandleManager;
   ctHashTable<ctVkKeyLimeGeometry, ctHandle> geometries;

   ctVkKeyLimeGlobalBufferData* pGlobalBufferData;

   size_t viewBufferCount = 1;
   ctVkKeyLimeViewBufferData* pViewBufferData;

   size_t transformsMax;
   ctKeyLimeStreamTransform* pTransformsData;
};