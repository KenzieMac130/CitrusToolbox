/*
   Copyright 2023 MacKenzie Strand

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

class CT_API ctAnimSpline {
public:
   ctAnimSpline(uint32_t points);
   ctAnimSpline(const ctAnimSpline& base);
   ctAnimSpline(const struct ctModel& model, int32_t splineIndex);
   ~ctAnimSpline();

   inline float GetLength() {
      return cachedLength;
   }
   inline int32_t GetPointCount() {
      return pointCount;
   };
   inline bool isCyclic() {
      return cyclic;
   }
   inline bool isCubic() {
      return cubic;
   }

   void EvaluateAtFactor(float factor, ctVec3& position, ctVec3& normal, ctVec3& tangent);
   void EvaluateAtPoint(int32_t point, ctVec3& position, ctVec3& normal, ctVec3& tangent);
   inline void
   EvaluateAtLength(float length, ctVec3& position, ctVec3& normal, ctVec3& tangent) {
      EvaluateAtFactor(LengthToFactor(length), position, normal, tangent);
   }

protected:
   inline int32_t WrapPointNumber(int32_t ptnum) {
      if (cyclic) {
         ptnum = ptnum % pointCount;
         ptnum = ptnum < 0 ? pointCount - ptnum : ptnum;
      } else {
         ptnum = ptnum > 0 ? ptnum : 0;
         ptnum = ptnum < pointCount ? ptnum : pointCount - 1;
      }
      return ptnum;
   }
   float LengthToFactor(float length);
   inline int32_t FactorToIndex(float factor) {
      return WrapPointNumber((int)(ctFloor((float)pointCount * factor)));
   }
   void CalculateLength();

   size_t allocSize;
   void* pAllocation;

   float cachedLength;
   bool cyclic;
   bool cubic;
   int32_t pointCount;
   float* pPointLengths;
   ctVec3* pPositions;
   ctVec3* pNormals;
   ctVec3* pTangents;
};