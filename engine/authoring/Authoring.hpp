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

#pragma once

#include "utilities/Common.h"

class CT_API ctAuthoringAssetGeneratorBase {
public:
   void SetOption(const char* optionName, const char* optionValue);
   const char* GetOptionString(const char* optionName, const char* fallback = NULL);
   bool GetOptionBool(const char* optionName, bool fallback = false);
   double GetOptionNumber(const char* optionName, double fallback = 0.0);

   virtual ctResults Open(ctFile& file) = 0;

   virtual ctResults GetDependencies(ctDynamicArray<ctStringUtf8>& output);
   virtual ctResults GetOutputs(ctDynamicArray<ctStringUtf8>& output);

   /* data to use for archive inheritence */
   virtual ctResults GetArchiveInheritableData(ctAuthoringArchiveData& output);
   virtual ctResults WriteOutputs() = 0;

protected:
   ctFile& OpenOutputFile(ctStringUtf8 output);

private:
   ctHashTable<ctStringUtf8, uint32_t> options;
};

ctAuthoringAssetGeneratorBase* ctAuthoringGetGenerator(const char* filePath);