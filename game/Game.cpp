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

#include "Game.hpp"

#if CITRUS_SCENE_ENGINE_IS_HONEYBELL
/* --------- This is all boilerplate code you probably don't need to touch... --------- */
#undef HB_TOY_REGISTER_ENTRY
#define HB_TOY_REGISTER_ENTRY(_CLASS)                                                    \
   ctHoneybell::ToyBase* toyNewFunc_GAME_##_CLASS(ctHoneybell::ConstructContext& ctx) {  \
      return new _CLASS(ctx);                                                            \
   };
using namespace Game;
HB_TOY_REGISTRIES()
#undef HB_TOY_REGISTER_ENTRY
#define HB_TOY_REGISTER_ENTRY(_CLASS)                                                    \
   registry.RegisterToyType(_CLASS::GetTypePath(), toyNewFunc_GAME_##_CLASS);

void Game::GameCore::HoneybellRegisterToys(class ctHoneybell::ToyTypeRegistry& registry) {
   ZoneScoped;
   HB_TOY_REGISTRIES()
}
#endif