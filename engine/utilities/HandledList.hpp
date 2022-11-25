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
class ctHandledList {
public:
   ctHandledList();
   ctHandledList(const ctHandledList<T>& list) = delete; /* needs to be stable */
   ctHandledList(ctHandledList<T>&& list) = delete;      /* needs to be stable */

   ctHandle Insert(T&& val);
   ctHandle Insert(const T& val);
   T& operator[](const ctHandle handle);
   T operator[](const ctHandle handle) const;
   void Remove(ctHandle hndl);

private:
   struct HandleListBucket {
      T _pData[CT_MAX_HANDLED_BUCKET_SIZE];
   };
   ctHandleManager _handleManager;
   ctDynamicArray<HandleListBucket*> _pBuckets;
};

template<class T>
inline ctHandledList<T>::ctHandledList() {
   _handleManager = ctHandleManager();
   _pBuckets = ctDynamicArray<HandleListBucket*>();
}

template<class T>
inline ctHandle ctHandledList<T>::Insert(T&& val) {
   const ctHandle handle = _handleManager.GetNewHandle();
   const uint32_t index = ctHandleGetIndex(handle);
   const uint32_t targetBucket = index / CT_MAX_HANDLED_BUCKET_SIZE;
   const uint32_t targetElement = index % CT_MAX_HANDLED_BUCKET_SIZE;
   while (targetBucket >= _pBuckets.Count()) {
      _pBuckets.Append(new ctHandledList<T>::HandleListBucket());
   }
   _pBuckets[targetBucket]->_pData[targetElement] = val;
   return handle;
}

template<class T>
inline ctHandle ctHandledList<T>::Insert(const T& val) {
   const ctHandle handle = _handleManager.GetNewHandle();
   const uint32_t index = ctHandleGetIndex(handle);
   const uint32_t targetBucket = index / CT_MAX_HANDLED_BUCKET_SIZE;
   const uint32_t targetElement = index % CT_MAX_HANDLED_BUCKET_SIZE;
   while (targetBucket >= _pBuckets.Count()) {
      _pBuckets.Append(new ctHandledList<T>::HandleListBucket());
   }
   _pBuckets[targetBucket]->_pData[targetElement] = val;
   return handle;
}

template<class T>
inline T& ctHandledList<T>::operator[](const ctHandle handle) {
   const uint32_t index = ctHandleGetIndex(handle);
   const uint32_t targetBucket = index / CT_MAX_HANDLED_BUCKET_SIZE;
   const uint32_t targetElement = index % CT_MAX_HANDLED_BUCKET_SIZE;
   return _pBuckets[targetBucket]->_pData[targetElement];
}

template<class T>
inline T ctHandledList<T>::operator[](const ctHandle handle) const {
   const uint32_t index = ctHandleGetIndex(handle);
   const uint32_t targetBucket = index / CT_MAX_HANDLED_BUCKET_SIZE;
   const uint32_t targetElement = index % CT_MAX_HANDLED_BUCKET_SIZE;
   return _pBuckets[targetBucket]->_pData[targetElement];
}

template<class T>
inline void ctHandledList<T>::Remove(ctHandle hndl) {
   _handleManager.FreeHandle(hndl);
}
