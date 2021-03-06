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

/* API Main Goal: Keep the interface simple for non-graphics programmers! */

#include "core/ModuleBase.hpp"
#include "utilities/Common.h"

struct ctKeyLimeCameraDesc {
   ctVec3 position;
   ctQuat rotation;
   float fov;
};

class CT_API ctKeyLimeRenderer : public ctModuleBase {
public:
   ctResults Startup() final;
   ctResults Shutdown() final;

   ctResults UpdateCamera(const ctKeyLimeCameraDesc& cameraDesc);
   ctResults RenderFrame();

#ifdef CITRUS_GFX_VULKAN
   class ctVkKeyLimeCore* vkKeyLime;
#endif
};