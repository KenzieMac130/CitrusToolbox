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

#include "utilities/Common.h"

#define TEST_NO_MAIN
#include "acutest/acutest.h"

#include "ReflectTest.hpp"

class DebugReflectContext : public ctReflectContext {
   virtual void OnBasicType(const char* typeName,
                            const char* name,
                            const char* metadata,
                            void* data) override {
      if (arrayIndex <= -1) { message.Printf(0, "%s %s = ", typeName, name); }

      if (data == NULL) {
         message += "NULL";
      } else if (ctCStrEql(typeName, "int32_t")) {
         message.Printf(0, "%d", *(int32_t*)data);
      } else if (ctCStrEql(typeName, "float")) {
         message.Printf(0, "%f", *(float*)data);
      } else if (ctCStrEql(typeName, "ctVec3")) {
         ctVec3 value = ctVec3((float*)data);
         message.Printf(0, "{%f, %f, %f}", value.x, value.y, value.z);
      }
      if (arrayIndex <= -1) {
         message += ";";
         FinishMessage();
      }
   }
   virtual void OnEnumType(const char* typeName,
                           const char* name,
                           const char* metadata,
                           const ctReflectEnumInfo& enumInfo,
                           uint64_t& data) {
      if (arrayIndex <= -1) { message.Printf(0, "%s %s = ", typeName, name); }

      const char* enumVal = "UNKNOWN";
      if (data == NULL) {
         message += "NULL";
      } else {
         for (uint64_t i = 0; true; i++) {
            if (!enumInfo.pEntries[i].programmerName) { break; }
            if (enumInfo.pEntries[i].value == data) {
               enumVal = enumInfo.pEntries[i].programmerName;
            }
         }
         message += enumVal;
      }

      if (arrayIndex <= -1) {
         message += ";";
         FinishMessage();
      }
   }
   virtual void
   OnByteBuffer(const char* name, const char* metadata, size_t size, uint8_t* data) {
      message.Printf(0, "uint8_t %s[] = \"", name);
      ctDynamicArray<char> buff;
      buff.Resize(size * 2);
      ctBytesToHex(size, data, buff.Data());
      message.Append(buff.Data(), buff.Count());
      message += "\";";
      FinishMessage();
   }
   virtual void
   OnCStringBuffer(const char* name, const char* metadata, size_t size, char* data) {
      OnDynamicString(name, metadata, ctStringUtf8(data, size));
   }
   virtual void
   OnDynamicString(const char* name, const char* metadata, ctStringUtf8& string) {
      message.Printf(0, "string %s = \"%s\";", name, string.CStr());
      FinishMessage();
   }

   virtual void
   OnStructBegin(const char* typeName, const char* name, const char* metadata) override {
      if (arrayIndex >= 0) {
         message += "{";
         FinishMessage();
         indentLevel++;
         return;
      }
      message.Printf(0, "%s %s = {", typeName, name);
      indentLevel++;
      FinishMessage();
   }
   virtual void
   OnStructEnd(const char* typeName, const char* name, const char* metadata) override {
      if (arrayIndex >= 0) {
         message += "}, ";
         FinishMessage();
         indentLevel--;
         return;
      }
      message = "};";
      indentLevel--;
      FinishMessage();
   }
   virtual void OnContainerBegin(const char* typeName,
                                 const char* name,
                                 const char* metadata,
                                 ctReflectContainerInterface& pCallbacks) override {
      message.Printf(0, "%s %s = [", typeName, name);
      arrayIndex = 0;
   }
   virtual void OnContainerEntryBegin(size_t index,
                                      ctReflectContainerInterface& containerInterface) {
      if (index > 0) { message.Printf(0, ", "); }
   }
   virtual void OnContainerEntryEnd(size_t index,
                                    ctReflectContainerInterface& containerInterface) {
   }
   virtual void OnContainerEnd(const char* typeName,
                               const char* name,
                               const char* metadata,
                               ctReflectContainerInterface& pCallbacks) override {
      message += "];";
      FinishMessage();
      arrayIndex = -1;
   }

private:
   void DoIndentation() {
      for (int i = 0; i < indentLevel; i++) {
         message += "   ";
      }
   }
   void FinishMessage() {
      ctDebugLog(message.CStr());
      message = "";
      DoIndentation();
   }
   ctStringUtf8 message = "";
   int indentLevel = 0;
   int arrayIndex = -1;
};

/* -------------- Make future codegen -------------- */
CT_REFLECT_ENUM_DECLARE(MyEnum);
CT_REFLECT_CLASS_DECLARE(AuxClassA)
CT_REFLECT_CLASS_DECLARE(AuxClassB)
CT_REFLECT_CLASS_DECLARE(AuxStruct)
CT_REFLECT_CLASS_DECLARE(MyBasicStruct)
/* END OF HEADER */

CT_REFLECT_ENUM_DEFINE_START(MyEnum, CT_REFLECT_ENUM_NUMBERED)
CT_REFLECT_ENUM_DEFINE_ENTRY(MyEnum, MY_ENUM_APPLES, "Apples")
CT_REFLECT_ENUM_DEFINE_ENTRY(MyEnum, MY_ENUM_ORANGES, "Oranges")
CT_REFLECT_ENUM_DEFINE_END(MyEnum)

CT_IMPLEMENT_REFLECTOR(AuxClassA) {
   CT_REFLECT_BASIC_TYPE(AuxClassA, float, a, "");
   CT_REFLECT_CONTAINER_BASIC_TYPE(AuxClassA, ctDynamicArray, int32_t, dArrayTest, "");
   CT_REFLECT_CONTAINER_ENUM_TYPE(AuxClassA, ctDynamicArray, MyEnum, dArrayTest2, "");
   CT_REFLECT_CONTAINER2_BASIC_TYPE(
     AuxClassA, ctStaticArray, int32_t, 32, sArrayTest, "");
   CT_REFLECT_BASIC_TYPE(AuxClassA, ctStringUtf8, dynamicString, "");
}

CT_IMPLEMENT_REFLECTOR(AuxClassB) {
   CT_REFLECT_BASIC_TYPE(AuxClassB, float, b, "");
   CT_REFLECT_CONTAINER_BASIC_TYPE(AuxClassB, ctHandlePtr, int32_t, smartPtr, "");
   CT_REFLECT_CARRAY_ENUM_TYPE(AuxClassB, MyEnum, myEnumArray, "");
   CT_REFLECT_CONTAINER_STRUCT_TYPE(AuxClassB, ctDynamicArray, AuxStruct, objArray, "");
}

CT_IMPLEMENT_REFLECTOR(AuxStruct) {
   CT_REFLECT_CARRAY_BASIC_TYPE(AuxStruct, char, myFixedString, "");
   CT_REFLECT_CARRAY_BASIC_TYPE(AuxStruct, uint8_t, bytesTest, "");
   CT_REFLECT_CARRAY_BASIC_TYPE(AuxStruct, float, myArray, "");
   CT_REFLECT_BASIC_TYPE(AuxStruct, float, data, "");
   CT_REFLECT_ENUM_TYPE(AuxStruct, MyEnum, enumTest, "");
}

CT_IMPLEMENT_REFLECTOR_VFUNC_BASE(AuxClassA);
CT_IMPLEMENT_REFLECTOR_VFUNC(AuxClassA, AuxClassB);

CT_IMPLEMENT_REFLECTOR(MyBasicStruct) {
   CT_REFLECT_BASIC_TYPE(MyBasicStruct, float, weight, "max = 3.0");
   CT_REFLECT_BASIC_TYPE(MyBasicStruct, float, stiffness, "");
   CT_REFLECT_BASIC_TYPE(MyBasicStruct, ctVec3, position, "");
   CT_REFLECT_STRUCT_TYPE(MyBasicStruct, AuxStruct, sidecar, "");
   CT_REFLECT_POLY_TYPE(MyBasicStruct, AuxClassB, complexClass, "");
}
/* ------------------------------------------------- */

void handle_exit(void) {
   _ctHandlePtrGlobalShutdown();
}

void basic_reflect_test() {
   _ctHandlePtrGlobalInit(1000);
   atexit(handle_exit); /* must happen after all destuctors */
   ctDebugLog("");
   MyBasicStruct object = MyBasicStruct();
   object.weight = 32.0f;
   object.stiffness = 0.5f;
   object.position.z = -32.0f;
   object.position.x = 69.0f;
   object.sidecar.data = 64.0f;
   object.sidecar.enumTest = MY_ENUM_ORANGES;
   object.complexClass.a = 0.5f;
   object.complexClass.b = 23.0f;
   object.complexClass.myEnumArray[1] = MY_ENUM_APPLES;
   object.complexClass.sArrayTest.Append(69);
   object.complexClass.objArray.Append(AuxStruct());
   for (size_t i = 0; i < 64; i++) {
      object.complexClass.dArrayTest.Append((int32_t)i);
   }
   object.complexClass.dArrayTest2.Append(MY_ENUM_APPLES);
   strncpy(object.sidecar.myFixedString, "Hello World!", 32);
   object.complexClass.SetDString("Its alive!!!");
   object.complexClass.smartPtr = new int32_t(32);
   for (size_t i = 0; i < ctCStaticArrayLen(object.sidecar.bytesTest); i++) {
      object.sidecar.bytesTest[i] = (uint8_t)ctRand() % CT_MAX_RAND;
   }
   DebugReflectContext ctx = DebugReflectContext();
   ctReflectionExecute(MyBasicStruct, ctx, &object);
}