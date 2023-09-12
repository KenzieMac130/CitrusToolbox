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

/* in order to implement a new type you need to:
1. overload the LoadTask() function with a threadsafe loading routine
2. oveload the isReady() function with a threadsafe load check
3. overload the OnRelease() function with a threadsafe unload routine 
4. create a ctResourceServerBase that creates a instance using CreateResource()
5. register your resource server with the resource manager

To support hot reload:
1. overload isHotReloadSupported() to return true
2. overload OnReloadComplete() and optionally OnReloadBegin() to notify subsystems
(note: hot reloading will not be threaded, these features will always be serial)

Note: DO NOT DO ANY INITIALIZATION IN THE CONSTRUCTOR! */
class ctResourceTypeBase {
public:
   ctResourceTypeBase(ctGUID guid);
   virtual bool isReady() = 0;

   inline void Reference() {
      ctAtomicAdd(refcount, 1);
   }
   inline void Dereference(){
      ctAtomicAdd(refcount, -1);
   }
   inline int32_t GetRefcount(){
      return ctAtomicGet(refcount);
   }
   inline ctGUID GetDataGUID() { return dataGUID; }
   /* simply just burn cycles while waiting
   avoid doing this as much as possible */
   inline void WaitForReady() { while(!isReady()){} }
protected:
   friend class ctResourceManager;
   
   /* executed by the resource manager using the async task manager */
   virtual ctResults LoadTask(ctEngineCore* Engine) = 0;
   /* called during the resource polling loop to release resource data */
   virtual void OnRelease(ctEngineCore* Engine) = 0;
   virtual bool isHotReloadSupported();
   /* called during the resource polling loop to update dependent systems */
   virtual void OnReloadBegin(ctEngineCore* Engine);
   virtual void OnReloadComplete(ctEngineCore* Engine);

   ctAtomic refcount;
   bool needsReload;
   ctGUID dataGUID;
};

class ctResourceServerBase {
public:
   /* should look like "return new ctResourceMyType();" */
   virtual ctResourceBase* CreateResource(ctGUID guid, ctEngineCore* Engine) = 0;

protected:
   friend class ctResourceManager;
   ctResourceBase* GetOrLoad(ctGUID guid, ctResourcePriority priority);

private:
   ctSpinLock resourceTableLock;
   ctHashTable<ctResourceBase*, uint64_t> resources>
};