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

#include "Math.hpp"

bool ctFloatCompare(const float a, const float b, const float threshold) {
   return ctAbs(a - b) < threshold;
}

float ctLerp(const float a, const float b, const float t) {
   return (1 - t) * a + t * b;
}

float ctSmoothStep(const float a, const float b, const float t) {
   const float c = ctSaturate((t - a) / (b - a));
   return c * c * (3.0f - 2.0f * c);
}

float ctPow(const float a, const float b) {
   return powf(a, b);
}

float ctLog(const float a) {
   return logf(a);
}

float ctSqrt(const float a) {
   return sqrtf(a);
}

float ctAbs(const float a) {
   return fabsf(a);
}

float ctExp(const float a) {
   return expf(a);
}

float ctMax(const float a, const float b) {
   return a > b ? a : b;
}

float ctMin(const float a, const float b) {
   return a < b ? a : b;
}

float ctClamp(const float a, const float min, const float max) {
   return ctMin(ctMax(a, min), max);
}

float ctSaturate(const float a) {
   return ctClamp(a, 0.0f, 1.0f);
}

float ctFloor(const float a) {
   return floorf(a);
}

float ctCeil(const float a) {
   return ceilf(a);
}

float ctRound(const float a) {
   return roundf(a);
}

float ctFrac(const float a) {
   return a - ctFloor(a);
}

float ctRad2Deg(const float a) {
   return a * 57.29578f;
}

float ctDeg2Rad(const float a) {
   return a * 0.01745329f;
}

float ctSin(const float a) {
   return sinf(a);
}

float ctCos(const float a) {
   return cosf(a);
}

float ctTan(const float a) {
   return tanf(a);
}

float ctArcSin(const float a) {
   return asinf(a);
}

float ctArcCos(const float a) {
   return acosf(a);
}

float ctArcTan(const float a) {
   return atanf(a);
}

float ctArcTan2(const float a, const float b) {
   return atan2f(a, b);
}

bool ctIsPrime(const size_t x) {
   if (x < 2) { return false; /*actually undefined*/ }
   if (x < 4) { return true; }
   if ((x % 2) == 0) { return false; }
   for (size_t i = 3; i <= (size_t)ctFloor(ctSqrt((float)x)); i += 2) {
      if ((x % i) == 0) { return false; }
   }
   return true;
}

size_t ctNextPrime(size_t x) {
   while (!ctIsPrime(x)) {
      x++;
   }
   return x;
}
