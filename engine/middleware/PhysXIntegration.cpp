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

#include "PhysXIntegration.hpp"

#include "core/EngineCore.hpp"
#include "core/Settings.hpp"
#include "core/Translation.hpp"

ctResults ctPhysXIntegration::Startup() {
   ctDebugLog("Starting PhysX...");
   pvdHostAddress = "127.0.0.1";
   pvdHostPort = 5425;
   pvdTimeout = 10;
#if CITRUS_PHYSX_CHECKED
   recordAllocations = true;
   connectPvd = true;
#else
   recordAllocations = false;
   connectPvd = false;
#endif

   toleranceLength = 1.0f;
   toleranceSpeed = 10.0f;

   ctSettingsSection* pSettings = Engine->Settings->CreateSection("PhysX", 32);
   pSettings->BindInteger(&connectPvd,
                          false,
                          true,
                          "ConnectPvd",
                          "Connect to PhysX visual debugger",
                          CT_SETTINGS_BOUNDS_BOOL);
   pSettings->BindString(
     &pvdHostAddress, false, true, "PvdHostAddress", "Address to host PhysX debug info on");
   pSettings->BindInteger(
     &pvdHostPort, false, true, "PvdHostPort", "Port to host PhysX debug info on", 0);
   pSettings->BindInteger(
     &pvdTimeout, false, true, "PvdTimeout", "Timeout to pvd connection", CT_SETTINGS_BOUNDS_UINT);
   pSettings->BindInteger(&recordAllocations,
                          false,
                          true,
                          "RecordAllocations",
                          "Record memory allocations",
                          CT_SETTINGS_BOUNDS_BOOL);

   pSettings->BindFloat(&toleranceLength,
                        false,
                        true,
                        "ToleranceLength",
                        "Tolerance length as documented by PhysX",
                        0.0f);
   pSettings->BindFloat(&toleranceSpeed,
                        false,
                        true,
                        "ToleranceSpeed",
                        "Tolerance speed as documented by PhysX",
                        0.0f);

   toleranceScale = PxTolerancesScale();
   toleranceScale.length = toleranceLength;
   toleranceScale.speed = toleranceSpeed;

   pFoundation =
     PxCreateFoundation(PX_PHYSICS_VERSION, defaultAllocatorCallback, defaultErrorCallback);
   if (!pFoundation) { ctFatalError(-1, "PxCreateFoundation failed!"); }

   if (connectPvd) {
      pPvd = PxCreatePvd(*pFoundation);
      PxPvdTransport* transport =
        PxDefaultPvdSocketTransportCreate(pvdHostAddress.CStr(), pvdHostPort, pvdTimeout);
      pPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);
   }

   pPhysics =
     PxCreateBasePhysics(PX_PHYSICS_VERSION, *pFoundation, toleranceScale, recordAllocations, pPvd);
   if (!pPhysics) { ctFatalError(-1, "PxCreatePhysics failed!"); }

   PxRegisterArticulations(*pPhysics);
   PxRegisterHeightFields(*pPhysics);

#if CITRUS_PHYSX_RUNTIMECOOK
   pCooking = PxCreateCooking(PX_PHYSICS_VERSION, *pFoundation, PxCookingParams(toleranceScale));
   if (!pCooking) { ctFatalError(-1, "PxCreateCooking failed!"); }
#endif

   if (!PxInitExtensions(*pPhysics, pPvd)) { ctFatalError(-1, "PxInitExtensions failed!"); }

   /* Todo: write CPU dispatcher tied into job system */
   pCpuDispatcher = PxDefaultCpuDispatcherCreate(2);

   return CT_SUCCESS;
}

ctResults ctPhysXIntegration::Shutdown() {
   PxCloseExtensions();
   if (pCooking) { pCooking->release(); }
   if (pPhysics) { pPhysics->release(); }
   if (pPvd) { pPvd->release(); }
   if (pFoundation) { pFoundation->release(); }
   return CT_SUCCESS;
}

const char* ctPhysXIntegration::GetModuleName() {
   return "PhysX";
}
