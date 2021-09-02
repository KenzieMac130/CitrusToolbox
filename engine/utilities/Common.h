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

/* Config File */
#include "Config.h"

#ifdef __cplusplus
#if CITRUS_TRACY
#include "tracy/Tracy.hpp"
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include <assert.h>
#include <stdarg.h>

#if CITRUS_SDL
#include "SDL.h"
#endif

/*Exportable*/
// clang-format off
#if CITRUS_STATIC_ONLY
    #define CT_API
#endif
#if !defined(CT_API)
#if defined(_MSC_VER)
    #if CITRUS_IMPORT
        #define CT_API __declspec(dllimport)
    #else
        #define CT_API __declspec(dllexport)
    #endif
#elif defined(__GNUC__)
    #if CITRUS_IMPORT
        #define CT_API __attribute__((visibility("default")))
    #else
        #define CT_API
    #endif
#else
    #define CT_API
#endif
#endif
// clang-format on

/*Errors*/
enum ctResults {
   CT_SUCCESS = 0,
   CT_FAILURE_UNKNOWN = -1,
   CT_FAILURE_OUT_OF_MEMORY = -2,
   CT_FAILURE_INVALID_PARAMETER = -3,
   CT_FAILURE_UNSUPPORTED_HARDWARE = -4,
   CT_FAILURE_UNKNOWN_FORMAT = -5,
   CT_FAILURE_OUT_OF_BOUNDS = -6,
   CT_FAILURE_PARSE_ERROR = -7,
   CT_FAILURE_DECOMPRESSION_ERROR = -8,
   CT_FAILURE_FILE_NOT_FOUND = -9,
   CT_FAILURE_INACCESSIBLE = -10,
   CT_FAILURE_DATA_DOES_NOT_EXIST = -11,
   CT_FAILURE_DUPLICATE_ENTRY = -12,
   CT_FAILURE_NOT_UPDATABLE = -13,
   CT_FAILURE_COULD_NOT_SHRINK = -14,
   CT_FAILURE_CORRUPTED_CONTENTS = -15,
   CT_FAILURE_DEPENDENCY_NOT_MET = -16,
   CT_FAILURE_MODULE_NOT_INITIALIZED = -17,
   CT_FAILURE_NOT_FOUND = -18,
   CT_FAILURE_SYNTAX_ERROR = -19,
   CT_FAILURE_RUNTIME_ERROR = -19,
   CT_FAILURE_TYPE_ERROR = -20
};

#define CT_PANIC_FAIL(_arg, _message)                                                    \
   {                                                                                     \
      if (_arg != CT_SUCCESS) { ctFatalError(-1, _message); }                            \
   }

#define CT_PANIC_UNTRUE(_arg, _message)                                                  \
   {                                                                                     \
      if (!_arg) { ctFatalError(-1, _message); }                                         \
   }

#define CT_RETURN_FAIL(_arg)                                                             \
   {                                                                                     \
      ctResults __res = (_arg);                                                          \
      if (__res != CT_SUCCESS) { return __res; }                                         \
   }

#define CT_RETURN_FAIL_CLEAN(_arg, _cleanup)                                             \
   {                                                                                     \
      ctResults __res = (_arg);                                                          \
      if (__res != CT_SUCCESS) {                                                         \
         _cleanup;                                                                       \
         return __res;                                                                   \
      }                                                                                  \
   }

#define CT_RETURN_ON_FAIL(_arg, _code)                                                   \
   if ((_arg) != CT_SUCCESS) { return _code; }

#define CT_RETURN_ON_UNTRUE(_arg, _code)                                                 \
   if (!(_arg)) { return _code; }

#define CT_RETURN_ON_NULL(_arg, _code) CT_RETURN_ON_UNTRUE(_arg, _code)

#define CT_ALIGN(x) alignas(x)

/*C Helpers*/
#define ctCStaticArrayLen(_arr) (sizeof(_arr) / sizeof(_arr[0]))
#define ctCStrEql(a, b)         strcmp(a, b) == 0
#define ctCStrNEql(a, b, n)     strncmp(a, b, n) == 0
#define ctCFlagCheck(v, f)      ((v) & (1 << (f)))

/*Assert*/
#define ctAssert(e) assert(e)

#define ctErrorCheck(_msg) (_msg != NE_SUCCESS)

/**
 * @brief should behave just like malloc but with messages to track leaks
 */
CT_API void* ctMalloc(size_t size);
/**
 * @brief should behave just like realloc but with messages to track leaks
 */
CT_API void* ctRealloc(void* old, size_t size);
/**
 * @brief should behave just like free
 */
CT_API void ctFree(void* block);
/**
 * @brief should behave similarly to malloc with alignment but with messages to track
 * leaks
 */
CT_API void* ctAlignedMalloc(size_t size, size_t alignment);
/**
 * @brief should behave similarly to realloc with alignment but with messages to track
 * leaks
 */
CT_API void* ctAlignedRealloc(void* block, size_t size, size_t alignment);
/**
 * @brief should behave similarly to free with alignment
 */
CT_API void ctAlignedFree(void* block);

#ifdef __cplusplus
}
#endif

/*Include common cpp files*/
#ifdef __cplusplus
#include "SharedLogging.h"
#include "DynamicArray.hpp"
#include "HandleManager.hpp"
#include "StaticArray.hpp"
#include "Math.hpp"
#include "Math3d.hpp"
#include "Hash.hpp"
#include "String.hpp"
#include "HashTable.hpp"
#include "Meta.hpp"
#include "Sync.hpp"
#include "JSON.hpp"
#include "Time.hpp"
#include "File.hpp"
#include "Random.hpp"
#endif