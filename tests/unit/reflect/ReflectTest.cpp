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

#include "reflect/Reflect.hpp"

#define TEST_NO_MAIN
#include "acutest/acutest.h"

class ctTestReflector : public ctReflector {
public:
   void GenerateTestData(uint32_t depth, uint32_t maxDepth, uint32_t children) {
      this->SetProperty("STRING", "Hello World!");
      this->SetPropertyFlag("FLAG");
      this->SetPropertyNumber("NUMBER", 25.76);
      this->SetPropertyNumber("DEPTH", (double)depth);
      if (depth >= maxDepth) { return; }
      for (uint32_t i = 0; i < children; i++) {
         ctTestReflector* pReflector = new ctTestReflector();
         pReflector->GenerateTestData(depth + 1, maxDepth, children);
         AddChild(pReflector);
      }
   }
   void PrintTestResults() {
      this;
      bool doshit = true;
   }
   void Save(ctFile file) {
      SaveBinaryContentsTree(file);
   }
   void Load(ctFile file) {
      LoadBinaryContentsTree(file);
   }
   void Parse(const char* str) {
      DigestContents(str);
   }
};

const char* data =
  "/* CT_REFLECT */\n/* CT_DOCS \"My very important ! \\\"FKOEKF\\\" string\" "
  "*/\ntypedef "
  "const "
  "ctHashTable<float, uint32_t>*** "
  "myconstvalue[43][32];\n/* CT_REFLECT */\nclass CT_API NAME : public ctVec2 "
  "{\n\npublic:\n    /* CT_REFLECT */\n    const ctHashTable<float, uint32_t>*** "
  "myconstvalue[43][32];\nfloat***& var;\n/* CT_REFLECT */\nvirtual float fke(int "
  "myvalue, bool myvalue2[34], const char* mystring) const;\n/* CT_REFLECT */\nstatic "
  "float fkd();\n};/* CT_MIN 0 */ /* CT_MAX 20409 */ /* CT_REFLECT */enum MyEnum { VAL0 = "
  "0, VAL1 = 1, VAL3 = 4, VALCOMB = VAL0 | VAL1, VALHEX = 0x21, VAL_MAX};";

void basic_reflect(void) {
   ctTestReflector* pReflector = new ctTestReflector();
   // pReflector->GenerateTestData(0, 4, 4);
   pReflector->Parse(data);

   ctFile bfile = ctFile("REFLECT_TEST_SERIAL.bin", CT_FILE_OPEN_WRITE);
   pReflector->Save(bfile);
   bfile.Close();
   bfile = ctFile("REFLECT_TEST_SERIAL.bin", CT_FILE_OPEN_READ);
   pReflector->Load(bfile);
   pReflector->PrintTestResults();
}