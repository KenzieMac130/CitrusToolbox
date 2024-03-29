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

class CT_API ctModuleBase {
public:
   virtual ctResults Startup() = 0;
   virtual ctResults Shutdown() = 0;
   ctResults ModuleStartup(class ctEngineCore* pEngine);
   ctResults ModuleShutdown();

   bool isStarted() const;

   virtual void DebugUI(bool useGizmos);
   virtual const char* GetModuleName() = 0;

protected:
   class ctEngineCore* Engine = NULL;

private:
   bool _started = false;
};