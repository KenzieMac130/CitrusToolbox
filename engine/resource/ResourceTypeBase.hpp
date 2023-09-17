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

#pragma once

#include "utilities/Common.h"
#include "ResourceManager.hpp"
#include "core/EngineCore.hpp"
#include "core/AsyncTasks.hpp"
#include "core/FileSystem.hpp"

enum ctResourceState {
   CT_RESOURCE_STATE_LOADING,
   CT_RESOURCE_STATE_LOADED,
   CT_RESOURCE_STATE_FAILED,
   CT_RESOURCE_STATE_UNLOADED
};

/* in order to implement a new type you need to:
1. overload the LoadTask() function with a threadsafe loading routine
2. oveload the isReady() function with a threadsafe load check
3. overload the OnRelease() function with a threadsafe unload routine
4. create a ctResourceServerBase that creates a instance using NewResource()
5. register your resource server with the resource managers StartupServers() function

To support hot reload:
1. overload isHotReloadSupported() to return true
2. overload OnReloadComplete() and optionally OnReloadBegin() to notify subsystems
(note: hot reloading will not be threaded, these features will always be serial)

Note: DO NOT DO ANY RESOURCE INITIALIZATION IN THE CONSTRUCTOR! */
class ctResourceBase {
public:
   inline ctResourceBase(ctResourceServerBase* pServer, ctGUID guid) {
      pCachedServer = pServer;
      dataGUID = guid;
      ctAtomicSet(refcount, 0);
      ctAtomicSet(state, CT_RESOURCE_STATE_LOADING);
   }
   ctResourceBase(ctResourceBase& other) = delete;
   ctResourceBase(const ctResourceBase& other) = delete;
   /* Was the resource loader finished (doesn't mean the resource is valid)
   ALWAYS USE THIS FUNCTION BEFORE ACCESSING THE RESOURCE */
   bool isReady();
   /* Is the resource valid (may include fallbacks)
   ALWAYS USE THIS FUNCTION BEFORE ACCESSING THE RESOURCE AFTER isReady() */
   virtual bool isValid();
   inline ctResourceState GetLoadState() {
      return (ctResourceState)ctAtomicGet(state);
   }

   inline void Reference() {
      ctAtomicAdd(refcount, 1);
   }
   void Dereference();
   inline int32_t GetRefcount() {
      return ctAtomicGet(refcount);
   }
   inline ctGUID GetDataGUID() {
      return dataGUID;
   }
   /* simply just burn cycles while waiting
   avoid doing this as much as possible */
   inline void WaitForReady() {
      while (!isReady()) {}
   }

protected:
   friend class ctResourceManager;
   friend class ctResourceServerBase;

   /* executed by the resource manager using the async task manager */
   virtual ctResults LoadTask(ctEngineCore* Engine) = 0;
   /* called during the resource polling loop to release resource data */
   virtual void OnRelease(ctEngineCore* Engine) = 0;
   virtual bool isHotReloadSupported();
   /* called during the resource polling loop to update dependent systems */
   virtual void OnReloadBegin(ctEngineCore* Engine);
   virtual void OnReloadComplete(ctEngineCore* Engine);
   void ctResourceBase::SetLoadState(ctResourceState newstate) {
      ctAtomicSet(state, newstate);
   }
   void ctResourceBase::MarkReady() {
      SetLoadState(CT_RESOURCE_STATE_LOADED);
   }

private:
   class ctResourceServerBase* pCachedServer;
   ctAtomic refcount;
   ctAtomic state;
   ctGUID dataGUID;
};

class ctResourceServerBase {
public:
   /* should look like "return new ctResourceMyType();"
   DO NOT DO ANY LOADING HERE! */
   virtual ctResourceBase* NewResource(ctGUID guid, ctEngineCore* Engine) = 0;

protected:
   friend class ctResourceManager;
   friend class ctResourceBase;
   inline void MarkForGarbageCollect(ctResourceBase* resource) {
      ctSpinLockEnterCriticalScoped(TABLE, resourceTableLock);
      toGarbageCollect.Append(resource);
   }
   ctResourceBase* GetOrLoad(ctGUID guid, ctResourcePriority priority);
   ctEngineCore* Engine;
   void DoGarbageCollection();
   void NotifyHotReload(ctGUID guid);

private:
   ctSpinLock resourceTableLock;
   ctHashTable<ctResourceBase*, uint64_t> resources;
   ctDynamicArray<ctResourceBase*> toGarbageCollect;
   ctDynamicArray<ctResourceBase*> toGarbageCollectCarryOver;
};