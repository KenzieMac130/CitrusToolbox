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

struct ctModelSkeletonBoneName {
   char name[64];
};

struct ctModelSkeleton {
   uint32_t boneCount;
   ctModelSkeletonBoneTransform* transformArray;
   ctModelSkeletonBoneGraph* graphArray;
   uint32_t* hashArray;
   ctModelSkeletonBoneName* nameArray;
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
   char name[64];
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

/* ------------------- Material ------------------- */

struct ctModelTextureInfo {
   int32_t textureFileIdx;
   int32_t uvChannel;
   float offset[2];
   float scale[2];
   float scroll[2];
};

struct ctModelMaterial {
   char name[64];
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

/* ------------------- Lights ------------------- */

enum ctModelLightType { CT_MODEL_LIGHT_POINT, CT_MODEL_LIGHT_SPOT };

struct ctModelLight {
   uint32_t boneIdx;
   float color[3];
   float intensity;
   ctModelLightType type;
   float parameter[4];
};

struct ctModelLights {
   uint32_t lightCount;
   ctModelLight* lights;
};

/* ------------------- Splines ------------------- */

struct ctModelSpline {
   char name[64];
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
   char name[64];
   float clipLength;

   float bboxDisplacement[2][3];
   float bsphereDisplacement;

   uint32_t channelCount;
   int32_t channelStart;
};

struct ctModelAnimationData {
   uint32_t channelCount;
   ctModelAnimationChannel* channels;

   uint32_t clipCount;
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
   char name[8];
   uint64_t size;
   uint8_t* data;
};

struct ctModelEmbeds {
   uint32_t sectionCount;
   ctModelEmbedSection* sections;
};

/* ------------------- Main ------------------- */

#define CT_MODEL_MAGIC   0x636D646C
#define CT_MODEL_VERSION 0x01

struct ctModelHeader {
   uint32_t magic = CT_MODEL_MAGIC;
   uint32_t version = CT_MODEL_VERSION;
   uint64_t cpuDataSize;
   uint64_t pointerFixupTableOffset;
   uint64_t pointerFixupTableSize;
   uint64_t gpuDataOffset;
   uint64_t gpuDataSize;
};

struct ctModel {
   ctModelHeader header;

   ctModelExternalFiles externalFiles;
   ctModelEmbeds embeddedData;

   ctModelSkeleton skeletonData;
   ctModelMeshData meshData;
   ctModelMaterials materialData;
   ctModelLights lightData;
   ctModelSplineData splineData;
   ctModelAnimationData animationData;

   uint64_t mappedCpuDataSize;
   void* mappedCpuData;
};

CT_API ctResults ctModelLoad(ctModel& model, ctFile& file);
CT_API ctResults ctModelSave(ctModel& model, ctFile& file);

CT_API void ctModelRelease(ctModel& model);