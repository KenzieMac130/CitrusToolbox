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
#include "stb/stb_perlin.h"

inline float ctNoiseImprovedPerlin(ctVec3 coord, int32_t seed = 0);
inline float ctNoiseRigid(ctVec3 coord,
                          int32_t seed = 0,
                          int32_t octaves = 6,
                          float offset = 1.0f,
                          float gain = 0.5f,
                          float lacunarity = 1.97231f);
inline float ctNoiseFBM(ctVec3 coord,
                        int32_t seed = 0,
                        int32_t octaves = 6,
                        float offset = 1.0f,
                        float gain = 0.5f,
                        float lacunarity = 1.97231f);
inline float ctNoiseTurbulence(ctVec3 coord,
                               int32_t octaves = 6,
                               float offset = 1.0f,
                               float gain = 0.5f,
                               float lacunarity = 1.97231f);
inline float ctNoiseVorronoi(ctVec3 coord);

inline float ctNoiseImprovedPerlin(ctVec3 coord, int32_t seed) {
   return 0;
}
inline float ctNoiseRigid(
  ctVec3 uvw, int32_t seed, int32_t octaves, float offset, float gain, float lacunarity) {
   return 0;
}
inline float ctNoiseFBM(
  ctVec3 uvw, int32_t seed, int32_t octaves, float offset, float gain, float lacunarity) {
   return 0;
}
inline float ctNoiseTurbulence(
  ctVec3 uvw, int32_t octaves, float offset, float gain, float lacunarity) {
   return 0;
}
inline float ctNoiseVorronoi(ctVec3 uvw) {
   return 0;
}
