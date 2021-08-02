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

#include "GameCommon.hpp"
#include "gamelayer/GameLayer.hpp"

#include "scene/honeybell/Toy.hpp"

/* Include toys */
#include "toys/FPSDemoPlayer.hpp"

namespace Game {

/* ---------------- Game Toys Defined Here ---------------- */
/* clang-format off */
#define HB_TOY_REGISTRIES() \
HB_TOY_REGISTER_ENTRY("fps/player", FPSPlayer)
/* clang-format on */

class GAME_API GameCore : public ctGameLayer {
public:
   virtual void HoneybellRegisterToys(ctHoneybell::ToyTypeRegistry& registry);
};

}