#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* Config File */
#include "Config.h"

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

/*Exportable*/
#define CT_EXPORT __declspec(dllexport)

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
   CT_FAILURE_FILE_INACCESSIBLE = -10,
   CT_FAILURE_DATA_DOES_NOT_EXIST = -11,
   CT_FAILURE_DUPLICATE_ENTRY = -12,
   CT_FAILURE_NOT_UPDATABLE = -13,
   CT_FAILURE_COULD_NOT_SHRINK = -14
};

/*Assert*/
#define ctAssert(e) assert(e)

#define ctErrorCheck(_msg) (_msg != NE_SUCCESS)

/**
 * @brief should behave just like malloc but with messages to track leaks
 */
#define ctMalloc(_size, _label) malloc(_size)
/**
 * @brief should behave just like malloc but with messages to track leaks
 */
#define ctCalloc(_count, _size, _label) calloc(_count, _size)
/**
 * @brief should behave just like realloc but with messages to track leaks
 */
#define ctRealloc(_block, _size, _label) realloc(_block, _size)
/**
 * @brief should behave just like free
 */
#define ctFree(_block) free(_block)

/**
 * @debug logging
 */
#define ctDebugLog(_format, ...)                                               \
   printf(_format, __VA_ARGS__);                                               \
   putchar('\n');

#define ctDebugWarning(_format, ...)                                           \
   printf("[WARNING] ");                                                       \
   ctDebugLog(_format, __VA_ARGS__);

#define ctDebugError(_format, ...)                                           \
   printf("[ERROR] ");                                                       \
   ctDebugLog(_format, __VA_ARGS__);

#ifdef __cplusplus
}
#endif

/*Include common cpp files*/
#ifdef __cplusplus
#include "DynamicArray.hpp"
#include "StaticArray.hpp"
#include "Math.hpp"
#include "Hash.hpp"
#include "String.hpp"
#include "HashTable.hpp"
#include "Vector.hpp"
#include "AABB.hpp"
#include "Quaternion.hpp"
#include "Color.hpp"
#include "Matrix.hpp"
#endif