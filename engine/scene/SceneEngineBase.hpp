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
#include "core/ModuleBase.hpp"

class CT_API ctSceneEngineBase : public ctModuleBase {
public:
   /* Called at the end of a frame */
   virtual ctResults NextFrame() = 0;

   /* Get ctCameraInfo for a camera (NULL must return the "main camera")*/
   virtual ctCameraInfo GetCameraInfo(const char* cameraId) = 0; /* todo: depreciate */
   /* Load scene/level */
   virtual ctResults LoadScene(const char* name, const char* message = NULL) = 0;
};