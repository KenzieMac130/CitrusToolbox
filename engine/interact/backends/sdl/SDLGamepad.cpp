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

#include "SDLGamepad.hpp"
#include "core/EngineCore.hpp"
#include "core/OSEvents.hpp"
#include "interact/InteractionEngine.hpp"

void SDLGamecontrollerOnEvent(SDL_Event* event, void* data) {
   ctInteractSDLGamepadBackend* pGamepadBackend = (ctInteractSDLGamepadBackend*)data;
   switch (event->type) {
      case SDL_CONTROLLERDEVICEADDED: {
         pGamepadBackend->AddController(event->cdevice.which);
         return;
      }
      case SDL_CONTROLLERDEVICEREMOVED: {
         pGamepadBackend->RemoveController(event->cdevice.which);
         return;
      }
      //case SDL_CONTROLLERDEVICEREMAPPED: {
      //   pGamepadBackend->OnRemapController(event->cdevice.which);
      //   return;
      //}
      default: break;
   }
}

ctResults ctInteractSDLGamepadBackend::Startup() {
   ZoneScoped;
   ctDebugLog("Starting SDL Gamepad...");
   SDL_Init(SDL_INIT_GAMECONTROLLER);
   Engine->OSEventManager->MiscEventHandlers.Append({SDLGamecontrollerOnEvent, this});
   return CT_SUCCESS;
}

ctResults ctInteractSDLGamepadBackend::Shutdown() {
   ZoneScoped;
   return CT_SUCCESS;
}

const char* ctInteractSDLGamepadBackend::GetModuleName() {
   return "SDL Gamepad";
}

ctResults ctInteractSDLGamepadBackend::Register(ctInteractDirectorySystem& directory) {
   ZoneScoped;
   /* Add mouse inputs */
   char* inputPaths[] = {
     "/input/a",
     "/input/b",
     "/input/x",
     "/input/y",
     "/input/dpad/up",
     "/input/dpad/down",
     "/input/dpad/left",
     "/input/dpad/right",
     "/input/shoulder_left",
     "/input/shoulder_right",
     "/input/trigger_left",
     "/input/trigger_right",
     "/input/thumbstick_left/click",
     "/input/thumbstick_right/click",
     "/input/thumbstick_left/x",
     "/input/thumbstick_left/y",
     "/input/thumbstick_right/x",
     "/input/thumbstick_right/y",
     "/input/select",
     "/input/start",
     "/input/guide",
   };
   for (int c = 0; c < 4; c++) {
      for (int i = 0; i < 21; i++) {
         ctInteractNode node = ctInteractNode();
         node.type = CT_INTERACT_NODETYPE_SCALAR;
         node.accessible = true;
         snprintf(node.path.str,
                  CT_MAX_INTERACT_PATH_SIZE,
                  "dev/gamepad/%d%s",
                  c,
                  inputPaths[i]);
         node.pData = &gamepads[c].data[i];
         directory.AddNode(node);
      }
   }
   ctFile file;
   Engine->FileSystem->OpenDataFileByGUID(
     file, CT_CDATA("Input_Gamepad"), CT_FILE_OPEN_READ_TEXT);
   directory.CreateBindingsFromFile(file);
   file.Close();
#if CITRUS_INCLUDE_AUDITION
   directory.configHotReload.RegisterData(CT_CDATA("Input_Gamepad"));
#endif
   ctFile gcontrollerdb;
   Engine->FileSystem->OpenDataFileByGUID(
     gcontrollerdb, CT_CDATA("Input_GamecontrollerDB"), CT_FILE_OPEN_READ_TEXT);
   ctStringUtf8 gcontrollerdbtext = "";
   gcontrollerdb.GetText(gcontrollerdbtext);
   SDL_GameControllerAddMapping(gcontrollerdbtext.CStr());
   return CT_SUCCESS;
}

#define GET_BUTTON(button)                                                               \
   SDL_GameControllerGetButton(gamepad.controller, button) ? 1.0f : 0.0f
#define GET_AXIS(axis)                                                                   \
   (float)SDL_GameControllerGetAxis(gamepad.controller, axis) / 32767.0f

ctResults ctInteractSDLGamepadBackend::Update(ctInteractDirectorySystem& directory) {
   ZoneScoped;
   for (int i = 0; i < 4; i++) {
      ctInteractSDLGamepadPlayerContent& gamepad = gamepads[i];
      gamepad.a = GET_BUTTON(SDL_CONTROLLER_BUTTON_A);
      gamepad.b = GET_BUTTON(SDL_CONTROLLER_BUTTON_B);
      gamepad.x = GET_BUTTON(SDL_CONTROLLER_BUTTON_X);
      gamepad.y = GET_BUTTON(SDL_CONTROLLER_BUTTON_Y);

      gamepad.dpad_up = GET_BUTTON(SDL_CONTROLLER_BUTTON_DPAD_UP);
      gamepad.dpad_down = GET_BUTTON(SDL_CONTROLLER_BUTTON_DPAD_DOWN);
      gamepad.dpad_left = GET_BUTTON(SDL_CONTROLLER_BUTTON_DPAD_LEFT);
      gamepad.dpad_right = GET_BUTTON(SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
      gamepad.shoulder_left = GET_BUTTON(SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
      gamepad.shoulder_right = GET_BUTTON(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);

      gamepad.thumbstick_left_click = GET_BUTTON(SDL_CONTROLLER_BUTTON_LEFTSTICK);
      gamepad.thumbstick_right_click = GET_BUTTON(SDL_CONTROLLER_BUTTON_RIGHTSTICK);

      gamepad.select = GET_BUTTON(SDL_CONTROLLER_BUTTON_BACK);
      gamepad.start = GET_BUTTON(SDL_CONTROLLER_BUTTON_START);
      gamepad.guide = GET_BUTTON(SDL_CONTROLLER_BUTTON_GUIDE);

      gamepad.trigger_left = GET_AXIS(SDL_CONTROLLER_AXIS_TRIGGERLEFT);
      gamepad.trigger_right = GET_AXIS(SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
      gamepad.thumbstick_left_x = GET_AXIS(SDL_CONTROLLER_AXIS_LEFTX);
      gamepad.thumbstick_left_y = GET_AXIS(SDL_CONTROLLER_AXIS_LEFTY);
      gamepad.thumbstick_right_x = GET_AXIS(SDL_CONTROLLER_AXIS_RIGHTX);
      gamepad.thumbstick_right_y = GET_AXIS(SDL_CONTROLLER_AXIS_RIGHTY);
   }
   return CT_SUCCESS;
}

void ctInteractSDLGamepadBackend::AddController(int32_t id) {
   ZoneScoped;
   if (SDL_IsGameController(id)) {
      SDL_GameController* sdlController = SDL_GameControllerOpen(id);
      int32_t insId = SDL_JoystickGetDeviceInstanceID(id);
      ctDebugLog("Controller Added (%d)", insId);
      int playerIdx = SDL_GameControllerGetPlayerIndex(sdlController);
      /* Exceeds maximum*/
      if (playerIdx >= 4) {
         SDL_GameControllerClose(sdlController);
         return;
      }
      /* Unknown bind (find empty slot) */
      if (playerIdx < 0) {
         for (int i = 0; i < 4; i++) {
            if (gamepads[i].controller == NULL) {
               playerIdx = i;
               SDL_GameControllerSetPlayerIndex(sdlController, i);
            }
         }
      }
      gamepads[playerIdx].controller = sdlController;
      gamepads[playerIdx].controllerId = insId;
   }
}

void ctInteractSDLGamepadBackend::RemoveController(int32_t id) {
   ZoneScoped;
   for (int i = 0; i < 4; i++) {
      if (gamepads[i].controllerId == id) {
         ctDebugLog("Controller Removed (%d)", id);
         SDL_GameControllerClose(gamepads[i].controller);
         memset(&gamepads[i], 0, sizeof(gamepads[0]));
      }
   }
}

//void ctInteractSDLGamepadBackend::OnRemapController(int32_t id) {
//   ZoneScoped;
//   // for (int i = 0; i < 4; i++) {
//   //   if (gamepads[i].controllerId == id) { remaps.Append({gamepads[i].controller, id});
//   //   }
//   //}
//}
