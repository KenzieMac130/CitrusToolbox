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

#include "FPSDemoPlayer.hpp"

Game::FPSPlayer::FPSPlayer(ctHoneybell::ConstructContext& ctx) :
    ctHoneybell::ToyBase(ctx) {
   Controller = ctx.pComponentRegistry->NewComponent<PhysXControllerComponent>(this);
   DebugShape = ctx.pComponentRegistry->NewComponent<DebugShapeComponent>(this);
   if (Controller.isValid()) { Controller->CopyOwnerTransform(); }
   if (DebugShape.isValid()) {
      DebugShape->rgba = CT_COLOR_GREEN;
      DebugShape->SetLocalBounds(
        ctBoundBox(ctVec3(-0.5f, -1.0f, -0.5f), ctVec3(0.5f, 1.0f, 0.5f)));
   }
}

Game::FPSPlayer::~FPSPlayer() {
}

ctResults Game::FPSPlayer::OnBegin(ctHoneybell::BeginContext& ctx) {
   ToyBase::OnBegin(ctx);
   if (Controller.isValid()) {
      CT_RETURN_FAIL(Controller->InitController());
      Controller->pPxController->setUpDirection(ctVec3ToPx(CT_VEC3_UP));
      ctx.canTickSerial = true;
   }
   return CT_SUCCESS;
}

ctResults Game::FPSPlayer::OnTickSerial(ctHoneybell::TickContext& ctx) {
   if (Controller.isValid()) {
      if (Controller->pPxController) {
         const ctVec3 moveto = (ctx.gravity + ctVec3(0.0f, 0.0f, 1.0f)) * (float)ctx.deltaTime;
         Controller->pPxController->move(
           ctVec3ToPx(moveto), 0.01f, (float)ctx.deltaTime, PxControllerFilters());
         DebugShape->SetWorldTransform(Controller->GetWorldTransform());
      }
   }
   return CT_SUCCESS;
}

ctResults Game::FPSPlayer::OnFrameUpdate(ctHoneybell::FrameUpdateContext& ctx) {
   return CT_SUCCESS;
}
