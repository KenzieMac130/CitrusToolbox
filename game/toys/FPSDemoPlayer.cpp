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

TOY_SIGNALS_BEGIN(Game::FPSPlayer)
TOY_SIGNAL("actions/player/moveForward", OnMoveForward)
TOY_SIGNAL("actions/player/moveRight", OnMoveRight)
TOY_SIGNAL("actions/player/lookUp", OnLookUp)
TOY_SIGNAL("actions/player/lookRight", OnLookRight)
TOY_SIGNAL("actions/player/jump", OnJump)
TOY_SIGNALS_END()

Game::FPSPlayer::FPSPlayer(ctHoneybell::ConstructContext& ctx) :
    ctHoneybell::ToyBase(ctx) {
   Controller = new PhysXControllerComponent(ctx, this);
   DebugShape = new DebugShapeComponent(ctx, this);
}

Game::FPSPlayer::~FPSPlayer() {
}

ctResults Game::FPSPlayer::OnBegin(ctHoneybell::BeginContext& ctx) {
   ToyBase::OnBegin(ctx);
   ctx.canTickSerial = true;
   ctx.canHandleSignals = true;
   BeginComponents(ctx);
   Controller->CopyOwnerTransform();
   DebugShape->rgba = CT_COLOR_PINK;
   DebugShape->SetLocalBounds(
     ctBoundBox(ctVec3(-0.5f, -1.0f, -0.5f), ctVec3(0.5f, 1.0f, 0.5f)));
   return CT_SUCCESS;
}

ctResults Game::FPSPlayer::OnTickSerial(ctHoneybell::TickContext& ctx) {
   const ctVec3 moveto = (ctx.gravity + moveTarget) * (float)ctx.deltaTime;
   Controller->rotation = ctQuat(CT_VEC3_UP, yaw);
   Controller->pPxController->move(
     ctVec3ToPx(moveto), 0.01f, (float)ctx.deltaTime, PxControllerFilters());
   DebugShape->SetWorldTransform(Controller->GetWorldTransform());
   CopyComponentTransform(Controller);
   PxControllerState state;
   Controller->pPxController->getState(state);
   if (state.standOnObstacle) { isJumping = false; }
   moveTarget = 0.0f;
   return CT_SUCCESS;
}

ctResults Game::FPSPlayer::OnTryPossess(PossessionContext& ctx) {
   return CT_SUCCESS;
}

ctResults Game::FPSPlayer::OnMoveForward(SignalContext& ctx) {
   moveTarget += GetWorldTransform().rotation.getForward() * ctx.value * moveSpeed;
   return CT_SUCCESS;
}

ctResults Game::FPSPlayer::OnMoveRight(SignalContext& ctx) {
   moveTarget += GetWorldTransform().rotation.getRight() * ctx.value * moveSpeed;
   return CT_SUCCESS;
}

ctResults Game::FPSPlayer::OnLookUp(SignalContext& ctx) {
   pitch += ctx.value * lookSpeed * ctx.deltaTime;
   return CT_SUCCESS;
}

ctResults Game::FPSPlayer::OnLookRight(SignalContext& ctx) {
   yaw += ctx.value * lookSpeed * ctx.deltaTime;
   return CT_SUCCESS;
}

ctResults Game::FPSPlayer::OnJump(SignalContext& ctx) {
   moveTarget += CT_VEC3_UP * jumpStrength;
   return CT_SUCCESS;
}
