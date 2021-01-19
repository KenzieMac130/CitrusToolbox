#pragma once

#include "Common.hpp"

#define CT_TMP_USE_STD

#ifdef CT_TMP_USE_STD
#include <vector>
#endif

namespace CT_UTILITIES_NAMESPACE {

template<class T>
class DynamicArray {
public:
   /* Constructors */
   DynamicArray();
   DynamicArray(DynamicArray<T>& arr);
   /* Destructor */
   ~DynamicArray();
   /* Array Access */
   T& operator[](const size_t index);
   /* Reserve */
   Results reserve(const size_t amount);
   /* Data */
   T* data();
   /* Count */
   size_t count();
   size_t capacity();
   /* Append */
   Results append(T&& val);
   Results append(const T& val);
   Results append(DynamicArray<T>& arr);
   Results append(T* pArray, const size_t length);
   /* Insert */
   Results insert(const T& val, const int64_t position);
   /* Remove */
   Results remove(const int64_t position);
   /* Clear */
   void clear();
   /* Memset and Clear */
   void setbytes(int val);
   /* IsEmpty */
   bool isEmpty();
   /* Exists */
   bool exists(const T& val);
   /* Find */
   int64_t findindex(const T& val, const int64_t position);
   int64_t findindex(const T& val, const int64_t position, const int step);
   T* findptr(const T& val, const int64_t position);
   T* findptr(const T& val, const int64_t position, const int step);
   /* Sort */
   void sort(const size_t position,
             const size_t amount,
             int (*compare)(const T*, const T*));
   /* Hash */
   uint32_t
   xxHash32(const size_t position, const size_t amount, const int seed);
   uint32_t xxHash32(const int seed);
   uint32_t xxHash32();
   uint64_t
   xxHash64(const size_t position, const size_t amount, const int seed);
   uint64_t xxHash64(const int seed);
   uint64_t xxHash64();

private:
#ifdef CT_TMP_USE_STD
   std::vector<T> _dirtysecret;
#else
   Results _expand_size(size_t amount);
   T* _pData;
   size_t _capacity;
   size_t _count;
 #endif
};

#ifndef CT_TMP_USE_STD
template<class T>
inline Results DynamicArray<T>::_expand_size(size_t amount) {
   const size_t neededamount = count() + amount;
   const size_t originalcapacity = capacity();
   if (neededamount > originalcapacity) {
      size_t targetamount = capacity();
      while (targetamount < neededamount) {
         targetamount *= 2;
      }
      return reserve(targetamount);
   }
   return Results::SUCCESS;
}
#endif

template<class T>
inline DynamicArray<T>::DynamicArray() {
#ifndef CT_TMP_USE_STD
 // Todo
   _pData = NULL;
   _capacity = 0;
   _count = 0;
#endif
}

template<class T>
inline DynamicArray<T>::DynamicArray(DynamicArray<T>& arr) {
#ifndef CT_TMP_USE_STD
// Todo
#endif
}

template<class T>
inline DynamicArray<T>::~DynamicArray() {
#ifndef CT_TMP_USE_STD
   CT_Free(_pData);
   _pData = NULL;
   _capacity = 0;
   _count = 0;
#endif
}

template<class T>
inline T& DynamicArray<T>::operator[](const size_t index) {
#ifdef CT_TMP_USE_STD
   return _dirtysecret[index];
#else
   return _pData[index];
#endif
}

template<class T>
inline Results DynamicArray<T>::reserve(const size_t amount) {
#ifdef CT_TMP_USE_STD
   _dirtysecret.reserve(amount);
#else
   if (amount > capacity()) {
      T* newptr = (T*)CT_Realloc(_pData, sizeof(T) * amount, "DynamicArray");
      CT_ASSERT(newptr);
      if (newptr) {
         _pData = newptr;
         _capacity = amount;
      } else {
         return Results::FAILURE_OUT_OF_MEMORY;
      }
   }
#endif
   return Results::SUCCESS;
}

template<class T>
inline T* DynamicArray<T>::data() {
#ifdef CT_TMP_USE_STD
   return _dirtysecret.data();
#else
   return _pData;
#endif
}

template<class T>
inline size_t DynamicArray<T>::count() {
#ifdef CT_TMP_USE_STD
   return _dirtysecret.size();
#else
   return _count;
#endif
}

template<class T>
inline size_t DynamicArray<T>::capacity() {
#ifdef CT_TMP_USE_STD
   return _dirtysecret.capacity();
#else
   return _capacity;
#endif
}

template<class T>
inline Results DynamicArray<T>::append(T&& val) {
#ifdef CT_TMP_USE_STD
   _dirtysecret.push_back(val);
   return Results::SUCCESS;
#else
   const Results result = _expand_size(1);
   if (result != Results::SUCCESS) { return result; }
   /* Note (move operators are ignored, data is just copied) */
   if (_pData) { memmove((void*)&_pData[count()], (void*)&val, sizeof(T)); }
   _count++;
   return result;
#endif
}

template<class T>
inline Results DynamicArray<T>::append(const T& val) {
   return append((const T&)val);
}

template<class T>
inline Results DynamicArray<T>::append(DynamicArray<T>& arr) {
   return append(arr.data(), arr.count());
}

template<class T>
inline Results DynamicArray<T>::append(T* pArray, const size_t length) {
   const Results result = reserve(count() + length);
   if (result != Results::SUCCESS) { return result; }
   for (int i = 0; i < length; i++) {
      append(pArray[i]);
   }
   return result;
}

template<class T>
inline Results DynamicArray<T>::insert(const T& val, const int64_t position) {
   const int64_t finalposition =
     position < 0 ? count() + position + 1 : position;
#ifdef CT_TMP_USE_STD
   _dirtysecret.insert(_dirtysecret.begin() + finalposition, val);
   return Results::SUCCESS;
#else
   const Results result = _expand_size(1);
   if (result != Results::SUCCESS) { return result; }
   // Todo move higher up (if availible)
   // Todo copy value
   return result;
#endif
}

template<class T>
inline Results DynamicArray<T>::remove(const int64_t position) {
   if (isEmpty()) { return Results::FAILURE_DATA_DOES_NOT_EXIST; }
   const int64_t finalposition = position < 0 ? count() + position : position;
#ifdef CT_TMP_USE_STD
   _dirtysecret.erase(_dirtysecret.begin() + finalposition);
#else
   // Todo destructor
   // Todo move higher down (if availible)
#endif
   return Results::SUCCESS;
}

template<class T>
inline void DynamicArray<T>::clear() {
#ifdef CT_TMP_USE_STD
   _dirtysecret.clear();
#else
   for (int i = 0; i < count(); i++) {
      _pData[i].~T();
   }
   _count = 0;
#endif
}

template<class T>
inline void DynamicArray<T>::setbytes(int val) {
   if (isEmpty()) { return; }
   clear();
   memset(data(), val, capacity() * sizeof(T));
}

template<class T>
inline bool DynamicArray<T>::isEmpty() {
#ifdef CT_TMP_USE_STD
   return _dirtysecret.empty();
#else
   return _count == 0;
#endif
}

template<class T>
inline int64_t DynamicArray<T>::findindex(const T& val,
                                          const int64_t position) {
   return findindex(val, position, 1);
}

template<class T>
inline int64_t DynamicArray<T>::findindex(const T& val,
                                          const int64_t position,
                                          const int direction) {
   if (isEmpty()) { return -1; }
   const int64_t amount = (int64_t)count();
   const int64_t finalposition = position < 0 ? count() + position : position;
   const int finaldirecton = direction >= 0 ? 1 : -1;
   for (int64_t i = finalposition; i < amount; i += finaldirecton) {
      if (i >= amount || i < 0) { return -1; }
      if (data()[i] == val) { return i; }
   }
   return -1;
}

template<class T>
inline T* DynamicArray<T>::findptr(const T& val, const int64_t position) {
   return findptr(val, position, 1);
}

template<class T>
inline T* DynamicArray<T>::findptr(const T& val,
                                   const int64_t position,
                                   const int direction) {
   if (isEmpty()) { return NULL; }
   const int64_t idx = findindex(val, position, direction);
   return idx >= 0 ? &(data()[idx]) : NULL;
}

template<class T>
inline void DynamicArray<T>::sort(size_t position,
                                  size_t amount,
                                  int (*compare)(const T*, const T*)) {
   if (isEmpty()) { return; }
   const size_t remaining_count = count() - position;
   const size_t final_amount =
     amount > remaining_count ? remaining_count : amount;
   qsort(data(),
         final_amount,
         sizeof(T),
         (int (*)(void const*, void const*))compare);
}

template<class T>
inline uint32_t DynamicArray<T>::xxHash32(const size_t position,
                                          const size_t amount,
                                          const int seed) {
   if (isEmpty()) { return 0; }
   return XXH32((const void*)(data() + position), amount, seed);
}

template<class T>
inline uint32_t DynamicArray<T>::xxHash32(const int seed) {
   return xxHash32(0, count(), seed);
}

template<class T>
inline uint32_t DynamicArray<T>::xxHash32() {
   return xxHash32(0);
}

template<class T>
inline uint64_t DynamicArray<T>::xxHash64(const size_t position,
                                          const size_t amount,
                                          const int seed) {
   if (isEmpty()) { return 0; }
   return XXH64((const void*)(data() + position), amount, seed);
}

template<class T>
inline uint64_t DynamicArray<T>::xxHash64(const int seed) {
   return xxHash64(0, count(), seed);
}

template<class T>
inline uint64_t DynamicArray<T>::xxHash64() {
   return xxHash64(0);
}

template<class T>
inline bool DynamicArray<T>::exists(const T& val) {
   return findindex(val, 0, 1) != -1 ? true : false;
}

}