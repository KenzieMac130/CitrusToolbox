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

enum ctResourcePriority {
   CT_RESOURCE_PRIORITY_BACKGROUND, /* scenery models, textures, distant level chunks, etc */
   CT_RESOURCE_PRIORITY_FOREGROUND, /* hud models, cutscene content, etc */
   CT_RESOURCE_PRIORITY_HIGHEST, /* highest priority gameplay config data */
};

#define ctGetResource(_RESOURCE_MANAGER_PTR, _TYPE, _KEY) (_TYPE)_RESOURCE_MANAGER_PTR->GetOrLoad(##_TYPE, _KEY, CT_RESOURCE_PRIORITY_BACKGROUND)
#define ctGetResourceWithPriority(_RESOURCE_MANAGER_PTR, _TYPE, _KEY, _PRIORITY) (_TYPE)_RESOURCE_MANAGER_PTR->GetOrLoad(##_TYPE, _KEY, _PRIORITY)

class ctResourceManager : public ctModuleBase {
   ctResourceBase* GetOrLoad(const char* className, ctGUID guid, ctResourcePriority priority);
   ctResourceBase* GetOrLoad(const char* className, const char* nickname, ctResourcePriority priority);
   bool isReloadOfTypeRequired(const char* className);
};