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

#include "Model.hpp"

struct ctModelPtrFixupCtx {
   uint8_t* base;
   size_t* table;
};

void* FixupPointer(ctModelPtrFixupCtx ctx, void* input) {
   if (!input) { return NULL; }
   while (*ctx.table != SIZE_MAX) {
      if (*ctx.table == (size_t)input) { return ctx.base + *ctx.table; }
      ctx.table++;
   }
}

CT_API ctResults ctModelLoad(ctModel& model, ctFile& file) {
   return CT_SUCCESS;
}

size_t WriteBlock(ctFile& file, void* data, size_t size) {
   size_t offset = (size_t)file.Tell();
   file.WriteRaw(data, size, 1);
   return offset;
}

#define WRITE_ARRAY(STRUCT, DATA_PATH, SIZE_PATH)                                        \
   relmodel.DATA_PATH = (STRUCT*)(WriteBlock(                                            \
     file, (void*)relmodel.DATA_PATH, sizeof(STRUCT) * relmodel.SIZE_PATH));

#define DUPLICATE_ARRAY(STRUCT, DATA_PATH, SIZE_PATH)                                    \
   relmodel.DATA_PATH = (STRUCT*)ctMalloc(sizeof(STRUCT) * relmodel.SIZE_PATH);          \
   memcpy(relmodel.DATA_PATH, model.DATA_PATH, sizeof(STRUCT) * relmodel.SIZE_PATH);

CT_API ctResults ctModelSave(ctModel& model, ctFile& file) {
   ctModel relmodel = model;
   file.Seek(sizeof(ctModel), CT_FILE_SEEK_CUR);

   /* external files */
   WRITE_ARRAY(ctModelExternalFileEntry,
               externalFiles.externalFiles,
               externalFiles.externalFileCount);

   ctModelEmbedSection* tmpembedbuff = NULL;
   ctModelSpline* tmpsplinebuff = NULL;

   /* embeds */
   if (relmodel.embeddedData.sections) {
       tmpembedbuff = DUPLICATE_ARRAY(
        ctModelEmbedSection, embeddedData.sections, embeddedData.sectionCount);
      for (uint32_t i = 0; i < model.embeddedData.sectionCount; i++) {
         WRITE_ARRAY(
           uint8_t, embeddedData.sections[i].data, embeddedData.sections[i].size);
      }
      WRITE_ARRAY(ctModelEmbedSection, embeddedData.sections, embeddedData.sectionCount);
   }

   /* skeleton */
   WRITE_ARRAY(
     ctModelSkeletonBoneTransform, skeletonData.transformArray, skeletonData.boneCount);
   WRITE_ARRAY(ctModelSkeletonBoneGraph, skeletonData.graphArray, skeletonData.boneCount);
   WRITE_ARRAY(uint32_t, skeletonData.hashArray, skeletonData.boneCount);
   WRITE_ARRAY(ctModelSkeletonBoneName, skeletonData.nameArray, skeletonData.boneCount);

   /* mesh (nongeometry) */
   WRITE_ARRAY(ctModelMeshInstance, meshData.instances, meshData.instanceCount);
   WRITE_ARRAY(ctModelMesh, meshData.meshes, meshData.meshCount);
   WRITE_ARRAY(ctModelSubmesh, meshData.submeshes, meshData.submeshCount);
   WRITE_ARRAY(ctModelMorphTarget, meshData.morphTargets, meshData.morphTargetCount);

   /* material */
   WRITE_ARRAY(ctModelMaterial, materialData.materials, materialData.materialCount);

   /* light */
   WRITE_ARRAY(ctModelLight, lightData.lights, lightData.lightCount);

   /* spline */
   tmpsplinebuff = DUPLICATE_ARRAY(ctModelSpline, splineData.splines, splineData.splineCount);
   for (uint32_t i = 0; i < model.splineData.splineCount; i++) {
      WRITE_ARRAY(
        float, splineData.splines[i].positions, splineData.splines[i].pointCount * 3);
      WRITE_ARRAY(
        float, splineData.splines[i].tangents, splineData.splines[i].pointCount * 3);
      WRITE_ARRAY(
        float, splineData.splines[i].bitangents, splineData.splines[i].pointCount * 3);
   }
   WRITE_ARRAY(ctModelSpline, splineData.splines, splineData.splineCount);

   /* animation */

   /* geometry */

   ctFree(tmpsplinebuff);
   return CT_SUCCESS;
}

CT_API void ctModelRelease(ctModel& model) {
   if (model.mappedCpuData) { ctFree(model.mappedCpuData); }
   if (model.meshData.inMemoryGeometryData) {
      ctFree(model.meshData.inMemoryGeometryData);
   }
   model = ctModel();
}