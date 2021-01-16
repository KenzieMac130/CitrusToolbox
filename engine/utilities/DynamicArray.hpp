#pragma once

#include "neCommon.hpp"

#include <vector>

namespace NE_NAMESPACE {

template<class T>
class dynamicArray {
public:
   /* Constructors */
   dynamicArray(); 
   dynamicArray(dynamicArray<T>& arr);
   /* Destructor */
   ~dynamicArray();
   /* Array Access */
   T& operator[](const size_t index) noexcept;
   /* Reserve */
   Results reserve(const size_t count);
   /* Data */
   T* data();
   /* Count */
   size_t count();
   size_t capacity();
   /* Append */
   Results append(T&& val);
   Results append(const T& val);
   Results append(dynamicArray<T>& arr);
   /* Insert */
   Results insert(T&& val, const int64_t position);
   /* Remove */
   Results remove(const int64_t position);
   /* Clear */
   void clear();
   /* Memset and Clear */
   void setbytes(int val);
   /* IsEmpty */
   bool isEmpty();
   /* Exists */
   bool exists(const T val);
   /* Find */
   int64_t findIndex(const T val, const int64_t position);
   int64_t findIndex(const T val, const int64_t position, const int step);
   T* findPtr(const T val, const int64_t position);
   T* findPtr(const T val, const int64_t position, const int step);
   /* Sort */
   void sort(const int64_t position,
             const int64_t amount,
             int (*compar)(const T*, const T*));
   /* Hash */
private:
   std::vector<T> _dirtysecret; //Todo: replace with custom
};

template<class T>
inline dynamicArray<T>::dynamicArray() {
}

template<class T>
inline dynamicArray<T>::dynamicArray(dynamicArray<T>& arr)
{
}

template<class T>
inline dynamicArray<T>::~dynamicArray() {
}

template<class T>
inline T& dynamicArray<T>::operator[](const size_t index) noexcept {
   return _dirtysecret[i];
}

template<class T>
inline Results dynamicArray<T>::reserve(const size_t count) {
   _dirtysecret.reserve(count);
   return Results::SUCCESS;
}

template<class T>
inline T* dynamicArray<T>::data() {
   return _dirtysecret.data();
}

template<class T>
inline size_t dynamicArray<T>::count() {
   return _dirtysecret.size();
}

template<class T>
inline size_t dynamicArray<T>::capacity() {
   return capacity();
}

template<class T>
inline Results dynamicArray<T>::append(T&& val) {
   _dirtysecret.push_back(val);
   return Results::SUCCESS;
}

template<class T>
inline Results dynamicArray<T>::append(const T& val) {
   _dirtysecret.push_back(val);
   return Results::SUCCESS;
}

template<class T>
inline Results dynamicArray<T>::append(dynamicArray<T>& arr) {
   reserve(count() + arr.count());
   for (int i = 0; i < arr.count(); i++) {
      append(arr[i]);
   }
   return Results::SUCCESS;
}

template<class T>
inline Results dynamicArray<T>::insert(T&& val, const int64_t position) {
   size_t finalposition = (size_t)(position % count());
   _dirtysecret.insert(_dirtysecret.begin() + finalposition);
   return Results::SUCCESS;
}

template<class T>
inline Results dynamicArray<T>::remove(const int64_t position) {
   const size_t finalposition = (size_t)(position % count());
   _dirtysecret.erase(_dirtysecret.begin() + finalposition);
   return Results::SUCCESS;
}

template<class T>
inline void dynamicArray<T>::clear() {
   _dirtysecret.clear();
}

template<class T>
inline void dynamicArray<T>::setbytes(int val) {
   clear();
   memset(_dirtysecret.data(), val, _dirtysecret.capacity() * sizeof(T));
}

template<class T>
inline bool dynamicArray<T>::isEmpty() {
   return _dirtysecret.empty();
}

template<class T>
inline int64_t dynamicArray<T>::findIndex(const T val, const int64_t position) {
   return findIndex(val, position, 1);
}

template<class T>
inline int64_t dynamicArray<T>::findIndex(const T val,
                                          const int64_t position,
                                          const int direction) {
   const size_t count = count() const size_t finalposition =
     (size_t)(position % count);
   for (int64_t i = 0; i < count; i += direction) {
      if (i >= count || i < 0) { return -1; }
      if (_dirtysecret[i] == val) { return i; }
   }
   return -1;
}

template<class T>
inline T* dynamicArray<T>::findPtr(const T val, const int64_t position) {
   findPtr(val, position, 1);
}

template<class T>
inline T*
dynamicArray<T>::findPtr(const T val, const int64_t position, const int step) {
   const int64_t idx = findIndex(val, position, step);
   return idx >= 0 ? _dirtysecret[idx] : NULL;
}

template<class T>
inline void dynamicArray<T>::sort(int64_t position,
                                  int64_t amount,
                                  int (*compar)(const T*, const T*)) {
   const size_t remaining_count = count() - position;
   const size_t final_amount =
     amount > remaining_count ? remaining_count : amount;
   qsort(_dirtysecret, final_amount, sizeof(T), compar);
}

template<class T>
inline bool dynamicArray<T>::exists(const T val) {
   return findIndex(val, 0, 1) != -1 ? true : false;
}

}