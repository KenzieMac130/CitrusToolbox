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

#include "Component.hpp"
#include "Toy.hpp"

/* ---------------- Toy Includes ---------------- */
#include "toys/DebugCameraToy.hpp"

namespace ctHoneybell {

#define HB_COMPONENT_REGISTER_ENTRY(_COMPCLASS, _FACTORYCLASS)
#define HB_TOY_REGISTER_ENTRY(_PATH, _CLASS)

/* clang-format off */
/* ---------------- Builtin Toys Defined Here ---------------- */
#define HB_TOY_REGISTRIES() \
HB_TOY_REGISTER_ENTRY("citrus/debugCamera", DebugCameraToy)

/* clang-format on */
/* ---------------- Internals ---------------- */

#undef HB_TOY_REGISTER_ENTRY
#define HB_TOY_REGISTER_ENTRY(_PATH, _CLASS)                                             \
   ToyBase* toyNewFunc_##_CLASS(ConstructContext& ctx) {                                 \
      return new _CLASS(ctx);                                                            \
   };

HB_TOY_REGISTRIES()

#undef HB_TOY_REGISTER_ENTRY
#define HB_TOY_REGISTER_ENTRY(_PATH, _CLASS)                                             \
   registry.RegisterToyType(_PATH, toyNewFunc_##_CLASS);

void RegisterBuiltinToys(ToyTypeRegistry& registry) {
   ZoneScoped;
   HB_TOY_REGISTRIES()
}
}