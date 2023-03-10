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

struct ctModelSkeletonBoneTransform {
   float translation[3];
   float rotation[4];
   float scale[3];
};

struct ctModelSkeletonBoneGraph {
   int32_t parent;
   int32_t firstChild;
   int32_t nextSibling;
};

struct ctModelSkeleton {
   uint32_t boneCount;
   ctModelSkeletonBoneTransform* transformArray;
   ctModelSkeletonBoneGraph* graphArray;
   uint32_t* hashArray;
   const char** nameArray;
};

/* ------------------- Mesh ------------------- */

struct ctModelMeshVertexPosition {
   float position[3];
};

struct ctModelMeshVertexNormalTangent {
   uint32_t normalPacked;  /* XYZ 10 bit unorm, 2 bits padding */
   uint32_t tangentPacked; /* XYZ 10 bit unorm, 1 bit to W sign, 1 padding */
};

struct ctModelMeshVertexSkinWeight {
   uint64_t weightsPacked; /* 4 16 bit unorms */
};

struct ctModelMeshVertexSkinIndex {
   uint16_t indices[4]; /* 4 16 bit uints */
};

struct ctModelMeshVertexUV {
   float uv[2];
};

struct ctModelMeshVertexColor {
   uint8_t rgba[4];
};

struct ctModelMeshInstance {
   int32_t boneRoot;
   int32_t renderMeshIdx;
};

struct ctModelMeshLod {
   float bias;

   int32_t submeshCount;
   int32_t submeshStart;

   int32_t morphTargetCount;
   int32_t morphTargetStart;

   uint32_t indexCount;
   uint64_t indexDataStart = UINT64_MAX;

   uint64_t vertexDataPositionStart = UINT64_MAX;
   uint64_t vertexDataNormalTangentStart = UINT64_MAX;
   uint64_t vertexDataSkinWeightStart = UINT64_MAX;
   uint64_t vertexDataSkinIndexStart = UINT64_MAX;

   uint64_t vertexDataUVStart[8] = {UINT64_MAX,
                                    UINT64_MAX,
                                    UINT64_MAX,
                                    UINT64_MAX,
                                    UINT64_MAX,
                                    UINT64_MAX,
                                    UINT64_MAX,
                                    UINT64_MAX};
   uint64_t vertexDataColorStart[8] = {UINT64_MAX,
                                       UINT64_MAX,
                                       UINT64_MAX,
                                       UINT64_MAX,
                                       UINT64_MAX,
                                       UINT64_MAX,
                                       UINT64_MAX,
                                       UINT64_MAX};
};

struct ctModelMesh {
   uint32_t editorLinkHash;
   float bbox[2][3];
   float bsphere;
   uint32_t lodCount;
   ctModelMeshLod lods[8];
};

struct ctModelMorphTarget {
   const char* name;
   float defaultWeight;
   float bboxDisplacement[2][3];
   float bsphereDisplacement;

   /* vertex count will match lod */
   uint64_t vertexDataPositionStart = UINT64_MAX;
   uint64_t vertexDataNormalTangentStart = UINT64_MAX;
   uint64_t vertexDataUVStart[8] = {UINT64_MAX,
                                    UINT64_MAX,
                                    UINT64_MAX,
                                    UINT64_MAX,
                                    UINT64_MAX,
                                    UINT64_MAX,
                                    UINT64_MAX,
                                    UINT64_MAX};
   uint64_t vertexDataColorStart[8] = {UINT64_MAX,
                                       UINT64_MAX,
                                       UINT64_MAX,
                                       UINT64_MAX,
                                       UINT64_MAX,
                                       UINT64_MAX,
                                       UINT64_MAX,
                                       UINT64_MAX};
};

struct ctModelSubmesh {
   uint32_t materialIndex;
   uint32_t indexOffset;
   uint32_t indexCount;
};

struct ctModelMeshData {
   uint32_t instanceCount;
   ctModelMeshInstance* instances;

   uint32_t meshCount;
   ctModelMesh* meshes;

   uint32_t submeshCount;
   ctModelSubmesh* submeshes;

   uint32_t morphTargetCount;
   ctModelMorphTarget* morphTargets;

   size_t inMemoryGeometryDataSize;
   uint8_t* inMemoryGeometryData;
};

/* ------------------- Spawner ------------------- */

enum ctModelSpawnerOpCodes {
   CT_MODEL_SPAWN_OP_NOOP = 0,
   CT_MODEL_SPAWN_OP_NEXT_OBJ = 1,
   CT_MODEL_SPAWN_OP_SET_FLAG = 2,
   CT_MODEL_SPAWN_OP_SET_INT32 = 3,
   CT_MODEL_SPAWN_OP_SET_UINT32 = 4,
   CT_MODEL_SPAWN_OP_SET_INT64 = 5,
   CT_MODEL_SPAWN_OP_SET_UINT64 = 6,
   CT_MODEL_SPAWN_OP_SET_FLOAT = 7,
   CT_MODEL_SPAWN_OP_SET_STRING = 8,
   CT_MODEL_SPAWN_OP_SET_VEC4F = 11,
   CT_MODEL_SPAWN_OP_SET_GUID = 12
};

struct ctModelSpawnerOpGeneral {
   uint8_t type;
};

struct ctModelSpawnerOpSetFlag : public ctModelSpawnerOpGeneral {
   uint32_t hash;
};

struct ctModelSpawnerOpSetInt32 : public ctModelSpawnerOpSetFlag {
   int32_t value;
};

struct ctModelSpawnerOpSetUInt32 : public ctModelSpawnerOpSetFlag {
   uint32_t value;
};

struct ctModelSpawnerOpSetInt64 : public ctModelSpawnerOpSetFlag {
   int64_t value;
};

struct ctModelSpawnerOpSetUInt64 : public ctModelSpawnerOpSetFlag {
   uint64_t value;
};

struct ctModelSpawnerOpSetFloat : public ctModelSpawnerOpSetFlag {
   float value;
};

struct ctModelSpawnerOpSetString : public ctModelSpawnerOpSetFlag {
   int32_t stringOffset;
};

struct ctModelSpawnerOpSetVec4f : public ctModelSpawnerOpSetFlag {
   float value[4];
};

struct ctModelSpawnerOpSetGUID : public ctModelSpawnerOpSetFlag {
   uint8_t value[16];
};

size_t ctModelSpawnerOpCodesSize[] = {
   sizeof(ctModelSpawnerOpGeneral),
   sizeof(ctModelSpawnerOpGeneral),
   sizeof(ctModelSpawnerOpSetFlag), 
   sizeof(ctModelSpawnerOpSetInt32),
   sizeof(ctModelSpawnerOpSetUInt32),
   sizeof(ctModelSpawnerOpSetInt64),
   sizeof(ctModelSpawnerOpSetUInt64),
   sizeof(ctModelSpawnerOpSetFloat),
   sizeof(ctModelSpawnerOpSetString),
   sizeof(ctModelSpawnerOpSetVec4f),
   sizeof(ctModelSpawnerOpSetGUID)
};

struct ctModelSpawnerData {
   uint32_t spawnerDataSize;
   uint8_t* spawnerData;

   uint32_t stringPoolSize;
   char* stringPool;
};

/* ------------------- Material ------------------- */

struct ctModelTextureInfo {
   int32_t textureFileIdx;
   int32_t uvChannel;
   float offset[2];
   float scale[2];
   float scroll[2];
};

struct ctModelMaterial {
   const char* name;
   uint32_t shaderIdentifier;

   float baseColorTint[3];
   float alphaModifier;

   float roughnessModifier;
   float indexOfRefraction;
   float metalModifier;
   float normalModifier;
   float emissionColor[3];
   float occlusionModifier;

   float auxProperties[16];

   ctModelTextureInfo baseColorTexture;
   ctModelTextureInfo pbrTexture;
   ctModelTextureInfo normalTexture;
   ctModelTextureInfo auxTexture1;
   ctModelTextureInfo auxTexture2;
   ctModelTextureInfo auxTexture3;
   ctModelTextureInfo auxTexture4;
};

struct ctModelMaterials {
   uint32_t materialCount;
   ctModelMaterial* materials;
};

/* ------------------- Splines ------------------- */

struct ctModelSpline {
   const char* splineName;
   uint32_t pointCount;
   float* positions;
   float* tangents;
   float* bitangents;
};

struct ctModelSplineData {
   uint32_t splineCount;
   ctModelSpline* splines;
};

/* ------------------- Animation ------------------- */

enum ctModelAnimationChannelType {
   CT_MODEL_ANIMCHAN_BONE_LOCATION,
   CT_MODEL_ANIMCHAN_BONE_ROTATION,
   CT_MODEL_ANIMCHAN_BONE_SCALE,
   CT_MODEL_ANIMCHAN_EVENT_FIRE,
   CT_MODEL_ANIMCHAN_CUSTOM_VALUE,
   CT_MODEL_ANIMCHAN_CUSTOM_VECTOR
};

enum ctModelAnimationInterpolation {
   CT_MODEL_ANIMINTERP_LINEAR,
   CT_MODEL_ANIMINTERP_STEP,
   CT_MODEL_ANIMINTERP_CUBIC
};

struct ctModelAnimationChannel {
   ctModelAnimationChannelType type;
   ctModelAnimationInterpolation interpolation;
   uint32_t targetHash;
   uint32_t keyCount;
   float* timeKeys;
   float* valueKeys;
};

struct ctModelAnimationClip {
   const char* name;
   float clipLength;

   float bboxDisplacement[2][3];
   float bsphereDisplacement;

   uint32_t channelCount;
   int32_t channelStart;
};

struct ctModelAnimationData {
   uint64_t channelCount;
   ctModelAnimationChannel* channels;

   uint64_t clipCount;
   ctModelAnimationClip* clips;
};

/* ------------------- External Files ------------------- */

struct ctModelExternalFileEntry {
   uint8_t guid[16];
};

struct ctModelExternalFiles {
   uint32_t externalFileCount;
   ctModelExternalFileEntry* externalFiles;
};

/* ------------------- Embedded Data ------------------- */

struct ctModelEmbedSection {
   const char* name;
   uint64_t size;
   void* data;
};

struct ctModelEmbeds {
   uint32_t sectionCount;
   ctModelEmbedSection* sections;
};

/* ------------------- Main ------------------- */

struct ctModelHeader {
   char magic[4];
   uint64_t cpuDataSize;
   uint64_t pointerFixupTableOffset;
   uint64_t pointerFixupTableSize;
   uint64_t gpuDataOffset;
   uint64_t gpuDataSize;
};

struct ctModel {
   ctModelHeader header;

   ctModelSkeleton skeletonData;
   ctModelMeshData meshData;
   ctModelMaterials materialData;
   ctModelSpawnerData spawnerData;
   ctModelSplineData splineData;
   ctModelAnimationData animationData;
   ctModelExternalFiles externalFiles;
   ctModelEmbeds embeddedData;

   uint64_t mappedCpuDataSize;
   void* mappedCpuData;
};

CT_API ctModel ctModelCreateEmpty();

CT_API ctResults ctModelLoad(ctModel& model, void* file);
CT_API ctResults ctModelSave(ctModel& model, void* file);

CT_API void ctModelRelease(ctModel);