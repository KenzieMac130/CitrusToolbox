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

ctResults ctInteractSDLKeyboardMouseBackend::Startup() {
   ctDebugLog("Starting SDL Keyboard and Mouse...");
   ctInteractAbstractDevice* devices[] = {&keyboard, &mouse};
   ConnectDeviceWithChildren(devices, ctCStaticArrayLen(devices), 0);
   return CT_SUCCESS;
}

ctResults ctInteractSDLKeyboardMouseBackend::Shutdown() {
   DisconnectDevice(&keyboard);
   return CT_SUCCESS;
}

ctStringUtf8 ctInteractSDLKeyboardMouseBackend::GetName() {
   return ctStringUtf8();
}

ctStringUtf8 ctInteractSDLKeyboardMouseBackend::GetDescription() {
   return ctStringUtf8();
}

bool ctInteractSDLMouseDevice::isActionsHandled() {
   return true;
}

ctResults ctInteractSDLMouseDevice::PumpActions(ctInteractActionInterface&) {
   return ctResults();
}

bool ctInteractSDLMouseDevice::isCursorHandled() {
   return true;
}

ctResults ctInteractSDLMouseDevice::PumpCursor(ctInteractCursorInterface&) {
   return ctResults();
}

ctStringUtf8 ctInteractSDLMouseDevice::GetName() {
   return "Mouse";
}

ctStringUtf8 ctInteractSDLMouseDevice::GetPath() {
   return "/devices/mouse/default";
}

ctResults ctInteractSDLMouseDevice::LoadInputBindings(const char* basePath) {
   ctStringUtf8 fullPath = basePath;
   fullPath += "/devices/mouse/default.json";
   ctInteractInternalBindingLoader bindLoader = ctInteractInternalBindingLoader();
   CT_RETURN_FAIL(bindLoader.LoadFile(fullPath.CStr()));
   return CT_SUCCESS;
}

bool ctInteractSDLKeyboardDevice::isActionsHandled() {
   return true;
}

ctResults ctInteractSDLKeyboardDevice::PumpActions(ctInteractActionInterface&) {
   return ctResults();
}

bool ctInteractSDLKeyboardDevice::isTextHandled() {
   return true;
}

ctResults ctInteractSDLKeyboardDevice::PumpText(ctInteractTextInterface&) {
   return ctResults();
}

ctStringUtf8 ctInteractSDLKeyboardDevice::GetName() {
   return "Keyboard";
}

ctStringUtf8 ctInteractSDLKeyboardDevice::GetPath() {
   return "/devices/keyboard/default";
}

ctResults ctInteractSDLKeyboardDevice::LoadInputBindings(const char* basePath) {
   ctStringUtf8 fullPath = basePath;
   fullPath += "/devices/keyboard/default.json";
   ctInteractInternalBindingLoader bindLoader = ctInteractInternalBindingLoader();
   CT_RETURN_FAIL(bindLoader.LoadFile(fullPath.CStr()));
   return CT_SUCCESS;
}