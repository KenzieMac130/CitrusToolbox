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

#include "Common.h"

#define CT_PI  3.141592653589793238462643383279f
#define CT_PI_D 3.141592653589793238462643383279

/* Float Compare */
inline bool ctFloatCompare(const float a,
                    const float b,
                    const float threshold = 0.001f);

/* Lerp */
inline float ctLerp(const float a, const float b, const float factor);

/* Smoothstep */
inline float ctSmoothStep(const float a, const float b, const float factor);

/* Power */
inline float ctPow(const float a, const float b);

/* Log */
inline float ctLog(const float a);

/* Sqrt */
inline float ctSqrt(const float a);

/* Abs */
inline float ctAbs(const float a);

/* Exp */
inline float ctExp(const float a);

/* Max */
inline float ctMax(const float a, const float b);

/* Min */
inline float ctMin(const float a, const float b);

/* Clamp */
inline float ctClamp(const float a, const float min, const float max);

/* Saturate */
inline float ctSaturate(const float a);

/* Floor */
inline float ctFloor(const float a);

/* Ceil */
inline float ctCeil(const float a);

/* Round */
inline float ctRound(const float a);

/* Frac */
inline float ctFrac(const float a);

/* RadiansToDegrees */
inline float ctRad2Deg(const float a);

/* DegreesToRadians */
inline float ctDeg2Rad(const float a);

/* Sin */
inline float ctSin(const float a);

/* Cos */
inline float ctCos(const float a);

/* Tan */
inline float ctTan(const float a);

/* ArcSin */
inline float ctArcSin(const float a);

/* ArcCos */
inline float ctArcCos(const float a);

/* ArcTan */
inline float ctArcTan(const float a);

/* ArcTan2 */
inline float ctArcTan2(const float a, const float b);

/* Prime number */
inline bool ctIsPrime(const size_t x);
inline size_t ctNextPrime(size_t x);

inline bool ctFloatCompare(const float a, const float b, const float threshold) {
    return ctAbs(a - b) < threshold;
}

inline float ctLerp(const float a, const float b, const float t) {
    return (1.0f - t) * a + t * b;
}

inline float ctSmoothStep(const float a, const float b, const float t) {
    const float c = ctSaturate((t - a) / (b - a));
    return c * c * (3.0f - 2.0f * c);
}

inline float ctPow(const float a, const float b) {
    return powf(a, b);
}

inline float ctLog(const float a) {
    return logf(a);
}

inline float ctSqrt(const float a) {
    return sqrtf(a);
}

inline float ctAbs(const float a) {
    return fabsf(a);
}

inline float ctExp(const float a) {
    return expf(a);
}

inline float ctMax(const float a, const float b) {
    return a > b ? a : b;
}

inline float ctMin(const float a, const float b) {
    return a < b ? a : b;
}

inline float ctClamp(const float a, const float min, const float max) {
    return ctMin(ctMax(a, min), max);
}

inline float ctSaturate(const float a) {
    return ctClamp(a, 0.0f, 1.0f);
}

inline float ctFloor(const float a) {
    return floorf(a);
}

inline float ctCeil(const float a) {
    return ceilf(a);
}

inline float ctRound(const float a) {
    return roundf(a);
}

inline float ctFrac(const float a) {
    return a - ctFloor(a);
}

inline float ctRad2Deg(const float a) {
    return a * 57.29578f;
}

inline float ctDeg2Rad(const float a) {
    return a * 0.01745329f;
}

inline float ctSin(const float a) {
    return sinf(a);
}

inline float ctCos(const float a) {
    return cosf(a);
}

inline float ctTan(const float a) {
    return tanf(a);
}

inline float ctArcSin(const float a) {
    return asinf(a);
}

inline float ctArcCos(const float a) {
    return acosf(a);
}

inline float ctArcTan(const float a) {
    return atanf(a);
}

inline float ctArcTan2(const float a, const float b) {
    return atan2f(a, b);
}

inline bool ctIsPrime(const size_t x) {
    if (x < 2) { return false; /*actually undefined*/ }
    if (x < 4) { return true; }
    if ((x % 2) == 0) { return false; }
    for (size_t i = 3; i <= (size_t)ctFloor(ctSqrt((float)x)); i += 2) {
        if ((x % i) == 0) { return false; }
    }
    return true;
}

inline size_t ctNextPrime(size_t x) {
    while (!ctIsPrime(x)) {
        x++;
    }
    return x;
}