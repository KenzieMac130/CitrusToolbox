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

/* Stop windef's unwise macro definition from messing with Tracy */
#ifdef _WIN32
#ifdef max
#undef max
#endif
#endif

#include "EngineCore.hpp"
#include "Settings.hpp"

ctJobSystem* gJobSystem;

#define CT_LONG_RUNNING_JOB 0x0001

ctJobSystem::ctJobSystem(int32_t _threadReserve, bool shared) {
   gJobSystem = this;
   threadReserve = _threadReserve;
   threadCountTarget = -1;
   activeJobs = ctDynamicArray<JobInternal>();
   threads = ctDynamicArray<ThreadInternal>();
   memset(groups, 0, sizeof(groups[0]) * CT_MAX_JOB_GROUPS);
   groupHandleManager = ctHandleManager();
   jobLock = ctMutex();
   threadLock = ctMutex();
}

ctResults ctJobSystem::Startup() {
   ZoneScoped;
   ctSettingsSection* settings = Engine->Settings->CreateSection("JobSystem", 1);
   settings->BindInteger(
     &threadReserve,
     true,
     true,
     "ThreadReserve",
     "Number of threads to set aside from the job pool under auto-select.");
   settings->BindInteger(&threadCountTarget,
                         true,
                         true,
                         "ThreadCount",
                         "Number of threads to use for common jobs. (-1: Auto-select)");
   settings->BindFloat(&longTermThreadPercent,
                       true,
                       true,
                       "LongTermThreadPercent",
                       "Percent of threads to use for long (possibly multi-frame) jobs.");

   int finalThreadCount = 1;
   if (threadCountTarget <= 0) {
      int coreCount = SDL_GetCPUCount();
      finalThreadCount = coreCount - threadReserve;
      if (finalThreadCount <= 0) { finalThreadCount = 1; }
   } else {
      finalThreadCount = threadCountTarget;
   }
   threads.Reserve(finalThreadCount);
   jobLock = ctMutexCreate();
   threadLock = ctMutexCreate();
   handleLock = ctMutexCreate();
   for (int i = 0; i < finalThreadCount; i++) {
      threads.Append(ThreadInternal());
      ThreadInternal& ctx = threads.Last();
      ctx.canHandleLongJobs = false;
      ctx.pJobSystem = this;
      ctx.wantsExit = false;
      ctx.thread = ctThreadCreate(ThreadInternal::JobWorker, &ctx, "Job Worker");
   }
   ctDebugLog("Thread Pool: Reserved %d threads...", finalThreadCount);
   return CT_SUCCESS;
}

ctResults ctJobSystem::Shutdown() {
   ZoneScoped;
   for (int32_t i = 0; i < threads.Count(); i++) {
      ctMutexLock(threadLock);
      threads[i].wantsExit = true;
      ctMutexUnlock(threadLock);
      ctThreadWaitForExit(threads[i].thread);
   }
   for (int32_t i = 0; i < CT_MAX_JOB_GROUPS; i++) {
      if (groups[i].lock) { ctMutexDestroy(groups[i].lock); }
   }
   ctMutexDestroy(handleLock);
   ctMutexDestroy(threadLock);
   ctMutexDestroy(jobLock);
   return CT_SUCCESS;
}

ctJobGroupHandle ctJobSystem::CreateGroup(const char* name) {
   ZoneScoped;
   ctMutexLock(handleLock);
   ctJobGroupHandle hndl = groupHandleManager.GetNewHandle();
   ctMutexUnlock(handleLock);
   const uint32_t idx = ctHandleGetIndex(hndl);
   if (idx >= CT_MAX_JOB_GROUPS) { ctFatalError(-1, "Job system ran out of groups..."); }
   GroupStateInternal& ctx = groups[idx];
   ctx.lock = ctMutexCreate();
   ctx.reference = 0;
   strncpy(ctx.name, name, 51);
   return hndl;
}

void ctJobSystem::DestroyGroup(ctJobGroupHandle groupHandle) {
   ZoneScoped;
   const uint32_t groupIdx = ctHandleGetIndex(groupHandle);
   ctAssert(groupIdx < CT_MAX_JOB_GROUPS);
   ctMutexLock(groups[groupIdx].lock);
   ctMutexDestroy(groups[groupIdx].lock);
   memset(&groups[groupIdx], 0, sizeof(groups[0]));
   ctMutexUnlock(groups[groupIdx].lock);
}

ctResults ctJobSystem::PushJob(ctJobGroupHandle groupHandle,
                               void (*fpFunction)(void*),
                               void* pData,
                               size_t dependencyCount,
                               ctJobGroupHandle* pDependencies,
                               bool longRunning) {
   ZoneScoped;
   return PushJobs(
     groupHandle, 1, &fpFunction, &pData, dependencyCount, pDependencies, longRunning);
}

ctResults ctJobSystem::PushJobs(ctJobGroupHandle groupHandle,
                                size_t count,
                                void (**pfpFunction)(void*),
                                void** ppData,
                                size_t dependencyCount,
                                ctJobGroupHandle* pDependencies,
                                bool longRunning) {
   ZoneScoped;
   if (dependencyCount > 9) { return CT_FAILURE_INVALID_PARAMETER; }
   ctMutexLock(jobLock);
   for (size_t i = 0; i < count; i++) {
      JobInternal newJob = JobInternal();
      newJob.groupHandle = groupHandle;
      newJob.fpFunction = pfpFunction[i];
      newJob.pData = ppData[i];
      newJob.dependencyCount = (uint16_t)dependencyCount;
      if (longRunning) { newJob.flags |= CT_LONG_RUNNING_JOB; }
      for (size_t j = 0; j < dependencyCount; j++) {
         newJob.dependencies[j] = pDependencies[j];
      }
      activeJobs.Append(newJob);
   }
   // ctDebugLog("Add: %d", (int)activeJobs.Count());
   ctMutexUnlock(jobLock);

   const uint32_t groupIdx = ctHandleGetIndex(groupHandle);
   ctAssert(groupIdx < CT_MAX_JOB_GROUPS);
   ctMutexLock(groups[groupIdx].lock);
   groups[groupIdx].reference += (int32_t)count;
   ctMutexUnlock(groups[groupIdx].lock);
   return CT_SUCCESS;
}

bool ctJobSystem::isFinished(ctJobGroupHandle groupHandle) {
   ZoneScoped;
   const uint32_t groupIdx = ctHandleGetIndex(groupHandle);
   ctAssert(groupIdx < CT_MAX_JOB_GROUPS);
   ctMutexLock(groups[groupIdx].lock);
   bool finished = groups[groupIdx].reference <= 0;
   ctMutexUnlock(groups[groupIdx].lock);
   return finished;
}

void ctJobSystem::Wait(ctJobGroupHandle groupHandle) {
   ZoneScoped;
   while (!isFinished(groupHandle)) {
      DoMoreWork(false);
   };
}

void ctJobSystem::Wait(size_t groupCount, ctJobGroupHandle* pGroupHandles) {
   ZoneScoped;
   for (size_t i = 0; i < groupCount; i++) {
      Wait(pGroupHandles[i]);
   }
}

ctResults ctJobSystem::SetPriorityMode(ctJobSystemPriorityModes mode) {
   ZoneScoped;
   int numLongTermThreads = 1;
   if (mode == CT_JOB_SYSTEM_PRIORITY_LOADING) {
      numLongTermThreads = (int)threads.Count() - 1;
   } else {
      numLongTermThreads =
        (int)ceil((float)threads.Count() * (longTermThreadPercent / 100));
   }
   if (numLongTermThreads < 0) { numLongTermThreads = 1; }
   if (numLongTermThreads > threads.Count()) {
      numLongTermThreads = (int)threads.Count();
   }
   ctMutexLock(threadLock);
   for (int i = 0; i < threads.Count(); i++) {
      if (numLongTermThreads > 0) {
         threads[i].canHandleLongJobs = true;
         numLongTermThreads--;
      } else {
         threads[i].canHandleLongJobs = false;
      }
   }
   ctMutexUnlock(threadLock);
   return CT_SUCCESS;
}

#include "imgui/imgui.h"
void ctJobSystem::DebugImGui() {
   ctMutexLock(jobLock);
   ImGui::Text("Active Job Count %d", (int)activeJobs.Count());
   ctMutexUnlock(jobLock);
}

void ctJobSystem::WaitForWork() {
   bool result = false;
   while (!result) {
      ctMutexLock(jobLock);
      result = activeJobs.isEmpty();
      ctMutexUnlock(jobLock);
   }
}

void ctJobSystem::DoMoreWork(bool longRunning) {
   ZoneScoped;
   JobInternal currentJob = JobInternal();
   ctMutexLock(jobLock);
   if (activeJobs.isEmpty()) {
      ctMutexUnlock(jobLock);
      return;
   }
   for (int i = (int)activeJobs.Count() - 1; i > 0; i--) {
      const JobInternal job = activeJobs[i];
      if (!longRunning && ctCFlagCheck(job.flags, CT_LONG_RUNNING_JOB)) { continue; }
      bool depsOk = true;
      for (int16_t j = 0; j < job.dependencyCount; j++) {
         ctJobGroupHandle dep = job.dependencies[j];
         if (!isFinished(dep)) {
            depsOk = false;
            break;
         }
      }
      if (!depsOk) { continue; }
      currentJob = job;
      break;
   }
   ctMutexUnlock(jobLock);

   if (!currentJob.fpFunction) {
      ctDebugError("INVALID");
      return;
   }
   currentJob.fpFunction(currentJob.pData);

   const uint32_t groupIdx = ctHandleGetIndex(currentJob.groupHandle);
   ctAssert(groupIdx < CT_MAX_JOB_GROUPS);
   ctMutexLock(groups[groupIdx].lock);
   groups[groupIdx].reference--;
   ctMutexUnlock(groups[groupIdx].lock);
   ctMutexLock(jobLock);
   activeJobs.Remove(currentJob); /* BAD IDEA, DO SOMETHING BETTER! */
   //ctDebugLog("Rem: %d", (int)activeJobs.Count());
   ctMutexUnlock(jobLock);
}

ctJobSystem* ctGetJobSystem() {
   ctAssert(gJobSystem);
   return gJobSystem;
}

int ctJobSystem::ThreadInternal::JobWorker(void* data) {
   ZoneScoped;
   bool running = true;
   ctJobSystem* pJobSystem = ((ThreadInternal*)data)->pJobSystem;
   while (running) {
      ctMutexLock(pJobSystem->threadLock);
      ThreadInternal ctx = *(ThreadInternal*)data;
      ctMutexUnlock(pJobSystem->threadLock);

      running = !ctx.wantsExit;
      // pJobSystem->WaitForWork();
      pJobSystem->DoMoreWork(ctx.canHandleLongJobs);
   }
   return 0;
}
