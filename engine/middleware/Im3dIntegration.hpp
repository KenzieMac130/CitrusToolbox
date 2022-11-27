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
#include "../core/ModuleBase.hpp"

#include "im3d/im3d.h"

class CT_API ctIm3dIntegration : public ctModuleBase {
public:
   ctResults Startup() final;
   ctResults Shutdown() final;

   ctResults StartupGPU(struct ctGPUDevice* pGPUDevice,
                        struct ctGPUBindlessManager* pBindless,
                        struct ctGPUExternalBufferPool* pGPUBufferPool,
                        size_t maxVerts,
                        int32_t viewBind,
                        int32_t vtxBind,
                        enum TinyImageFormat colorFormat,
                        enum TinyImageFormat depthFormat);
   ctResults ShutdownGPU(struct ctGPUDevice* pGPUDevice,
                         struct ctGPUExternalBufferPool* pGPUBufferPool);

   void DrawImguiText(ctMat4 viewProj, ctCameraInfo cameraInfo);

   ctResults NextFrame();

private:
   void* pPipelines[Im3d::DrawPrimitive_Count];
};