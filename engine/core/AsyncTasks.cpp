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

#include "core/EngineCore.hpp"
#include "AsyncTasks.hpp"

ctAsyncManager* gSharedAsync = NULL;

ctAsyncManager* ctGetAsyncManager() {
   return gSharedAsync;
}

ctAsyncManager::ctAsyncManager(bool shared) {
   if (shared) { gSharedAsync = this; }
}

int ctAsyncWorker(void* data) {
   ZoneScoped;
   ctAsyncManager* pManager = (ctAsyncManager*)data;
   return pManager->RunAsyncLoop();
}

ctResults ctAsyncManager::Startup() {
   ZoneScoped;
   taskLock = ctMutexCreate();
   stateLock = ctMutexCreate();
   asyncThread = ctThreadCreate(ctAsyncWorker, this, "ctAsyncTasks");
   return CT_SUCCESS;
}

ctResults ctAsyncManager::Shutdown() {
   ZoneScoped;
   ctThreadWaitForExit(asyncThread);
   ctMutexDestroy(taskLock);
   ctMutexDestroy(stateLock);
   return ctResults();
}

const char* ctAsyncManager::GetModuleName() {
   return "Async Manager";
}

ctAsyncTaskHandle ctAsyncManager::CreateTask(const char* name,
                                             ctAsyncTaskFunction fpTask,
                                             void* userdata,
                                             int32_t priority) {
   ZoneScoped;
   ctMutexLock(stateLock);
   ctAsyncTaskHandle hndl = handleManager.GetNewHandle();
   states.Insert(hndl, TaskState());
   ctMutexUnlock(stateLock);

   TaskInternal newTask = TaskInternal();
   newTask.priority = priority;
   strncpy(newTask.name, name, 32);
   newTask.data = userdata;
   newTask.fn = fpTask;
   newTask.handle = hndl;
   ctMutexLock(taskLock);
   activeTasks.Append(newTask);
   activeTasks.QSort(TaskInternal::Compare);
   ctMutexUnlock(taskLock);
   return hndl;
}

bool ctAsyncManager::isFinished(ctAsyncTaskHandle handle, ctResults* pResultsOut) {
   bool finished = true; /* it isn't there so return true to avoid blockage */
   ctMutexLock(stateLock);
   TaskState* pTaskState = states.FindPtr(handle);
   if (pTaskState) {
      if (pResultsOut) { *pResultsOut = pTaskState->results; }
      finished = pTaskState->complete;
   }
   ctMutexUnlock(stateLock);
   return finished;
}

bool ctAsyncManager::isEmpty() {
   ctMutexLock(stateLock);
   bool result = activeTasks.isEmpty();
   ctMutexUnlock(stateLock);
   return result;
}

void ctAsyncManager::ReleaseTask(ctHandle handle) {
   ctMutexLock(stateLock);
   handleManager.FreeHandle(handle);
   states.Remove(handle);
   ctMutexUnlock(stateLock);
}

int ctAsyncManager::RunAsyncLoop() {
   while (!Engine->isExitRequested()) {
      TaskInternal nextTask;
      ctMutexLock(taskLock);
      if (activeTasks.isEmpty()) {
         ctMutexUnlock(taskLock);
         ctWait(1);
         continue;
      }
      nextTask = activeTasks[0];
      activeTasks.RemoveAt(0);
      ctMutexUnlock(taskLock);
      ctResults results = nextTask.fn(nextTask.data);
      ctMutexLock(stateLock);
      TaskState* pTaskState = states.FindPtr(nextTask.handle);
      if (pTaskState) {
         pTaskState->complete = true;
         pTaskState->results = results;
      }
      ctMutexUnlock(stateLock);
   }
   return 0;
}

#include "imgui/imgui.h"
void ctAsyncManager::DebugUI(bool useGizmos) {
   ctMutexLock(taskLock);
   for (int i = 0; i < activeTasks.Count(); i++) {
      ImGui::Text("%.*s", 32, activeTasks[i].name);
   }
   ctMutexUnlock(taskLock);
}

int ctAsyncManager::TaskInternal::Compare(const TaskInternal* A, const TaskInternal* B) {
   return A->priority - B->priority;
}
