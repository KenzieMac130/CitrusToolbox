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

/*Todo: Itterator*/

template<class T, class K>
class ctHashTable {
public:
   ctHashTable();
   ctHashTable(const size_t capacity);
   T* Insert(const K key, const T& value);
   T* Insert(const K key, T&& value);
   T* FindPtr(const K key);
   void Remove(const K key);
   bool isEmpty() const;
   size_t Count() const;
   size_t Capacity() const;

private:
   ctDynamicArray<K> _keys;
   ctDynamicArray<T> _values;
   size_t _actualCount;
};

template<class T, class K>
inline ctHashTable<T, K>::ctHashTable() {
   _actualCount = 0;
}

template<class T, class K>
inline ctHashTable<T, K>::ctHashTable(const size_t reserve) {
   _values.Resize(reserve);
   _keys.Resize(reserve);
   _keys.SetBytes(0);
   _actualCount = 0;
}

#define _HASH_LOOP_BEGIN                                                       \
   for (K attempt = 0; attempt < Capacity(); attempt++) {                      \
      const K idx = (key + attempt) % Capacity();
#define _HASH_LOOP_END }

template<class T, class K>
inline T* ctHashTable<T, K>::Insert(const K key, const T& value) {
   if (key == 0) { return NULL; }
   if (Count() >= Capacity()) { return NULL; }
   _HASH_LOOP_BEGIN {
      if (_keys[idx] == 0) {
         _keys[idx] = key;
         _values[idx] = value;
         _actualCount++;
         return &_values[idx];
      }
      _HASH_LOOP_END
   }
   return NULL;
}

template<class T, class K>
inline T* ctHashTable<T, K>::Insert(const K key, T&& value) {
   return Insert(key, value);
}

template<class T, class K>
inline T* ctHashTable<T, K>::FindPtr(const K key) {
   if (key == 0) { return NULL; }
   _HASH_LOOP_BEGIN {
      if (_keys[idx] == key) { return &_values[idx]; }
      _HASH_LOOP_END
   }
   return NULL;
}

template<class T, class K>
inline void ctHashTable<T, K>::Remove(const K key) {
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

template<class T, class K>
inline bool ctHashTable<T, K>::isEmpty() const {
   return _keys.isEmpty();
}

template<class T, class K>
inline size_t ctHashTable<T, K>::Count() const {
   return _actualCount;
}

template<class T, class K>
inline size_t ctHashTable<T, K>::Capacity() const {
   return _keys.Count();
}
