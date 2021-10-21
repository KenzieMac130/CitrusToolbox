/*
   Copyright 2021 MacKenzie Strand

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

#include "formats/package/CitrusPackage.h"

#define PACKAGE_NAME_1 "TEST_PACKAGE_1"
#define PACKAGE_NAME_2 "TEST_PACKAGE_2"

void debugCallback(int level, const char* format, va_list args) {
   char tmp[CT_MAX_LOG_LENGTH];
   memset(tmp, 0, CT_MAX_LOG_LENGTH);
   vsnprintf(tmp, CT_MAX_LOG_LENGTH - 1, format, args);
   TracyMessage(tmp, strlen(tmp));
   vprintf(format, args);
   putchar('\n');
}

int write_test(const char* path,
               size_t dataCount,
               const char** ppNames,
               const char** ppStrings) {
   ctPackageWriteContext ctx = ctPackageWriteContextCreate(path);
   for (size_t i = 0; i < dataCount; i++) {
      ctPackageWriteSection(ctx,
                            ppNames[i],
                            NULL,
                            strlen(ppStrings[i]) + 1,
                            (void*)ppStrings[i],
                            CT_PACKAGE_COMPRESSION_NONE);
   }
   ctPackageWriteFinish(ctx);
   ctPackageWriteDestroy(ctx);
   return 0;
}

int main(int argc, char* argv[]) {
   ZoneScoped;
   _ctDebugLogSetCallback(debugCallback);
   const char* sectionNames_1[] = {"SECTION_A", "SECTION_B"};
   const char* sectionNames_2[] = {"SECTION_C"};
   write_test(
     PACKAGE_NAME_1, ctCStaticArrayLen(sectionNames_1), sectionNames_1, sectionNames_1);
   write_test(
     PACKAGE_NAME_2, ctCStaticArrayLen(sectionNames_2), sectionNames_2, sectionNames_2);
   return 0;
}