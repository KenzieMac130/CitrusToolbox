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
#include "EngineCore.hpp"

struct ctAppVersion { int32_t major; int32_t minor; int32_t patch; };

class CT_API ctApplication {
public:
   virtual int Execute(int argc, char* argv[]);

   /* Event Handling */
   virtual ctResults OnStartup();
   virtual ctResults OnTick(const float deltatime);
   virtual ctResults OnUIUpdate();
   virtual ctResults OnShutdown();

   virtual const char* GetAppName() = 0;
   virtual const char* GetAppDeveloperName() = 0;
   virtual ctAppVersion GetAppVersion() = 0;

   class ctEngineCore* Engine;
};