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

template<class T,class K=uint32_t>
class ctIndirectTable {
public:
   ctIndirectTable();
   ctIndirectTable(const size_t reserve);
   void Reserve(const size_t amount);
   ctResults Insert(const K& key, const T& value);
   ctResults Insert(const K& key, T&& value);
   T* FindPtr(const K& key) const;
   void Remove(const K& key);
   bool isEmpty() const;
   bool Exists(const K& key) const;

private:
   ctDynamicArray<K> _keys;
   ctDynamicArray<T> _values;
};

template<class T,class K>
inline ctIndirectTable<T,K>::ctIndirectTable(const size_t reserve) {
   Reserve(reserve);
}

template<class T,class K>
inline void ctIndirectTable<T,K>::Reserve(const size_t amount) {
   _keys.Reserve(amount);
   _values.Reserve(amount);
}

template<class T,class K>
inline ctIndirectTable<T,K>::ctIndirectTable() {
}

template<class T,class K>
inline ctResults ctIndirectTable<T,K>::Insert(const K& key, const T& value) {
   if (Exists(key)) { return CT_FAILURE_DUPLICATE_ENTRY; }
   _keys.Append(key);
   _values.Append(value);
   return CT_SUCCESS;
}

template<class T,class K>
inline ctResults ctIndirectTable<T,K>::Insert(const K& key, T&& value) {
   if (Exists(key)) { return CT_FAILURE_DUPLICATE_ENTRY; }
   _keys.Append(key);
   _values.Append(value);
   return CT_SUCCESS;
}

template<class T,class K>
inline T* ctIndirectTable<T,K>::FindPtr(const K& key) const {
   const int64_t idx = _keys.FindIndex(key, 0);
   if (idx >= 0) { return &_values[idx]; }
   return NULL;
}

template<class T,class K>
inline void ctIndirectTable<T,K>::Remove(const K& key) {
   const int64_t idx = _keys.FindIndex(key, 0);
   if (idx >= 0) {
      _values.RemoveAt(idx);
      _keys.RemoveAt(idx);
   }
}

template<class T,class K>
inline bool ctIndirectTable<T,K>::isEmpty() const {
   return _keys.isEmpty();
}

template<class T,class K>
inline bool ctIndirectTable<T,K>::Exists(const K& key) const {
   return _keys.Exists(key);
}
