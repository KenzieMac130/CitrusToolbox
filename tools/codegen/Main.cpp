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
#include "CodeGenerator.hpp"

const char* gHelpString =
  "Example:\n\t<OPTIONS> codegenBasePath projectBasePath pathToInputList.txt\n"
  "Options:"
  "\n\t-help: Show help";

#define FindFlag(_name) _FindFlag(_name, argc, argv)
bool _FindFlag(const char* name, int argc, char* argv[]) {
   for (int i = 3; i < argc; i++) {
      if (ctCStrEql(argv[i], name)) { return true; }
   }
   return false;
}

#define FindParamOccurance(_name, _occ) _FindParamOccurance(_name, _occ, argc, argv)
char* _FindParamOccurance(const char* name, int occurance, int argc, char* argv[]) {
   int j = 0;
   for (int i = 3; i < argc; i++) {
      if (ctCStrEql(argv[i], name)) {
         if (j == occurance) { return argv[i + 1]; }
         j++;
      }
   }
   return NULL;
}

#define FindParam(_name) FindParamOccurance(_name, 0)

int main(int argc, char* argv[]) {
   if (argc < 3) {
      ctDebugError("Not enough args!\n%s", gHelpString);
      return -1;
   }

   if (FindFlag("-help")) { ctDebugLog(gHelpString); }

   ctCodeGenerator codegen = ctCodeGenerator();
   ctStringUtf8 codegenPath = argv[1];
   ctStringUtf8 projectPath = argv[2];
   ctStringUtf8 fileListPath = argv[3];
   codegenPath.FilePathUnify();
   codegenPath.FilePathAppend("codegen");
   projectPath.FilePathUnify();

   ctFile listFile;
   CT_RETURN_ON_FAIL(listFile.Open(fileListPath, CT_FILE_OPEN_READ_TEXT), -1);
   ctStringUtf8 listContents = "";
   listFile.GetText(listContents);
   listFile.Close();

   int32_t idx = 0;
   ctStringUtf8 headerPath;
   ctStringUtf8 remainder = listContents;
   ctResults splitResult = CT_SUCCESS;
   do {
      splitResult = remainder.Split(";", headerPath);
      ctStringUtf8 outputRelativeFilePath = headerPath;
      outputRelativeFilePath.FilePathMakeRelative(projectPath);
      ctStringUtf8 outputHeaderFilePath = codegenPath;
      outputHeaderFilePath.FilePathAppend(outputRelativeFilePath);
      ctStringUtf8 outputImplFilePath = outputHeaderFilePath;
      outputImplFilePath.FilePathRemoveExtension();
      outputImplFilePath += ".cpp";

      ctFile headerFile;
      ctFile outHeaderFile;
      ctFile outImplFile;
      CT_RETURN_ON_FAIL(headerFile.Open(headerPath, CT_FILE_OPEN_READ_TEXT), -1);
      CT_RETURN_ON_FAIL(outHeaderFile.Open(outputHeaderFilePath, CT_FILE_OPEN_WRITE_TEXT),
                        -1);
      CT_RETURN_ON_FAIL(outImplFile.Open(outputImplFilePath, CT_FILE_OPEN_WRITE_TEXT),
                        -1);
      CT_RETURN_ON_FAIL(codegen.Execute(headerFile, outHeaderFile, outImplFile), -2);
      headerFile.Close();
      outHeaderFile.Close();
      outImplFile.Close();
   } while (splitResult == CT_SUCCESS);

   return 0;
}