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
#include "core/ModuleBase.hpp"

class CT_API ctKeyLimeRenderer : public ctModuleBase {
public:
   ctResults Startup() final;
   ctResults Shutdown() final;
   const char* GetModuleName() final;
   virtual void DebugUI(bool useGizmos);

   ctResults UpdateCamera(const ctCameraInfo& cameraInfo);

   ctResults RenderFrame();

   struct ctGPUDevice* pGPUDevice;
   struct ctGPUPresenter* pGPUPresenter;
   struct ctGPUBindlessManager* pGPUBindless;
   struct ctGPUArchitect* pGPUArchitect;
   struct ctGPUExternalBufferPool* pGPUBufferPool;
   struct ctGPUExternalTexturePool* pGPUTexturePool;

   void* pTestPipeline;
   bool rebuildRequired;

private:
   ctCameraInfo mainCamera;

   static void HandleWindowEvent(SDL_Event* event, ctKeyLimeRenderer* renderer);

   /* Features */
   struct ResourceUploadS {
      bool needsUpload;
      static ctResults Define(struct ctGPUArchitectDefinitionContext* pCtx,
                              ctKeyLimeRenderer* pRenderer);
      static ctResults Execute(struct ctGPUArchitectExecutionContext* pCtx,
                               ctKeyLimeRenderer* pRenderer);
   } ResourceUpload;
   struct CullS {
      static ctResults Define(struct ctGPUArchitectDefinitionContext* pCtx,
                              ctKeyLimeRenderer* pRenderer);
   } Cull;
   struct DeferredS {
      static ctResults DefineDepthPrepass(struct ctGPUArchitectDefinitionContext* pCtx,
                                          ctKeyLimeRenderer* pRenderer);
      static ctResults DefineGBufferPass(struct ctGPUArchitectDefinitionContext* pCtx,
                                         ctKeyLimeRenderer* pRenderer);
      static ctResults DefineLightPass(struct ctGPUArchitectDefinitionContext* pCtx,
                                       ctKeyLimeRenderer* pRenderer);
   } Deferred;
   struct PostProcessS {
      static ctResults DefineTonemapPass(struct ctGPUArchitectDefinitionContext* pCtx,
                                         ctKeyLimeRenderer* pRenderer);
      static ctResults DefineGizmoPassMain(struct ctGPUArchitectDefinitionContext* pCtx,
                                           ctKeyLimeRenderer* pRenderer);
      static ctResults DefineGizmoPassXray(struct ctGPUArchitectDefinitionContext* pCtx,
                                           ctKeyLimeRenderer* pRenderer);
      static ctResults DefineGUIPass(struct ctGPUArchitectDefinitionContext* pCtx,
                                     ctKeyLimeRenderer* pRenderer);
   } PostProcess;
};