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

#define IS_CITRUS_CODEGEN
#include "Common.h"
#undef IS_CITRUS_CODEGEN

struct ctReflectEnumEntryInfo {
   const char* programmerName;
   const char* friendlyName;
   uint64_t value;
};

enum ctReflectEnumType { CT_REFLECT_ENUM_NUMBERED, CT_REFLECT_ENUM_BITMASK };

struct ctReflectEnumInfo {
   ctReflectEnumType type; /* SEQUENCE or BITMASK */
   const ctReflectEnumEntryInfo* pEntries;
};

/* the reflector context is responsible for implementing ui/saving/loading/etc */
class ctReflectContext {
public:
   /* number types, booleans, 3D math classes */
   virtual void OnBasicType(const char* typeName,
                            const char* name,
                            const char* metadata,
                            void* data) = 0;
   virtual void OnEnumType(const char* typeName,
                           const char* name,
                           const char* metadata,
                           const ctReflectEnumInfo& enumInfo,
                           uint64_t& data) = 0;
   virtual void
   OnByteBuffer(const char* name, const char* metadata, size_t size, uint8_t* data) = 0;
   virtual void
   OnCStringBuffer(const char* name, const char* metadata, size_t size, char* data) = 0;
   virtual void OnDynamicString(const char* name,
                                const char* metadata,
                                class ctStringUtf8& string) = 0;

   virtual void
   OnStructBegin(const char* typeName, const char* name, const char* metadata) = 0;
   virtual void
   OnStructEnd(const char* typeName, const char* name, const char* metadata) = 0;
   virtual void OnContainerBegin(const char* typeName,
                                 const char* name,
                                 const char* metadata,
                                 ctReflectContainerInterface& containerInterface) = 0;
   virtual void
   OnContainerEntryBegin(size_t index,
                         ctReflectContainerInterface& containerInterface) = 0;
   virtual void OnContainerEntryEnd(size_t index,
                                    ctReflectContainerInterface& containerInterface) = 0;
   virtual void OnContainerEnd(const char* typeName,
                               const char* name,
                               const char* metadata,
                               ctReflectContainerInterface& containerInterface) = 0;

   void LowLevelBasicType(const char* typeName,
                          const char* name,
                          size_t offset,
                          size_t size,
                          size_t fixedCount,
                          ctReflectContainerInterface* pContainerInterface,
                          const char* metadata,
                          void* data);
   void LowLevelEnumType(const char* typeName,
                         const char* name,
                         size_t offset,
                         size_t size,
                         size_t fixedCount,
                         ctReflectContainerInterface* pContainerInterface,
                         const ctReflectEnumInfo& enumInfo,
                         const char* metadata,
                         void* data);
   void LowLevelStructType(const char* typeName,
                           const char* name,
                           size_t offset,
                           size_t size,
                           size_t fixedCount,
                           ctReflectContainerInterface* pContainerInterface,
                           const char* metadata,
                           const class ctReflectorBase& reflector,
                           void* data);
   void LowLevelPolyType(const char* typeName,
                         const char* name,
                         size_t offset,
                         size_t size,
                         size_t fixedCount,
                         ctReflectContainerInterface* pContainerInterface,
                         const char* metadata,
                         void* data);
};

/* sidecar or even base class object which caries reflection */
class ctReflectorBase {
public:
   virtual size_t GetStructSize() const = 0;
   virtual const char* GetStructName() const = 0;
   virtual void GetStructReflection(ctReflectContext& ctx, void* data) const = 0;
};

/* ------------------ DEFINITION ------------------ */

// clang-format off
#define CT_REFLECT_ENUM_DECLARE(_TYPE_NAME) const ctReflectEnumInfo& _##_TYPE_NAME##_citenumreflget();
#define CT_REFLECT_ENUM_DEFINE_START(_TYPE_NAME, _ENUM_TYPE) const ctReflectEnumInfo& _##_TYPE_NAME##_citenumreflget() \
   { static const ctReflectEnumType etype = _ENUM_TYPE; \
static const ctReflectEnumEntryInfo _##_TYPE_NAME##_enum_info_ents[] = {
#define CT_REFLECT_ENUM_DEFINE_ENTRY(_TYPE_NAME, _NAME, _FRIENDLY_NAME) { #_NAME, _FRIENDLY_NAME, (uint64_t)_NAME },
#define CT_REFLECT_ENUM_DEFINE_END(_TYPE_NAME) CT_REFLECT_ENUM_DEFINE_ENTRY(NULL, NULL, 0)}; \
    static const ctReflectEnumInfo _##_TYPE_NAME##_enum_info = { etype, _##_TYPE_NAME##_enum_info_ents };\
    return _##_TYPE_NAME##_enum_info; \
};
// clang-format on

#define CT_REFLECT_ENUM_FORWARD(_CLASS_TYPE)                                            \
   const ctReflectEnumInfo& _##_CLASS_TYPE##_citenumreflget();

/* Put in include files to add reflectors to a type */
#define CT_REFLECT_CLASS_DECLARE(_CLASS_TYPE)                                            \
   class _##_CLASS_TYPE##_citrefl : public ctReflectorBase {                             \
   public:                                                                               \
      virtual void GetStructReflection(ctReflectContext& ctx, void* data) const;         \
      virtual const char* GetStructName() const;                                         \
      virtual size_t GetStructSize() const;                                              \
   };                                                                                    \
   const ctReflectorBase& _##_CLASS_TYPE##_citreflget();

/* add this to classes which inherit from ctReflectorBase */
#define CT_REFLECT_CLASS_DECLARE_VFUNC()                                                 \
   virtual void GetStructReflection(ctReflectContext& ctx, void* data) const;            \
   virtual const char* GetStructName() const;                                            \
   virtual size_t GetStructSize() const;

/* allow reflector to access protected members */
#define CT_REFLECT_ACCESS_PROTECTED(_CLASS_TYPE) friend class _##_CLASS_TYPE##_citrefl;

/* forward declarations in code generation */
#define CT_REFLECT_CLASS_FORWARD(_CLASS_TYPE)                                            \
   const ctReflectorBase& _##_CLASS_TYPE##_citreflget();

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
      return sizeof(_CLASS_TYPE);                                                        \
   }                                                                                     \
   void _##_CLASS_TYPE##_citrefl::GetStructReflection(ctReflectContext& _CTX,            \
                                                      void* _DATA) const

/* to implement the reflector vfunctions on a poly type (inherits from ctReflectorBase) */
#define CT_IMPLEMENT_REFLECTOR_VFUNC_BASE(_CLASS_TYPE)                                   \
   void _CLASS_TYPE::GetStructReflection(ctReflectContext& _CTX, void* _DATA) const {    \
      _##_CLASS_TYPE##_citreflget().GetStructReflection(_CTX, _DATA);                    \
   }                                                                                     \
   const char* _CLASS_TYPE::GetStructName() const {                                      \
      return _##_CLASS_TYPE##_citreflget().GetStructName();                              \
   }                                                                                     \
   size_t _CLASS_TYPE::GetStructSize() const {                                           \
      return _##_CLASS_TYPE##_citreflget().GetStructSize();                              \
   }

/* for children of classes that inherit from ctReflectorBase */
#define CT_IMPLEMENT_REFLECTOR_VFUNC(_BASE_CLASS, _CLASS_TYPE)                           \
   void _CLASS_TYPE::GetStructReflection(ctReflectContext& _CTX, void* _DATA) const {    \
      _##_BASE_CLASS##_citreflget().GetStructReflection(_CTX, _DATA);                    \
      _##_CLASS_TYPE##_citreflget().GetStructReflection(_CTX, _DATA);                    \
   }                                                                                     \
   const char* _CLASS_TYPE::GetStructName() const {                                      \
      return _##_CLASS_TYPE##_citreflget().GetStructName();                              \
   }                                                                                     \
   size_t _CLASS_TYPE::GetStructSize() const {                                           \
      return _##_CLASS_TYPE##_citreflget().GetStructSize();                              \
   }

/* static inheritance */
#define CT_IMPLEMENT_REFLECTOR_STATIC_INHERIT(_BASE_CLASS, _CLASS_TYPE)                  \
   _##_BASE_CLASS##_citreflget().GetStructReflection(_CTX, _DATA);

/* Adds a basic type (whatever is handled natively by the context) */
#define CT_REFLECT_BASIC_TYPE(_CLASS_TYPE, _TYPE, _NAME, _METADATA)                      \
   _CTX.LowLevelBasicType(#_TYPE,                                                        \
                          #_NAME,                                                        \
                          offsetof(_CLASS_TYPE, _NAME),                                  \
                          sizeof(_TYPE),                                                 \
                          1,                                                             \
                          NULL,                                                          \
                          _METADATA,                                                     \
                          _DATA)

#define CT_REFLECT_ENUM_TYPE(_CLASS_TYPE, _TYPE, _NAME, _METADATA)                       \
   _CTX.LowLevelEnumType(#_TYPE,                                                         \
                         #_NAME,                                                         \
                         offsetof(_CLASS_TYPE, _NAME),                                   \
                         sizeof(_TYPE),                                                  \
                         1,                                                              \
                         NULL,                                                           \
                         _##_TYPE##_citenumreflget(),                                    \
                         _METADATA,                                                      \
                         _DATA)

/* for other classes/structs that have a sidecar ctReflectorBase defined */
#define CT_REFLECT_STRUCT_TYPE(_CLASS_TYPE, _TYPE, _NAME, _METADATA)                     \
   _CTX.LowLevelStructType(#_TYPE,                                                       \
                           #_NAME,                                                       \
                           offsetof(_CLASS_TYPE, _NAME),                                 \
                           sizeof(_TYPE),                                                \
                           1,                                                            \
                           NULL,                                                         \
                           _METADATA,                                                    \
                           _##_TYPE##_citreflget(),                                      \
                           _DATA)

/* for classes that derive from ctReflectorBase themself */
#define CT_REFLECT_POLY_TYPE(_CLASS_TYPE, _TYPE, _NAME, _METADATA)                       \
   _CTX.LowLevelPolyType(#_TYPE,                                                         \
                         #_NAME,                                                         \
                         offsetof(_CLASS_TYPE, _NAME),                                   \
                         sizeof(_TYPE),                                                  \
                         1,                                                              \
                         NULL,                                                           \
                         _METADATA,                                                      \
                         _DATA)

/* Adds a citrus container */
#define CT_REFLECT_CONTAINER_BASIC_TYPE(                                                 \
  _CLASS_TYPE, _CONTAINER, _TYPE, _NAME, _METADATA)                                      \
   ctReflectContainerInterface _##_NAME##_CIF =                                          \
     _CONTAINER<_TYPE>::_GetReflectInterface();                                          \
   _CTX.LowLevelBasicType(#_TYPE,                                                        \
                          #_NAME,                                                        \
                          offsetof(_CLASS_TYPE, _NAME),                                  \
                          sizeof(_TYPE),                                                 \
                          1,                                                             \
                          &_##_NAME##_CIF,                                               \
                          _METADATA,                                                     \
                          _DATA)

#define CT_REFLECT_CONTAINER_ENUM_TYPE(_CLASS_TYPE, _CONTAINER, _TYPE, _NAME, _METADATA) \
   ctReflectContainerInterface _##_NAME##_CIF =                                          \
     _CONTAINER<_TYPE>::_GetReflectInterface();                                          \
   _CTX.LowLevelEnumType(#_TYPE,                                                         \
                         #_NAME,                                                         \
                         offsetof(_CLASS_TYPE, _NAME),                                   \
                         sizeof(_TYPE),                                                  \
                         1,                                                              \
                         &_##_NAME##_CIF,                                                \
                         _##_TYPE##_citenumreflget(),                                    \
                         _METADATA,                                                      \
                         _DATA)

#define CT_REFLECT_CONTAINER_STRUCT_TYPE(                                                \
  _CLASS_TYPE, _CONTAINER, _TYPE, _NAME, _METADATA)                                      \
   ctReflectContainerInterface _##_NAME##_CIF =                                          \
     _CONTAINER<_TYPE>::_GetReflectInterface();                                          \
   _CTX.LowLevelStructType(#_TYPE,                                                       \
                           #_NAME,                                                       \
                           offsetof(_CLASS_TYPE, _NAME),                                 \
                           sizeof(_TYPE),                                                \
                           1,                                                            \
                           &_##_NAME##_CIF,                                              \
                           _METADATA,                                                    \
                           _##_TYPE##_citreflget(),                                      \
                           _DATA)

#define CT_REFLECT_CONTAINER_POLY_TYPE(_CLASS_TYPE, _CONTAINER, _TYPE, _NAME, _METADATA) \
   ctReflectContainerInterface _##_NAME##_CIF =                                          \
     _CONTAINER<_TYPE>::_GetReflectInterface();                                          \
   _CTX.LowLevelPolyType(#_TYPE,                                                         \
                         #_NAME,                                                         \
                         offsetof(_CLASS_TYPE, _NAME),                                   \
                         sizeof(_TYPE),                                                  \
                         1,                                                              \
                         &_##_NAME##_CIF,                                                \
                         _METADATA,                                                      \
                         _DATA)

/* 2 param containers */
#define CT_REFLECT_CONTAINER2_BASIC_TYPE(                                                \
  _CLASS_TYPE, _CONTAINER, _TYPE, _AUX, _NAME, _METADATA)                                \
   ctReflectContainerInterface _##_NAME##_CIF =                                          \
     _CONTAINER<_TYPE, _AUX>::_GetReflectInterface();                                    \
   _CTX.LowLevelBasicType(#_TYPE,                                                        \
                          #_NAME,                                                        \
                          offsetof(_CLASS_TYPE, _NAME),                                  \
                          sizeof(_TYPE),                                                 \
                          1,                                                             \
                          &_##_NAME##_CIF,                                               \
                          _METADATA,                                                     \
                          _DATA)

#define CT_REFLECT_CONTAINER2_ENUM_TYPE(                                                 \
  _CLASS_TYPE, _CONTAINER, _TYPE, _AUX, _NAME, _METADATA)                                \
   ctReflectContainerInterface _##_NAME##_CIF =                                          \
     _CONTAINER<_TYPE, _AUX>::_GetReflectInterface();                                    \
   _CTX.LowLevelEnumType(#_TYPE,                                                         \
                         #_NAME,                                                         \
                         offsetof(_CLASS_TYPE, _NAME),                                   \
                         sizeof(_TYPE),                                                  \
                         1,                                                              \
                         &_##_NAME##_CIF,                                                \
                         _##_TYPE##_citenumreflget(),                                    \
                         _METADATA,                                                      \
                         _DATA)

#define CT_REFLECT_CONTAINER2_STRUCT_TYPE(                                               \
  _CLASS_TYPE, _CONTAINER, _TYPE, _AUX, _NAME, _METADATA)                                \
   ctReflectContainerInterface _##_NAME##_CIF =                                          \
     _CONTAINER<_TYPE, _AUX>::_GetReflectInterface();                                    \
   _CTX.LowLevelStructType(#_TYPE,                                                       \
                           #_NAME,                                                       \
                           offsetof(_CLASS_TYPE, _NAME),                                 \
                           sizeof(_TYPE),                                                \
                           1,                                                            \
                           &_##_NAME##_CIF,                                              \
                           _METADATA,                                                    \
                           _##_TYPE##_citreflget(),                                      \
                           _DATA)

#define CT_REFLECT_CONTAINER2_POLY_TYPE(                                                 \
  _CLASS_TYPE, _CONTAINER, _TYPE, _AUX, _NAME, _METADATA)                                \
   ctReflectContainerInterface _##_NAME##_CIF =                                          \
     _CONTAINER<_TYPE, _AUX>::_GetReflectInterface();                                    \
   _CTX.LowLevelPolyType(#_TYPE,                                                         \
                         #_NAME,                                                         \
                         offsetof(_CLASS_TYPE, _NAME),                                   \
                         sizeof(_TYPE),                                                  \
                         1,                                                              \
                         &_##_NAME##_CIF,                                                \
                         _METADATA,                                                      \
                         _DATA)

/* C arrays */
#define CT_REFLECT_CARRAY_BASIC_TYPE(_CLASS_TYPE, _TYPE, _NAME, _METADATA)               \
   _CTX.LowLevelBasicType(#_TYPE,                                                        \
                          #_NAME,                                                        \
                          offsetof(_CLASS_TYPE, _NAME),                                  \
                          sizeof(_TYPE),                                                 \
                          ctCStaticArrayLen(_CLASS_TYPE::_NAME),                         \
                          NULL,                                                          \
                          _METADATA,                                                     \
                          _DATA)

#define CT_REFLECT_CARRAY_ENUM_TYPE(_CLASS_TYPE, _TYPE, _NAME, _METADATA)                \
   _CTX.LowLevelEnumType(#_TYPE,                                                         \
                         #_NAME,                                                         \
                         offsetof(_CLASS_TYPE, _NAME),                                   \
                         sizeof(_TYPE),                                                  \
                         ctCStaticArrayLen(_CLASS_TYPE::_NAME),                          \
                         NULL,                                                           \
                         _##_TYPE##_citenumreflget(),                                    \
                         _METADATA,                                                      \
                         _DATA)

#define CT_REFLECT_CARRAY_STRUCT_TYPE(_CLASS_TYPE, _TYPE, _NAME, _METADATA)              \
   _CTX.LowLevelStructType(#_TYPE,                                                       \
                           #_NAME,                                                       \
                           offsetof(_CLASS_TYPE, _NAME),                                 \
                           sizeof(_TYPE),                                                \
                           ctCStaticArrayLen(_CLASS_TYPE::_NAME),                        \
                           NULL,                                                         \
                           _METADATA,                                                    \
                           _##_TYPE##_citreflget(),                                      \
                           _DATA)

#define CT_REFLECT_CARRAY_POLY_TYPE(_CLASS_TYPE, _TYPE, _NAME, _METADATA)                \
   _CTX.LowLevelPolyType(#_TYPE,                                                         \
                         #_NAME,                                                         \
                         offsetof(_CLASS_TYPE, _NAME),                                   \
                         sizeof(_TYPE),                                                  \
                         ctCStaticArrayLen(_CLASS_TYPE::_NAME),                          \
                         NULL,                                                           \
                         _METADATA,                                                      \
                         _DATA)

/* ------------------ Calling ------------------ */

#define CT_REFLECTOR_GET(_CLASS_TYPE) _##_CLASS_TYPE##_citreflget()

#define ctReflectionExecute(_CLASS_TYPE, _REFLECTOR_CONTEXT, _DATA)                      \
   CT_REFLECTOR_GET(_CLASS_TYPE).GetStructReflection(_REFLECTOR_CONTEXT, _DATA)