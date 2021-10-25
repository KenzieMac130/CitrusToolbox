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

#include "scripting/LuaScript.hpp"
#include "scene/SceneEngineBase.hpp"

#include "Scene.hpp"
#include "audition/HotReloadDetection.hpp"

#define CITRUS_SCENE_ENGINE_CLASS ctHoneybellSceneEngine

class CT_API ctHoneybellSceneEngine : public ctSceneEngineBase {
public:
   ctResults Startup();
   ctResults Shutdown();

   ctResults NextFrame();

   void SetCameraInfo(ctCameraInfo camera, const char* cameraId = NULL);
   ctCameraInfo GetCameraInfo(const char* cameraId) final;
   ctResults LoadScene(const char* path, const char* message = NULL) final;
   ctResults PossessToy(ctHandle handle,
                        bool wantCamera = true,
                        int32_t playerIdx = 0,
                        const char* message = NULL);

   int32_t pauseSim;
   int32_t simSingleShots;
   int32_t sceneReload;

private:
#if CITRUS_INCLUDE_AUDITION
   ctHotReloadCategory hotReload;
#endif

   ctStringUtf8 activeSceneName;
   ctHoneybell::Scene mainScene;
   ctHoneybell::ToyTypeRegistry toyRegistry;

   ctHandle possessedToy;

   ctCameraInfo CurrentCamera;

   int32_t debugCameraAllowed = false;
   int32_t debugCameraActive = false;
   ctCameraInfo debugCamera;
   float camYaw;
   float camPitch;
   float camSpeedBase;
};