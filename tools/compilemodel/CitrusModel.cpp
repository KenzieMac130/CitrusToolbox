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

#define CGLTF_IMPLEMENTATION
#pragma warning(push)
#pragma warning(disable : 4267)
#include "../engine/utilities/JSON.hpp"
#include "../thirdparty/cgltf/cgltf.h"
#pragma warning(pop)

#include "../engine/formats/mesh/RenderMesh.h"
#include "../engine/formats/wad/WADCore.h"

#include "../engine/formats/wad/prototypes/MarkersAndBlobs.h"
#include "../engine/formats/wad/prototypes/Header.h"
#include "../engine/formats/wad/prototypes/Spawner.h"
#include "../engine/formats/wad/prototypes/Bones.h"
#include "../engine/formats/wad/prototypes/Material.h"
#include "../engine/formats/wad/prototypes/RenderMesh.h"
#include "../engine/formats/wad/prototypes/Camera.h"
#include "../engine/formats/wad/prototypes/Light.h"

//#include "../engine/renderer/KeyLimeDataTypes.hpp"

#include "PxPhysicsAPI.h"
#include "extensions/PxDefaultAllocator.h"
using namespace physx;

#include "mikkt/mikktspace.h"

struct WadWriteSection {
   ctWADLump lump;
   void* data;
};
ctDynamicArray<WadWriteSection> gWadSections;
ctDynamicArray<char> gStringsContent;

const char* gHelpString =
  "Example:\n\t<OPTIONS> inputGltfPath outputWadPath\n"
  "Options:"
  "\n\t-help: Show help"
  "\n\t-toy <TYPE>: Toy type path to associate as a prefab"
  "\n\t-lua <PATH>: Path of the lua script to embed"
  "\n\t-embp <PATH>: Path of a file to embed in the WAD (*)"
  "\n\t-emba <ALIAS>: Wad lump alias of a file to embed in lump (*)"
  "\n\t-map: Marks this asset as a map"
  "\n\t(*): Multiple usages allowed";

int32_t gNextLumpOffset = 0;
int32_t MakeSection(char name[8], size_t size, void* data) {
   WadWriteSection result = WadWriteSection();
   memcpy(result.lump.name, name, 8);
   result.lump.size = (int32_t)size;
   result.lump.filepos = gNextLumpOffset;
   result.data = data;
   gWadSections.Append(result);
   gNextLumpOffset += result.lump.size;
   return (int32_t)gWadSections.Count() - 1;
};

int32_t SaveString(const char* str) {
   if (!str) { return -1; }
   int32_t start = (int32_t)gStringsContent.Count();
   gStringsContent.Append(str, strlen(str) + 1);
   return start;
}

ctStringUtf8 ConvertImagePath(const char* input) {
   return input;
}

#define FindFlag(_name) _FindFlag(_name, argc, argv)
bool _FindFlag(const char* name, int argc, char* argv[]) {
   for (int i = 1; i < argc; i++) {
      if (ctCStrEql(argv[i], name)) { return true; }
   }
   return false;
}

#define FindParamOccurance(_name, _occ) _FindParamOccurance(_name, _occ, argc, argv)
char* _FindParamOccurance(const char* name, int occurance, int argc, char* argv[]) {
   int j = 0;
   for (int i = 1; i < argc; i++) {
      if (ctCStrEql(argv[i], name)) {
         if (j == occurance) { return argv[i + 1]; }
         j++;
      }
   }
   return NULL;
}

#define FindParam(_name) FindParamOccurance(_name, 0)

int main(int argc, char* argv[]) {
   if (argc < 3) {
      ctDebugError("Not enough args!\n%s", gHelpString);
      return -1;
   }
   cgltf_options options = {};
   cgltf_data* gltf;

   if (FindFlag("-help")) { ctDebugLog(gHelpString); }

   const char* relativePath = FindParam("-rp");
   const char* gltfPath = argv[argc - 2];
   const char* outPath = argv[argc - 1];
   const char* outGPUPath = FindParam("-gpu");

   if (!relativePath) {
      ctDebugWarning("Relative path undefined!");
      relativePath = "";
   }

   if (cgltf_parse_file(&options, gltfPath, &gltf) != cgltf_result_success) {
      ctDebugError("FAILED TO PARSE FILE %s", gltfPath);
      return -2;
   };
   if (cgltf_load_buffers(&options, gltf, gltfPath) != cgltf_result_success) {
      ctDebugError("FAILED TO LOAD BUFFERS %s", gltfPath);
      return -3;
   }

   /* Common Citrus Header */
   ctWADProtoHeader header = ctWADProtoHeader();
   header.magic = CT_WADPROTO_HEADER_MAGIC;
   header.revision = CT_WADPROTO_HEADER_INTERNAL_REV;
   MakeSection(CT_WADPROTO_NAME_HEADER, sizeof(header), &header);

   /* Lua Script */
   ctDynamicArray<char> lua;
   {
      const char* luaPath = FindParam("-lua");
      if (luaPath) {
         FILE* pFile = fopen(luaPath, "wb");
         if (pFile) {
            fseek(pFile, 0, SEEK_END);
            size_t fsize = ftell(pFile);
            fseek(pFile, 0, SEEK_SET);
            lua.Resize(fsize);
            fread(lua.Data(), 1, fsize, pFile);
            fclose(pFile);
            if (!lua.isEmpty()) {
               MakeSection(CT_WADBLOB_NAME_LUA, lua.Count(), lua.Data());
            }
         }
      }
   }

   /* Spawners */
   ctDynamicArray<ctWADProtoSpawner> spawners;
   {
      if (!spawners.isEmpty()) {
         MakeSection(CT_WADPROTO_NAME_SPAWNER, sizeof(header), &header);
      }
   }

   /* Bones */
   ctWADProtoRigSignature rigSignature = ctWADProtoRigSignature();
   ctDynamicArray<ctWADProtoBoneTransform> boneXfwd;
   ctDynamicArray<ctWADProtoBoneParentIdx> boneParents;
   ctDynamicArray<ctWADProtoBoneNameEntry> boneNameOffs;
   ctDynamicArray<cgltf_node*> outGltfNodes;
   {
      int numRoot = 0;
      for (size_t i = 0; i < gltf->nodes_count; i++) {
         const cgltf_node node = gltf->nodes[i];

         /* Skip camera or light */
         if (node.light || node.camera) { continue; }

         ctTransform xform = ctTransform();
         if (node.has_translation) { xform.position = node.translation; }
         if (node.has_rotation) { xform.rotation = node.rotation; }
         if (node.has_scale) { xform.scale = node.scale; }
         if (node.has_matrix) { ctDebugWarning("Matrix on %s ignored...", node.name); }
         if (node.parent) {
            bool found = false;
            for (size_t j = 0; j < gltf->nodes_count; j++) {
               /* Skip camera and light orientations */
               const cgltf_node* parent = node.parent;
               while (parent && (parent->light || parent->camera)) {
                  parent = parent->parent;
               }
               if (parent) {
                  if (&gltf->nodes[j] == parent) {
                     boneParents.Append((int16_t)j);
                     found = true;
                     break;
                  }
               }
            }
            if (!found) { boneParents.Append(-1); }
         } else {
            boneParents.Append(-1);
            numRoot++;
         }

         ctWADProtoBoneNameEntry nameEntry = ctWADProtoBoneNameEntry();
         if (node.name) {
            nameEntry.strOffset = SaveString(node.name);
            nameEntry.hash = ctXXHash32(node.name);
            rigSignature.identifier =
              ctXXHash32(node.name, strlen(node.name), rigSignature.identifier);
         } else {
            nameEntry.hash = UINT32_MAX;
            nameEntry.strOffset = -1;
            rigSignature.identifier++; /* Just scramble it for each node like this */
         }
         boneNameOffs.Append(nameEntry);

         ctWADProtoBoneTransform finalData;
         finalData.position[0] = xform.position.x;
         finalData.position[1] = xform.position.y;
         finalData.position[2] = xform.position.z;
         finalData.rotation[0] = xform.rotation.x;
         finalData.rotation[1] = xform.rotation.y;
         finalData.rotation[2] = xform.rotation.z;
         finalData.rotation[3] = xform.rotation.w;
         finalData.scale[0] = xform.scale.x;
         finalData.scale[1] = xform.scale.y;
         finalData.scale[2] = xform.scale.z;
         boneXfwd.Append(finalData);
         outGltfNodes.Append(&gltf->nodes[i]);
      }

      rigSignature.boneCount = (uint32_t)boneXfwd.Count();

      MakeSection(CT_WADPROTO_NAME_RIG_SIGNATURE, sizeof(rigSignature), &rigSignature);
      MakeSection(CT_WADPROTO_NAME_BONE_TRANSFORM,
                  boneXfwd.Count() * sizeof(ctWADProtoBoneTransform),
                  boneXfwd.Data());
      MakeSection(CT_WADPROTO_NAME_BONE_PARENT,
                  boneParents.Count() * sizeof(boneParents[0]),
                  boneParents.Data());
      MakeSection(CT_WADPROTO_NAME_BONE_NAME,
                  boneNameOffs.Count() * sizeof(boneNameOffs[0]),
                  boneNameOffs.Data());
   }

   /* Lights */
   ctDynamicArray<ctWADProtoLight> lights;
   {
      if (gltf->lights_count) {
         for (size_t i = 0; i < gltf->lights_count; i++) {
            const cgltf_light light = gltf->lights[i];
            ctWADProtoLight outLight = ctWADProtoLight();

            if (light.type == cgltf_light_type_spot) {
               outLight.flags |= CT_WADPROTO_LIGHT_SPOT;
            }
            outLight.color[0] = light.color[0] * light.intensity;
            outLight.color[1] = light.color[1] * light.intensity;
            outLight.color[2] = light.color[2] * light.intensity;
            outLight.coneInner = light.spot_inner_cone_angle;
            outLight.coneOuter = light.spot_outer_cone_angle;
            lights.Append(outLight);
         }
         if (!lights.isEmpty()) {
            MakeSection(
              CT_WADPROTO_NAME_LIGHT, lights.Count() * sizeof(lights[0]), lights.Data());
         }
      }
   }

   /* Cameras */
   ctDynamicArray<ctWADProtoCamera> cameras;
   {
      if (gltf->cameras_count) {
         for (size_t i = 0; i < gltf->cameras_count; i++) {
            const cgltf_camera cam = gltf->cameras[i];
            ctWADProtoCamera outCam = ctWADProtoCamera();
            if (cam.type == cgltf_camera_type_perspective) {
               outCam.fovBase = cam.data.perspective.yfov;
               cameras.Append(outCam);
            }
         }
         if (!cameras.isEmpty()) {
            MakeSection(CT_WADPROTO_NAME_CAMERA,
                        cameras.Count() * sizeof(cameras[0]),
                        cameras.Data());
         }
      }
   }

   /* Materials */
   ctDynamicArray<ctWADProtoRenderMaterialScalar> matScalars;
   ctDynamicArray<ctWADProtoRenderMaterialVector> matVectors;
   ctDynamicArray<ctWADProtoRenderMaterialTexture> matTextures;
   ctDynamicArray<ctWADProtoRenderMaterialEntry> materials;
   {
      for (size_t i = 0; i < gltf->materials_count; i++) {
         const cgltf_material mat = gltf->materials[i];

         ctWADProtoRenderMaterialEntry matOut = {0};

         /* Name */
         matOut.nameStr = SaveString(mat.name);
         matOut.scalarBegin = (int32_t)matScalars.Count();
         matOut.vectorBegin = (int32_t)matVectors.Count();
         matOut.textureBegin = (int32_t)matTextures.Count();

         if (mat.unlit) {
            matOut.shaderNameStr = SaveString("defaultsurfaceunlit");
         } else {
            matOut.shaderNameStr = SaveString("defaultsurface");
         }

#define ADD_MAT_SCALAR(name, value)                                                      \
   {                                                                                     \
      ctWADProtoRenderMaterialScalar val = {0};                                          \
      val.nameStr = SaveString(name);                                                    \
      val.scalar = value;                                                                \
      matScalars.Append(val);                                                            \
   }

#define ADD_MAT_VEC4(name, value)                                                        \
   {                                                                                     \
      ctWADProtoRenderMaterialVector val = {0};                                          \
      val.nameStr = SaveString(name);                                                    \
      memcpy(val.vector, value, sizeof(float) * 4);                                      \
      matVectors.Append(val);                                                            \
   }

#define ADD_MAT_VEC3(name, value, w)                                                     \
   {                                                                                     \
      ctWADProtoRenderMaterialVector val = {0};                                          \
      val.nameStr = SaveString(name);                                                    \
      memcpy(val.vector, value, sizeof(float) * 3);                                      \
      matVectors.Append(val, w);                                                         \
   }

#define ADD_MAT_TEXTURE(name, value, w)                                                  \
   {                                                                                     \
      ctWADProtoRenderMaterialTexture val = {0};                                         \
      val.nameStr = SaveString(name);                                                    \
      val.xform = {1.0f, 1.0f, 0.0f, 0.0f};                                              \
      val.textureStr = SaveString(value);                                                \
   }

         if (mat.has_pbr_metallic_roughness) {
            /* Base Color and Alpha */
            ADD_MAT_VEC4("BaseColor", mat.pbr_metallic_roughness.base_color_factor);
         } else {
            ctDebugWarning("%s does not have base color!", mat.name);
            float defaultColor[] = {0.5f, 0.5f, 0.5f, 1.0f};
            ADD_MAT_VEC4("BaseColor", defaultColor);
         }
         if (mat.alpha_mode == cgltf_alpha_mode_mask) {
            ADD_MAT_SCALAR("AlphaCutoff", mat.alpha_cutoff);
         }
         /* Emission */
         ADD_MAT_VEC3("EmissionColor", mat.emissive_factor, 1);
         /* Roughness */
         if (mat.has_pbr_metallic_roughness) {
            ADD_MAT_SCALAR("Roughness", mat.pbr_metallic_roughness.roughness_factor);
         } else {
            ctDebugWarning("%s does not have roughness!", mat.name);
            ADD_MAT_SCALAR("Roughness", 0.5f);
         }

         if (mat.normal_texture.texture) {
            ADD_MAT_SCALAR("NormalMapStrength", mat.normal_texture.scale);
         }
         if (mat.occlusion_texture.texture) {
            ADD_MAT_SCALAR("OcclusionMapStrength", mat.occlusion_texture.scale);
         }

         /* Base Color Texture */
         // if (mat.has_pbr_metallic_roughness) {
         //   if (mat.pbr_metallic_roughness.base_color_texture.texture) {
         //      if (mat.pbr_metallic_roughness.base_color_texture.texture->image) {
         //         matOut.baseTexture = SaveString(
         //           ConvertImagePath(
         //             mat.pbr_metallic_roughness.base_color_texture.texture->image->uri)
         //             .CStr());
         //      }
         //   }
         //   /* PBR Texture */
         //   if (mat.pbr_metallic_roughness.metallic_roughness_texture.texture) {
         //      if (mat.pbr_metallic_roughness.metallic_roughness_texture.texture->image)
         //      {
         //         matOut.pbrTexture = SaveString(
         //           ConvertImagePath(mat.pbr_metallic_roughness.metallic_roughness_texture
         //                              .texture->image->uri)
         //             .CStr());
         //      }
         //   }
         //}
         ///* Normal Texture */
         // if (mat.normal_texture.texture) {
         //   if (mat.normal_texture.texture->image) {
         //      matOut.normalTexture =
         //        SaveString(ConvertImagePath(mat.normal_texture.texture->image->uri).CStr());
         //   }
         //}
         ///* Emissive Texture */
         // if (mat.emissive_texture.texture) {
         //   if (mat.emissive_texture.texture->image) {
         //      matOut.emissiveTexture = SaveString(
         //        ConvertImagePath(mat.emissive_texture.texture->image->uri).CStr());
         //   }
         //}

         // materials.Append(matOut);
      }
      if (gltf->materials_count) {
         MakeSection(
           "RMATV1", materials.Count() * sizeof(materials[0]), materials.Data());
      }
   }

   /* Mesh Data */
   ctDynamicArray<ctWADProtoRenderMesh> renderMeshes;
   ctDynamicArray<cgltf_mesh*> outGltfMeshes;
   if (outGPUPath) {
      ctStringUtf8 meshOutPath = outPath;
      FILE* pFile = fopen(outGPUPath, "wb");
      if (!pFile) { return -10; }

      /* Write actual mesh data */
      for (size_t meshIdx = 0; meshIdx < gltf->meshes_count; meshIdx++) {
         /* Streams */
         ctDynamicArray<ctKeyLimeStreamSubmesh> submeshes;
         ctDynamicArray<uint32_t> indexData;
         ctDynamicArray<ctKeyLimeStreamPosition> positionData;
         ctDynamicArray<ctKeyLimeStreamNormalTangent> normalTangentData;
         ctDynamicArray<ctKeyLimeStreamUV> uvData[4];
         ctDynamicArray<ctKeyLimeStreamColor> colorData[4];
         ctDynamicArray<ctKeyLimeStreamSkin> skinData;

         const cgltf_mesh mesh = gltf->meshes[meshIdx];

#define GPU_ALIGNMENT 64

         ctGeometryFormatHeader geoHeader = {};
         memcpy(geoHeader.magic, "GPU0", 4);
         geoHeader.alignment = GPU_ALIGNMENT;

         /* For each primitive */
         for (size_t primIdx = 0; primIdx < mesh.primitives_count; primIdx++) {
            cgltf_primitive prim = mesh.primitives[primIdx];
            /* Find accessors */
            cgltf_accessor* pPositionAccessor = NULL;
            cgltf_accessor* pNormalAccessor = NULL;
            cgltf_accessor* pTangentAccessor = NULL;
            cgltf_accessor* pTexcoordAccessors[4] = {0};
            cgltf_accessor* pColorAccessors[4] = {0};
            cgltf_accessor* pJointAccessor = NULL;
            cgltf_accessor* pWeightAccessor = NULL;
            for (size_t i = 0; i < prim.attributes_count; i++) {
               if (prim.attributes[i].type == cgltf_attribute_type_position) {
                  pPositionAccessor = prim.attributes[i].data;
               } else if (prim.attributes[i].type == cgltf_attribute_type_normal) {
                  pNormalAccessor = prim.attributes[i].data;
               } else if (prim.attributes[i].type == cgltf_attribute_type_tangent) {
                  pTangentAccessor = prim.attributes[i].data;
               } else if (prim.attributes[i].type == cgltf_attribute_type_texcoord) {
                  if (prim.attributes[i].index >= 4) { continue; }
                  pTexcoordAccessors[prim.attributes[i].index] = prim.attributes[i].data;
               } else if (prim.attributes[i].type == cgltf_attribute_type_color) {
                  if (prim.attributes[i].index >= 4) { continue; }
                  pColorAccessors[prim.attributes[i].index] = prim.attributes[i].data;
               } else if (prim.attributes[i].type == cgltf_attribute_type_joints) {
                  pJointAccessor = prim.attributes[i].data;
               } else if (prim.attributes[i].type == cgltf_attribute_type_weights) {
                  pWeightAccessor = prim.attributes[i].data;
               }
            }

            /* Reasons to ignore primitives */
            const char* ignorePrimReason = NULL;
            if (prim.type != cgltf_primitive_type_triangles) {
               ignorePrimReason = "Unsupported type";
            }
            if (!pPositionAccessor) { ignorePrimReason = "No positions"; }
            if (!prim.indices) { ignorePrimReason = "No index buffer"; }
            if (!prim.material) { ignorePrimReason = "No material"; }
            if (prim.has_draco_mesh_compression) {
               ignorePrimReason = "Draco compression unsupported";
            }
            for (size_t i = 0; i < prim.attributes_count; i++) {
               if (prim.attributes[i].data->is_sparse) {
                  ignorePrimReason = "Sparse accessors unsupported";
               }
            }
            if (ignorePrimReason) {
               ctDebugWarning("Primitive on #%d ignored (%s)...", (int)primIdx);
               continue;
            }

            /* Reserve for performance */
            const size_t vertexCount = pPositionAccessor->count;
            indexData.Reserve(prim.indices->count);
            positionData.Reserve(vertexCount);
            normalTangentData.Reserve(vertexCount);
            skinData.Reserve(vertexCount);
            for (int i = 0; i < 4; i++) {
               uvData[i].Reserve(vertexCount);
               colorData[i].Reserve(vertexCount);
            }

            /* Submesh Data */
            ctKeyLimeStreamSubmesh submesh;
            submesh.idxOffset = (int32_t)indexData.Count();
            submesh.idxCount = (int32_t)prim.indices->count;
            submesh.matIdx = (int32_t)(prim.material - gltf->materials);
            submeshes.Append(submesh);

            /* Index Data */
            for (size_t i = 0; i < prim.indices->count; i++) {
               indexData.Append((int32_t)cgltf_accessor_read_index(prim.indices, i));
            }

            /* Position Data */
            ctBoundBox bbox = ctBoundBox();
            for (size_t i = 0; i < pPositionAccessor->count; i++) {
               ctKeyLimeStreamPosition entry = {0};
               cgltf_accessor_read_float(pPositionAccessor, i, entry.position, 3);
               positionData.Append(entry);
               bbox.AddPoint(entry.position);
            }
            ctBoundSphere bsphere = bbox;
            geoHeader.cener[0] = bsphere.position.x;
            geoHeader.cener[1] = bsphere.position.y;
            geoHeader.cener[2] = bsphere.position.z;
            geoHeader.radius = bsphere.radius;

            /* UV Data */
            for (int aidx = 0; aidx < 4; aidx++) {
               if (pTexcoordAccessors[aidx]) {
                  for (size_t i = 0; i < pTexcoordAccessors[aidx]->count; i++) {
                     ctKeyLimeStreamUV entry = {0};
                     cgltf_accessor_read_float(pTexcoordAccessors[aidx], i, entry.uv, 2);
                     uvData[aidx].Append(entry);
                  }
                  geoHeader.uvChannelCount++;
               }
            }

            /* Color Data */
            for (int aidx = 0; aidx < 4; aidx++) {
               if (pColorAccessors[aidx]) {
                  for (size_t i = 0; i < pColorAccessors[aidx]->count; i++) {
                     ctKeyLimeStreamColor entry = {0};
                     float fData[4] = {0};
                     cgltf_accessor_read_float(pColorAccessors[aidx], i, fData, 4);
                     for (int j = 0; j < 4; j++) {
                        entry.color[j] = (uint8_t)fData[j] * 255;
                     }
                     colorData[aidx].Append(entry);
                  }
                  geoHeader.colorChannelCount++;
               }
            }

            // Todo: Other Data...

            geoHeader.submeshCount++;
            geoHeader.indexCount += (uint32_t)prim.indices->count;
            geoHeader.vertexCount += (uint32_t)pPositionAccessor->count;
         }

         /* Write data and build offsets */
#define WRITE_DYN_ARRAY(_arr)                                                            \
   if (!_arr.isEmpty()) { fwrite(_arr.Data(), sizeof(_arr[0]), _arr.Count(), pFile); }

         int _pad = 0;
#define WRITE_PADDING()                                                                  \
   {                                                                                     \
      long amount = GPU_ALIGNMENT - (ftell(pFile) % GPU_ALIGNMENT);                      \
      for (long i = 0; i < amount; i++) {                                                \
         fwrite(&_pad, 1, 1, pFile);                                                     \
      }                                                                                  \
   }

         uint64_t _bOffset =
           sizeof(ctGeometryFormatHeader) +
           (GPU_ALIGNMENT - (sizeof(ctGeometryFormatHeader) % GPU_ALIGNMENT));
#define GET_OFFSET(_var, _arr)                                                           \
   if (_arr.isEmpty()) {                                                                 \
      _var = UINT32_MAX;                                                                 \
   } else {                                                                              \
      _var = _bOffset;                                                                   \
      _bOffset += sizeof(_arr[0]) * _arr.Count();                                        \
      _bOffset += GPU_ALIGNMENT - (_bOffset % GPU_ALIGNMENT);                            \
   }
         GET_OFFSET(geoHeader.submeshOffset, submeshes);
         GET_OFFSET(geoHeader.indexOffset, indexData);
         GET_OFFSET(geoHeader.positionOffset, positionData);
         GET_OFFSET(geoHeader.tangentNormalOffset, normalTangentData);
         GET_OFFSET(geoHeader.skinOffset, skinData);
         for (int i = 0; i < 4; i++) {
            GET_OFFSET(geoHeader.uvOffsets[i], uvData[i]);
         }
         for (int i = 0; i < 4; i++) {
            GET_OFFSET(geoHeader.colorOffsets[i], uvData[i]);
         }

         fwrite(&geoHeader, sizeof(geoHeader), 1, pFile);
         WRITE_PADDING();
         WRITE_DYN_ARRAY(submeshes);
         WRITE_PADDING();
         WRITE_DYN_ARRAY(indexData);
         WRITE_PADDING();
         WRITE_DYN_ARRAY(positionData);
         WRITE_PADDING();
         WRITE_DYN_ARRAY(normalTangentData);
         WRITE_PADDING();
         WRITE_DYN_ARRAY(skinData);
         WRITE_PADDING();
         for (int i = 0; i < 4; i++) {
            WRITE_DYN_ARRAY(uvData[i]);
            WRITE_PADDING();
         }
         for (int i = 0; i < 4; i++) {
            WRITE_DYN_ARRAY(colorData[i]);
            WRITE_PADDING();
         }
#undef WRITE_DYN_ARRAY
#undef GET_OFFSET

         ctWADProtoRenderMesh renderMesh = {};
         renderMesh.filePath = SaveString("gpu");
         renderMeshes.Append(renderMesh);
         outGltfMeshes.Append(&gltf->meshes[meshIdx]);
      }
      fclose(pFile);

      MakeSection(CT_WADPROTO_NAME_RENDER_MESH,
                  renderMeshes.Count() * sizeof(ctWADProtoRenderMesh),
                  renderMeshes.Data());
   }

   /* Mesh instances */
   ctDynamicArray<ctWADProtoRenderMeshInstance> renderMeshInstances;
   {
      for (size_t i = 0; i < outGltfNodes.Count(); i++) {
         const cgltf_node* node = outGltfNodes[i];
         if (node->mesh) {
            ctWADProtoRenderMeshInstance inst = {0};
            inst.boneIdx = (int32_t)i;
            inst.meshIdx = (int32_t)outGltfMeshes.FindIndex(node->mesh);
            renderMeshInstances.Append(inst);
         }
      }
      MakeSection(CT_WADPROTO_NAME_RENDER_MESH_INSTANCE,
                  renderMeshInstances.Count() * sizeof(ctWADProtoRenderMeshInstance),
                  renderMeshInstances.Data());
   }

   /* Physics Shape Cook */
   {
      PxTolerancesScale toleranceScale;
      PxFoundation* pFoundation;
      PxPhysics* pPhysics;
      PxCooking* pCooking;
      toleranceScale = PxTolerancesScale();
      PxDefaultErrorCallback defaultErrorCallback;
      PxDefaultAllocator defaultAllocatorCallback;

      pFoundation = PxCreateFoundation(
        PX_PHYSICS_VERSION, defaultAllocatorCallback, defaultErrorCallback);
      if (!pFoundation) { ctFatalError(-1, "PxCreateFoundation failed!"); }

      pPhysics = PxCreateBasePhysics(
        PX_PHYSICS_VERSION, *pFoundation, toleranceScale, false, NULL);
      if (!pPhysics) { ctFatalError(-1, "PxCreatePhysics failed!"); }

      PxRegisterArticulations(*pPhysics);
      PxRegisterHeightFields(*pPhysics);

      pCooking = PxCreateCooking(
        PX_PHYSICS_VERSION, *pFoundation, PxCookingParams(toleranceScale));
      if (!pCooking) { ctFatalError(-1, "PxCreateCooking failed!"); }

      if (!PxInitExtensions(*pPhysics, NULL)) {
         ctFatalError(-1, "PxInitExtensions failed!");
      }

      MakeSection(CT_WADBLOB_NAME_PHYSX_COOK, 0, NULL);
   }

   /* Embedded Data */
   ctDynamicArray<void*> embeds;
   {
      MakeSection(CT_WADMARKER_NAME_EMBED_START, 0, NULL);
      int embedIdx = 0;
      const char* embedPath = "";
      const char* embedAlias = "";
      do {
         embedPath = FindParamOccurance("-embp", embedIdx);
         embedAlias = FindParamOccurance("-emba", embedIdx);
         if (embedPath && embedAlias) {
            FILE* pFile = fopen(embedPath, "rb");
            if (pFile) {
               fseek(pFile, 0, SEEK_END);
               size_t fsize = ftell(pFile);
               fseek(pFile, 0, SEEK_SET);
               void* data = ctMalloc(fsize);
               fread(data, 1, fsize, pFile);
               fclose(pFile);
               char alias[8];
               memset(alias, 0, 8);
               strncpy(alias, embedAlias, 8);
               embeds.Append(data);
               if (data) { MakeSection(alias, fsize, data); }
            }
            embedIdx++;
         }
      } while (embedPath && embedAlias);
      MakeSection(CT_WADMARKER_NAME_EMBED_END, 0, NULL);
   }

   /* Write Strings Section */
   MakeSection(CT_WADBLOB_NAME_STRINGS, gStringsContent.Count(), gStringsContent.Data());

   /* Write WAD */
   {
      ctWADInfo wadInfo = {
        {'P', 'W', 'A', 'D'}, (int32_t)gWadSections.Count(), sizeof(ctWADInfo)};
      FILE* pFile = fopen(outPath, "wb");
      if (!pFile) { return -10; }
      fwrite(&wadInfo, sizeof(wadInfo), 1, pFile);
      for (size_t i = 0; i < gWadSections.Count(); i++) {
         /* Make absolute offset for non-markers */
         if (gWadSections[i].lump.size > 0) {
            gWadSections[i].lump.filepos +=
              (int32_t)(sizeof(wadInfo) + sizeof(ctWADLump) * gWadSections.Count());
         } else {
            gWadSections[i].lump.filepos = 0;
         }
         fwrite(&gWadSections[i].lump, sizeof(ctWADLump), 1, pFile);
      }
      for (size_t i = 0; i < gWadSections.Count(); i++) {
         if (gWadSections[i].data) {
            fwrite(gWadSections[i].data, gWadSections[i].lump.size, 1, pFile);
         }
      }
      ctDebugLog("Wrote %s with %d sections", outPath, (int)gWadSections.Count());
   }

   for (size_t i = 0; i < embeds.Count(); i++) {
      ctFree(embeds[i]);
   }

   return 0;
}