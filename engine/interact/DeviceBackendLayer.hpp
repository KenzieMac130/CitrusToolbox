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

#include "interfaces/Actions.hpp"
#include "interfaces/Cursor.hpp"
#include "interfaces/Message.hpp"
#include "interfaces/Text.hpp"

class ctInteractInternalBindingLoader {
    virtual ctResults LoadDevice(const char* path);
    virtual ctResults LoadOverride(const char* profile);

private:
    struct actionInfo {
        char name[64];
    };
    class _actionSet {
        char name[64];
        /* Maps a binding to a unique action */
        ctHashTable<actionInfo, uint32_t> bindingToAction;
    };
    ctDynamicArray<_actionSet> actionSets;
};

class ctInteractAbstractDevice {
public:
   virtual bool isActionsHandled();
   virtual bool isCursorHandled();
   virtual bool isTextHandled();
   virtual bool isMessageHandled();

   virtual ctResults PumpActions(class ctInteractActionInterface&);
   virtual ctResults PumpCursor(class ctInteractCursorInterface&);
   virtual ctResults PumpText(class ctInteractTextInterface&);
   virtual ctResults PumpMessage(class ctInteractMessageInterface&);

   virtual ctResults LoadBindingsForPlayer(int32_t player);

   virtual ctStringUtf8 GetName() = 0;
   virtual ctStringUtf8 GetPath() = 0;
};

class ctInteractAbstractBackend : public ctModuleBase {
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