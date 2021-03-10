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
#include "renderer/Renderer.hpp"
#include "ModuleBase.hpp"

#include "SDL_video.h"

enum ctWindowMode {
   CT_WINDOWMODE_WINDOWED,
   CT_WINDOWMODE_WINDOWED_RESIZABLE,
   CT_WINDOWMODE_FULLSCREEN,
   CT_WINDOWMODE_FULLSCREEN_DESKTOP,
   CT_WINDOWMODE_FULLSCREEN_BORDERLESS
};

/* Not guaranteed to always be SDL */
typedef SDL_Window ctWindow;

class ctWindowManager : public ctModuleBase {
public:
   ctWindowManager();
   ~ctWindowManager();

   ctResults Startup() final;
   ctResults Shutdown() final;

   ctResults CreateWindow(ctWindow** ppWindow,
                          const char* name,
                          int32_t monitor,
                          int32_t w,
                          int32_t h,
                          ctWindowMode windowMode);

   ctResults CreateNativeWindow(ctWindow* pWindow, const void* pData);

   ctResults DestroyWindow(ctWindow* pWindow);

   int32_t GetWindowCount();
   ctWindow* GetWindow(int32_t index);

protected:
   ctDynamicArray<ctWindow*> windows;
};