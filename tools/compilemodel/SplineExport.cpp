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

#include "../engine/utilities/Common.h"
#include "CitrusModel.hpp"

#define CONTINUE_FAIL(_CONTENTS)                                                         \
   {                                                                                     \
      ctResults _res = _CONTENTS;                                                        \
      if (_res != CT_SUCCESS) { continue; }                                              \
   }

ctResults ctGltf2Model::ExtractSplines() {
   ctDebugLog("Extracting Splines...");
   for (size_t i = 0; i < gltf.nodes_count; i++) {
      const cgltf_node& node = gltf.nodes[i];
      if (!isNodeSpline(node.name)) { continue; }
      /* find extension */
      for (size_t extIdx = 0; extIdx < node.extensions_count; extIdx++) {
         cgltf_extension* ext = &node.extensions[extIdx];
         if (ctCStrEql(ext->name, "CITRUS_node_spline")) {
            ctJSONReader json = ctJSONReader();
            json.BuildJsonForPtr(ext->data, strlen(ext->data));

            /* get root objects */
            ctJSONReadEntry jroot;
            ctJSONReadEntry jtype;
            ctJSONReadEntry jcyclic;
            ctJSONReadEntry jattributes;
            CONTINUE_FAIL(json.GetRootEntry(jroot));
            CONTINUE_FAIL(jroot.GetObjectEntry("type", jtype));
            CONTINUE_FAIL(jroot.GetObjectEntry("cyclic", jcyclic));
            CONTINUE_FAIL(jroot.GetObjectEntry("attributes", jattributes));

            /* get flags */
            uint32_t flags = 0;
            ctStringUtf8 typeStr = "";
            ctStringUtf8 attribStr = "";
            bool isCyclic = false;
            jtype.GetString(typeStr);
            jattributes.GetString(attribStr);
            CONTINUE_FAIL(jcyclic.GetBool(isCyclic));
            if (isCyclic) { flags |= CT_MODEL_SPLINE_CYCLIC; }
            if (typeStr == "cubic") { flags |= CT_MODEL_SPLINE_INTERPOLATE_CUBIC; }

            /* get accessors */
            ctJSONReadEntry jposition;
            ctJSONReadEntry jnormal;
            ctJSONReadEntry jtangent;
            int accessorPosIdx = 0;
            int accessorNormalIdx = 0;
            int accessorTangentIdx = 0;
            CONTINUE_FAIL(jattributes.GetObjectEntry("POSITION", jposition));
            CONTINUE_FAIL(jattributes.GetObjectEntry("NORMAL", jnormal));
            CONTINUE_FAIL(jattributes.GetObjectEntry("TANGENT", jtangent));
            CONTINUE_FAIL(jposition.GetNumber(accessorPosIdx));
            CONTINUE_FAIL(jnormal.GetNumber(accessorNormalIdx));
            CONTINUE_FAIL(jtangent.GetNumber(accessorTangentIdx));
            const cgltf_accessor& accessorPos = gltf.accessors[accessorPosIdx];
            const cgltf_accessor& accessorNormal = gltf.accessors[accessorNormalIdx];
            const cgltf_accessor& accessorTangent = gltf.accessors[accessorTangentIdx];
            size_t count = accessorPos.count;
            size_t offset = splinePositions.Count();
            splinePositions.Resize(offset + count);
            splineNormals.Resize(offset + count);
            splineTangents.Resize(offset + count);
            CopyAccessorToReserve(accessorPos,
                                  (uint8_t*)&splinePositions[offset],
                                  TinyImageFormat_R32G32B32_SFLOAT,
                                  sizeof(ctVec3),
                                  0);
            CopyAccessorToReserve(accessorNormal,
                                  (uint8_t*)&splineNormals[offset],
                                  TinyImageFormat_R32G32B32_SFLOAT,
                                  sizeof(ctVec3),
                                  0);
            CopyAccessorToReserve(accessorTangent,
                                  (uint8_t*)&splineTangents[offset],
                                  TinyImageFormat_R32G32B32_SFLOAT,
                                  sizeof(ctVec3),
                                  0);

            /* add spline */
            ctModelSpline spline = ctModelSpline();
            spline.flags = flags;
            spline.pointCount = (uint32_t)count;
            spline.pointOffset = (uint32_t)offset;
            splines.Append(spline);
         }

         /* ensure normalized normals/tangents */
         for (size_t i = 0; i < splinePositions.Count(); i++) {
             splineNormals[i] = normalize(splineNormals[i]);
             splineTangents[i] = normalize(splineTangents[i]);
         }
      }
   }
   if (splines.isEmpty()) { return CT_SUCCESS; }

   model.splines.segmentCount = (uint32_t)splines.Count();
   model.splines.pointCount = (uint32_t)splinePositions.Count();

   model.splines.segments = splines.Data();
   model.splines.positions = splinePositions.Data();
   model.splines.normals = splineNormals.Data();
   model.splines.tangents = splineTangents.Data();
   return CT_SUCCESS;
}