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
#include "core/ModuleBase.hpp"

#include "imgui/imgui.h"

class CT_API ctImguiIntegration : public ctModuleBase {
public:
   ctResults Startup() final;
   ctResults Shutdown() final;

   ctResults StartupGPU(struct ctGPUDevice* pGPUDevice,
                        struct ctGPUExternalBufferPool* pGPUBufferPool,
                        struct ctGPUExternalTexturePool* pGPUTexturePool,
                        size_t maxVerts,
                        size_t maxIndices,
                        int32_t fontBind,
                        int32_t idxBind,
                        int32_t vtxBind,
                        enum TinyImageFormat colorFormat,
                        enum TinyImageFormat depthFormat);
   ctResults ShutdownGPU(struct ctGPUDevice* pGPUDevice,
                         struct ctGPUExternalBufferPool* pGPUBufferPool,
                         struct ctGPUExternalTexturePool* pGPUTexturePool);
   static ctResults DrawCallback(struct ctGPUArchitectExecutionContext* pCtx,
                                 void* pUserData);

   ctResults NextFrame();

private:
   void _DrawGPU(struct ctGPUArchitectExecutionContext* pCtx);
   ctStringUtf8 iniPath;
   struct ctGPUExternalTexture* pFontTexture;
   struct ctGPUExternalBuffer* pIndexBuffer;
   struct ctGPUExternalBuffer* pVertexBuffer;
   void* pPipeline;
};