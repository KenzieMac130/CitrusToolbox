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

#include "utilities/Common.h"
#include "utilities/BloomFilter.hpp"

/* To find entries in a cell we take a simple spacial key (K) and
 * use it to index into the bucket count hash map to (N) many buckets are associated
 * with that spot in the world. We then take that spacial key and look into the bucket
 * hash map for the key (K) with (N) number of retries for hash collisions */

/*
 *           X Coord                  Y Coord                  Z Coord           1
 * XXXX XXXX XXXX XXXX XXXX XYYY YYYY YYYY YYYY YYYY YYZZ ZZZZ ZZZZ ZZZZ ZZZZ ZZZ1
 *
 * 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0001 1
 * 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 00ZZ ZZZZ ZZZZ ZZZZ ZZZZ ZZZ0 Z
 * 0000 0000 0000 0000 0000 0YYY YYYY YYYY YYYY YYYY YY00 0000 0000 0000 0000 0000 Y
 * XXXX XXXX XXXX XXXX XXXX X000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 X
 */

struct CT_API ctSpacialCellKey {
   inline ctSpacialCellKey() {
      memset(this, 0, sizeof(*this));
   }
   inline ctSpacialCellKey(ctVec3 v) {
      const unsigned int x = ((unsigned int)v.x + 1048575) % 2097152;
      const unsigned int y = ((unsigned int)v.y + 1048575) % 2097152;
      const unsigned int z = ((unsigned int)v.z + 1048575) % 2097152;
      data = x;
      data *= 2097152;
      data += y;
      data *= 2097152;
      data += z;
      data |= 0x8000000000000000;
   }
   uint64_t data;
};

inline bool operator==(const ctSpacialCellKey a, const ctSpacialCellKey b) {
   return memcmp(&a, &b, sizeof(ctSpacialCellKey)) == 0;
}

struct CT_API ctSpacialCellBucket {
   inline ctSpacialCellBucket() {
      memset(entries, 0, sizeof(ctHandle) * CT_MAX_SPACIAL_QUERY_ENTRIES_PER_CELL);
   }
   inline ctSpacialCellBucket(ctHandle first) {
      memset(entries, 0, sizeof(ctHandle) * CT_MAX_SPACIAL_QUERY_ENTRIES_PER_CELL);
      entries[0] = first;
   };
   inline int FindEmptySlot() {
      for (int i = 0; i < CT_MAX_SPACIAL_QUERY_ENTRIES_PER_CELL; i++) {
         if (entries[i] == 0) { return i; }
      }
      return -1;
   }
   inline bool RemoveEntry(ctHandle entry) {
      for (int i = 0; i < CT_MAX_SPACIAL_QUERY_ENTRIES_PER_CELL; i++) {
         if (entries[i] == entry) {
            entries[i] = 0;
            return true;
         }
      }
      return false;
   }
   ctHandle entries[CT_MAX_SPACIAL_QUERY_ENTRIES_PER_CELL];
};

class CT_API ctSpacialQuery {
public:
   inline uint32_t GetBucketCount(ctSpacialCellKey k) const;
   inline ctSpacialCellBucket* GetBucket(const ctSpacialCellKey k,
                                         const uint32_t i) const;

   inline void Reserve(size_t amount);
   inline void Add(ctHandle v, ctSpacialCellKey k);
   inline void Remove(ctHandle v, ctSpacialCellKey k);
   inline void Reset();

private:
   inline bool FindFalsePositive(ctSpacialCellKey k) const;
   ctBloomFilter<ctSpacialCellKey, 8096, 4> bloom;
   // ctBloomFilter<ctSpacialCellKey, 1024, 4> potentialFalsePositive;
   // ctDynamicArray<ctSpacialCellKey> falsePositives;
   ctHashTable<uint32_t, uint64_t> counts;
   ctHashTable<ctSpacialCellBucket, uint64_t> buckets;
};

inline bool ctSpacialQuery::FindFalsePositive(ctSpacialCellKey k) const {
   // if (!potentialFalsePositive.MightExist(k)) { return false; }
   // if (falsePositives.FindIndex(k) >= 0) { return true; }
   return false;
}

inline uint32_t ctSpacialQuery::GetBucketCount(ctSpacialCellKey k) const {
   ZoneScoped;
   if (!bloom.MightExist(k)) { return 0; }
   if (FindFalsePositive(k)) { return 0; }
   const uint32_t* pNum = counts.FindPtr(k.data);
   if (pNum) { return *pNum; }
   return 0;
}

inline ctSpacialCellBucket* ctSpacialQuery::GetBucket(ctSpacialCellKey k,
                                                      uint32_t i) const {
   ZoneScoped;
   return buckets.FindPtr(k.data, (int)i);
}

inline void ctSpacialQuery::Reserve(size_t amount) {
   counts.Reserve(amount);
   buckets.Reserve(amount);
}

inline void ctSpacialQuery::Add(ctHandle v, ctSpacialCellKey k) {
   ZoneScoped;
   // if (!falsePositives.isEmpty()) {
   //   falsePositives.Clear();
   //   potentialFalsePositive.Reset();
   //}
   uint32_t* pBucketCount = NULL;
   if (bloom.MightExist(k)) { pBucketCount = counts.FindPtr(k.data); }
   if (pBucketCount) {
      const uint32_t bucketCount = *pBucketCount;
      for (uint32_t i = 0; i < bucketCount; i++) {
         ctSpacialCellBucket* pBucket = buckets.FindPtr(k.data, i);
         ctAssert(pBucket);
         int emptySlot = pBucket->FindEmptySlot();
         if (emptySlot > 0) {
            pBucket->entries[emptySlot] = v;
            return;
         }
      }
      ++*pBucketCount;
      buckets.Insert(k.data, ctSpacialCellBucket(v));
   } else {
      bloom.Insert(k);
      counts.Insert(k.data, 1);
      buckets.Insert(k.data, ctSpacialCellBucket(v));
   }
}

inline void ctSpacialQuery::Remove(ctHandle v, ctSpacialCellKey k) {
   ZoneScoped;
   const uint32_t count = GetBucketCount(k);
   for (uint32_t i = 0; i < count; i++) {
      ctSpacialCellBucket* pBucket = buckets.FindPtr(k.data, i);
      ctAssert(pBucket);
      if (pBucket->RemoveEntry(v)) { return; }
   }
}

inline void ctSpacialQuery::Reset() {
   ZoneScoped;
   // falsePositives.Clear();
   // potentialFalsePositive.Reset();
   bloom.Reset();
   counts.Clear();
   buckets.Clear();
}