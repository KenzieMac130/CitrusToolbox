/*
   Copyright 2022 MacKenzie Strand

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

typedef SDL_Window* ctWindow;

enum ctCursorMode {
   CT_CURSOR_MODE_DEFAULT,
   CT_CURSOR_MODE_RELATIVE,
   CT_CURSOR_MODE_COUNT
};

class CT_API ctWindowManager : public ctModuleBase {
public:
   ctWindowManager();

   ctResults Startup() final;
   ctResults Shutdown() final;
   const char* GetModuleName() final;

   ctResults ShowErrorMessage(const char* title, const char* msg);
   ctResults ShowMainWindow();

   ctWindow mainWindow;
   ctResults GetMainWindowDrawableSize(int32_t* pWidth, int32_t* pHeight);

   ctCursorMode GetCursorMode();
   ctResults SetCursorMode(ctCursorMode);

   int32_t mainDesiredWindowWidth;
   int32_t mainDesiredWindowHeight;
   int32_t mainWindowMonitorIdx;
   int32_t mainWindowVSync;
   ctStringUtf8 mainWindowMode;
};