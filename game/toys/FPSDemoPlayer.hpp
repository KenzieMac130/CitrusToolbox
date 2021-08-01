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

#include "../GameCommon.hpp"
#include "scene/honeybell/Toy.hpp"

#include "scene/honeybell/components/PhysXControllerComponent.hpp"
#include "scene/honeybell/components/DebugShapeComponent.hpp"

using namespace ctHoneybell;

namespace Game {

class GAME_API FPSPlayer : public ctHoneybell::ToyBase {
public:
   FPSPlayer(ctHoneybell::ConstructContext& ctx);
   ~FPSPlayer();
   virtual ctResults OnBegin(ctHoneybell::BeginContext& ctx);
   virtual ctResults OnTickSerial(ctHoneybell::TickContext& ctx);
   virtual ctResults OnFrameUpdate(ctHoneybell::FrameUpdateContext& ctx);

private:
   ComponentPtr<PhysXControllerComponent> Controller;
   ComponentPtr<DebugShapeComponent> DebugShape;
};

}