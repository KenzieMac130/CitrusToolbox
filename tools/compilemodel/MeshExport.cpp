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

#include "../engine/utilities/Common.h"
#include "CitrusModel.hpp"

#include "mikkt/mikktspace.h"
#include "meshoptimizer/meshoptimizer.h"

/* --------------------------------- EXTRACT GLTF --------------------------------- */

ctResults ctGltf2Model::ExtractGeometry(bool allowSkinning) {
   ZoneScoped;
   ctDebugLog("Extracting Geometry...");
   ctHashTable<uint32_t, size_t> meshRedundancyTable;
   for (size_t nodeidx = 0; nodeidx < gltf.nodes_count; nodeidx++) {
      const cgltf_node& node = gltf.nodes[nodeidx];
      if (!node.mesh) { continue; }
      if (isNodeCollision(node.name)) { continue; }
      if (isNodeLODLevel(node.name)) { continue; } /* don't write lods in top level */
      const cgltf_mesh& mesh = *node.mesh;

      /* write instance if already exits (doesn't apply for skinned meshes) */
      /* todo: if node has lods that thee parent doesnt, send it through */
      if (!(node.skin && allowSkinning)) {
         uint32_t* pFoundIndex = meshRedundancyTable.FindPtr((size_t)node.mesh);
         if (pFoundIndex) {
            tree.instances.Append(
              {(uint32_t)*pFoundIndex, (uint32_t)(&node - gltf.nodes)});
            continue;
         }
      }

      ctGltf2ModelMesh* outmesh = new ctGltf2ModelMesh();
      strncpy(outmesh->original.name, node.name, 32);

      /* iterate through all LODs as they are found */
      const cgltf_node* pLodNode = &node;
      const cgltf_node* pLodRoot = &node;
      uint32_t lodLevel = 0;
      while (pLodNode) {
         const cgltf_mesh& lodMesh = *pLodNode->mesh;

         ctAssert(lodLevel < ctCStaticArrayLen(outmesh->lods));
         ctGltf2ModelLod& lod = outmesh->lods[lodLevel];
         /* skip if LOD already exists for mesh */
         if (lod.submeshes.Count() > 0) { continue; }

         /* set lod count based on highest (gaps can be filled by GenerateLODs) */
         if (outmesh->lodCount <= lodLevel) { outmesh->lodCount = lodLevel + 1; }

         /* iterate through mesh primitives and translate them to submeshes */
         for (size_t primidx = 0; primidx < lodMesh.primitives_count; primidx++) {
            const cgltf_primitive& prim = lodMesh.primitives[primidx];
            if (prim.has_draco_mesh_compression) {
               ctDebugError("DRACO COMPRESSION UNSUPPORTED! %s", pLodNode->name);
               return CT_FAILURE_UNKNOWN_FORMAT;
            }
            if (prim.type != cgltf_primitive_type_triangles) {
               ctDebugError("ONLY TRIANGLE LISTS ARE SUPPORTED! %s", pLodNode->name);
               return CT_FAILURE_UNKNOWN_FORMAT;
            }

            /* setup material slot */
            uint32_t materialSlot = /* todo: reindex based on exported materials */
              prim.material ? (uint32_t)(prim.material - gltf.materials) : UINT32_MAX;
            ctGltf2ModelSubmesh* submesh = new ctGltf2ModelSubmesh();
            submesh->original.materialIndex = materialSlot;

            /* extract indices */
            if (prim.indices) {
               submesh->indices.Resize(prim.indices->count);
               CT_RETURN_FAIL(CopyAccessorToReserve(*prim.indices,
                                                    (uint8_t*)submesh->indices.Data(),
                                                    TinyImageFormat_R32_UINT,
                                                    sizeof(submesh->indices[0]),
                                                    0));
            } else {
               ctDebugError("ONLY INDEXED GEOMETRY IS SUPPORTED! %s", pLodNode->name);
               return CT_FAILURE_UNKNOWN_FORMAT;
            }

            /* extract all attributes */
            ctAssert(prim.attributes);
            lod.original.vertexCount = (uint32_t)prim.attributes[0].data->count;
            submesh->vertices.Resize(lod.original.vertexCount);
            for (size_t attribidx = 0; attribidx < prim.attributes_count; attribidx++) {
               CT_RETURN_FAIL(ExtractAttribute(
                 prim.attributes[attribidx], lod.original, submesh->vertices.Data()));
            }
            if (lod.original.vertexDataCoordsStart == UINT32_MAX) {
               ctDebugError(
                 "PRIMITIVE %d FOR %s DOES NOT HAVE POSITIONS!", primidx, pLodNode->name);
               return CT_FAILURE_CORRUPTED_CONTENTS;
            }

            /* todo: coordinate conversion */
            /* todo: bone index conversion */

            /* extract all morph targets */
            // todo refactor for submesh sections
            // if (prim.targets_count != mesh.target_names_count) {
            //   ctDebugError("PRIMITIVE %d FOR %s DOES NOT HAVE MATCHING TARGET COUNTS!",
            //                primidx,
            //                pLodNode->name);
            //   return CT_FAILURE_CORRUPTED_CONTENTS;
            //}
            // for (size_t targetidx = 0; targetidx < prim.targets_count; targetidx++) {
            //   cgltf_morph_target& morph = prim.targets[targetidx];
            //   ctGltf2ModelMorph* outmorph = new ctModelMeshMorphTarget();
            //   strncpy(outmorph.name, mesh.target_names[targetidx], 32);
            //   outmorph.vertexDataMorphOffset = 0;
            //   outmorph.vertexCount = lod.original.vertexCount;
            //   ctModelMeshLod dummy = ctModelMeshLod();

            //   /* extract morph target attributes */
            //   ctAssert(morph.attributes);
            //   vertices.Resize(vertices.Count() + morph.attributes[0].data->count);
            //   for (size_t attribidx = 0; attribidx < morph.attributes_count;
            //        attribidx++) {
            //      CT_RETURN_FAIL(
            //        ExtractAttribute(morph.attributes[attribidx], baseVertex, dummy));
            //   }

            //   submesh->morphs.Append(outmorph);
            //}

            /* apply lod transform offset */
            if (lodLevel > 0) {
               ctMat4 posmat = ctMat4Identity();
               ctMat4 normmat = ctMat4Identity();
               ctMat4Translate(posmat, pLodNode->translation);
               ctMat4Rotate(posmat, pLodNode->rotation);
               ctMat4Scale(posmat, pLodNode->scale);
               ctMat4Rotate(normmat, pLodNode->rotation);
               ctMat4Scale(normmat, pLodNode->scale);
               for (size_t i = 0; i < submesh->vertices.Count(); i++) {
                  ctVec3& pos = submesh->vertices[i].position;
                  ctVec3& normal = submesh->vertices[i].normal;
                  pos = pos * posmat;
                  normal = normalize(normal * normmat);
               }
            }

            /* output submesh */
            lod.submeshes.Append(submesh);
         }

         /* find next lod */
         pLodNode = NULL;
         ctAssert(pLodRoot);
         for (size_t i = 0; i < pLodRoot->children_count; i++) {
            ctAssert(pLodRoot->children[i]);
            if (isNodeLODLevel(pLodRoot->children[i]->name)) {
               const cgltf_node* pLodNodeTmp = pLodRoot->children[i];
               const uint32_t thisLodLevel =
                 (uint32_t)atol(&pLodNodeTmp->name[strlen(pLodNodeTmp->name) - 1]);
               if (thisLodLevel == lodLevel + 1) {
                  if (thisLodLevel < 0 ||
                      thisLodLevel >= ctCStaticArrayLen(outmesh->original.lods)) {
                     ctDebugWarning("LOD %s IS OUT OF BOUNDS, SKIPPED!",
                                    pLodNodeTmp->name);
                     break;
                  }
                  pLodNode = pLodNodeTmp;
                  lodLevel = thisLodLevel;
               }
            }
         }
      }

      /* append mesh and instance */
      if (!(node.skin && allowSkinning)) {
         meshRedundancyTable.Insert((size_t)node.mesh, (uint32_t)tree.meshes.Count());
      }
      tree.instances.Append(
        {(uint32_t)tree.meshes.Count(), (uint32_t)(pLodRoot - gltf.nodes)});
      tree.meshes.Append(outmesh);
   }
   return CT_SUCCESS;
}

ctResults ctGltf2Model::ExtractAttribute(cgltf_attribute& attribute,
                                         ctModelMeshLod& lod,
                                         ctGltf2ModelVertex* vertices) {
   ZoneScoped;
   TinyImageFormat format;
   size_t stride = sizeof(ctGltf2ModelVertex);
   size_t memberoffset;
   void* dest = vertices;
   switch (attribute.type) {
      case cgltf_attribute_type_position:
         format = TinyImageFormat_R32G32B32_SFLOAT;
         memberoffset = offsetof(ctGltf2ModelVertex, position);
         if (lod.vertexDataCoordsStart == UINT32_MAX) { lod.vertexDataCoordsStart = 0; }
         break;
      case cgltf_attribute_type_normal:
         format = TinyImageFormat_R32G32B32_SFLOAT;
         memberoffset = offsetof(ctGltf2ModelVertex, normal);
         if (lod.vertexDataCoordsStart == UINT32_MAX) { lod.vertexDataCoordsStart = 0; }
         break;
      case cgltf_attribute_type_tangent:
         format = TinyImageFormat_R32G32B32A32_SFLOAT;
         memberoffset = offsetof(ctGltf2ModelVertex, tangent);
         if (lod.vertexDataCoordsStart == UINT32_MAX) { lod.vertexDataCoordsStart = 0; }
         break;
      case cgltf_attribute_type_joints:
         format = TinyImageFormat_R16G16B16A16_UINT;
         memberoffset = offsetof(ctGltf2ModelVertex, boneIndex);
         if (lod.vertexDataSkinDataStart == UINT32_MAX) {
            lod.vertexDataSkinDataStart = 0;
         }
         break;
      case cgltf_attribute_type_weights:
         format = TinyImageFormat_R32G32B32A32_SFLOAT;
         memberoffset = offsetof(ctGltf2ModelVertex, boneWeight);
         if (lod.vertexDataSkinDataStart == UINT32_MAX) {
            lod.vertexDataSkinDataStart = 0;
         }
         break;
      case cgltf_attribute_type_color:
         if (attribute.index >= 4) {
            ctDebugWarning("CANNOT WRITE COLOR ATTRIBUTE TO SLOT %d", attribute.index);
            return CT_FAILURE_OUT_OF_BOUNDS;
         }
         format = TinyImageFormat_R32G32B32A32_SFLOAT;
         memberoffset = offsetof(ctGltf2ModelVertex, color) +
                        sizeof(ctGltf2ModelVertex::color[0]) * attribute.index;
         if (lod.vertexDataColorStarts[attribute.index] == UINT32_MAX) {
            lod.vertexDataColorStarts[attribute.index] = 0;
         }
         break;
      case cgltf_attribute_type_texcoord:
         if (attribute.index >= 4) {
            ctDebugWarning("CANNOT WRITE UV ATTRIBUTE TO SLOT %d", attribute.index);
            return CT_FAILURE_OUT_OF_BOUNDS;
         }
         format = TinyImageFormat_R32G32_SFLOAT;
         memberoffset = offsetof(ctGltf2ModelVertex, uv) +
                        sizeof(ctGltf2ModelVertex::uv[0]) * attribute.index;
         if (lod.vertexDataUVStarts[attribute.index] == UINT32_MAX) {
            lod.vertexDataUVStarts[attribute.index] = 0;
         }
         break;
      default:
         ctDebugWarning("IGNORED UNKNOWN ATTRIBUTE TYPE %d", (int)attribute.type);
         return CT_SUCCESS;
   }
   return CopyAccessorToReserve(
     *attribute.data, (uint8_t*)dest, format, stride, memberoffset);
}

ctResults ctGltf2Model::CopyAccessorToReserve(const cgltf_accessor& accessor,
                                              uint8_t* destination,
                                              TinyImageFormat destFormat,
                                              size_t destStride,
                                              size_t destOffset,
                                              size_t destArrayLevels,
                                              size_t destArraySliceSize) {
   ZoneScoped;
   if (accessor.is_sparse) {
      ctDebugError("SPARSE BUFFERS ARE UNSUPPORTED!");
      return CT_FAILURE_UNKNOWN_FORMAT;
   }
   if (accessor.buffer_view->has_meshopt_compression) {
      ctDebugError("GLTF MESHOPT COMPRESSION IS UNSUPPORTED!");
      return CT_FAILURE_UNKNOWN_FORMAT;
   }
   size_t srcStride = accessor.stride;
   accessor.buffer_view->stride ? accessor.buffer_view->stride : accessor.stride;
   size_t srcOffset = accessor.offset + accessor.buffer_view->offset;
   size_t srcArrayLevels = 1;
   size_t srcArraySliceSize = 0;
   TinyImageFormat srcFormat = GltfToTinyImageFormat(accessor.type,
                                                     accessor.component_type,
                                                     accessor.normalized,
                                                     srcArrayLevels,
                                                     srcArraySliceSize);
   uint8_t* source = (uint8_t*)accessor.buffer_view->buffer->data;
   for (size_t i = 0; i < accessor.count; i++) {
      float tmpBuffer[4][4];
      for (size_t j = 0; j < srcArrayLevels; j++) {
         TinyImageFormat_DecodeInput decodeIn = TinyImageFormat_DecodeInput();
         decodeIn.pixel =
           (void*)&source[srcOffset + (srcStride * i) + (srcArraySliceSize * j)];
         TinyImageFormat_DecodeLogicalPixelsF(srcFormat, &decodeIn, 1, tmpBuffer[j]);
      }

      for (size_t j = 0; j < destArrayLevels; j++) {
         TinyImageFormat_EncodeOutput encodeOut = TinyImageFormat_EncodeOutput();
         encodeOut.pixel =
           (void*)&destination[destOffset + (destStride * i) + (destArraySliceSize * j)];
         TinyImageFormat_EncodeLogicalPixelsF(destFormat, tmpBuffer[j], 1, &encodeOut);
      }
   }

   return CT_SUCCESS;
}

TinyImageFormat ctGltf2Model::GltfToTinyImageFormat(cgltf_type vartype,
                                                    cgltf_component_type comtype,
                                                    bool norm,
                                                    size_t& arrayLevels,
                                                    size_t& arraySliceSize) {
   arrayLevels = 1;
   arraySliceSize = 0;
   size_t valueWidth = 1;
   switch (vartype) {
      case cgltf_type_invalid: return TinyImageFormat_UNDEFINED; break;
      case cgltf_type_scalar:
         switch (comtype) {
            case cgltf_component_type_invalid: return TinyImageFormat_UNDEFINED;
            case cgltf_component_type_r_8:
               arraySliceSize = 1 * valueWidth;
               return norm ? TinyImageFormat_R8_SNORM : TinyImageFormat_R8_SINT;
            case cgltf_component_type_r_8u:
               arraySliceSize = 1 * valueWidth;
               return norm ? TinyImageFormat_R8_UNORM : TinyImageFormat_R8_UINT;
            case cgltf_component_type_r_16:
               arraySliceSize = 2 * valueWidth;
               return norm ? TinyImageFormat_R16_SNORM : TinyImageFormat_R16_SINT;
            case cgltf_component_type_r_16u:
               arraySliceSize = 2 * valueWidth;
               return norm ? TinyImageFormat_R16_UNORM : TinyImageFormat_R16_UINT;
            case cgltf_component_type_r_32u:
               arraySliceSize = 4 * valueWidth;
               return TinyImageFormat_R32_UINT;
            case cgltf_component_type_r_32f:
               arraySliceSize = 4 * valueWidth;
               return TinyImageFormat_R32_SFLOAT;
            default: return TinyImageFormat_UNDEFINED;
         }
      case cgltf_type_vec2:
         switch (comtype) {
            case cgltf_component_type_invalid: return TinyImageFormat_UNDEFINED;
            case cgltf_component_type_r_8:
               arraySliceSize = 1 * valueWidth;
               return norm ? TinyImageFormat_R8G8_SNORM : TinyImageFormat_R8G8_SINT;
            case cgltf_component_type_r_8u:
               arraySliceSize = 1 * valueWidth;
               return norm ? TinyImageFormat_R8G8_UNORM : TinyImageFormat_R8G8_UINT;
            case cgltf_component_type_r_16:
               arraySliceSize = 2 * valueWidth;
               return norm ? TinyImageFormat_R16G16_SNORM : TinyImageFormat_R16G16_SINT;
            case cgltf_component_type_r_16u:
               arraySliceSize = 2 * valueWidth;
               return norm ? TinyImageFormat_R16G16_UNORM : TinyImageFormat_R16G16_UINT;
            case cgltf_component_type_r_32u:
               arraySliceSize = 4 * valueWidth;
               return TinyImageFormat_R32G32_UINT;
            case cgltf_component_type_r_32f:
               arraySliceSize = 4 * valueWidth;
               return TinyImageFormat_R32G32_SFLOAT;
            default: return TinyImageFormat_UNDEFINED;
         }
      case cgltf_type_vec3:
         switch (comtype) {
            case cgltf_component_type_invalid: return TinyImageFormat_UNDEFINED;
            case cgltf_component_type_r_8:
               arraySliceSize = 1 * valueWidth;
               return norm ? TinyImageFormat_R8G8B8_SNORM : TinyImageFormat_R8G8B8_SINT;
            case cgltf_component_type_r_8u:
               arraySliceSize = 1 * valueWidth;
               return norm ? TinyImageFormat_R8G8B8_UNORM : TinyImageFormat_R8G8B8_UINT;
            case cgltf_component_type_r_16:
               arraySliceSize = 2 * valueWidth;
               return norm ? TinyImageFormat_R16G16B16_SNORM
                           : TinyImageFormat_R16G16B16_SINT;
            case cgltf_component_type_r_16u:
               arraySliceSize = 2 * valueWidth;
               return norm ? TinyImageFormat_R16G16B16_UNORM
                           : TinyImageFormat_R16G16B16_UINT;
            case cgltf_component_type_r_32u:
               arraySliceSize = 4 * valueWidth;
               return TinyImageFormat_R32G32B32_UINT;
            case cgltf_component_type_r_32f:
               arraySliceSize = 4 * valueWidth;
               return TinyImageFormat_R32G32B32_SFLOAT;
            default: return TinyImageFormat_UNDEFINED;
         }
      case cgltf_type_vec4:
         switch (comtype) {
            case cgltf_component_type_invalid: return TinyImageFormat_UNDEFINED;
            case cgltf_component_type_r_8:
               arraySliceSize = 1 * valueWidth;
               return norm ? TinyImageFormat_R8G8B8A8_SNORM
                           : TinyImageFormat_R8G8B8A8_SINT;
            case cgltf_component_type_r_8u:
               arraySliceSize = 1 * valueWidth;
               return norm ? TinyImageFormat_R8G8B8A8_UNORM
                           : TinyImageFormat_R8G8B8A8_UINT;
            case cgltf_component_type_r_16:
               arraySliceSize = 2 * valueWidth;
               return norm ? TinyImageFormat_R16G16B16A16_SNORM
                           : TinyImageFormat_R16G16B16_SINT;
            case cgltf_component_type_r_16u:
               arraySliceSize = 2 * valueWidth;
               return norm ? TinyImageFormat_R16G16B16A16_UNORM
                           : TinyImageFormat_R16G16B16A16_UINT;
            case cgltf_component_type_r_32u:
               arraySliceSize = 4 * valueWidth;
               return TinyImageFormat_R32G32B32A32_UINT;
            case cgltf_component_type_r_32f:
               arraySliceSize = 4 * valueWidth;
               return TinyImageFormat_R32G32B32A32_SFLOAT;
            default: return TinyImageFormat_UNDEFINED;
         }
      case cgltf_type_mat2:
         arrayLevels = 2;
         switch (comtype) {
            case cgltf_component_type_invalid: return TinyImageFormat_UNDEFINED;
            case cgltf_component_type_r_8:
               arraySliceSize = 1 * valueWidth;
               return norm ? TinyImageFormat_R8G8_SNORM : TinyImageFormat_R8G8_SINT;
            case cgltf_component_type_r_8u:
               arraySliceSize = 1 * valueWidth;
               return norm ? TinyImageFormat_R8G8_UNORM : TinyImageFormat_R8G8_UINT;
            case cgltf_component_type_r_16:
               arraySliceSize = 2 * valueWidth;
               return norm ? TinyImageFormat_R16G16_SNORM : TinyImageFormat_R16G16_SINT;
            case cgltf_component_type_r_16u:
               arraySliceSize = 2 * valueWidth;
               return norm ? TinyImageFormat_R16G16_UNORM : TinyImageFormat_R16G16_UINT;
            case cgltf_component_type_r_32u:
               arraySliceSize = 4 * valueWidth;
               return TinyImageFormat_R32G32_UINT;
            case cgltf_component_type_r_32f:
               arraySliceSize = 4 * valueWidth;
               return TinyImageFormat_R32G32_SFLOAT;
            default: return TinyImageFormat_UNDEFINED;
         }
      case cgltf_type_mat3:
         arrayLevels = 3;
         switch (comtype) {
            case cgltf_component_type_invalid: return TinyImageFormat_UNDEFINED;
            case cgltf_component_type_r_8:
               arraySliceSize = 1 * valueWidth;
               return norm ? TinyImageFormat_R8G8B8_SNORM : TinyImageFormat_R8G8B8_SINT;
            case cgltf_component_type_r_8u:
               arraySliceSize = 1 * valueWidth;
               return norm ? TinyImageFormat_R8G8B8_UNORM : TinyImageFormat_R8G8B8_UINT;
            case cgltf_component_type_r_16:
               arraySliceSize = 2 * valueWidth;
               return norm ? TinyImageFormat_R16G16B16_SNORM
                           : TinyImageFormat_R16G16B16_SINT;
            case cgltf_component_type_r_16u:
               arraySliceSize = 2 * valueWidth;
               return norm ? TinyImageFormat_R16G16B16_UNORM
                           : TinyImageFormat_R16G16B16_UINT;
            case cgltf_component_type_r_32u:
               arraySliceSize = 4 * valueWidth;
               return TinyImageFormat_R32G32B32_UINT;
            case cgltf_component_type_r_32f:
               arraySliceSize = 4 * valueWidth;
               return TinyImageFormat_R32G32B32_SFLOAT;
            default: return TinyImageFormat_UNDEFINED;
         }
      case cgltf_type_mat4:
         arrayLevels = 4;
         switch (comtype) {
            case cgltf_component_type_invalid: return TinyImageFormat_UNDEFINED;
            case cgltf_component_type_r_8:
               arraySliceSize = 1 * valueWidth;
               return norm ? TinyImageFormat_R8G8B8A8_SNORM
                           : TinyImageFormat_R8G8B8A8_SINT;
            case cgltf_component_type_r_8u:
               arraySliceSize = 1 * valueWidth;
               return norm ? TinyImageFormat_R8G8B8A8_UNORM
                           : TinyImageFormat_R8G8B8A8_UINT;
            case cgltf_component_type_r_16:
               arraySliceSize = 2 * valueWidth;
               return norm ? TinyImageFormat_R16G16B16A16_SNORM
                           : TinyImageFormat_R16G16B16_SINT;
            case cgltf_component_type_r_16u:
               arraySliceSize = 2 * valueWidth;
               return norm ? TinyImageFormat_R16G16B16A16_UNORM
                           : TinyImageFormat_R16G16B16A16_UINT;
            case cgltf_component_type_r_32u:
               arraySliceSize = 4 * valueWidth;
               return TinyImageFormat_R32G32B32A32_UINT;
            case cgltf_component_type_r_32f:
               arraySliceSize = 4 * valueWidth;
               return TinyImageFormat_R32G32B32A32_SFLOAT;
            default: return TinyImageFormat_UNDEFINED;
         }
      default: return TinyImageFormat_UNDEFINED;
   }
   return TinyImageFormat();
}

/* -------------------------- LEVEL OF DETAIL --------------------------------- */

ctResults ctGltf2Model::GenerateLODs(float percentageDrop) {
   ZoneScoped;
   ctDebugLog("Generating LODs...");
   /* todo: have fun!
   don't worry about shared vertices!
   they can be done as part of "CombineFromMeshTree" */
   return CT_SUCCESS;
}

/* ------------------------------ MERGE --------------------------------------- */

ctResults ctGltf2Model::MergeMeshes(bool allowSkinning) {
   ZoneScoped;
   ctDebugLog("Merging Meshes...");

   ctGltf2ModelTreeSplit newTree = ctGltf2ModelTreeSplit();
   ctGltf2ModelMesh* outmesh = new ctGltf2ModelMesh();
   strncpy(outmesh->original.name, "MERGED", 32);

   /* get the max number of lods that we need to fill */
   for (size_t midx = 0; midx < tree.meshes.Count(); midx++) {
      ctGltf2ModelMesh* mesh = tree.meshes[midx];
      for (uint32_t lodidx = 0; lodidx < mesh->lodCount; lodidx++) {
         if (outmesh->lodCount <= lodidx) { outmesh->lodCount = lodidx + 1; }
      }
   }

   /* initialize outlods */
   for (uint32_t lodidx = 0; lodidx < outmesh->lodCount; lodidx++) {
      outmesh->lods[lodidx] = ctGltf2ModelLod();
   }

   /* iterate through each instance */
   for (size_t instidx = 0; instidx < tree.instances.Count(); instidx++) {
      /* get mesh used for instance */
      const ctGltf2ModelMesh* inmesh = tree.meshes[tree.instances[instidx].meshIndex];
      uint32_t instNode = tree.instances[instidx].nodeIndex;
      for (uint32_t lodidx = 0; lodidx < outmesh->lodCount; lodidx++) {
         ctGltf2ModelLod& outlod = outmesh->lods[lodidx];
         /* get closest lod */
         uint32_t inlodidx = lodidx >= inmesh->lodCount ? inmesh->lodCount - 1 : lodidx;
         const ctGltf2ModelLod& inlod = inmesh->lods[inlodidx];

         /* wire up outputs */
         if (inlod.original.vertexDataCoordsStart != UINT32_MAX) {
            outlod.original.vertexDataCoordsStart = inlod.original.vertexDataCoordsStart;
         }
         if (inlod.original.vertexDataSkinDataStart != UINT32_MAX) {
            outlod.original.vertexDataSkinDataStart =
              inlod.original.vertexDataSkinDataStart;
         }
         for (uint32_t i = 0; i < 4; i++) {
            if (inlod.original.vertexDataUVStarts[i] != UINT32_MAX) {
               outlod.original.vertexDataUVStarts[i] =
                 inlod.original.vertexDataUVStarts[i];
            }
            if (inlod.original.vertexDataColorStarts[i] != UINT32_MAX) {
               outlod.original.vertexDataColorStarts[i] =
                 inlod.original.vertexDataColorStarts[i];
            }
         }

         /* for each submesh */
         for (size_t submeshidx = 0; submeshidx < inlod.submeshes.Count(); submeshidx++) {
            const ctGltf2ModelSubmesh* insubmesh = inlod.submeshes[submeshidx];
            ctGltf2ModelSubmesh* outsubmesh = NULL;
            /* find the right submesh */
            for (size_t i = 0; i < outlod.submeshes.Count(); i++) {
               if (outlod.submeshes[i]->original.materialIndex ==
                   insubmesh->original.materialIndex) {
                  outsubmesh = outlod.submeshes[i];
               }
            }

            /* if not create a submesh */
            if (!outsubmesh) {
               outlod.submeshes.Append(new ctGltf2ModelSubmesh());
               outsubmesh = outlod.submeshes.Last();
               ctAssert(outsubmesh);
               outsubmesh->original.materialIndex = insubmesh->original.materialIndex;
            }

            /* offset and append indices */
            uint32_t indexOffset = (uint32_t)outsubmesh->vertices.Count();
            for (size_t i = 0; i < insubmesh->indices.Count(); i++) {
               outsubmesh->indices.Append(indexOffset + insubmesh->indices[i]);
            }

            /* transform append vertices */
            for (size_t i = 0; i < insubmesh->vertices.Count(); i++) {
               ctGltf2ModelVertex& vtx = insubmesh->vertices[i];
               ctMat4 transform = WorldMatrixFromGltfNodeIdx(instNode);
               vtx.position = vtx.position * transform;
               ctMat4RemoveTranslation(transform);
               vtx.normal = normalize(vtx.normal * transform);
               /* todo: skinning indices (if instance is unskinned) */
               outsubmesh->vertices.Append(vtx);
            }
         }
      }
   }

   newTree.meshes.Append(outmesh);
   tree = newTree;
   return CT_SUCCESS;
}

void ctGltf2Model::CombineFromMeshTree(ctGltf2ModelTreeSplit& tree) {
   ZoneScoped;
   finalMeshes.Clear();
   finalMorphs.Clear();
   finalSubmeshes.Clear();
   bucketVertices.Clear();
   bucketIndices.Clear();

   /* for each mesh */
   for (size_t meshIdx = 0; meshIdx < tree.meshes.Count(); meshIdx++) {
      ctGltf2ModelMesh& mesh = *tree.meshes[meshIdx];
      mesh.original.lodCount = (uint32_t)mesh.lodCount;

      /* for each lod */
      for (size_t lodIdx = 0; lodIdx < mesh.lodCount; lodIdx++) {
         ctGltf2ModelLod& lod = mesh.lods[lodIdx];

         /* reconnect offsets */
         lod.original.morphTargetCount = (uint32_t)lod.originalMorphs.Count();
         lod.original.morphTargetStart = (uint32_t)finalMorphs.Count();

         lod.original.submeshCount = (uint32_t)lod.submeshes.Count();
         lod.original.submeshStart = (uint32_t)finalSubmeshes.Count();

         lod.original.vertexCount = 0;
         lod.original.vertexDataCoordsStart =
           lod.original.vertexDataCoordsStart != UINT32_MAX
             ? (uint32_t)bucketVertices.Count()
             : UINT32_MAX;
         lod.original.vertexDataSkinDataStart =
           lod.original.vertexDataSkinDataStart != UINT32_MAX
             ? (uint32_t)bucketVertices.Count()
             : UINT32_MAX;
         for (int i = 0; i < 4; i++) {
            lod.original.vertexDataUVStarts[i] =
              lod.original.vertexDataUVStarts[i] != UINT32_MAX
                ? (uint32_t)bucketVertices.Count()
                : UINT32_MAX;
            lod.original.vertexDataColorStarts[i] =
              lod.original.vertexDataColorStarts[i] != UINT32_MAX
                ? (uint32_t)bucketVertices.Count()
                : UINT32_MAX;
         }

         /* for each submesh */
         uint32_t indexCheckpoint = 0;
         for (size_t submeshIdx = 0; submeshIdx < lod.submeshes.Count(); submeshIdx++) {
            ctGltf2ModelSubmesh& submesh = *lod.submeshes[submeshIdx];

            /* setup offsets */
            submesh.original.indexOffset = (uint32_t)bucketIndices.Count();
            submesh.original.indexCount = (uint32_t)submesh.indices.Count();

            /* write indices */
            for (uint32_t i = 0; i < submesh.indices.Count(); i++) {
               bucketIndices.Append(submesh.indices[i] + indexCheckpoint);
            }

            /* write vertices */
            bucketVertices.Append(submesh.vertices);
            lod.original.vertexCount += (uint32_t)submesh.vertices.Count();
            indexCheckpoint += (uint32_t)submesh.vertices.Count();

            finalSubmeshes.Append(submesh.original);
         }

         for (size_t morphIdx = 0; morphIdx < lod.originalMorphs.Count(); morphIdx++) {
            ctModelMeshMorphTarget& morph = *lod.originalMorphs[morphIdx];
            morph.vertexCount = lod.original.vertexCount; /* assumed for sanity */
            for (size_t submeshIdx = 0; submeshIdx < lod.submeshes.Count();
                 submeshIdx++) {
               ctGltf2ModelMorph& submeshMorph =
                 *lod.submeshes[submeshIdx]->morphs[morphIdx];
               bucketVertices.Append(submeshMorph.vertices, submeshMorph.vertexCount);
            }
            finalMorphs.Append(morph);
         }
         mesh.original.lods[lodIdx] = lod.original;
      }
      finalMeshes.Append(mesh.original);
   }
   CommitGeoArrays();
}

/* ----------------------------- TANGENTS ------------------------------------- */

struct ctMikktUserData {
   uint32_t inputIndexCount;
   uint32_t* pIndexIn;
   ctGltf2ModelVertex* pVertexIn;
   ctGltf2ModelVertex* pVertexOut;
};

int ctMikktGetNumFaces(const SMikkTSpaceContext* pContext) {
   return ((ctMikktUserData*)pContext->m_pUserData)->inputIndexCount / 3;
}

int ctMikktGetNumVerticesOfFace(const SMikkTSpaceContext* pContext, const int iFace) {
   return 3;
}

void ctMikktGetPosition(const SMikkTSpaceContext* pContext,
                        float fvPosOut[],
                        const int iFace,
                        const int iVert) {
   const ctMikktUserData* ud = (ctMikktUserData*)pContext->m_pUserData;
   const uint32_t index = ud->pIndexIn[iFace * 3 + iVert];
   fvPosOut[0] = ud->pVertexIn[index].position.x;
   fvPosOut[1] = ud->pVertexIn[index].position.y;
   fvPosOut[2] = ud->pVertexIn[index].position.z;
}

void ctMikktGetNormal(const SMikkTSpaceContext* pContext,
                      float fvNormOut[],
                      const int iFace,
                      const int iVert) {
   const ctMikktUserData* ud = (ctMikktUserData*)pContext->m_pUserData;
   const uint32_t index = ud->pIndexIn[iFace * 3 + iVert];
   fvNormOut[0] = ud->pVertexIn[index].normal.x;
   fvNormOut[1] = ud->pVertexIn[index].normal.y;
   fvNormOut[2] = ud->pVertexIn[index].normal.z;
}

void ctMikktGetTexCoord(const SMikkTSpaceContext* pContext,
                        float fvTexcOut[],
                        const int iFace,
                        const int iVert) {
   const ctMikktUserData* ud = (ctMikktUserData*)pContext->m_pUserData;
   const uint32_t index = ud->pIndexIn[iFace * 3 + iVert];
   fvTexcOut[0] = ud->pVertexIn[index].uv[0].x;
   fvTexcOut[1] = ud->pVertexIn[index].uv[0].y;
}

void ctMikktSetTSpaceBasic(const SMikkTSpaceContext* pContext,
                           const float fvTangent[],
                           const float fSign,
                           const int iFace,
                           const int iVert) {
   const ctMikktUserData* ud = (ctMikktUserData*)pContext->m_pUserData;
   const int index = iFace * 3 + iVert;
   ud->pVertexOut[index].tangent.x = fvTangent[0];
   ud->pVertexOut[index].tangent.y = fvTangent[1];
   ud->pVertexOut[index].tangent.z = fvTangent[2];
   ud->pVertexOut[index].tangent.w = fSign;
}

ctResults ctGltf2Model::GenerateTangents() {
   ZoneScoped;
   ctDebugLog("Generating Tangents...");

   ctDynamicArray<uint32_t> scratchIndices;
   ctDynamicArray<ctGltf2ModelVertex> scratchVertices;
   ctDynamicArray<uint32_t> remapTable;
   ctDynamicArray<meshopt_Stream> streams;

   /* setup mikkt */
   SMikkTSpaceInterface mikktInterface = SMikkTSpaceInterface();
   mikktInterface.m_getNumFaces = ctMikktGetNumFaces;
   mikktInterface.m_getNumVerticesOfFace = ctMikktGetNumVerticesOfFace;
   mikktInterface.m_getPosition = ctMikktGetPosition;
   mikktInterface.m_getNormal = ctMikktGetNormal;
   mikktInterface.m_getTexCoord = ctMikktGetTexCoord;
   mikktInterface.m_setTSpaceBasic = ctMikktSetTSpaceBasic;

   /* iterate meshes */
   for (uint32_t meshIdx = 0; meshIdx < tree.meshes.Count(); meshIdx++) {
      auto& mesh = *tree.meshes[meshIdx];
      for (uint32_t lodIdx = 0; lodIdx < mesh.lodCount; lodIdx++) {
         auto& lod = mesh.lods[lodIdx];
         for (uint32_t submeshIdx = 0; submeshIdx < lod.submeshes.Count(); submeshIdx++) {
            auto& submesh = *lod.submeshes[submeshIdx];

            /* setup scratch vertices */
            scratchVertices.Clear();
            for (uint32_t i = 0; i < submesh.indices.Count(); i++) {
               scratchVertices.Append(submesh.vertices[submesh.indices[i]]);
            }

            /* generate tangents */
            ctMikktUserData userData = ctMikktUserData();
            userData.inputIndexCount = (uint32_t)submesh.indices.Count();
            userData.pIndexIn = submesh.indices.Data();
            userData.pVertexIn = submesh.vertices.Data();
            userData.pVertexOut = scratchVertices.Data();
            SMikkTSpaceContext ctx = {&mikktInterface, &userData};
            genTangSpaceDefault(&ctx);

            /* setup base vertex stream */
            streams.Clear();
            streams.Append({scratchVertices.Data(),
                            sizeof(ctGltf2ModelVertex),
                            sizeof(ctGltf2ModelVertex)});

            /* generate remap table */
            remapTable.Clear();
            remapTable.Resize(submesh.indices.Count());
            size_t vertexCount = meshopt_generateVertexRemapMulti(remapTable.Data(),
                                                                  NULL,
                                                                  submesh.indices.Count(),
                                                                  submesh.indices.Count(),
                                                                  streams.Data(),
                                                                  streams.Count());

            /* remap index buffer */
            meshopt_remapIndexBuffer(
              submesh.indices.Data(), NULL, submesh.indices.Count(), remapTable.Data());

            /* regen vertex buffer */
            submesh.vertices.Resize(vertexCount);
            meshopt_remapVertexBuffer(submesh.vertices.Data(),
                                      scratchVertices.Data(),
                                      submesh.indices.Count(),
                                      sizeof(ctGltf2ModelVertex),
                                      remapTable.Data());

            /* todo morph targets */
         }
      }
   }
   return CT_SUCCESS;
}

/* ----------------------------- VERTEX CACHE --------------------------------- */

ctResults ctGltf2Model::OptimizeVertexCache() {
   ZoneScoped;
   ctDebugLog("Optimizing Vertex Cache...");
   return CT_SUCCESS;
}

/* --------------------------------- OVERDRAW --------------------------------- */

ctResults ctGltf2Model::OptimizeOverdraw(float threshold) {
   ZoneScoped;
   ctDebugLog("Optimizing Overdraw...");
   return CT_SUCCESS;
}

/* -------------------------------- VERTEX FETCH ------------------------------ */

ctResults ctGltf2Model::OptimizeVertexFetch() {
   ZoneScoped;
   ctDebugLog("Optimizing Vertex Fetch...");
   return CT_SUCCESS;
}

/* -------------------------------- INDEX BUCKETS ------------------------------ */

#define INDEX_BUCKET_SIZE 100
ctResults ctGltf2Model::BucketIndices(bool* pSubmeshesDirty) {
   ZoneScoped;
   ctDebugLog("Bucketing Indices...");
   /* pass 1. for each submesh find the amount of INDEX_BUCKET_SIZE needed to represent
    * the mesh, pass 2. for each triangle check which bucket it falls into (maximum) and
    * add the triangle vertices as well as the indices - INDEX_BUCKET_SIZE * bucketIdx
    * into the bucket submesh. */
   return CT_SUCCESS;
}

/* ------------------------------- BOUNDING BOX ------------------------------ */

ctResults ctGltf2Model::ComputeBounds() {
   ZoneScoped;
   ctDebugLog("Computing Bounding Boxes...");
   /* for each mesh */
   for (uint32_t meshIdx = 0; meshIdx < tree.meshes.Count(); meshIdx++) {
      auto& mesh = *tree.meshes[meshIdx];
      /* for each lod */
      for (uint32_t lodIdx = 0; lodIdx < mesh.lodCount; lodIdx++) {
         auto& lod = mesh.lods[lodIdx];
         ctModelMeshLod& lodorigin = mesh.lods[lodIdx].original;
         lodorigin.bbox = ctBoundBox();
         lodorigin.radius = 0.0f;

         /* for each submesh */
         for (uint32_t submeshIdx = 0; submeshIdx < lod.submeshes.Count(); submeshIdx++) {
            auto& submesh = *lod.submeshes[submeshIdx];

            /* calculate position bounding box */
            for (uint32_t v = 0; v < submesh.vertices.Count(); v++) {
               lodorigin.bbox.AddPoint(submesh.vertices[v].position);
            }

            /* calculate position bounding sphere */
            for (uint32_t v = 0; v < submesh.vertices.Count(); v++) {
               float dist = length((submesh.vertices[v].position));
               if (lodorigin.radius < dist) { lodorigin.radius = dist; }
            }

            /* calculate uv bounding boxes */
            for (uint32_t uv = 0; uv < ctCStaticArrayLen(lod.original.vertexDataUVStarts);
                 uv++) {
               if (lod.original.vertexDataUVStarts[uv] == UINT32_MAX) { continue; }
               for (uint32_t v = 0; v < submesh.vertices.Count(); v++) {
                  lodorigin.uvbox[uv].AddPoint(submesh.vertices[v].uv[uv]);
               }
            }
         }
      }
   }

   /* for each morph target */
   // for (uint32_t morphIdx = 0; morphIdx < model.geometry.morphTargetCount; morphIdx++)
   // {
   //   ctModelMeshMorphTarget& morph = model.geometry.morphTargets[morphIdx];
   //   morph.bboxDisplacement = ctBoundBox();
   //   morph.radiusDisplacement = 0.0f;
   //   // morph.uvDisplacement = ctBoundBox2D();

   //   /* calculate position bounding box */
   //   for (uint32_t v = 0; v < morph.vertexCount; v++) {
   //      morph.bboxDisplacement.AddPoint(
   //        bucketVertices[(size_t)morph.vertexDataMorphOffset + v].position);
   //   }

   //   /* calculate position bounding sphere */
   //   for (uint32_t v = 0; v < morph.vertexCount; v++) {
   //      float dist =
   //        length((bucketVertices[(size_t)morph.vertexDataMorphOffset + v].position));
   //      if (morph.radiusDisplacement < dist) { morph.radiusDisplacement = dist; }
   //   }

   /* calculate uv bounding boxes */
   // for (uint32_t v = 0; v < morph.vertexCount; v++) {
   //   morph.uvDisplacement.AddPoint(
   //     vertices[(size_t)morph.vertexDataMorphOffset + v].uv[0]);
   //}
   //}
   return CT_SUCCESS;
}

/* --------------------------------- ENCODING --------------------------------- */

ctResults ctGltf2Model::EncodeVertices() {
   ZoneScoped;
   ctDebugLog("Encoding Compressed Vertices...");

   /* create combined mesh */
   CombineFromMeshTree(tree);

   /* downsize indices (now that we ensured 16 bit bounds) */
   finalIndices.Reserve(bucketIndices.Count());
   for (uint32_t i = 0; i < bucketIndices.Count(); i++) {
      finalIndices.Append((uint16_t)bucketIndices[i]);
   }

   /* for each mesh */
   for (uint32_t meshIdx = 0; meshIdx < model.geometry.meshCount; meshIdx++) {
      ctModelMesh& mesh = model.geometry.meshes[meshIdx];
      /* for each lod */
      for (uint32_t lodIdx = 0; lodIdx < mesh.lodCount; lodIdx++) {
         ctAssert(lodIdx < ctCStaticArrayLen(mesh.lods));
         ctModelMeshLod& lod = mesh.lods[lodIdx];
         size_t inputVertexStart = lod.vertexDataCoordsStart;

         /* ctModelMeshVertexCoords */
         if (lod.vertexDataCoordsStart != UINT32_MAX) {
            lod.vertexDataCoordsStart = (uint32_t)finalVertexCoords.Count();
            for (uint32_t v = 0; v < lod.vertexCount; v++) {
               ctGltf2ModelVertex in = bucketVertices[inputVertexStart + v];

               /* position */
               ctVec3 npos = lod.bbox.NormalizeVector(in.position);
               ctModelMeshVertexCoords coord = ctModelMeshVertexCoords();
               coord.position[0] = meshopt_quantizeUnorm(npos.x, 16);
               coord.position[1] = meshopt_quantizeUnorm(npos.y, 16);
               coord.position[2] = meshopt_quantizeUnorm(npos.z, 16);

               /* normal */
               coord.normal = (meshopt_quantizeUnorm(in.normal.x, 10) << 20) |
                              (meshopt_quantizeUnorm(in.normal.y, 10) << 10) |
                              meshopt_quantizeUnorm(in.normal.z, 10);

               /* tangent */
               coord.tangent =
                 (meshopt_quantizeUnorm(in.tangent.w >= 0.0f ? 1.0f : 0.0f, 2) << 30) |
                 (meshopt_quantizeUnorm(in.tangent.x, 10) << 20) |
                 (meshopt_quantizeUnorm(in.tangent.y, 10) << 10) |
                 meshopt_quantizeUnorm(in.tangent.z, 10);

               finalVertexCoords.Append(coord);
            }
         }

         /* skinning */
         if (lod.vertexDataSkinDataStart != UINT32_MAX) {
            lod.vertexDataSkinDataStart = (uint32_t)finalVertexSkinData.Count();
            for (uint32_t v = 0; v < lod.vertexCount; v++) {
               ctGltf2ModelVertex in = bucketVertices[inputVertexStart + v];
               ctModelMeshVertexSkinData skin;
               memcpy(skin.indices, in.boneIndex, sizeof(skin.indices));
               for (uint32_t i = 0; i < 4; i++) {
                  skin.weights[i] = meshopt_quantizeUnorm(in.boneWeight[i], 16);
               }
               finalVertexSkinData.Append(skin);
            }
         }

         /* uv */
         for (uint32_t ch = 0; ch < ctCStaticArrayLen(lod.vertexDataUVStarts); ch++) {
            if (lod.vertexDataUVStarts[ch] != UINT32_MAX) {
               lod.vertexDataUVStarts[ch] = (uint32_t)finalVertexUVs.Count();
               for (uint32_t v = 0; v < lod.vertexCount; v++) {
                  ctGltf2ModelVertex in = bucketVertices[inputVertexStart + v];
                  ctVec2 npos = lod.uvbox[ch].NormalizeVector(in.uv[ch]);
                  ctModelMeshVertexUV out;
                  out.uv[0] = meshopt_quantizeUnorm(npos.x, 16);
                  out.uv[1] = meshopt_quantizeUnorm(npos.y, 16);
                  finalVertexUVs.Append(out);
               }
            }
         }

         /* color */
         for (uint32_t ch = 0; ch < ctCStaticArrayLen(lod.vertexDataColorStarts); ch++) {
            if (lod.vertexDataColorStarts[ch] != UINT32_MAX) {
               lod.vertexDataColorStarts[ch] = (uint32_t)finalVertexColors.Count();
               for (uint32_t v = 0; v < lod.vertexCount; v++) {
                  ctGltf2ModelVertex in = bucketVertices[inputVertexStart + v];
                  ctModelMeshVertexColor out;
                  out.rgba[0] = meshopt_quantizeUnorm(in.color[ch].x, 8);
                  out.rgba[1] = meshopt_quantizeUnorm(in.color[ch].y, 8);
                  out.rgba[2] = meshopt_quantizeUnorm(in.color[ch].z, 8);
                  out.rgba[3] = meshopt_quantizeUnorm(in.color[ch].w, 8);
                  finalVertexColors.Append(out);
               }
            }
         }
      }
   }

   /* for each morph target */
   for (uint32_t morphIdx = 0; morphIdx < model.geometry.morphTargetCount; morphIdx++) {
      ctModelMeshMorphTarget& morph = model.geometry.morphTargets[morphIdx];
      size_t inputVertexStart = morph.vertexDataMorphOffset;
      morph.vertexDataMorphOffset = (uint32_t)finalVertexMorph.Count();
      /* morph target data */
      for (uint32_t v = 0; v < morph.vertexCount; v++) {
         ctGltf2ModelVertex in = bucketVertices[inputVertexStart + v];
         ctModelMeshVertexMorph out;

         /* position */
         ctVec3 npos = morph.bboxDisplacement.NormalizeVector(in.position);
         out.position[0] = meshopt_quantizeUnorm(npos.x, 16);
         out.position[1] = meshopt_quantizeUnorm(npos.y, 16);
         out.position[2] = meshopt_quantizeUnorm(npos.z, 16);

         /* normal */
         out.normal = (meshopt_quantizeUnorm(in.normal.x, 10) << 20) |
                      (meshopt_quantizeUnorm(in.normal.y, 10) << 10) |
                      meshopt_quantizeUnorm(in.normal.z, 10);

         /* color */
         out.rgba[0] = meshopt_quantizeUnorm(in.color[0].x, 8);
         out.rgba[1] = meshopt_quantizeUnorm(in.color[0].y, 8);
         out.rgba[2] = meshopt_quantizeUnorm(in.color[0].z, 8);
         out.rgba[3] = meshopt_quantizeUnorm(in.color[0].w, 8);
      }
   }
   return CT_SUCCESS;
}

/* --------------------------------- PACKAGE --------------------------------- */

#define SET_GPU_TABLE(SIZE, OFFSET, ARRAY)                                               \
   model.gpuTable.SIZE = sizeof(ARRAY[0]) * ARRAY.Count();                               \
   model.gpuTable.OFFSET = model.gpuTable.SIZE ? runningOffset : UINT64_MAX;             \
   runningOffset =                                                                       \
     ctAlign(runningOffset + (sizeof(ARRAY[0]) * ARRAY.Count()), CT_ALIGNMENT_MODEL_GPU)

#define WRITE_GPU_TABLE(SIZE, OFFSET, ARRAY)                                             \
   memcpy(&model.inMemoryGeometryData[model.gpuTable.OFFSET],                            \
          ARRAY.Data(),                                                                  \
          sizeof(ARRAY[0]) * ARRAY.Count())

ctResults ctGltf2Model::CreateGeometryBlob() {
   ZoneScoped;
   ctDebugLog("Packaging Geometry...");

   size_t runningOffset = 0;
   SET_GPU_TABLE(indexDataSize, indexDataStart, finalIndices);
   SET_GPU_TABLE(vertexDataCoordsSize, vertexDataCoordsStart, finalVertexCoords);
   SET_GPU_TABLE(vertexDataSkinSize, vertexDataSkinStart, finalVertexSkinData);
   SET_GPU_TABLE(vertexDataUVSize, vertexDataUVStart, finalVertexUVs);
   SET_GPU_TABLE(vertexDataColorSize, vertexDataColorStart, finalVertexColors);
   SET_GPU_TABLE(vertexDataMorphSize, vertexDataMorphStart, finalVertexMorph);
   // SET_GPU_TABLE(scatterDataSize, scatterDataStart, NULL);

   model.inMemoryGeometryData = (uint8_t*)ctMalloc(runningOffset);
   model.inMemoryGeometryDataSize = runningOffset;

   WRITE_GPU_TABLE(indexDataSize, indexDataStart, finalIndices);
   WRITE_GPU_TABLE(vertexDataCoordsSize, vertexDataCoordsStart, finalVertexCoords);
   WRITE_GPU_TABLE(vertexDataSkinSize, vertexDataSkinStart, finalVertexSkinData);
   WRITE_GPU_TABLE(vertexDataUVSize, vertexDataUVStart, finalVertexUVs);
   WRITE_GPU_TABLE(vertexDataColorSize, vertexDataColorStart, finalVertexColors);
   WRITE_GPU_TABLE(vertexDataMorphSize, vertexDataMorphStart, finalVertexMorph);

   return CT_SUCCESS;
}
