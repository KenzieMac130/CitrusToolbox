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

#include "Sync.hpp"

ctMutex ctMutexCreate() {
   return SDL_CreateMutex();
}

CT_API void ctMutexDestroy(ctMutex mutex) {
   SDL_DestroyMutex(mutex);
}

CT_API bool ctMutexLock(ctMutex mutex) {
   return SDL_LockMutex(mutex) == 0;
}

CT_API bool ctMutexTryLock(ctMutex mutex) {
   return SDL_TryLockMutex(mutex) == 0;
}

CT_API bool ctMutexUnlock(ctMutex mutex) {
   return SDL_UnlockMutex(mutex) == 0;
}

CT_API ctConditional ctConditionalCreate() {
   return SDL_CreateCond();
}

CT_API void ctConditionalDestroy(ctConditional cond) {
   SDL_DestroyCond(cond);
}

CT_API void ctConditionalWait(ctConditional cond, ctMutex lock) {
   SDL_CondWait(cond, lock);
}

CT_API void ctConditionalSignalOne(ctConditional cond) {
   SDL_CondSignal(cond);
}

CT_API void ctConditionalSignalAll(ctConditional cond) {
   SDL_CondBroadcast(cond);
}

CT_API ctThread ctThreadCreate(int (*func)(void*), void* data, const char* name) {
   return SDL_CreateThread(func, name, data);
}

CT_API int ctThreadWaitForExit(ctThread thread) {
   int result;
   SDL_WaitThread(thread, &result);
   return result;
}
