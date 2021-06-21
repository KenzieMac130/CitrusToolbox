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
#include "DeviceBackendLayer.hpp"

#include "interfaces/Actions.hpp"
#include "interfaces/Cursor.hpp"
#include "interfaces/Message.hpp"
#include "interfaces/Text.hpp"
#include "interfaces/UserConfig.hpp"

class CT_API ctInteractPlayer {
public:
   ctInteractActionInterface Action;
   ctInteractCursorInterface Cursor;
   ctInteractTextInterface Text;
   ctInteractMessageInterface Message;
   ctInteractUserConfigInterface Config;
};

class CT_API ctInteractDeviceBinding {
public:
   ctInteractDeviceBinding();
   ctInteractDeviceBinding(class ctInteractAbstractDevice* device);
   ctInteractDeviceBinding(size_t subdeviceCount,
                            class ctInteractAbstractDevice** ppDevices);

   ctResults BindToPlayer(int32_t player, const char* configPath);
   ctResults Unbind();

   size_t GetSubdeviceCount();
   ctInteractAbstractDevice* GetDevicePtr(uint32_t subDevice = 0);
   int32_t GetPlayerIdx();

private:
   int32_t _playerIdx;
   ctStaticArray<ctInteractAbstractDevice*, CT_MAX_INTERACT_SUBDEVICES> _devices;
};

class CT_API ctInteractionEngine : public ctModuleBase {
public:
   ctResults Startup() final;
   ctResults Shutdown() final;

   ctResults PumpInput();
   ctResults DebugImGui();

   ctInteractPlayer Players[CT_MAX_PLAYERS];
   ctDynamicArray<ctInteractDeviceBinding> DeviceBindings;

protected:
   ctDynamicArray<ctInteractAbstractBackend*> pBackends;
};