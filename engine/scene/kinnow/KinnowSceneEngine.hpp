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
#include "scene/SceneEngineBase.hpp"

#define CITRUS_SCENE_ENGINE_CLASS ctKinnowSceneEngine

class CT_API ctKinnowSceneEngine : public ctSceneEngineBase {
public:
   virtual ctResults Startup();
   virtual ctResults Shutdown();
   /* Called at the end of a frame */
   virtual ctResults OnNextFrame(double deltaTime);

private:
   ctCameraInfo mainCamera;

   float rotationDistance = 3.0f;
   float rotationSpeed = 0.02f;
   float rotationPhase = 0.0f;
   ctVec3 cameraPos = ctVec3(0.0f, 0.5f, -1.0f); /* todo: removeme!!! */
   ctVec3 cameraTarget = ctVec3();              /* todo: removeme!!! */
};