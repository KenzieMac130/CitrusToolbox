/*
   Copyright 2023 MacKenzie Strand

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

#include "utilities/Common.h"

/* --------------- Internal --------------- */
/* must either be initted from instance or copied from other memory*/
CT_API void _ctHandlePtrGlobalInit(size_t maxPointers);
CT_API void _ctHandlePtrGlobalShutdown();
CT_API void* _ctHandlePtrGetCtx();
CT_API void _ctHandlePtrSetCtx(void* ctx);
typedef void (*_ctHandleOpaquePtrGarbageCollectFn)(void*, void*);
class CT_API _ctHandleOpaquePtr {
public:
   _ctHandleOpaquePtr() = default;
   _ctHandleOpaquePtr(void* value, _ctHandleOpaquePtrGarbageCollectFn, void* pUserData);
   inline _ctHandleOpaquePtr(const _ctHandleOpaquePtr& other) {
      handle = other.handle;
   }
   void* Get() const;
   void SwapPointer(void* value, bool garbageCollect);
   void Reference();
   void Dereference();
   int32_t Refcount() const;
   ctHandle handle;
};
/* --------------------------------------- */

/* acts as a shared pointer with automatic refcounting (this is useful for holding
references to long term data) Use this member in conjunction with RAII patterns to hold
and ensure that the pointed data stays alive */
template<class T>
class ctHandlePtr {
protected:
   _ctHandleOpaquePtr opaque;

public:
   inline ctHandlePtr() {
      opaque = _ctHandleOpaquePtr();
   }
   inline ctHandlePtr(const ctHandlePtr<T>& other) {
      opaque = _ctHandleOpaquePtr(other.opaque);
      opaque.Reference();
   }
   inline ctHandlePtr(ctHandle hndl) {
      opaque.handle = hndl;
   }
   inline ~ctHandlePtr() {
      if (!ctHandleIsValid(opaque.handle)) { return; }
      opaque.Dereference();
   }
   /* this will immediately take ownership over a pointer and the original pointer should
    * be dropped from any further use. By default a lambda which calls the delete function
    * is used for garbage collection, you can call the constructor explicitly to assign a
    * manual garbage collection function */
   inline ctHandlePtr(T* value,
                      void (*fpGarbageCollection)(T*, void*) = {[](T* ptr, void* unused) {
                         delete ptr;
                      }},
                      void* pUserData = NULL) {
      opaque = _ctHandleOpaquePtr(
        value, (_ctHandleOpaquePtrGarbageCollectFn)fpGarbageCollection, pUserData);
   }
   /* Swaps the pointer value for this and all handles which have coppied this one
   MAKE SURE NO OTHER THREADS ARE ACTIVELY USING THE Get() RESULTS */
   inline void SwapPointer(T* value, bool garbageCollect = true) {
      opaque.SwapPointer((void*)value, garbageCollect);
   }

   /* pointer access (do not store long term)
   guarantees this handle will be alive and not NULL (assuming the original pointer was
   not NULL) */
   inline T* GetPtr() const {
      return (T*)opaque.Get();
   }
   inline bool isValid() const {
      if (!ctHandleIsValid(opaque.handle)) { return false; }
      return opaque.Get();
   }
   inline T& Get() const {
      return *(T*)opaque.Get();
   }
   inline int32_t Refcount() const {
      return opaque.Refcount();
   }
   /* holding the baked handle does not hold a reference */
   inline ctHandle GetHandle() const {
      return opaque.handle;
   }
};

template<class T, class U>
ctHandlePtr<T> ctHandlePtrCast(const ctHandlePtr<U>& original) {
   return ctHandlePtr<T>(original.GetHandle());
}