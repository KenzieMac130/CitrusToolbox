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

bool ctResourceBase::isHotReloadSupported() {
   return false;
}

void ctResourceBase::OnReloadBegin(ctEngineCore* Engine) {
}

void ctResourceBase::OnReloadComplete(ctEngineCore* Engine) {
}

bool ctResourceBase::isReady() {
   int rstate = ctAtomicGet(state);
   return rstate == CT_RESOURCE_STATE_LOADED || rstate == CT_RESOURCE_STATE_FAILED;
}

bool ctResourceBase::isValid() {
   return true;
}

void ctResourceBase::Dereference() {
   ctAtomicAdd(refcount, -1);
   if (ctAtomicGet(refcount) <= 0) { pCachedServer->MarkForGarbageCollect(this); }
}

ctResourceBase* ctResourceServerBase::GetOrLoad(ctGUID guid,
                                                ctResourcePriority priority) {
   uint64_t key = ctXXHash64(guid.data, sizeof(guid.data));
   ctSpinLockEnterCriticalScoped(RESOURCE, resourceTableLock);
   ctResourceBase** ppSearch = resources.FindPtr(key);
   if (ppSearch) {
      (*ppSearch)->Reference();
      return *ppSearch;
   }
   ctResourceBase* pCreated = NewResource(guid, Engine);
   resources.Insert(key, pCreated);
   pCreated->Reference();
   return pCreated;
}

/* this is guaranteed to be serial in game update, but not with loading tasks */
void ctResourceServerBase::DoGarbageCollection() {
   for (size_t i = 0; i < toGarbageCollect.Count(); i++) {
      ctResourceBase* pResource = toGarbageCollect[i];
      /* refcount could have changed */
      if (pResource->GetRefcount() <= 0) { continue; }
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
      pResource->OnRelease(Engine);
      pResource->SetLoadState(CT_RESOURCE_STATE_UNLOADED);
   }
   /* clear current frames garbage collection and handle remaining next frame */
   toGarbageCollect.Clear();
   toGarbageCollect.Append(toGarbageCollectCarryOver);
   toGarbageCollectCarryOver.Clear();
}

void ctResourceServerBase::NotifyHotReload(ctGUID guid) {
}
