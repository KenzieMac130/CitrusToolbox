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

#include "Reflect.hpp"

size_t ctReflectGetCArrayGetCount(void* baseAddress, void* extra) {
   return (size_t)extra;
}

/* if fixedCount is greater than 1 then this should be the passed interface */
ctReflectContainerInterface ctReflectGetCArrayContainerInterface(size_t count) {
   ctReflectContainerInterface result = ctReflectContainerInterface();
   result.extra = (void*)count; /* we will never address this! */
   result.containerType = "CArray";
   result.GetCapacity = ctReflectGetCArrayGetCount;
   result.GetCount = ctReflectGetCArrayGetCount;
   return result;
};

#define CT_REFLECT_HANDLE_CONTAINER_START()                                              \
   uint8_t* varAddressBase = (uint8_t*)data + offset;                                    \
   ctReflectContainerInterface fixedInterface = {};                                      \
   if (fixedCount > 1 && !pCallbacks) { /* get fixed array interface */                  \
      fixedInterface = ctReflectGetCArrayContainerInterface(fixedCount);                 \
      pCallbacks = &fixedInterface;                                                      \
   }                                                                                     \
   size_t count = 1; /* get count */                                                     \
   if (pCallbacks) { count = pCallbacks->GetCount(varAddressBase, pCallbacks->extra); }  \
                                                                                         \
   uint8_t* varAddress = varAddressBase;                                                 \
   if (ctCStrEql(typeName, "char") && count > 1) { /* special case: CStrings */          \
      OnCStringBuffer(name, metadata, count, (char*)varAddress);                         \
      return;                                                                            \
   }                                                                                     \
   if (ctCStrEql(typeName, "uint8_t") && count > 1) { /* special case: Bytes */          \
      OnByteBuffer(name, metadata, count, varAddress);                                   \
      return;                                                                            \
   }                                                                                     \
                                                                                         \
   if (pCallbacks) { OnContainerBegin(typeName, name, metadata, *pCallbacks); }          \
   for (size_t i = 0; i < count; i++) {                                                  \
      if (pCallbacks) { OnContainerEntryBegin(i, *pCallbacks); } /* update count */      \
      if (pCallbacks) {              /* get address for container */                     \
         if (pCallbacks->GetValue) { /* has value getter */                              \
            varAddress =                                                                 \
              (uint8_t*)pCallbacks->GetValue(i, varAddressBase, pCallbacks->extra);      \
         } else { /* fixed array */                                                      \
            varAddress = varAddressBase + size * i;                                      \
         }                                                                               \
      }                                                                                  \
      if (ctCStrEql(typeName, "ctStringUtf8")) { /* special case: Citrus String */       \
         OnDynamicString(name, metadata, *(ctStringUtf8*)varAddress);                    \
      } else {
/* DO LOGIC HERE... */
#define CT_REFLECT_HANDLE_CONTAINER_END()                                                \
   }                                                                                     \
   if (pCallbacks) { OnContainerEntryEnd(i, *pCallbacks); }                              \
   if (pCallbacks) { count = pCallbacks->GetCount(varAddressBase, pCallbacks->extra); }  \
   }                                                                                     \
   if (pCallbacks) { OnContainerEnd(typeName, name, metadata, *pCallbacks); }

void ctReflectContext::LowLevelBasicType(const char* typeName,
                                         const char* name,
                                         size_t offset,
                                         size_t size,
                                         size_t fixedCount,
                                         ctReflectContainerInterface* pCallbacks,
                                         const char* metadata,
                                         void* data) {
   CT_REFLECT_HANDLE_CONTAINER_START();
   OnBasicType(typeName, name, metadata, varAddress);
   CT_REFLECT_HANDLE_CONTAINER_END();
}

void ctReflectContext::LowLevelEnumType(const char* typeName,
                                        const char* name,
                                        size_t offset,
                                        size_t size,
                                        size_t fixedCount,
                                        ctReflectContainerInterface* pCallbacks,
                                        const ctReflectEnumInfo& enumInfo,
                                        const char* metadata,
                                        void* data) {
   CT_REFLECT_HANDLE_CONTAINER_START();
   uint64_t ivalue = 0;
   if (size == 1) { ivalue = (uint64_t)(*(uint8_t*)varAddress); }
   if (size == 2) { ivalue = (uint64_t)(*(uint16_t*)varAddress); }
   if (size == 4) { ivalue = (uint64_t)(*(uint32_t*)varAddress); }
   if (size == 8) { ivalue = (uint64_t)(*(uint64_t*)varAddress); }
   uint64_t ivalueOriginal = ivalue;
   OnEnumType(typeName, name, metadata, enumInfo, ivalue);
   if (ivalue != ivalueOriginal) {
      if (size == 1) { *(uint8_t*)varAddress = (uint8_t)ivalue; }
      if (size == 2) { *(uint16_t*)varAddress = (uint16_t)ivalue; }
      if (size == 4) { *(uint32_t*)varAddress = (uint32_t)ivalue; }
      if (size == 8) { *(uint64_t*)varAddress = (uint64_t)ivalue; }
   }
   CT_REFLECT_HANDLE_CONTAINER_END();
}

void ctReflectContext::LowLevelStructType(const char* typeName,
                                          const char* name,
                                          size_t offset,
                                          size_t size,
                                          size_t fixedCount,
                                          ctReflectContainerInterface* pCallbacks,
                                          const char* metadata,
                                          const ctReflectorBase& reflector,
                                          void* data) {
   CT_REFLECT_HANDLE_CONTAINER_START();
   OnStructBegin(typeName, name, metadata);
   reflector.GetStructReflection(*this, varAddress);
   OnStructEnd(typeName, name, metadata);
   CT_REFLECT_HANDLE_CONTAINER_END();
}

void ctReflectContext::LowLevelPolyType(const char* typeName,
                                        const char* name,
                                        size_t offset,
                                        size_t size,
                                        size_t fixedCount,
                                        ctReflectContainerInterface* pCallbacks,
                                        const char* metadata,
                                        void* data) {
   CT_REFLECT_HANDLE_CONTAINER_START();
   ctReflectorBase* pReflector = (ctReflectorBase*)varAddress;
   OnStructBegin(typeName, name, metadata);
   pReflector->GetStructReflection(*this, varAddress);
   OnStructEnd(typeName, name, metadata);
   CT_REFLECT_HANDLE_CONTAINER_END();
}