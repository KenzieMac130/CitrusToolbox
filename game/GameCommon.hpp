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

#pragma once

/* Base engine config */
#include "Config.h"

/* Game layer exportable */
// clang-format off
#if CITRUS_STATIC_ONLY
#define GAME_API
#endif
#if !defined(GAME_API)
#if defined(_MSC_VER)
    #if GAME_IMPORT
        #define GAME_API __declspec(dllimport)
    #else
        #define GAME_API __declspec(dllexport)
    #endif
#elif defined(__GNUC__)
    #if GAME_IMPORT
        #define GAME_API __attribute__((visibility("default")))
    #else
        #define GAME_API
    #endif
#else
    #define GAME_API
#endif
#endif

/* Import engine content */
#undef CITRUS_IMPORT
#define CITRUS_IMPORT 1
// clang-format on