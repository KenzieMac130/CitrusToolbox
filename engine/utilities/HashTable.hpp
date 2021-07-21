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

/* See: https://github.com/jamesroutley/write-a-hash-table.
 In this implementation we use open addressing instead of double hashing.
 Containers use zipped arrays instead of linked lists for performance.
 Collisions are expected to be resolved offline or by the key being a mask.
 A key value of 0 is reserved for empty items, make sure keys are never 0! */
template<class T, class K>
class CT_API_EXP ctHashTable {
public:
   ctHashTable();
   ctHashTable(const size_t baseSize);
   /* Key must never be 0! */
   T* Insert(const K key, const T& value);
   /* Key must never be 0! */
   T* Insert(const K key, T&& value);
   T* FindPtr(const K key) const;
   /* Does not call destructor or free for value! */
   void Remove(const K key);
   bool isEmpty() const;
   bool Exists(const K key) const;
   size_t Count() const;
   size_t Capacity() const;
   ctResults Reserve(const size_t amount);

   class CT_API_EXP Iterator {
   public:
      Iterator(ctHashTable<T, K>* pTable);
      T& Value() const;
      const K& Key() const;
      inline Iterator& operator++() {
         ctAssert(pTable);
         currentIdx++;
         findNextValid();
         return *this;
      }
      inline Iterator operator++(int) {
         Iterator tmp = *this;
         ++*this;
         return tmp;
      }
      inline operator bool() const {
         return currentIdx < pTable->_Capacity;
      }

   private:
      inline void findNextValid();
      ctHashTable<T, K>* pTable;
      size_t currentIdx;
   };
   /* Only call if not empty */
   Iterator GetIterator() {
      return Iterator(this);
   }

private:
   K* _pKeys;
   T* _pValues;
   size_t _Capacity;
   size_t _Count;
   size_t _baseSize;
};

template<class T, class K>
inline ctHashTable<T, K>::ctHashTable() {
   _Count = 0;
   _baseSize = 0;
}

template<class T, class K>
inline ctHashTable<T, K>::ctHashTable(const size_t baseSize) {
   _baseSize = baseSize > 0 ? baseSize : 1;

   size_t capacity = ctNextPrime(_baseSize);
   _pValues = new T[capacity];
   _pKeys = new K[capacity];
   memset(_pKeys, 0, sizeof(K) * capacity);
   _Capacity = capacity;
   _Count = 0;
}

#define _HASH_LOOP_BEGIN(_capacity_)                                                     \
   for (K attempt = 0; attempt < _capacity_; attempt++) {                                \
      const K idx = (key + attempt) % _capacity_;
#define _HASH_LOOP_END }

template<class T, class K>
inline T* ctHashTable<T, K>::Insert(const K key, const T& value) {
   if (key == 0) { return NULL; }
   if (!_pKeys || !_pValues) { Reserve(31); }

   /* Resize if needed */
   size_t load = Count() * 100 / Capacity();
   while (load > 70 || Count() >= Capacity()) {
      Reserve(_baseSize * 2);
      load = Count() * 100 / Capacity();
   }

   /* Look for empty slot and insert */
   _HASH_LOOP_BEGIN(Capacity()) {
      if (_pKeys[idx] == 0) {
         _pKeys[idx] = key;
         _pValues[idx] = value;
         _Count++;
         return &_pValues[idx];
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
inline T* ctHashTable<T, K>::FindPtr(const K key) const {
   if (key == 0) { return NULL; }
   if (!_pKeys || !_pValues) { return NULL; }
   _HASH_LOOP_BEGIN(Capacity()) {
      if (_pKeys[idx] == key) { return &_pValues[idx]; }
      _HASH_LOOP_END
   }
   return NULL;
}

template<class T, class K>
inline void ctHashTable<T, K>::Remove(const K key) {
   if (key == 0) { return; }
   if (!_pKeys || !_pValues) { return; }
   _HASH_LOOP_BEGIN(Capacity()) {
      if (_pKeys[idx] == key) {
         _pKeys[idx] = 0;
         _Count--;
         return;
      }
      _HASH_LOOP_END
   }
}

template<class T, class K>
inline bool ctHashTable<T, K>::isEmpty() const {
   return _Count == 0;
}

template<class T, class K>
inline bool ctHashTable<T, K>::Exists(const K key) const {
   return (FindPtr(key) != NULL);
}

template<class T, class K>
inline size_t ctHashTable<T, K>::Count() const {
   return _Count;
}

template<class T, class K>
inline size_t ctHashTable<T, K>::Capacity() const {
   return _Capacity;
}

template<class T, class K>
inline ctResults ctHashTable<T, K>::Reserve(const size_t baseSize) {
   if (baseSize <= _baseSize) { return CT_SUCCESS; }
   _baseSize = baseSize;

   T* oldValues = _pValues;
   K* oldKeys = _pKeys;
   size_t oldCapacity = _Capacity;

   size_t capacity = ctNextPrime(baseSize);
   _pValues = new T[capacity];
   _pKeys = new K[capacity];
   memset(_pKeys, 0, sizeof(K) * capacity);
   _Capacity = capacity;
   _Count = 0;

   if (!oldKeys || !oldValues) { return CT_SUCCESS; }

   for (size_t i = 0; i < oldCapacity; i++) {
      if (oldKeys[i] != 0) { Insert(oldKeys[i], oldValues[i]); }
   }

   delete[] oldValues;
   delete[] oldKeys;

   return CT_SUCCESS;
}

template<class T, class K>
inline ctHashTable<T, K>::Iterator::Iterator(ctHashTable<T, K>* _pTable) {
   ctAssert(pTable);
   pTable = _pTable;
   currentIdx = 0;
   findNextValid();
}

template<class T, class K>
inline T& ctHashTable<T, K>::Iterator::Value() const {
   ctAssert(pTable);
   ctAssert(currentIdx < pTable->_Capacity);
   return pTable->_pValues[currentIdx];
}

template<class T, class K>
inline const K& ctHashTable<T, K>::Iterator::Key() const {
   ctAssert(pTable);
   ctAssert(currentIdx < pTable->_Capacity);
   return pTable->_pKeys[currentIdx];
}

template<class T, class K>
inline void ctHashTable<T, K>::Iterator::findNextValid() {
   if (currentIdx < pTable->_Capacity) {
      while (pTable->_pKeys[currentIdx] == 0) {
         currentIdx++;
         if (currentIdx >= pTable->_Capacity) { break; }
      }
   }
}