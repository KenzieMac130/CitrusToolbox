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

#include "Application.hpp"
#include "FileSystem.hpp"
#include "Logging.hpp"
#include "Settings.hpp"
#include "WindowManager.hpp"
#include "renderer/Renderer.hpp"

class ctEngineCore {
public:
    /* Initialize the engine and all subsystems */
    ctResults Ignite(class ctApplication* pApp);
    /* Enter the game loop (returns on shutdown) */
    ctResults EnterLoop();
    /* Exit */
    void Exit();
    /* Is Running */
    bool isExitRequested();
    /* Single shot the game loop (returns after frame) */
    ctResults LoopSingleShot(const float deltatime);
    ctResults Shutdown();

    class ctApplication* App;
    ctFileSystem* FileSystem;
    ctDebugSystem* Debug;
    ctWindowManager* WindowManager;
    ctRenderer* Renderer;
private:
    bool _isRunning = true;
};