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
#include "core/ModuleBase.hpp"

#include "DebugCamera.hpp"

class CT_API ctSceneEngine : public ctModuleBase {
public:
   virtual ctResults Startup();
   virtual ctResults Shutdown();
   virtual ctResults NextFrame(double deltaTime);
   virtual const char* GetModuleName();

   /* ------------ Camera Override ------------ */
   inline void EnableCameraOverride() {
      cameraOverride = true;
   }
   inline void SetCameraOverride(ctCameraInfo camera) {
      mainCamera = camera;
   }
   inline void DisableCameraOverride() {
      cameraOverride = false;
   }
   inline bool isCameraOverrideActive() const {
      return cameraOverride;
   }

   /* ------------ Debug Camera ------------ */
   void EnableDebugCamera();
   void DisableDebugCamera();
   inline bool isDebugCameraActive() const {
      return debugCameraEnabled;
   }

protected:
   double timeAccumulator;
   float timeStep = 1.0f / 30;
   float maxFrameTime = 1.0f / 15.0f;

   bool cameraOverride;
   ctCameraInfo mainCamera;
   ctVec3 cursorDirection;

   bool debugCameraEnabled;
   ctDebugCamera debugCamera;

   void UpdateCursor();
   void PushCameraToRenderer();
   void PushAndResetCursor();
};