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

#include "Common.h"

typedef SDL_mutex* ctMutex;
CT_API ctMutex ctMutexCreate();
CT_API void ctMutexDestroy(ctMutex mutex);
CT_API bool ctMutexLock(ctMutex mutex);
CT_API bool ctMutexTryLock(ctMutex mutex);
CT_API bool ctMutexUnlock(ctMutex mutex);

typedef SDL_SpinLock ctSpinLock;
inline void ctSpinLockEnterCritical(ctSpinLock val) {
   SDL_AtomicLock(&val);
};
inline void ctSpinLockExitCritical(ctSpinLock val) {
   SDL_AtomicUnlock(&val);
};

typedef SDL_cond* ctConditional;
CT_API ctConditional ctConditionalCreate();
CT_API void ctConditionalDestroy(ctConditional cond);
CT_API void ctConditionalWait(ctConditional cond, ctMutex lock);
CT_API void ctConditionalSignalOne(ctConditional cond);
CT_API void ctConditionalSignalAll(ctConditional cond);

typedef struct SDL_Thread* ctThread;
CT_API ctThread ctThreadCreate(int (*func)(void*), void* data, const char* name);
CT_API int ctThreadWaitForExit(ctThread thread);