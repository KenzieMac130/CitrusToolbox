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

/* Manages the presentation of a window */
struct ctGPUPresenter;

struct ctGPUPresenterCreateInfo {
   void* pWindow; /* Expects an SDL window if applicable */
   bool useVSync;
};

enum ctGPUPresenterState {
   CT_GPU_PRESENTER_INVALID,
   CT_GPU_PRESENTER_RESIZED,
   CT_GPU_PRESENTER_NORMAL
};

CT_API ctResults ctGPUPresenterStartup(struct ctGPUDevice* pDevice,
                                       struct ctGPUPresenter** ppPresenter,
                                       struct ctGPUPresenterCreateInfo* pCreateInfo);
CT_API ctResults ctGPUPresenterShutdown(struct ctGPUDevice* pDevice,
                                        struct ctGPUPresenter* pPresenter);
CT_API ctResults ctGPUPresenterExecute(struct ctGPUDevice* pDevice,
                                       struct ctGPUPresenter* pPresenter,
                                       struct ctGPUArchitect* pArchitect);
CT_API ctGPUPresenterState ctGPUPresenterHandleState(struct ctGPUDevice* pDevice,
                                                     struct ctGPUPresenter* pPresenter,
                                                     uint32_t* pWidth,
                                                     uint32_t* pHeight);
CT_API void ctGPUPresenterSignalStateChange(struct ctGPUDevice* pDevice,
                                            struct ctGPUPresenter* pPresenter,
                                            ctGPUPresenterState state);
CT_API void ctGPUPresenterChangeVSync(struct ctGPUDevice* pDevice,
                                      struct ctGPUPresenter* pPresenter,
                                      bool enable);
									  
#ifdef __cplusplus
}
#endif