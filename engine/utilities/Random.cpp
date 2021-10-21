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

#include "Random.hpp"

#define RND_IMPLEMENTATION
#include "mattias/rnd.h"

ctRandomGenerator::ctRandomGenerator() {
   SetSeed(0);
}

ctRandomGenerator::ctRandomGenerator(uint32_t seed) {
   SetSeed(seed);
}

void ctRandomGenerator::SetSeed(uint32_t seed) {
   return rnd_pcg_seed(&rnd, seed);
}

float ctRandomGenerator::GetFloatUNorm() {
   return rnd_pcg_nextf(&rnd);
}

float ctRandomGenerator::GetFloat() {
   return GetFloatUNorm() * 2.0f - 1.0f;
}

float ctRandomGenerator::GetFloat(float min, float max) {
   return ctRemap(GetFloatUNorm(), 0, 1, min, max);
}

int32_t ctRandomGenerator::GetInt() {
   return rnd_pcg_next(&rnd);
}

int32_t ctRandomGenerator::GetInt(int32_t min, int32_t max) {
   return rnd_pcg_range(&rnd, min, max);
}

ctVec2 ctRandomGenerator::GetVec2() {
   return ctVec2(GetFloat(), GetFloat());
}

ctVec3 ctRandomGenerator::GetVec3() {
   return ctVec3(GetFloat(), GetFloat(), GetFloat());
}

ctVec4 ctRandomGenerator::GetVec4() {
   return ctVec4(GetFloat(), GetFloat(), GetFloat(), GetFloat());
}

ctVec2 ctRandomGenerator::GetVec2(ctVec2 min, ctVec2 max) {
   return ctVec2(GetFloat(min.x, max.x), GetFloat(min.y, max.y));
}

ctVec3 ctRandomGenerator::GetVec3(ctVec3 min, ctVec3 max) {
   return ctVec3(GetFloat(min.x, max.x), GetFloat(min.y, max.y), GetFloat(min.z, max.z));
}

ctVec4 ctRandomGenerator::GetVec4(ctVec4 min, ctVec4 max) {
   return ctVec4(GetFloat(min.x, max.x),
                 GetFloat(min.y, max.y),
                 GetFloat(min.z, max.z),
                 GetFloat(min.w, max.w));
}

/* https://stackoverflow.com/questions/5825680/code-to-generate-gaussian-normally-distributed-random-numbers-in-ruby
 */
float ctRandomGenerator::GetGaussian(float mean, float standardDeviation) {
   const float theta = 2.0f * CT_PI * GetFloatUNorm();
   const float rho = ctSqrt(-2.0f * ctLog(1.0f - (GetFloatUNorm() + FLT_EPSILON)));
   const float scale = standardDeviation * rho;
   return mean + scale * ctCos(theta);
}

ctVec2 ctRandomGenerator::GetGaussian2D(ctVec2 mean, float standardDeviation) {
   const float theta = 2.0f * CT_PI * GetFloatUNorm();
   const float rho = ctSqrt(-2.0f * ctLog(1.0f - (GetFloatUNorm() + FLT_EPSILON)));
   const float scale = standardDeviation * rho;
   return ctVec2(mean.x + scale * ctCos(theta), mean.y + scale * ctSin(theta));
}

ctVec3 ctRandomGenerator::GetOnSphere(float radius) {
   return normalize(ctVec3(
            GetGaussian(0.0f, 1.0f), GetGaussian(0.0f, 1.0f), GetGaussian(0.0f, 1.0f))) *
          radius;
}

ctVec4 ctRandomGenerator::GetColor(float alpha) {
   return ctVec4(GetFloatUNorm(), GetFloatUNorm(), GetFloatUNorm(), alpha);
}

/* Not guaranteed to be threadsafe
Windows: Multithreaded CRT is safe
POSIX: Use rand_r() for safety */
int32_t ctRand() {
#if _POSIX_C_SOURCE >= 1 || _XOPEN_SOURCE || _POSIX_SOURCE
   return rand_r();
#else
   return rand();
#endif
}
