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

typedef struct ctModelT* ctModel;

CT_API ctModel ctModelCreateEmpty();

CT_API ctResults ctModelLoad(ctModel* pModel, void* file);
CT_API ctResults ctModelSave(ctModel model, void* file);

CT_API const char* ctModelGetString(const ctModel model, int32_t handle);
CT_API int32_t ctModelCreateString(ctModel model, const char* string);

CT_API void ctModelRelease(ctModel);