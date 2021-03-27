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

#include "Common.h"

void* ctMalloc(size_t size) {
   ZoneScoped;
   void* ptr = malloc(size);
   TracyAlloc(ptr, size);
   return ptr;
}

void ctFree(void* block) {
   ZoneScoped;
   TracyFree(block);
   free(block);
}

struct alignedAllocTracker {
   void* rawMemory;
   size_t allocSize;
};

void* ctAlignedMalloc(size_t size, size_t alignment) {
   ZoneScoped;
   char* rawMemory =
     (char*)ctMalloc(size + alignment + sizeof(alignedAllocTracker));
   alignedAllocTracker* ptr =
     (alignedAllocTracker*)((uintptr_t)(rawMemory + alignment +
                                        sizeof(alignedAllocTracker)) &
                            ~(alignment - 1));
   ptr[-1] = {(void*)rawMemory, size};
   return (void*)ptr;
}

void* ctAlignedRealloc(void* block, size_t size, size_t alignment) {
   ZoneScoped;
   void* pNew = ctAlignedMalloc(size, alignment);
   memcpy(pNew, block, ((alignedAllocTracker*)block)[-1].allocSize);
   ctAlignedFree(block);
   return pNew;
}

void ctAlignedFree(void* block) {
   ZoneScoped;
   if (!block) { return; }
   ctFree(((alignedAllocTracker*)block)[-1].rawMemory);
}

void* operator new(size_t size) {
   ZoneScoped;
   void* ptr = ctMalloc(size);
   ctAssert(ptr);
   return ptr;
}

void operator delete(void* ptr) {
   ZoneScoped;
   ctFree(ptr);
}

void* operator new[](size_t size) {
   ZoneScoped;
   void* ptr = ctMalloc(size);
   ctAssert(ptr);
   return ptr;
}
void operator delete[](void* ptr) {
   ZoneScoped;
   ctFree(ptr);
}