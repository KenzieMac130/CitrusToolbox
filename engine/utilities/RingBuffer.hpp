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

template<class T>
class ctRingBuffer {
public:
   /* Constructors */
   ctRingBuffer() = default;
   ctRingBuffer(const ctRingBuffer<T>& arr);

   T& First();
   T First() const;
   T& Last();
   T Last() const;
   /* Reserve */
   ctResults Reserve(const size_t amount);
   /* Count */
   size_t Count() const;
   size_t Capacity() const;
   /* Append */
   ctResults Append(T&& val);
   ctResults Append(const T& val);
   void RemoveFirst();
   /* Clear */
   void Clear();
   /* isEmpty */
   bool isEmpty() const;

private:
   ctDynamicArray<T> storage;
   size_t firstIndex = 0;
};

template<class T>
inline ctRingBuffer<T>::ctRingBuffer(const ctRingBuffer<T>& arr) {
   storage = arr.storage;
   firstIndex = arr.firstIndex;
}

template<class T>
inline T& ctRingBuffer<T>::First() {
   return storage.First();
}

template<class T>
inline T ctRingBuffer<T>::First() const {
   return storage.First();
}

template<class T>
inline T& ctRingBuffer<T>::Last() {
   return storage.Last();
}

template<class T>
inline T ctRingBuffer<T>::Last() const {
   return storage.Last();
}

template<class T>
inline ctResults ctRingBuffer<T>::Reserve(const size_t amount) {
   return storage.Reserve(amount);
}

template<class T>
inline size_t ctRingBuffer<T>::Count() const {
   return storage.Count();
}

template<class T>
inline size_t ctRingBuffer<T>::Capacity() const {
   return storage.Capacity();
}

template<class T>
inline ctResults ctRingBuffer<T>::Append(T&& val) {
   return storage.Append(val);
}

template<class T>
inline ctResults ctRingBuffer<T>::Append(const T& val) {
   return storage.Append(val);
}

template<class T>
inline void ctRingBuffer<T>::RemoveFirst() {
   storage.RemoveFirst();
}

template<class T>
inline void ctRingBuffer<T>::Clear() {
   return storage.Clear();
}

template<class T>
inline bool ctRingBuffer<T>::isEmpty() const {
   return storage.isEmpty();
}
