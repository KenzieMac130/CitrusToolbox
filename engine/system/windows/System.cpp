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

#include "../System.h"
#include <Windows.h>

int ctSystemCreateGUID(void* guidPtr) {
   static_assert(sizeof(GUID) == 16, "Windows GUID no longer is 128bit!");
   const HRESULT result = CoCreateGuid((GUID*)guidPtr);
   if (result != S_OK) { return -1; };
   return 0;
}

int ctSystemFilePathLocalize(char* str) {
   for (int i = 0; i < strlen(str); i++) {
      if (str[i] == '/') { str[i] = '\\'; }
   }
   return 0;
}

int ctSystemInitialGetLanguage(char* buff, size_t max) {
   wchar_t data[LOCALE_NAME_MAX_LENGTH];
   GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, LOCALE_SNAME, data, LOCALE_NAME_MAX_LENGTH);
   WideCharToMultiByte(
     CP_UTF8, 0, data, LOCALE_NAME_MAX_LENGTH, buff, (int)max, NULL, NULL);
   return 0;
}

int ctSystemExecuteCommand(const char* commandAlias, int argc, const char* argv[]) {
   size_t argStrSize = strlen(commandAlias) + 1;
   for (int i = 0; i < argc; i++) {
      argStrSize += strlen(argv[i]) + 1;
   }
   char* argStr = (char*)malloc(argStrSize);
   if (!argStr) { return -1000000; }
   memset(argStr, 0, argStrSize);
   argStrSize -= strlen(commandAlias) + 1;
   strncat(argStr, commandAlias, argStrSize);
   for (int i = 0; i < argc; i++) {
      strncat(argStr, " ", argStrSize);
      argStrSize -= 1;
      strncat(argStr, argv[i], argStrSize);
      argStrSize -= strlen(argv[i]);
   }
   int result = system(argStr);
   free(argStr);
   return result;
}

int ctSystemShowFileToDeveloper(const char* path) {
   return system(path);
}
