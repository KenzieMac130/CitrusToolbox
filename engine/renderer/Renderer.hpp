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

#include "lowlevel/GfxBase.hpp"

#include "core/ModuleBase.hpp"
#include "core/Logging.hpp"

class ctRenderer : public ctModuleBase {
public:
   ctRenderer(ctGfxBackend backend,
              bool validate,
              const char* appName,
              int appVersion[3]);
   ~ctRenderer();

   ctResults LoadConfig(ctJSONReader::Entry& json) final;
   ctResults SaveConfig(ctJSONWriter& writer) final;
   ctResults Startup(ctEngineCore* pEngine) final;
   ctResults Shutdown() final;

   ctGfxCoreBase* GfxCore;
};