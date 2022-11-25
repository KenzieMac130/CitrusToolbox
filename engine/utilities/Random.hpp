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

#include "utilities/Common.h"

#include "mattias/rnd.h"

#define CT_MAX_RAND RAND_MAX
int32_t ctRand();

class ctRandomGenerator {
public:
   ctRandomGenerator();
   ctRandomGenerator(uint32_t seed);

   void SetSeed(uint32_t seed);

   float GetFloatUNorm();
   float GetFloat();
   float GetFloat(float min, float max);

   int32_t GetInt();
   int32_t GetInt(int32_t min, int32_t max);

   ctVec2 GetVec2();
   ctVec3 GetVec3();
   ctVec4 GetVec4();
   ctVec2 GetVec2(ctVec2 min, ctVec2 max);
   ctVec3 GetVec3(ctVec3 min, ctVec3 max);
   ctVec4 GetVec4(ctVec4 min, ctVec4 max);

   float GetGaussian(float mean = 0.0f, float standardDeviation = 1.0f);
   ctVec2 GetGaussian2D(ctVec2 mean = ctVec2(0.0f), float standardDeviation = 1.0f);
   ctVec3 GetInSphere(float radius = 1.0f);
   ctVec3 GetOnSphere(float radius = 1.0f);

   ctVec4 GetColor(float alpha = 1.0f);

private:
   rnd_pcg_t rnd;
};