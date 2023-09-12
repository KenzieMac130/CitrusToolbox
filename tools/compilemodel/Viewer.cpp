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

#include "animation/Skeleton.hpp"
#include "animation/MorphSet.hpp"
#include "animation/Canvas.hpp"
#include "animation/Bank.hpp"
#include "animation/LayerClip.hpp"
#include "animation/Spline.hpp"

#include "scene/SceneEngine.hpp"
#include "core/JobSystem.hpp"

enum ctModelViewerMeshMode {
   CT_MODELVIEW_SUBMESH,
   CT_MODELVIEW_WIREFRAME,
   CT_MODELVIEW_NORMAL,
   CT_MODELVIEW_TANGENT,
   CT_MODELVIEW_TANGENT_SIGN,
   CT_MODELVIEW_BITANGENT,
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
                             "Bitangent",
                             "UV",
                             "Color",
                             "Bone",
                             "Material"};

struct ctModelViewerAnimState {
   float time = 0.0f;
   float speed = 1.0f;
   float weight = 0.0f;
};

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
      ctPhysicsEngineUpdate(Engine->Physics->GetPhysicsEngine(), deltaTime);

      timer += deltaTime;

      /* animation */
      if (applyKeyframes && pAnimCanvas && pAnimBank) {
         pAnimCanvas->ResetToDefault();
         for (uint32_t i = 0; i < pAnimBank->GetClipCount(); i++) {
            animStates[i].time += deltaTime * animStates[i].speed;
            if (animStates[i].time > pAnimBank->GetClip(i).GetLength()) {
               animStates[i].time =
                 animStates[i].time - pAnimBank->GetClip(i).GetLength();
            } else if (animStates[i].time < 0.0f) {
               animStates[i].time =
                 pAnimBank->GetClip(i).GetLength() - ctAbs(animStates[i].time);
            }
            if (animStates[i].weight > 0.0f) {
               pAnimCanvas->ApplyLayer(
                 ctAnimLayerClip(pAnimBank->GetClip(i), animStates[i].time),
                 animStates[i].weight);
            }
         }
         if (pSkeleton) {
            pAnimCanvas->CopyToSkeleton(pSkeleton);
            pSkeleton->FlushBoneTransforms();
         }
         if (pMorphSet) { pAnimCanvas->CopyToMorphSet(pMorphSet); }
      }
      return CT_SUCCESS;
   }
   virtual ctResults OnShutdown();

   /* GUI */
   void SkeletonInfo();
   void BoneInfo(ctAnimBone bone);
   void GeometryInfo();
   void AnimationInfo();
   void SplinesInfo();
   void PhysicsInfo();
   void NavmeshInfo();
   void MaterialInfo();
   void SceneInfo();

   /* Debug Rendering */
   void RenderModel();
   void RenderSkeleton();
   void RenderBoundingBoxes();
   void RenderGeometry();
   void RenderSplines();

   /* Settings */
   bool renderSkeleton = false;
   bool renderBoneNames = false;
   bool renderGeometry = true;
   bool renderBBOX = true;
   bool renderSplines = true;
   bool applySkin = true;
   bool applyKeyframes = false;
   float splineTailLength = 0.1f;
   ctModelViewerMeshMode viewMode = CT_MODELVIEW_WIREFRAME;
   int32_t uvChannel = 0;
   int32_t colorChannel = 0;
   int32_t previewBone = 0;
   int32_t lodView = 0;

   void FillAnimMeshData(ctModelMeshVertexCoords* coords,
                         ctModelMeshVertexUV* uvs,
                         ctModelMeshVertexColor* colors,
                         uint32_t vertexCount,
                         ctBoundBox bbox,
                         ctBoundBox2D uvbox);
   void ApplyMorph(ctModelMeshLod& lod, int32_t idx, float weight);
   void ApplySkeleton(ctModelMeshVertexSkinData* pSkins, size_t vertexCount);
   ctDynamicArray<ctVec3> animPositions;
   ctDynamicArray<ctVec3> animNormals;
   ctDynamicArray<ctVec4> animTangents;
   ctDynamicArray<ctVec2> animUVs;
   ctDynamicArray<ctVec4> animColors;

   ctAnimBank* pAnimBank;
   ctAnimSkeleton* pSkeleton;
   ctAnimMorphSet* pMorphSet;
   ctAnimCanvas* pAnimCanvas;
   ctDynamicArray<ctModelViewerAnimState> animStates;

   ctDynamicArray<ctAnimSpline*> splines;

   ctDynamicArray<ctPhysicsBody> bodies;
   ctDynamicArray<ctPhysicsBody> physicsTestBodies;
   int32_t testShapeGrid[2] = {16, 16};
   float testShapeSize = 0.3f;
   float testShapeSpacing = 0.25f;
   float testShapeHeight = 25.0f;
   void CreateTestShapes();
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
   Engine->SceneEngine->EnableDebugCamera();

   /* setup animation */
   if (model.geometry.morphTargetMapping) { pMorphSet = new ctAnimMorphSet(model); }
   if (model.skeleton.boneCount) { pSkeleton = new ctAnimSkeleton(model); }
   if (model.animation.clipCount) {
      pAnimBank = new ctAnimBank(model);
      animStates.Resize(pAnimBank->GetClipCount());
   }
   pAnimCanvas = new ctAnimCanvas(pSkeleton, pMorphSet);
   for (uint32_t i = 0; i < model.splines.segmentCount; i++) {
      splines.Append(new ctAnimSpline(model, i));
   }

   /* setup collisions */
   for (uint32_t i = 0; i < model.physics.shapeCount; i++) {
      ctPhysicsBody body;
      ctModelCollisionShape shape = model.physics.shapes[i];
      uint8_t* data = model.physics.bake.data + shape.bakeOffset;
      ctPhysicsBodyDesc desc = ctPhysicsBodyDesc();
      desc.motion = CT_PHYSICS_STATIC;
      desc.shape = ctPhysicsShapeBaked(data, shape.bakeSize);
      ctPhysicsCreateBody(Engine->Physics->GetPhysicsEngine(), body, desc);
      bodies.Append(body);
   }
   return CT_SUCCESS;
}

void ctModelViewer::CreateTestShapes() {
   ctGetJobSystem()->WaitBarrier();
   for (size_t i = 0; i < physicsTestBodies.Count(); i++) {
      ctPhysicsDestroy(Engine->Physics->GetPhysicsEngine(), physicsTestBodies[i]);
   }
   physicsTestBodies.Clear();

   ctVec3 startPos = ctVec3(-testShapeSize * (testShapeGrid[0] / 2),
                            testShapeHeight,
                            -testShapeSize * (testShapeGrid[1] / 2));
   ctVec3 vecProgX = ctVec3(testShapeSize, 0.0f, 0.0f);
   ctVec3 vecProgY = ctVec3(0.0f, 0.0f, testShapeSize);
   float finalShapeSize = testShapeSize * testShapeSpacing;
   ctRandomGenerator rng = ctRandomGenerator();

   for (int32_t x = 0; x < testShapeGrid[0]; x++) {
      for (int32_t y = 0; y < testShapeGrid[1]; y++) {
         ctVec3 randomJitter = rng.GetVec3() * testShapeSize * 0.5f;
         ctPhysicsBody body;
         ctPhysicsBodyDesc desc = ctPhysicsBodyDesc();
         desc.position =
           startPos + (vecProgX * (float)x) + (vecProgY * (float)y) + randomJitter;
         desc.rotation = normalize(ctQuat(rng.GetVec4()));
         desc.shape = ctPhysicsShapeSphere(finalShapeSize);
         desc.isHighSpeed = true;
         desc.startActive = true;
         ctPhysicsCreateBody(Engine->Physics->GetPhysicsEngine(), body, desc);
         physicsTestBodies.Append(body);
      }
   }
}

ctResults ctModelViewer::OnUIUpdate() {
   ZoneScoped;
   if (ImGui::Begin(CT_NC("Model"))) {
      if (ImGui::CollapsingHeader(CT_NC("Skeleton"))) { SkeletonInfo(); }
      if (ImGui::CollapsingHeader(CT_NC("Geometry"))) { GeometryInfo(); }
      if (ImGui::CollapsingHeader(CT_NC("Animation"))) { AnimationInfo(); }
      if (ImGui::CollapsingHeader(CT_NC("Splines"))) { SplinesInfo(); }
      if (ImGui::CollapsingHeader(CT_NC("Material"))) { MaterialInfo(); }
      if (ImGui::CollapsingHeader(CT_NC("Physics"))) { PhysicsInfo(); }
      if (ImGui::CollapsingHeader(CT_NC("Navmesh"))) { NavmeshInfo(); }
      if (ImGui::CollapsingHeader(CT_NC("Scene"))) { SceneInfo(); }
   }
   ImGui::End();
   RenderModel();
   return CT_SUCCESS;
}

ctResults ctModelViewer::OnShutdown() {
   delete pSkeleton;
   delete pMorphSet;
   delete pAnimCanvas;
   return CT_SUCCESS;
}

void ctModelViewer::SkeletonInfo() {
   if (!pSkeleton) { return; }
   ImGui::PushID("BONE_TREE");
   ImGui::Checkbox(CT_NC("Render Skeleton"), &renderSkeleton);
   ImGui::Checkbox(CT_NC("Render Bone Names"), &renderBoneNames);
   ImGui::Text("Bone Count: %u", pSkeleton->GetBoneCount());
   for (int32_t i = 0; i < pSkeleton->GetBoneCount(); i++) {
      if (!pSkeleton->GetBoneParent(i).isValid()) { BoneInfo(i); }
   }
   ImGui::PopID();
}

void ctModelViewer::BoneInfo(ctAnimBone bone) {
   char namebuff[33];
   memset(namebuff, 0, 33);
   pSkeleton->GetBoneName(bone, namebuff);
   if (ImGui::TreeNode(namebuff)) {
      if (ImGui::Button("Show Weights")) {
         previewBone = bone.index;
         viewMode = CT_MODELVIEW_BONE;
      }
      ImGui::Text("Graph: %d %d %d",
                  pSkeleton->GetBoneParent(bone).index,
                  pSkeleton->GetBoneFirstChild(bone).index,
                  pSkeleton->GetBoneNextSibling(bone).index);

      bool needsUpdate = false;
      ctTransform transform = pSkeleton->GetLocalTransform(bone);
      if (ImGui::InputFloat3("Local Translation", transform.translation.data)) {
         needsUpdate = true;
      }
      if (ImGui::InputFloat4("Local Rotation", transform.rotation.data)) {
         needsUpdate = true;
      }
      if (ImGui::InputFloat3("Local Scale", transform.scale.data)) { needsUpdate = true; }
      if (needsUpdate) {
         pSkeleton->SetLocalTransform(bone, transform);
         pSkeleton->FlushBoneTransforms();
      }

      transform = pSkeleton->GetModelTransform(bone);
      ImGui::Text("Model Translation: %f %f %f",
                  transform.translation.x,
                  transform.translation.y,
                  transform.translation.z);
      ImGui::Text("Model Rotation: %f %f %f %f",
                  transform.rotation.x,
                  transform.rotation.y,
                  transform.rotation.z,
                  transform.rotation.w);
      ImGui::Text(
        "Model Scale: %f %f %f", transform.scale.x, transform.scale.y, transform.scale.z);

      if (pSkeleton->GetBoneFirstChild(bone).isValid()) {
         BoneInfo(pSkeleton->GetBoneFirstChild(bone));
      }
      ImGui::TreePop();
   }
   if (pSkeleton->GetBoneNextSibling(bone).isValid()) {
      BoneInfo(pSkeleton->GetBoneNextSibling(bone));
   }
}

void ctModelViewer::GeometryInfo() {
   ImGui::PushID("GEO_TREE");
   ImGui::Checkbox(CT_NC("Render Geometry"), &renderGeometry);
   ImGui::Checkbox(CT_NC("Render Bounding Boxes"), &renderBBOX);
   ImGui::Checkbox(CT_NC("Apply Skinning"), &applySkin);
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
        CT_NC("Active Bone"), &previewBone, 0, model.skeleton.boneCount - 1);
   }

   if (ImGui::TreeNode("Meshes")) {
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

                  ImGui::Text("Morph Count: %d", lod.morphTargetCount);
                  for (uint32_t morphIdx = 0; morphIdx < lod.morphTargetCount;
                       morphIdx++) {
                     ctModelMeshMorphTarget& morph =
                       model.geometry.morphTargets[lod.morphTargetStart + morphIdx];
                     memset(namebuff, 0, 33);
                     strncpy(namebuff,
                             model.geometry
                               .morphTargetMapping[mesh.morphMapStart + morph.mapping]
                               .name,
                             32);
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
                        ImGui::Text("Vertex Offset: %u", morph.vertexDataMorphOffset);
                        ImGui::TreePop();
                     }
                  }
                  ImGui::TreePop();
               }
            }
            ImGui::TreePop();
         }
      }
      ImGui::TreePop();
   }

   if (pMorphSet) {
      if (ImGui::TreeNode("Morph Target Layers")) {
         for (int32_t i = 0; i < pMorphSet->GetTargetCount(); i++) {
            char buff[33];
            memset(buff, 0, 33);
            pMorphSet->GetTargetName(i, buff);
            float value = pMorphSet->GetTarget(i);
            if (ImGui::SliderFloat(buff, &value, 0.0f, 1.0f)) {
               pMorphSet->SetTarget(i, value);
            }
         }
         ImGui::TreePop();
      }
   }

   ImGui::PopID();
}

void ctModelViewer::AnimationInfo() {
   if (pAnimBank) {
      ImGui::Checkbox("Apply Keyframes", &applyKeyframes);
      if (ImGui::TreeNode("Animation Clips")) {
         for (uint32_t i = 0; i < pAnimBank->GetClipCount(); i++) {
            char buff[33];
            memset(buff, 0, 33);
            pAnimBank->GetClip(i).GetName(buff);
            if (ImGui::TreeNode(buff)) {
               ImGui::InputFloat("Time", &animStates[i].time);
               ImGui::InputFloat("Speed", &animStates[i].speed);
               ImGui::InputFloat("Weight", &animStates[i].weight);
               ImGui::TreePop();
            }
         }
         ImGui::TreePop();
      }
   }
}

void ctModelViewer::SplinesInfo() {
   if (splines.isEmpty()) { return; }
   ImGui::Checkbox("Render Splines", &renderSplines);
   ImGui::SliderFloat("Axis Size", &splineTailLength, 0.001f, 1.0f);
   if (ImGui::TreeNode("Segments")) {
      for (uint32_t i = 0; i < (uint32_t)splines.Count(); i++) {
         ImGui::Text("Segment %u, Point Count %u, Length %f, Type %s %s",
                     i,
                     splines[i]->GetPointCount(),
                     splines[i]->GetLength(),
                     splines[i]->isCubic() ? "CUBIC" : "LINEAR",
                     splines[i]->isCyclic() ? "CYCLIC" : "ACYCLIC");
      }
      ImGui::TreePop();
   }
}

void ctModelViewer::PhysicsInfo() {
   ImGui::SliderInt2("Grid Size", testShapeGrid, 1, 64);
   ImGui::SliderFloat("Size", &testShapeSize, 0.01f, 50.0f);
   ImGui::SliderFloat("Spacing", &testShapeSpacing, 0.01f, 1.0f);
   ImGui::InputFloat("Height", &testShapeHeight);
   if (ImGui::Button("Drop Shapes")) { CreateTestShapes(); }
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
}

void ctModelViewer::RenderModel() {
   ZoneScoped;
   if (renderSkeleton) { RenderSkeleton(); }
   if (renderBBOX) { RenderBoundingBoxes(); }
   if (renderGeometry) { RenderGeometry(); }
   if (renderSplines) { RenderSplines(); }
}

void ctModelViewer::RenderSkeleton() {
   ZoneScoped;
   for (int32_t i = 0; i < pSkeleton->GetBoneCount(); i++) {
      Im3d::PushMatrix(ctMat4ToIm3d((pSkeleton->GetModelMatrix(i))));
      Im3d::DrawXyzAxes();
      if (renderBoneNames) {
         char buff[33];
         memset(buff, 0, 33);
         pSkeleton->GetBoneName(i, buff);
         Im3d::PushColor(Im3d::Color_White);
         Im3d::Text(Im3d::Vec3(0.0f), Im3d::TextFlags_Default, buff);
         Im3d::PopColor();
      }
      Im3d::PopMatrix();
   }
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

struct ctModelViewFixedRendererModeDesc {
   uint16_t* indices;
   ctVec3* positions;
   ctVec3* normals;
   ctVec4* tangents;
   ctVec2* uvs;
   ctVec4* colors;
   ctModelMeshVertexSkinData* skins;
   ctBoundBox bbox;
   ctBoundBox2D uvbox;
   ctModelSubmesh submesh;
   float timer;
};

class ctModelViewFixedRendererModeBase {
public:
   ctModelViewFixedRendererModeBase(ctModelViewFixedRendererModeDesc& desc) {
      indices = desc.indices;
      positions = desc.positions;
      normals = desc.normals;
      tangents = desc.tangents;
      uvs = desc.uvs;
      colors = desc.colors;
      skins = desc.skins;
      bbox = desc.bbox;
      uvbox = desc.uvbox;
      submesh = desc.submesh;
      timer = desc.timer;
   }

   virtual ctVec4 GetOutputColor(uint16_t idx) {
      return CT_COLOR_WHITE;
   };

   virtual ctVec3 GetVertexPosition(uint16_t idx) {
      return positions[idx];
   };

   virtual void Setup() {
   }

   virtual void Render() {
      Setup();
      Im3d::PushColor();
      Im3d::BeginTriangles();
      for (uint32_t i = 0; i < submesh.indexCount; i++) {
         uint32_t idx = (uint32_t)indices[i + submesh.indexOffset] + submesh.vertexOffset;
         ctVec3 position = GetVertexPosition(idx);
         ctVec4 color = GetOutputColor(idx);
         Im3d::SetColor(ctVec4ToIm3dColor(color));
         Im3d::Vertex(ctVec3ToIm3d(position));
      }
      Im3d::End();
      Im3d::PopColor();
   }

   uint16_t* indices;
   ctVec3* positions;
   ctVec3* normals;
   ctVec4* tangents;
   ctVec2* uvs;
   ctVec4* colors;
   ctModelMeshVertexSkinData* skins;
   ctBoundBox bbox;
   ctBoundBox2D uvbox;
   ctModelSubmesh submesh;
   float timer;
};

class ctModelViewFixedRendererModeWireframe : public ctModelViewFixedRendererModeBase {
public:
   ctModelViewFixedRendererModeWireframe(ctModelViewFixedRendererModeDesc& desc) :
       ctModelViewFixedRendererModeBase(desc) {};

   virtual void Render() {
      Setup();
      Im3d::PushColor();
      Im3d::BeginLines();
      for (uint32_t i = 0; i < submesh.indexCount / 3; i++) {
         for (uint32_t j = 0; j < 3; j++) {
            uint32_t idx0 =
              (uint32_t)indices[submesh.indexOffset + i * 3 + j] + submesh.vertexOffset;
            uint32_t idx1 =
              (uint32_t)indices[submesh.indexOffset + i * 3 + ((j + 1) % 3)] +
              submesh.vertexOffset;
            ctVec3 position0 = GetVertexPosition(idx0);
            ctVec3 position1 = GetVertexPosition(idx1);
            ctVec4 color0 = GetOutputColor(idx0);
            ctVec4 color1 = GetOutputColor(idx1);
            Im3d::SetColor(ctVec4ToIm3dColor(color0));
            Im3d::Vertex(ctVec3ToIm3d(position0));
            Im3d::SetColor(ctVec4ToIm3dColor(color1));
            Im3d::Vertex(ctVec3ToIm3d(position1));
         }
      }
      Im3d::End();
      Im3d::PopColor();
   }
};

class ctModelViewFixedRendererModeSubmesh : public ctModelViewFixedRendererModeBase {
public:
   ctModelViewFixedRendererModeSubmesh(ctModelViewFixedRendererModeDesc& desc) :
       ctModelViewFixedRendererModeBase(desc) {};

   virtual ctVec4 GetOutputColor(uint16_t idx) {
      return color;
   }
   virtual void Setup() {
      ctRandomGenerator rng =
        ctRandomGenerator((submesh.indexOffset + submesh.indexCount) * 384298);
      color = rng.GetColor();
   }
   ctVec4 color;
};

class ctModelViewFixedRendererModeMaterial : public ctModelViewFixedRendererModeBase {
public:
   ctModelViewFixedRendererModeMaterial(ctModelViewFixedRendererModeDesc& desc) :
       ctModelViewFixedRendererModeBase(desc) {};

   virtual ctVec4 GetOutputColor(uint16_t idx) {
      return color;
   }
   virtual void Setup() {
      ctRandomGenerator rng = ctRandomGenerator(submesh.materialIndex * 87657);
      color = submesh.materialIndex != UINT32_MAX
                ? rng.GetColor()
                : CT_COLOR_RED * (ctSin(timer * 4.0f) * 0.5f + 0.5f);
      color.a = 1.0f;
   }
   ctVec4 color;
};

class ctModelViewFixedRendererModeNormal : public ctModelViewFixedRendererModeBase {
public:
   ctModelViewFixedRendererModeNormal(ctModelViewFixedRendererModeDesc& desc) :
       ctModelViewFixedRendererModeBase(desc) {};

   virtual ctVec4 GetOutputColor(uint16_t idx) {
      ctVec3 normal = normals[idx];
      return saturate(ctVec4(normal, 1.0f));
   }
};

class ctModelViewFixedRendererModeTangent : public ctModelViewFixedRendererModeBase {
public:
   ctModelViewFixedRendererModeTangent(ctModelViewFixedRendererModeDesc& desc,
                                       bool sign) :
       ctModelViewFixedRendererModeBase(desc) {
      signOnly = sign;
   };

   virtual ctVec4 GetOutputColor(uint16_t idx) {
      ctVec4 tangent = tangents[idx];
      if (signOnly) { tangent = ctVec4(ctVec3(tangent.w > 0.0f), 1.0f); }
      tangent.w = 1.0f;
      return saturate(tangent);
   }

   bool signOnly;
};

class ctModelViewFixedRendererModeBitangent : public ctModelViewFixedRendererModeBase {
public:
   ctModelViewFixedRendererModeBitangent(ctModelViewFixedRendererModeDesc& desc) :
       ctModelViewFixedRendererModeBase(desc) {};

   virtual ctVec4 GetOutputColor(uint16_t idx) {
      const ctVec4 tangent = tangents[idx];
      const ctVec3 normal = normals[idx];
      const ctVec3 bitangent = cross(normal, ctVec3(tangent)) * tangent.w;
      return ctVec4(saturate(bitangent), 1.0f);
   }
};

class ctModelViewFixedRendererModeUV : public ctModelViewFixedRendererModeBase {
public:
   ctModelViewFixedRendererModeUV(ctModelViewFixedRendererModeDesc& desc) :
       ctModelViewFixedRendererModeBase(desc) {};

   virtual ctVec4 GetOutputColor(uint16_t idx) {
      return ctVec4(uvs[idx], 0.0f, 1.0f);
   }
};

class ctModelViewFixedRendererModeColor : public ctModelViewFixedRendererModeBase {
public:
   ctModelViewFixedRendererModeColor(ctModelViewFixedRendererModeDesc& desc) :
       ctModelViewFixedRendererModeBase(desc) {};

   virtual ctVec4 GetOutputColor(uint16_t idx) {
      return colors[idx];
   }
};

class ctModelViewFixedRendererModeBone : public ctModelViewFixedRendererModeBase {
public:
   ctModelViewFixedRendererModeBone(ctModelViewFixedRendererModeDesc& desc,
                                    uint32_t activeBoneIndex) :
       ctModelViewFixedRendererModeBase(desc) {
      activeBone = activeBoneIndex;
   };

   virtual ctVec4 GetOutputColor(uint16_t idx) {
      ctModelMeshVertexSkinData skin = skins ? skins[idx] : ctModelMeshVertexSkinData();

      ctVec3 position;
      float weight = 0.0f;
      for (int b = 0; b < 4; b++) {
         if (skin.indices[b] == activeBone) {
            weight += (float)skin.weights[b] / UINT16_MAX;
         }
      }
      return ctVec4(ctVec3(weight), 1.0f);
   }

   uint32_t activeBone;
};

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
        (uint16_t*)&model.inMemoryGeometryData[model.gpuTable.indexData.start];
      ctModelMeshVertexCoords* coords =
        (ctModelMeshVertexCoords*)&model
          .inMemoryGeometryData[model.gpuTable.vertexDataCoords.start];
      ctModelMeshVertexUV* uvs =
        (ctModelMeshVertexUV*)&model
          .inMemoryGeometryData[model.gpuTable.vertexDataUV.start];
      ctModelMeshVertexColor* colors =
        (ctModelMeshVertexColor*)&model
          .inMemoryGeometryData[model.gpuTable.vertexDataColor.start];
      ctModelMeshVertexSkinData* skins =
        (ctModelMeshVertexSkinData*)&model
          .inMemoryGeometryData[model.gpuTable.vertexDataSkin.start];

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

      /* fill anim data */
      FillAnimMeshData(coords, uvs, colors, lod.vertexCount, bbox, uvbox);
      /* apply morph targets*/
      if (pMorphSet) {
         for (int32_t i = 0; i < pMorphSet->GetTargetCount(); i++) {
            ApplyMorph(lod, i, pMorphSet->GetTarget(i));
         }
      }
      if (applySkin && pSkeleton && skins) { ApplySkeleton(skins, lod.vertexCount); }

      /* for each submesh */
      for (uint32_t submeshIdx = 0; submeshIdx < lod.submeshCount; submeshIdx++) {
         ctModelSubmesh submesh = model.geometry.submeshes[lod.submeshStart + submeshIdx];

         ctModelViewFixedRendererModeDesc desc = {};
         desc.bbox = bbox;
         desc.positions = animPositions.Data();
         desc.normals = animNormals.Data();
         desc.tangents = animTangents.Data();
         desc.uvs = animUVs.Data();
         desc.colors = animColors.Data();
         desc.skins = skins;
         desc.indices = indices;
         desc.submesh = submesh;
         desc.uvbox = uvbox;
         desc.timer = timer;

         /* for each index */
         switch (viewMode) {
            case CT_MODELVIEW_SUBMESH: {
               ctModelViewFixedRendererModeSubmesh(desc).Render();
            } break;
            case CT_MODELVIEW_WIREFRAME: {
               ctModelViewFixedRendererModeWireframe(desc).Render();
            } break;
            case CT_MODELVIEW_NORMAL: {
               ctModelViewFixedRendererModeNormal(desc).Render();
            } break;
            case CT_MODELVIEW_TANGENT:
            case CT_MODELVIEW_TANGENT_SIGN: {
               ctModelViewFixedRendererModeTangent(desc,
                                                   viewMode == CT_MODELVIEW_TANGENT_SIGN)
                 .Render();
            } break;
            case CT_MODELVIEW_BITANGENT:
               ctModelViewFixedRendererModeBitangent(desc).Render();
               break;
            case CT_MODELVIEW_UV: {
               ctModelViewFixedRendererModeUV(desc).Render();
            } break;
            case CT_MODELVIEW_COLOR: {
               ctModelViewFixedRendererModeColor(desc).Render();
            } break;
            case CT_MODELVIEW_BONE: {
               ctModelViewFixedRendererModeBone(desc, previewBone).Render();
            } break;
            case CT_MODELVIEW_MATERIAL: {
               ctModelViewFixedRendererModeMaterial(desc).Render();
            } break;
            default: break;
         }
      }
   }
}

void ctModelViewer::RenderSplines() {
   for (size_t seg = 0; seg < splines.Count(); seg++) {
      ctAnimSpline* spline = splines[seg];

      /* line */
      Im3d::BeginLineStrip();
      for (int32_t i = 0; i < spline->GetPointCount(); i++) {
         ctVec3 p, n, t;
         spline->EvaluateAtPoint(i, p, n, t);
         Im3d::Vertex(ctVec3ToIm3d(p));
      }
      if (spline->isCyclic()) {
         ctVec3 p, n, t;
         spline->EvaluateAtPoint(0, p, n, t);
         Im3d::Vertex(ctVec3ToIm3d(p));
      }
      Im3d::End();

      /* normals */
      Im3d::PushColor(Im3d::Color_Red);
      for (int32_t i = 0; i < spline->GetPointCount(); i++) {
         ctVec3 p, n, t;
         spline->EvaluateAtPoint(i, p, n, t);
         n *= splineTailLength;
         Im3d::DrawArrow(ctVec3ToIm3d(p), ctVec3ToIm3d((p + n)));
      }
      Im3d::PopColor();

      /* tangents */
      Im3d::PushColor(Im3d::Color_Green);
      for (int32_t i = 0; i < spline->GetPointCount(); i++) {
         ctVec3 p, n, t;
         spline->EvaluateAtPoint(i, p, n, t);
         t *= splineTailLength;
         Im3d::DrawArrow(ctVec3ToIm3d(p), ctVec3ToIm3d((p + t)));
      }
      Im3d::PopColor();

      /* bitangents */
      Im3d::PushColor(Im3d::Color_Blue);
      for (int32_t i = 0; i < spline->GetPointCount(); i++) {
         ctVec3 p, n, t;
         spline->EvaluateAtPoint(i, p, n, t);
         ctVec3 b = cross(t, n) * splineTailLength;
         Im3d::DrawArrow(ctVec3ToIm3d(p), ctVec3ToIm3d((p + b)));
      }
      Im3d::PopColor();
   }
}

void ctModelViewer::FillAnimMeshData(ctModelMeshVertexCoords* coords,
                                     ctModelMeshVertexUV* uvs,
                                     ctModelMeshVertexColor* colors,
                                     uint32_t vertexCount,
                                     ctBoundBox bbox,
                                     ctBoundBox2D uvbox) {
   animPositions.Clear();
   animNormals.Clear();
   animTangents.Clear();
   animColors.Clear();
   animUVs.Clear();
   for (uint32_t i = 0; i < vertexCount; i++) {
      float buffer[4];
      TinyImageFormat_DecodeInput decode = TinyImageFormat_DecodeInput();
      ctModelMeshVertexCoords coord = coords[i];

      decode.pixel = coord.position;
      TinyImageFormat_DecodeLogicalPixelsF(
        TinyImageFormat_R16G16B16_UNORM, &decode, 1, buffer);
      ctVec3 position = ctVec3(buffer[0], buffer[1], buffer[2]);
      position = bbox.DeNormalizeVector(position);

      decode.pixel = &coord.normal;
      TinyImageFormat_DecodeLogicalPixelsF(
        TinyImageFormat_R10G10B10A2_UNORM, &decode, 1, buffer);
      ctVec3 normal = ctVec3(buffer[0], buffer[1], buffer[2]);
      normal *= 2.0f;
      normal -= ctVec3(1.0f);

      decode.pixel = &coord.tangent;
      TinyImageFormat_DecodeLogicalPixelsF(
        TinyImageFormat_R10G10B10A2_UNORM, &decode, 1, buffer);
      ctVec4 tangent = ctVec4(buffer[0], buffer[1], buffer[2], buffer[3]);
      tangent *= ctVec4(2.0f, 2.0f, 2.0f, 2.0f);
      tangent -= ctVec4(1.0f, 1.0f, 1.0f, 1.0f);

      ctModelMeshVertexUV uvc = uvs ? uvs[i] : ctModelMeshVertexUV();
      decode.pixel = uvc.uv;
      TinyImageFormat_DecodeLogicalPixelsF(
        TinyImageFormat_R16G16_UNORM, &decode, 1, buffer);
      ctVec2 uv = ctVec2(buffer[0], buffer[1]);
      uv = uvbox.DeNormalizeVector(uv);

      ctModelMeshVertexColor colorc = colors ? colors[i] : ctModelMeshVertexColor();
      ctVec4 color;
      decode.pixel = colorc.rgba;
      TinyImageFormat_DecodeLogicalPixelsF(
        TinyImageFormat_R8G8B8A8_UNORM, &decode, 1, color.data);

      animPositions.Append(position);
      animNormals.Append(normal);
      animTangents.Append(tangent);
      animUVs.Append(uv);
      animColors.Append(color);
   }
}

void ctModelViewer::ApplyMorph(ctModelMeshLod& lod, int32_t idx, float weight) {
   if (weight == 0.0f) { return; }
   ctModelMeshVertexMorph* mverts =
     (ctModelMeshVertexMorph*)&model
       .inMemoryGeometryData[model.gpuTable.vertexDataMorph.start];
   for (uint32_t midx = 0; midx < lod.morphTargetCount; midx++) {
      const auto& morph = model.geometry.morphTargets[lod.morphTargetStart + midx];
      if (morph.mapping == idx) {
         for (uint32_t i = 0; i < morph.vertexCount; i++) {
            ctModelMeshVertexMorph mvert = mverts[morph.vertexDataMorphOffset + i];

            TinyImageFormat_DecodeInput decode = TinyImageFormat_DecodeInput();
            float buffer[4];
            decode.pixel = mvert.position;
            TinyImageFormat_DecodeLogicalPixelsF(
              TinyImageFormat_R16G16B16_UNORM, &decode, 1, buffer);
            ctVec3 position = ctVec3(buffer[0], buffer[1], buffer[2]);
            position = morph.bboxDisplacement.DeNormalizeVector(position);
            animPositions[i] += (position * weight);

            decode.pixel = &mvert.normal;
            TinyImageFormat_DecodeLogicalPixelsF(
              TinyImageFormat_R10G10B10A2_UNORM, &decode, 1, buffer);
            ctVec3 normal = ctVec3(buffer[0], buffer[1], buffer[2]);
            normal -= ctVec3(0.5f);
            normal *= 2.0f;
            animNormals[i] = normalize(animNormals[i] + (normal * weight));

            decode.pixel = &mvert.tangent;
            TinyImageFormat_DecodeLogicalPixelsF(
              TinyImageFormat_R10G10B10A2_UNORM, &decode, 1, buffer);
            ctVec3 tangent = ctVec3(buffer[0], buffer[1], buffer[2]);
            tangent *= ctVec3(2.0f, 2.0f, 2.0f);
            tangent -= ctVec3(1.0f, 1.0f, 1.0f);
            animTangents[i] = ctVec4(
              normalize(ctVec3(animTangents[i]) + (tangent * weight)), animTangents[i].w);
         }
         break;
      }
   }
}

void ctModelViewer::ApplySkeleton(ctModelMeshVertexSkinData* pSkins, size_t vertexCount) {
   /* loop through vertices */
   for (size_t i = 0; i < vertexCount; i++) {
      const ctVec3 originalposition = animPositions[i];
      const ctVec3 originalnormal = animNormals[i];
      const ctVec3 originaltangent = animTangents[i];
      /* for each skinnable bone on vertex */
      for (int b = 0; b < 4; b++) {
         /* initialize incoming data */
         ctAnimBone bone = pSkins[i].indices[b];
         float weight = (float)pSkins[i].weights[b] / UINT16_MAX;
         ctVec3 position = originalposition;
         ctVec3 normal = originalnormal;
         ctVec3 tangent = originaltangent;

         /* bring to root using inverse bind */
         ctMat4 inverseBind = pSkeleton->GetInverseBindMatrix(bone);
         position = position * inverseBind;
         ctMat4RemoveTranslation(inverseBind);
         normal = normal * inverseBind;
         tangent = tangent * inverseBind;

         /* bring to pose using model transform */
         ctMat4 modelMatrix = pSkeleton->GetModelMatrix(bone);
         position = position * modelMatrix;
         ctMat4RemoveTranslation(modelMatrix);
         normal = normal * modelMatrix;
         tangent = tangent * modelMatrix;

         /* turn to displacement */
         position = (position - originalposition) * weight;
         normal = (normal - originalnormal) * weight;
         tangent = (tangent - originaltangent) * weight;

         /* apply offsets */
         animPositions[i] += position;
         animNormals[i] = normalize(normal + animNormals[i]);
         animTangents[i] += ctVec4(normalize(ctVec3(animTangents[i]) + tangent), 0.0f);
      }
   }
}
