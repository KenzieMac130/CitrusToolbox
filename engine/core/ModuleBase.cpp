#include "ModuleBase.hpp"
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

ctResults ctModuleBase::ModuleStartup(class ctEngineCore* pEngine) {
   Engine = pEngine;
   ctResults result = Startup();
   if (result == CT_SUCCESS) { _started = true; }
   return result;
}

ctResults ctModuleBase::ModuleShutdown() {
   _started = false;
   return Shutdown();
}

bool ctModuleBase::isStarted() const
{
    return _started;
}
