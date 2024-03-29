#include "ModuleBase.hpp"
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

ctResults ctModuleBase::ModuleStartup(class ctEngineCore* pEngine) {
   Engine = pEngine;
   ctResults result = Startup();
   if (result == CT_SUCCESS) { _started = true; }
   return result;
}

ctResults ctModuleBase::ModuleShutdown() {
   if (_started) {
      _started = false;
      return Shutdown();
   }
   return CT_FAILURE_MODULE_NOT_INITIALIZED;
}

bool ctModuleBase::isStarted() const {
   return _started;
}

#if CITRUS_IMGUI
#include "imgui/imgui.h"
#endif
void ctModuleBase::DebugUI(bool useGizmos) {
#if CITRUS_IMGUI
   ImGui::Text("%s has nothing to display.", GetModuleName());
#endif
}
