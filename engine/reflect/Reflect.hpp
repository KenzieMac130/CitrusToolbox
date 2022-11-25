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

#include "utilities/Common.h"

enum ctReflectorVisibility {
   CT_REFLECT_VIS_UNDEFINED = 0,
   CT_REFLECT_VIS_PUBLIC = 1,
   CT_REFLECT_VIS_PROTECTED = 2,
   CT_REFLECT_VIS_PRIVATE = 3
};

enum ctReflectorType {
   CT_REFLECT_TYPE_ROOT = 0,
   CT_REFLECT_TYPE_VARIABLE = 1,
   CT_REFLECT_TYPE_TYPEDEF = 2,
   CT_REFLECT_TYPE_ENUM = 3,
   CT_REFLECT_TYPE_ENUM_VALUE = 4,
   CT_REFLECT_TYPE_FUNCTION = 5,
   CT_REFLECT_TYPE_STRUCT = 6,
   CT_REFLECT_TYPE_BASIC_TYPE = 7
};

class CT_API ctReflector {
public:
   inline ctReflector() {
      pParent = NULL;
      seqId = 0;
      parentSeqId = 0;
      children.Clear();
      properties.Clear();
      type = CT_REFLECT_TYPE_ROOT;
      parseTracking = false;
   }
   ~ctReflector();

   inline const ctDynamicArray<ctReflector*>& GetChildren() const {
      return children;
   }
   inline const ctReflector* FindChild(const char* name) const {
      for (size_t i = 0; i < children.Count(); i++) {
         if (ctCStrEql(children[i]->GetProperty("NAME", ""), name)) {
            return children[i];
         }
      }
      return NULL;
   }

   inline const char* GetProperty(const char* name,
                                  const char* defaultValue = NULL) const {
      ctStringUtf8* pString = properties.FindPtr(ctXXHash32(name));
      if (pString) { return pString->CStr(); }
      return defaultValue;
   }
   inline double GetPropertyNumber(const char* name, double defaultValue = 0.0) const {
      const char* string = GetProperty(name, NULL);
      if (string) { return strtod(string, NULL); }
      return defaultValue;
   }
   inline bool HasProperty(const char* name) const {
      const char* string = GetProperty(name, NULL);
      return string ? true : false;
   }

   inline const char* GetName() const {
      return GetProperty("NAME");
   }

   inline const char* GetLabel() const {
      return GetProperty("LABEL");
   }

   inline const char* GetDocumentation() const {
      return GetProperty("DOCS");
   }

   inline ctReflectorType GetReflectType() const {
      return type;
   }

   inline ctReflectorVisibility
   GetVisibility(ctReflectorVisibility defaultVisibility = CT_REFLECT_VIS_PUBLIC) {
      const char* visibility = GetProperty("CLASS_VISIBILITY");
      if (ctCStrEql(visibility, "PUBLIC")) {
         return CT_REFLECT_VIS_PUBLIC;
      } else if (ctCStrEql(visibility, "PROTECTED")) {
         return CT_REFLECT_VIS_PROTECTED;
      } else if (ctCStrEql(visibility, "PRIVATE")) {
         return CT_REFLECT_VIS_PRIVATE;
      } else {
         return defaultVisibility;
      }
   }

protected:
   void InitBaseTypes();
   ctResults DigestContents(const char* text);

   ctResults LoadBinaryContents(ctFile& file);
   ctResults SaveBinaryContents(ctFile& file);

   ctResults LoadBinaryContentsTree(ctFile& file);
   ctResults SaveBinaryContentsTree(ctFile& file, uint32_t depth = 0);

   void AddChild(ctReflector* child) {
      children.Append(child);
      child->pParent = this;
   }

   void SetParent(ctReflector* parent) {
      AddChild(this);
   }

   const ctReflector* GetRoot() const {
      if (!pParent) {
         return this;
      } else {
         return GetRoot();
      }
   }

   inline void SetProperty(const char* name, const char* value) {
      properties.Insert(ctXXHash32(name), value);
   }
   inline void SetPropertyNumber(const char* name, double value) {
      char str[16];
      snprintf(str, 16, "%f", value);
      properties.Insert(ctXXHash32(name), str);
   }
   inline void SetPropertyFlag(const char* name) {
      properties.Insert(ctXXHash32(name), "");
   }

private:
   void AddBaseReflector(const char* name,
                         size_t size,
                         const char* argA = NULL,
                         const char* argB = NULL);
   void ParseCommitReflector(ctReflector*& reflector);
   ctResults ExtractComment(const char*& text, size_t& size, const char*& token);
   ctResults ExtractCodeLine(const char*& text, size_t& size, const char*& token);
   ctResults ExtractStruct(const char*& text, size_t& size, const char*& token);
   ctResults ExtractEnum(const char*& text, size_t& size, const char*& token);
   ctResults ExtractEnumValue(const char*& text, size_t& size, const char*& token);
   ctResults ExtractVarOrFunction(const char*& text,
                                  size_t& size,
                                  const char*& token,
                                  bool restrictToVariable = false);

   uint32_t seqId;
   uint32_t parentSeqId;
   inline void GenSeqIds(uint32_t& counter) {
      seqId = counter;
      counter++;
      for (size_t i = 0; i < children.Count(); i++) {
         children[i]->GenSeqIds(counter);
      }
   }

   bool parseTracking;
   ctReflectorType type;
   ctReflector* pParent;
   ctDynamicArray<ctReflector*> children;
   ctHashTable<ctStringUtf8, uint32_t> properties;
};

class CT_API ctReflectorBasicType : public ctReflector {
public:
   inline bool isChar() const {
      return HasProperty("IS_CHAR");
   }
   inline bool isBool() const {
      return HasProperty("IS_BOOL");
   }
   inline bool isInteger() const {
      return HasProperty("IS_INTEGER");
   }
   inline bool isFloat() const {
      return HasProperty("IS_FLOAT");
   }
   inline bool isUnsigned() const {
      return HasProperty("IS_UNSIGNED");
   }
   inline size_t GetByteSize() const {
      return GetPropertyNumber("BYTE_COUNT", 0);
   }
};

class CT_API ctReflectorVariable : public ctReflector {
public:
   inline bool isConst() const {
      return HasProperty("IS_CONST");
   }

   inline bool isDynamicArray() const {
      return HasProperty("IS_DYNAMIC_ARRAY");
   }
   inline bool isStaticArray() const {
      return HasProperty("IS_STATIC_ARRAY");
   }
   inline bool isHashTable() const {
      return HasProperty("IS_HASH_TABLE");
   }

   inline const ctReflector* GetType() const {
      const char* typeString = GetProperty("TYPE");
      if (!typeString) { return NULL; }
      return GetRoot()->FindChild(typeString);
   }

   inline ctReflectorVariable* GetHashKeyType() const {
      const char* typeString = GetProperty("HASH_TYPE");
      if (!typeString) { return NULL; }
      return (ctReflectorVariable*)GetRoot()->FindChild(typeString);
   }
   inline size_t GetStaticArrayCapacity() const {
      return (size_t)GetPropertyNumber("STATIC_ARRAY_CAPACITY", 0);
   }

   inline size_t GetPointerLevel() const {
      return (size_t)GetPropertyNumber("POINTER_LEVEL", 0);
   }
   inline size_t GetReferenceLevel() const {
      return (size_t)GetPropertyNumber("REFERENCE_LEVEL", 0);
   }

   inline size_t GetCArrayLevel() const {
      return (size_t)GetPropertyNumber("C_ARRAY_LEVEL", 0);
   }
   inline size_t GetCountAtCArrayLevel(size_t level = 0) const {
      const char* token = GetProperty("C_ARRAY_SIZES", "0");
      char numStr[16];
      memset(numStr, 0, 16);
      for (int i = 0; i < level; i++) {
         const char* nextDivider = strchr(token, ',');
         if (!nextDivider) { nextDivider = token + strlen(token); }
         size_t length = nextDivider - token;
         if (length > 15) { length = 15; }
         strncpy(numStr, token, length);
         token = nextDivider;
      }
      numStr[15] = 0;
      return strtoll(numStr, NULL, 2);
   }

   inline const char* GetDefaultValueRaw() const {
      GetProperty("DEFAULT", "0");
   }
};

class CT_API ctReflectorEnumValue : public ctReflector {
public:
   size_t GetValue() const {
      return (size_t)GetPropertyNumber("VALUE", 0);
   }
   inline const char* GetDefaultValueRaw() const {
      GetProperty("VALUE", "0");
   }
};

class CT_API ctReflectorEnum : public ctReflector {
public:
   inline size_t GetEntryCount() const {
      return GetChildren().Count();
   }
   inline ctReflectorEnumValue* GetEntry(size_t index = 0) const {
      return (ctReflectorEnumValue*)GetChildren()[index];
   }
};

class CT_API ctReflectorFunction : public ctReflector {
public:
   inline bool isVirtual() const {
      return HasProperty("IS_VIRTUAL");
   }
   inline bool isConstructor() const {
      return HasProperty("IS_CONSTRUCTOR");
   }
   inline bool isDestructor() const {
      return HasProperty("IS_DESTRUCTOR");
   }
   inline bool isOperator() const {
      return HasProperty("IS_OPERATOR");
   }
   inline bool isStatic() const {
      return HasProperty("IS_STATIC");
   }
   inline bool isConst() const {
      return HasProperty("IS_MEMBER_CONST");
   }
   inline bool isInline() const {
      return HasProperty("IS_INLINE");
   }

   inline const ctReflectorVariable* GetReturnType() const {
      return (ctReflectorVariable*)this;
   }

   inline size_t GetArgumentCount() const {
      return GetChildren().Count();
   }
   inline ctReflectorVariable* GetArgument(size_t index = 0) const {
      if (HasProperty("IS_STATIC_ARRAY") || HasProperty("IS_DYNAMIC_ARRAY")) {
         index += 1;
      } else if (HasProperty("IS_HASH_TABLE")) {
         index += 2;
      }
      return (ctReflectorVariable*)GetChildren()[index];
   }
};

class CT_API ctReflectorStruct : public ctReflector {
public:
   inline ctReflectorStruct* GetInheritance() const {
      const char* typeString = GetProperty("CLASS_PARENT");
      if (!typeString) { return NULL; }
      return (ctReflectorStruct*)GetRoot()->FindChild(typeString);
   }
};