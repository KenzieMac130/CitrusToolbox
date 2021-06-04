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

#include "core/EngineCore.hpp"
#include "DeviceBackendLayer.hpp"

bool ctInteractAbstractDevice::isActionsHandled() {
   return false;
}

bool ctInteractAbstractDevice::isCursorHandled() {
   return false;
}

bool ctInteractAbstractDevice::isTextHandled() {
   return false;
}

bool ctInteractAbstractDevice::isMessageHandled() {
   return false;
}

ctResults ctInteractAbstractDevice::PumpActions(ctInteractActionInterface&) {
   return CT_SUCCESS;
}

ctResults ctInteractAbstractDevice::PumpCursor(ctInteractCursorInterface&) {
   return CT_SUCCESS;
}

ctResults ctInteractAbstractDevice::PumpText(ctInteractTextInterface&) {
   return CT_SUCCESS;
}

ctResults ctInteractAbstractDevice::PumpMessage(ctInteractMessageInterface&) {
   return CT_SUCCESS;
}

ctResults ctInteractAbstractBackend::Update() {
   return CT_SUCCESS;
}

ctResults ctInteractAbstractBackend::DebugImGui() {
   return CT_SUCCESS;
}

ctResults ctInteractAbstractBackend::ConnectDevice(ctInteractAbstractDevice* pDevice,
                                                   int32_t wantsPlayerId) {
   return ConnectDeviceWithChildren(&pDevice, 1, wantsPlayerId);
}

ctResults ctInteractAbstractBackend::DisconnectDevice(ctInteractAbstractDevice* pDevice) {
   ctDynamicArray<ctInteractDeviceBindings>& bindings = Engine->Interact->DeviceBindings;
   for (int i = 0; i < bindings.Count(); i++) {
      if (bindings[i].GetDevicePtr() == pDevice) {
         bindings[i].Unbind();
         bindings.RemoveAt(i);
         return CT_SUCCESS;
      }
   }
   return CT_FAILURE_NOT_FOUND;
}

ctResults
ctInteractAbstractBackend::ConnectDeviceWithChildren(ctInteractAbstractDevice** ppDevice,
                                               size_t deviceCount,
                                               int32_t wantsPlayerId) {
   ctDynamicArray<ctInteractDeviceBindings>& bindings = Engine->Interact->DeviceBindings;
   bool found = false;
   for (int i = 0; i < bindings.Count(); i++) {
      if (bindings[i].GetDevicePtr() == ppDevice[0]) { found = true; }
   }
   if (found) { return CT_FAILURE_DUPLICATE_ENTRY; }

   ctInteractDeviceBindings binding = ctInteractDeviceBindings(deviceCount, ppDevice);
   if (wantsPlayerId >= 0 && wantsPlayerId < CT_MAX_PLAYERS) {
      binding.BindToPlayer(wantsPlayerId);
   }
   bindings.Append(binding);
   return CT_SUCCESS;
}
