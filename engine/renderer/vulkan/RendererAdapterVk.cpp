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

#include "utilities/Common.h"
#include "renderer/Renderer.hpp"
#include "core/EngineCore.hpp"

class ctVkBackend {
public:
   int32_t width;
   int32_t height;
   int32_t centerMonitorIdx;
   ctStringUtf8 windowMode;
};

ctResults ctRenderer::Startup() {
   vkBackend = new ctVkBackend();

   ctSettingsSection* settings = Engine->Settings->CreateSection("Renderer", 16);
   settings->BindInteger(
     &vkBackend->width, true, true, "WindowWidth", "Width of the main window.");
   settings->BindInteger(&vkBackend->height,
                         true,
                         true,
                         "WindowHeight",
                         "Height of the main window.");
   settings->BindInteger(&vkBackend->centerMonitorIdx,
                         true,
                         true,
                         "MonitorIndex",
                         "Target monitor to place the window in.");
   settings->BindString(&vkBackend->windowMode,
                        true,
                        true,
                        "WindowMode",
                        "Main Window Mode. (Windowed, Resizable, Borderless)");
   return CT_SUCCESS;
};
ctResults ctRenderer::Shutdown() {
   delete vkBackend;
   return CT_SUCCESS;
};