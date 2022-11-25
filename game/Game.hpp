/*
   Copyright 2022 MacKenzie Strand

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

#if CITRUS_SCENE_ENGINE_IS_HONEYBELL
#include "scene/honeybell/Toy.hpp"
#endif

namespace Game {

#if CITRUS_SCENE_ENGINE_IS_HONEYBELL
/* ---------------- Game Toys Defined Here ---------------- */
/* clang-format off */
#define HB_TOY_REGISTRIES() \
HB_TOY_REGISTER_ENTRY(ClassName)
/* clang-format on */
#endif

class GAME_API GameCore : public ctGameLayer {
public:
#if CITRUS_SCENE_ENGINE_IS_HONEYBELL
   virtual void HoneybellRegisterToys(ctHoneybell::ToyTypeRegistry& registry);
#endif
};

}