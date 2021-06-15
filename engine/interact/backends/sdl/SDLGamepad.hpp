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

#include "utilities/Common.h"
#include "interact/DeviceBackendLayer.hpp"

class ctInteractSDLGamepadDevice : public ctInteractAbstractDevice {
public:
   ctInteractSDLGamepadDevice(SDL_GameController* controller);
   virtual bool isActionsHandled();
   virtual ctResults PumpActions(class ctInteractActionInterface&);

   virtual ctResults LoadBindingsForPlayer(int32_t player);

   virtual ctStringUtf8 GetName();
   virtual ctStringUtf8 GetPath();

   SDL_GameController* gameController;
   ctStringUtf8 deviceName;
};

class ctInteractSDLGamepadBackend : public ctInteractAbstractBackend {
public:
   ctResults Startup() final;
   ctResults Shutdown() final;
   ctStringUtf8 GetName() final;
   ctStringUtf8 GetDescription() final;

   void AddController(int32_t id);
   void RemoveController(int32_t id);
   void OnRemapController(int32_t id);
private:
   ctHashTable<ctInteractSDLGamepadDevice*, int32_t> controllers;
};