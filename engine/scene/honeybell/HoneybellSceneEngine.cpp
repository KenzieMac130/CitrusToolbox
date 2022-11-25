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

#include "HoneybellSceneEngine.hpp"
#include "imgui/imgui.h"

#include "core/EngineCore.hpp"
#include "core/Settings.hpp"
#include "core/Translation.hpp"
#include "gamelayer/GameLayer.hpp"
#include "interact/InteractionEngine.hpp"
#include "renderer/KeyLimeRenderer.hpp"

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
   pSettings->BindInteger(&debugCameraAllowed,
                          true,
                          true,
                          "DebugCameraAllowed",
                          "Allow debug fly camera",
                          CT_SETTINGS_BOUNDS_BOOL);
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
   mainScene.Engine = Engine;
   mainScene.Startup();
   mainScene.pToyRegistry = &toyRegistry;
   mainScene.tickInterval = 1.0 / 30;

   CurrentCamera.fov = 0.785f;
   CurrentCamera.position = {0.0f, 0.0f, -5.0f};
   camSpeedBase = 1.0f;

   debugCamera = CurrentCamera;

#if CITRUS_INCLUDE_AUDITION
   Engine->HotReload->RegisterDataCategory(&hotReload);
#endif
   return CT_SUCCESS;
}

ctResults ctHoneybellSceneEngine::Shutdown() {
   ZoneScoped;
   mainScene.Shutdown();
   return CT_SUCCESS;
}

struct HBUserInputCtx {
   ctHandle toyHandle;
   ctHoneybell::SignalManager* pSignalManager;
   float deltaTime;
};
void HBHandleUserInput(const char* path, float value, void* userData) {
   HBUserInputCtx* pCtx = (HBUserInputCtx*)userData;
   ctHoneybell::SignalDesc desc = ctHoneybell::SignalDesc();
   desc.value = value;
   desc.path = path;
   desc.deltaTime = 1.0f / 60;
   pCtx->pSignalManager->BroadcastTargetedSignal(desc, pCtx->toyHandle);
}

ctResults ctHoneybellSceneEngine::NextFrame() {
   ZoneScoped;
   float deltaTime = Engine->FrameTime.GetDeltaTimeFloat();
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

   if (!debugCameraActive) {
      /* Todo: CRITICAL! needs to be moved inside tick to avoid tunneling on higher FPS */
      HBUserInputCtx ctx = HBUserInputCtx();
      ctx.deltaTime = deltaTime;
      ctx.toyHandle = possessedToy;
      ctx.pSignalManager = &mainScene.signalManager;
      Engine->Interact->Directory.FireActions(
        CT_INTERACT_ACTIONDISPATCH_UPDATE, HBHandleUserInput, &ctx);
   }

   if (!pauseSim || simSingleShots) {
      mainScene.Simulate(deltaTime, Engine->JobSystem);
      if (simSingleShots > 0) { simSingleShots--; }
   }

   if (!debugCameraActive) {
      ctHoneybell::ToyBase* pToy = mainScene.FindToyByHandle(possessedToy);
      if (pToy) {
         ctHoneybell::PointOfViewContext ctx;
         ctx.cameraInfo = ctCameraInfo();
         pToy->GetPointOfView(ctx);
         CurrentCamera = ctx.cameraInfo;
      }
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
         ImGui::Text(CT_NCT("HB_DEBUGCAM_HELP",
                            "Fly: WASD\nUp/Down: EQ\nFaster: Shift\nLook: "
                            "Right-click\nRe-open: Ctrl+'`'"));
      }
      ImGui::End();
      if (!debugCamWindowOpen) { debugCameraActive = false; }
#endif

      float speed = camSpeedBase * deltaTime;
      float lookSpeed = 2.0f * deltaTime;

      /* Look */
      if (Engine->Interact->Directory.GetSignal(
            ctInteractPath("actions/debug/lookEnable"))) {
         float horizontalMove = Engine->Interact->Directory.GetSignal(
           ctInteractPath("actions/debug/lookRight"));
         float verticalMove =
           Engine->Interact->Directory.GetSignal(ctInteractPath("actions/debug/lookUp"));
         camYaw += horizontalMove * lookSpeed;
         camPitch += verticalMove * lookSpeed;
         if (camPitch < -CT_PI / 2.0f + 0.05f) { camPitch = -CT_PI / 2.0f + 0.05f; }
         if (camPitch > CT_PI / 2.0f - 0.05f) { camPitch = CT_PI / 2.0f - 0.05f; }
      }
      debugCamera.rotation = ctQuat(CT_VEC3_UP, camYaw) * ctQuat(CT_VEC3_RIGHT, camPitch);

      /* Speedup */
      if (Engine->Interact->Directory.GetSignal(
            ctInteractPath("actions/debug/moveFaster"))) {
         speed *= 4.0f;
      }

      /* Forward */
      const float moveForward = Engine->Interact->Directory.GetSignal(
        ctInteractPath("actions/debug/moveForward"));
      debugCamera.position =
        debugCamera.position + (debugCamera.rotation.getForward() * moveForward * speed);
      /* Right */
      const float moveRight =
        Engine->Interact->Directory.GetSignal(ctInteractPath("actions/debug/moveRight"));
      debugCamera.position =
        debugCamera.position + (debugCamera.rotation.getRight() * moveRight * speed);
      /* Up */
      const float moveUp =
        Engine->Interact->Directory.GetSignal(ctInteractPath("actions/debug/moveUp"));
      debugCamera.position =
        debugCamera.position + (debugCamera.rotation.getUp() * moveUp * speed);

      /* Override camera */
      CurrentCamera = debugCamera;
   } else if (debugCameraAllowed && Engine->Interact->Directory.GetSignal(
                                      ctInteractPath("actions/debug/enable"))) {
      debugCameraActive = true;
   }

   Engine->Renderer->UpdateCamera(CurrentCamera);
   return CT_SUCCESS;
}

void ctHoneybellSceneEngine::SetCameraInfo(ctCameraInfo camera, const char* cameraId) {
   CurrentCamera = camera;
}

ctCameraInfo ctHoneybellSceneEngine::GetCameraInfo(const char* cameraId) {
   return CurrentCamera;
}

ctResults ctHoneybellSceneEngine::LoadScene(const char* path, const char* message) {
   mainScene.ClearScene();
   // Todo: remove testing code
   mainScene.SpawnToy("citrus/groundPlane", ctTransform(ctVec3(0.0f, -1.0f, 0.0f)));
   ctHandle fpsHandle;
   mainScene.SpawnToy("fps/player", ctTransform(ctVec3(0.0f)), "", "", &fpsHandle);
   PossessToy(fpsHandle);
   for (int i = 0; i < 1000; i++) {
      mainScene.SpawnToy(
        "citrus/testShape", ctTransform(ctVec3(0.0f, (float)i, 1 * 2)), "1 0 0 1");
   }
   return CT_SUCCESS;
}

ctResults ctHoneybellSceneEngine::PossessToy(ctHandle handle,
                                             bool wantCamera,
                                             int32_t playerIdx,
                                             const char* message) {
   ctHoneybell::ToyBase* toy = mainScene.FindToyByHandle(handle);
   if (toy) {
      ctHoneybell::PossessionContext ctx = ctHoneybell::PossessionContext();
      ctx.playerIdx = 0; /* Todo: multi-player support */
      ctx.wantsCameraControl = true;
      ctx.pSignalManager = &mainScene.signalManager;
      if (toy->OnTryPossess(ctx) == CT_SUCCESS) {
         possessedToy = handle;
         return CT_SUCCESS;
      };
      return CT_FAILURE_INACCESSIBLE;
   }
   return CT_FAILURE_NOT_FOUND;
}