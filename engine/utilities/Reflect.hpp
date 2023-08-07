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

/* ------------------ USEAGE ------------------ */
/*
Example.hpp:

struct Foo {
   ctString mvar;
};

class Baz : public ctReflectorBase {
public:
   CT_DEFINE_REFLECTOR_VFUNC()
   int* mvar3; // pointers are supported!
};

struct Bar {
   int mvar;
   Foo mvar2;
   Baz myvar3;
};

CT_DEFINE_REFLECTOR(Foo)
CT_DEFINE_REFLECTOR(Bar)
CT_DEFINE_REFLECTOR(Baz)

Example.cpp:

CT_IMPLEMENT_REFLECTOR(Foo){
   CT_REFLECT_BASIC_TYPE(Foo, ctStringUtf8, myvar, "")
}

CT_IMPLEMENT_REFLECTOR(Bar){
   CT_REFLECT_BASIC_TYPE(Bar, int, myvar, "")
   CT_REFLECT_STRUCT_TYPE(Bar, Foo, myvar, "")
   CT_REFLECT_POLY_TYPE(Bar, Baz, myvar, "")
}

CT_IMPLEMENT_REFLECTOR(Baz){
   CT_REFLECT_BASIC_TYPE(Baz, int*, myvar, "")
}

CT_IMPLEMENT_REFLECTOR_VFUNC(Baz)
*/

/* the reflector context is responsible for implementing ui/saving/loading/etc */
class ctReflectContext {
public:
   virtual void OnBasicType(const char* typeName,
                            const char* name,
                            size_t offset,
                            const char* metadata) const = 0;
   virtual void OnStructType(const char* typeName,
                             const char* name,
                             size_t offset,
                             const char* metadata,
                             const class ctReflectorBase& reflector) const = 0;
   virtual void OnPolyType(const char* typeName,
                           const char* name,
                           size_t offset,
                           const char* metadata) const = 0;
};

/* sidecar or even base class object which caries reflection */
class ctReflectorBase {
public:
   virtual size_t GetStructSize() const = 0;
   virtual const char* GetStructName() const = 0;
   virtual void GetStructReflection(ctReflectContext& ctx) const = 0;
};

/* ------------------ DEFINITION ------------------ */

/* Put in include files to add reflectors to a type */
#define CT_DEFINE_REFLECTOR(_CLASS_TYPE)                                                 \
   class _##_CLASS_TYPE##_citrefl : public ctReflectorBase {                             \
   public:                                                                               \
      virtual void GetStructReflection(ctReflectContext& ctx) const override;            \
      virtual const char* GetName() const override;                                      \
      virtual size_t GetSize() const override;                                           \
   };                                                                                    \
   const ctReflectorBase& _##_CLASS_TYPE##_citreflget();

/* add this to classes which inherit from ctReflectorBase */
#define CT_DEFINE_REFLECTOR_VFUNC()                                                      \
   virtual void GetStructReflection(class ctReflectContext& ctx) override;               \
   virtual const char* GetName() const override;                                         \
   virtual size_t GetSize() const override;

/* ------------------ IMPLEMENTATION ------------------ */

/* Called to start a reflection implementation block */
#define CT_IMPLEMENT_REFLECTOR(_CLASS_TYPE)                                              \
   const ctReflectorBase& _##_CLASS_TYPE##_citreflget() {                                \
      static _##_CLASS_TYPE##_citrefl s;                                                 \
      return s;                                                                          \
   }                                                                                     \
   const char* _##_CLASS_TYPE##_citrefl::GetStructName() const {                         \
      return #_CLASS_TYPE;                                                               \
   }                                                                                     \
   size_t _##_CLASS_TYPE##_citrefl::GetStructSize() const {                              \
      return sizeof(_CLASS_TYPE)                                                         \
   }                                                                                     \
   void _##_CLASS_TYPE##_citrefl::GetStructReflection(ctReflectContext& ctx) const

/* to implement the reflector vfunctions on a poly type (inherits from ctReflectorBase) */
#define CT_IMPLEMENT_REFLECTOR_VFUNC(_CLASS_TYPE)                                        \
   void _CLASS_TYPE::GetStructReflection(ctReflectContext& ctx) {                        \
      _##_TYPE##_citreflget().GetStructReflection(ctx);                                  \
   }                                                                                     \
   const char* _CLASS_TYPE::GetStructName() const {                                      \
      return _##_TYPE##_citreflget().GetStructName();                                    \
   }                                                                                     \
   size_t _CLASS_TYPE::GetStructSize() const {                                           \
      return _##_TYPE##_citreflget().GetStructSize()                                     \
   }

/* Adds a basic type (whatever is handled natively by the context) */
#define CT_REFLECT_BASIC_TYPE(_CLASS_TYPE, _TYPE, _NAME, _METADATA)                      \
   ctx.OnBasicType(#_TYPE, #_NAME, offsetof(_CLASS_TYPE, _NAME), _METADATA)

/* for other classes/structs that have a sidecar ctReflectorBase defined */
#define CT_REFLECT_STRUCT_TYPE(_CLASS_TYPE, _TYPE, _NAME, _METADATA)                     \
   ctx.OnStructType(                                                                     \
     #_TYPE, #_NAME, offsetof(_CLASS_TYPE, _NAME), _METADATA, _##_TYPE##_citreflget())

/* for classes that derive from ctReflectorBase themself */
#define CT_REFLECT_POLY_TYPE(_CLASS_TYPE, _TYPE, _NAME, _METADATA)                       \
   ctx.OnPolyType(#_TYPE, #_NAME, offsetof(_CLASS_TYPE, _NAME), _METADATA)

/* todo: ctDynamicArray support */