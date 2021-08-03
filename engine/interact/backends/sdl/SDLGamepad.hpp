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

struct ctInteractSDLGamepadInstance {
   SDL_GameController* controller;
   int32_t controllerId;
   int32_t idx;
};

struct ctInteractSDLGamepadPlayerContent {
   union {
      struct {
         float a;
         float b;
         float x;
         float y;
         float dpad_up;
         float dpad_down;
         float dpad_left;
         float dpad_right;
         float shoulder_left;
         float shoulder_right;
         float trigger_left;
         float trigger_right;
         float thumbstick_left_click;
         float thumbstick_right_click;
         float thumbstick_left_x;
         float thumbstick_left_y;
         float thumbstick_right_x;
         float thumbstick_right_y;
         float select;
         float start;
         float guide;
      };
      float data[21];
   };
};

class CT_API ctInteractSDLGamepadBackend : public ctInteractAbstractBackend {
public:
   ctInteractSDLGamepadBackend(class ctOSEventManager* pOSEvents);
   ctResults Startup() final;
   ctResults Shutdown() final;

   virtual ctResults Register(class ctInteractDirectorySystem& directory);
   virtual ctResults Update(double deltaTime, class ctInteractDirectorySystem& directory);

   void AddController(int32_t id);
   void RemoveController(int32_t id);
   void OnRemapController(int32_t id);

private:
   ctInteractSDLGamepadPlayerContent gamepads[4] = {0};
};