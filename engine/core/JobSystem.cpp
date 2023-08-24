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

#include "JobSystem.hpp"
#include "EngineCore.hpp"
#include "Settings.hpp"

ctJobSystem* gJobSystem = NULL;

ctJobSystem::ctJobSystem(int32_t _threadReserve, bool shared) {
   if (shared) { gJobSystem = this; }
   threadReserve = _threadReserve;
   threadCount = -1;
   ctSpinLockInit(jobLock);
   ctAtomicSet(jobCountAtom, 0);
}

int ctJobWorker(void* data) {
   ZoneScoped;
   ctJobSystem* pJobSystem = (ctJobSystem*)data;
   pJobSystem->WorkLoop();
   return 0;
}

ctResults ctJobSystem::Startup() {
   ZoneScoped;
   ctSettingsSection* settings = Engine->Settings->CreateSection("JobSystem", 1);
   settings->BindInteger(&threadCount,
                         true,
                         true,
                         "ThreadCount",
                         "Number of threads to use for common jobs. (-1: Auto-select)");

   int finalThreadCount = 1;
   if (threadCount <= 0) {
      int coreCount = SDL_GetCPUCount();
      finalThreadCount = coreCount - threadReserve;
      if (finalThreadCount <= 0) { finalThreadCount = 1; }
   } else {
      finalThreadCount = threadCount;
   }
   ctAtomicSet(jobCountAtom, 0);
   ctSpinLockInit(jobLock);
   threadPool.Reserve(finalThreadCount);
   jobQueue.Reserve(4096);
   for (int i = 0; i < finalThreadCount; i++) {
      ThreadInternal thread = ThreadInternal();
      thread.thread = ctThreadCreate(ctJobWorker, this, "Job Thread");
      threadPool.Append(thread);
   }
   ctDebugLog("Thread Pool: Reserved %d threads...", finalThreadCount);
   return CT_SUCCESS;
}

ctResults ctJobSystem::Shutdown() {
   ZoneScoped;
   wantsExit = true;
   for (int i = 0; i < threadPool.Count(); i++) {
      ctThreadWaitForExit(threadPool[i].thread);
   }
   return CT_SUCCESS;
}

const char* ctJobSystem::GetModuleName() {
   return "Job System";
}

ctJobSystemDependency ctJobSystem::DeclareDependency(const char* name) {
   return ctJobSystemDependency();
   /* todo*/
}

ctResults ctJobSystem::PushJob(void (*fpFunction)(void*),
                               void* pData,
                               size_t dependencyCount,
                               ctJobSystemDependency* pDependencies) {
   ZoneScoped;
   return PushJobs(1, &fpFunction, &pData, dependencyCount, pDependencies);
}

ctResults ctJobSystem::PushJobs(size_t count,
                                void (**pfpFunction)(void*),
                                void** ppData,
                                size_t dependencyCount,
                                ctJobSystemDependency* pDependencies) {
   ZoneScoped;
   ctSpinLockEnterCritical(jobLock);
   JobInternal jobBase = JobInternal();
   ctAssert(dependencyCount <= ctCStaticArrayLen(jobBase.dependencies));
   for (size_t j = 0; j < dependencyCount; j++) {
      jobBase.dependencies[j] = pDependencies[j];
   }
   for (size_t i = 0; i < count; i++) {
      JobInternal job = jobBase;
      job.fpFunction = pfpFunction[i];
      job.pData = ppData[i];
      jobQueue.Append(job);
      /* todo: job slot target is last non-empty, else loop to first */
      /* in practice looping to first should never happen as all will be cleared by a host
       * call to WaitBarrier() at least once each frame unless dependencyPool saturates */
   }
   ctSpinLockExitCritical(jobLock);
   ctAtomicAdd(jobCountAtom, (int)count);
   return CT_SUCCESS;
}

void ctJobSystem::DebugImGui() {
}

bool ctJobSystem::isExiting() const {
   return wantsExit;
}

bool ctJobSystem::isMoreWorkAvailible() {
   const bool val = ctAtomicGet(jobCountAtom) > 0;
   return val;
}

void ctJobSystem::WaitBarrier() {
   ZoneScoped;
   while (isMoreWorkAvailible()) {
      DoMoreWork();
   }
}

bool ctJobSystem::DoMoreWork() {
   ZoneScoped;
   ctSpinLockEnterCritical(jobLock);
   if (jobQueue.isEmpty()) {
      ctSpinLockExitCritical(jobLock);
      return false;
   }
   JobInternal job = jobQueue.First();
   /* todo: factor in dependencies */
   ctAtomicAdd(jobCountAtom, -1);
   jobQueue.RemoveFirst();
   /* todo: fixed job slots which are scanned for sequentially */
   /* removal will be nullification of function pointer */
   ctSpinLockExitCritical(jobLock);
   /* try to lock mutexes or continue */
   job.fpFunction(job.pData);
   /* unlock mutexes */
   return true;
}

void ctJobSystem::WorkLoop() {
   while (!isExiting()) {
      while (isMoreWorkAvailible()) {
         DoMoreWork();
      }
   }
}

ctJobSystem* ctGetJobSystem() {
   return gJobSystem;
}
