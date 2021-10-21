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

typedef void* ctKeyLimeGeometryReference;
typedef void* ctKeyLimeMaterialReference;
typedef void* ctKeyLimeTransformPoolReference;
typedef void* ctKeyLimeGeoInstanceReference;
typedef void* ctKeyLimeTextureReference;

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
   ctKeyLimeTextureReference textureHandle;
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
   ctKeyLimeTransformPoolReference transformsHandle;
   size_t materialCount;
   ctKeyLimeMaterialReference materialHandles[CT_MAX_MESH_MATERIALS];
   ctKeyLimeGeometryReference geometryHandle;
};

struct ctKeyLimeImageRange {
   uint32_t start;
   uint32_t size;
};

enum ctKeyLimeTextureType {
   CT_TEXTURE_TYPE_1D,
   CT_TEXTURE_TYPE_1D_ARRAY,
   CT_TEXTURE_TYPE_2D,
   CT_TEXTURE_TYPE_2D_ARRAY,
   CT_TEXTURE_TYPE_CUBE,
   CT_TEXTURE_TYPE_CUBE_ARRAY,
   CT_TEXTURE_TYPE_3D
};

enum ctKeyLimeTextureCubeFaces {
   CT_TEXTURE_CUBE_FRONT,
   CT_TEXTURE_CUBE_BACK,
   CT_TEXTURE_CUBE_TOP,
   CT_TEXTURE_CUBE_BOTTOM,
   CT_TEXTURE_CUBE_LEFT,
   CT_TEXTURE_CUBE_RIGHT
};

struct ctKeyLimeTextureDesc {
   int32_t flags;
   uint32_t width;
   uint32_t height;
   uint32_t depth;
   uint32_t mips;
   uint32_t layers;
   enum TinyImageFormat format;
   ctKeyLimeTextureType type;
   ctKeyLimeImageRange ranges[6][CT_MAX_MIP_LEVELS];
   uint8_t* data;

   void* userData;
   void (*fpOnUploadFinish)(ctKeyLimeTextureDesc* pDesc);
};

struct ctKeyLimeGeometryDesc {
   int32_t flags;
   uint32_t submeshCount;
   uint32_t submeshCapacity;
   ctKeyLimeStreamSubmesh* pSubmeshes;

   uint32_t indexCount;
   uint32_t indexCapacity;
   ctKeyLimeMeshIndex* pIndices;

   uint32_t vertexCount;
   uint32_t vertexCapacity;
   ctKeyLimeStreamPosition* pPositions;
   ctKeyLimeStreamNormalTangent* pNormalTangents;
   ctKeyLimeStreamSkin* pSkinning;

   uint32_t uvChannels;
   ctKeyLimeStreamUV* pUVs[CT_MAX_VERTEX_UV_CHANNELS];
   uint32_t colorChannels;
   ctKeyLimeStreamColor* pColors[CT_MAX_VERTEX_COLOR_CHANNELS];

   void* userData;
   void (*fpOnUploadFinish)(ctKeyLimeGeometryDesc* pDesc);
};