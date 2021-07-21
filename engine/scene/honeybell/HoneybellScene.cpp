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

/* Defined in TypeRegistration */
namespace ctHoneybell {
void RegisterBuiltinToys(ctHoneybell::ToyTypeRegistry& registry);
}

ctResults ctHoneybellSceneEngine::Startup() {
   ZoneScoped;
   RegisterBuiltinToys(toyRegistry);
   mainScene.ModuleStartup(Engine);
   mainScene.tickInterval = 1.0 / 60;

   CurrentCamera.fov = 0.785f;
   CurrentCamera.position = {0.0f, 0.0f, -5.0f};
   LevelScript.Startup(false);
   LevelScript.OpenEngineLibrary("scene");

   /* Todo: wrap spawn */
   for (int i = 0; i < 1; i++) {
      ctHoneybell::SpawnData spawnData = ctHoneybell::SpawnData();
      spawnData.rotation = ctQuat(CT_VEC3_UP, (float)i / 1 * CT_PI*2);
      spawnData.position = ctVec3(0, 0, 0);
      spawnData.scale = ctVec3(1.0f);
      ctHoneybell::PrefabData prefabData = ctHoneybell::PrefabData();
      ctHoneybell::ConstructContext constructCtx = ctHoneybell::ConstructContext();
      constructCtx.pOwningScene = &mainScene;
      constructCtx.pComponentRegistry = &mainScene.componentRegistry;
      constructCtx.spawn = spawnData;
      constructCtx.prefab = prefabData;
      constructCtx.typePath = "citrus/debugCamera";
      ctHoneybell::ToyBase* myTestToy = toyRegistry.NewToy(constructCtx);
      if (myTestToy) {
         myTestToy->_SetIdentifier(mainScene._RegisterToy(myTestToy));
         myTestToys.Append(myTestToy);
      }
   }
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
   mainScene.Update(deltaTime, Engine->JobSystem);
   {
      ctCameraInfo debugCamera = CurrentCamera;
      ImGui::Begin("Debug Camera");
      ImGui::DragFloat("Fov", &debugCamera.fov, 0.01f, 0.01f, CT_PI / 2.0f);
      ImGui::InputFloat3("Position", debugCamera.position.data);
      ImGui::DragFloat("Yaw", &camYaw, 0.01f, -CT_PI / 2.0f, CT_PI / 2.0f);
      ImGui::DragFloat("Pitch", &camPitch, 0.01f, -CT_PI / 2.0f, CT_PI / 2.0f);
      ImGui::End();

      float speed = 1.0f * deltaTime;
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

      SetCameraInfo(debugCamera);
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
   CT_RETURN_FAIL(LevelScript.LoadFromFile(str.CStr()));
   CT_RETURN_FAIL(LevelScript.RunScript());

   int returnValue = -1;
   ctScriptTypedLightData loaderData = {CT_SCRIPTOBTYPE_NULL, NULL};
   CT_RETURN_FAIL(
     LevelScript.CallFunction("loadScene", "u:i", &loaderData, &returnValue));
   return CT_SUCCESS;
}