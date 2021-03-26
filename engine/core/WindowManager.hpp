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
#include "ModuleBase.hpp"

#include "SDL_video.h"

class ctWindow {
public:
    SDL_Window* pSDLWindow;
};

class ctWindowManager : public ctModuleBase {
public:
    ctWindowManager();

    ctResults Startup() final;
    ctResults Shutdown() final;

    ctResults ShowErrorMessage(const char* title, const char* msg);

    ctWindow mainWindow;

    int32_t mainWindowWidth;
    int32_t mainWindowHeight;
    int32_t mainWindowMonitorIdx;
    ctStringUtf8 mainWindowMode;
};