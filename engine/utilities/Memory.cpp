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

#include "Common.h"

struct alignedAllocTracker {
   void* rawMemory;
   size_t originalSize;
};

ctAtomic gAllocCount = ctAtomic();

void* ctAlignedMalloc(size_t size, size_t alignment) {
   ZoneScoped;
   const size_t allocSize = size + alignment + sizeof(alignedAllocTracker);
   char* rawMemory = (char*)malloc(allocSize);
   TracyAlloc(rawMemory, allocSize);
   ctAtomicAdd(gAllocCount, 1);
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
   if (block) {
      memcpy(pNew, block, ((alignedAllocTracker*)block)[-1].originalSize);
      ctAlignedFree(block);
   }
   return pNew;
}

void ctAlignedFree(void* block) {
   ZoneScoped;
   if (!block) { return; }
   void* pFinal = ((alignedAllocTracker*)block)[-1].rawMemory;
   TracyFree(pFinal);
   ctAtomicAdd(gAllocCount, -1);
   free(pFinal);
}

CT_API size_t ctGetAliveAllocations() {
   return (size_t)ctAtomicGet(gAllocCount);
}

void* ctMalloc(size_t size) {
   ZoneScoped;
   return ctAlignedMalloc(size, CT_ALIGNMENT_CACHE);
}

CT_API void* ctRealloc(void* old, size_t size) {
   ZoneScoped;
   return ctAlignedRealloc(old, size, CT_ALIGNMENT_CACHE);
}

void ctFree(void* block) {
   ZoneScoped;
   return ctAlignedFree(block);
}

CT_API void* ctGroupAlloc(size_t count, ctGroupAllocDesc* groups, size_t* pSizeOut) {
   ZoneScoped;
   size_t size = 0;
   for (size_t i = 0; i < count; i++) {
      ctAssert(groups[i].alignment != 0);
      size += groups[i].size + groups[i].alignment;
   }
   uint8_t* output = (uint8_t*)ctMalloc(size);
   uint8_t* offset = output;
   for (size_t i = 0; i < count; i++) {
      offset = (uint8_t*)ctAlign((size_t)offset, (ptrdiff_t)groups[i].alignment);
      *groups[i].output = offset;
      offset += groups[i].size;
      ctAssert((size_t)(offset - output) <= size);
   }
   if (pSizeOut) { *pSizeOut = size; }
   return output;
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