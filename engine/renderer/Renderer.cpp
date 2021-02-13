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

#include "Renderer.hpp"

#include "lowlevel/vulkan/GfxCoreVk.hpp"

ctRenderer::ctRenderer(ctGfxBackend backend,
                       bool validate,
                       const char* appName,
                       int appVersion[3]) {
   if (backend == CT_GFX_VULKAN) {
      GfxCore = new ctGfxCoreVk(validate, appName, appVersion);
   } else {
      GfxCore = NULL;
      ctAssert(1);
   }
}

ctRenderer::~ctRenderer() {
   delete GfxCore;
}

ctResults ctRenderer::LoadConfig(ctJSONReader::Entry& json) {
   return CT_SUCCESS;
}

ctResults ctRenderer::SaveConfig(ctJSONWriter& writer) {
   return CT_SUCCESS;
}

ctResults ctRenderer::Startup(ctEngineCore* pEngine) {
   Engine = pEngine;
   GfxCore->Startup(Engine);
   return CT_SUCCESS;
}

ctResults ctRenderer::Shutdown() {
   GfxCore->Shutdown();
   return CT_SUCCESS;
}
