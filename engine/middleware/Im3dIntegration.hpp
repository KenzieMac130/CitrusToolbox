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

enum ctIm3dLayer { CT_IM3D_LAYER_DEFAULT = 0, CT_IM3D_LAYER_XRAY = 1 };

class CT_API ctIm3dIntegration : public ctModuleBase {
public:
   ctResults Startup() final;
   ctResults Shutdown() final;
   const char* GetModuleName() final;

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
   void SkipGPU();
   ctResults PrepareFrameGPU(struct ctGPUDevice* pGPUDevice,
                             struct ctGPUExternalBufferPool* pGPUBufferPool);
   static ctResults DrawCallbackMain(struct ctGPUArchitectExecutionContext* pCtx,
                                     void* pUserData);
   static ctResults DrawCallbackXRay(struct ctGPUArchitectExecutionContext* pCtx,
                                     void* pUserData);
   void DrawImguiText();

   void SetCamera(ctCameraInfo camera);
   void SetSelection(ctVec3 origin, ctVec3 direction);
   ctResults NextFrame();

   virtual void DebugUI(bool useGizmos);

private:
   ctCameraInfo cameraInfo;
   void DrawGPU(struct ctGPUArchitectExecutionContext* pCtx, bool xray);
   static void ctIm3dUploadViewData(uint8_t* dest, size_t size, void* data);
   static void ctIm3dUploadVertexData(uint8_t* dest, size_t size, void* data);

   void* pPipelines[Im3d::DrawPrimitive_Count];
   struct ctGPUExternalBuffer* pViewBuffer;
   struct ctGPUExternalBuffer* pVertexBuffer;

   struct ctGPUStructAssembler* pVertexStructAssembler;
   uint32_t positionScale;
   uint32_t icolor;

   struct ctGPUStructAssembler* pViewStructAssembler;
   uint32_t viewProj;
   uint32_t viewportSize;

   uint32_t drawListCount = 0;
   uint32_t vertexCount = 0;
   uint32_t triangleCount = 0;
   uint32_t lineCount = 0;
   uint32_t pointCount = 0;
   uint32_t textCount = 0;
};