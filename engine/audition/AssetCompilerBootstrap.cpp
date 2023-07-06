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

#include "AssetCompilerBootstrap.hpp"
#include "core/Settings.hpp"
#include "core/EngineCore.hpp"
#include "system/System.h"

struct ctAssetCompilerContext {
   ctStringUtf8 pythonPath;
   ctStringUtf8 wafBasePath;
   ctStringUtf8 wafPath;
   ctStringUtf8 subargs[8];

   ctSpinLock* pLock;
   ctStringUtf8* pOutput;
   bool* pFinished;
};

void OnCompilerOutput(const char* text, void* data) {
   ctAssetCompilerContext& compiler = *(ctAssetCompilerContext*)data;
   ctDebugLog("Asset Compiler: %s", text);
   ctSpinLockEnterCritical(*compiler.pLock);
   *compiler.pOutput += text;
   ctSpinLockExitCritical(*compiler.pLock);
}

int CompilerThread(void* data) {
   ctAssetCompilerContext& compiler = *(ctAssetCompilerContext*)data;
   const char* args[] = {compiler.wafPath.CStr(), "build", "-v"};
   int result = ctSystemExecuteCommand(compiler.pythonPath.CStr(),
                                       3,
                                       args,
                                       OnCompilerOutput,
                                       data,
                                       compiler.wafBasePath.CStr());
   ctSpinLockEnterCritical(*compiler.pLock);
   *compiler.pFinished = true;
   ctSpinLockExitCritical(*compiler.pLock);
   return result;
}

ctResults ctAssetCompilerBootstrap::Startup() {
   ctSettingsSection* settings = Engine->Settings->GetOrCreateSection("Audition", 3);
   settings->BindString(&pythonPath,
                        true,
                        true,
                        "PythonPath",
                        "Command or path to a python interpreter.",
                        NULL,
                        NULL);
   settings->BindString(&wafPath,
                        true,
                        true,
                        "WafPath",
                        "Path to the waf file.",
                        NULL,
                        NULL);
   ctSpinLockInit(lock);
   return CT_SUCCESS;
}

ctResults ctAssetCompilerBootstrap::Shutdown() {
   return CT_SUCCESS;
}

const char* ctAssetCompilerBootstrap::GetModuleName() {
   return "Asset Compiler Bootstrap";
}

#include "middleware/ImguiIntegration.hpp"
void ctAssetCompilerBootstrap::DebugUI(bool useGizmos) {
   AcquireOutputLock();
   ImGui::Text(GetOutputText());
   ReleaseOutputLock();
}

void ctAssetCompilerBootstrap::AcquireOutputLock() {
   ctSpinLockEnterCritical(lock);
}
const char* ctAssetCompilerBootstrap::GetOutputText() {
   return outputText.CStr();
}
void ctAssetCompilerBootstrap::ReleaseOutputLock() {
   ctSpinLockExitCritical(lock);
}

ctResults ctAssetCompilerBootstrap::StartCompiler(size_t subargCount,
                                                  const char** subargs) {
   ctSpinLockEnterCritical(lock);
   if (!compilerFinished) {
      ctSpinLockExitCritical(lock);
      return CT_FAILURE_NOT_FINISHED;
   }
   compilerFinished = false;
   ctSpinLockExitCritical(lock);
   if (pCompilerCtx) { delete pCompilerCtx; }
   pCompilerCtx = new ctAssetCompilerContext();
   pCompilerCtx->pythonPath = pythonPath;
   pCompilerCtx->wafBasePath = wafPath;
   pCompilerCtx->wafPath = wafPath;
   pCompilerCtx->wafPath.FilePathUnify();
   pCompilerCtx->wafPath.FilePathAppend("waf");
   pCompilerCtx->wafPath.FilePathLocalize();
   pCompilerCtx->pLock = &lock;
   pCompilerCtx->pOutput = &outputText;
   pCompilerCtx->pFinished = &compilerFinished;
   compilerThread = ctThreadCreate(CompilerThread, pCompilerCtx, "Asset Compiler");
   return CT_SUCCESS;
}

void ctAssetCompilerBootstrap::ClearResults() {
   ctSpinLockEnterCritical(lock);
   outputText.Clear();
   ctSpinLockExitCritical(lock);
}
