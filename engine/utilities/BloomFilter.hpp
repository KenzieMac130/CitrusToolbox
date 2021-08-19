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

template<class T, size_t TBYTECOUNT, int THASHCOUNT>
class ctBloomFilter {
public:
   inline ctBloomFilter() {
      Reset();
   };
   void Reset();
   void Insert(const T& val);
   bool MightExist(const T& val) const;

private:
   inline void SetBit(const uint32_t pos);
   inline int GetBit(const uint32_t pos) const;
   uint8_t bytes[TBYTECOUNT];
};

template<class T, size_t TBYTECOUNT, int THASHCOUNT>
inline void ctBloomFilter<T, TBYTECOUNT, THASHCOUNT>::Reset() {
   memset(bytes, 0, TBYTECOUNT);
}

template<class T, size_t TBYTECOUNT, int THASHCOUNT>
inline void ctBloomFilter<T, TBYTECOUNT, THASHCOUNT>::Insert(const T& val) {
   for (int i = 0; i < THASHCOUNT; i++) {
      SetBit(ctXXHash32(&val, sizeof(val), i) % (TBYTECOUNT * 8));
   }
}

template<class T, size_t TBYTECOUNT, int THASHCOUNT>
inline bool ctBloomFilter<T, TBYTECOUNT, THASHCOUNT>::MightExist(const T& val) const {
   for (int i = 0; i < THASHCOUNT; i++) {
      if (!GetBit(ctXXHash32(&val, sizeof(val), i) % (TBYTECOUNT * 8))) { return false; }
   }
   return true;
}

template<class T, size_t TBYTECOUNT, int THASHCOUNT>
inline void ctBloomFilter<T, TBYTECOUNT, THASHCOUNT>::SetBit(const uint32_t pos) {
   const int byte = pos / 8;
   const int bit = pos % 8;
   ctAssert(byte < TBYTECOUNT);
   bytes[byte] |= 1 << bit;
}

template<class T, size_t TBYTECOUNT, int THASHCOUNT>
inline int ctBloomFilter<T, TBYTECOUNT, THASHCOUNT>::GetBit(const uint32_t pos) const {
   const unsigned int byte = pos / 8;
   const unsigned int bit = pos % 8;
   ctAssert(byte < TBYTECOUNT);
   return (int)((bytes[byte] & (1 << bit)) >> bit);
}
