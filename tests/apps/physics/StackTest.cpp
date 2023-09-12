/*
   Copyright 2023 MacKenzie Strand

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

#include "StackTest.hpp"

const char* StackTest::GetName() {
   return "Stack Test";
}

void StackTest::UIOptions() {
   ImGui::DragInt("Bodies", &bodyCount, 1.0f, 0, 5000);
}

void StackTest::OnTestStartup() {
   ctPhysicsEngine physics = Engine->Physics->GetPhysicsEngine();
   /* shape baking */
   ctDynamicArray<uint8_t> shapeBake;
   ctPhysicsShapeSettings shapes[3];
   shapes[0] = ctPhysicsShapeSphere();
   shapes[1] = ctPhysicsShapeBox();
   shapes[2] = ctPhysicsShapeCapsule();
   shapes[1].transform.translation = ctVec3(0.5f, 0.0f, 0.0f);
   shapes[2].transform.translation = ctVec3(-0.75f, 0.0f, 0.0f);
   ctPhysicsShapeSettings compound =
     ctPhysicsShapeCompound(ctCStaticArrayLen(shapes), shapes);
   ctPhysicsBakeShape(physics, compound, shapeBake);

   /* rigidbodies */ {
      bodies.Resize(bodyCount);
      for (int i = 0; i < bodyCount; i++) {
         ctPhysicsBodyDesc desc = ctPhysicsBodyDesc();
         desc.position = ctVec3(0.0f, 50.0f + (1.0f * i), 5.0f);
         desc.shape = ctPhysicsShapeBaked(shapeBake.Data(), shapeBake.Count());
         desc.startActive = true;
         desc.friction = 0.8f;
         ctPhysicsCreateBody(physics, bodies[i], desc);
      }
   }
   /* static ground */ {
      ctPhysicsBody ground;
      ctPhysicsBodyDesc desc = ctPhysicsBodyDesc();
      desc.position = ctVec3(0.0f, -0.5f, 0.0f);
      desc.motion = CT_PHYSICS_STATIC;
      desc.shape = ctPhysicsShapeBox(ctVec3(100.0f, 0.1f, 100.0f));
      desc.friction = 0.8f;
      ctPhysicsCreateBody(physics, ground, desc);
      bodies.Append(ground);
   }
}

void StackTest::OnTick(float deltaTime) {
}

void StackTest::UIStatus() {
}

void StackTest::OnTestShutdown() {
   for (size_t i = 0; i < bodies.Count(); i++) {
      ctPhysicsDestroy(Engine->Physics->GetPhysicsEngine(), bodies[i]);
   }
}
