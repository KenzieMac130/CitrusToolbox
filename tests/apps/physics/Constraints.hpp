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

#pragma once

#include "utilities/Common.h"

#include "PhysicsTest.hpp"

class ConstraintTest : public PhysicsTestBase {
public:
   virtual const char* GetName();
   virtual void UIOptions();
   virtual void OnTestStartup();
   virtual void OnTick(float);
   virtual void UIStatus();
   virtual void OnTestShutdown();

private:
   int bodyCount = 50;
   ctDynamicArray<ctPhysicsBody> bodies;
};