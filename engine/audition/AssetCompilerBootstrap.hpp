/*
   Copyright 2023 MacKenzie Strand

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

class CT_API ctAssetCompilerBootstrap : public ctModuleBase {
public:
   ctResults Startup() final;
   ctResults Shutdown() final;
   const char* GetModuleName() final;
   virtual void DebugUI(bool useGizmos);

   ctResults StartCompiler(size_t subargCount = 0, const char** subargs = NULL);
   void ClearResults();

   void AcquireOutputLock();
   /* copy the results before releasing the lock! */
   const char* GetOutputText();
   void ReleaseOutputLock();

   ctStringUtf8 pythonPath = "python";
   ctStringUtf8 wafPath =
     "C:\\Users\\Kenzie\\Documents\\GitHub\\CitrusToolbox\\assets"; /* todo: duh! */

private:
   ctSpinLock lock;
   bool compilerFinished = true;
   ctStringUtf8 outputText = "";

   struct ctAssetCompilerContext* pCompilerCtx;
   ctThread compilerThread;
};