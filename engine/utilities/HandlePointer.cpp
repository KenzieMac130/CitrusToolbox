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

#include "HandlePointer.hpp"

struct ctHandlePtrEntry {
   ctHandlePtrEntry() = default;
   ctHandlePtrEntry(void* ptr,
                    void (*garbage)(void*, void*),
                    void* user,
                    ctHandle handle) {
#if CITRUS_IS_DEBUG
      self = handle;
#endif
      fpGarbageCollection = garbage;
      pUserdata = user;
      pPointer = ptr;
      refcount = 1;
   }
#if CITRUS_IS_DEBUG
   ctHandle self;
   bool Validate(ctHandle other) {
      return other == self;
   }
#endif
   void GarbageCollect() {
      ctAssert(fpGarbageCollection);
      fpGarbageCollection(pPointer, pUserdata);
   }
   _ctHandleOpaquePtrGarbageCollectFn fpGarbageCollection;
   void* pUserdata;
   void* pPointer;
   int32_t refcount;
};

struct ctHandlePtrGlobalPool {
   ctSpinLock lock;
   ctHandleManager handleManager;
   size_t maxPointers;
   ctHandlePtrEntry* pEntries;
   ctHandlePtrEntry& GetEntry(ctHandle handle) {
      ctHandlePtrEntry& entry = pEntries[ctHandleGetIndex(handle)];
#if CITRUS_IS_DEBUG
      entry.Validate(handle);
#endif
      return entry;
   }
};

ctHandlePtrGlobalPool _gGlobalHandlePoolLocalInstance;
ctHandlePtrGlobalPool* gGlobalHandlePtrPool = &_gGlobalHandlePoolLocalInstance;

void _ctHandlePtrGlobalInit(size_t maxPointers) {
   ctSpinLockInit(gGlobalHandlePtrPool->lock);
   gGlobalHandlePtrPool->handleManager = ctHandleManager();
   gGlobalHandlePtrPool->handleManager.GetNewHandle();
   gGlobalHandlePtrPool->maxPointers = maxPointers;
   gGlobalHandlePtrPool->pEntries = new ctHandlePtrEntry[maxPointers];
}

CT_API void _ctHandlePtrGlobalShutdown() {
   delete gGlobalHandlePtrPool->pEntries;
}

void* _ctHandlePtrGetCtx() {
   return gGlobalHandlePtrPool;
}

void _ctHandlePtrSetCtx(void* ctx) {
   gGlobalHandlePtrPool = (ctHandlePtrGlobalPool*)ctx;
}

_ctHandleOpaquePtr::_ctHandleOpaquePtr(
  void* value, _ctHandleOpaquePtrGarbageCollectFn fpGarbageCollection, void* pUserData) {
   ctAssert(gGlobalHandlePtrPool);
   ctSpinLockEnterCriticalScoped(LOCK, gGlobalHandlePtrPool->lock);
   handle = gGlobalHandlePtrPool->handleManager.GetNewHandle();
   uint32_t idx = ctHandleGetIndex(handle);
#if CITRUS_IS_DEBUG
   ctAssert(idx < gGlobalHandlePtrPool->maxPointers);
#endif
   gGlobalHandlePtrPool->pEntries[idx] =
     ctHandlePtrEntry(value, fpGarbageCollection, pUserData, handle);
}

void* _ctHandleOpaquePtr::Get() const {
   ctAssert(gGlobalHandlePtrPool);
   ctSpinLockEnterCriticalScoped(LOCK, gGlobalHandlePtrPool->lock);
   ctHandlePtrEntry& entry = gGlobalHandlePtrPool->GetEntry(handle);
   return entry.pPointer;
}

void _ctHandleOpaquePtr::SwapPointer(void* value, bool garbageCollect) {
   ctAssert(gGlobalHandlePtrPool);
   ctSpinLockEnterCritical(gGlobalHandlePtrPool->lock);
   ctHandlePtrEntry& entry = gGlobalHandlePtrPool->GetEntry(handle);
   ctHandlePtrEntry entryCopy = entry; /* technically we no longer own entry */
   entry.pPointer = value;
   ctSpinLockExitCritical(gGlobalHandlePtrPool->lock);
   /* we are now free to call heavy functions */
   if (garbageCollect) { entryCopy.GarbageCollect(); }
}

void _ctHandleOpaquePtr::Reference() {
   ctAssert(gGlobalHandlePtrPool);
   ctSpinLockEnterCriticalScoped(LOCK, gGlobalHandlePtrPool->lock);
   ctHandlePtrEntry& entry = gGlobalHandlePtrPool->GetEntry(handle);
   entry.refcount++;
}

void _ctHandleOpaquePtr::Dereference() {
   ctAssert(gGlobalHandlePtrPool);
   ctSpinLockEnterCritical(gGlobalHandlePtrPool->lock);
   ctHandlePtrEntry& entry = gGlobalHandlePtrPool->GetEntry(handle);
   ctHandlePtrEntry entryCopy = entry; /* technically we no longer own entry */
   entry.refcount--;
   bool garbageCollect = entry.refcount <= 0;
   if (garbageCollect) { gGlobalHandlePtrPool->handleManager.FreeHandle(handle); }
   ctSpinLockExitCritical(gGlobalHandlePtrPool->lock);
   /* we are now free to call heavy functions */
   if (garbageCollect) { entryCopy.GarbageCollect(); }
}

int32_t _ctHandleOpaquePtr::Refcount() const {
   ctAssert(gGlobalHandlePtrPool);
   ctSpinLockEnterCriticalScoped(LOCK, gGlobalHandlePtrPool->lock);
   ctHandlePtrEntry& entry = gGlobalHandlePtrPool->GetEntry(handle);
   return entry.refcount;
}