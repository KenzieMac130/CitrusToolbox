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

#include "utilities/Common.h"
#include "interact/DeviceBackendLayer.hpp"
#include "core/EngineCore.hpp"

CT_API ctResults ctToggleInteractBackend(const char* name, bool use);
CT_API void ctRetrieveInteractBackends(ctDynamicArray<ctInteractAbstractBackend*>& backendList);
CT_API ctResults ctStartAndRetrieveInteractBackends(ctEngineCore* pEngine, ctDynamicArray<ctInteractAbstractBackend*>& backendList);
CT_API ctResults ctShutdownInteractBackends();