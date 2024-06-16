/*
   Copyright 2023 MacKenzie Strand

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

#include "ResourceTypeBase.hpp"
#include "ResourceManager.hpp"

void ctResourceBase::LoadTaskFn(ctResourceBase* resource) {
   if (resource->LoadTask() != CT_SUCCESS) {
      resource->SetLoadState(CT_RESOURCE_STATE_FAILED);
   }
   resource->SetLoadState(CT_RESOURCE_STATE_LOADED);
}

bool ctResourceBase::isHotReloadSupported() {
   return false;
}

void ctResourceBase::OnReloadBegin() {
}

void ctResourceBase::OnReloadComplete() {
}

bool ctResourceBase::isReady() {
   int rstate = ctAtomicGet(state);
   return rstate == CT_RESOURCE_STATE_LOADED || rstate == CT_RESOURCE_STATE_FAILED;
}

bool ctResourceBase::isValid() {
   return ctAtomicGet(state) != CT_RESOURCE_STATE_FAILED;
}

void ctResourceBase::MarkGarbageCollect() {
   pCachedServer->MarkForGarbageCollect(this);
}

void ctResourceBase::HandlePointerGarbageCollect(ctResourceBase* object, void* unused) {
   object->MarkGarbageCollect();
}

const ctGUID gNullGuid = ctGUID("00000000000000000000000000000000");
ctGUID ctResourceServerBase::GetDefaultResourceGUID() {
   ctDebugError("NO FALLBACK FOR RESOURCE");
   return gNullGuid;
}

int TaskPriorityFromResourcePriority(ctResourcePriority priority) {
   switch (priority) {
      case CT_RESOURCE_PRIORITY_BACKGROUND: return 8;
      case CT_RESOURCE_PRIORITY_FOREGROUND: return 7;
      case CT_RESOURCE_PRIORITY_HIGHEST: return 0;
      case CT_RESOURCE_PRIORITY_CRITICAL: return 0;
      default: return 8;
   }
}

ctHandlePtr<ctResourceBase> ctResourceServerBase::GetOrLoad(ctGUID guid,
                                                            ctResourcePriority priority) {
   if (guid == gNullGuid) { guid = GetDefaultResourceGUID(); }
   uint64_t key = ctXXHash64(guid.data, sizeof(guid.data));
   ctSpinLockEnterCriticalScoped(RESOURCE, resourceTableLock);
   ctHandle* pSearch = resourceHandles.FindPtr(key);
   if (pSearch) { return *pSearch; }
   ctHandlePtr<ctResourceBase> created = ctHandlePtr<ctResourceBase>(
     NewResource(guid), ctResourceBase::HandlePointerGarbageCollect);
   resourceHandles.Insert(key, created.GetHandle());

   char buff[33];
   memset(buff, 0, 33);
   guid.ToHex(buff);
   /* schedule or kick off */
   if (0 /*Engine->AsyncTasks->isStarted()*/) {
      Engine->AsyncTasks->CreateTask(buff,
                                     (ctAsyncTaskFunction)ctResourceBase::LoadTaskFn,
                                     created.GetPtr(),
                                     TaskPriorityFromResourcePriority(priority));
   } else { /* if the async isn't running we need to handle it now */
      ctResourceBase::LoadTaskFn(created.GetPtr());
   }
   /* ensure critical resource is availible now */
   if (priority == CT_RESOURCE_PRIORITY_CRITICAL) {
      created.Get().WaitForReady();
      if (!created.Get().isValid()) {
         ctFatalError(-1, "COULD NOT LOAD CRITICAL RESOURCE %s", buff);
      }
   }
   return created;
}

/* this is guaranteed to be serial in game update, but not with loading tasks */
void ctResourceServerBase::DoGarbageCollection() {
   for (size_t i = 0; i < toGarbageCollect.Count(); i++) {
      ctResourceBase* pResource = toGarbageCollect[i];
      ctResourceState state = pResource->GetLoadState();
      /* resource may have been already unloaded */
      if (state == CT_RESOURCE_STATE_UNLOADED) {
         continue;
      }
      /* if resource is still loading we cannot cancel it */
      else if (state == CT_RESOURCE_STATE_LOADING) {
         toGarbageCollectCarryOver.Append(pResource); /* handle later */
         continue;
      }
      /* otherwise we can unload it now */
      pResource->OnRelease();
      pResource->SetLoadState(CT_RESOURCE_STATE_UNLOADED);
   }
   /* clear current frames garbage collection and handle remaining next frame */
   toGarbageCollect.Clear();
   toGarbageCollect.Append(toGarbageCollectCarryOver);
   toGarbageCollectCarryOver.Clear();
}

void ctResourceServerBase::NotifyHotReload(ctGUID guid) {
   /* empty by default */
}

void ctResourceServerBase::MarkLiveForGarbageCollect() {
   for (auto it = resourceHandles.GetIterator(); it; it++) {
      ctHandlePtr<ctResourceBase> resouce = it.Value();
      MarkForGarbageCollect(resouce.GetPtr());
   }
}
