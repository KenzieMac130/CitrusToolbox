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

#pragma once

#include "utilities/Common.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Uses ImGui to display useful information about the backend */
/* Assumes you have started a window when you call these functions */

void ctGPUDebugUIDevice(struct ctGPUDevice* pDevice);
void ctGPUDebugUIArchitect(struct ctGPUDevice* pDevice,
                           struct ctGPUArchitect* pArchitect,
                           bool allowGVisDump);
void ctGPUDebugUIPresent(struct ctGPUDevice* pDevice, struct ctGPUPresenter* pPresenter);
void ctGPUDebugUIBindless(struct ctGPUDevice* pDevice,
                          struct ctGPUBindlessManager* pBindless);
void ctGPUDebugUIExternalBufferPool(struct ctGPUDevice* pDevice,
                                    struct ctGPUExternalBufferPool* pPool);
void ctGPUDebugUIExternalTexturePool(struct ctGPUDevice* pDevice,
                                     struct ctGPUExternalTexturePool* pPool);

#ifdef __cplusplus
}
#endif