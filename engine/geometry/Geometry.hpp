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

/* For Each Morph target in a Mesh: */
/*    Add the morph target name */
/* For Each pre-defined LOD: */
/*    For Each Submesh: */
/*       1. Import the existing data into the submesh data (ex. material slot) */
/*       1a. If morph targets are used then populate them */
/*       2. Convert the axis from the format's model space using LocalizeAxis() */
/*       3. If the importer uses edge splits/groups/etc add them and apply using ApplySplits() */
/*       4. If the importer uses quads or ngons then call Triangulate() */
/*       5. If the importer doesn't index vertices then use GenerateIndices() */
/*       6. Call OptimizeVertexCache(), OptimizeOverdraw(), and OptimizeVertexFetch() */
/*       7. If splits were used or normals were not provided call GenerateNormals() */
/*       8. Calling GenerateTangents() is recommended in most scenarios even if exported */
/*       9. Generate micromap data if necessary */
/* If LODs are not specified and generation is desired use GenerateLODs() */
/* If the geometry is used with physics then use GenerateCollision() */

class  ctGeometryBakerMorphTarget {
public:
    bool IsValid(class ctGeometrySubmesh& parent);
    
    inline size_t GetTangentCount() {return tangents.Count(); }
    inline size_t GetUVCount() {return uvs.Count(); }
    inline size_t GetColorCount() {return colors.Count(); }

    inline ctDynamicArray<ctVec3>& GetPositions() { return positions; }
    inline ctDynamicArray<ctVec3>& GetNormals() { return normals; }
    inline ctDynamicArray<ctVec4>& GetTangents(size_t index = 0) { return tangents[index]; }
    inline ctDynamicArray<ctVec2>& GetUVs(size_t index = 0) { return uvs[index]; }
    inline ctDynamicArray<ctColorRGBA8>& GetColors(size_t index = 0) { return colors[index]; }

private:
    ctDynamicArray<ctVec3> positions;
    ctDynamicArray<ctVec3> normals;
    ctStaticArray<ctDynamicArray<ctVec4>, 4> tangents;
    ctStaticArray<ctDynamicArray<ctVec2>, 4> uvs;
    ctStaticArray<ctDynamicArray<ctColorRGBA8>, 4> colors;
};

class ctGeometryBakerSubmesh {
public:
    bool isTriangleMesh();
    bool IsValid();

    inline size_t GetTangentCount() {return tangents.Count(); }
    inline size_t GetUVCount() {return uvs.Count(); }
    inline size_t GetColorCount() {return colors.Count(); }

    /* a continuous span of integers with the amount of consecutive indices to use for each face
    if undefined the mesh is assumed to be made of triangles*/
    inline ctDynamicArray<uint32_t>& GetFaceIndexCounts() { return faceIndexCounts; }

    /* indices which map into vertex attributes for each polygon corner
    if undefined we assume each face corner is the next vertex */
    inline ctDynamicArray<uint32_t>& GetIndices() { return indices; }

    /* a per-index attribute which helps generate split normals
    these come recommended with two schemas:
    Smoothing Groups: an integer assinged for each face's indices in the dcc
    Vertex Splits: each smooth vertex is given UINT_MAX and sharp verts are given their index */
    inline ctDynamicArray<uint32_t>& GetIndexSplitGroup() { return indexSplitGroup; }

    /* Vertex model space positions */
    inline ctDynamicArray<ctVec3>& GetPositions() { return positions; }

    /* Vertex model space normals */
    inline ctDynamicArray<ctVec3>& GetNormals() { return normals; }

    /* Model space tangents (per-uv) either import or use GenerateTangents() */
    inline ctDynamicArray<ctVec4>& GetTangents(size_t index = 0) { return tangents[index]; }

    /* Vertex UV Maps */
    inline ctDynamicArray<ctVec2>& GetUVs(size_t index = 0) { return uvs[index]; }

    /* Vertex Colors */
    inline ctDynamicArray<ctColorRGBA8>& GetColors(size_t index = 0) { return colors[index]; }

    /* Bone indices to use for skinning (max of 4, 0 for unbound) */
    inline ctDynamicArray<ctVec4>& GetSkinIndices() { return skinIndices; }

    /* Vertex per-bound-bone skin weights */
    inline ctDynamicArray<ctVec4>& GetSkinWeights() { return skinWeights; }

    /* get the main material index when not using material blending */
    uint32_t GetMainMaterialSlotIndex();

    /* set the main material index when not using material blending */
    void SetMainMaterialSlotIndex(uint32_t idx);

    /* simple primitive generation, see https://prideout.net/shapes */
    ctResults FromParShapes(const struct par_shapes_mesh& parMesh);

    /* convert all coordinate axis into the native coordinate space */
    ctResults LocalizeAxis(ctAxis inputUp, ctAxis inputRight, float inputUnitsInMeter);

    /* apply splits to unique geometry */
    ctResults ApplySplits();

    /* triangulate all faces (invalidates split data) */
    ctResults Triangulate();

    /* generate indices for an unindexed mesh or duplicate vertex mesh */
    ctResults GenerateIndices(bool weld = true);

    /* optimize vertex cache https://meshoptimizer.org/#vertex-cache-optimization  */
    ctResults OptimizeVertexCache();

    /* optimize overdraw https://meshoptimizer.org/#overdraw-optimization */
    ctResults OptimizeOverdraw(float threshold = 1.05f);

    /* optimize vertex fetch https://meshoptimizer.org/#vertex-fetch-optimization */
    ctResults OptimizeVertexFetch();

    /* generate vertex normals */
    ctResults GenerateNormals();

    /* generate vertex tangents using mikkt-space (first or for every uv channel) */
    ctResults GenerateTangents(bool tangentPerUV = true);

    ctVec4 SampleImageNearest(const ctImage* pImage,
    const ctVec2 uv,
    const size_t uvindex = 0) const;
    bool isImageActiveInBarycentric() const;
private:
    uint32_t materialSlotIndices;
    ctDynamicArray<ctGeometryBakerTriangleMaterialBlend> triangleMaterialBlend;
    ctDynamicArray<uint32_t> faceIndexCounts;
    ctDynamicArray<uint32_t> indices;
    ctDynamicArray<uint32_t> indexSplitGroup;
    ctDynamicArray<ctVec3> positions;
    ctDynamicArray<ctVec3> normals;
    ctStaticArray<ctDynamicArray<ctVec4>, 4> tangents;
    ctStaticArray<ctDynamicArray<ctVec2>, 4> uvs;
    ctStaticArray<ctDynamicArray<ctColorRGBA8>, 4> colors;
    ctDynamicArray<ctVec4> skinWeights;
    ctDynamicArray<ctVec4> skinIndices;

    ctDynamicArray<ctGeometryMorphTarget> morphTargets;
};

class  ctGeometryBakerLOD {
public:
    bool IsValid();
    inline size_t GetSubmeshCount() { return submeshes.Count(); }
    inline ctGeometrySubmesh& GetSubmesh(size_t index) { return submeshes[index]; }
private:
    ctStaticArray<ctGeometryBakerSubmesh, 8> submeshes;
};

struct ctGeometryBakerCollisionDesc {
    bool isConvex;
    struct ctPhysicsShapeSettings* pCustomShape;
    struct ctPhysicsConvexDecomposition* pConvexDecompose;
};

struct ctGeometryBakerGenerateRenderableDesc {
    bool optimizeMehslets = true;
    size_t maxMeshletVertices = 64;
    size_t maxMeshletTriangles = 124;
    float coneWeight = 0.0f;
};

class ctGeometryBaker {
public:
    bool isValid();

    inline size_t GetLODCount(size_t index) { return lods.Count(); }
    inline ctGeometryBakerLOD& GetLODs(size_t index) { return lods[index]; }
    inline ctResults CreateLOD() { return lods.Append(ctGeometryBakerLOD()); }
    inline ctDynamicArray<uint8_t>& GetPhysicsSerialization() { return physicsSerialization; }
    inline ctDynamicArray<ctStringUtf8>& GetMorphTargetNames() { return morphTargetNames; }

    inline ctDynamicArray<uint32_t>& GetScatterTypeRanges() { return scatterTypeRanges; }
    inline ctDynamicArray<ctTransform>& GetScatterTransforms() { return scatterTransforms; }
    inline ctDynamicArray<ctColorRGBA8>& GetScatterColors() { return scatterColors; }
    inline ctDynamicArray<ctVec4>& GetScatterSkinWeights() { return scatterSkinWeights; }
    inline ctDynamicArray<ctVec4>& GetScatterSkinIndices() { return scatterSkinIndices; }

    void RemoveMorphTargets();
    void RemoveMeshData();
    void RemovePhysicsData();
    void RemoveRenderableData();

    void InitializeMorphTargets();
    ctResults GenerateLODs(size_t srcIndex, size_t dstIndex, float ratio = 0.5f);
    ctResults GenerateLODs(float ratios[4] = {1.0f, 0.5f, 0.33f, 0.25f});
    ctResults GenerateCollision(const ctGeometryBakerCollisionDesc& desc);

    ctResults GenerateFinalGeometry(class ctGeometry& output);

private:
    ctDynamicArray<ctStringUtf8> morphTargetNames;
    ctDynamicArray<uint8_t> collisionSerialization;
    ctStaticArray<ctGeometryLOD, 8> lods;

    ctDynamicArray<uint32_t> scatterTypeRanges;
    ctDynamicArray<ctTransform> scatterTransforms;
    ctDynamicArray<ctColorRGBA8> scatterColors;
    ctDynamicArray<ctVec4> skinWeights;
    ctDynamicArray<ctVec4> skinIndices;
};

/* ---------------- Runtime Data ---------------- */

typedef uint32_t ctGeometryVertexIndex;

/* positions are quanitized with the mesh bounding box */
struct  ctGeometryVertexPosition {
    ctGeometryVertexPosition();
    ctGeometryVertexPosition(ctBoundBox& bounds, ctVec3& value);
    ctVec3 Unpack(ctBoundBox& bounds);
    uint16_t data[3]; /* RGB16_SNORM */
};

struct  ctGeometryVertexNormal {
    ctGeometryVertexNormal();
    ctGeometryVertexNormal(ctVec3& value);
    ctVec3 Unpack();
    uint32_t data; /* RGB10A2_SNORM */
};

typedef ctGeometryNormal ctGeometryVertexTangent;

struct  ctGeometryVertexUV {
    ctGeometryVertexUV();
    ctGeometryVertexUV(ctBoundBox2D& bounds, ctVec2& value);
    ctVec2 Unpack(ctBoundBox2D& bounds);
    uint16_t data[2]; /* RG16_SNORM */
};

typedef ctColorRGBA8 ctGeometryVertexColor;

struct ctGeometryVertexSkinning {
    ctGeometryVertexSkinning();
    ctGeometryVertexSkinning(ctVec4 indices, ctVec4 weights);
    uint16_t data[4]; /* RGBA16_UINT */
    uint16_t data[4]; /* RGBA16_SNORM */
};

/* material blending is handled by using blend masks to affect
a single triange, a triangle can have up to 4 materials mapped to it
these materials are determined by the materialOffsets
the micromap masks are then used to encode the blend mask into the
face itself using a tesselated pattern as a "micromap"
each triangle is given 64 sample points at 2^4 precision with only
roughly 33 bytes per triangle (similar to uncompressed skinning!)
while using even low resolution textures produces a much higher footprint
in addition each triangle can address a unique material set making
it possible to blend extremely large terrain elements at a low cost */
struct ctGeometryTriangleMaterialBlend {
    uint8_t materialSlotIndices[4];
    uint32_t micromapSubdivLevel;
    uint32_t micromapBitOffset; /* 3 continuous micromaps expected */
};

/* see https://developer.nvidia.com/blog/introduction-turing-mesh-shaders/
and meshopt_computeMeshletBounds() for more info */
struct ctGeometryMeshlet {
    ctVec4 centerAndRadius;
    ctVec4 coneApex;
    ctVec4 coneAxisAndCutoff;

    uint32_t indexCount;
    uint32_t indexOffset;
    uint32_t primitiveCount;
    uint32_t primitiveOffset;
};

typedef uint8_t ctGeometryMeshletPrimIndex;

/* a submesh represents a single material+lod level 
all geometry submeshes are fed and culled based on distance
inside the task shader/relevant fallback */
struct ctGeometrySubmesh {
    ctVec4 boundSphere;
    float minScreenSize;
    float maxScreenSize;

    uint32_t materialOffset;

    uint32_t meshletOffset;
    uint32_t meshletCount;

    uint32_t fixedFuncIndexCount;
    uint32_t fixedFuncIndexOffset;

    uint32_t positionOffset;
    uint32_t normalOffset;
    uint32_t tangentOffset;
    uint32_t uvOffsets[4];
    uint32_t colorOffsets[4];
    uint32_t skinningOffset;
};

struct ctGeometryPointTransform {
    uint8_t rotScale[4]; /* euler and scale */
    uint16_t position[3]; 
};

typedef ctGeometryVertexColor ctGeometryPointColor;
typedef ctGeometryVertexSkinning ctGeometryPointSkinning;
typedef uint32_t ctGeometryPointRange;

/* morph vertices are applied in the vertex/mesh shader
a morph layer points to a list of morph vertices, these
vertices should be found after the submesh vertices
this is applied ontop of the mesh's positions, normals
and tangents before offset (this will not be sparse)
as such a morphable geometry should be kept small */
struct  ctGeometryMorphLayer {
    ctBoundBox GetBounds();

    ctVec4 boundMin;
    ctVec4 boundMax;
    float sphereDisplacement;
    uint32_t morphNameHash;
    uint32_t morphPositionsOffset;
    uint32_t morphNormalOffset;
    uint32_t morphTangentOffset;
};

class ctGeometryArchive {
    ctGeometry();
    bool isValid();

    ctBoundBox GetBounds();
    ctBoundBox2D GetUVBounds(size_t channel = 0);

protected:
    size_t allocSize;
    void* pAllocation;

    uint32_t expectedBoneCount;

    uint32_t collisionDataSize;
    void* pCollisionData;

    ctVec4* pBoundBox; /* min/max bounding box */
    ctVec4* pUVBoundBox; /* min/max uv bounds */

    ctGeometryVertexIndex* pIndices;

    ctGeometryVertexPosition* pPositions;
    ctGeometryVertexNormal* pNormals;
    ctGeometryVertexTangent* pTangents;
    ctGeometryVertexUV* pUVs;
    ctGeometryVertexColor* pVertexColor;
    ctGeometryVertexSkinning* pSkinning;

    uint32_t submeshCount;
    ctGeometrySubmesh* pSubmeshes;

    ctGeometryMeshlet* pMeshlets;
    ctGeometryMeshletPrimIndex* pMeshletPrimIndices;

    ctVec4* pScatterBoundBox; /* min/max bounding box */
    ctGeometryPointInstance* pScatterTransforms;
    ctGeometryVertexColor* pScatterColors;
    ctGeometryPointRange* pScatterRanges
    ctGeometryPointSkinning* pScatterSkinning;

    uint32_t morphTargetCount;
    ctGeometryMorphLayer* pMorphTargetLayers;
};