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
#include "JoltContext.hpp"

#ifdef JPH_DEBUG_RENDERER
#include "Jolt/Renderer/DebugRenderer.h"
#include "im3d/im3d.h"

class CitrusJoltDebugRenderer : public JPH::DebugRenderer {
public:
   inline CitrusJoltDebugRenderer() {
      ctSpinLockInit(immediateLinesLock);
      ctSpinLockInit(immediateTrisLock);
      ctSpinLockInit(immediateTextLock);
      ctSpinLockInit(renderBatchesLock);
      Initialize();
   }

   void ClearLast();
   void Render();

   void DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor);
   void DrawTriangle(JPH::RVec3Arg inV1,
                     JPH::RVec3Arg inV2,
                     JPH::RVec3Arg inV3,
                     JPH::ColorArg inColor,
                     ECastShadow inCastShadow = ECastShadow::Off);
   JPH::DebugRenderer::Batch CreateTriangleBatch(const Triangle* inTriangles,
                                                 int inTriangleCount);
   JPH::DebugRenderer::Batch CreateTriangleBatch(const Vertex* inVertices,
                                                 int inVertexCount,
                                                 const JPH::uint32* inIndices,
                                                 int inIndexCount);
   void DrawGeometry(JPH::RMat44Arg inModelMatrix,
                     const JPH::AABox& inWorldSpaceBounds,
                     float inLODScaleSq,
                     JPH::ColorArg inModelColor,
                     const GeometryRef& inGeometry,
                     ECullMode inCullMode = ECullMode::CullBackFace,
                     ECastShadow inCastShadow = ECastShadow::On,
                     EDrawMode inDrawMode = EDrawMode::Solid);
   void DrawText3D(JPH::RVec3Arg inPosition,
                   const JPH::string_view& inString,
                   JPH::ColorArg inColor = JPH::Color::sWhite,
                   float inHeight = 0.5f);

private:
   struct DebugLine {
      ctVec3 a;
      ctVec3 b;
      ctVec4 color;
   };
   ctSpinLock immediateLinesLock;
   ctDynamicArray<DebugLine> immediateLines;

   struct DebugTriangle {
      ctVec3 a;
      ctVec3 b;
      ctVec3 c;
      ctVec4 color;
   };
   ctSpinLock immediateTrisLock;
   ctDynamicArray<DebugTriangle> immediateTris;

   struct DebugText {
      ctVec3 position;
      ctVec4 color;
      float fontSize;
      size_t textOffset;
      size_t textSize;
   };
   ctSpinLock immediateTextLock;
   ctDynamicArray<char> textBuffer;
   ctDynamicArray<DebugText> immediateText;

   struct DebugVertex {
      ctVec3 position;
      ctVec3 normal;
      ctVec4 color;
   };

   class CitrusJoltDebugGeometryBatch : public JPH::RefTargetVirtual {
   public:
      CitrusJoltDebugGeometryBatch(const Triangle* inTriangles, int inTriangleCount);
      CitrusJoltDebugGeometryBatch(const Vertex* inVertices,
                                   int inVertexCount,
                                   const JPH::uint32* inIndices,
                                   int inIndexCount);

      virtual void AddRef() override {
         refcount++;
      }
      virtual void Release() override {
         if (--refcount == 0) { delete this; }
      }
      int32_t refcount;
      ctDynamicArray<DebugVertex> verts;
      ctDynamicArray<uint32_t> indices;
   };

   struct DebugBatch {
      ctMat4 matrix;
      ctMat4 mnotranslate;
      CitrusJoltDebugGeometryBatch* pGeometry;
      bool wireframe;
      ctVec4 color;

      inline void DrawVertex(size_t idx);
      inline ctVec4 ShadeVertex(ctVec4 color, ctVec3 normal);
      void Draw();
   };
   ctSpinLock renderBatchesLock;
   ctDynamicArray<DebugBatch> renderBatches;
};
#endif