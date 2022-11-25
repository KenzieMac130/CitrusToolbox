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
#include "stb/stb_perlin.h"

/* Scalar White Noise */
inline float ctNoiseWhite(float factor);
/* White Noise giving a random vector for three dimensions */
inline ctVec3 ctNoiseWhite3D(ctVec3 coord);
/* White Noise giving a random scalar for three dimensions */
inline float ctNoiseWhite3DScalar(ctVec3 coord);
/* Quick single octave of Improved Perlin Noise */
inline float ctNoiseSimplePerlin(ctVec3 coord);
/* Multiple octaves of Improved Perlin Noise summed together */
inline float ctNoiseTurbulence(ctVec3 coord,
                     int32_t octaves = 3,
                     float gain = 0.5f,
                     float lacunarity = 1.97231f);
/* Fractal Bronian Motion Noise */
inline float ctNoiseFBM(ctVec3 coord,
                        int32_t octaves = 6,
                        float gain = 0.5f,
                        float lacunarity = 1.97231f);
/* Ridge Perlin Noise */
inline float ctNoiseRidgePerlin(ctVec3 coord,
                                int32_t octaves = 6,
                                float offset = 1.0f,
                                float gain = 0.5f,
                                float lacunarity = 1.97231f);
/* Vector Curl Noise for Advecting Particles */
inline ctVec3 ctNoiseCurl(ctVec3 coord,
                          float phase = 0.0f,
                          ctVec3 offsets = ctVec3(0.0f, 53.859f, 109.257f));
/* Classic Voronoi Noise */
inline float ctNoiseVorronoi(ctVec3 coord, float minDistanceToCell = 10.0f);
/* Voronoi Noise with a random value each cell */
inline float ctNoiseVorronoiCell(ctVec3 coord, float minDistanceToCell = 10.0f);
/* Voronoi Noise that returns a distance to each cell (rock like) */
inline float ctNoiseVorronoiDistance(ctVec3 coord, float minDistanceToCell = 10.0f);
/* Voronoi Noise that returns all the above (X: Classic, Y: Cell, Z: Distance) */
inline ctVec3 ctNoiseVorronoiRich(ctVec3 coord, float minDistanceToCell = 10.0f);

/* ----------------------------------------------------------------------------- */

inline float ctNoiseSimplePerlin(ctVec3 coord) {
   return stb_perlin_noise3(coord.x, coord.y, coord.z, 0, 0, 0);
}

inline float ctNoiseTurbulence(ctVec3 coord, int32_t octaves, float gain, float lacunarity) {
   return stb_perlin_turbulence_noise3(
     coord.x, coord.y, coord.z, lacunarity, gain, octaves);
}

inline float ctNoiseFBM(ctVec3 coord, int32_t octaves, float gain, float lacunarity) {
   return stb_perlin_fbm_noise3(coord.x, coord.y, coord.z, lacunarity, gain, octaves);
}

inline float ctNoiseRidgePerlin(
  ctVec3 coord, int32_t octaves, float offset, float gain, float lacunarity) {
   return stb_perlin_ridge_noise3(
     coord.x, coord.y, coord.z, lacunarity, gain, offset, octaves);
}

inline ctVec3 ctNoiseCurl(ctVec3 coord, float phase, ctVec3 offsets) {
   const ctVec3 shiftDirection = ctVec3(0.0f, 0.0f, 1.0f);
   return (ctVec3(ctNoiseSimplePerlin(coord + (shiftDirection * phase + offsets.x)),
                  ctNoiseSimplePerlin(coord + (shiftDirection * phase + offsets.y)),
                  ctNoiseSimplePerlin(coord + (shiftDirection * phase + offsets.z))));
}

/* https://www.ronja-tutorials.com/post/024-white-noise/ */

inline float _ctNoiseRand1DTo1D(float value, float mutator = 0.546f) {
   return ctFrac(ctSin(value + mutator) * 143758.5453f);
}

inline float _ctNoiseRand3DTo1D(ctVec3 value,
                                ctVec3 dotDir = ctVec3(12.9898f, 78.233f, 37.719f)) {
   ctVec3 smallValue = ctVec3(ctSin(value.x), ctSin(value.y), ctSin(value.z));
   float random = dot(smallValue, dotDir);
   return ctFrac(ctSin(random) * 143758.5453f);
}

inline ctVec3 _ctNoiseRand3DTo3D(ctVec3 value) {
   return ctVec3(_ctNoiseRand3DTo1D(value, ctVec3(12.989f, 78.233f, 37.719f)),
                 _ctNoiseRand3DTo1D(value, ctVec3(39.346f, 11.135f, 83.155f)),
                 _ctNoiseRand3DTo1D(value, ctVec3(73.156f, 52.235f, 09.151f)));
}

/* https://www.ronja-tutorials.com/post/028-voronoi-noise/ */

inline ctVec3 ctNoiseVorronoiRich(ctVec3 coord, float minDistanceToCell) {
   float minEdgeDistance = minDistanceToCell;
   ctVec3 baseCell = ctVec3(ctFloor(coord.x), ctFloor(coord.y), ctFloor(coord.z));
   ctVec3 closestCell;
   ctVec3 toClosestCell;
   for (int32_t x = -1; x <= 1; x++) {
      for (int32_t y = -1; y <= 1; y++) {
         for (int32_t z = -1; z <= 1; z++) {
            ctVec3 cell = baseCell + ctVec3((float)x, (float)y, (float)z);
            ctVec3 cellPosition = cell + _ctNoiseRand3DTo3D(cell);
            ctVec3 toCell = cellPosition - coord;
            float distToCell = length(toCell);
            if (distToCell < minDistanceToCell) {
               minDistanceToCell = distToCell;
               closestCell = cell;
               toClosestCell = toCell;
            }
         }
      }
   }

   for (int32_t x = -1; x <= 1; x++) {
      for (int32_t y = -1; y <= 1; y++) {
         for (int32_t z = -1; z <= 1; z++) {
            ctVec3 cell = baseCell + ctVec3((float)x, (float)y, (float)z);
            ctVec3 cellPosition = cell + _ctNoiseRand3DTo3D(cell);
            ctVec3 toCell = cellPosition - coord;

            ctVec3 subClosestCell = closestCell - cell;
            ctVec3 diffToClosestCell = ctVec3(
              ctAbs(subClosestCell.x), ctAbs(subClosestCell.y), ctAbs(subClosestCell.z));
            if (diffToClosestCell.x + diffToClosestCell.y + diffToClosestCell.z >= 0.1f) {
               ctVec3 toCenter = (toClosestCell + toCell) * 0.5;
               ctVec3 cellDifference = normalize(toCell - toClosestCell);
               float edgeDistance = dot(toCenter, cellDifference);
               minEdgeDistance = ctMin(minEdgeDistance, edgeDistance);
            }
         }
      }
   }

   return ctVec3(minDistanceToCell, _ctNoiseRand3DTo1D(closestCell), minEdgeDistance);
}

inline float ctNoiseVorronoi(ctVec3 coord, float minDistanceToCell) {
   ctNoiseVorronoiRich(coord, minDistanceToCell).x;
}

inline float ctNoiseVorronoiCell(ctVec3 coord, float minDistanceToCell) {
   ctNoiseVorronoiRich(coord, minDistanceToCell).y;
}

inline float ctNoiseVorronoiDistance(ctVec3 coord, float minDistanceToCell) {
   ctNoiseVorronoiRich(coord, minDistanceToCell).z;
}

inline float ctNoiseWhite(float factor) {
   return _ctNoiseRand1DTo1D(factor);
}

inline ctVec3 ctNoiseWhite3D(ctVec3 coord) {
   return _ctNoiseRand3DTo3D(coord);
}

inline float ctNoiseWhite3DScalar(ctVec3 coord) {
   return _ctNoiseRand3DTo1D(_ctNoiseRand3DTo3D(coord));
}