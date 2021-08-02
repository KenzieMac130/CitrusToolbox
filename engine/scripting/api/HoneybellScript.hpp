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

#include "CommonApi.hpp"

namespace ctScriptApi {
namespace Honeybell {
   int spawnToy(ctScriptTypedLightData* scene,
                const char* path,
                float x,
                float y,
                float z,
                float yaw,
                float pitch,
                float roll,
                float scale,
                const char* message);
   int spawnInternalToy(ctScriptTypedLightData* scene,
                        const char* type,
                        float x,
                        float y,
                        float z,
                        float yaw,
                        float pitch,
                        float roll,
                        float scale,
                        const char* message);
}
}