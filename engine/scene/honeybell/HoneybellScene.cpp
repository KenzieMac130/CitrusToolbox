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

#include "scripting/api/HoneybellScript.hpp"
#include "gamelayer/GameLayer.hpp"

#include "middleware/PhysXIntegration.hpp"

#include "asset/AssetManager.hpp"
#include "asset/types/WAD.hpp"

/* Defined in TypeRegistration */
namespace ctHoneybell {
void RegisterBuiltinToys(ctHoneybell::ToyTypeRegistry& registry);
}

ctResults ctHoneybellSceneEngine::Startup() {
   ZoneScoped;
   /* Configuration */
   pauseSim = false;
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

   RegisterBuiltinToys(toyRegistry);
   ctGetGameLayer().HoneybellRegisterToys(toyRegistry);
   mainScene.ModuleStartup(Engine);
   mainScene.tickInterval = 1.0 / 30;

   CurrentCamera.fov = 0.785f;
   CurrentCamera.position = {0.0f, 0.0f, -5.0f};
   camSpeedBase = 1.0f;

   debugCamera = CurrentCamera;

   LevelScript.Startup(false);
   LevelScript.OpenEngineLibrary("scene");
   return CT_SUCCESS;
}

ctResults ctHoneybellSceneEngine::Shutdown() {
   ZoneScoped;
   LevelScript.Shutdown();

   /* Todo: wrap delete */
   for (int i = 0; i < myTestToys.Count(); i++) {
      ctHoneybell::ToyBase* myTestToy = myTestToys[i];
      if (myTestToy) {
         mainScene._UnregisterToy(myTestToy->GetIdentifier());
         delete myTestToy;
      }
   }

   mainScene.ModuleShutdown();
   return CT_SUCCESS;
}

ctResults ctHoneybellSceneEngine::NextFrame() {
   ZoneScoped;
   float deltaTime = Engine->FrameTime.GetDeltaTimeFloat();

   if (!pauseSim) { mainScene.Simulate(deltaTime, Engine->JobSystem); }
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
   CT_RETURN_FAIL_CLEAN(LevelScript.LoadFromFile(str.CStr()));
   CT_RETURN_FAIL(LevelScript.RunScript());

   int returnValue = -1;
   ctScriptTypedLightData loaderData = {CT_SCRIPTOBTYPE_HBENGINE, this};
   CT_RETURN_FAIL(
     LevelScript.CallFunction("loadScene", "u:i", &loaderData, &returnValue));
   return CT_SUCCESS;
}

ctResults ctHoneybellSceneEngine::SpawnToy(const char* prefabPath,
                                           ctTransform& transform,
                                           const char* message) {
   ctWADAsset* pWadAsset = Engine->AssetManager->GetWADAsset(prefabPath);
   if (!pWadAsset) {
      ctDebugWarning("Failed to spawn %s at (%f,%f,%f)",
                     prefabPath,
                     transform.position.x,
                     transform.position.y,
                     transform.position.z);
      SpawnErrorToy(transform);
      return CT_FAILURE_FILE_NOT_FOUND;
   }

   char* pTypePath = NULL;
   int32_t typePathSize = 0;
   ctStringUtf8 sanitizedPath;
   ctWADFindLump(&pWadAsset->wadReader, "TOYTYPE", (void**)&pTypePath, &typePathSize);
   if (!pTypePath) {
      ctDebugWarning("Failed to spawn %s: No toy type!", prefabPath);
      SpawnErrorToy(transform);
      return CT_FAILURE_CORRUPTED_CONTENTS;
   }
   sanitizedPath = ctStringUtf8(pTypePath, typePathSize);

   ctHoneybell::SpawnData spawnData = ctHoneybell::SpawnData();
   spawnData.transform = transform;
   spawnData.message = message;
   ctHoneybell::PrefabData prefabData = ctHoneybell::PrefabData();
   prefabData.wadReader = pWadAsset->wadReader;
   ctHoneybell::ConstructContext constructCtx = ctHoneybell::ConstructContext();
   constructCtx.pOwningScene = &mainScene;
   constructCtx.pComponentRegistry = &mainScene.componentRegistry;
   constructCtx.pPhysics = Engine->PhysXIntegration->pPhysics;
   constructCtx.spawn = spawnData;
   constructCtx.prefab = prefabData;
   constructCtx.typePath = sanitizedPath.CStr();
   ctHoneybell::ToyBase* myTestToy = toyRegistry.NewToy(constructCtx);
   if (myTestToy) {
      myTestToy->_SetIdentifier(mainScene._RegisterToy(myTestToy));
      myTestToys.Append(myTestToy);
   }
   pWadAsset->Dereferene();
   return CT_SUCCESS;
}

ctResults ctHoneybellSceneEngine::SpawnErrorToy(ctTransform& transform) {
   ctHoneybell::SpawnData spawnData = ctHoneybell::SpawnData();
   spawnData.transform = transform;
   spawnData.message = "";
   ctHoneybell::PrefabData prefabData = ctHoneybell::PrefabData();
   ctHoneybell::ConstructContext constructCtx = ctHoneybell::ConstructContext();
   constructCtx.pOwningScene = &mainScene;
   constructCtx.pComponentRegistry = &mainScene.componentRegistry;
   constructCtx.pPhysics = Engine->PhysXIntegration->pPhysics;
   constructCtx.spawn = spawnData;
   constructCtx.prefab = prefabData;
   constructCtx.typePath = "citrus/testShape";
   ctHoneybell::ToyBase* myTestToy = toyRegistry.NewToy(constructCtx);
   if (myTestToy) {
      myTestToy->_SetIdentifier(mainScene._RegisterToy(myTestToy));
      myTestToys.Append(myTestToy);
   }
   return ctResults();
}

int ctScriptApi::Honeybell::SpawnToy(ctScriptTypedLightData* ldata,
                                     const char* path,
                                     float x,
                                     float y,
                                     float z,
                                     float yaw,
                                     float pitch,
                                     float roll,
                                     float scale,
                                     const char* message) {
   ctTransform transform = ctTransform(ctVec3(x, y, z), ctQuat(), ctVec3(scale));
   ((ctHoneybellSceneEngine*)(ldata->ptr))->SpawnToy(path, transform, message);
   return 0;
}