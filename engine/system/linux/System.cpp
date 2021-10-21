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

#include <stdlib.h>
#include <string.h>

int ctSystemCreateGUID(void* guidPtr){
   /* This is dirty and probably very bad... better than nothing! */
   for (int i = 0; i < 16; i++) {
      data[i] = rand() % 255;
   }
   return 0;
}

int ctSystemFilePathLocalize(char* str){
	for (int i = 0; i < strlen(str); i++) {
		if (str[i] == '\\') { str[i] = '//'; }
	}
	return 0;
}

int ctSystemInitialGetLanguage(char* buff, size_t _max){
/* Try to extract similar string off "setlocale" (tested only on some Debian distros) */
    const char* cbuf = setlocale(LC_ALL, "");
    size_t max = strlen(cbuf) > _max ? _max : strlen(cbuf);
    memset(buff, 0, _max);
    strncpy(buff, cbuf, max);
    char* nextVal = buff;
    while (*nextVal != '\0') {
       if (*nextVal == '_') { *nextVal = '-'; }
       if (*nextVal == '.') {
          *nextVal = '\0';
          break;
       }
       nextVal++;
    }
    if (strncmp(buff,"C")==0) { 
       memset(buff, 0, _max);
    	strncpy(buff, "DEFAULT", max);
    }
    return 0;
}