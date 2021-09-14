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

enum ctJobSystemPriorityModes {
   CT_JOB_SYSTEM_PRIORITY_REALTIME,
   CT_JOB_SYSTEM_PRIORITY_LOADING,
   CT_JOB_SYSTEM_PRIORITY_DEFAULT = CT_JOB_SYSTEM_PRIORITY_REALTIME
};

typedef ctHandle ctJobGroupHandle;

class CT_API ctJobSystem : public ctModuleBase {
public:
   ctJobSystem(int32_t threadReserve, bool shared = true);
   ctResults Startup() final;
   ctResults Shutdown() final;

/* https://blog.molecular-matters.com/2015/08/24/job-system-2-0-lock-free-work-stealing-part-1-basics/*/
   ctJobGroupHandle CreateGroup(const char* name);
   void DestroyGroup(ctJobGroupHandle groupHandle);
   ctResults PushJob(ctJobGroupHandle groupHandle,
                     void (*fpFunction)(void*),
                     void* pData,
                     size_t dependencyCount = 0,
                     ctJobGroupHandle* pDependencies = NULL,
                     bool longRunning = false);
   ctResults PushJobs(ctJobGroupHandle groupHandle,
                      size_t count,
                      void (**pfpFunction)(void*),
                      void** ppData,
                      size_t dependencyCount = 0,
                      ctJobGroupHandle* pDependencies = NULL,
                      bool longRunning = false);
   bool isFinished(ctJobGroupHandle groupHandle);
   void Wait(ctJobGroupHandle groupHandle);
   void Wait(size_t groupCount, ctJobGroupHandle* pGroupHandles);
   ctResults SetPriorityMode(ctJobSystemPriorityModes mode);

   void DebugImGui();

   void WaitForWork();
   void DoMoreWork(bool longRunning);

protected:
   int32_t threadReserve;
   int32_t threadCountTarget;
   float longTermThreadPercent = 20.0f;

   struct JobInternal {
      bool operator==(const JobInternal& B) {
         return memcmp(this, &B, sizeof(JobInternal)) == 0;
      }
      void (*fpFunction)(void*);
      void* pData;
      ctJobGroupHandle groupHandle;
      int16_t flags;
      int16_t dependencyCount;
      ctJobGroupHandle dependencies[9];
   };
   static_assert(sizeof(JobInternal) == CT_ALIGNMENT_ALLOCATIONS,
                 "JobInternal does not fit cache boundary");
   ctDynamicArray<JobInternal> activeJobs;
   ctMutex jobLock;

   struct ThreadInternal {
      ctThread thread;
      ctJobSystem* pJobSystem;
      int32_t canHandleLongJobs;
      int32_t wantsExit;
      char _pad[40];
      static int JobWorker(void* ctx);
   };
   static_assert(sizeof(ThreadInternal) == CT_ALIGNMENT_ALLOCATIONS,
                 "ThreadInternal does not fit cache boundary");
   ctDynamicArray<ThreadInternal> threads;
   ctMutex threadLock;

   struct GroupStateInternal {
      ctMutex lock;
      int32_t reference;
      char name[52];
   };
   static_assert(sizeof(GroupStateInternal) == CT_ALIGNMENT_ALLOCATIONS,
                 "GroupStateInternal does not fit cache boundary");
   GroupStateInternal groups[CT_MAX_JOB_GROUPS];

   ctMutex handleLock;
   ctHandleManager groupHandleManager;
};

ctJobSystem* ctGetJobSystem();