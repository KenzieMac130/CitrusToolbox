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

#include "SDLKeyboardMouse.hpp"
#include "interact/InteractionEngine.hpp"
#include "core/EngineCore.hpp"
#include "core/OSEvents.hpp"
#include "imgui/imgui.h"

void SDLKeyboardMouseOnEvent(SDL_Event* event, void* data) {
   ctInteractSDLKeyboardMouseBackend* pBackend = (ctInteractSDLKeyboardMouseBackend*)data;
   switch (event->type) {
      case SDL_MOUSEWHEEL: {
         if (event->wheel.which == SDL_TOUCH_MOUSEID) { return; }
         pBackend->mouseAxisStates[2] = (float)event->wheel.x;
         pBackend->mouseAxisStates[3] = (float)event->wheel.y;
         if (event->wheel.direction == SDL_MOUSEWHEEL_FLIPPED) {
            pBackend->mouseAxisStates[2] *= -1.0f;
            pBackend->mouseAxisStates[3] *= -1.0f;
         }
      }
      default: break;
   }
}

void SDLKeyboardMouseOnPrePoll(void* data) {
   ctInteractSDLKeyboardMouseBackend* pBackend = (ctInteractSDLKeyboardMouseBackend*)data;
   pBackend->mouseAxisStates[2] = 0.0f;
   pBackend->mouseAxisStates[3] = 0.0f;
}

ctResults ctInteractSDLKeyboardMouseBackend::Startup() {
   ZoneScoped;
   ctDebugLog("Starting SDL Keyboard and Mouse...");
   keyStates = (uint8_t*)SDL_GetKeyboardState(NULL);
   Engine->OSEventManager->PrePollHandlers.Append({SDLKeyboardMouseOnPrePoll, this});
   Engine->OSEventManager->MiscEventHandlers.Append({SDLKeyboardMouseOnEvent, this});
   return CT_SUCCESS;
}

ctResults ctInteractSDLKeyboardMouseBackend::Shutdown() {
   return CT_SUCCESS;
}

ctResults
ctInteractSDLKeyboardMouseBackend::Register(ctInteractDirectorySystem& directory) {
   ZoneScoped;
   /* Add all key inputs */
   for (int i = 0; i < SDL_NUM_SCANCODES; i++) {
      ctInteractNode node = ctInteractNode();
      node.type = CT_INTERACT_NODETYPE_BOOL;
      node.accessible = true;
      node.pData = &keyStates[i];
      snprintf(
        node.path.str, CT_MAX_INTERACT_PATH_SIZE, "dev/keyboard/input/scancode/%d", i);
      directory.AddNode(node);
   }

   /* Add mouse inputs */
   char* mouseButtonPaths[] = {"dev/mouse/input/button/left",
                               "dev/mouse/input/button/right",
                               "dev/mouse/input/button/middle",
                               "dev/mouse/input/button/x1",
                               "dev/mouse/input/button/x2"};
   for (int i = 0; i < 5; i++) {
      ctInteractNode node = ctInteractNode();
      node.type = CT_INTERACT_NODETYPE_BOOL;
      node.accessible = true;
      node.path = mouseButtonPaths[i];
      node.pData = &mouseButtonStates[i];
      directory.AddNode(node);
   }

   char* mouseAxisPaths[] = {"dev/mouse/input/relative_move/x",
                             "dev/mouse/input/relative_move/y",
                             "dev/mouse/input/scroll/x",
                             "dev/mouse/input/scroll/y"};
   for (int i = 0; i < 4; i++) {
      ctInteractNode node = ctInteractNode();
      node.type = CT_INTERACT_NODETYPE_SCALAR;
      node.accessible = true;
      node.path = mouseAxisPaths[i];
      node.pData = &mouseAxisStates[i];
      directory.AddNode(node);
   }

   ctFile file;
   Engine->FileSystem->OpenAssetFileNamed(
     file, "input/keyboard.json", CT_FILE_OPEN_READ_TEXT);
   directory.CreateBindingsFromFile(file);
   file.Close();
   Engine->FileSystem->OpenAssetFileNamed(
     file, "input/mouse.json", CT_FILE_OPEN_READ_TEXT);
   directory.CreateBindingsFromFile(file);
   file.Close();
#if CITRUS_INCLUDE_AUDITION
   directory.configHotReload.RegisterPath("input/keyboard.json");
   directory.configHotReload.RegisterPath("input/mouse.json");
#endif

   return CT_SUCCESS;
}

ctResults
ctInteractSDLKeyboardMouseBackend::Update(ctInteractDirectorySystem& directory) {
   ZoneScoped;
   int x, y;
   uint32_t mouseFlags = SDL_GetRelativeMouseState(&x, &y);
   memset(mouseButtonStates, 0, sizeof(mouseButtonStates));
   if (mouseFlags & SDL_BUTTON(SDL_BUTTON_LEFT)) { mouseButtonStates[0] = true; }
   if (mouseFlags & SDL_BUTTON(SDL_BUTTON_RIGHT)) { mouseButtonStates[1] = true; }
   if (mouseFlags & SDL_BUTTON(SDL_BUTTON_MIDDLE)) { mouseButtonStates[2] = true; }
   if (mouseFlags & SDL_BUTTON(SDL_BUTTON_X1)) { mouseButtonStates[3] = true; }
   if (mouseFlags & SDL_BUTTON(SDL_BUTTON_X2)) { mouseButtonStates[4] = true; }

   /* Calculate normalized mouse movement */
   int w, h;
   SDL_Window* pWindow = SDL_GetMouseFocus();
   SDL_GetWindowSize(pWindow, &w, &h);
   const float aspect = (float)w / (float)h;
   const float time = Engine->FrameTime.GetDeltaTimeFloat();
   mouseAxisStates[0] = ((float)x / (float)w) / time;
   mouseAxisStates[1] = ((float)y * aspect / (float)h) / time;
   return CT_SUCCESS;
}

ctResults ctInteractSDLKeyboardMouseBackend::DebugImGui() {
   ImGui::PlotHistogram("Mouse Axis", mouseAxisStates, 4, 0, NULL, -1.0f, 1.0f);
   ImGui::Text("Mouse Axis Print: %f, %f, %f, %f",
               mouseAxisStates[0],
               mouseAxisStates[1],
               mouseAxisStates[2],
               mouseAxisStates[3]);
   return CT_SUCCESS;
}
