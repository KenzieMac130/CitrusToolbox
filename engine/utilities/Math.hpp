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

#define ctPi  3.141592653589793238462643383279
#define ctPiF 3.141592653589793238462643383279f

/* Float Compare */
bool ctFloatCompare(const float a,
                    const float b,
                    const float threshold = 0.001f);

/* Lerp */
float ctLerp(const float a, const float b, const float factor);

/* Smoothstep */
float ctSmoothStep(const float a, const float b, const float factor);

/* Power */
float ctPow(const float a, const float b);

/* Log */
float ctLog(const float a);

/* Sqrt */
float ctSqrt(const float a);

/* Abs */
float ctAbs(const float a);

/* Exp */
float ctExp(const float a);

/* Max */
float ctMax(const float a, const float b);

/* Min */
float ctMin(const float a, const float b);

/* Clamp */
float ctClamp(const float a, const float min, const float max);

/* Saturate */
float ctSaturate(const float a);

/* Floor */
float ctFloor(const float a);

/* Ceil */
float ctCeil(const float a);

/* Round */
float ctRound(const float a);

/* Frac */
float ctFrac(const float a);

/* RadiansToDegrees */
float ctRad2Deg(const float a);

/* DegreesToRadians */
float ctDeg2Rad(const float a);

/* Sin */
float ctSin(const float a);

/* Cos */
float ctCos(const float a);

/* Tan */
float ctTan(const float a);

/* ArcSin */
float ctArcSin(const float a);

/* ArcCos */
float ctArcCos(const float a);

/* ArcTan */
float ctArcTan(const float a);

/* ArcTan2 */
float ctArcTan2(const float a, const float b);