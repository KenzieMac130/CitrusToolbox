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
#include <inttypes.h>
#include <stdbool.h>

/* Generic Container Interface */
struct ctReflectContainerInterface {
   void* extra;
   const char* containerType;
   const char* (*GetKeyType)(void* baseAddress,
                             void* extra); /* only support basic types */

   size_t (*GetCount)(void* baseAddress, void* extra);
   size_t (*GetCapacity)(void* baseAddress, void* extra);

   void* (*GetKey)(size_t index, void* baseAddress, void* extra);
   void* (*GetValue)(size_t index, void* baseAddress, void* extra);
   void* (*GetValueWithKey)(void* keyPtr, void* baseAddress, void* extra);

   enum ctResults (*Swap)(int64_t indexA, int64_t indexB, void* baseAddress, void* extra);
   enum ctResults (*RemoveAt)(int64_t index, void* baseAddress, void* extra);
   enum ctResults (*InsertNew)(int64_t index, void* baseAddress, void* extra);
   enum ctResults (*AppendNew)(void* baseAddress, void* extra);
   enum ctResults (*InsertNewWithKey)(void* keyPtr, void* baseAddress, void* extra);
};