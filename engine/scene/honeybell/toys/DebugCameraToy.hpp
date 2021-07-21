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

#include "../Toy.hpp"
#include "../components/CameraComponent.hpp"

namespace ctHoneybell {

class CT_API DebugCameraToy : public ToyBase {
public:
   DebugCameraToy(ConstructContext& ctx);
   virtual ~DebugCameraToy();

   virtual ctResults OnBegin(BeginContext& ctx);
   virtual ctResults OnTickParallel(TickParallelContext& ctx);
   virtual ctResults OnFrameUpdate(FrameUpdateContext& ctx);

private:
   float angle;
   ComponentPtr<CameraComponent> camera;
};
}