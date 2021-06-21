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
#include "core/ModuleBase.hpp"
#include "core/FileSystem.hpp"

#include "interfaces/Actions.hpp"
#include "interfaces/Cursor.hpp"
#include "interfaces/Message.hpp"
#include "interfaces/Text.hpp"

struct CT_API ctInteractInternalBinding {
    float deadzoneInner;
    float deadzoneOuter;
    float scale;
    uint64_t scancode;
};

typedef ctInteractInternalBinding ctInteractInternalBindingList[CT_MAX_ACTION_BINDS];

class CT_API ctInteractInternalBindingActionMap {
public:
   void AddBind(const char* set, const char* action, ctInteractInternalBindingList internalRep);
   bool GetBind(const char* set, const char* action, ctInteractInternalBindingList& result);

private:
   ctHashTable<uint64_t, ctInteractInternalBindingList> mapping;
};

typedef ctResults(*ctInteractInternalBindingTranslateBindPathFn)(const char* path, uint64_t* out);

  class CT_API ctInteractInternalBindingLoader {
public:
   virtual ctResults LoadFile(const char* path);
   ~ctInteractInternalBindingLoader();
   virtual ctResults PopulateActionMap(ctInteractInternalBindingActionMap& outMap,
                                       ctInteractInternalBindingTranslateBindPathFn fpRemap);

private:
   ctFile file;
   char* text;
   ctJSONReader json;
};

class CT_API ctInteractAbstractDevice {
public:
   virtual bool isActionsHandled();
   virtual bool isCursorHandled();
   virtual bool isTextHandled();
   virtual bool isMessageHandled();

   virtual ctResults PumpActions(class ctInteractActionInterface&);
   virtual ctResults PumpCursor(class ctInteractCursorInterface&);
   virtual ctResults PumpText(class ctInteractTextInterface&);
   virtual ctResults PumpMessage(class ctInteractMessageInterface&);

   virtual ctStringUtf8 GetName() = 0;
   virtual ctStringUtf8 GetPath() = 0;

   virtual ctResults LoadInputBindings(const char* basePath);
};

class CT_API ctInteractAbstractBackend : public ctModuleBase {
public:
   virtual ~ctInteractAbstractBackend() = default;
   virtual ctStringUtf8 GetName() = 0;
   virtual ctStringUtf8 GetDescription() = 0;
   virtual ctResults Update();
   virtual ctResults DebugImGui();

   ctResults ConnectDevice(ctInteractAbstractDevice* pDevice, int32_t wantsPlayerId = -1);
   ctResults DisconnectDevice(ctInteractAbstractDevice* pDevice);
   ctResults ConnectDeviceWithChildren(ctInteractAbstractDevice** ppDevices,
                                       size_t deviceCount,
                                       int32_t wantsPlayerId = -1);
};