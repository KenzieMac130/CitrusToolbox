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
#include "utilities/RingBuffer.hpp"
#include "ModuleBase.hpp"

typedef uint16_t ctJobSystemDependency;

class CT_API ctJobSystem : public ctModuleBase {
public:
   ctJobSystem(int32_t threadReserve, bool shared = true);
   ctResults Startup() final;
   ctResults Shutdown() final;
   const char* GetModuleName() final;

   ctJobSystemDependency DeclareDependency(const char* name);
   ctResults PushJob(void (*fpFunction)(void*),
                     void* pData,
                     size_t dependencyCount = 0,
                     ctJobSystemDependency* pDependencies = NULL);
   ctResults PushJobs(size_t count,
                      void (**pfpFunction)(void*),
                      void** ppData,
                      size_t dependencyCount = 0,
                      ctJobSystemDependency* pDependencies = NULL);
   void WaitBarrier();

   void DebugImGui();

   bool isMoreWorkAvailible();
   bool DoMoreWork();
   bool isExiting() const;
   void WorkLoop();

   inline size_t GetThreadCount() {
      return threadPool.Count();
   }

protected:
   int32_t threadReserve;
   int32_t threadCount;

   struct JobInternal {
      void (*fpFunction)(void*);
      void* pData;
      uint16_t dependencies[24];
   };
   static_assert(sizeof(JobInternal) == CT_ALIGNMENT_CACHE,
                 "JobInternal does not fit cache boundary");
   ctRingBuffer<JobInternal> jobQueue;

   ctSpinLock jobLock;
   ctAtomic jobCountAtom;

   struct ThreadInternal {
      ctThread thread;
   };
   ctDynamicArray<ThreadInternal> threadPool;

   struct DependencyInternal {
      ctMutex mutex;
   };
   ctDynamicArray<DependencyInternal> dependencyPool;

   bool wantsExit = false;
};

ctJobSystem* ctGetJobSystem();