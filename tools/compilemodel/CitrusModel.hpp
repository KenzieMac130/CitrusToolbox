/*
   Copyright 2022 MacKenzie Strand

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
#include "cgltf/cgltf.h"
#include "formats/model/Model.hpp"
#include "tiny_imageFormat/tinyimageformat.h"
#include "middleware/PhysXIntegration.hpp"

struct ctGltf2ModelVertex {
   ctVec3 position;
   ctVec3 normal;
   ctVec4 tangent;
   uint16_t boneIndex[4];
   float boneWeight[4];
   ctVec2 uv[4];
   ctVec4 color[4];
};

struct ctGltf2ModelInstance {
   uint32_t meshIndex;
   uint32_t nodeIndex;
};

/* useful for merge/split operations where indices are annoying */

struct ctGltf2ModelMorph {
   ctDynamicArray<ctGltf2ModelVertex> vertices;
};

struct ctGltf2ModelSubmesh {
   ctModelSubmesh original;
   ctDynamicArray<ctGltf2ModelMorph*> morphs;
   ctDynamicArray<ctGltf2ModelVertex> vertices;
   ctDynamicArray<uint32_t> indices;
};

struct ctGltf2ModelLod {
   int32_t inheritsVertsFromOffset;
   inline bool hasUniqueVerts() {
      return true;  // todo: fix for lod gen
      // return inheritsVertsFromOffset == 0;
   }

   ctModelMeshLod original;
   ctDynamicArray<ctModelMeshMorphTarget> originalMorphs;
   ctDynamicArray<ctGltf2ModelSubmesh*> submeshes;
};

struct ctGltf2ModelMesh {
   ctModelMesh original;
   uint32_t lodCount;
   ctGltf2ModelLod lods[4];
   ctDynamicArray<ctModelMeshMorphTargetMapping> morphMap;

   inline uint32_t CreateOrGetMorphMap(const char* name) {
      for (uint32_t i = 0; i < (uint32_t)morphMap.Count(); i++) {
         if (ctCStrNEql(morphMap[i].name, name, 32)) { return i; }
      }
      ctModelMeshMorphTargetMapping newMap = {};
      newMap.defaultValue = 0.0f;
      strncpy(newMap.name, name, 32);
      morphMap.Append(newMap);
      return (uint32_t)(morphMap.Count() - 1);
   }
   void ExtendLods(uint32_t count);
};

struct ctGltf2ModelTreeSplit {
   ctDynamicArray<ctGltf2ModelMesh*> meshes;
   ctDynamicArray<ctGltf2ModelInstance> instances;
};

enum ctGltf2ModelLodQuality {
   CT_GLTF2MODEL_LODQ_HIGH,
   CT_GLTF2MODEL_LODQ_MED,
   CT_GLTF2MODEL_LODQ_LOW
};

enum ctGltf2ModelPhysicsMode {
   CT_GLTF2MODEL_PHYS_SCENE,    /* mutiple shapes */
   CT_GLTF2MODEL_PHYS_COMPOUND, /* single compound shape */
   CT_GLTF2MODEL_PHYS_RAGDOLL   /* articulation */
};

enum ctGltf2ModelCollisionType {
   CT_GLTF2MODEL_COLLISION_NONE,
   CT_GLTF2MODEL_COLLISION_BOX,
   CT_GLTF2MODEL_COLLISION_SPHERE,
   CT_GLTF2MODEL_COLLISION_PILL,
   CT_GLTF2MODEL_COLLISION_TRI,
   CT_GLTF2MODEL_COLLISION_CONVEX
};

class ctGltf2ModelPxOutStream : public PxOutputStream {
public:
   virtual uint32_t write(const void* src, uint32_t count) {
      output.Append((uint8_t*)src, count);
      return count;
   }

   ctDynamicArray<uint8_t> output;
};

class ctGltf2Model {
public:
   ctResults LoadGltf(const char* filepath);
   ctResults SaveModel(const char* filepath);
   ctResults LoadModel(const char* filepath);

   /* Skeleton Phase */
   ctResults ExtractSkeleton();

   /* Mesh Phase */
   ctResults ExtractGeometry(bool allowSkinning);
   ctResults GenerateLODs(ctGltf2ModelLodQuality quality = CT_GLTF2MODEL_LODQ_HIGH,
                          uint32_t lodCount = 4,
                          float percentageDrop = 0.25f);
   ctResults MergeMeshes(bool allowSkinning); /* optional */
   ctResults GenerateTangents();
   ctResults OptimizeVertexCache();
   ctResults OptimizeOverdraw(float threshold);
   ctResults OptimizeVertexFetch();
   ctResults ComputeBounds();
   ctResults BucketIndices(bool* pSubmeshesDirty);
   ctResults EncodeVertices();
   ctResults CreateGeometryBlob();

   /* Animation */
   ctResults ExtractAnimations();

   /* Materials */
   ctResults ExtractMaterials();

   /* Physics */
   ctResults ExtractPhysics(ctGltf2ModelPhysicsMode mode);

   /* Scene */

   /* Viewer */
   ctResults ModelViewer(int argc, char* argv[]);

protected:
   /* Skeleton Helpers */
   static ctGltf2ModelCollisionType getNodeCollisionType(const char* name);
   static bool isNodeCollision(const char* name);
   static bool isNodeLODLevel(const char* name);
   static bool isNodePath(const char* name);
   static bool isNodeBlockout(const char* name);
   static bool isNodeNavmesh(const char* name);
   static bool isNodeNavmeshConvexVolume(const char* name);
   static bool isNodeNavmeshOfflinkStart(const char* name);
   static bool isNodeNavmeshOfflinkEnd(const char* name);
   static bool isNodeNavmeshRelated(const char* name) {
      return isNodeNavmesh(name) || isNodeNavmeshConvexVolume(name) ||
             isNodeNavmeshOfflinkStart(name) || isNodeNavmeshOfflinkEnd(name);
   }
   static bool isNodePreserved(const char* name);
   int32_t BoneIndexFromGltfNode(const char* nodeName);
   ctMat4 WorldMatrixFromGltfNodeIdx(uint32_t gltfNodeIdx);

   /* Mesh Helpers */
   ctResults ExtractAttribute(cgltf_attribute& attribute,
                              ctModelMeshLod& lod,
                              ctGltf2ModelVertex* vertices);
   ctResults CopyAccessorToReserve(const cgltf_accessor& accessor,
                                   uint8_t* destination,
                                   TinyImageFormat format,
                                   size_t stride,
                                   size_t offset,
                                   size_t arrayLevels = 1,
                                   size_t arraySliceSize = 0);
   TinyImageFormat GltfToTinyImageFormat(cgltf_type vartype,
                                         cgltf_component_type comtype,
                                         bool norm,
                                         size_t& arrayLevels,
                                         size_t& arraySliceSize);
   inline void CommitGeoArrays() {
      model.geometry.meshCount = (uint32_t)finalMeshes.Count();
      model.geometry.meshes = finalMeshes.Data();
      model.geometry.submeshCount = (uint32_t)finalSubmeshes.Count();
      model.geometry.submeshes = finalSubmeshes.Data();
      model.geometry.morphTargetCount = (uint32_t)finalMorphs.Count();
      model.geometry.morphTargets = finalMorphs.Data();
      model.geometry.morphTargetMappingCount = (uint32_t)finalMorphMap.Count();
      model.geometry.morphTargetMapping = finalMorphMap.Data();
   }
   void CombineFromMeshTree(ctGltf2ModelTreeSplit& tree);

   /* Material Helpers */
   inline void WriteScalarProp(ctJSONWriter& writer, const char* name, float prop) {
      writer.DeclareVariable(name);
      writer.WriteNumber(prop);
   }
   inline void WriteVectorProp(ctJSONWriter& writer, const char* name, ctVec4 prop) {
      writer.DeclareVariable(name);
      writer.PushArray();
      writer.WriteNumber(prop.x);
      writer.WriteNumber(prop.y);
      writer.WriteNumber(prop.z);
      writer.WriteNumber(prop.w);
      writer.PopArray();
   }
   inline void WriteStringProp(ctJSONWriter& writer, const char* name, const char* prop) {
      writer.DeclareVariable(name);
      writer.WriteString(prop);
   }
   inline ctResults
   WriteTextureProp(ctJSONWriter& writer, const char* name, const char* uri) {
      ctStringUtf8 path = gltfRootPath;
      path.FilePathAppend(uri);
      ctGUID guid;
      CT_RETURN_FAIL(ctGUIDFromAssetPath(guid, path.CStr()));
      char buff[33];
      memset(buff, 0, 33);
      guid.ToHex(buff);
      WriteStringProp(writer, name, buff);
      return CT_SUCCESS;
   }

   /* Physics Helpers */
   ctResults ExtractPxAsBodies(bool isCompound);
   ctResults ExtractPxAsArticulation();
   inline PxSerialObjectId GetSerialIdForPtr(const void* ptr) {
      return (PxSerialObjectId)ptr;
   }
   PxShape* GetPxBox(const cgltf_node& node, PxMaterial& material);
   PxShape* GetPxSphere(const cgltf_node& node, PxMaterial& material);
   PxShape* GetPxCapsule(const cgltf_node& node, PxMaterial& material);
   PxShape* GetPxConvex(const cgltf_node& node, PxMaterial& material);
   PxShape* GetPxTris(const cgltf_node& node, PxMaterial& material);

private:
   cgltf_data gltf;
   ctModel model;
   ctStringUtf8 gltfRootPath;

   ctGltf2ModelTreeSplit tree;

   /* Skeleton */
   ctDynamicArray<uint32_t> boneHashes;
   ctDynamicArray<ctModelSkeletonBoneName> boneNames;
   ctDynamicArray<ctModelSkeletonBoneTransform> boneTransforms;
   ctDynamicArray<ctModelSkeletonBoneTransform> boneInverseBinds;
   ctDynamicArray<ctModelSkeletonBoneGraph> boneGraph;

   /* Mesh Data */
   ctDynamicArray<ctModelMesh> finalMeshes;
   ctDynamicArray<ctModelSubmesh> finalSubmeshes;
   ctDynamicArray<ctModelMeshMorphTargetMapping> finalMorphMap;
   ctDynamicArray<ctModelMeshMorphTarget> finalMorphs;

   ctDynamicArray<uint32_t> bucketIndices;
   ctDynamicArray<ctGltf2ModelVertex> bucketVertices;

   /* Compressed Vertex Data */
   ctDynamicArray<uint16_t> finalIndices;
   ctDynamicArray<ctModelMeshVertexCoords> finalVertexCoords;
   ctDynamicArray<ctModelMeshVertexSkinData> finalVertexSkinData;
   ctDynamicArray<ctModelMeshVertexColor> finalVertexColors;
   ctDynamicArray<ctModelMeshVertexUV> finalVertexUVs;
   ctDynamicArray<ctModelMeshVertexMorph> finalVertexMorph;

   /* Material */
   ctStringUtf8 materialText;

   /* PhysX */
   ctPhysXIntegration* pPhysXIntegration;
   PxSerializationRegistry* pxSerialRegistry;
   PxCollection* pxGlobalCollection;
   PxCollection* pxInstanceCollection;
   ctGltf2ModelPxOutStream instanceStream;
   ctGltf2ModelPxOutStream globalStream;
};