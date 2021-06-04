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
#include "core/EngineCore.hpp"
#include "core/Application.hpp"

uint32_t windowModeFlags(const ctStringUtf8& windowMode) {
   if (windowMode == "Windowed") {
      return 0;
   } else if (windowMode == "Resizable") {
      return SDL_WINDOW_RESIZABLE;
   } else if (windowMode == "Fullscreen") {
      return SDL_WINDOW_FULLSCREEN;
   } else if (windowMode == "Desktop") {
      return SDL_WINDOW_FULLSCREEN_DESKTOP;
   } else if (windowMode == "Borderless") {
      return SDL_WINDOW_BORDERLESS;
   }
   return 0;
}

ctWindowManager::ctWindowManager() {
   mainWindowWidth = 640;
   mainWindowHeight = 480;
   mainWindowMonitorIdx = 0;
   mainWindowVSync = 1;
#ifdef NDEBUG
   mainWindowMode = "Borderless";
#else
   mainWindowMode = "Resizable";
#endif
   mainWindow = ctWindow();
}

ctResults ctWindowManager::Startup() {
   ZoneScoped;
   ctSettingsSection* settings = Engine->Settings->CreateSection("Window", 4);
   settings->BindInteger(&mainWindowWidth,
                         true,
                         true,
                         "WindowWidth",
                         "Width of the main window.",
                         CT_SETTINGS_BOUNDS_UINT);
   settings->BindInteger(&mainWindowHeight,
                         true,
                         true,
                         "WindowHeight",
                         "Height of the main window.",
                         CT_SETTINGS_BOUNDS_UINT);
   settings->BindInteger(&mainWindowMonitorIdx,
                         true,
                         true,
                         "MonitorIndex",
                         "Target monitor to place the window in.");
   settings->BindString(
     &mainWindowMode,
     true,
     true,
     "WindowMode",
     "Main Window Mode. (Windowed, Resizable, Fullscreen, Desktop, Borderless)");
   settings->BindInteger(&mainWindowVSync,
                         false,
                         true,
                         "VSync",
                         "Enable vertical sync.",
                         CT_SETTINGS_BOUNDS_BOOL);

#if !CITRUS_HEADLESS
   uint32_t flags = windowModeFlags(mainWindowMode);
#ifdef CITRUS_GFX_VULKAN
   flags |= SDL_WINDOW_VULKAN;
#endif
   flags |= SDL_WINDOW_HIDDEN;
   SDL_Rect windowDim;
   if (mainWindowMode == "Borderless") {
      SDL_GetDisplayBounds(mainWindowMonitorIdx, &windowDim);
   } else {
      windowDim.x = SDL_WINDOWPOS_CENTERED_DISPLAY(mainWindowMonitorIdx);
      windowDim.y = SDL_WINDOWPOS_CENTERED_DISPLAY(mainWindowMonitorIdx);
      windowDim.w = mainWindowWidth;
      windowDim.h = mainWindowHeight;
   }
   SDL_Window* window = SDL_CreateWindow(Engine->App->GetAppName(),
                                         windowDim.x,
                                         windowDim.y,
                                         windowDim.w,
                                         windowDim.h,
                                         flags);
   if (!window) { return CT_FAILURE_UNKNOWN; }
   SDL_SetWindowMinimumSize(window, 640, 480);
   mainWindow.pSDLWindow = window;
#endif
   return CT_SUCCESS;
}

ctResults ctWindowManager::Shutdown() {
#if !CITRUS_HEADLESS
   SDL_DestroyWindow(mainWindow.pSDLWindow);
#endif
   return CT_SUCCESS;
}

ctResults ctWindowManager::ShowErrorMessage(const char* title, const char* msg) {
#if !CITRUS_HEADLESS
   if (SDL_ShowSimpleMessageBox(
         SDL_MESSAGEBOX_ERROR, title, msg, mainWindow.pSDLWindow)) {
      return CT_FAILURE_UNKNOWN;
   }
#endif
   return CT_SUCCESS;
}

ctResults ctWindowManager::ShowMainWindow() {
#if !CITRUS_HEADLESS
   if (mainWindow.pSDLWindow) { SDL_ShowWindow(mainWindow.pSDLWindow); }
#endif
   return CT_SUCCESS;
}
