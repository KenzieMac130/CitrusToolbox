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

#include "lua.hpp"

#include "api/CommonApi.hpp"

class CT_API ctLuaContext {
public:
   ctResults Startup(bool trusted);
   ctResults Shutdown();

   ctResults OpenEngineLibrary(const char* name);

   ctResults LoadFromBuffer(const char* data, size_t size, const char* name);
   ctResults LoadFromFile(const char* path);

   ctResults RunScript();

   ctResults CallFunction(const char* name, const char* signature, ...);

   lua_State* L;
};