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

#pragma once /* ignored by parser */
#include "utilities/Common.h" /* ignored by parser*/
#include "scene/kinnow/KinnowReflection.h"

/* 
any comment starting without a recognized tag will be ignored.
supported types by editor: xint##_t, bool, float, double, char, char[], enum, struct, ctVec#, ctQuat, ctGUID
unsupported types are ignored 
*/

/* CT_MODULE: "testcomponents" */

typedef enum myEnumTypeNumberedBits {
   MY_ENUM_A = 0, /* CT_LABEL: "My Enum A" */
   MY_ENUM_B = 1, /* CT_LABEL: "My Enum B" */
   MY_ENUM_C = 2, /* CT_LABEL: "My Enum C", CT_OLD_NAME: "MY_ENUM_D" */
} myEnumTypeNumberedBits;
/* CT_ENUM: "myEnumTypeFlagBits" */
typedef int32_t myEnumTypeNumbered;

typedef enum myEnumTypeFlagBits {
   MY_ENUM_FLAG_A = 0x0001, /* CT_LABEL: "My Flag A" */
   MY_ENUM_FLAG_B = 0x0002, /* CT_LABEL: "My Flag B" */
   MY_ENUM_FLAG_C = 0x0004, /* CT_LABEL: "My Flag C" */
} myEnumTypeFlagBits;
typedef int32_t myEnumTypeFlagged; /* CT_BITS: "myEnumTypeFlagBits" */

typedef struct myReusableStruct {
   CT_KINNOW_STRUCT(myReusableStruct)
   uint8_t testType; /* CT_DEFAULT: "32" */
   int32_t pointCount; /* CT_HIDDEN */
   ctVec3* pPoints; /* CT_EMBED_BLOB CT_LENGTH_VAR: "pointCount" */
   const char* myname;
   ctVec3** ppOverItsHead;
} myReusableStruct;

/* CT_COMPONENT_NAME: "testComponent" */
/* CT_LABEL: "My Component" */
/* CT_OLD_NAME: "testComponentOldA" */
/* CT_OLD_NAME: "testComponentOldB" */
/* CT_DOCUMENTATION: "This is a long winded multi-line comment
that spans multiple lines and should contain documentation
info for the comment below" */
typedef struct myComponent {
   CT_KINNOW_STRUCT(myComponent)
   uint32_t standardIntType; /* CT_DEFAULT: "3000" */
   bool booleanType; /* CT_DEFAULT: "true" CT_READ_ONLY */
   myEnumTypeNumbered enumType; /* CT_DEFAULT: "MY_ENUM_B" */
   myEnumTypeFlagged flagType; /* CT_DEFAULT: "MY_ENUM_FLAG_A | MY_ENUM_FLAG_B" */
   /* CT_UNITS: "SECONDS" */
   /* CT_DEFAULT: "32.5" */
   /* CT_MIN: "0" */
   /* CT_MAX: "1000000" */
   /* CT_SOFT_MAX: "1000" */
   float floatType;
   float arrayType[4]; /* CT_DEFAULT: "[0,3.2,43,1000]" */
   char fixedString[32]; /* CT_DEFAULT: "My String" */

   uint8_t varArrayLen; /* CT_HIDDEN */
   myReusableStruct varArray[256]; /* CT_LENGTH_VAR: "varArrayLen" */

   ctVec3 mathType; /* CT_UNITS: "METERS" */
   ctVec4 colorType; /* CT_UNITS: "COLOR" */

   /* CT_HIDDEN */
   bool privateData;
} myComponent;

/* CT_COMPONENT_NAME: "testComponentPartTwo" */
/* CT_HIDDEN */
typedef struct myComponentComplimentary {
   CT_KINNOW_STRUCT(myComponentComplimentary)
   int32_t otherParameter;
};

/* CT_COMPONENT_GROUPS: "{'testComponentCompound':['testComponent','testComponentPartTwo']}"