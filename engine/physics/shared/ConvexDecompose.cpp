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

#include "../Baking.hpp"
#define ENABLE_VHACD_IMPLEMENTATION 1
#include "vhacd/VHACD.h"

class CitrusVHACDLogger : public VHACD::IVHACD::IUserLogger {
   virtual void Log(const char* const msg) {
      ctDebugLog(msg);
   }
};

ctResults ctPhysicsConvexDecomposition::ProcessMesh(ctVec3* pVertices,
                                                    size_t vertexCount,
                                                    uint32_t* pIndices,
                                                    size_t indexCount,
                                                    uint32_t surfaceTypeHash) {
   if (maxHulls > 64) {
      ctDebugError("Max allowed convex hulls is 64!");
      return CT_FAILURE_INVALID_PARAMETER;
   }
   /* decompose with vhacd */
   VHACD::IVHACD::Parameters params = VHACD::IVHACD::Parameters();
   CitrusVHACDLogger logger = CitrusVHACDLogger();
   params.m_maxConvexHulls = maxHulls;
   params.m_resolution = resolution;
   params.m_minimumVolumePercentErrorAllowed = (double)minVolumePercentError;
   params.m_maxRecursionDepth = maxRecursion;
   params.m_shrinkWrap = shrinkWrap;
   params.m_fillMode = (VHACD::FillMode)fill;
   params.m_maxNumVerticesPerCH = maxVertsPerHull;
   params.m_minEdgeLength = minEdgeLength;
   params.m_asyncACD = false;
   params.m_logger = &logger;
   VHACD::IVHACD* vhacd = VHACD::CreateVHACD();
   vhacd->Compute((float*)pVertices,
                  (uint32_t)vertexCount,
                  pIndices,
                  (uint32_t)indexCount / 3,
                  params);
   while (!vhacd->IsReady()) {}; /* should return instantly on non-async */

   /* create points from hull list */
   for (uint32_t h = 0; h < vhacd->GetNConvexHulls(); h++) {
      VHACD::IVHACD::ConvexHull vhull;
      vhacd->GetConvexHull(h, vhull);
      for (size_t i = 0; i < vhull.m_points.size(); i++) {
         ctVec3 point;
         point.x = (float)vhull.m_points[i].mX;
         point.y = (float)vhull.m_points[i].mY;
         point.z = (float)vhull.m_points[i].mZ;
         pointsAll.Append(point);
      }
   }

   /* create hull list */
   size_t pointOffset = 0;
   for (uint32_t h = 0; h < vhacd->GetNConvexHulls(); h++) {
      VHACD::IVHACD::ConvexHull vhull;
      vhacd->GetConvexHull(h, vhull);
      size_t pointCount = vhull.m_points.size();
      subshapes.Append(ctPhysicsShapeConvexHull(
        &pointsAll[pointOffset], (uint32_t)pointCount, surfaceTypeHash));
      pointOffset += pointCount;
   }

   /* create compound shape */
   rootShape = ctPhysicsShapeCompound(subshapes.Count(), subshapes.Data());
   return CT_SUCCESS;
}