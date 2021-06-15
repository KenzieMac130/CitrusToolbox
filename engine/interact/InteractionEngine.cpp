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

#include "InteractionEngine.hpp"
#include "backends/BackendManifest.hpp"

ctResults ctInteractionEngine::Startup() {
   ctToggleInteractBackend("SdlGamepad", true);
   ctToggleInteractBackend("SdlKeyboardMouse", true);
   return ctStartAndRetrieveInteractBackends(Engine, pBackends);
}

ctResults ctInteractionEngine::Shutdown() {
   return ctShutdownInteractBackends();
}

ctResults ctInteractionEngine::PumpInput() {
   for (int i = 0; i < pBackends.Count(); i++) {
      pBackends[i]->Update();
   }
   for (int i = 0; i < DeviceBindings.Count(); i++) {
      ctInteractAbstractDevice* device = DeviceBindings[i].GetDevicePtr();
      int32_t playerIdx = DeviceBindings[i].GetPlayerIdx();
      if (!device || playerIdx < 0 || playerIdx >= CT_MAX_PLAYERS) { continue; }
      ctInteractPlayer& player = Players[playerIdx];
      if (device->isActionsHandled()) { device->PumpActions(player.Action); }
      if (device->isCursorHandled()) { device->PumpCursor(player.Cursor); }
      if (device->isTextHandled()) { device->PumpText(player.Text); }
      if (device->isMessageHandled()) { device->PumpMessage(player.Message); }
   }
   return CT_SUCCESS;
}

ctResults ctInteractionEngine::DebugImGui() {
   for (int i = 0; i < CT_MAX_PLAYERS; i++) {
      /* show interfaces */
   }
   for (int i = 0; i < DeviceBindings.Count(); i++) {
       /* todo: better debug vis */
       for (int j = 0; j < DeviceBindings[i].GetSubdeviceCount(); j++) {
           ctInteractAbstractDevice* pDevice = DeviceBindings[i].GetDevicePtr(j);
           if (!pDevice) { continue; }
           ImGui::Text("Name: %s\nPath: %s\nPlayer: %d",
                       pDevice->GetName().CStr(),
                       pDevice->GetPath().CStr(),
                       DeviceBindings[i].GetPlayerIdx());
       }
   }
   for (int i = 0; i < pBackends.Count(); i++) {
      pBackends[i]->DebugImGui();
   }
   return CT_SUCCESS;
}

ctInteractDeviceBindings::ctInteractDeviceBindings() {
   _playerIdx = -1;
}

ctInteractDeviceBindings::ctInteractDeviceBindings(ctInteractAbstractDevice* device) {
   _playerIdx = -1;
   _devices.Append(device);
}

ctInteractDeviceBindings::ctInteractDeviceBindings(size_t subdeviceCount,
                                                   ctInteractAbstractDevice** ppDevices) {
   _playerIdx = -1;
   for (size_t i = 0; i < subdeviceCount; i++) {
      _devices.Append(ppDevices[i]);
   }
}

ctResults ctInteractDeviceBindings::BindToPlayer(int32_t player) {
   if (player < 0 && player >= CT_MAX_PLAYERS) { return CT_FAILURE_OUT_OF_BOUNDS; }
   if (!GetDevicePtr()) { return CT_FAILURE_CORRUPTED_CONTENTS; }
   _playerIdx = player;
   // Todo: load bindings from profile for a player
   GetDevicePtr()->LoadBindingsForPlayer(player);
   ctDebugLog("Bound Player %d to %s", player, GetDevicePtr()->GetName().CStr());
   return CT_SUCCESS;
}

ctResults ctInteractDeviceBindings::Unbind() {
   ctDebugLog("Unbound Player %d from %s", _playerIdx, GetDevicePtr()->GetName().CStr());
   _playerIdx = -1;
   return CT_SUCCESS;
}

size_t ctInteractDeviceBindings::GetSubdeviceCount() {
   return _devices.Count();
}

ctInteractAbstractDevice* ctInteractDeviceBindings::GetDevicePtr(uint32_t subDevice) {
   if (subDevice >= _devices.Count()) { return NULL; }
   return _devices[subDevice];
}

int32_t ctInteractDeviceBindings::GetPlayerIdx() {
   return _playerIdx;
}
