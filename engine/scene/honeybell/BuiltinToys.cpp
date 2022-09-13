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

#include "Component.hpp"
#include "Toy.hpp"

/* ---------------- Toy Includes ---------------- */
#include "toys/TestShape.hpp"
#include "toys/GroundPlane.hpp"
#include "toys/SceneChunk.hpp"

namespace ctHoneybell {

/* ---------------- Builtin Toys Defined Here ---------------- */
/* clang-format off */
#define HB_TOY_REGISTRIES() \
HB_TOY_REGISTER_ENTRY(SceneChunk)\
HB_TOY_REGISTER_ENTRY(TestShape)\
HB_TOY_REGISTER_ENTRY(GroundPlane)
/* clang-format on */

/* ---------------- Internals ---------------- */

#undef HB_TOY_REGISTER_ENTRY
#define HB_TOY_REGISTER_ENTRY(_CLASS)                                                    \
   ToyBase* toyNewFunc_##_CLASS(ConstructContext& ctx) {                                 \
      return new _CLASS(ctx);                                                            \
   };

HB_TOY_REGISTRIES()

#undef HB_TOY_REGISTER_ENTRY
#define HB_TOY_REGISTER_ENTRY(_CLASS)                                                    \
   registry.RegisterToyType(_CLASS::GetTypePath(), toyNewFunc_##_CLASS);

void RegisterBuiltinToys(ToyTypeRegistry& registry) {
   ZoneScoped;
   HB_TOY_REGISTRIES()
}
}