#pragma once

#include "Common.h"

typedef SDL_mutex* ctMutex;

ctMutex ctMutexCreate();
void ctMutexDestroy(ctMutex mutex);

bool ctMutexLock(ctMutex mutex);
bool ctMutexTryLock(ctMutex mutex);
bool ctMutexUnlock(ctMutex mutex);