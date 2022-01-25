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
class ctDynamicArray {
public:
   /* Constructors */
   ctDynamicArray();
   ctDynamicArray(ctDynamicArray<T>& arr);
   ctDynamicArray(const ctDynamicArray<T>& arr);
   /* Destructor */
   ~ctDynamicArray();
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
   /* Reserve */
   ctResults Resize(const size_t amount);
   ctResults Reserve(const size_t amount);
   /* Data */
   T* Data() const;
   /* Count */
   size_t Count() const;
   size_t Capacity() const;
   /* Assignment */
   ctDynamicArray<T>& operator=(const ctDynamicArray<T>& arr);
   /* Append */
   ctResults Append(T&& val);
   ctResults Append(const T& val);
   ctResults Append(const ctDynamicArray<T>& arr);
   ctResults Append(const T* pArray, const size_t length);
   ctResults Append(const T& val, const size_t amount);
   /* Insert */
   ctResults Insert(const T& val, const int64_t position);
   ctResults InsertUnique(const T& val);
   /* Remove */
   void RemoveAt(const int64_t position = 0);
   ctResults Remove(const T& val, const int64_t position = 0);
   void RemoveFirst();
   void RemoveLast();
   void RemoveAllOf(const T& val);
   /* Clear */
   void Clear();
   /* Memset */
   void Memset(int val);
   /* isEmpty */
   bool isEmpty() const;
   /* Exists */
   bool Exists(const T& val) const;
   /* Find */
   int64_t FindIndex(const T& val, const int64_t position = 0, const int step = 1) const;
   T* FindPtr(const T& val, const int64_t position = 0, const int step = 1) const;
   T& Fetch(const T& val) const;
   /* Sort */
   void QSort(int (*compare)(const T*, const T*),
              const size_t position = 0,
              const size_t amount = 0);
   /* Hash */
   uint32_t xxHash32(const size_t position, const size_t amount, const int seed) const;
   uint32_t xxHash32(const int seed) const;
   uint32_t xxHash32() const;
   uint64_t xxHash64(const size_t position, const size_t amount, const int seed) const;
   uint64_t xxHash64(const int seed) const;
   uint64_t xxHash64() const;

private:
   ctResults _expand_size(size_t amount);
   T* _pData;
   size_t _capacity;
   size_t _count;
};

template<class T>
inline ctResults ctDynamicArray<T>::_expand_size(size_t amount) {
   const size_t neededamount = Count() + amount;
   const size_t originalcapacity = Capacity();
   if (neededamount > originalcapacity) {
      size_t targetamount = Capacity();
      if (targetamount <= 0) { targetamount = 32; }
      while (targetamount < neededamount) {
         targetamount += 32;
      }
      return Reserve(targetamount);
   }
   return CT_SUCCESS;
}

template<class T>
inline ctDynamicArray<T>::ctDynamicArray() {
   _pData = NULL;
   _capacity = 0;
   _count = 0;
}

template<class T>
inline ctDynamicArray<T>::ctDynamicArray(const ctDynamicArray<T>& arr) {
   const size_t inputcount = arr.Count();
   Resize(inputcount);
   for (size_t i = 0; i < inputcount; i++) {
      _pData[i] = arr[i];
   }
}

template<class T>
inline ctDynamicArray<T>::ctDynamicArray(ctDynamicArray<T>& arr) {
   const size_t inputcount = arr.Count();
   Resize(inputcount);
   for (size_t i = 0; i < inputcount; i++) {
      _pData[i] = arr[i];
   }
}

template<class T>
inline ctDynamicArray<T>::~ctDynamicArray() {
   if (_pData) { delete[] _pData; }
   _pData = NULL;
   _capacity = 0;
   _count = 0;
}

template<class T>
inline T& ctDynamicArray<T>::operator[](const size_t index) {
   ctAssert(index < Count());
   ctAssert(_pData);
   return _pData[index];
}

template<class T>
inline T ctDynamicArray<T>::operator[](const size_t index) const {
   ctAssert(index < Count());
   ctAssert(_pData);
   return _pData[index];
}

template<class T>
inline T& ctDynamicArray<T>::First() {
   ctAssert(Count() > 0);
   ctAssert(_pData);
   return _pData[0];
}

template<class T>
inline T ctDynamicArray<T>::First() const {
   ctAssert(Count() > 0);
   ctAssert(_pData);
   return _pData[0];
}

template<class T>
inline T& ctDynamicArray<T>::Last() {
   size_t idx = Count() - 1 > 0 ? Count() - 1 : 0;
   ctAssert(Count() > 0);
   ctAssert(_pData);
   return _pData[idx];
}

template<class T>
inline T ctDynamicArray<T>::Last() const {
   size_t idx = Count() - 1 > 0 ? Count() - 1 : 0;
   ctAssert(Count() > 0);
   ctAssert(_pData);
   return _pData[idx];
}

template<class T>
inline T* ctDynamicArray<T>::Begin() {
   ctAssert(_pData);
   return pData;
}

template<class T>
inline const T* ctDynamicArray<T>::Begin() const {
   ctAssert(_pData);
   return pData;
}

template<class T>
inline T* ctDynamicArray<T>::End() {
   ctAssert(_pData);
   return pData + Count();
}

template<class T>
inline const T* ctDynamicArray<T>::End() const {
   ctAssert(_pData);
   return pData + Count();
}

template<class T>
inline ctResults ctDynamicArray<T>::Resize(const size_t amount) {
   if (amount == Count()) {
      return CT_SUCCESS;
   } else if (amount <= 0) {
      _pData = NULL;
      Clear();
      return CT_SUCCESS;
   }
   T* pOldData = _pData;
   _pData = new T[amount];
   ctAssert(_pData);
   if (pOldData) {
      for (size_t i = 0; i < amount; i++) {
         _pData[i] = pOldData[i];
      }
      delete[] pOldData;
   }
   _count = amount;
   _capacity = amount;
   return CT_SUCCESS;
}

template<class T>
inline ctResults ctDynamicArray<T>::Reserve(const size_t amount) {
   if (amount > Capacity()) {
      T* pOldData = _pData;
      _pData = new T[amount];
      ctAssert(_pData);
      if (pOldData) {
         for (size_t i = 0; i < Count(); i++) {
            _pData[i] = pOldData[i];
         }
         delete[] pOldData;
      }
      _capacity = amount;
   }
   return CT_SUCCESS;
}

template<class T>
inline T* ctDynamicArray<T>::Data() const {
   return _pData;
}

template<class T>
inline size_t ctDynamicArray<T>::Count() const {
   return _count;
}

template<class T>
inline size_t ctDynamicArray<T>::Capacity() const {
   return _capacity;
}

template<class T>
inline ctDynamicArray<T>& ctDynamicArray<T>::operator=(const ctDynamicArray<T>& arr) {
   if (arr.isEmpty()) { return *this; }
   const size_t inputcount = arr.Count();
   Resize(inputcount);
   for (size_t i = 0; i < inputcount; i++) {
      _pData[i] = arr[i];
   }
   return *this;
}

template<class T>
inline ctResults ctDynamicArray<T>::Append(const T& val) {
   const ctResults result = _expand_size(1);
   if (result != CT_SUCCESS) { return result; }
   if (_pData) { _pData[Count()] = val; }
   _count++;
   return result;
}

template<class T>
inline ctResults ctDynamicArray<T>::Append(T&& val) {
   return Append((const T&)val);
}

template<class T>
inline ctResults ctDynamicArray<T>::Append(const ctDynamicArray<T>& arr) {
   return Append(arr.Data(), arr.Count());
}

template<class T>
inline ctResults ctDynamicArray<T>::Append(const T* pArray, const size_t length) {
   const ctResults result = Reserve(Count() + length);
   if (result != CT_SUCCESS) { return result; }
   for (int i = 0; i < length; i++) {
      Append(pArray[i]);
   }
   return result;
}

template<class T>
inline ctResults ctDynamicArray<T>::Append(const T& val, const size_t amount) {
   const ctResults result = Reserve(Count() + amount);
   if (result != CT_SUCCESS) { return result; }
   for (int i = 0; i < amount; i++) {
      Append(val);
   }
   return result;
}

template<class T>
inline ctResults ctDynamicArray<T>::Insert(const T& val, const int64_t position) {
   int64_t finalposition = position < 0 ? Count() + position + 1 : position;
   const ctResults result = _expand_size(1);
   if (result != CT_SUCCESS) { return result; }
   if (finalposition < 0) { finalposition = 0; }
   if (finalposition > (int64_t)Count()) { finalposition = Count(); }
   for (int64_t i = Count(); i > finalposition; i--) {
      _pData[i] = _pData[i - 1];
   }
   _count++;
   _pData[finalposition] = val;
   return result;
}

template<class T>
inline ctResults ctDynamicArray<T>::InsertUnique(const T& val) {
   if (Exists(val)) { return CT_FAILURE_DUPLICATE_ENTRY; }
   return Append(val);
}

template<class T>
inline void ctDynamicArray<T>::RemoveAt(const int64_t position) {
   if (isEmpty()) { return; }
   const int64_t finalposition = position < 0 ? Count() + position : position;
   if (finalposition < 0 || finalposition >= (int64_t)Count()) { return; }
   _count--;
   for (int64_t i = finalposition; i < (int64_t)Count(); i++) {
      _pData[i] = _pData[i + 1];
   }
}

template<class T>
inline ctResults ctDynamicArray<T>::Remove(const T& val, const int64_t position) {
   int64_t idx = FindIndex(val, position, 1);
   if (idx >= 0 && idx < (int64_t)Count()) {
      RemoveAt(idx);
      return CT_SUCCESS;
   } else {
      return CT_FAILURE_DATA_DOES_NOT_EXIST;
   }
}

template<class T>
inline void ctDynamicArray<T>::RemoveFirst() {
   RemoveAt(0);
}

template<class T>
inline void ctDynamicArray<T>::RemoveLast() {
   RemoveAt(-1);
}

template<class T>
inline void ctDynamicArray<T>::RemoveAllOf(const T& val) {
   while (Remove(val) == CT_SUCCESS)
      ;
}

template<class T>
inline void ctDynamicArray<T>::Clear() {
   _count = 0;
}

template<class T>
inline void ctDynamicArray<T>::Memset(int val) {
   if (isEmpty()) { return; }
   memset(Data(), val, Capacity() * sizeof(T));
}

template<class T>
inline bool ctDynamicArray<T>::isEmpty() const {
   return _count == 0 || !_pData;
}

template<class T>
inline int64_t ctDynamicArray<T>::FindIndex(const T& val,
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

template<class T>
inline T* ctDynamicArray<T>::FindPtr(const T& val,
                                     const int64_t position,
                                     const int direction) const {
   if (isEmpty()) { return NULL; }
   const int64_t idx = FindIndex(val, position, direction);
   return idx >= 0 ? &(Data()[idx]) : NULL;
}

template<class T>
inline T& ctDynamicArray<T>::Fetch(const T& val) const {
   int64_t idx = FindIndex(val);
   ctAssert(idx >= 0);
   return Data()[idx];
}

template<class T>
inline void ctDynamicArray<T>::QSort(int (*compare)(const T*, const T*),
                                     const size_t position,
                                     const size_t amount) {
   if (isEmpty()) { return; }
   size_t adjustedAmount = amount == 0 ? Count() : amount;
   const size_t remainingCount = Count() - position;
   const size_t finalAmount =
     adjustedAmount > remainingCount ? remainingCount : adjustedAmount;
   qsort(Data(), finalAmount, sizeof(T), (int (*)(void const*, void const*))compare);
}

template<class T>
inline uint32_t ctDynamicArray<T>::xxHash32(const size_t position,
                                            const size_t amount,
                                            const int seed) const {
   if (isEmpty()) { return 0; }
   return XXH32((const void*)(Data() + position), amount, seed);
}

template<class T>
inline uint32_t ctDynamicArray<T>::xxHash32(const int seed) const {
   return xxHash32(0, Count(), seed);
}

template<class T>
inline uint32_t ctDynamicArray<T>::xxHash32() const {
   return xxHash32(0);
}

template<class T>
inline uint64_t ctDynamicArray<T>::xxHash64(const size_t position,
                                            const size_t amount,
                                            const int seed) const {
   if (isEmpty()) { return 0; }
   return XXH64((const void*)(Data() + position), amount, seed);
}

template<class T>
inline uint64_t ctDynamicArray<T>::xxHash64(const int seed) const {
   return xxHash64(0, Count(), seed);
}

template<class T>
inline uint64_t ctDynamicArray<T>::xxHash64() const {
   return xxHash64(0);
}

template<class T>
inline bool ctDynamicArray<T>::Exists(const T& val) const {
   return FindIndex(val, 0, 1) != -1 ? true : false;
}