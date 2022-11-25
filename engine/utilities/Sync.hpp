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

#pragma once

#include "Common.h"

typedef SDL_mutex* ctMutex;
CT_API ctMutex ctMutexCreate();
CT_API void ctMutexDestroy(ctMutex mutex);
CT_API bool ctMutexLock(ctMutex mutex);
CT_API bool ctMutexTryLock(ctMutex mutex);
CT_API bool ctMutexUnlock(ctMutex mutex);

#define CT_MEMORY_BARRIER_ACQUIRE SDL_MemoryBarrierRelease
#define CT_MEMORY_BARRIER_RELEASE SDL_MemoryBarrierAcquire

typedef SDL_SpinLock ctSpinLock;
inline void ctSpinLockInit(ctSpinLock& val) {
   val = 0;
};
inline void ctSpinLockEnterCritical(ctSpinLock& val) {
   SDL_AtomicLock(&val);
};
inline void ctSpinLockExitCritical(ctSpinLock& val) {
   SDL_AtomicUnlock(&val);
};

typedef SDL_atomic_t ctAtomic;
inline void ctAtomicAdd(ctAtomic& atomic, int val) {
   SDL_AtomicAdd(&atomic, val);
}
inline int ctAtomicGet(ctAtomic& atomic) {
   return SDL_AtomicGet(&atomic);
}
inline int ctAtomicSet(ctAtomic& atomic, int val) {
   return SDL_AtomicSet(&atomic, val);
}

typedef SDL_cond* ctConditional;
CT_API ctConditional ctConditionalCreate();
CT_API void ctConditionalDestroy(ctConditional cond);
CT_API void ctConditionalWait(ctConditional cond, ctMutex lock);
CT_API void ctConditionalSignalOne(ctConditional cond);
CT_API void ctConditionalSignalAll(ctConditional cond);

typedef struct SDL_Thread* ctThread;
CT_API ctThread ctThreadCreate(int (*func)(void*), void* data, const char* name);
CT_API int ctThreadWaitForExit(ctThread thread);