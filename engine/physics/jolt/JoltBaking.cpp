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
#include "JoltBaking.hpp"

void WriteShape(const JPH::Shape* pShape, CitrusJoltStreamOut& stream) {
   ctDynamicArray<CitrusJoltBakeSection> sections;
   sections.Reserve(64);

   /* seek forward into the header*/
   size_t headerOffset = stream.Tell();
   CitrusJoltBakeHeader dummyHeader;
   stream.Write(dummyHeader);

   /* write base shape */
   size_t baseShapeOffset = stream.Tell();
   pShape->SaveBinaryState(stream);
   size_t baseShapeSize = stream.Tell() - baseShapeOffset;
   sections.Append({baseShapeOffset, baseShapeSize});

   /* write material hash section */
   size_t materialSectionOffset = stream.Tell();
   JPH::PhysicsMaterialList matlist;
   pShape->SaveMaterialState(matlist);
   for (size_t i = 0; i < matlist.size(); i++) {
      uint32_t hash = 0;
      if (matlist[i]) { hash = ctXXHash32(matlist[i].GetPtr()->GetDebugName()); }
      stream.WriteBytes(&hash, sizeof(hash));
   }
   size_t materialSectionSize = stream.Tell() - materialSectionOffset;
   sections.Append({materialSectionOffset, materialSectionSize});

   /* write subshapes */
   JPH::ShapeList subshapes;
   pShape->SaveSubShapeState(subshapes);
   for (size_t i = 0; i < subshapes.size(); i++) {
      size_t subshapeSectionOffset = stream.Tell();
      WriteShape(subshapes[i].GetPtr(), stream);
      size_t subshapeSectionSize = stream.Tell() - subshapeSectionOffset;
      sections.Append({subshapeSectionOffset, subshapeSectionSize});
   }

   /* write section list */
   size_t sectionsOffset = stream.Tell();
   stream.WriteBytes(sections.Data(), sizeof(CitrusJoltBakeSection) * sections.Count());

   /* write header */
   CitrusJoltBakeHeader* pHeader = (CitrusJoltBakeHeader*)stream.Ptr(headerOffset);
   pHeader->sectionsCount = sections.Count();
   pHeader->sectionsOffset = sectionsOffset;
}

ctResults ctPhysicsBakeShape(ctPhysicsEngine ctx,
                             ctPhysicsShapeSettings& shape,
                             ctDynamicArray<uint8_t>& output) {
   output.Reserve(250000); /* reserve 0.25mb to avoid possible slowdown */
   JPH::Shape::ShapeResult shaperes = CreateShapeFromCitrus(ctx, shape);
   if (!shaperes.IsValid()) { return CT_FAILURE_INVALID_PARAMETER; }
   CitrusJoltStreamOut stream = CitrusJoltStreamOut(&output);
   WriteShape(shaperes.Get(), stream);
   return CT_SUCCESS;
}

ctResults ctPhysicsBakeConstraint(ctPhysicsEngine ctx,
                                  ctPhysicsConstraintSettings& constraint,
                                  ctDynamicArray<uint8_t>& output) {
   /* todo */
   return CT_SUCCESS;
}

ctResults ctPhysicsShapeToMesh(ctPhysicsEngine ctx,
                               ctPhysicsShapeSettings& shape,
                               ctDynamicArray<ctVec3>& vertices) {
   JPH::Shape::ShapeResult shaperes = CreateShapeFromCitrus(ctx, shape);
   if (!shaperes.IsValid()) { return CT_FAILURE_UNKNOWN; }
   JPH::Shape& jshape = *shaperes.Get();

   /* see the following code for more details, code was mostly taken from this
    * https://github.com/jrouwe/JoltPhysics/blob/577c84920434b41995616acaf5714a2111ebb43d/Samples/SamplesApp.cpp#L2244
    */
   JPH::Shape::GetTrianglesContext trictx;
   jshape.GetTrianglesStart(trictx,
                            JPH::AABox::sBiggest(),
                            JPH::Vec3::sZero(),
                            JPH::Quat::sIdentity(),
                            JPH::Vec3::sReplicate(1.0f));
   for (;;) {
      const int cMaxTriangles = 256;
      JPH::Float3 jverts[3 * cMaxTriangles];
      int triangle_count = jshape.GetTrianglesNext(trictx, cMaxTriangles, jverts);
      if (triangle_count == 0) break;
      vertices.Reserve(vertices.Count() + ((size_t)triangle_count * 3));
      for (int tri = 0; tri < triangle_count; tri++) {
         vertices.Append(ctVec3FromJoltF3((jverts[(tri * 3) + 0])));
         vertices.Append(ctVec3FromJoltF3((jverts[(tri * 3) + 1])));
         vertices.Append(ctVec3FromJoltF3((jverts[(tri * 3) + 2])));
      }
   }
   return CT_SUCCESS;
}
