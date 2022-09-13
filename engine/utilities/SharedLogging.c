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

#include "SharedLogging.h"

void (*shared_callback)(int level, const char* format, va_list args);

void _ctDebugLogSetCallback(void (*callback)(int level,
                                             const char* format,
                                             va_list args)) {
   shared_callback = callback;
}

void _ctDebugLogCallLogger(int level, const char* format, ...) {
   va_list args;
   va_start(args, format);
   if (shared_callback) {
      shared_callback(level, format, args);
   } else {
      vprintf(format, args);
      putchar('\n');
   }
   va_end(args);
}
