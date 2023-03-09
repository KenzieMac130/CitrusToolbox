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
#include "EntityComponentSystem.hpp"
#include "NetworkBus.hpp"

#define CITRUS_SCENE_ENGINE_CLASS ctKinnowSceneEngine

class CT_API ctKinnowSceneEngine : public ctSceneEngineBase {
public:
   virtual ctResults Startup();
   virtual ctResults Shutdown();
   /* Called at the end of a frame */
   virtual ctResults OnNextFrame(double deltaTime);

   ctResults StartClient();
   ctResults StartServer(int32_t port = -1);

   inline void EnableCameraOverride() {
      cameraOverride = true;
   }
   inline void SetCameraOverride(ctCameraInfo camera) {
      mainCamera = camera;
   }
   inline void DisableCameraOverride() {
      cameraOverride = false;
   }

protected:
   ctSceneContext clientScene;  /* handles rendering, sound, animations, camera/player
                                   visuals (won't exist on server builds) */
   ctSceneNetworkBus clientBus; /* sends stuff to server (mostly just player things) */
   ctSceneNetworkBus serverBus; /* sends stuff from the server to the client */
   ctSceneContext serverScene;  /* does gameplay simulations and sends create/destroy
                                   commands and transforms/animation state components to
                                   client (won't exist on online modes) */
   /* simply put: the server will patch its renderable components into the client, the
    * client will patch its changes back into the server, neither need a full picture */
#if CITRUS_INCLUDE_AUDITION
   ctSceneNetworkBus
     auditionBus; /* sends spawn/destroy commands from audition live sync to the server */
#endif
   bool cameraOverride;
   ctCameraInfo mainCamera;
   void PushCameraToRenderer();
};