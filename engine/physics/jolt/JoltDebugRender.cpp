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

#include "JoltDebugRender.hpp"

#ifdef JPH_DEBUG_RENDERER
void CitrusJoltDebugRenderer::ClearLast() {
   ctSpinLockEnterCriticalScoped(L1, immediateLinesLock);
   ctSpinLockEnterCriticalScoped(L2, immediateTrisLock);
   ctSpinLockEnterCriticalScoped(L3, immediateTextLock);
   ctSpinLockEnterCriticalScoped(L4, renderBatchesLock);
   immediateText.Clear();
   immediateLines.Clear();
   immediateTris.Clear();
   textBuffer.Clear();
   renderBatches.Clear();
}
void CitrusJoltDebugRenderer::Render() {
   ctSpinLockEnterCritical(immediateLinesLock);
   if (!immediateLines.isEmpty()) {
      Im3d::BeginLines();
      for (size_t i = 0; i < immediateLines.Count(); i++) {
         Im3d::Vertex(ctVec3ToIm3d(immediateLines[i].a),
                      1.0f,
                      ctVec4ToIm3dColor(immediateLines[i].color));
         Im3d::Vertex(ctVec3ToIm3d(immediateLines[i].b),
                      1.0f,
                      ctVec4ToIm3dColor(immediateLines[i].color));
      }
      Im3d::End();
   }
   ctSpinLockExitCritical(immediateLinesLock);

   ctSpinLockEnterCritical(immediateTrisLock);
   if (!immediateTris.isEmpty()) {
      Im3d::BeginTriangles();
      for (size_t i = 0; i < immediateTris.Count(); i++) {
         Im3d::Vertex(ctVec3ToIm3d(immediateTris[i].a),
                      1.0f,
                      ctVec4ToIm3dColor(immediateTris[i].color));
         Im3d::Vertex(ctVec3ToIm3d(immediateTris[i].b),
                      1.0f,
                      ctVec4ToIm3dColor(immediateTris[i].color));
         Im3d::Vertex(ctVec3ToIm3d(immediateTris[i].c),
                      1.0f,
                      ctVec4ToIm3dColor(immediateTris[i].color));
      }
      Im3d::End();
   }
   ctSpinLockExitCritical(immediateTrisLock);

   ctSpinLockEnterCritical(immediateTextLock);
   if (!immediateText.isEmpty()) {
      for (size_t i = 0; i < immediateText.Count(); i++) {
         Im3d::Text(ctVec3ToIm3d(immediateText[i].position),
                    immediateText[i].fontSize,
                    ctVec4ToIm3dColor(immediateText[i].color),
                    Im3d::TextFlags_Default,
                    &textBuffer[immediateText[i].textOffset]);
      }
   }
   ctSpinLockExitCritical(immediateTextLock);

   ctSpinLockEnterCritical(renderBatchesLock);
   if (!renderBatches.isEmpty()) {
      for (size_t i = 0; i < renderBatches.Count(); i++) {
         renderBatches[i].Draw();
      }
   }
   ctSpinLockExitCritical(renderBatchesLock);

   ClearLast();
}

const ctVec3 gLightDir = normalize(ctVec3(0.5f, 0.5f, -0.5f));

inline ctVec4 CitrusJoltDebugRenderer::DebugBatch::ShadeVertex(ctVec4 color,
                                                               ctVec3 normal) {
   const float lighting =
     ctClamp(dot((normal * mnotranslate), gLightDir) * 0.5f + 0.5f, 0.0f, 1.0f);
   return ctVec4(color.r * lighting, color.g * lighting, color.b * lighting, color.a);
}

inline void CitrusJoltDebugRenderer::DebugBatch::DrawVertex(size_t idx) {
   const DebugVertex vertex = pGeometry->verts[idx];
   const ctVec4 outcolor = ShadeVertex(vertex.color * color, vertex.normal);
   Im3d::Vertex(ctVec3ToIm3d(vertex.position), 1.0, ctVec4ToIm3dColor(outcolor));
}

void CitrusJoltDebugRenderer::DebugBatch::Draw() {
   Im3d::PushMatrix(ctMat4ToIm3d(matrix));
   if (!wireframe) { /* solid */
      Im3d::BeginTriangles();
      if (pGeometry->indices.isEmpty()) { /* unindexed */
         for (size_t i = 0; i < pGeometry->verts.Count(); i++) {
            DrawVertex(i);
         }
      } else { /* indexed */
         for (size_t i = 0; i < pGeometry->indices.Count(); i++) {
            uint32_t idx = pGeometry->indices[i];
            DrawVertex(idx);
         }
      }
   } else { /* wireframe */
      Im3d::BeginLines();
      if (pGeometry->indices.isEmpty()) { /* unindexed */
         for (size_t i = 0; i < pGeometry->verts.Count() / 3; i++) {
            for (int j = 0; j < 3; j++) {
               size_t idx = i * 3 + (j % 3);
               DrawVertex(idx);
            }
         }
      } else { /* indexed */
         for (size_t i = 0; i < pGeometry->indices.Count() / 3; i++) {
            for (int j = 0; j < 3; j++) {
               uint32_t idx = pGeometry->indices[i * 3 + (j % 3)];
               DrawVertex(idx);
            }
         }
      }
   }
   Im3d::End();
   Im3d::PopMatrix();
}

void CitrusJoltDebugRenderer::DrawLine(JPH::RVec3Arg inFrom,
                                       JPH::RVec3Arg inTo,
                                       JPH::ColorArg inColor) {
   ctSpinLockEnterCriticalScoped(LOCK, immediateLinesLock);
   immediateLines.Append(
     {ctVec3FromJolt(inFrom), ctVec3FromJolt(inTo), ctVec4FromJoltColor(inColor)});
}

void CitrusJoltDebugRenderer::DrawTriangle(JPH::RVec3Arg inV1,
                                           JPH::RVec3Arg inV2,
                                           JPH::RVec3Arg inV3,
                                           JPH::ColorArg inColor,
                                           ECastShadow inCastShadow) {
   ctSpinLockEnterCriticalScoped(LOCK, immediateTrisLock);
   immediateTris.Append({ctVec3FromJolt(inV1),
                         ctVec3FromJolt(inV2),
                         ctVec3FromJolt(inV3),
                         ctVec4FromJoltColor(inColor)});
}

void CitrusJoltDebugRenderer::DrawText3D(JPH::RVec3Arg inPosition,
                                         const JPH::string_view& inString,
                                         JPH::ColorArg inColor,
                                         float inHeight) {
   ctSpinLockEnterCriticalScoped(LOCK, immediateTextLock);
   size_t textSize = inString.size();
   size_t textOffset = textBuffer.Count();
   textBuffer.Append(inString.data(), textSize);
   textBuffer.Append('\0');
   immediateText.Append({ctVec3FromJolt(inPosition),
                         ctVec4FromJoltColor(inColor),
                         inHeight,
                         textOffset,
                         textSize});
}

CitrusJoltDebugRenderer::CitrusJoltDebugGeometryBatch::CitrusJoltDebugGeometryBatch(
  const Triangle* inTriangles, int inTriangleCount) {
   refcount = 0;    /* todo: is this right? */
   indices.Clear(); /* no indices = triangle list */
   for (int i = 0; i < inTriangleCount; i++) {
      verts.Append({ctVec3FromJoltF3(inTriangles[i].mV[0].mPosition),
                    ctVec3FromJoltF3(inTriangles[i].mV[0].mNormal),
                    ctVec4FromJoltColor(inTriangles[i].mV[0].mColor)});
      verts.Append({ctVec3FromJoltF3(inTriangles[i].mV[1].mPosition),
                    ctVec3FromJoltF3(inTriangles[i].mV[1].mNormal),
                    ctVec4FromJoltColor(inTriangles[i].mV[1].mColor)});
      verts.Append({ctVec3FromJoltF3(inTriangles[i].mV[2].mPosition),
                    ctVec3FromJoltF3(inTriangles[i].mV[2].mNormal),
                    ctVec4FromJoltColor(inTriangles[i].mV[2].mColor)});
   }
}

CitrusJoltDebugRenderer::CitrusJoltDebugGeometryBatch::CitrusJoltDebugGeometryBatch(
  const Vertex* inVertices,
  int inVertexCount,
  const JPH::uint32* inIndices,
  int inIndexCount) {
   refcount = 0; /* todo: is this right? */
   indices.Append(inIndices, inIndexCount);
   for (int i = 0; i < inVertexCount; i++) {
      verts.Append({ctVec3FromJoltF3(inVertices[i].mPosition),
                    ctVec3FromJoltF3(inVertices[i].mNormal),
                    ctVec4FromJoltColor(inVertices[i].mColor)});
   }
}

JPH::DebugRenderer::Batch
CitrusJoltDebugRenderer::CreateTriangleBatch(const Triangle* inTriangles,
                                             int inTriangleCount) {
   ctSpinLockEnterCriticalScoped(LOCK, renderBatchesLock);
   return new CitrusJoltDebugGeometryBatch(inTriangles, inTriangleCount);
}

JPH::DebugRenderer::Batch
CitrusJoltDebugRenderer::CreateTriangleBatch(const Vertex* inVertices,
                                             int inVertexCount,
                                             const JPH::uint32* inIndices,
                                             int inIndexCount) {
   ctSpinLockEnterCriticalScoped(LOCK, renderBatchesLock);
   return new CitrusJoltDebugGeometryBatch(
     inVertices, inVertexCount, inIndices, inIndexCount);
}

void CitrusJoltDebugRenderer::DrawGeometry(JPH::RMat44Arg inModelMatrix,
                                           const JPH::AABox& inWorldSpaceBounds,
                                           float inLODScaleSq,
                                           JPH::ColorArg inModelColor,
                                           const GeometryRef& inGeometry,
                                           ECullMode inCullMode,
                                           ECastShadow inCastShadow,
                                           EDrawMode inDrawMode) {
   ctSpinLockEnterCriticalScoped(LOCK, renderBatchesLock);
   /* ignore LODS always render last one, immediate rendering is precious */
   auto batch = inGeometry->mLODs.at(inGeometry->mLODs.size() - 1).mTriangleBatch;
   ctMat4 matrix = ctMat4((float*)&inModelMatrix);
   ctMat4 untranslated = matrix;
   ctMat4RemoveTranslation(untranslated);
   renderBatches.Append({matrix,
                         untranslated,
                         (CitrusJoltDebugGeometryBatch*)batch.GetPtr(),
                         inDrawMode == EDrawMode::Wireframe,
                         ctVec4FromJoltColor(inModelColor)});
}
#endif
