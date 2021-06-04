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

#include "SDLGamepad.hpp"
#include "core/EngineCore.hpp"

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
      case SDL_CONTROLLERDEVICEREMAPPED: {
         pGamepadBackend->OnRemapController(event->cdevice.which);
         return;
      }
      default: break;
   }
}

ctResults ctInteractSDLGamepadBackend::Startup() {
   ctDebugLog("Starting SDL Gamepad...");
   SDL_Init(SDL_INIT_GAMECONTROLLER);
   this->Engine->OSEventManager->MiscEventHandlers.Append(
     {SDLGamecontrollerOnEvent, this});
   return CT_SUCCESS;
}

ctResults ctInteractSDLGamepadBackend::Shutdown() {
   return CT_SUCCESS;
}

ctStringUtf8 ctInteractSDLGamepadBackend::GetName() {
   return CT_NC("SDL Game Controller");
}

ctStringUtf8 ctInteractSDLGamepadBackend::GetDescription() {
   return CT_NC("");
}

void ctInteractSDLGamepadBackend::AddController(int32_t id) {
   if (SDL_IsGameController(id)) {
      SDL_GameController* sdlController = SDL_GameControllerOpen(id);
      ctInteractSDLGamepadDevice* pDevice = new ctInteractSDLGamepadDevice(sdlController);
      int32_t insId = SDL_JoystickGetDeviceInstanceID(id);
      ctDebugLog("Controller Added (%d) %s", insId, pDevice->GetName().CStr());
      controllers.Insert(insId + 1, pDevice);
      ConnectDevice(pDevice, SDL_GameControllerGetPlayerIndex(sdlController));
      // Todo: load bindings from profile
   }
}

void ctInteractSDLGamepadBackend::RemoveController(int32_t id) {
   ctInteractSDLGamepadDevice** ppController = controllers.FindPtr(id + 1);
   if (ppController) {
      ctDebugLog("Controller Removed: (%d) %s", id, (*ppController)->GetName().CStr());
      DisconnectDevice(*ppController);
      delete *ppController;
   }
   controllers.Remove(id);
}

void ctInteractSDLGamepadBackend::OnRemapController(int32_t id) {
   ctInteractSDLGamepadDevice** ppController = controllers.FindPtr(id + 1);
   if (ppController) {
      ctDebugLog("Controller Remapped: (%d) %s", id, (*ppController)->GetName().CStr());
      DisconnectDevice(*ppController);
      ConnectDevice(*ppController,
                    SDL_GameControllerGetPlayerIndex((*ppController)->gameController));
   }
}

ctInteractSDLGamepadDevice::ctInteractSDLGamepadDevice(SDL_GameController* controller) {
   gameController = controller;
   deviceName = SDL_GameControllerName(controller);
}

bool ctInteractSDLGamepadDevice::isActionsHandled() {
   return true;
}

ctResults ctInteractSDLGamepadDevice::PumpActions(ctInteractActionInterface& actions) {
   ctDebugLog("Hello");
   return CT_SUCCESS;
}

ctStringUtf8 ctInteractSDLGamepadDevice::GetName() {
   return deviceName;
}

ctStringUtf8 ctInteractSDLGamepadDevice::GetPath() {
   SDL_GameControllerType type = SDL_GameControllerGetType(gameController);
   switch (type) {
      case SDL_CONTROLLER_TYPE_PS3: return "/devices/gamepad/types/sony/dualshock3";
      case SDL_CONTROLLER_TYPE_PS4: return "/devices/gamepad/types/sony/dualshock4";
      case SDL_CONTROLLER_TYPE_PS5: return "/devices/gamepad/types/sony/dualsense";
      case SDL_CONTROLLER_TYPE_XBOX360: return "/devices/gamepad/types/microsoft/xbox360";
      case SDL_CONTROLLER_TYPE_XBOXONE: return "/devices/gamepad/types/microsoft/xboxone";
      case SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO:
         return "/devices/gamepad/types/nintendo/switch/default";
      default: return "/devices/gamepad/default";
   }
}
