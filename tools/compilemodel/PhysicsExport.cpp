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

ctResults ctGltf2Model::ExtractPhysics(ctGltf2ModelPhysicsMode mode) {
   /* setup physx */
   pPhysXIntegration = new ctPhysXIntegration();
   pPhysXIntegration->ModuleStartup(NULL);
   pPhysXIntegration->SetupCooking();

   /* setup serialization stuff */
   pxSerialRegistry = PxSerialization::createSerializationRegistry(PxGetPhysics());
   pxGlobalCollection = PxCreateCollection();
   pxInstanceCollection = PxCreateCollection();

   switch (mode) {
      case CT_GLTF2MODEL_PHYS_SCENE: CT_RETURN_FAIL(ExtractPxAsBodies(false)); break;
      case CT_GLTF2MODEL_PHYS_COMPOUND: CT_RETURN_FAIL(ExtractPxAsBodies(true)); break;
      case CT_GLTF2MODEL_PHYS_RAGDOLL: CT_RETURN_FAIL(ExtractPxAsArticulation()); break;
      default: ctAssert(0);
   }

   /* write streams */
   instanceStream = ctGltf2ModelPxOutStream();
   globalStream = ctGltf2ModelPxOutStream();
   PxSerialization::serializeCollectionToBinaryDeterministic(
     instanceStream, *pxInstanceCollection, *pxSerialRegistry);
   PxSerialization::serializeCollectionToBinaryDeterministic(
     globalStream, *pxGlobalCollection, *pxSerialRegistry);
   model.physxSerialInstance.data = instanceStream.output.Data();
   model.physxSerialInstance.size = instanceStream.output.Count();
   model.physxSerialGlobal.data = globalStream.output.Data();
   model.physxSerialGlobal.size = globalStream.output.Count();
   pPhysXIntegration->Shutdown();
   return CT_SUCCESS;
}

/* --------------------------------- SCENE --------------------------------- */

ctResults ctGltf2Model::ExtractPxAsBodies(bool isCompound) {
   for (size_t nodeidx = 0; nodeidx < gltf.nodes_count; nodeidx++) {
      /* get node information */
      const cgltf_node& node = gltf.nodes[nodeidx];
      if (!node.mesh) { continue; }
      bool isBlockout = isNodeBlockout(node.name) && !isCompound;
      bool isCollision = isNodeCollision(node.name);
      if (!(isCollision || isBlockout)) { continue; }
      if (isCollision && !node.parent) { /* unparented blockouts are fine */
         ctDebugError("UNPARENTED COLLISION NODE UNSUPPORTED");
         return CT_FAILURE_INVALID_PARAMETER;
      }

      /* get material */
      cgltf_material* gltfmat = node.mesh->primitives[0].material;
      PxSerialObjectId matid = gltfmat ? GetSerialIdForPtr(&node) : 1;
      PxMaterial* material =
        pPhysXIntegration->pPhysics->createMaterial(0.0f, 0.0f, 0.0f);
      material->userData = (void*)ctHornerHash("SURFACENAME");
      // todo properly extract material
      pxGlobalCollection->add(*material, matid);

      /* get shape */
      ctGltf2ModelCollisionType type = getNodeCollisionType(node.name);
      PxShape* shape = NULL;
      switch (type) {
         case CT_GLTF2MODEL_COLLISION_BOX: shape = GetPxBox(node, *material); break;
         case CT_GLTF2MODEL_COLLISION_SPHERE: shape = GetPxSphere(node, *material); break;
         case CT_GLTF2MODEL_COLLISION_PILL: shape = GetPxCapsule(node, *material); break;
         case CT_GLTF2MODEL_COLLISION_TRI: shape = GetPxConvex(node, *material); break;
         case CT_GLTF2MODEL_COLLISION_CONVEX: shape = GetPxTris(node, *material); break;
         default: ctAssert(0);
      }
      pxInstanceCollection->add(*shape, GetSerialIdForPtr(&node));
   }
   return CT_SUCCESS;
}

PxShape* ctGltf2Model::GetPxBox(const cgltf_node& node, PxMaterial& material) {
   return nullptr;
}

PxShape* ctGltf2Model::GetPxSphere(const cgltf_node& node, PxMaterial& material) {
   return nullptr;
}

PxShape* ctGltf2Model::GetPxCapsule(const cgltf_node& node, PxMaterial& material) {
   return nullptr;
}

PxShape* ctGltf2Model::GetPxConvex(const cgltf_node& node, PxMaterial& material) {
   return nullptr;
}

PxShape* ctGltf2Model::GetPxTris(const cgltf_node& node, PxMaterial& material) {
   return nullptr;
}

/* --------------------------------- ARTICULATION --------------------------------- */

ctResults ctGltf2Model::ExtractPxAsArticulation() {
   return CT_SUCCESS;
}
