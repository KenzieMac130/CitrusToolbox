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

#include "core/Application.hpp"
#include "imgui/imgui.h"
#include "interact/InteractionEngine.hpp"
#include "physics/Physics.hpp"
#include "scene/SceneEngine.hpp"

class TestApp : public ctApplication {
   virtual const char* GetAppName();
   virtual const char* GetAppDeveloperName();
   virtual ctAppVersion GetAppVersion();
   virtual ctResults OnStartup();
   virtual ctResults OnTick(const float deltatime);
   virtual ctResults OnUIUpdate();
   virtual ctResults OnShutdown();

   ctPhysicsBody ground;
   ctDynamicArray<ctPhysicsBody> bodies;
};

const char* TestApp::GetAppName() {
   return "PhsicsTest";
}

const char* TestApp::GetAppDeveloperName() {
   return "CitrusToolbox";
}

ctAppVersion TestApp::GetAppVersion() {
   return {1, 0, 0};
}

ctResults TestApp::OnStartup() {
   ctPhysicsEngine physics = Engine->Physics->GetPhysicsEngine();
   Engine->SceneEngine->EnableCameraOverride();

   ctCameraInfo camera;
   camera.position = ctVec3(0.0f, 3.0f, -50.0f);
   Engine->SceneEngine->SetCameraOverride(camera);

   /* rigidbodies */ {
      bodies.Resize(1000);
      for (uint32_t i = 0; i < 1000; i++) {
         ctPhysicsBodyDesc desc = ctPhysicsBodyDesc();
         desc.position = ctVec3(0.0f, 50.0f + (1.0f * i), 5.0f);

         ctPhysicsShapeSettings shapes[3];
         shapes[0] = ctPhysicsShapeSphere();
         shapes[1] = ctPhysicsShapeBox();
         shapes[2] = ctPhysicsShapeCapsule();
         shapes[1].transform.translation = ctVec3(0.5f, 0.0f, 0.0f);
         shapes[2].transform.translation = ctVec3(-0.75f, 0.0f, 0.0f);
         desc.shape = ctPhysicsShapeCompound(ctCStaticArrayLen(shapes), shapes);
         desc.startActive = true;
         desc.friction = 0.8f;
         ctPhysicsCreateBody(physics, bodies[i], desc);
      }
   }
   /* static ground */ {
      ctPhysicsBodyDesc desc = ctPhysicsBodyDesc();
      desc.position = ctVec3(0.0f, -0.5f, 0.0f);
      desc.motion = CT_PHYSICS_STATIC;
      desc.shape = ctPhysicsShapeBox(ctVec3(100.0f, 0.1f, 100.0f));
      desc.friction = 0.8f;
      ctPhysicsCreateBody(physics, ground, desc);
   }
   return CT_SUCCESS;
}

ctResults TestApp::OnTick(const float deltatime) {
   ctPhysicsEngineUpdate(Engine->Physics->GetPhysicsEngine(), 1.0f / 60.0f);
   return CT_SUCCESS;
}

ctResults TestApp::OnUIUpdate() {
   return CT_SUCCESS;
}

ctResults TestApp::OnShutdown() {
   return CT_SUCCESS;
}

int main(int argc, char* argv[]) {
   TestApp* pApplication = new TestApp();
   return pApplication->Execute(argc, argv);
}