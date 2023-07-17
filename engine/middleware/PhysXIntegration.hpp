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
#include "core/ModuleBase.hpp"

#include "PxPhysicsAPI.h"
#include "extensions/PxDefaultAllocator.h"
using namespace physx;

class CT_API ctPhysXIntegration : public ctModuleBase {
public:
   ctResults Startup() final;
   ctResults Shutdown() final;
   const char* GetModuleName() final;

   ctResults SetupCooking();

   ctStringUtf8 pvdHostAddress;
   int32_t pvdHostPort;
   int32_t pvdTimeout;
   int32_t recordAllocations;
   int32_t connectPvd;
   float toleranceLength;
   float toleranceSpeed;

   PxTolerancesScale toleranceScale;
   PxFoundation* pFoundation;
   PxPvd* pPvd;
   PxPhysics* pPhysics;
   PxCooking* pCooking;
   PxCpuDispatcher* pCpuDispatcher;

   PxDefaultErrorCallback defaultErrorCallback;
   PxDefaultAllocator defaultAllocatorCallback;
};