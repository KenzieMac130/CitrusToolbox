#include "OSEvents.hpp"
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

#include "EngineCore.hpp"

ctResults ctOSEventManager::Startup() {
   ZoneScoped;
   return CT_SUCCESS;
}

ctResults ctOSEventManager::Shutdown() {
   return CT_SUCCESS;
}

ctResults ctOSEventManager::PollOSEvents() {
   ZoneScoped;
   SDL_Event event;
   while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
         wantsExit = true;
      } else if (event.type == SDL_WINDOWEVENT) {
         for (size_t i = 0; i < WindowEventHandlers.Count(); i++) {
            const ctOSEventHandler handler = WindowEventHandlers[i];
            handler.callback(&event, handler.data);
         }
      } else {
         for (size_t i = 0; i < MiscEventHandlers.Count(); i++) {
            const ctOSEventHandler handler = MiscEventHandlers[i];
            handler.callback(&event, handler.data);
         }
      }
   }
   return CT_SUCCESS;
}
