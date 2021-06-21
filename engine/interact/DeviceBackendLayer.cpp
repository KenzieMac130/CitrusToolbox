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

ctResults ctInteractAbstractDevice::LoadInputBindings(const char* basePath) {
   ctDebugWarning("No Bindings Specified for Device");
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
   ctDynamicArray<ctInteractDeviceBinding>& bindings = Engine->Interact->DeviceBindings;
   for (int i = 0; i < bindings.Count(); i++) {
      if (bindings[i].GetDevicePtr() == pDevice) {
         bindings[i].Unbind();
         bindings.RemoveAt(i);
         return CT_SUCCESS;
      }
   }
   return CT_FAILURE_NOT_FOUND;
}

ctResults ctInteractAbstractBackend::ConnectDeviceWithChildren(
  ctInteractAbstractDevice** ppDevice, size_t deviceCount, int32_t wantsPlayerId) {
   ctDynamicArray<ctInteractDeviceBinding>& bindings = Engine->Interact->DeviceBindings;
   bool found = false;
   for (int i = 0; i < bindings.Count(); i++) {
      if (bindings[i].GetDevicePtr() == ppDevice[0]) { found = true; }
   }
   if (found) { return CT_FAILURE_DUPLICATE_ENTRY; }

   ctInteractDeviceBinding binding = ctInteractDeviceBinding(deviceCount, ppDevice);
   if (wantsPlayerId >= 0 && wantsPlayerId < CT_MAX_PLAYERS) {
      ctStringUtf8 configPath = Engine->FileSystem->GetAssetPath();
      configPath += "input";
      configPath.FilePathUnify();
      binding.BindToPlayer(wantsPlayerId, configPath.CStr());
   }
   bindings.Append(binding);
   return CT_SUCCESS;
}

ctResults ctInteractInternalBindingLoader::LoadFile(const char* path) {
   CT_RETURN_FAIL(file.Open(path, CT_FILE_OPEN_READ));
   size_t size = file.GetFileSize();
   text = (char*)ctMalloc(size);
   ctAssert(text);
   memset(text, 0, size);
   file.ReadRaw(text, size, 1);
   file.Close();
   json.BuildJsonForPtr(text, size);
   return CT_SUCCESS;
}

ctInteractInternalBindingLoader::~ctInteractInternalBindingLoader() {
   if (text) { ctFree(text); }
}

ctResults ctInteractInternalBindingLoader::PopulateActionMap(
  ctInteractInternalBindingActionMap& outMap,
  ctInteractInternalBindingTranslateBindPathFn fpRemap) {

   ctJSONReadEntry rootJson = ctJSONReadEntry();
   json.GetRootEntry(rootJson);

   /* Get action set array */
   ctJSONReadEntry actionSetArrayEntry = ctJSONReadEntry();
   CT_RETURN_FAIL(rootJson.GetObjectEntry("actionSets", actionSetArrayEntry));

   int actionSetCount = actionSetArrayEntry.GetArrayLength();
   for (int i = 0; i < actionSetCount; i++) {
       /* Get action set */
       ctJSONReadEntry actionSetEntry = ctJSONReadEntry();
       CT_RETURN_FAIL(actionSetArrayEntry.GetArrayEntry(i, actionSetEntry));

       /* Get action set name */
       ctJSONReadEntry actionSetNameEntry = ctJSONReadEntry();
       ctStringUtf8 actionSetName;
       actionSetEntry.GetObjectEntry("setName", actionSetNameEntry);
       actionSetEntry.GetString(actionSetName);

       /* Get action array */
       ctJSONReadEntry actionArrayEntry = ctJSONReadEntry();
       CT_RETURN_FAIL(actionSetEntry.GetObjectEntry("actions", actionArrayEntry));
       int actionCount = actionSetArrayEntry.GetArrayLength();
       for (int j = 0; j < actionCount; j++) {
           /* Get action */
           ctJSONReadEntry actionEntry = ctJSONReadEntry();
           CT_RETURN_FAIL(actionSetArrayEntry.GetArrayEntry(i, actionEntry));

           /* Get action name */
           ctJSONReadEntry actionNameEntry = ctJSONReadEntry();
           ctStringUtf8 actionName;
           actionEntry.GetObjectEntry("actionName", actionNameEntry);
           actionEntry.GetString(actionName);

           /* Get binding array */
           ctJSONReadEntry bindingArrayEntry = ctJSONReadEntry();
           CT_RETURN_FAIL(actionEntry.GetObjectEntry("bindings", bindingArrayEntry));
           int actionCount = actionSetArrayEntry.GetArrayLength();
           for (int k = 0; k < actionCount; k++) {
           }
       }
   }
   return CT_SUCCESS;
}