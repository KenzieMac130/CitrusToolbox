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

class CT_API ctKeyLimeRenderPassDefinitionContext {
public:
   ctKeyLimeRenderPassDefinitionContext(ctGPUArchitectDefinitionContext* ctx) { pArchitectCtx = ctx; }
   void RunAfter(ctGPUArchitectDefinitionContext* ctx, const char* passName);
   void SetActive(bool);

   void DeclareResource();

private:
   ctGPUArchitectDefinitionContext* pArchitectCtx;
   ctStaticArray<ArchitectDependency, 32>
};

class CT_API ctKeyLimeRenderPassExecutionContext {
public:
   ctGPUArchitectExecutionContext(ctGPUArchitectExecutionContext* ctx) { pArchitectCtx = ctx; }

   void RasterDrawTriangle(const char* pipeline);
   void RasterDrawVertices(const char* pipeline, uint32_t triCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t firstInstance);
   void RasterDrawScene(const char* renderMode);
   void RasterDrawImGUI();
   void RasterDrawIm3D(const char* layer);

   void ComputeDispatch(pipeline, x, y, z);
private:
   ctGPUArchitectExecutionContext* pArchitectCtx;
};

class CT_API ctKeyLimeRenderPassBase {
public:
   virtual bool Poll(); /* decide whether to run and set the parent */
   virtual void Define(ctKeyLinmRenderPassDefinitionContext& ctx); /* define needed resources */
   virtual void Execute(ctKeyLimeRenderPassExecutionContext& ctx); /* render the data */

   private:
   ctGPUDependencyID GetFinishedBarrier();

   static ctResults DefinePassCallback(ctGPUArchitectDefinitionContext* ctx, void* pSelf);
   static ctResults (*ctGPUArchitectTaskExecutionFn)(ctGPUArchitectExecutionContext* pCtx,
                                                      void* pUserData)
};