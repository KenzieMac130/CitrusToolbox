#pragma once

#include "Common.hpp"

//#define TMP_USE_STD

#ifdef TMP_USE_STD
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
private:
#ifdef TMP_USE_STD
   std::vector<T> _dirtysecret;
#endif
   Results _expand_size(size_t amount);
   T* _pData;
   size_t _capacity;
   size_t _count;
};

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

template<class T>
inline DynamicArray<T>::DynamicArray() {
#ifndef TMP_USE_STD
   // Todo
   _pData = NULL;
   _capacity = 0;
   _count = 0;
#endif
}

template<class T>
inline DynamicArray<T>::DynamicArray(DynamicArray<T>& arr) {
#ifndef TMP_USE_STD
// Todo
#endif
}

template<class T>
inline DynamicArray<T>::~DynamicArray() {
#ifndef TMP_USE_STD
   CT_Free(_pData);
   _pData = NULL;
   _capacity = 0;
   _count = 0;
#endif
}

template<class T>
inline T& DynamicArray<T>::operator[](const size_t index) {
#ifdef TMP_USE_STD
   return _dirtysecret[index];
#else
   return _pData[index];
#endif
}

template<class T>
inline Results DynamicArray<T>::reserve(const size_t amount) {
#ifdef TMP_USE_STD
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
#ifdef TMP_USE_STD
   return _dirtysecret.data();
#else
   return _pData;
#endif
}

template<class T>
inline size_t DynamicArray<T>::count() {
#ifdef TMP_USE_STD
   return _dirtysecret.size();
#else
   return _count;
#endif
}

template<class T>
inline size_t DynamicArray<T>::capacity() {
#ifdef TMP_USE_STD
   return _dirtysecret.capacity();
#else
   return _capacity;
#endif
}

template<class T>
inline Results DynamicArray<T>::append(T&& val) {
#ifdef TMP_USE_STD
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
#ifdef TMP_USE_STD
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
   const int64_t finalposition = position < 0 ? count() + position : position;
#ifdef TMP_USE_STD
   _dirtysecret.erase(_dirtysecret.begin() + finalposition);
#else
   // Todo destructor
   // Todo move higher down (if availible)
#endif
   return Results::SUCCESS;
}

template<class T>
inline void DynamicArray<T>::clear() {
#ifdef TMP_USE_STD
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
   clear();
   memset(data(), val, capacity() * sizeof(T));
}

template<class T>
inline bool DynamicArray<T>::isEmpty() {
#ifdef TMP_USE_STD
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
   const int64_t amount = (int64_t)count();
   const int64_t finalposition = position < 0 ? count() + position : position;
   const int finaldirecton = direction >= 0 ? 1 : -1;
   for (int64_t i = finalposition; i < amount; i += finaldirecton) {
      if (i >= amount || i < 0) { return -1; }
#ifdef TMP_USE_STD
      if (_dirtysecret[i] == val) { return i; }
#else
      if (_pData[i] == val) { return i; }
#endif
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
   const int64_t idx = findindex(val, position, direction);
#ifdef TMP_USE_STD
   return idx >= 0 ? &_dirtysecret[idx] : NULL;
#else
   return idx >= 0 ? &_pData[idx] : NULL;
#endif
}

template<class T>
inline void DynamicArray<T>::sort(size_t position,
                                  size_t amount,
                                  int (*compare)(const T*, const T*)) {
   const size_t remaining_count = count() - position;
   const size_t final_amount =
     amount > remaining_count ? remaining_count : amount;
   qsort(data(),
         final_amount,
         sizeof(T),
         (int (*)(void const*, void const*))compare);
}

template<class T>
inline bool DynamicArray<T>::exists(const T& val) {
   return findindex(val, 0, 1) != -1 ? true : false;
}

}