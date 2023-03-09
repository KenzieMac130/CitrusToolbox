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
#include "../Model.hpp"

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
   char name[32];
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

CT_API ctModelMeshData* ctModelGetMeshData(ctModel model);