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

#include "core/Application.hpp"
#include "core/Translation.hpp"

#include "middleware/Im3dIntegration.hpp"
#include "middleware/ImguiIntegration.hpp"

#include "scene/SceneEngine.hpp"

enum ctModelViewerMeshMode {
   CT_MODELVIEW_SUBMESH,
   CT_MODELVIEW_WIREFRAME,
   CT_MODELVIEW_NORMAL,
   CT_MODELVIEW_TANGENT,
   CT_MODELVIEW_TANGENT_SIGN,
   CT_MODELVIEW_UV,
   CT_MODELVIEW_COLOR,
   CT_MODELVIEW_BONE,
   CT_MODELVIEW_MATERIAL
};

const char* meshModeNames[] {"Submesh",
                             "Wireframe",
                             "Normal",
                             "Tangent",
                             "Tangent Sign",
                             "UV",
                             "Color",
                             "Bone",
                             "Material"};

class ctModelViewer : public ctApplication {
public:
   virtual const char* GetAppName() {
      return "ModelViewer";
   }
   virtual const char* GetAppDeveloperName() {
      return "CitrusToolbox";
   }
   virtual ctAppVersion GetAppVersion() {
      return ctAppVersion();
   }

   ctModel model;

   virtual ctResults OnStartup();
   virtual ctResults OnUIUpdate();
   virtual ctResults OnTick(float deltaTime) {
      timer += deltaTime;
      return CT_SUCCESS;
   }
   virtual ctResults OnShutdown();

   /* GUI */
   void SkeletonInfo();
   void BoneInfo(int32_t boneIdx);
   void GeometryInfo();
   void AnimationInfo();
   void SplinesInfo();
   void PhysXInfo();
   void NavmeshInfo();
   void MaterialInfo();
   void SceneInfo();

   /* Debug Rendering */
   void RenderModel();
   void RenderSkeleton();
   void RenderBone(int32_t boneIdx);
   void RenderBoundingBoxes();
   void RenderGeometry();

   /* Settings */
   bool renderSkeleton = false;
   bool renderBoneNames = false;
   bool renderGeometry = true;
   bool renderBBOX = false;
   ctModelViewerMeshMode viewMode = CT_MODELVIEW_COLOR;
   int32_t uvChannel = 0;
   int32_t colorChannel = 0;
   int32_t activeBone = 0;
   int32_t lodView = 0;

   ctCameraInfo camera;
   float timer;
};

ctResults ctGltf2Model::ModelViewer(int argc, char* argv[]) {
   ctModelViewer* modelViewer = new ctModelViewer();
   modelViewer->model = model;
   modelViewer->Execute(argc, argv);
   delete modelViewer;
   return CT_SUCCESS;
}

ctResults ctModelViewer::OnStartup() {
   ctSceneEngine* scene;
   scene = (ctSceneEngine*)Engine->SceneEngine;
   scene->EnableCameraOverride();
   camera.position = ctVec3(0.0f, 1.0f, -10.0f);
   scene->SetCameraOverride(camera);
   return CT_SUCCESS;
}

ctResults ctModelViewer::OnUIUpdate() {
   ZoneScoped;
   ImGui::Begin(CT_NC("Model"));
   if (ImGui::CollapsingHeader(CT_NC("Skeleton"))) { SkeletonInfo(); }
   if (ImGui::CollapsingHeader(CT_NC("Geometry"))) { GeometryInfo(); }
   if (ImGui::CollapsingHeader(CT_NC("Animation"))) { AnimationInfo(); }
   if (ImGui::CollapsingHeader(CT_NC("Splines"))) { SplinesInfo(); }
   if (ImGui::CollapsingHeader(CT_NC("Material"))) { MaterialInfo(); }
   if (ImGui::CollapsingHeader(CT_NC("PhysX"))) { PhysXInfo(); }
   if (ImGui::CollapsingHeader(CT_NC("Navmesh"))) { NavmeshInfo(); }
   if (ImGui::CollapsingHeader(CT_NC("Scene"))) { SceneInfo(); }
   ImGui::End();
   RenderModel();
   return CT_SUCCESS;
}

ctResults ctModelViewer::OnShutdown() {
   return CT_SUCCESS;
}

void ctModelViewer::SkeletonInfo() {
   ImGui::PushID("BONE_TREE");
   ImGui::Checkbox(CT_NC("Render Skeleton"), &renderSkeleton);
   ImGui::Checkbox(CT_NC("Render Bone Names"), &renderBoneNames);
   ImGui::Text("Bone Count: %u", model.skeleton.boneCount);
   for (uint32_t i = 0; i < model.skeleton.boneCount; i++) {
      if (model.skeleton.graphArray[i].parent == -1) { BoneInfo(i); }
   }
   ImGui::PopID();
}

void ctModelViewer::BoneInfo(int32_t boneIdx) {
   ctModelSkeletonBoneGraph& graph = model.skeleton.graphArray[boneIdx];
   ctModelSkeletonBoneTransform& transform = model.skeleton.transformArray[boneIdx];
   ctModelSkeletonBoneTransform& invbind = model.skeleton.inverseBindArray[boneIdx];
   ctModelSkeletonBoneName& name = model.skeleton.nameArray[boneIdx];
   uint32_t hash = model.skeleton.hashArray[boneIdx];

   char namebuff[33];
   memset(namebuff, 0, 33);
   strncpy(namebuff, name.name, 32);
   if (ImGui::TreeNode(namebuff)) {
      if (ImGui::Button("Show Weights")) {
         activeBone = boneIdx;
         viewMode = CT_MODELVIEW_BONE;
      }
      ImGui::Text("Hash: %u", hash);
      ImGui::Text("Graph: %d %d %d", graph.parent, graph.firstChild, graph.nextSibling);
      ImGui::Text("Translation: %f %f %f",
                  transform.translation.x,
                  transform.translation.y,
                  transform.translation.z);
      ImGui::Text("Rotation: %f %f %f %f",
                  transform.rotation.x,
                  transform.rotation.y,
                  transform.rotation.z,
                  transform.rotation.w);
      ImGui::Text(
        "Scale: %f %f %f", transform.scale.x, transform.scale.y, transform.scale.z);
      ImGui::Text("Inverse Bind Translation: %f %f %f",
                  invbind.translation.x,
                  invbind.translation.y,
                  invbind.translation.z);
      ImGui::Text("Inverse Bind Rotation: %f %f %f %f",
                  invbind.rotation.x,
                  invbind.rotation.y,
                  invbind.rotation.z,
                  invbind.rotation.w);
      ImGui::Text("Inverse Bind Scale: %f %f %f",
                  invbind.scale.x,
                  invbind.scale.y,
                  invbind.scale.z);

      if (graph.firstChild != -1) { BoneInfo(graph.firstChild); }
      ImGui::TreePop();
   }
   if (graph.nextSibling != -1) { BoneInfo(graph.nextSibling); }
}

void ctModelViewer::GeometryInfo() {
   ImGui::PushID("GEO_TREE");
   ImGui::Checkbox(CT_NC("Render Geometry"), &renderGeometry);
   ImGui::Checkbox(CT_NC("Render Bounding Boxes"), &renderBBOX);
   ImGui::Combo(CT_NC("Geometry View"),
                (int*)&viewMode,
                meshModeNames,
                ctCStaticArrayLen(meshModeNames));
   ImGui::SliderInt(CT_NC("LOD Level"), &lodView, 0, 3);
   if (viewMode == CT_MODELVIEW_UV) {
      ImGui::SliderInt(CT_NC("UV Channel"), &uvChannel, 0, 3);
   }
   if (viewMode == CT_MODELVIEW_COLOR) {
      ImGui::SliderInt(CT_NC("Color Channel"), &colorChannel, 0, 3);
   }
   if (viewMode == CT_MODELVIEW_BONE) {
      ImGui::SliderInt(
        CT_NC("Active Bone"), &activeBone, 0, model.skeleton.boneCount - 1);
   }

   for (uint32_t meshIdx = 0; meshIdx < model.geometry.meshCount; meshIdx++) {
      ctModelMesh& mesh = model.geometry.meshes[meshIdx];
      char namebuff[33];
      memset(namebuff, 0, 33);
      strncpy(namebuff, mesh.name, 32);
      if (ImGui::TreeNode(namebuff)) {
         for (uint32_t lodIdx = 0; lodIdx < mesh.lodCount; lodIdx++) {
            ctModelMeshLod& lod = mesh.lods[lodIdx];
            snprintf(namebuff, 32, "LOD %d", lodIdx);
            if (ImGui::TreeNode(namebuff)) {
               ImGui::Text("Bounding Box: %f %f %f - %f %f %f",
                           lod.bbox.min.x,
                           lod.bbox.min.y,
                           lod.bbox.min.z,
                           lod.bbox.max.x,
                           lod.bbox.max.y,
                           lod.bbox.max.z);
               ImGui::Text("Radius: %f", lod.radius);

               ImGui::Text("Submesh Count: %u", lod.submeshCount);
               for (uint32_t submeshIdx = 0; submeshIdx < lod.submeshCount;
                    submeshIdx++) {
                  snprintf(namebuff, 32, "Submesh %u", submeshIdx);
                  if (ImGui::TreeNode(namebuff)) {
                     ctModelSubmesh& submesh =
                       model.geometry.submeshes[lod.submeshStart + submeshIdx];
                     ImGui::Text("Material Index: %u", submesh.materialIndex);
                     ImGui::Text("Index Count: %u", submesh.indexCount);
                     ImGui::Text("Index Offset: %u", submesh.indexOffset);
                     ImGui::Text("Vertex Offset: %u", submesh.vertexOffset);
                     ImGui::TreePop();
                  }
               }
               ImGui::TreePop();
            }

            ImGui::Text("Morph Count: %d", lod.morphTargetCount);
            for (uint32_t morphIdx = 0; morphIdx < lod.morphTargetCount; morphIdx++) {
               ctModelMeshMorphTarget& morph =
                 model.geometry.morphTargets[lod.morphTargetStart + morphIdx];
               memset(namebuff, 0, 33);
               strncpy(namebuff, morph.name, 32);
               if (ImGui::TreeNode(namebuff)) {
                  ImGui::Text("Bounding Box: %f %f %f - %f %f %f",
                              morph.bboxDisplacement.min.x,
                              morph.bboxDisplacement.min.y,
                              morph.bboxDisplacement.min.z,
                              morph.bboxDisplacement.max.x,
                              morph.bboxDisplacement.max.y,
                              morph.bboxDisplacement.max.z);
                  ImGui::Text("Radius: %f", morph.radiusDisplacement);
                  ImGui::Text("Vertex Count: %u", morph.vertexCount);
                  ImGui::Text("Vertex Offset: %u", morph.vertexCount);
                  ImGui::TreePop();
               }
            }
         }
         ImGui::TreePop();
      }
   }
   ImGui::PopID();
}

void ctModelViewer::AnimationInfo() {
}

void ctModelViewer::SplinesInfo() {
}

void ctModelViewer::PhysXInfo() {
}

void ctModelViewer::NavmeshInfo() {
}

void ctModelViewer::MaterialInfo() {
   if (!model.materialSet.data) {
      ImGui::Text("No material data");
   } else {
      ImGui::TextUnformatted((const char*)model.materialSet.data);
   }
}

void ctModelViewer::SceneInfo() {
   if (!model.sceneScript.data) {
      ImGui::Text("No scene data");
   } else {
      ImGui::TextUnformatted((const char*)model.sceneScript.data);
   }
}

void ctModelViewer::RenderModel() {
   ZoneScoped;
   ctQuat quat = ctQuatYawPitchRoll(timer, 0.0f, 0.0f);
   Im3d::PushMatrix(ctMat4ToIm3d(ctMat4FromQuat(quat)));
   if (renderSkeleton) { RenderSkeleton(); }
   if (renderBBOX) { RenderBoundingBoxes(); }
   if (renderGeometry) { RenderGeometry(); }
   Im3d::PopMatrix();
}

void ctModelViewer::RenderSkeleton() {
   ZoneScoped;
   for (uint32_t i = 0; i < model.skeleton.boneCount; i++) {
      if (model.skeleton.graphArray[i].parent == -1) { RenderBone(i); }
   }
}

void ctModelViewer::RenderBone(int32_t boneIdx) {
   ZoneScoped;
   ctModelSkeletonBoneGraph& graph = model.skeleton.graphArray[boneIdx];
   ctModelSkeletonBoneTransform& transform = model.skeleton.inverseBindArray[boneIdx];
   ctMat4 matrix = ctMat4Identity();
   /* todo: calculate world transform */
   ctMat4Scale(matrix, transform.scale);
   ctMat4Rotate(matrix, transform.rotation);
   ctMat4Translate(matrix, transform.translation);
   ctQuat quat = ctQuatYawPitchRoll(timer, 0.0f, 0.0f);
   Im3d::PushMatrix(ctMat4ToIm3d((ctMat4FromQuat(quat) * matrix)));
   Im3d::DrawXyzAxes();
   if (renderBoneNames) {
      char namebuff[33];
      memset(namebuff, 0, 33);
      strncpy(namebuff, model.skeleton.nameArray[boneIdx].name, 32);
      Im3d::PushColor(ctVec4ToIm3dColor(CT_COLOR_WHITE));
      Im3d::Text(ctVec3ToIm3d(ctVec3()), Im3d::TextFlags_Default, namebuff);
      Im3d::PopColor();
   }
   Im3d::PopMatrix();
   if (graph.firstChild != -1) { RenderBone(graph.firstChild); }
   if (graph.nextSibling != -1) { RenderBone(graph.nextSibling); }
}

void ctModelViewer::RenderBoundingBoxes() {
   ZoneScoped;
   for (uint32_t i = 0; i < model.geometry.meshCount; i++) {
      uint32_t lodSlot = (uint32_t)lodView < model.geometry.meshes[i].lodCount
                           ? lodView
                           : model.geometry.meshes[i].lodCount - 1;
      ctRandomGenerator rng =
        ctRandomGenerator(ctXXHash32(model.geometry.meshes[i].name, 32));
      Im3d::PushColor(ctVec4ToIm3dColor(rng.GetColor()));
      Im3d::DrawAlignedBox(ctVec3ToIm3d(model.geometry.meshes[i].lods[lodSlot].bbox.min),
                           ctVec3ToIm3d(model.geometry.meshes[i].lods[lodSlot].bbox.max));
      Im3d::PopColor();
   }
}

void ctModelViewer::RenderGeometry() {
   ZoneScoped;
   for (uint32_t meshIdx = 0; meshIdx < model.geometry.meshCount; meshIdx++) {
      /* setup mesh */
      ctModelMesh& mesh = model.geometry.meshes[meshIdx];

      /* select lod*/
      uint32_t lodSlot = (uint32_t)lodView < mesh.lodCount ? lodView : mesh.lodCount - 1;
      ctModelMeshLod& lod = mesh.lods[lodSlot];

      /* map arrays */
      uint16_t* indices =
        (uint16_t*)&model.inMemoryGeometryData[model.gpuTable.indexDataStart];
      ctModelMeshVertexCoords* coords =
        (ctModelMeshVertexCoords*)&model
          .inMemoryGeometryData[model.gpuTable.vertexDataCoordsStart];
      ctModelMeshVertexUV* uvs =
        (ctModelMeshVertexUV*)&model
          .inMemoryGeometryData[model.gpuTable.vertexDataUVStart];
      ctModelMeshVertexColor* colors =
        (ctModelMeshVertexColor*)&model
          .inMemoryGeometryData[model.gpuTable.vertexDataColorStart];
      ctModelMeshVertexSkinData* skins =
        (ctModelMeshVertexSkinData*)&model
          .inMemoryGeometryData[model.gpuTable.vertexDataSkinStart];

      coords += lod.vertexDataCoordsStart;
      skins = lod.vertexDataSkinDataStart != UINT32_MAX
                ? skins + lod.vertexDataSkinDataStart
                : NULL;
      uvs = lod.vertexDataUVStarts[uvChannel] != UINT32_MAX
              ? uvs + lod.vertexDataUVStarts[uvChannel]
              : NULL;
      colors = lod.vertexDataColorStarts[colorChannel] != UINT32_MAX
                 ? colors + lod.vertexDataColorStarts[colorChannel]
                 : NULL;
      ctBoundBox bbox = lod.bbox;
      ctBoundBox2D uvbox = lod.uvbox[uvChannel];

      /* for each submesh */
      for (uint32_t submeshIdx = 0; submeshIdx < lod.submeshCount; submeshIdx++) {
         ctModelSubmesh submesh = model.geometry.submeshes[lod.submeshStart + submeshIdx];

         /* for each index */
         switch (viewMode) {
            case CT_MODELVIEW_SUBMESH: {
               /* for now just do random colors */
               ctRandomGenerator rng =
                 ctRandomGenerator(ctXXHash32(mesh.name, 32) + submeshIdx);
               Im3d::PushColor(ctVec4ToIm3dColor(rng.GetColor()));
               Im3d::BeginTriangles();
               for (uint32_t i = 0; i < submesh.indexCount; i++) {
                  uint32_t idx =
                    (uint32_t)indices[i + submesh.indexOffset] + submesh.vertexOffset;
                  ctModelMeshVertexCoords coord = coords[idx];

                  TinyImageFormat_DecodeInput posDecode = TinyImageFormat_DecodeInput();
                  posDecode.pixel = coord.position;
                  ctVec3 position;
                  TinyImageFormat_DecodeLogicalPixelsF(
                    TinyImageFormat_R16G16B16_UNORM, &posDecode, 1, position.data);
                  position = bbox.DeNormalizeVector(position);
                  Im3d::Vertex(ctVec3ToIm3d(position));
               }
               Im3d::End();
               Im3d::PopColor();
            } break;
            case CT_MODELVIEW_WIREFRAME: {
               Im3d::PushColor(ctVec4ToIm3dColor(CT_COLOR_WHITE));
               Im3d::BeginLines();
               for (uint32_t i = 0; i < submesh.indexCount / 3; i++) {
                  for (uint32_t j = 0; j < 3; j++) {
                     uint32_t idx0 = (uint32_t)indices[submesh.indexOffset + i * 3 + j] +
                                     submesh.vertexOffset;
                     uint32_t idx1 =
                       (uint32_t)indices[submesh.indexOffset + i * 3 + ((j + 1) % 3)] +
                       submesh.vertexOffset;
                     ctModelMeshVertexCoords coord0 = coords[idx0];
                     ctModelMeshVertexCoords coord1 = coords[idx1];

                     TinyImageFormat_DecodeInput posDecode =
                       TinyImageFormat_DecodeInput();
                     posDecode.pixel = coord0.position;
                     ctVec3 pos1;
                     TinyImageFormat_DecodeLogicalPixelsF(
                       TinyImageFormat_R16G16B16_UNORM, &posDecode, 1, pos1.data);
                     posDecode.pixel = coord1.position;
                     ctVec3 pos2;
                     TinyImageFormat_DecodeLogicalPixelsF(
                       TinyImageFormat_R16G16B16_UNORM, &posDecode, 1, pos2.data);
                     pos1 = bbox.DeNormalizeVector(pos1);
                     pos2 = bbox.DeNormalizeVector(pos2);
                     Im3d::Vertex(ctVec3ToIm3d(pos1));
                     Im3d::Vertex(ctVec3ToIm3d(pos2));
                  }
               }
               Im3d::End();
               Im3d::PopColor();
            } break;
            case CT_MODELVIEW_NORMAL: {
               Im3d::PushColor();
               Im3d::BeginTriangles();
               for (uint32_t i = 0; i < submesh.indexCount; i++) {
                  uint32_t idx =
                    (uint32_t)indices[i + submesh.indexOffset] + submesh.vertexOffset;
                  ctModelMeshVertexCoords coord = coords[idx];

                  ctVec3 position;
                  ctVec4 normal;

                  TinyImageFormat_DecodeInput decode = TinyImageFormat_DecodeInput();
                  decode.pixel = coord.position;
                  TinyImageFormat_DecodeLogicalPixelsF(
                    TinyImageFormat_R16G16B16_UNORM, &decode, 1, position.data);
                  position = bbox.DeNormalizeVector(position);
                  decode.pixel = &coord.normal;
                  TinyImageFormat_DecodeLogicalPixelsF(
                    TinyImageFormat_R10G10B10A2_UNORM, &decode, 1, normal.data);
                  normal.w = 1.0f;
                  Im3d::SetColor(ctVec4ToIm3dColor(normal));
                  Im3d::Vertex(ctVec3ToIm3d(position));
               }
               Im3d::End();
               Im3d::PopColor();
            } break;
            case CT_MODELVIEW_TANGENT:
            case CT_MODELVIEW_TANGENT_SIGN: {
               Im3d::PushColor();
               Im3d::BeginTriangles();
               for (uint32_t i = 0; i < submesh.indexCount; i++) {
                  uint32_t idx =
                    (uint32_t)indices[i + submesh.indexOffset] + submesh.vertexOffset;
                  ctModelMeshVertexCoords coord = coords[idx];

                  ctVec3 position;
                  ctVec4 tangent = ctVec4();

                  TinyImageFormat_DecodeInput decode = TinyImageFormat_DecodeInput();
                  decode.pixel = coord.position;
                  TinyImageFormat_DecodeLogicalPixelsF(
                    TinyImageFormat_R16G16B16_UNORM, &decode, 1, position.data);
                  position = bbox.DeNormalizeVector(position);
                  decode.pixel = &coord.tangent;
                  TinyImageFormat_DecodeLogicalPixelsF(
                    TinyImageFormat_R10G10B10A2_UNORM, &decode, 1, tangent.data);

                  if (viewMode == CT_MODELVIEW_TANGENT_SIGN) {
                     tangent = ctVec4(ctVec3(tangent.w), 1.0f);
                  }
                  tangent.w = 1.0f;

                  Im3d::SetColor(ctVec4ToIm3dColor(tangent));
                  Im3d::Vertex(ctVec3ToIm3d(position));
               }
               Im3d::End();
               Im3d::PopColor();
            } break;
            case CT_MODELVIEW_UV: {
               Im3d::PushColor();
               Im3d::BeginTriangles();
               for (uint32_t i = 0; i < submesh.indexCount; i++) {
                  uint32_t idx =
                    (uint32_t)indices[i + submesh.indexOffset] + submesh.vertexOffset;
                  ctModelMeshVertexCoords coord = coords[idx];
                  ctModelMeshVertexUV uvc = uvs ? uvs[idx] : ctModelMeshVertexUV();

                  ctVec3 position;
                  ctVec2 uv;

                  float buff[4];
                  TinyImageFormat_DecodeInput decode = TinyImageFormat_DecodeInput();
                  decode.pixel = coord.position;
                  TinyImageFormat_DecodeLogicalPixelsF(
                    TinyImageFormat_R16G16B16_UNORM, &decode, 1, position.data);
                  position = bbox.DeNormalizeVector(position);
                  decode.pixel = uvc.uv;
                  TinyImageFormat_DecodeLogicalPixelsF(
                    TinyImageFormat_R16G16_UNORM, &decode, 1, buff);
                  uv.x = buff[0];
                  uv.y = buff[1];
                  uv = uvbox.DeNormalizeVector(uv);
                  Im3d::SetColor(ctVec4ToIm3dColor(ctVec4(uv, 0.0f, 1.0f)));
                  Im3d::Vertex(ctVec3ToIm3d(position));
               }
               Im3d::End();
               Im3d::PopColor();
            } break;
            case CT_MODELVIEW_COLOR: {
               Im3d::PushColor();
               Im3d::BeginTriangles();
               for (uint32_t i = 0; i < submesh.indexCount; i++) {
                  uint32_t idx =
                    (uint32_t)indices[i + submesh.indexOffset] + submesh.vertexOffset;
                  ctModelMeshVertexCoords coord = coords[idx];
                  ctModelMeshVertexColor colorc =
                    colors ? colors[idx] : ctModelMeshVertexColor();

                  ctVec3 position;
                  ctVec4 color;

                  TinyImageFormat_DecodeInput decode = TinyImageFormat_DecodeInput();
                  decode.pixel = coord.position;
                  TinyImageFormat_DecodeLogicalPixelsF(
                    TinyImageFormat_R16G16B16_UNORM, &decode, 1, position.data);
                  position = bbox.DeNormalizeVector(position);
                  decode.pixel = colorc.rgba;
                  TinyImageFormat_DecodeLogicalPixelsF(
                    TinyImageFormat_R8G8B8A8_UNORM, &decode, 1, color.data);
                  Im3d::SetColor(ctVec4ToIm3dColor(color));
                  Im3d::Vertex(ctVec3ToIm3d(position));
               }
               Im3d::End();
               Im3d::PopColor();
            } break;
            case CT_MODELVIEW_BONE: {
               Im3d::PushColor();
               Im3d::BeginTriangles();
               for (uint32_t i = 0; i < submesh.indexCount; i++) {
                  uint32_t idx =
                    (uint32_t)indices[i + submesh.indexOffset] + submesh.vertexOffset;
                  ctModelMeshVertexCoords coord = coords[idx];
                  ctModelMeshVertexSkinData skin =
                    skins ? skins[idx] : ctModelMeshVertexSkinData();

                  ctVec3 position;
                  float weight = 0.0f;
                  for (int b = 0; b < 4; b++) {
                     if (skin.indices[b] == activeBone) { weight += skin.weights[b]; }
                  }

                  TinyImageFormat_DecodeInput decode = TinyImageFormat_DecodeInput();
                  decode.pixel = coord.position;
                  TinyImageFormat_DecodeLogicalPixelsF(
                    TinyImageFormat_R16G16B16_UNORM, &decode, 1, position.data);
                  position = bbox.DeNormalizeVector(position);

                  Im3d::SetColor(ctVec4ToIm3dColor(ctVec4(ctVec3(weight), 1.0f)));
                  Im3d::Vertex(ctVec3ToIm3d(position));
               }
               Im3d::End();
               Im3d::PopColor();
            } break;
            case CT_MODELVIEW_MATERIAL: {
               ctRandomGenerator rng = ctRandomGenerator(submesh.materialIndex);
               ctVec4 color = submesh.materialIndex != UINT32_MAX
                                ? rng.GetColor()
                                : CT_COLOR_RED * (ctSin(timer * 4.0f) * 0.5f + 0.5f);
               color.a = 1.0f;
               Im3d::PushColor(ctVec4ToIm3dColor(color));
               Im3d::BeginTriangles();
               for (uint32_t i = 0; i < submesh.indexCount; i++) {
                  uint32_t idx =
                    (uint32_t)indices[i + submesh.indexOffset] + submesh.vertexOffset;
                  ctModelMeshVertexCoords coord = coords[idx];

                  TinyImageFormat_DecodeInput posDecode = TinyImageFormat_DecodeInput();
                  posDecode.pixel = coord.position;
                  ctVec3 position;
                  TinyImageFormat_DecodeLogicalPixelsF(
                    TinyImageFormat_R16G16B16_UNORM, &posDecode, 1, position.data);
                  position = bbox.DeNormalizeVector(position);
                  Im3d::Vertex(ctVec3ToIm3d(position));
               }
               Im3d::End();
               Im3d::PopColor();
            } break;
            default: break;
         }
      }
   }
}
