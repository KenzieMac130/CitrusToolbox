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

#include "Spline.hpp"
#include "formats/model/Model.hpp"

ctAnimSpline::ctAnimSpline(uint32_t points) {
   pointCount = points;
   cachedLength = 0.0f;
   cyclic = false;
   cubic = true;
   ctGroupAllocDesc groups[] = {
     {1, pointCount * sizeof(pPointLengths[0]), (void**)&pPointLengths},
     {CT_ALIGNMENT_VEC3, pointCount * sizeof(pPositions[0]), (void**)&pPositions},
     {CT_ALIGNMENT_VEC3, pointCount * sizeof(pNormals[0]), (void**)&pNormals},
     {CT_ALIGNMENT_VEC3, pointCount * sizeof(pTangents[0]), (void**)&pTangents}};
   pAllocation = ctGroupAlloc(ctCStaticArrayLen(groups), groups, &allocSize);
}

ctAnimSpline::ctAnimSpline(const ctAnimSpline& base) : ctAnimSpline(base.pointCount) {
   memcpy(pAllocation, base.pAllocation, allocSize);
}

uint32_t GetSplinePointCount(const ctModel& model, int32_t splineIndex) {
   if ((int32_t)model.splines.segmentCount <= splineIndex) { return 0; }
   return model.splines.segments[splineIndex].pointCount;
}

ctAnimSpline::ctAnimSpline(const ctModel& model, int32_t splineIndex) :
    ctAnimSpline(GetSplinePointCount(model, splineIndex)) {
   if ((int32_t)model.splines.segmentCount <= splineIndex) { return; }
   const ctModelSpline& segment = model.splines.segments[splineIndex];
   cyclic = ctCFlagCheck(segment.flags, CT_MODEL_SPLINE_CYCLIC);
   cubic = ctCFlagCheck(segment.flags, CT_MODEL_SPLINE_INTERPOLATE_CUBIC);
   memcpy(pPositions,
          &model.splines.positions[segment.pointOffset],
          sizeof(pPositions[0]) * segment.pointCount);
   memcpy(pNormals,
          &model.splines.normals[segment.pointOffset],
          sizeof(pNormals[0]) * segment.pointCount);
   memcpy(pTangents,
          &model.splines.tangents[segment.pointOffset],
          sizeof(pTangents[0]) * segment.pointCount);
   CalculateLength();
}

ctAnimSpline::~ctAnimSpline() {
   ctFree(pAllocation);
}

void ctAnimSpline::EvaluateAtFactor(float factor,
                                    ctVec3& position,
                                    ctVec3& normal,
                                    ctVec3& tangent) {
   // if (cubic) {
   //   // todo
   //} else { /* linear */
   int32_t idx0 = FactorToIndex(factor);
   int32_t idx1 = idx0 + 1;
   ctVec3 p0, p1;
   ctVec3 n0, n1;
   ctVec3 t0, t1;
   float f = ctFrac(pointCount * factor);
   EvaluateAtPoint(idx0, p0, n0, t0);
   EvaluateAtPoint(idx1, p1, n1, t1);
   position = lerp(p0, p1, f);
   normal = lerp(n0, n1, f);
   tangent = lerp(t0, t1, f);
   //}
}

void ctAnimSpline::EvaluateAtPoint(int32_t point,
                                   ctVec3& position,
                                   ctVec3& normal,
                                   ctVec3& tangent) {
   point = WrapPointNumber(point);
   position = pPositions[point];
   normal = pNormals[point];
   tangent = pTangents[point];
}

float ctAnimSpline::LengthToFactor(float length) {
   /* the same as GetScalarGroupAtTime in Bank.cpp */
   int32_t rightKey = pointCount - 1;
   int32_t leftKey = rightKey;
   float leftLength = 0.0f;
   float rightLength = 0.0f;
   for (int32_t i = 0; i < pointCount; i++) {
      if (pPointLengths[i] >= length) {
         rightKey = i;
         rightLength = pPointLengths[rightKey];
         if (i == 0) {
            leftKey = rightKey;
         } else {
            leftKey = rightKey - 1;
         }
         leftLength = pPointLengths[leftKey];
         break;
      }
   }
   const float fractional =
     ctClamp(
       (length - leftLength) / ((rightLength + FLT_EPSILON) - leftLength), 0.0f, 1.0f) /
     pointCount;
   const float major = (float)leftKey / pointCount;
   return major + fractional;
}

void ctAnimSpline::CalculateLength() {
   float runningLength = 0.0f;
   ctVec3 lastVertex = pPositions[0];
   /* todo: investigate accuracy due to cubic interpolation */
   for (int32_t i = 0; i < pointCount; i++) {
      ctVec3 curVertex = pPositions[i];
      runningLength += distance(lastVertex, curVertex);
      lastVertex = curVertex;
      pPointLengths[i] = runningLength;
   }
   cachedLength = runningLength;
}
