#pragma once

#include "Common.h"

template<class T>
class ctHashTable {
public:
   ctHashTable();
   ctHashTable(const size_t reserve);
   void Reserve(const size_t amount);
   ctResults Insert(const uint32_t hash, const T& value);
   ctResults Insert(const uint32_t hash, T&& value);
   T* FindPtr(const uint32_t hash) const;
   void Remove(const uint32_t hash);
   bool isEmpty() const;
   bool Exists(const uint32_t hash) const;

private:
   ctDynamicArray<uint32_t> _hashes;
   ctDynamicArray<T*> _pValues;
};

template<class T>
inline ctHashTable<T>::ctHashTable(const size_t reserve) {
   Reserve(reserve);
}

template<class T>
inline void ctHashTable<T>::Reserve(const size_t amount) {
   _hashes.Reserve(amount);
   _pValues.Reserve(amount);
}

template<class T>
inline ctHashTable<T>::ctHashTable() {
}

template<class T>
inline ctResults ctHashTable<T>::Insert(const uint32_t hash, const T& value) {
   if (Exists(hash)) { return CT_FAILURE_DUPLICATE_ENTRY; }
   _hashes.Append(hash);
   _pValues.Append(new T(value));
   return CT_SUCCESS;
}

template<class T>
inline ctResults ctHashTable<T>::Insert(const uint32_t hash, T&& value) {
   if (Exists(hash)) { return CT_FAILURE_DUPLICATE_ENTRY; }
   _hashes.Append(hash);
   _pValues.Append(new T(value));
   return CT_SUCCESS;
}

template<class T>
inline T* ctHashTable<T>::FindPtr(const uint32_t hash) const {
   const int64_t idx = _hashes.FindIndex(hash, 0);
   if (idx >= 0) { return _pValues[idx]; }
   return NULL;
}

template<class T>
inline void ctHashTable<T>::Remove(const uint32_t hash) {
   const int64_t idx = _hashes.FindIndex(hash, 0);
   if (idx >= 0) {
      delete _pValues[idx];
      _pValues.RemoveAt(idx);
      _hashes.RemoveAt(idx);
   }
}

template<class T>
inline bool ctHashTable<T>::isEmpty() const {
   return _hashes.isEmpty();
}

template<class T>
inline bool ctHashTable<T>::Exists(const uint32_t hash) const {
   return _hashes.Exists(hash);
}
