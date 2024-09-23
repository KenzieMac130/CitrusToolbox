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

#include "CodeGenerator.hpp"

ctResults
ctCodeGenerator::Execute(ctFile& inHeader, ctFile& outHeader, ctFile& outImplementation) {
   lexer.SetContents(inHeader);
   CT_RETURN_FAIL(lexer.BuildTokens());
   ctLexerIterator it = ctLexerIterator(lexer);
   it.IncludeComments(true);
   headerResult = "/* DO NOT INCLUDE IN OTHER HEADERS! */\n/* CODE GENERATED FILE! DO "
                  "NOT MODIFY! */\n";
   implForwards = "/* CODE GENERATED FILE! DO NOT MODIFY! */\n";

   headerResult += "#ifndef IS_CITRUS_CODEGEN\n";
   implBottom += "#define IS_CITRUS_CODEGEN\n";

   headerResult += "#include \"Reflect.hpp\"\n";
   implForwards.Printf(0, "#include \"%s\"\n", inHeader.GetFilePath());

   for (; it; it++) {
      if (it.Token().isComment("CT_REFLECT")) {}
      /* citrus reflect comment */
      if (it.Token().isComment("CT_REFLECT")) {
         ctLexerIterator lineContents = it;
         CT_RETURN_FAIL(lineContents.SeekToCommentedLineSubject());

         if (lineContents.Token().isIdentifier("class") ||
             lineContents.Token().isIdentifier("struct")) {
            CT_RETURN_FAIL(ParseCtReflectStruct(lineContents));
         }

         if (lineContents.Token().isIdentifier("enum")) {
            CT_RETURN_FAIL(ParseCtReflectEnum(lineContents));
         }

         it.CopyIfGreater(lineContents);
      }
   }
   headerResult += "#endif\n";

   ctDebugLog("Header: %s", headerResult.CStr());
   ctDebugLog("Impl: %s%s", implForwards.CStr(), implBottom.CStr());
   outHeader.Printf("%s", headerResult.CStr());
   outImplementation.Printf("%s%s", implForwards.CStr(), implBottom.CStr());
   return CT_SUCCESS;
}

ctResults ctCodeGenerator::ParseCtReflectEnum(ctLexerIterator& it) {
   ctLexerIterator enumName = ctLexerIterator();
   CT_RETURN_FAIL(it.ReadAsEnumDefinition(&enumName));
   ctStringUtf8 enumNameStr = enumName.ToString();

   headerResult.Printf(0, "CT_REFLECT_ENUM_DECLARE(%s);\n", enumNameStr.CStr());
   implBottom.Printf(0,
                     "CT_REFLECT_ENUM_DEFINE_START(%s, %s)\n",
                     enumNameStr.CStr(),
                     "CT_REFLECT_ENUM_NUMBERED"); /* todo: handle bitmasks */

   ctLexerIterator enumBrackets = ctLexerIterator();
   CT_RETURN_FAIL(it.GetBracketIterator(enumBrackets));
   size_t definitionCount = enumBrackets.GetArraySize();
   for (size_t i = 0; i < definitionCount; i++) {
      ctLexerIterator entryName = ctLexerIterator();
      ctLexerIterator entryDefinition = ctLexerIterator();
      ctLexerIterator entryValue = ctLexerIterator();
      CT_RETURN_FAIL(enumBrackets.ReadAsEnumEntry(&entryName, &entryValue));
      /* todo: friendly name */
      implBottom.Printf(0,
                        "CT_REFLECT_ENUM_DEFINE_ENTRY(%s, %s, \"%s\")\n",
                        enumNameStr.CStr(),
                        entryName.ToString().CStr(),
                        "TODO FRIENDLY NAME");
   }
   implBottom += "CT_REFLECT_ENUM_DEFINE_END()\n";
   return CT_SUCCESS;
}

ctResults ctCodeGenerator::ParseCtReflectStruct(ctLexerIterator& it) {
   ctLexerIterator structName = ctLexerIterator();
   CT_RETURN_FAIL(it.ReadAsStructDefinition(&structName));
   ctStringUtf8 structNameStr = structName.ToString();
   headerResult.Printf(0, "CT_REFLECT_CLASS_DECLARE(%s);\n", structNameStr.CStr());
   implBottom.Printf(0, "CT_IMPLEMENT_REFLECTOR(%s) {\n", structNameStr.CStr());

   ctLexerIterator inheritedName = ctLexerIterator();
   ctLexerIterator inheritedScope = ctLexerIterator();
   while (it.ReadAsStructInheritanceInfo(&inheritedName, &inheritedScope) == CT_SUCCESS) {
      if (inheritedScope) { /* skip if private */
         if (inheritedScope.Token().ExpectContents("private")) { continue; }
      } else { /* default to private */
         continue;
      }

      ctStringUtf8 inheritedNameString = inheritedName.ToString();
      uint32_t classForwardHash = inheritedNameString.xxHash32();
      if (!(inheritedNameString == "ctReflectorBase") &&
          !classForwards.Exists(classForwardHash)) {
         implForwards.Printf(
           0, "CT_REFLECT_CLASS_FORWARD(%s)\n", inheritedNameString.CStr());
         implBottom.Printf(
           0, "CT_IMPLEMENT_REFLECTOR_STATIC_INHERIT(%s);\n", inheritedNameString.CStr());
         classForwards.Insert(classForwardHash, true);
      }
   }
   ctLexerIterator structContents = ctLexerIterator();
   CT_RETURN_FAIL(it.GetBracketIterator(structContents));
   structContents.IncludeComments(true);
   for (; structContents; structContents++) {
      /* citrus reflect comment */
      if (structContents.Token().isComment("CT_REFLECT")) {
         ctLexerIterator lineContents = structContents;
         CT_RETURN_FAIL(lineContents.SeekToCommentedLineSubject());
         ctLexerIterator varName = ctLexerIterator();
         ctLexerIterator typeInfo = ctLexerIterator();
         ctLexerIterator arrayInfo = ctLexerIterator();
         lineContents.ReadAsVariable(&varName, &typeInfo, NULL, &arrayInfo);
         // impl: todo: add the respective type macro
      }
   }
   implBottom += "}\n";
   return CT_SUCCESS;
}
