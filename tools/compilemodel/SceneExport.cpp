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
#include "scripting/lua/LuaScript.hpp"

void ctGltf2Model::SpawnScriptStart(const char* type) {
   sceneScript.Printf(0, "ctSpawnBegin(\"%s\")\n", type);
}

void ctGltf2Model::SpawnScriptNumber(const char* key, int64_t value) {
   sceneScript.Printf(0, "ctSpawnNum(\"%s\", %" PRId64 ")\n", key, value);
}

void ctGltf2Model::SpawnScriptNumber(const char* key, float value) {
   sceneScript.Printf(0, "ctSpawnNum(\"%s\", %f)\n", key, value);
}

void ctGltf2Model::SpawnScriptBool(const char* key, bool value) {
   sceneScript.Printf(0, "ctSpawnBool(\"%s\", %s)\n", key, value ? "true" : "false");
}

void ctGltf2Model::SpawnScriptString(const char* key, const char* value) {
   ctStringUtf8 str = value;
   str.ExpandToEscapeCodes();
   sceneScript.Printf(0, "ctSpawnString(\"%s\", \"%s\")\n", key, str.CStr());
}

void ctGltf2Model::SpawnScriptVec2(const char* key, ctVec2 value) {
   sceneScript.Printf(0, "ctSpawnVec2(\"%s\", %f, %f)\n", key, value.x, value.y);
}

void ctGltf2Model::SpawnScriptVec3(const char* key, ctVec3 value) {
   sceneScript.Printf(
     0, "ctSpawnVec3(\"%s\", %f, %f, %f)\n", key, value.x, value.y, value.z);
}

void ctGltf2Model::SpawnScriptVec4(const char* key, ctVec4 value) {
   sceneScript.Printf(
     0, "ctSpawnVec4(\"%s\", %f, %f, %f, %f)\n", key, value.x, value.y, value.z, value.w);
}

void ctGltf2Model::SpawnScriptEnd() {
   sceneScript += "ctSpawnEnd()\n";
}

ctStringUtf8 ctGltf2Model::GetSpawnTypeName(const cgltf_node& node) {
   /* the default scene entity handles collision, lights, and meshes */
   return "default";
}

int32_t ctGltf2Model::GetNodeMeshAssociation(const cgltf_node& node) {
   return -1;
}

int32_t ctGltf2Model::GetNodeScatterAssociation(const cgltf_node& node) {
   return -1;
}

int32_t ctGltf2Model::GetNodeCollisionAssociation(const cgltf_node& node) {
   return -1;
}

int32_t ctGltf2Model::GetNodeSplineAssociation(const cgltf_node& node,
                                               int32_t currentIndex,
                                               ctStringUtf8& nameOut) {
   return -1;
}

void ctGltf2Model::TryCreateSpawnerForNode(const cgltf_node& node) {
   /* ignore lod/collision/navmesh info nodes */
   if (!isNodePreserved(node.name)) { return; }

   /* setup spawning */
   SpawnScriptStart(GetSpawnTypeName(node).CStr());

   /* write name */
   SpawnScriptString("name", node.name);

   /* write world transform */
   ctMat4 worldMatrix;
   cgltf_node_transform_world(&node, &worldMatrix.data[0][0]);
   ctVec3 translation;
   ctQuat rotation;
   ctVec3 scale;
   ctMat4AwkwardDecompose(worldMatrix, translation, rotation, scale);
   SpawnScriptVec3("translation", translation);
   SpawnScriptVec4("rotation", ctVec4(rotation.data));
   SpawnScriptVec3("scale", scale);

   /* light properties */
   if (node.light) {
      const cgltf_light& light = *node.light;
      const char* lightTypes[] = {"point", "directional", "point", "spot"};
      SpawnScriptString("light_type", lightTypes[light.type]);
      SpawnScriptNumber("light_intensity", light.intensity);
      SpawnScriptVec4("light_color", light.color);
      SpawnScriptNumber("light_range", light.range);
      if (light.type == cgltf_light_type_spot) {
         SpawnScriptVec2(
           "light_spot_angles",
           ctVec2(light.spot_inner_cone_angle, light.spot_outer_cone_angle));
      }
   }

   /* mesh */
   int32_t meshIdx = GetNodeMeshAssociation(node);
   if (meshIdx >= 0) { SpawnScriptNumber("mesh_index", (int64_t)meshIdx); }

   /* scatter */
   int32_t scatterIdx = GetNodeScatterAssociation(node);
   if (scatterIdx >= 0) { SpawnScriptNumber("scatter_index", (int64_t)scatterIdx); }

   /* collision */
   int32_t collisionIdx = GetNodeCollisionAssociation(node);
   if (collisionIdx >= 0) { SpawnScriptNumber("collision_index", (int64_t)collisionIdx); }

   /* splines */
   for (int32_t i = 0; true; i++) {
      ctStringUtf8 splineName;
      int32_t splineIdx = GetNodeSplineAssociation(node, i, splineName);
      if (splineIdx < 0) { break; }
      ctStringUtf8 propName = "spline_";
      propName += splineName;
      SpawnScriptNumber(propName.CStr(), (int64_t)splineIdx);
   }

   /* custom spawner args */
   // todo

   SpawnScriptEnd();
}

ctResults ctGltf2Model::ExtractSceneScript() {
   ctDebugLog("Exporting Scene Script...");
   sceneScript.Reserve(
     1000000); /* reserve 1mb for scene script (should be only a few kb) */
   for (size_t i = 0; i < gltf.nodes_count; i++) {
      TryCreateSpawnerForNode(gltf.nodes[i]);
   }
   ctLuaContext lua = ctLuaContext();
   lua.Startup(false);
   CT_RETURN_FAIL(
     lua.LoadFromBuffer(sceneScript.CStr(), sceneScript.ByteLength(), "scene"));
   CT_RETURN_FAIL(lua.Compile(sceneScriptCompiled));
   lua.Shutdown();
   model.sceneScript.data = sceneScriptCompiled.Data();
   model.sceneScript.size = sceneScriptCompiled.Count();
   return CT_SUCCESS;
}