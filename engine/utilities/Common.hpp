#pragma once

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

/*Assert*/
#define CT_ASSERT(e) assert(e)

/*Namespace*/
#define CT_UTILITIES_NAMESPACE Ct
#define CT_MODULE_NAMESPACE CitrusToolbox

namespace CT_UTILITIES_NAMESPACE {

/*Errors*/
enum class Results {
   SUCCESS = 0,
   FAILURE_UNKNOWN = -1,
   FAILURE_OUT_OF_MEMORY = -2,
   FAILURE_INVALID_PARAMETER = -3,
   FAILURE_UNSUPPORTED_HARDWARE = -4,
   FAILURE_UNKNOWN_FORMAT = -5,
   FAILURE_OUT_OF_BOUNDS = -6,
   FAILURE_PARSE_ERROR = -7,
   FAILURE_DECOMPRESSION_ERROR = -8,
   FAILURE_FILE_NOT_FOUND = -9,
   FAILURE_FILE_INACCESSIBLE = -10,
   FAILURE_DATA_DOES_NOT_EXIST = -11,
   FAILURE_DUPLICATE_ENTRY = -12,
   FAILURE_NOT_UPDATABLE = -13,
   FAILURE_COULD_NOT_SHRINK = -14
};

#define CT_ERROR_CHECK(_process) _process != NE_SUCCESS

/**
 * @brief should behave just like malloc but with messages to track leaks
 */
#define CT_Malloc(_size, _label) malloc(_size)
/**
 * @brief should behave just like malloc but with messages to track leaks
 */
#define CT_Calloc(_count, _size, _label) calloc(_count, _size)
/**
 * @brief should behave just like realloc but with messages to track leaks
 */
#define CT_Realloc(_block, _size, _label) realloc(_block, _size)
/**
 * @brief should behave just like free
 */
#define CT_Free(_block) free(_block)
}

/*Include common files*/
#include "Math.hpp"
#include "Hash.hpp"
#include "DynamicArray.hpp"
#include "String.hpp"
#include "HashTable.hpp"
#include "Vector.hpp"
#include "AABB.hpp"
#include "Quaternion.hpp"
#include "Color.hpp"
#include "Matrix.hpp"