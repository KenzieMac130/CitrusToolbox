#include "WindowManager.hpp"
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

#include "WindowManager.hpp"
#include "EngineCore.hpp"

ctWindowManager::ctWindowManager() {
}

ctWindowManager::~ctWindowManager() {
}

ctResults ctWindowManager::Startup() {
   return CT_SUCCESS;
}

ctResults ctWindowManager::Shutdown() {
   for (int i = 0; i < windows.Count(); i++) {
      DestroyWindow(windows[i]);
   }
   return CT_SUCCESS;
}

uint32_t windowModeFlags(ctWindowMode windowMode) {
   switch (windowMode) {
      case CT_WINDOWMODE_WINDOWED: return 0;
      case CT_WINDOWMODE_WINDOWED_RESIZABLE: return SDL_WINDOW_RESIZABLE;
      case CT_WINDOWMODE_FULLSCREEN: return SDL_WINDOW_FULLSCREEN;
      case CT_WINDOWMODE_FULLSCREEN_DESKTOP:
         return SDL_WINDOW_FULLSCREEN_DESKTOP;
      case CT_WINDOWMODE_FULLSCREEN_BORDERLESS: return SDL_WINDOW_BORDERLESS;
      default: return 0;
   }
}

ctResults ctWindowManager::CreateWindow(ctWindow** ppWindow,
                                        const char* name,
                                        int32_t monitor,
                                        int32_t w,
                                        int32_t h,
                                        ctWindowMode windowMode) {
    uint32_t flags = windowModeFlags(windowMode);
#if CT_GFX_VULKAN
    flags |= SDL_WINDOW_VULKAN;
#endif
    SDL_Rect windowDim;
    if (windowMode == CT_WINDOWMODE_FULLSCREEN_BORDERLESS) {
        SDL_GetDisplayBounds(monitor, &windowDim);
    }
    else {
        windowDim.x = SDL_WINDOWPOS_CENTERED_DISPLAY(monitor);
        windowDim.y = SDL_WINDOWPOS_CENTERED_DISPLAY(monitor);
        windowDim.w = w;
        windowDim.h = h;
    }
    ctWindow* window = SDL_CreateWindow(
        name, windowDim.x, windowDim.y, windowDim.w, windowDim.h, flags);
    if (!window) { return CT_FAILURE_UNKNOWN; }
    SDL_SetWindowMinimumSize(window, 640, 480);
    windows.Append(window);
    if (ppWindow) { *ppWindow = window; }
    return CT_SUCCESS;
}

ctResults ctWindowManager::CreateNativeWindow(ctWindow* pWindow,
                                              const void* pData) {
   ctWindow* window = SDL_CreateWindowFrom(pData);
   if (!window) { return CT_FAILURE_UNKNOWN; }
   windows.Append(window);
   return CT_SUCCESS;
}

ctResults ctWindowManager::DestroyWindow(ctWindow* pWindow) {
   windows.Remove(pWindow);
   return CT_SUCCESS;
}

int32_t ctWindowManager::GetWindowCount() {
   return (int32_t)windows.Count();
}

ctWindow* ctWindowManager::GetWindow(int32_t index) {
   return windows[index];
}
