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

#ifdef CT_TMP_USE_STD
#include <vector>
#endif

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
   /* Remove */
   void RemoveAt(const int64_t position = 0);
   ctResults Remove(const T& val, const int64_t position = 0);
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
   int64_t FindIndex(const T& val,
                     const int64_t position = 0,
                     const int step = 1) const;
   T* FindPtr(const T& val,
              const int64_t position = 0,
              const int step = 1) const;
   /* Sort */
   void QSort(const size_t position,
              const size_t amount,
              int (*compare)(const T*, const T*));
   /* Hash */
   uint32_t
   xxHash32(const size_t position, const size_t amount, const int seed) const;
   uint32_t xxHash32(const int seed) const;
   uint32_t xxHash32() const;
   uint64_t
   xxHash64(const size_t position, const size_t amount, const int seed) const;
   uint64_t xxHash64(const int seed) const;
   uint64_t xxHash64() const;

private:
#ifdef CT_TMP_USE_STD
   std::vector<T> _dirtysecret;
#else
   ctResults _expand_size(size_t amount);
   T* _pData;
   size_t _capacity;
   size_t _count;
#endif
};

#ifndef CT_TMP_USE_STD
template<class T>
inline ctResults ctDynamicArray<T>::_expand_size(size_t amount) {
   const size_t neededamount = Count() + amount;
   const size_t originalcapacity = Capacity();
   if (neededamount > originalcapacity) {
      size_t targetamount = Capacity();
      if (targetamount <= 0) { targetamount = 1; }
      while (targetamount < neededamount) {
         targetamount *= 2;
      }
      return Reserve(targetamount);
   }
   return CT_SUCCESS;
}
#endif

template<class T>
inline ctDynamicArray<T>::ctDynamicArray() {
#ifndef CT_TMP_USE_STD
   _pData = NULL;
   _capacity = 0;
   _count = 0;
#endif
}

template<class T>
inline ctDynamicArray<T>::ctDynamicArray(const ctDynamicArray<T>& arr) {
#ifndef CT_TMP_USE_STD
   const size_t inputcount = arr.Count();
   Resize(inputcount);
   for (size_t i = 0; i < inputcount; i++) {
      _pData[i] = arr[i];
   }
#endif
}

template<class T>
inline ctDynamicArray<T>::ctDynamicArray(ctDynamicArray<T>& arr) {
#ifndef CT_TMP_USE_STD
   const size_t inputcount = arr.Count();
   Resize(inputcount);
   for (size_t i = 0; i < inputcount; i++) {
      _pData[i] = arr[i];
   }
#endif
}

template<class T>
inline ctDynamicArray<T>::~ctDynamicArray() {
#ifndef CT_TMP_USE_STD
   delete[] _pData;
   _pData = NULL;
   _capacity = 0;
   _count = 0;
#endif
}

template<class T>
inline T& ctDynamicArray<T>::operator[](const size_t index) {
#ifdef CT_TMP_USE_STD
   return _dirtysecret[index];
#else
   ctAssert(index <= Count());
   return _pData[index];
#endif
}

template<class T>
inline T ctDynamicArray<T>::operator[](const size_t index) const {
#ifdef CT_TMP_USE_STD
   return _dirtysecret[index];
#else
   ctAssert(index <= Count());
   return _pData[index];
#endif
}

template<class T>
inline T& ctDynamicArray<T>::First() {
#ifdef CT_TMP_USE_STD
   return _dirtysecret[0];
#else
   ctAssert(Count() > 0);
   return _pData[0];
#endif
}

template<class T>
inline T ctDynamicArray<T>::First() const {
#ifdef CT_TMP_USE_STD
   return _dirtysecret[0];
#else
   ctAssert(Count() > 0);
   return _pData[0];
#endif
}

template<class T>
inline T& ctDynamicArray<T>::Last() {
   size_t idx = Count() - 1 > 0 ? Count() - 1 : 0;
#ifdef CT_TMP_USE_STD
   return _dirtysecret[idx];
#else
   ctAssert(Count() > 0);
   return _pData[idx];
#endif
}

template<class T>
inline T ctDynamicArray<T>::Last() const {
   size_t idx = Count() - 1 > 0 ? Count() - 1 : 0;
#ifdef CT_TMP_USE_STD
   return _dirtysecret[idx];
#else
   ctAssert(Count() > 0);
   return _pData[idx];
#endif
}

template<class T>
inline ctResults ctDynamicArray<T>::Resize(const size_t amount) {
#ifdef CT_TMP_USE_STD
   _dirtysecret.resize(amount);
#else
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
#endif
   return CT_SUCCESS;
}

template<class T>
inline ctResults ctDynamicArray<T>::Reserve(const size_t amount) {
#ifdef CT_TMP_USE_STD
   _dirtysecret.reserve(amount);
#else
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
#endif
   return CT_SUCCESS;
}

template<class T>
inline T* ctDynamicArray<T>::Data() const {
#ifdef CT_TMP_USE_STD
   return (T*)_dirtysecret.data();
#else
   return _pData;
#endif
}

template<class T>
inline size_t ctDynamicArray<T>::Count() const {
#ifdef CT_TMP_USE_STD
   return _dirtysecret.size();
#else
   return _count;
#endif
}

template<class T>
inline size_t ctDynamicArray<T>::Capacity() const {
#ifdef CT_TMP_USE_STD
   return _dirtysecret.capacity();
#else
   return _capacity;
#endif
}

template<class T>
inline ctDynamicArray<T>&
ctDynamicArray<T>::operator=(const ctDynamicArray<T>& arr) {
   const size_t inputcount = arr.Count();
   Resize(inputcount);
   for (size_t i = 0; i < inputcount; i++) {
      _pData[i] = arr[i];
   }
   return *this;
}

template<class T>
inline ctResults ctDynamicArray<T>::Append(const T& val) {
#ifdef CT_TMP_USE_STD
   _dirtysecret.push_back(val);
   return CT_SUCCESS;
#else
   const ctResults result = _expand_size(1);
   if (result != CT_SUCCESS) { return result; }
   if (_pData) { _pData[Count()] = val; }
   _count++;
   return result;
#endif
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
inline ctResults ctDynamicArray<T>::Append(const T* pArray,
                                           const size_t length) {
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
inline ctResults ctDynamicArray<T>::Insert(const T& val,
                                           const int64_t position) {
   int64_t finalposition = position < 0 ? Count() + position + 1 : position;
#ifdef CT_TMP_USE_STD
   _dirtysecret.insert(_dirtysecret.begin() + finalposition, val);
   return CT_SUCCESS;
#else
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
#endif
}

template<class T>
inline void ctDynamicArray<T>::RemoveAt(const int64_t position) {
   if (isEmpty()) { return; }
   const int64_t finalposition = position < 0 ? Count() + position : position;
#ifdef CT_TMP_USE_STD
   _dirtysecret.erase(_dirtysecret.begin() + finalposition);
#else
   if (finalposition < 0 || finalposition >= (int64_t)Count()) { return; }
   _count--;
   for (int64_t i = finalposition; i < (int64_t)Count(); i++) {
      _pData[i] = _pData[i + 1];
   }
#endif
}

template<class T>
inline ctResults ctDynamicArray<T>::Remove(const T& val,
                                           const int64_t position) {
   int64_t idx = FindIndex(val, position, 1);
   if (idx >= 0 && idx < (int64_t)Count()) {
      RemoveAt(idx);
      return CT_SUCCESS;
   } else {
      return CT_FAILURE_DATA_DOES_NOT_EXIST;
   }
}

template<class T>
inline void ctDynamicArray<T>::RemoveLast() {
#ifdef CT_TMP_USE_STD
   _dirtysecret.pop_back();
#else
   RemoveAt(-1);
#endif
}

template<class T>
inline void ctDynamicArray<T>::Clear() {
#ifdef CT_TMP_USE_STD
   _dirtysecret.clear();
#else
   _count = 0;
#endif
}

template<class T>
inline void ctDynamicArray<T>::SetBytes(int val) {
   if (isEmpty()) { return; }
   memset(Data(), val, Capacity() * sizeof(T));
}

template<class T>
inline bool ctDynamicArray<T>::isEmpty() const {
#ifdef CT_TMP_USE_STD
   return _dirtysecret.empty();
#else
   return _count == 0 || !_pData;
#endif
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
inline void ctDynamicArray<T>::QSort(size_t position,
                                     size_t amount,
                                     int (*compare)(const T*, const T*)) {
   if (isEmpty()) { return; }
   const size_t remaining_count = Count() - position;
   const size_t final_amount =
     amount > remaining_count ? remaining_count : amount;
   qsort(Data(),
         final_amount,
         sizeof(T),
         (int (*)(void const*, void const*))compare);
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