#include "Sync.hpp"

ctMutex ctMutexCreate()
{
	return SDL_CreateMutex();
}

void ctMutexDestroy(ctMutex mutex)
{
	SDL_DestroyMutex(mutex);
}

bool ctMutexLock(ctMutex mutex)
{
	return SDL_LockMutex(mutex) == 0;
}

bool ctMutexTryLock(ctMutex mutex)
{
	return SDL_TryLockMutex(mutex) == 0;
}

bool ctMutexUnlock(ctMutex mutex)
{
	return SDL_UnlockMutex(mutex) == 0;
}
