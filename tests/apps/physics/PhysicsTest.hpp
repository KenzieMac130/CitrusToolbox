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
#include "core/EngineCore.hpp"
#include "physics/Physics.hpp"
#include "physics/Baking.hpp"
#include "imgui/imgui.h"
#include "interact/InteractionEngine.hpp"
#include "scene/SceneEngine.hpp"

class PhysicsTestBase {
public:
   virtual const char* GetName() = 0;
   virtual void UIOptions() = 0;

   virtual void OnTestStartup() = 0;
   virtual void OnTick(float deltaTime) = 0;
   virtual void UIStatus() = 0;
   virtual void OnTestShutdown() = 0;
   class ctEngineCore* Engine;
};