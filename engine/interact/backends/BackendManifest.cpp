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

#include "BackendManifest.hpp"

#if CITRUS_INTERACT_INPUT_SDL
#include "sdl/SDLGamepad.hpp"
#include "sdl/SDLKeyboardMouse.hpp"
#endif

#define CTI_REGISTER(_enum, _name, _class)

/* Register SDL based backends*/
#if CITRUS_INTERACT_INPUT_SDL
#define CTI_REGISTER_SDL()                                                               \
   CTI_REGISTER(                                                                         \
     CT_INTERACT_BACKEND_SDL_GAMEPAD, "SdlGamepad", ctInteractSDLGamepadBackend)         \
   CTI_REGISTER(CT_INTERACT_BACKEND_SDL_KEYBOARDMOUSE,                                   \
                "SdlKeyboardMouse",                                                      \
                ctInteractSDLKeyboardMouseBackend)
#else
#define CTI_REGISTER_SDL()
#endif

/* Register all backends*/
#define CTI_REGISTER_ALL() CTI_REGISTER_SDL()
// clang-format on

/* ---------------------------------- Internal ---------------------------------- */

#undef CTI_REGISTER
#define CTI_REGISTER(_enum, _name, _class) _enum,

enum ctInteractBackends {
   CT_INTERACT_BACKEND_NULL,
   CTI_REGISTER_ALL() CT_INTERACT_BACKEND_COUNT,
};

#undef CTI_REGISTER
#define CTI_REGISTER(_enum, _name, _class) _name,

const char* backendNames[CT_INTERACT_BACKEND_COUNT] {"NULL", CTI_REGISTER_ALL()};

#undef CTI_REGISTER
#define CTI_REGISTER(_enum, _name, _class)                                               \
   case _enum: return new _class();

ctInteractAbstractBackend* newBackend(ctInteractBackends type) {
   switch (type) {
      CTI_REGISTER_ALL()
      default: ctAssert(1);
   }
   return NULL;
}

void deleteBackend(ctInteractAbstractBackend* pObj) {
   delete pObj;
}

ctInteractAbstractBackend* pBackends[CT_INTERACT_BACKEND_COUNT];
bool backendEnabled[CT_INTERACT_BACKEND_COUNT];

ctResults ctToggleInteractBackend(const char* name, bool initialize) {
   if (name == NULL) { return CT_FAILURE_INVALID_PARAMETER; }
   for (int i = 0; i < CT_INTERACT_BACKEND_COUNT; i++) {
      if (ctCStrEql(name, backendNames[i])) {
         backendEnabled[i] = initialize;
         return CT_SUCCESS;
      }
   }
   return CT_FAILURE_NOT_FOUND;
}

void ctRetrieveInteractBackends(ctDynamicArray<ctInteractAbstractBackend*>& backendList) {
   for (int i = 0; i < CT_INTERACT_BACKEND_COUNT; i++) {
      if (backendEnabled[i]) { backendList.Append(pBackends[i]); }
   }
}

ctResults ctStartAndRetrieveInteractBackends(
  ctEngineCore* pEngine, ctDynamicArray<ctInteractAbstractBackend*>& backendList) {
   ZoneScoped;
   for (int i = 0; i < CT_INTERACT_BACKEND_COUNT; i++) {
      if (backendEnabled[i]) {
         pBackends[i] = newBackend((ctInteractBackends)i);
         ctAssert(pBackends[i]);
         pBackends[i]->ModuleStartup(pEngine);
      }
   }
   ctRetrieveInteractBackends(backendList);
   return CT_SUCCESS;
}

ctResults ctShutdownInteractBackends() {
   ZoneScoped;
   for (int i = 0; i < CT_INTERACT_BACKEND_COUNT; i++) {
      if (backendEnabled[i]) {
         pBackends[i]->ModuleShutdown();
         deleteBackend(pBackends[i]);
      }
   }
   return CT_SUCCESS;
}
