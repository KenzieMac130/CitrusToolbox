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

#pragma once

#include "utilities/Common.h"

#ifdef __cplusplus
extern "C" {
#endif

void _ctDebugLogSetCallback(void (*callback)(int level,
                                             const char* format,
                                             va_list args));
void _ctDebugLogCallLogger(int level, const char* format, ...);

/**
 * @brief debug logging
 */
#define ctDebugLog(_format_, ...) _ctDebugLogCallLogger(0, _format_, __VA_ARGS__);

#define ctDebugWarning(_format_, ...) _ctDebugLogCallLogger(1, _format_, __VA_ARGS__);

#define ctDebugError(_format_, ...) _ctDebugLogCallLogger(2, _format_, __VA_ARGS__);

#define ctFatalError(_code, _format_, ...)                                               \
   _ctDebugLogCallLogger(3, _format_, __VA_ARGS__);                                      \
   ctDebugError("ERROR EXIT CODE: %d", _code);                                           \
   exit(_code);

#ifdef __cplusplus
}
#endif