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

#define ctSerializeStruct(_TYPE, _OBJ,, _SERIALIZER_CLASS) _SERIALIZER_CLASS().Serialize(_##_TYPE##_citreflget(), &_OBJ)
#define ctSerializePolymorphic(_OBJ, _SERIALIZER_CLASS) _SERIALIZER_CLASS().Serialize((ctReflectorBase&)_OBJ, &_OBJ)
#define ctDeserializeStruct(_CLASS, _SERIALIZER_CLASS) _##_TYPE##_citreflget() _SERIALIZER_CLASS().Serialize(_##_TYPE##_citreflget(), &_OBJ)
#define ctDeserializePolymorphic(_CLASS, _SERIALIZER_CLASS)

class ctSerializerBase {
public:
   ctSerializerBase();
   virtual ctResults Serialize(ctReflectorBase& reflector, void* pSrc, ctDynamicArray<uint8_t>& dest);
   virtual ctResults Deserialize(ctReflectorBase& reflector, void* pDest, const ctDynamicArray<uint8_t>& src);
}