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

#include "MeshExport.hpp"
#include "SkeletonExport.hpp"
#include "mikkt/mikktspace.h"

class ctModelExportGeomContainer {
public:
   ctDynamicArray<ctModelMeshInstance> instances;
   ctDynamicArray<ctModelMesh> meshes;
   ctHashTable<int32_t, size_t> meshMapping;
   uint32_t runningIndexOffset = 0;

   ctDynamicArray<uint32_t> indices;
   ctDynamicArray<ctModelMeshVertexPosition> positions;
   ctDynamicArray<ctModelMeshVertexNormalTangent> normalTangents;
   ctDynamicArray<ctModelMeshVertexSkinIndex> skinIndices;
   ctDynamicArray<ctModelMeshVertexSkinWeight> skinWeights;
   ctDynamicArray<ctModelMeshVertexColor> vertexColors[8];
   ctDynamicArray<ctModelMeshVertexUV> uvs[8];
   uint64_t tmpTangentOffset;
};

ctResults ctModelExportMesh::Export(const cgltf_data& input,
                                    ctModel& output,
                                    ctModelExportContext& ctx) {
   ctDebugLog("Exporting Mesh Data...");
   ctModelExportGeomContainer out = ctModelExportGeomContainer();

   /* if single mesh only one instance is made of the merged data */
   if (ctx.singleMesh) {
      out.instances.Append({0, 0});
      out.meshes.Append(ctModelMesh());
   }

   /* loop through each node to find meshes we want to render */
   for (size_t nodeidx = 0; nodeidx < input.nodes_count; nodeidx++) {
      const cgltf_node& node = input.nodes[nodeidx];
      if (!node.mesh) { continue; }
      if (isNodeCollision(node.name)) { continue; }
      if (isNodeLod(node.name)) { continue; } /* don't write lods in top level */
      const cgltf_mesh& mesh = *node.mesh;

      /* create an instance if multimesh */
      bool skipBaseMeshWrite = false;
      int32_t* mapping = out.meshMapping.FindPtr((size_t)&mesh);
      if (!ctx.singleMesh) {
         int32_t rootBone = 0;
         if (mapping) { /* if mesh already has been written */
            rootBone = *mapping;
            skipBaseMeshWrite = true; /* don't write the base mesh again */
         } else {                     /* otherwise create a new one */
            if (!ctx.singleBone) {
               rootBone = ctx.pSkeletonExport->FindBoneForName(node.name);
            }
         }
         out.instances.Append({rootBone, (int32_t)out.meshes.Count()});
      }

      /* fetch or create mesh for node */
      ctModelMesh* pOutMesh = NULL;
      if (ctx.singleMesh) {
         pOutMesh = &out.meshes[0];
      } else {
         if (!mapping) {
            out.meshes.Append(ctModelMesh());
            pOutMesh = &out.meshes.Last();
         } else {
            pOutMesh = &out.meshes[*mapping];
         }
      }
      ctModelMesh& outMesh = *pOutMesh;

      /* iterate through all LODs as they are found */
      const cgltf_node* pLodNode = &node;
      const cgltf_node* pLodRoot = &node;
      int32_t lodIteration = 0;
      while (pLodNode) {
         const cgltf_mesh& lodMesh = *pLodNode->mesh;
         int32_t lodLevel = 0;

         /* get node level from name if not the first mesh */
         if (lodIteration > 0) {
            if (isNodeLod(pLodNode->name)) {
               lodLevel = (int32_t)atol(&pLodNode->name[strlen(pLodNode->name) - 1]);
               ctAssert(lodLevel >= 0);
            }
         }

         /* skip if LOD already exists for mesh */
         if (outMesh.lods[lodLevel].submeshCount > 0) { continue; }

         ctModelMeshLod& lod = outMesh.lods[lodLevel];
         uint32_t tangentRefs = 0; /* number of submeshes referencing tangents */

         /* iterate through mesh primitives and translate them to submeshes */
         for (size_t primidx = 0; primidx < lodMesh.primitives_count; primidx++) {
            const cgltf_primitive& prim = lodMesh.primitives[primidx];
            if (prim.has_draco_mesh_compression) {
               ctDebugError("DRACO COMPRESSION UNSUPPORTED!");
               return CT_FAILURE_UNKNOWN_FORMAT;
            }
            if (prim.type != cgltf_primitive_type_triangles) {
               ctDebugError("ONLY TRIANGLE LISTS ARE SUPPORTED!");
               return CT_FAILURE_UNKNOWN_FORMAT;
            }
            uint32_t materialSlot =
              prim.material ? (uint32_t)(prim.material - input.materials) : UINT32_MAX;
            uint32_t indexOffset = out.runningIndexOffset;
            uint32_t indexCount = 0;

            if (prim.indices) {
               indexCount = (uint32_t)prim.indices->count;
               ExtractIndexBuffer(*prim.indices, lod, out);
            } else {
               ctDebugError("ONLY INDEXED GEOMETRY IS SUPPORTED!");
               return CT_FAILURE_UNKNOWN_FORMAT;
            }

            /* offset indices into the mesh */
            if (out.runningIndexOffset) { ApplyIndexMerge(lod, out); }

            out.tmpTangentOffset =
              out.normalTangents.Count(); /* store offset to write tangents */

            /* extract all attributes */
            for (size_t attribidx = 0; attribidx < prim.attributes_count; attribidx++) {
               if (prim.attributes[attribidx].type == cgltf_attribute_type_tangent) {
                  tangentRefs++;
               }
               ExtractAttribute(prim.attributes[attribidx], lod, out);
            }

            /* increment index offset for primitive */
            out.runningIndexOffset += indexCount;
         }

         /* calculate tangents if needed for lod */
         if (tangentRefs != mesh.primitives_count) { CalculateTangents(lod, out); }

         /* reset index offset for next gltf mesh if not merging into one citrus mesh */
         if (!ctx.singleMesh) { out.runningIndexOffset = 0; }

         /* find next lod */
         bool foundFirst = false;
         const cgltf_node* pLodNodeLast = &node;
         pLodNode = NULL;
         for (size_t i = 0; i < pLodRoot->children_count; i++) {
            if (foundFirst && isNodeLod(pLodRoot->children[i]->name)) {
               pLodNode = pLodRoot->children[i];
            }
            if (!foundFirst && pLodRoot->children[i] == pLodNodeLast) {
               foundFirst = true;
            }
         }
         lodIteration++;
      }
   }

   /* validate no missing lod levels */
   // todo

   /* write out */
   // todo

   return CT_SUCCESS;
}

ctResults ctModelExportMesh::ExtractIndexBuffer(const cgltf_accessor& accessor,
                                                ctModelMeshLod& lod,
                                                ctModelExportGeomContainer& geo) {
   lod.indexCount += (uint32_t)accessor.count;
   /* setup index start if not appending */
   if (lod.indexDataStart == UINT64_MAX) { lod.indexDataStart = geo.indices.Count(); };

   uint32_t offset = (uint32_t)geo.indices.Count();
   geo.indices.Resize(offset + lod.indexCount);
   return CopyAccessorToReserve(accessor, &geo.indices[offset], TinyImageFormat_R32_UINT);
}

ctResults ctModelExportMesh::ApplyIndexMerge(ctModelMeshLod& lod,
                                             ctModelExportGeomContainer& geo) {
   for (size_t i = lod.indexDataStart; i < lod.indexDataStart + lod.indexCount; i++) {
      geo.indices[i] += geo.runningIndexOffset;
   }
   return CT_SUCCESS;
}

ctResults ctModelExportMesh::ExtractAttribute(const cgltf_attribute& attribute,
                                              ctModelMeshLod& lod,
                                              ctModelExportGeomContainer& geo) {
   TinyImageFormat format;
   size_t stride = 0;
   size_t memberoffset = 0;
   size_t offset = 0;
   void* dest = NULL;
   switch (attribute.type) {
      case cgltf_attribute_type_position:
         if (lod.vertexDataPositionStart == UINT64_MAX) {
            lod.vertexDataPositionStart = geo.positions.Count();
         };
         stride = sizeof(ctModelMeshVertexPosition);
         format = TinyImageFormat_R32G32B32_SFLOAT;
         offset = geo.positions.Count();
         geo.positions.Resize(offset + attribute.data->count);
         dest = &geo.positions[offset];
         break;
      case cgltf_attribute_type_normal:
         if (lod.vertexDataNormalTangentStart == UINT64_MAX) {
            lod.vertexDataNormalTangentStart = geo.normalTangents.Count();
         };
         stride = sizeof(ctModelMeshVertexNormalTangent);
         format = TinyImageFormat_R10G10B10A2_SNORM;
         offset = geo.normalTangents.Count();
         geo.normalTangents.Resize(offset + attribute.data->count);
         break;
      case cgltf_attribute_type_tangent:
         if (lod.vertexDataNormalTangentStart == UINT64_MAX) {
            lod.vertexDataNormalTangentStart = geo.normalTangents.Count();
         };
         stride = sizeof(ctModelMeshVertexNormalTangent);
         offset = format = TinyImageFormat_R10G10B10A2_SNORM;
         memberoffset = 4; /* second pair of values */
         offset = geo.tmpTangentOffset;
         geo.normalTangents.Resize(offset + attribute.data->count);
         break;
      case cgltf_attribute_type_joints:
         if (lod.vertexDataSkinIndexStart == UINT64_MAX) {
            lod.vertexDataSkinIndexStart = geo.skinIndices.Count();
         };
         stride = sizeof(ctModelMeshVertexSkinIndex);
         format = TinyImageFormat_R32G32B32A32_UINT;
         offset = geo.skinIndices.Count();
         geo.skinIndices.Resize(offset + attribute.data->count);
         dest = &geo.skinIndices[offset];
         break;
      case cgltf_attribute_type_weights:
         if (lod.vertexDataSkinWeightStart == UINT64_MAX) {
            lod.vertexDataSkinWeightStart = geo.skinWeights.Count();
         };
         stride = sizeof(ctModelMeshVertexSkinWeight);
         format = TinyImageFormat_R16G16B16A16_UNORM;
         offset = geo.skinWeights.Count();
         geo.skinWeights.Resize(offset + attribute.data->count);
         dest = &geo.skinWeights[offset];
         break;
      case cgltf_attribute_type_color:
         if (lod.vertexDataColorStart[attribute.index] == UINT64_MAX) {
            lod.vertexDataColorStart[attribute.index] =
              geo.vertexColors[attribute.index].Count();
         }
         stride = sizeof(ctModelMeshVertexColor);
         format = TinyImageFormat_R8G8B8A8_UNORM;
         offset = geo.vertexColors[attribute.index].Count();
         geo.vertexColors[attribute.index].Resize(offset + attribute.data->count);
         dest = &geo.vertexColors[attribute.index][offset];
         break;
      case cgltf_attribute_type_texcoord:
         if (lod.vertexDataUVStart[attribute.index] == UINT64_MAX) {
            lod.vertexDataUVStart[attribute.index] = geo.uvs[attribute.index].Count();
         }
         stride = sizeof(ctModelMeshVertexUV);
         format = TinyImageFormat_R32G32_SFLOAT;
         offset = geo.uvs[attribute.index].Count();
         geo.uvs[attribute.index].Resize(offset + attribute.data->count);
         dest = &geo.uvs[attribute.index][offset];
         break;
      default: return CT_SUCCESS; /* ignore unknown attribute */
   }
   return CopyAccessorToReserve(*attribute.data, dest, format, stride, memberoffset);
}

ctResults ctModelExportMesh::CopyAccessorToReserve(const cgltf_accessor& accessor,
                                                   void* destination,
                                                   TinyImageFormat format,
                                                   size_t stride,
                                                   size_t offset) {
   if (accessor.is_sparse) {
      ctDebugError("SPARSE GEOMETRY UNSUPPORTED!");
      return CT_FAILURE_UNKNOWN_FORMAT;
   }
   /* todo */
   return CT_SUCCESS;
}

ctResults ctModelExportMesh::CalculateTangents(ctModelMeshLod& lod,
                                               ctModelExportGeomContainer& geo) {
   /* todo: run mikkt */
   return CT_SUCCESS;
}
