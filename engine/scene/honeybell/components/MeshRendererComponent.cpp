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

#include "MeshRendererComponent.hpp"
#include "scene/honeybell/Scene.hpp"
#include "im3d/im3d.h"

#include "formats/wad/WADCore.h"
#include "formats/wad/prototypes/RenderMesh.h"

ctHoneybell::MeshRendererComponent::MeshRendererComponent(struct ConstructContext ctx,
                                                          class ToyBase* _toy,
                                                          int32_t meshInstanceIdx) :
    ComponentBase::ComponentBase(ctx, _toy) {
   ctWADProtoRenderMesh* renderMeshProto = NULL;
   int32_t boneIdx = 0;
   /* Find data in WAD */
   {
      ctWADProtoRenderMeshInstance* pInstances = NULL;
      int32_t instanceLumpSize;
      ctWADFindLump(
        &ctx.prefab.wadReader, "RINST", (void**)&pInstances, &instanceLumpSize);
      const int32_t instanceCount = instanceLumpSize / sizeof(pInstances[0]);
      ctWADProtoRenderMesh* pRenderMeshes = NULL;
      ctWADFindLump(&ctx.prefab.wadReader, "RMESH", (void**)&pRenderMeshes, NULL);
   }
}

ctHoneybell::MeshRendererComponent::~MeshRendererComponent() {
}

ctResults ctHoneybell::MeshRendererComponent::Begin(BeginContext& ctx) {
   return CT_SUCCESS;
}

bool ctHoneybell::MeshRendererComponent::hasTransform() const {
   return true;
}

void ctHoneybell::MeshRendererComponent::SetWorldTransform(ctTransform& v) {
   transform = v;
}

ctTransform ctHoneybell::MeshRendererComponent::GetWorldTransform() const {
   return transform;
}

void ctHoneybell::MeshRendererComponent::SetLocalBounds(ctBoundBox& v) {
   bounds = v;
}

ctBoundBox ctHoneybell::MeshRendererComponent::GetLocalBounds() const {
   return bounds;
}

const char* ctHoneybell::MeshRendererComponent::GetTypeName() {
   return "MeshRendererComponent";
}
