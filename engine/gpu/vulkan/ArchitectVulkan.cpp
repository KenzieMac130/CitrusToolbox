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

#include "ArchitectVulkan.hpp"
#include "gpu/vulkan/DeviceVulkan.hpp"

ctGPUArchitectBackend* ctGPUNewArchitectBackend(ctGPUDevice* pDevice) {
   return new ctGPUArchitectBackendVulkan();
}

ctResults ctGPUArchitectBackendVulkan::Startup(ctGPUDevice* pDevice) {
   return CT_SUCCESS;
}

ctResults ctGPUArchitectBackendVulkan::Shutdown(ctGPUDevice* pDevice) {
   return CT_SUCCESS;
}

ctResults ctGPUArchitectBackendVulkan::OptimizeOrder(ctGPUDevice* pDevice,
                                                     ctGPUArchitect* pArchitect) {
   /* Todo: Firsts thing first... Needs to work... then optimize! */
   return CT_SUCCESS;
}

ctResults ctGPUArchitectBackendVulkan::BuildInternal(ctGPUDevice* pDevice,
                                                     ctGPUArchitect* pArchitect) {
   // find each needed resource and create it:
   //
   // for each feedback resource:
   // create physical resource (if out of date/not already built)
   //
   // for each non feedback resource:
   // make a special id for the creation description
   // find first appeared task idx
   // find last appeared task itx
   // add it to the toscan list
   //
   // for each resource in toscan list/not in alias list:
   // find all compatible resources not overlapped in timeline
   // create physical resource
   // add all compatible resource identifiers to alias list, point to physical
   return CT_SUCCESS;
}

ctResults ctGPUArchitectBackendVulkan::ExecuteInternal(ctGPUDevice* pDevice,
                                                       ctGPUArchitect* pArchitect) {
   // for (auto cur = pArchitect->GetFinalTaskIterator(CT_GPU_TASK_RASTER); cur; cur++) {
   /* Find the last state of each other resource and transition */
   // for: resource in task get last use
   //   if: last used in different queue, do barrier
   //}
   return ctResults();
}

ctResults ctGPUArchitectBackendVulkan::ResetInternal(ctGPUDevice* pDevice,
                                                     ctGPUArchitect* pArchitect) {
   return ctResults();
}
