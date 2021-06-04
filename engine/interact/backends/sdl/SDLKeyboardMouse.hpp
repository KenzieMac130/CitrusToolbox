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

class ctInteractSDLKeyboardDevice : public ctInteractAbstractDevice {
public:
    virtual bool isActionsHandled();
    virtual ctResults PumpActions(class ctInteractActionInterface&);
    virtual bool isTextHandled();
    virtual ctResults PumpText(class ctInteractTextInterface&);

    virtual ctStringUtf8 GetName();
    virtual ctStringUtf8 GetPath();
};

class ctInteractSDLMouseDevice : public ctInteractAbstractDevice {
public:
    virtual bool isActionsHandled();
    virtual ctResults PumpActions(class ctInteractActionInterface&);
    virtual bool isCursorHandled();
    virtual ctResults PumpCursor(class ctInteractCursorInterface&);

    virtual ctStringUtf8 GetName();
    virtual ctStringUtf8 GetPath();
};

class ctInteractSDLKeyboardMouseBackend : public ctInteractAbstractBackend {
    ctResults Startup() final;
    ctResults Shutdown() final;
    ctStringUtf8 GetName() final;
    ctStringUtf8 GetDescription() final;
private:
    ctInteractSDLKeyboardDevice keyboard;
    ctInteractSDLMouseDevice mouse;
};