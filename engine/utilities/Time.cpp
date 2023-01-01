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

#include "Time.hpp"

ctStopwatch::ctStopwatch() {
   freq = SDL_GetPerformanceFrequency();
   lastTick = 0;
   currentTick = SDL_GetPerformanceCounter();
}

void ctStopwatch::NextLap() {
   lastTick = currentTick;
   currentTick = SDL_GetPerformanceCounter();
}

float ctStopwatch::GetDeltaTimeFloat() {
   return (float)GetDeltaTime();
}

double ctStopwatch::GetDeltaTime() {
   return ((double)(currentTick - lastTick) * 1000 / (double)freq) * 0.001;
}

uint64_t ctGetTimestamp() {
   return SDL_GetTicks();
}

void ctWait(uint32_t ms) {
   SDL_Delay(ms);
}
