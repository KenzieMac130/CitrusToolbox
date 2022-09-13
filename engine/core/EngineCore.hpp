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

class CT_API ctEngineCore {
public:
   /* Initialize the engine and all subsystems */
   ctResults Ignite(class ctApplication* pApp, int argc, char* argv[]);
   /* Enter the game loop (returns on shutdown) */
   ctResults EnterLoop();
   /* Exit */
   void Exit();
   /* Is Running */
   bool isExitRequested() const;
   /* Single shot the game loop (returns after frame) */
   ctResults LoopSingleShot(const float deltatime);
   ctResults Shutdown();

   class ctApplication* App;
   class ctAsyncManager* AsyncTasks;
   class ctJobSystem* JobSystem;
   class ctOSEventManager* OSEventManager;
   class ctTranslation* Translation;
   class ctFileSystem* FileSystem;
   class ctSettingsManager* Settings;
#if CITRUS_INCLUDE_AUDITION
   class ctHotReloadDetection* HotReload;
#endif
   class ctDebugSystem* Debug;
   class ctWindowManager* WindowManager;
   class ctInteractionEngine* Interact;
   class ctImguiIntegration* ImguiIntegration;
   class ctIm3dIntegration* Im3dIntegration;
   class ctKeyLimeRenderer* Renderer;
   class ctSceneEngineBase* SceneEngine;

#if CITRUS_PHYSX
   class ctPhysXIntegration* PhysXIntegration;
#endif

   ctStopwatch FrameTime;

private:
   bool _isRunning = true;
};