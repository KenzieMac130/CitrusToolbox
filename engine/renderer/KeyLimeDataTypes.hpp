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

typedef ctHandle ctKeyLimeGeometryHandle;
typedef ctHandle ctKeyLimeMaterialHandle;
typedef ctHandle ctKeyLimeTransformPoolHandle;
typedef ctHandle ctKeyLimeGeoInstanceHandle;
typedef ctHandle ctKeyLimeTextureHandle;

struct CT_API ctKeyLimeStreamSubmesh {
   int32_t idxOffset;
   int32_t idxCount;
   int32_t matIdx;
};

struct CT_API ctKeyLimeStreamPosition {
   float position[3];
};

struct CT_API ctKeyLimeStreamNormalTangent {
   float tangent[4];
   float normal[3];
};

struct CT_API ctKeyLimeStreamUV {
   float uv[2];
};

struct CT_API ctKeyLimeStreamColor {
   uint8_t color[4];
};

struct CT_API ctKeyLimeStreamSkin {
   uint16_t weight[4];
   uint16_t boneIdx[4];
};

struct CT_API ctKeyLimeStreamTransform {
   float position[3];
   float orientation[4];
   float scale[3];
};

typedef uint32_t ctKeyLimeMeshIndex;

struct ctKeyLimeCameraDesc {
   ctVec3 position;
   ctQuat rotation;
   float fov;
};

struct ctKeyLimeGeometryHeader {
   char magic[4];
   int32_t flags;
   int32_t alignment;

   float cener[3];
   float radius;

   uint32_t submeshCount;
   uint32_t indexCount;
   uint32_t vertexCount;
   uint32_t uvChannelCount;
   uint32_t colorChannelCount;

   uint64_t submeshOffset;
   uint64_t indexOffset;
   uint64_t positionOffset;
   uint64_t tangentNormalOffset;
   uint64_t skinOffset;
   uint64_t uvOffsets[4];
   uint64_t colorOffsets[4];
};

struct ctKeyLimeCreateGeometryDesc {
   ctKeyLimeGeometryHeader header;
   void* blob;
   size_t blobSize;
};

struct ctKeyLimeMaterialScalarDesc {
   const char* name;
   float value;
};

struct ctKeyLimeMaterialVectorDesc {
   const char* name;
   float value[4];
};

struct ctKeyLimeMaterialTextureDesc {
   const char* name;
   ctKeyLimeTextureHandle textureHandle;
};

struct ctKeyLimeMaterialDesc {
   int32_t flags;
   const char* shaderType;

   size_t scalarCount;
   ctKeyLimeMaterialScalarDesc* pScalars;
   size_t vectorCount;
   ctKeyLimeMaterialVectorDesc* pVectors;
   size_t textureCount;
   ctKeyLimeMaterialTextureDesc* pTextures;
};

struct ctKeyLimeTransformsDesc {
   int32_t flags;
   size_t transformCount;
   ctKeyLimeStreamTransform* pTransforms;
};

struct ctKeyLimeInstanceDesc {
   int32_t flags;
   ctHandle transformsHandle;
   size_t materialCount;
   ctHandle* pMaterialHandles;
   ctHandle geometryHandle;
};