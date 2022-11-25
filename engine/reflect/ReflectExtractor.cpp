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

#include "Reflect.hpp"
#include "ctype.h"

int isAlnumToken(int chr) {
   return (isalnum(chr) || chr == '.' || chr == '_');
}

int NonAlnumTokenLength(const char* str) {
   const char* trippleCombos[] = {">>=", ">>="};
   const char* doubleCombos[] = {"/*",
                                 "*/",
                                 "->",
                                 "::",
                                 ">>",
                                 "<<",
                                 "==",
                                 "++",
                                 "--",
                                 ">=",
                                 "<=",
                                 "<<=",
                                 ">>="
                                 "|=",
                                 "^=",
                                 "&=",
                                 "!=",
                                 "&&",
                                 "||",
                                 "/=",
                                 "+=",
                                 "-=",
                                 "%="};
   for (int i = 0; i < ctCStaticArrayLen(trippleCombos); i++) {
      if (ctCStrNEql(str, trippleCombos[i], 3)) { return 3; }
   }
   for (int i = 0; i < ctCStaticArrayLen(doubleCombos); i++) {
      if (ctCStrNEql(str, doubleCombos[i], 2)) { return 2; }
   }
   return 1;
};

inline const char* GetNextToken(const char*& io, size_t& size) {
   const char* begin = io;
   const char* end;
   if (*begin == '\0') {
      size = 0;
      return NULL;
   }
   while (isspace(*begin)) {
      begin++;
   }
   end = begin;
   if (isAlnumToken(*end)) {
      while (isAlnumToken(*end)) {
         end++;
      }
   } else if (*end == '\"') {
      end++;
      while (*end && *end != '\"') {
         if (*end == '\\') {
            end += 2;
         } else {
            end++;
         }
      }
      end++;
   } else {
      end += NonAlnumTokenLength(end);
   }
   io = end;
   size = (size_t)(end - begin);
   return begin;
}

ctReflectorVisibility ProcessAnyVisibility(ctReflectorVisibility defaultVis,
                                           const char*& text,
                                           size_t& size,
                                           const char*& token) {
   if (ctCStrNEql(token, "public", size)) {
      return CT_REFLECT_VIS_PUBLIC;
   } else if (ctCStrNEql(token, "private", size)) {
      return CT_REFLECT_VIS_PROTECTED;
   } else if (ctCStrNEql(token, "protected", size)) {
      return CT_REFLECT_VIS_PRIVATE;
   } else {
      return defaultVis;
   }
}

/* only standard sized data is allowed */
void ctReflector::InitBaseTypes() {
   AddBaseReflector("bool", 1, "IS_BOOL", NULL); /* force bool size */

   AddBaseReflector("int8_t", sizeof(int8_t), "IS_INTEGER", NULL);
   AddBaseReflector("int16_t", sizeof(int16_t), "IS_INTEGER", NULL);
   AddBaseReflector("int32_t", sizeof(int32_t), "IS_INTEGER", NULL);
   AddBaseReflector("int64_t", sizeof(int64_t), "IS_INTEGER", NULL);

   AddBaseReflector("uint8_t", sizeof(uint8_t), "IS_INTEGER", "IS_UNSIGNED");
   AddBaseReflector("uint16_t", sizeof(uint16_t), "IS_INTEGER", "IS_UNSIGNED");
   AddBaseReflector("uint32_t", sizeof(uint32_t), "IS_INTEGER", "IS_UNSIGNED");
   AddBaseReflector("uint64_t", sizeof(uint64_t), "IS_INTEGER", "IS_UNSIGNED");

   AddBaseReflector("float", sizeof(float), "IS_FLOAT", NULL);
   AddBaseReflector("double", sizeof(double), "IS_FLOAT", NULL);

   AddBaseReflector("char", sizeof(char), "IS_CHAR", NULL);
   AddBaseReflector("wchar_t", sizeof(wchar_t), "IS_CHAR", NULL);
}

ctResults ctReflector::DigestContents(const char* text) {
   size_t size = 0;
   const char* token = text;
   ctReflector* nextReflector = new ctReflector();
   ctReflectorVisibility visibility = CT_REFLECT_VIS_UNDEFINED;
   while (token = GetNextToken(text, size)) {
      if (ctCStrNEql(token, "/*", size)) {
         nextReflector->ExtractComment(text, size, token);
      }
      if (type == CT_REFLECT_TYPE_STRUCT) {
         visibility = ProcessAnyVisibility(visibility, text, size, token);
      }

      if (nextReflector->parseTracking) {
         nextReflector->ExtractCodeLine(text, size, token);
         /* add visibility if we are sitting inside the struct */
         if (type == CT_REFLECT_TYPE_STRUCT) {
            switch (visibility) {
               case CT_REFLECT_VIS_PUBLIC:
                  nextReflector->SetProperty("CLASS_VISIBILITY", "PUBLIC");
                  break;
               case CT_REFLECT_VIS_PROTECTED:
                  nextReflector->SetProperty("CLASS_VISIBILITY", "PROTECTED");
                  break;
               case CT_REFLECT_VIS_PRIVATE:
                  nextReflector->SetProperty("CLASS_VISIBILITY", "PRIVATE");
                  break;
               default: break;
            }
         }
         ParseCommitReflector(nextReflector);
      }
   }
   return CT_SUCCESS;
}

void ctReflector::AddBaseReflector(const char* name,
                                   size_t size,
                                   const char* argA,
                                   const char* argB) {
   ctReflector* reflector = new ctReflector();
   reflector->SetProperty("NAME", name);
   reflector->SetPropertyNumber("BYTE_COUNT", (double)size);
   if (argA) { reflector->SetPropertyFlag(argA); }
   if (argB) { reflector->SetPropertyFlag(argB); }
}

void ctReflector::ParseCommitReflector(ctReflector*& reflector) {
   AddChild(reflector);
   reflector = new ctReflector();
   parseTracking = false;
}

ctResults
ctReflector::ExtractComment(const char*& text, size_t& size, const char*& token) {
   token = GetNextToken(text, size);
   if (!ctCStrNEql(token, "CT_", 3)) { return CT_SUCCESS; }
   token += 3;
   size -= 3;
   ctStringUtf8 name = ctStringUtf8(token, size);
   ctStringUtf8 contents = "";
   while (1) {
      token = GetNextToken(text, size);
      if (!token) { return CT_FAILURE_SYNTAX_ERROR; }
      if (ctCStrNEql(token, "*/", size)) { break; }
      if (token[0] == '"') {
         token++;
         size -= 2;
      }
      contents.Append(token, size);
   }

   if (name == "REFLECT") { parseTracking = true; }

   contents.ProcessEscapeCodes();
   SetProperty(name.CStr(), contents.CStr());
   return CT_SUCCESS;
}

ctResults
ctReflector::ExtractCodeLine(const char*& text, size_t& size, const char*& token) {
   token = GetNextToken(text, size);
   if (ctCStrNEql(token, "typedef", size)) {
      type = CT_REFLECT_TYPE_TYPEDEF;
      token = GetNextToken(text, size);
   }
   if (ctCStrNEql(token, "struct", size) || ctCStrNEql(token, "class", size)) {
      type = CT_REFLECT_TYPE_STRUCT;
      return ExtractStruct(text, size, token);
   } else if (ctCStrNEql(token, "enum", size)) {
      type = CT_REFLECT_TYPE_ENUM;
      return ExtractEnum(text, size, token);
   } else {
      return ExtractVarOrFunction(text, size, token);
   }
}

ctResults
ctReflector::ExtractStruct(const char*& text, size_t& size, const char*& token) {
   token = GetNextToken(text, size);
   while (ctCStrNEql(token, "CT_API", size)) {
      token = GetNextToken(text, size);
   }
   SetProperty("NAME", ctStringUtf8(token, size).CStr());
   token = GetNextToken(text, size);
   if (ctCStrNEql(token, ":", size)) {
      token = GetNextToken(text, size);
      while (ctCStrNEql(token, "public", size) || ctCStrNEql(token, "private", size) ||
             ctCStrNEql(token, "protected", size)) {
         token = GetNextToken(text, size);
      }
      SetProperty("CLASS_PARENT", ctStringUtf8(token, size).CStr());
   }
   while (!ctCStrNEql(token, "{", size)) {
      if (ctCStrNEql(token, ";", size)) { return CT_SUCCESS; } /* pre-declatation? */
      token = GetNextToken(text, size);
   }
   const char* blockStart = token;
   int32_t bracketLevel = 1;
   while (bracketLevel > 0) {
      token = GetNextToken(text, size);
      if (ctCStrNEql(token, "{", size)) {
         bracketLevel++;
      } else if (ctCStrNEql(token, "}", size)) {
         bracketLevel--;
      }
   }
   /* recurse into structure */
   return DigestContents(ctStringUtf8(blockStart, token - blockStart).CStr());
}

ctResults ctReflector::ExtractEnum(const char*& text, size_t& size, const char*& token) {
   token = GetNextToken(text, size);
   SetProperty("NAME", ctStringUtf8(token, size).CStr());
   token = GetNextToken(text, size); /* Burn '{' */
   while (!ctCStrNEql(token, "}", size)) {
      ctReflector* valReflect = new ctReflector();
      while (ctCStrNEql(token, "/*", size)) {
         valReflect->ExtractComment(text, size, token);
      }
      valReflect->type = CT_REFLECT_TYPE_ENUM_VALUE;
      valReflect->ExtractEnumValue(text, size, token);
   }
   return CT_SUCCESS;
}

ctResults
ctReflector::ExtractEnumValue(const char*& text, size_t& size, const char*& token) {
   token = GetNextToken(text, size);
   SetProperty("NAME", ctStringUtf8(token, size).CStr());
   token = GetNextToken(text, size); /* Burn "=" */
   ctStringUtf8 valueKey = "";
   while (1) {
      token = GetNextToken(text, size);
      if (token == ";") { break; }
      valueKey.Append(token, size);
   }
   SetProperty("VALUE", valueKey.CStr());
   return CT_SUCCESS;
}

bool isOperatorDef(const char* text, size_t size, const char* token) {
   token = GetNextToken(text, size);
   return ctCStrNEql(token, "(", size);
}

/* function pointers are not supported in reflect so we can make this assumption */
bool isFunction(const char* text, size_t size, const char* token) {
   while (!ctCStrNEql(token, ";", size) && !ctCStrNEql(token, "{", size)) {
      if (ctCStrNEql(token, "(", size)) { return true; }
   }
   return false;
}

ctResults ctReflector::ExtractVarOrFunction(const char*& text,
                                            size_t& size,
                                            const char*& token,
                                            bool restrictToVariable) {
   if (type == CT_REFLECT_TYPE_TYPEDEF) {
      restrictToVariable = true;
   } else if (isFunction(text, size, token)) {
      type = CT_REFLECT_TYPE_VARIABLE;
   } else {
      type = CT_REFLECT_TYPE_VARIABLE;
   }
   while (ctCStrNEql(token, "CT_API", size)) {
      token = GetNextToken(text, size);
   }
   bool skipToFunctionArgs = false;
   if (!restrictToVariable) { /* might be function definition */

      if (ctCStrNEql(token, "virtual", size)) {
         SetPropertyFlag("IS_VIRTUAL");
         token = GetNextToken(text, size);
      } else if (ctCStrNEql(token, "inline", size)) {
         SetPropertyFlag("IS_INLINE");
         token = GetNextToken(text, size);
      }
      /* check if member function is static */
      if (ctCStrNEql(token, "static", size)) {
         SetPropertyFlag("IS_STATIC");
         token = GetNextToken(text, size);

         /* order of "inline" is independent */
         if (ctCStrNEql(token, "inline", size)) {
            SetPropertyFlag("IS_INLINE");
            token = GetNextToken(text, size);
         }
      }

      if (token[0] == '~') {
         SetPropertyFlag("IS_DESTRUCTOR");
         token = GetNextToken(text, size); /* burn ~ */
         token = GetNextToken(text, size); /* burn class name */
         skipToFunctionArgs = true;
      } else if (isOperatorDef(text, size, token)) {
         SetPropertyFlag("IS_OPERATOR");
         token = GetNextToken(text, size); /* burn class name */
         skipToFunctionArgs = true;
      }
   }

   if (!skipToFunctionArgs) {
      /* check if const */
      if (ctCStrNEql(token, "const", size)) {
         SetPropertyFlag("IS_CONST");
         token = GetNextToken(text, size);
      }
      /* check if template type and process */
      if (ctCStrNEql(token, "ctDynamicArray", size)) {
         SetPropertyFlag("IS_DYNAMIC_ARRAY");
         ctReflector* argReflect = new ctReflector();
         ExtractVarOrFunction(text, size, token, true); /* templated type */
         SetProperty("TYPE", argReflect->GetProperty("TYPE"));
         AddChild(argReflect);
         token = GetNextToken(text, size); /* burn > */
      } else if (ctCStrNEql(token, "ctStaticArray", size)) {
         SetPropertyFlag("IS_STATIC_ARRAY");
         ctReflector* argReflect = new ctReflector();
         ExtractVarOrFunction(text, size, token, true); /* templated type */
         SetProperty("TYPE", argReflect->GetProperty("TYPE"));
         AddChild(argReflect);
         token = GetNextToken(text, size); /* burn , */
         SetProperty("STATIC_ARRAY_CAPACITY",
                     ctStringUtf8(token, size).CStr()); /* array size */
         token = GetNextToken(text, size);              /* burn > */
      } else if (ctCStrNEql(token, "ctStaticArray", size)) {
         SetPropertyFlag("IS_HASH_TABLE");
         ctReflector* argReflect = new ctReflector();
         ExtractVarOrFunction(text, size, token, true); /* key */
         SetProperty("TYPE", argReflect->GetProperty("TYPE"));
         AddChild(argReflect);
         ExtractVarOrFunction(text, size, token, true); /* value */
         AddChild(argReflect);
         token = GetNextToken(text, size); /* burn > */
      } else {                             /* get simple type name */
         SetProperty("TYPE", ctStringUtf8(token, size).CStr());
      }

      /* get pointer level */
      size_t pointerLevel = 0;
      while (token[0] == '*') {
         pointerLevel++;
         token = GetNextToken(text, size);
      }
      if (pointerLevel) { SetPropertyNumber("POINTER_LEVEL", (double)pointerLevel); }

      /* get reference level */
      size_t referenceLevel = 0;
      while (token[0] == '&') {
         referenceLevel++;
         token = GetNextToken(text, size);
      }
      if (referenceLevel) {
         SetPropertyNumber("REFERENCE_LEVEL", (double)referenceLevel);
      }

      /* check if operator */
      if (!restrictToVariable && ctCStrNEql(token, "operator", size)) {
         SetPropertyFlag("IS_OPERATOR");
         token = GetNextToken(text, size);

         ctStringUtf8 opName = "";
         while (!ctCStrNEql(token, "(", size)) {
            opName.Append(token, size);
            token = GetNextToken(text, size);
         }
         SetProperty("NAME", opName.CStr());

         skipToFunctionArgs = true;
      }

      if (!skipToFunctionArgs) {
         /* get name */
         SetProperty("NAME", ctStringUtf8(token, size).CStr());

         /* get c static arrays */
         token = GetNextToken(text, size);
         size_t arrayLevel = 0;
         ctStringUtf8 arraySizes = "";
         while (token[0] == '[') {
            token = GetNextToken(text, size);
            arraySizes.Append(token, size);
            arraySizes += ',';
            arrayLevel++;
            token = GetNextToken(text, size); /* burn ] */
         }
         if (arrayLevel) {
            SetProperty("C_ARRAY_SIZES", arraySizes.CStr());
            SetPropertyNumber("C_ARRAY_LEVEL", (double)arrayLevel);
         }
      }
   }

   if (!restrictToVariable) { /* might be function definition */
      if (ctCStrNEql(token, "(", size)) {
         type = CT_REFLECT_TYPE_FUNCTION;
         while (!ctCStrNEql(token, ")", size)) {
            ctReflector* argReflect = new ctReflector();
            ExtractVarOrFunction(text, size, token, true);
            AddChild(argReflect);
         }
         token = GetNextToken(text, size); /* burn ) */

         if (ctCStrNEql(token, "const", size)) {
            SetPropertyFlag("IS_MEMBER_CONST");
            token = GetNextToken(text, size);
         }
      }
   }
   return CT_SUCCESS;
}