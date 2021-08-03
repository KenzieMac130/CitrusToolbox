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

#include "HoneybellScene.hpp"
#include "imgui/imgui.h"

#include "core/EngineCore.hpp"
#include "core/Settings.hpp"
#include "core/Translation.hpp"
#include "gamelayer/GameLayer.hpp"
#include "interact/InteractionEngine.hpp"

/* Defined in TypeRegistration */
namespace ctHoneybell {
void RegisterBuiltinToys(ctHoneybell::ToyTypeRegistry& registry);
}

ctResults ctHoneybellSceneEngine::Startup() {
   ZoneScoped;
   /* Configuration */
   pauseSim = false;
   simSingleShots = 0;
#if NDEBUG
   debugCameraActive = false;
#else
   debugCameraActive = true;
#endif
   ctSettingsSection* pSettings = Engine->Settings->CreateSection("Honeybell", 32);
   pSettings->BindInteger(&debugCameraActive,
                          true,
                          true,
                          "DebugCameraActive",
                          "Enable debug fly camera",
                          CT_SETTINGS_BOUNDS_BOOL);
   pSettings->BindInteger(
     &pauseSim, false, true, "PauseSim", "Pause the simulation", CT_SETTINGS_BOUNDS_BOOL);
   pSettings->BindInteger(&simSingleShots,
                          false,
                          true,
                          "SimSingleShots",
                          "Number of shots to inject while paused.",
                          CT_SETTINGS_BOUNDS_UINT);

   RegisterBuiltinToys(toyRegistry);
   ctGetGameLayer().HoneybellRegisterToys(toyRegistry);
   mainScene.ModuleStartup(Engine);
   mainScene.pToyRegistry = &toyRegistry;
   mainScene.tickInterval = 1.0 / 30;

   CurrentCamera.fov = 0.785f;
   CurrentCamera.position = {0.0f, 0.0f, -5.0f};
   camSpeedBase = 1.0f;

   debugCamera = CurrentCamera;

   levelScript.Startup(false);
   levelScript.OpenEngineLibrary("scene");

#if CITRUS_INCLUDE_AUDITION
   Engine->HotReload->RegisterAssetCategory(&hotReload);
#endif
   return CT_SUCCESS;
}

ctResults ctHoneybellSceneEngine::Shutdown() {
   ZoneScoped;
   levelScript.Shutdown();

   mainScene.ModuleShutdown();
   return CT_SUCCESS;
}

ctResults ctHoneybellSceneEngine::NextFrame() {
   ZoneScoped;

#if CITRUS_INCLUDE_AUDITION
   if (hotReload.isContentUpdated()) {
      sceneReload = true;
      hotReload.ClearChanges();
   }
#endif
   if (sceneReload) {
      sceneReload = false;
      LoadScene(activeSceneName.CStr());
   }

   float deltaTime = Engine->FrameTime.GetDeltaTimeFloat();

   if (!pauseSim || simSingleShots) {
      mainScene.Simulate(deltaTime, Engine->JobSystem);
      if (simSingleShots > 0) { simSingleShots--; }
   }
   mainScene.NextFrame();

   /* Lastly apply the debug camera */
   if (debugCameraActive) {
#if CITRUS_IMGUI
      bool debugCamWindowOpen = true;
      bool _simPaused = pauseSim;
      if (ImGui::Begin(CT_NC("Debug Camera"), &debugCamWindowOpen)) {
         ImGui::DragFloat(CT_NC("Fov"), &debugCamera.fov, 0.01f, 0.01f, CT_PI / 2.0f);
         ImGui::InputFloat3(CT_NC("Position"), debugCamera.position.data);
         ImGui::DragFloat(CT_NC("Yaw"), &camYaw, 0.01f, -CT_PI / 2.0f, CT_PI / 2.0f);
         ImGui::DragFloat(CT_NC("Pitch"), &camPitch, 0.01f, -CT_PI / 2.0f, CT_PI / 2.0f);
         ImGui::DragFloat(CT_NC("Camera Speed"), &camSpeedBase, 0.01f, 0.5f, 64.0f);
         if (ImGui::Checkbox(CT_NC("Pause Simulation"), &_simPaused)) {
            pauseSim = _simPaused;
         }
         if (ImGui::Button(CT_NC("Single Shot"))) { simSingleShots++; }
         if (ImGui::Button(CT_NC("Reset Scene"))) { sceneReload = true; }
         ImGui::Text(
           "Fly: WASD\nUp/Down: EQ\nFaster: Shift\nLook: Right-click\nRe-open: Ctrl+'`'");
      }
      ImGui::End();
      if (!debugCamWindowOpen) { debugCameraActive = false; }
#endif

      float speed = camSpeedBase * deltaTime;
      float lookSpeed = 2.0f * deltaTime;

      /* Look */
      if (Engine->Interact->GetSignal(ctInteractPath("/dev/mouse/input/button/right"))) {
         float horizontalMove = Engine->Interact->GetSignal(
           ctInteractPath("/dev/mouse/input/relative_move/x"));
         float verticalMove = Engine->Interact->GetSignal(
           ctInteractPath("/dev/mouse/input/relative_move/y"));
         camYaw += horizontalMove * lookSpeed;
         camPitch += verticalMove * lookSpeed;
         if (camPitch < -CT_PI / 2.0f + 0.05f) { camPitch = -CT_PI / 2.0f + 0.05f; }
         if (camPitch > CT_PI / 2.0f - 0.05f) { camPitch = CT_PI / 2.0f - 0.05f; }
      }
      debugCamera.rotation = ctQuat(CT_VEC3_UP, camYaw) * ctQuat(CT_VEC3_RIGHT, camPitch);

      /* Speedup */
      if (Engine->Interact->GetSignal(
            ctInteractPath("/dev/keyboard/input/scancode/225"))) {
         speed *= 4.0f;
      }

      /* Forward */
      if (Engine->Interact->GetSignal(
            ctInteractPath("/dev/keyboard/input/scancode/26"))) {
         debugCamera.position =
           debugCamera.position + (debugCamera.rotation.getForward() * speed);
      }
      /* Back */
      if (Engine->Interact->GetSignal(
            ctInteractPath("/dev/keyboard/input/scancode/22"))) {
         debugCamera.position =
           debugCamera.position + (debugCamera.rotation.getBack() * speed);
      }
      /* Left */
      if (Engine->Interact->GetSignal(ctInteractPath("/dev/keyboard/input/scancode/4"))) {
         debugCamera.position =
           debugCamera.position + (debugCamera.rotation.getLeft() * speed);
      }
      /* Right */
      if (Engine->Interact->GetSignal(ctInteractPath("/dev/keyboard/input/scancode/7"))) {
         debugCamera.position =
           debugCamera.position + (debugCamera.rotation.getRight() * speed);
      }
      /* Up */
      if (Engine->Interact->GetSignal(ctInteractPath("/dev/keyboard/input/scancode/8"))) {
         debugCamera.position =
           debugCamera.position + (debugCamera.rotation.getUp() * speed);
      }
      /* Down */
      if (Engine->Interact->GetSignal(
            ctInteractPath("/dev/keyboard/input/scancode/20"))) {
         debugCamera.position =
           debugCamera.position + (debugCamera.rotation.getDown() * speed);
      }

      CurrentCamera = debugCamera;
   } else if (Engine->Interact->GetSignal(
                ctInteractPath("/dev/keyboard/input/scancode/53")) &&
              Engine->Interact->GetSignal(
                ctInteractPath("/dev/keyboard/input/scancode/224"))) {
      debugCameraActive = true;
   }

   LastCamera = CurrentCamera;
   return CT_SUCCESS;
}

void ctHoneybellSceneEngine::SetCameraInfo(ctCameraInfo camera, const char* cameraId) {
   CurrentCamera = camera;
}

ctCameraInfo ctHoneybellSceneEngine::GetCameraInfo(const char* cameraId) {
   return CurrentCamera;
}

ctCameraInfo ctHoneybellSceneEngine::GetCameraInfoLastFrame(const char* cameraId) {
   return LastCamera;
}

ctResults ctHoneybellSceneEngine::LoadScene(const char* name) {
   ZoneScoped;
   ctStringUtf8 str = Engine->FileSystem->GetAssetPath();
   str += "scene/";
   str += name;
   str += "/scene.lua";
   str.FilePathLocalize();
   CT_RETURN_FAIL(levelScript.LoadFromFile(str.CStr()));
   CT_RETURN_FAIL(levelScript.RunScript());
   mainScene.ClearScene();
   activeSceneName = name;

#if CITRUS_INCLUDE_AUDITION
   str.Clear();
   str.Printf(24 + strlen(name), "scene/%s/scene.lua", name);
   hotReload.Reset();
   hotReload.RegisterPath(str.CStr());
#endif

   int returnValue = -1;
   ctScriptTypedLightData loaderData = {CT_SCRIPTOBTYPE_HBSCENE, &mainScene};
   CT_RETURN_FAIL(
     levelScript.CallFunction("loadScene", "u:i", &loaderData, &returnValue));
   return CT_SUCCESS;
}