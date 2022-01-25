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
#include "core/ModuleBase.hpp"

typedef ctHandle ctAsyncTaskHandle;
typedef ctResults (*ctAsyncTaskFunction)(void* userData);

/*
 * Mainly used for file IO/long running tasks
 * You likely should use the job system instead
 */
class CT_API ctAsyncManager : public ctModuleBase {
public:
   ctAsyncManager(bool shared = false);
   virtual ctResults Startup();
   virtual ctResults Shutdown();

   ctAsyncTaskHandle CreateTask(const char* name,
                                ctAsyncTaskFunction fpTask,
                                void* userdata,
                                int32_t priority);
   bool isFinished(ctAsyncTaskHandle handle, ctResults* pResultsOut = NULL);
   inline void WaitForAllToExit() {
      while (!isEmpty()) {}
   }
   bool isEmpty();
   void ReleaseTask(ctHandle handle);

   int RunAsyncLoop();

   void DebugImGui();

protected:
   ctHandleManager handleManager;

   struct TaskInternal {
      static int Compare(const TaskInternal* A, const TaskInternal* B);
      int32_t priority;
      ctAsyncTaskHandle handle;
      ctAsyncTaskFunction fn;
      void* data;
      char name[32];
   };
   ctDynamicArray<TaskInternal> activeTasks;
   ctMutex taskLock;

   struct TaskState {
      bool complete;
      ctResults results;
   };
   ctHashTable<TaskState, ctAsyncTaskHandle> states;
   ctMutex stateLock;

   ctThread asyncThread;
};

ctAsyncManager* ctGetAsyncManager();