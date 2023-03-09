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
#include "cgltf/cgltf.h"
#include "formats/model/Model.hpp"

struct ctModelExportContext {
   bool singleBone;
   bool singleMesh;

   class ctModelExportSkeleton* pSkeletonExport;
   class ctModelExportMesh* pMeshExport;
};

class CT_API ctModelExportPhaseBase {
public:
   virtual ctResults
   Export(const cgltf_data& input, ctModel& output, ctModelExportContext& ctx) = 0;

   static inline bool isNodeCollision(const char* name) {
      size_t len = strlen(name);
      if (len < 4) { return false; }
      const char* postfix = &name[len - 4];
      if (ctCStrNEql(postfix, "_CBX", 4) || /* box collision */
          ctCStrNEql(postfix, "_CSP", 4) || /* sphere collision */
          ctCStrNEql(postfix, "_CPL", 4) || /* pill collision */
          ctCStrNEql(postfix, "_CTR", 4) || /* triangle collision */
          ctCStrNEql(postfix, "_CVX", 4))   /* convex collision */
      {
         return true;
      }
      return false;
   }

   static inline bool isNodeLod(const char* name) {
      size_t len = strlen(name);
      if (len < 5) { return false; }
      const char* postfix = &name[len - 4];
      if (ctCStrNEql(postfix, "LOD", 3)) { /* lod levels LOD1-LOD9 */
         return true;
      }
      return false;
   }
};