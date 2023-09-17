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

/* acts as a shared pointer with automatic refcounting (this is useful for holding references to long term data)
Use this member in conjunction with RAII patterns to hold and ensure that the pointed data stays alive */

/* just calls delete on a pointer, your subsystem may have other needs or its own garbage collection */
void(fpGarbageCollection)(T*, void*) ctGetHandlePtrDeleteDefaultFn();

template<class T>
class ctHandlePtr {
   ctHandlePtr();
   ctHandlePtr(const ctHandlePtr other);
   ~ctHandlePtr();
   ctHandlePtr MakeHandlePointer(T*, void(fpGarbageCollection)(T*, void*) = ctGetHandlePtrDeleteDefaultFn(), void* pUserData = NULL) static;
   /* Swaps the pointer value for this and all handles which have coppied this one
   MAKE SURE NO OTHER THREADS ARE ACTIVELY USING THE Get() RESULTS */
   void SwapPointer(T* value, bool garbageCollect = true);

   /* pointer access (do not store long term) 
   guarantees this handle will be alive and not NULL (assuming the original pointer was not NULL) */
   T* Get();
   int32_t Refcount();
protected:
   ctHandle handle;
};