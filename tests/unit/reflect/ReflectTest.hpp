/*
   Copyright 2024 MacKenzie Strand

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
#include "codegen/tests/unit/reflect/ReflectTest.hpp"

/* CT_REFLECT */
enum MyEnum {
   MY_ENUM_APPLES = 3,   /* CT_REFLECT Name = "Apples" */
   MY_ENUM_ORANGES = 7,  /* CT_REFLECT name = "Oranges"; */
   MY_ENUM_LEMONS = 420, /* CT_REFLECT NAME = "Lemons" */
};

/* CT_REFLECT */
class CT_API AuxClassA : public ctReflectorBase {
public:
   CT_REFLECT_CLASS_DECLARE_VFUNC()
   CT_REFLECT_ACCESS_PROTECTED(AuxClassA);
   /* CT_REFLECT
   Min = 0.0;
   Max = 32.0; */
   /* My Useless comment */
   float a;
   ctDynamicArray<int32_t> dArrayTest;                /* CT_REFLECT */
   ctDynamicArray<MyEnum> dArrayTest2;                /* CT_REFLECT */
   ctStaticArray<int32_t, 32> sArrayTest;             /* CT_REFLECT */
   ctDynamicArray<ctStaticArray<int, 32> > trickster; /* CT_REFLECT */
   /* Useless comment */
   inline void SetDString(const char* str) {
      dynamicString = str;
   }

protected:
   ctStringUtf8 dynamicString; /* CT_REFLECT */
};

/* CT_REFLECT */
struct CT_API AuxStruct {
   char myFixedString[32]; /* CT_REFLECT */
   uint8_t bytesTest[64];  /* CT_REFLECT */
   float myArray[8];       /* CT_REFLECT */
   float data;             /* CT_REFLECT */
   MyEnum enumTest;        /* CT_REFLECT */
   const char* doot;       /* CT_REFLECT */
};

/* CT_REFLECT */
class AuxClassB : public AuxClassA {
public:
   CT_REFLECT_CLASS_DECLARE_VFUNC()
   float b;                            /* CT_REFLECT */
   ctHandlePtr<int32_t> smartPtr;      /* CT_REFLECT */
   MyEnum myEnumArray[4];              /* CT_REFLECT */
   ctDynamicArray<AuxStruct> objArray; /* CT_REFLECT */
};

/* CT_REFLECT */
struct MyBasicStruct {
   /* CT_REFLECT:
   Min = 0.0f
   Max = 5.0f */
   float weight;
   AuxClassB complexClass; /* CT_REFLECT */
   float stiffness;        /* CT_REFLECT */
   ctVec3 position;        /* CT_REFLECT */
   AuxStruct sidecar;      /* CT_REFLECT */
};