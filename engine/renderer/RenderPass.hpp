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

class CT_API ctKeyLimeRenderPassBase {
public:
const char* name;

virtual bool Define(ctGPUArchitectDefinitionContext* ctx);
virtual void Execute(ctGPUArchitectExecutionContext* ctx);

/* Features for definition */
void RunAfter(ctGPUArchitectDefinitionContext* ctx, ctKeyLimeRenderPassBase* otherFeature);

private:
ctGPUDependencyID GetFinishedBarrier();

static ctResults DefinePassCallback(ctGPUArchitectDefinitionContext* ctx, void* pSelf);
static ctResults (*ctGPUArchitectTaskExecutionFn)(ctGPUArchitectExecutionContext* pCtx,
                                                   void* pUserData)
}