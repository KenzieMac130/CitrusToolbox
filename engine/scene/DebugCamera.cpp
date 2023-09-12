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
#include "DebugCamera.hpp"

#include "interact/InteractionEngine.hpp"

ctDebugCamera::ctDebugCamera() {
   speedDefault = 5.0f;
   speedFast = 15.0f;
   lookSpeed = 2.0f;
   camYaw = 0.0f;
   camPitch = 0.0f;
   camera = ctCameraInfo();
   camera.position = ctVec3(0.0f, 0.0f, -10.0f);
}

void ctDebugCamera::FrameUpdate(float deltaTime) {
   if (!ctGetSignal("actions/debug/lookEnable")) { return; }
   ctRequestRelativePointer();
   float forwardMovement = ctGetSignal("actions/debug/moveForward");
   float rightMovement = ctGetSignal("actions/debug/moveRight");
   float upMovement = ctGetSignal("actions/debug/moveUp");

   if (ctGetSignal("actions/debug/moveFaster")) {
      forwardMovement *= speedFast * deltaTime;
      rightMovement *= speedFast * deltaTime;
      upMovement *= speedFast * deltaTime;
   } else {
      forwardMovement *= speedDefault * deltaTime;
      rightMovement *= speedDefault * deltaTime;
      upMovement *= speedDefault * deltaTime;
   }

   float lookHorizontal = -ctGetSignal("actions/debug/lookRight");
   float lookVertical = -ctGetSignal("actions/debug/lookUp");
   lookHorizontal *= lookSpeed * deltaTime;
   lookVertical *= lookSpeed * deltaTime;

   camYaw += lookHorizontal * lookSpeed;
   camPitch += lookVertical * lookSpeed;
   if (camPitch < -CT_PI / 2.0f + 0.05f) { camPitch = -CT_PI / 2.0f + 0.05f; }
   if (camPitch > CT_PI / 2.0f - 0.05f) { camPitch = CT_PI / 2.0f - 0.05f; }
   camera.rotation = ctQuat(CT_VEC3_UP, camYaw) * ctQuat(CT_VEC3_RIGHT, camPitch);

   camera.position += camera.rotation.getForward() * forwardMovement;
   camera.position += camera.rotation.getRight() * rightMovement;
   camera.position += camera.rotation.getUp() * upMovement;
}
