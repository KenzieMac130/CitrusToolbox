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

#pragma once

#include "Common.h"

template<class T, size_t TCAPACITY>
class ctStaticArray {
public:
   /* Constructors */
   ctStaticArray();
   ctStaticArray(const T* arr, const size_t amount);
   ctStaticArray(const ctStaticArray<T, TCAPACITY>& arr);
   ~ctStaticArray();
   /* Data */
   T* Data() const;
   /* Count */
   size_t Count() const;
   size_t Capacity() const;
   /* Array Access */
   T& operator[](const size_t index);
   T operator[](const size_t index) const;
   T& First();
   T First() const;
   T& Last();
   T Last() const;
   /* Iteration */
   T* Begin();
   const T* Begin() const;
   T* End();
   const T* End() const;
   /* Resize */
   ctResults Resize(const size_t amount);
   /* Append */
   ctResults Append(T&& val);
   ctResults Append(const T& val);
   size_t Append(const T* pArray, const size_t length);
   size_t Append(const T& val, const size_t amount);
   /* Insert */
   ctResults Insert(const T& val, const int64_t position);
   ctResults InsertUnique(const T& val);
   /* Remove */
   void RemoveAt(const int64_t position);
   ctResults Remove(const T& val);
   void RemoveLast();
   /* Clear */
   void Clear();
   /* Memset and Clear */
   void SetBytes(int val);
   /* isEmpty */
   bool isEmpty() const;
   /* Exists */
   bool Exists(const T& val) const;
   /* Find */
   int64_t FindIndex(const T& val, const int64_t position = 0, const int step = 1) const;
   T* FindPtr(const T& val, const int64_t position = 0, const int step = 1) const;
   T& Fetch(const T& val) const;
   /* Sort */
   void
   QSort(const size_t position, const size_t amount, int (*compare)(const T*, const T*));
   /* Reverse */
   void Reverse();

   /* Hash */
   uint32_t xxHash32(const size_t position, const size_t amount, const int seed) const;
   uint32_t xxHash32(const int seed) const;
   uint32_t xxHash32() const;
   uint64_t xxHash64(const size_t position, const size_t amount, const int seed) const;
   uint64_t xxHash64(const int seed) const;
   uint64_t xxHash64() const;

private:
   size_t _count;
   T _pData[TCAPACITY];
};

template<class T, size_t TCAPACITY>
inline ctStaticArray<T, TCAPACITY>::ctStaticArray() {
   _count = 0;
   SetBytes(0);
}

template<class T, size_t TCAPACITY>
inline ctStaticArray<T, TCAPACITY>::ctStaticArray(const T* arr, const size_t amount) {
   _count = 0;
   SetBytes(0);
   Append(arr, amount);
}

template<class T, size_t TCAPACITY>
inline ctStaticArray<T, TCAPACITY>::ctStaticArray(
  const ctStaticArray<T, TCAPACITY>& arr) {
}

template<class T, size_t TCAPACITY>
inline ctStaticArray<T, TCAPACITY>::~ctStaticArray() {
}

template<class T, size_t TCAPACITY>
inline T* ctStaticArray<T, TCAPACITY>::Data() const {
   return (T*)_pData;
}

template<class T, size_t TCAPACITY>
inline size_t ctStaticArray<T, TCAPACITY>::Count() const {
   return _count;
}

template<class T, size_t TCAPACITY>
inline size_t ctStaticArray<T, TCAPACITY>::Capacity() const {
   return TCAPACITY;
}

template<class T, size_t TCAPACITY>
inline T& ctStaticArray<T, TCAPACITY>::operator[](const size_t index) {
   ctAssert(index < Count());
   return _pData[index];
}

template<class T, size_t TCAPACITY>
inline T ctStaticArray<T, TCAPACITY>::operator[](const size_t index) const {
   ctAssert(index < Count());
   return _pData[index];
}

template<class T, size_t TCAPACITY>
inline T& ctStaticArray<T, TCAPACITY>::First() {
   ctAssert(Count() > 0);
   return _pData[0];
}

template<class T, size_t TCAPACITY>
inline T ctStaticArray<T, TCAPACITY>::First() const {
   ctAssert(Count() > 0);
   return _pData[0];
}

template<class T, size_t TCAPACITY>
inline T& ctStaticArray<T, TCAPACITY>::Last() {
   ctAssert(Count() > 0);
   size_t idx = Count() - 1 > 0 ? Count() - 1 : 0;
   return _pData[idx];
}

template<class T, size_t TCAPACITY>
inline T ctStaticArray<T, TCAPACITY>::Last() const {
   ctAssert(Count() > 0);
   size_t idx = Count() - 1 > 0 ? Count() - 1 : 0;
   return _pData[idx];
}

template<class T, size_t TCAPACITY>
inline T* ctStaticArray<T, TCAPACITY>::Begin() {
   ctAssert(_pData);
   return pData;
}

template<class T, size_t TCAPACITY>
inline const T* ctStaticArray<T, TCAPACITY>::Begin() const {
   ctAssert(_pData);
   return pData;
}

template<class T, size_t TCAPACITY>
inline T* ctStaticArray<T, TCAPACITY>::End() {
   ctAssert(_pData);
   return pData + Count();
}

template<class T, size_t TCAPACITY>
inline const T* ctStaticArray<T, TCAPACITY>::End() const {
   ctAssert(_pData);
   return pData + Count();
}

template<class T, size_t TCAPACITY>
inline ctResults ctStaticArray<T, TCAPACITY>::Resize(const size_t amount) {
   if (amount > Capacity()) { return CT_FAILURE_OUT_OF_BOUNDS; }
   _count = amount;
   return CT_SUCCESS;
}

template<class T, size_t TCAPACITY>
inline ctResults ctStaticArray<T, TCAPACITY>::Append(T&& val) {
   if (Count() >= Capacity()) { return CT_FAILURE_OUT_OF_BOUNDS; }
   _pData[Count()] = val;
   _count++;
   return CT_SUCCESS;
}

template<class T, size_t TCAPACITY>
inline ctResults ctStaticArray<T, TCAPACITY>::Append(const T& val) {
   if (Count() >= Capacity()) { return CT_FAILURE_OUT_OF_BOUNDS; }
   _pData[Count()] = val;
   _count++;
   return CT_SUCCESS;
}

template<class T, size_t TCAPACITY>
inline size_t ctStaticArray<T, TCAPACITY>::Append(const T* pArray, const size_t amount) {
   size_t finalcount = amount + Count() > Capacity() ? Capacity() : amount + Count();
   for (int i = _count; i < finalcount; i++) {
      _pData[i] = pArray[i];
   }
   const size_t writtenamount = finalcount - _count;
   _count = finalcount;
   return writtenamount;
}

template<class T, size_t TCAPACITY>
inline size_t ctStaticArray<T, TCAPACITY>::Append(const T& val, const size_t amount) {
   size_t finalcount = amount + Count() > Capacity() ? Capacity() : amount + Count();
   for (int i = _count; i < finalcount; i++) {
      _pData[i] = val;
   }
   const size_t writtenamount = finalcount - _count;
   _count = finalcount;
   return writtenamount;
}

template<class T, size_t TCAPACITY>
inline ctResults ctStaticArray<T, TCAPACITY>::Insert(const T& val,
                                                     const int64_t position) {
   if (Count() >= Capacity()) { return CT_FAILURE_OUT_OF_BOUNDS; }
   int64_t finalposition = position < 0 ? Count() + 1 + position : position;
   if (finalposition < 0) { finalposition = 0; }
   if (finalposition > (int64_t)Count()) { finalposition = Count(); }
   for (int64_t i = Count(); i > finalposition; i--) {
      _pData[i] = _pData[i - 1];
   }
   _count++;
   _pData[finalposition] = val;
   return CT_SUCCESS;
}

template<class T, size_t TCAPACITY>
inline ctResults ctStaticArray<T, TCAPACITY>::InsertUnique(const T& val) {
   if (Exists(val)) { return CT_FAILURE_DUPLICATE_ENTRY; }
   return Append(val);
}

template<class T, size_t TCAPACITY>
inline void ctStaticArray<T, TCAPACITY>::RemoveAt(const int64_t position) {
   const int64_t finalposition = position < 0 ? Count() + position : position;
   if (finalposition < 0 || finalposition >= (int64_t)Count()) { return; }
   _count--;
   for (int64_t i = finalposition; i < (int64_t)Count(); i++) {
      _pData[i] = _pData[i + 1];
   }
}

template<class T, size_t TCAPACITY>
inline ctResults ctStaticArray<T, TCAPACITY>::Remove(const T& val) {
   int64_t idx = FindIndex(val, position, 1);
   if (idx >= 0 && idx < (int64_t)Count()) {
      RemoveAt(idx);
      return CT_SUCCESS;
   } else {
      return CT_FAILURE_DATA_DOES_NOT_EXIST;
   }
}

template<class T, size_t TCAPACITY>
inline void ctStaticArray<T, TCAPACITY>::RemoveLast() {
   RemoveAt(-1);
}

template<class T, size_t TCAPACITY>
inline void ctStaticArray<T, TCAPACITY>::Clear() {
   _count = 0;
}

template<class T, size_t TCAPACITY>
inline void ctStaticArray<T, TCAPACITY>::SetBytes(int val) {
   Clear();
   memset(Data(), val, Capacity() * sizeof(T));
}

template<class T, size_t TCAPACITY>
inline bool ctStaticArray<T, TCAPACITY>::isEmpty() const {
   return _count == 0;
}

template<class T, size_t TCAPACITY>
inline bool ctStaticArray<T, TCAPACITY>::Exists(const T& val) const {
   return FindIndex(val, 0, 1) != -1 ? true : false;
}

template<class T, size_t TCAPACITY>
inline int64_t ctStaticArray<T, TCAPACITY>::FindIndex(const T& val,
                                                      const int64_t position,
                                                      const int direction) const {
   if (isEmpty()) { return -1; }
   const int64_t amount = (int64_t)Count();
   const int64_t finalposition = position < 0 ? Count() + position : position;
   const int finaldirecton = direction >= 0 ? 1 : -1;
   for (int64_t i = finalposition; i < amount; i += finaldirecton) {
      if (i >= amount || i < 0) { return -1; }
      if (Data()[i] == val) { return i; }
   }
   return -1;
}

template<class T, size_t TCAPACITY>
inline T* ctStaticArray<T, TCAPACITY>::FindPtr(const T& val,
                                               const int64_t position,
                                               const int direction) const {
   if (isEmpty()) { return NULL; }
   const int64_t idx = FindIndex(val, position, direction);
   return idx >= 0 ? &(Data()[idx]) : NULL;
}

template<class T, size_t TCAPACITY>
inline T& ctStaticArray<T, TCAPACITY>::Fetch(const T& val) const {
   int64_t idx = FindIndex(val);
   ctAssert(idx >= 0);
   return Data()[idx];
}

template<class T, size_t TCAPACITY>
inline void ctStaticArray<T, TCAPACITY>::QSort(const size_t position,
                                               const size_t amount,
                                               int (*compare)(const T*, const T*)) {
   if (isEmpty()) { return; }
   const size_t remaining_count = Count() - position;
   const size_t final_amount = amount > remaining_count ? remaining_count : amount;
   qsort(Data(), final_amount, sizeof(T), (int (*)(void const*, void const*))compare);
}

template<class T, size_t TCAPACITY>
inline void ctStaticArray<T, TCAPACITY>::Reverse() {
   if (isEmpty()) { return; }
   size_t left = 0;
   size_t right = Count() - 1;
   while (left < right) {
      T tmp = _pData[left];
      _pData[left] = _pData[right];
      _pData[right] = tmp;
      left++;
      right--;
   }
}

template<class T, size_t TCAPACITY>
inline uint32_t ctStaticArray<T, TCAPACITY>::xxHash32(const size_t position,
                                                      const size_t amount,
                                                      const int seed) const {
   if (isEmpty()) { return 0; }
   return XXH32((const void*)(Data() + position), amount, seed);
}

template<class T, size_t TCAPACITY>
inline uint32_t ctStaticArray<T, TCAPACITY>::xxHash32(const int seed) const {
   return xxHash32(0, Count(), seed);
}

template<class T, size_t TCAPACITY>
inline uint32_t ctStaticArray<T, TCAPACITY>::xxHash32() const {
   return xxHash32(0);
}

template<class T, size_t TCAPACITY>
inline uint64_t ctStaticArray<T, TCAPACITY>::xxHash64(const size_t position,
                                                      const size_t amount,
                                                      const int seed) const {
   if (isEmpty()) { return 0; }
   return XXH64((const void*)(Data() + position), amount, seed);
}

template<class T, size_t TCAPACITY>
inline uint64_t ctStaticArray<T, TCAPACITY>::xxHash64(const int seed) const {
   return xxHash64(0, Count(), seed);
}

template<class T, size_t TCAPACITY>
inline uint64_t ctStaticArray<T, TCAPACITY>::xxHash64() const {
   return xxHash64(0);
}
