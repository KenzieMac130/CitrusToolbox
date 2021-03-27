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
#include "ModuleBase.hpp"

class ctJobSystem : public ctModuleBase {
public:
   ctJobSystem(int32_t threadReserve);
   ctResults Startup() final;
   ctResults Shutdown() final;

   ctResults PushJob(void (*fpFunction)(void*), void* pData);
   ctResults PushJobs(size_t count, void (**pfpFunction)(void*), void** ppData);
   ctResults RunAllJobs();

protected:
   int32_t threadReserve;
   int32_t threadCount;
   struct cute_threadpool_t* pool;
};