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

template<class T>
class ctHashTable {
public:
   ctHashTable();
   ctHashTable(const size_t capacity);
   ctResults Insert(const uint32_t key, const T& value);
   ctResults Insert(const uint32_t key, T&& value);
   T* FindPtr(const uint32_t key);
   void Remove(const uint32_t key);
   bool isEmpty() const;
   size_t Count() const;
   size_t Capacity() const;

private:
   ctDynamicArray<uint32_t> _keys;
   ctDynamicArray<T> _values;
   size_t _actualCount;
};

template<class T>
inline ctHashTable<T>::ctHashTable() {
   _actualCount = 0;
}

template<class T>
inline ctHashTable<T>::ctHashTable(const size_t reserve) {
   _values.Resize(reserve);
   _keys.Resize(reserve);
   _keys.SetBytes(0);
   _actualCount = 0;
}

#define _HASH_LOOP_BEGIN                                                       \
   for (uint32_t attempt = 0; attempt < Capacity(); attempt++) {               \
      const uint32_t idx = (key + attempt) % Capacity();
#define _HASH_LOOP_END }

template<class T>
inline ctResults ctHashTable<T>::Insert(const uint32_t key, const T& value) {
   if (key == 0) { return CT_FAILURE_INVALID_PARAMETER; }
   if (Count() >= Capacity()) { return CT_FAILURE_OUT_OF_MEMORY; }
   _HASH_LOOP_BEGIN {
      if (_keys[idx] == 0) {
         _keys[idx] = key;
         _values[idx] = value;
         _actualCount++;
         return CT_SUCCESS;
      }
      _HASH_LOOP_END
   }
   return CT_FAILURE_UNKNOWN;
}

template<class T>
inline ctResults ctHashTable<T>::Insert(const uint32_t key, T&& value) {
   return Insert(key, value);
}

template<class T>
inline T* ctHashTable<T>::FindPtr(const uint32_t key) {
   if (key == 0) { return NULL; }
   _HASH_LOOP_BEGIN {
      if (_keys[idx] == key) { return &_values[idx]; }
      _HASH_LOOP_END
   }
   return NULL;
}

template<class T>
inline void ctHashTable<T>::Remove(const uint32_t key) {
   if (key == 0) { return; }
   _HASH_LOOP_BEGIN {
      if (_keys[idx] == key) {
         _keys[idx] = 0;
         _actualCount--;
         return;
      }
      _HASH_LOOP_END
   }
}

template<class T>
inline bool ctHashTable<T>::isEmpty() const {
   return _keys.isEmpty();
}

template<class T>
inline size_t ctHashTable<T>::Count() const {
   return _actualCount;
}

template<class T>
inline size_t ctHashTable<T>::Capacity() const {
   return _keys.Count();
}
