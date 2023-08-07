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
/* ------------------- Skeleton ------------------- */

/* ctMat4 without alignment enforcement */
struct ctModelMatrix {
   float data[4][4];
};

struct ctModelSkeletonBoneGraph {
   int32_t parent;      /* -1: none */
   int32_t firstChild;  /* -1: none */
   int32_t nextSibling; /* -1: none */
};

struct ctModelSkeletonBoneName {
   char name[32];
};

enum ctModelConstraintFlags {
   CT_MODEL_CONSTRAINT_LIMIT_ROTATION_X = 0x01,
   CT_MODEL_CONSTRAINT_LIMIT_ROTATION_Y = 0x02,
   CT_MODEL_CONSTRAINT_LIMIT_ROTATION_Z = 0x04
};

struct ctModelBoneConstraint {
   int32_t flags;
   ctVec3 rotationMin;
   ctVec3 rotationMax;
};

struct ctModelSkeleton {
   uint32_t boneCount;
   ctTransform* transformArray;            /* BXFORMS */
   ctModelMatrix* inverseBindArray;        /* BINVBIND */
   ctModelSkeletonBoneGraph* graphArray;   /* BGRAPH */
   ctModelBoneConstraint* constraintArray; /* BCONSTR */
   uint32_t* hashArray;                    /* BHASHES */
   ctModelSkeletonBoneName* nameArray;     /* BNAMES */
};

/* ------------------- Mesh ------------------- */

struct ctModelMeshVertexCoords {
   uint32_t normal;     /* XYZ 10 bit unorm, 2 bits padding */
   uint32_t tangent;    /* XYZ 10 bit unorm, 1 bit to W sign, 1 padding */
   int16_t position[3]; /* 3 snorms scaled by bbox */
};

struct ctModelMeshVertexSkinData {
   uint16_t indices[4]; /* 4 16 bit uints */
   uint16_t weights[4]; /* 4 16 bit unorms */
};

struct ctModelMeshVertexUV {
   int16_t uv[2]; /* 2 snorms scaled by uvbox */
};

struct ctModelMeshVertexColor {
   uint8_t rgba[4]; /* RGBA8 unorm */
};

struct ctModelMeshVertexMorph {
   uint32_t normal;     /* XYZ 10 bit unorm, 2 bits padding */
   uint32_t tangent;    /* XYZ 10 bit unorm, 1 bit to W sign, 1 padding */
   int8_t rgba[4];      /* RGBA8 snorm first vertex color displacement */
   int16_t position[3]; /* 3 snorms scaled by parent lod bbox */
};

struct ctModelMeshScatterData {
   uint16_t rotation[4]; /* Quaternion in 16bit unorm*/
   uint16_t position[3]; /* Position in scatter bounding box */
   uint8_t scale;        /* Portion between min and max scale */
   uint8_t variance;     /* Variation parameter */
};

struct ctModelSubmesh {
   uint32_t materialIndex;
   uint32_t indexOffset;
   uint32_t vertexOffset;
   uint32_t indexCount;
};

struct ctModelMeshLod {
   ctBoundBox bbox;       /* bounding box of vertices */
   float radius;          /* bounding sphere of vertices */
   ctBoundBox2D uvbox[4]; /* bounding box of uv coords per-channel */

   uint32_t submeshCount;
   uint32_t submeshStart;

   uint32_t morphTargetCount;
   uint32_t morphTargetStart;

   uint32_t vertexCount;
   uint32_t vertexDataCoordsStart = UINT32_MAX;
   uint32_t vertexDataSkinDataStart = UINT32_MAX;
   uint32_t vertexDataUVStarts[4] = {UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX};
   uint32_t vertexDataColorStarts[4] = {UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX};
};

struct ctModelMesh {
   char name[32];
   uint32_t morphMapStart;
   uint32_t morphMapCount;
   uint32_t lodCount;
   ctModelMeshLod lods[4];
};

struct ctModelMeshMorphTargetMapping {
   char name[32];
   float value;
};

struct ctModelMeshMorphTarget {
   uint32_t mapping;
   ctBoundBox bboxDisplacement;
   float radiusDisplacement;

   /* vertexCount will match parent lod */
   uint32_t vertexCount;
   uint32_t vertexDataMorphOffset = UINT32_MAX;
};

struct ctModelMeshScatter {
   ctBoundBox bbox;
   uint32_t meshIdx;
   uint32_t scatterCount;
   uint32_t scatterDataOffset = UINT32_MAX;
};

struct ctModelMeshData {
   uint32_t meshCount;
   ctModelMesh* meshes; /* MESHES */

   uint32_t submeshCount;
   ctModelSubmesh* submeshes; /* SUBMESH */

   uint32_t morphTargetMappingCount;
   ctModelMeshMorphTargetMapping* morphTargetMapping; /* MORPHMAP */

   uint32_t morphTargetCount;
   ctModelMeshMorphTarget* morphTargets; /* MORPHS */

   uint32_t scatterCount;
   ctModelMeshScatter* scatters; /* MSCATTER */
};

/* ------------------- GPU Data ------------------- */

enum ctModelGPUCompression {
   CT_MODEL_GPU_COMPRESS_NONE = 0,
   CT_MODEL_GPU_COMPRESS_DEFLATE = 1,
   CT_MODEL_GPU_COMPRESS_COUNT
};

struct ctModelGPUPayload {
   uint32_t compression = CT_MODEL_GPU_COMPRESS_NONE;
   uint64_t compressedSize = 0;
   uint64_t size = 0;
   uint64_t start = UINT64_MAX;
};

struct ctModelGPUPayloadInfo {
   ctModelGPUPayload indexData;
   ctModelGPUPayload vertexDataCoords;
   ctModelGPUPayload vertexDataSkin;
   ctModelGPUPayload vertexDataUV;
   ctModelGPUPayload vertexDataColor;
   ctModelGPUPayload vertexDataMorph;
   ctModelGPUPayload scatterData;
};

/* ------------------- Splines ------------------- */

enum ctModelSplineFlags {
   CT_MODEL_SPLINE_CYCLIC = 0x01,
   CT_MODEL_SPLINE_INTERPOLATE_CUBIC = 0x02
};

struct ctModelSpline {
   uint32_t flags;
   uint32_t pointCount;
   uint32_t pointOffset;
};

struct ctModelSplineData {
   uint32_t segmentCount;
   ctModelSpline* segments; /* SSEGS */

   uint32_t pointCount;
   ctVec3* positions; /* SPOS */
   ctVec3* normals;   /* SNRM */
   ctVec3* tangents;  /* STAN */
};

/* ------------------- Animation ------------------- */

enum ctModelAnimationChannelType {
   CT_MODEL_ANIMCHAN_BONE_TRANSLATION,
   CT_MODEL_ANIMCHAN_BONE_ROTATION,
   CT_MODEL_ANIMCHAN_BONE_SCALE,
   CT_MODEL_ANIMCHAN_MORPH_FACTOR,
   CT_MODEL_ANIMCHAN_CUSTOM_VALUE,
};

enum ctModelAnimationInterpolation {
   CT_MODEL_ANIMINTERP_LINEAR,
   CT_MODEL_ANIMINTERP_STEP
};

struct ctModelAnimationChannel {
   ctModelAnimationChannelType type;
   ctModelAnimationInterpolation interpolation;
   uint32_t targetHash;
   uint32_t keyCount;
   uint32_t timeScalarOffset;
   uint32_t valueScalarOffset;
};

struct ctModelAnimationClip {
   char name[32];
   float clipLength;

   uint32_t channelCount;
   int32_t channelStart;
};

struct ctModelAnimationData {
   uint32_t clipCount;
   ctModelAnimationClip* clips; /* ACLIPS */

   uint32_t channelCount;
   ctModelAnimationChannel* channels; /* ACHANS */

   uint32_t scalarCount;
   float* scalars; /* ASCALARS */
};

/* ------------------- Blobs ------------------- */

struct ctModelBlobData {
   uint64_t size;
   uint8_t* data;
};

/*
MATCODE: material json
PXBAKEG: physx serialization for global (materials/shapes)
PXBAKEI: physx serialization for instance (bodies, joints)
NAVMESH: detour navmesh
SCNCODE: lua script for the scene
*/

/* ------------------- Main ------------------- */

#define CT_MODEL_MAGIC   0x6C646D63
#define CT_MODEL_VERSION 0x01

enum ctModelCPUCompression {
   CT_MODEL_CPU_COMPRESS_NONE = 0,
   CT_MODEL_CPU_COMPRESS_LZ4 = 1,
   CT_MODEL_CPU_COMPRESS_COUNT
};

struct ctModelHeader {
   uint32_t magic;
   uint32_t version;
   uint32_t cpuCompressionType;
   uint64_t cpuCompressionSize;
   uint64_t wadDataOffset;
   uint64_t wadDataSize;
   uint64_t gpuDataOffset;
   uint64_t gpuDataSize;
};

struct ctModel {
   ctModelHeader header;

   ctModelSkeleton skeleton;
   ctModelMeshData geometry;
   ctModelAnimationData animation;
   ctModelSplineData splines;
   ctModelBlobData materialSet;         /* MATSET */
   ctModelBlobData physxSerialGlobal;   /* PXBAKEG */
   ctModelBlobData physxSerialInstance; /* PXBAKEI */
   ctModelBlobData navmeshData;         /* NAVMESH */
   ctModelBlobData sceneScript;         /* SCNCODE */
   ctModelGPUPayloadInfo gpuTable;      /* GPUTABLE */

   uint64_t mappedCpuDataSize;
   void* mappedCpuData;

   size_t inMemoryGeometryDataSize;
   uint8_t* inMemoryGeometryData;
};

CT_API ctResults ctModelLoad(ctModel& model, ctFile& file, bool CPUGeometryData = false);
CT_API ctResults
ctModelSave(ctModel& model,
            ctFile& file,
            ctModelCPUCompression compression = CT_MODEL_CPU_COMPRESS_LZ4);

CT_API void ctModelReleaseGeometry(ctModel& model);
CT_API void ctModelRelease(ctModel& model);