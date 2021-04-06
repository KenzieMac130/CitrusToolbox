#include "JobSystem.hpp"
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

#include "JobSystem.hpp"
#include "SDL.h"
#ifdef _WIN32
#include <Windows.h>
#endif
#define CUTE_SYNC_IMPLEMENTATION
#define CUTE_SYNC_SDL
#include "cute/cute_sync.h"

/* Stop windef's unwise macro definition from messing with Tracy */
#ifdef _WIN32
#ifdef max
#undef max
#endif
#endif

#include "EngineCore.hpp"

ctJobSystem::ctJobSystem(int32_t _threadReserve) {
   threadReserve = _threadReserve;
   threadCount = -1;
}

ctResults ctJobSystem::Startup() {
   ZoneScoped;
   ctSettingsSection* settings =
     Engine->Settings->CreateSection("JobSystem", 1);
   settings->BindInteger(&threadCount,
                         true,
                         true,
                         "ThreadCount",
                         "Number of threads to use for common jobs.");

   int finalThreadCount = 1;
   if (threadCount <= 0) {
      int coreCount = SDL_GetCPUCount();
      finalThreadCount = coreCount - threadReserve;
      if (finalThreadCount <= 0) { finalThreadCount = 1; }
   } else {
      finalThreadCount = threadCount;
   }
   pool = cute_threadpool_create(finalThreadCount, NULL);
   if (pool == NULL) {
      ctFatalError(
        -1, CT_NC("Failed to create threadpool!"));
      return CT_FAILURE_UNSUPPORTED_HARDWARE;
   }
   return CT_SUCCESS;
}

ctResults ctJobSystem::Shutdown() {
   cute_threadpool_destroy(pool);
   return CT_SUCCESS;
}

ctResults ctJobSystem::PushJob(void (*fpFunction)(void*), void* pData) {
   cute_threadpool_add_task(pool, fpFunction, pData);
   return CT_SUCCESS;
}

ctResults ctJobSystem::PushJobs(size_t count,
                                void (**pfpFunction)(void*),
                                void** ppData) {
   for (size_t i = 0; i < count; i++) {
      PushJob(pfpFunction[i], ppData[i]);
   }
   return CT_SUCCESS;
}

ctResults ctJobSystem::RunAllJobs() {
   ZoneScoped;
   cute_threadpool_kick_and_wait(pool);
   return CT_SUCCESS;
}
