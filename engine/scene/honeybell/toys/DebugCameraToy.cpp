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

#include "DebugCameraToy.hpp"
#include "im3d/im3d.h"

ctHoneybell::DebugCameraToy::DebugCameraToy(ConstructContext& ctx) : ToyBase(ctx) {
   camera = ctx.pComponentRegistry->NewComponent<CameraComponent>(this);
   angle = 0.0f;
}

ctHoneybell::DebugCameraToy::~DebugCameraToy() {
}

ctResults ctHoneybell::DebugCameraToy::OnBegin(BeginContext& ctx) {
   ctx.canTickParallel = true;
   ctx.canFrameUpdate = true;
   return CT_SUCCESS;
}

ctResults ctHoneybell::DebugCameraToy::OnTickParallel(TickParallelContext& ctx) {
   ctVec3 pos = GetWorldPosition();
   ctQuat rot = GetWorldRotation();
   pos += rot.getForward() * (float)ctx.deltaTime * 3.0f;
   rot *= ctQuat(CT_VEC3_UP, angle * 0.005);
   angle += (float)ctx.deltaTime;
   angle = ctSin(angle);
   SetWorldPosition(pos);
   SetWorldRotation(rot);
   if (camera.isValid()) {
      camera->CopyOwnerTransform();
      camera->fov = 32.0f;
   }
   return CT_SUCCESS;
}

ctResults ctHoneybell::DebugCameraToy::OnFrameUpdate(FrameUpdateContext& ctx) {
   Im3d::PushColor(Im3d::Color_Green);
   Im3d::DrawSphere(ctVec3ToIm3d(GetWorldPosition()), 1.0f);
   Im3d::PopColor();
   return CT_SUCCESS;
}
