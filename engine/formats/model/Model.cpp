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
#include "formats/wad/WADCore.h"

#include "lz4/lz4.h"

CT_API ctResults ctModelLoad(ctModel& model, ctFile& file, bool CPUGeometryData) {
   file.ReadRaw(&model.header, sizeof(ctModelHeader), 1);
   if (model.header.magic != CT_MODEL_MAGIC) { return CT_FAILURE_CORRUPTED_CONTENTS; }
   if (model.header.version != CT_MODEL_VERSION) { return CT_FAILURE_UNKNOWN_FORMAT; }

   file.Seek(model.header.wadDataOffset, CT_FILE_SEEK_SET);

   model.mappedCpuDataSize = model.header.wadDataSize;
   model.mappedCpuData = ctMalloc(model.header.wadDataSize);

   /* read cpu data */
   if (model.header.cpuCompressionType == CT_MODEL_CPU_COMPRESS_NONE) {
      file.ReadRaw(model.mappedCpuData, model.mappedCpuDataSize, 1);
   } else if (model.header.cpuCompressionType == CT_MODEL_CPU_COMPRESS_LZ4) {
      void* compBlob = ctMalloc(model.header.cpuCompressionSize);
      file.ReadRaw(compBlob, model.header.cpuCompressionSize, 1);
      LZ4_decompress_safe((const char*)compBlob,
                          (char*)model.mappedCpuData,
                          (int)model.header.cpuCompressionSize,
                          (int)model.mappedCpuDataSize);
      ctFree(compBlob);
   }

   ctWADReader wad;
   int32_t tmpsize = 0;
   ctWADReaderBind(&wad, (uint8_t*)model.mappedCpuData, model.mappedCpuDataSize);

   /* Skeleton */
   ctWADFindLump(&wad, "BXFORMS", (void**)&model.skeleton.transformArray, &tmpsize);
   ctWADFindLump(&wad, "BINVBIND", (void**)&model.skeleton.inverseBindArray, NULL);
   ctWADFindLump(&wad, "BGRAPH", (void**)&model.skeleton.graphArray, NULL);
   ctWADFindLump(&wad, "BHASHES", (void**)&model.skeleton.hashArray, NULL);
   ctWADFindLump(&wad, "BNAMES", (void**)&model.skeleton.nameArray, NULL);
   model.skeleton.boneCount = (uint32_t)tmpsize / sizeof(ctModelSkeletonBoneTransform);

   /* Mesh */
   ctWADFindLump(&wad, "MESHES", (void**)&model.geometry.meshes, &tmpsize);
   model.geometry.meshCount = (uint32_t)tmpsize / sizeof(ctModelMesh);
   ctWADFindLump(&wad, "SUBMESH", (void**)&model.geometry.submeshes, &tmpsize);
   model.geometry.submeshCount = (uint32_t)tmpsize / sizeof(ctModelSubmesh);
   ctWADFindLump(&wad, "MORPHS", (void**)&model.geometry.morphTargets, &tmpsize);
   model.geometry.morphTargetCount = (uint32_t)tmpsize / sizeof(ctModelMeshMorphTarget);
   ctWADFindLump(&wad, "MORPHMAP", (void**)&model.geometry.morphTargetMapping, &tmpsize);
   model.geometry.morphTargetMappingCount =
     (uint32_t)tmpsize / sizeof(ctModelMeshMorphTargetMapping);
   ctWADFindLump(&wad, "MSCATTER", (void**)&model.geometry.scatters, &tmpsize);
   model.geometry.scatterCount = (uint32_t)tmpsize / sizeof(ctModelMeshScatter);

   /* Splines */
   ctWADFindLump(&wad, "SSEGS", (void**)&model.splines.segments, &tmpsize);
   model.splines.segmentCount = (uint32_t)tmpsize / sizeof(ctModelSpline);
   ctWADFindLump(&wad, "SPOS", (void**)&model.splines.positions, &tmpsize);
   ctWADFindLump(&wad, "STAN", (void**)&model.splines.tangents, NULL);
   ctWADFindLump(&wad, "SBITAN", (void**)&model.splines.bitangents, NULL);
   model.splines.pointCount = (uint32_t)tmpsize / sizeof(ctVec3);

   /* Animation */
   ctWADFindLump(&wad, "ACHANS", (void**)&model.animation.channels, &tmpsize);
   model.animation.channelCount = (uint32_t)tmpsize / sizeof(ctModelAnimationChannel);
   ctWADFindLump(&wad, "ACLIPS", (void**)&model.animation.clips, &tmpsize);
   model.animation.clipCount = (uint32_t)tmpsize / sizeof(ctModelAnimationClip);
   ctWADFindLump(&wad, "ASCALARS", (void**)&model.animation.scalars, &tmpsize);
   model.animation.scalarCount = (uint32_t)tmpsize / sizeof(float);

   /* Material Script */
   ctWADFindLump(&wad, "MATSET", (void**)&model.materialSet.data, &tmpsize);
   model.materialSet.size = tmpsize;

   /* PhysX Cook */
   ctWADFindLump(&wad, "PXBAKEG", (void**)&model.physxSerialGlobal.data, &tmpsize);
   model.physxSerialGlobal.size = tmpsize;
   ctWADFindLump(&wad, "PXBAKEI", (void**)&model.physxSerialInstance.data, &tmpsize);
   model.physxSerialInstance.size = tmpsize;

   /* Navmesh */
   ctWADFindLump(&wad, "NAVMESH", (void**)&model.navmeshData.data, &tmpsize);
   model.navmeshData.size = tmpsize;

   /* Scene Script */
   ctWADFindLump(&wad, "SCNCODE", (void**)&model.sceneScript.data, &tmpsize);
   model.sceneScript.size = tmpsize;

   /* GPU info table */
   ctWADFindLump(&wad, "GPUTABLE", (void**)&model.gpuTable, &tmpsize);

   /* Geometry Data */
   if (CPUGeometryData) {
      model.inMemoryGeometryData = (uint8_t*)ctMalloc(model.header.gpuDataSize);
      file.Seek(model.header.gpuDataOffset, CT_FILE_SEEK_SET);
      file.ReadRaw(model.inMemoryGeometryData, model.header.gpuDataSize, 1);
   }

   return CT_SUCCESS;
}

CT_API ctResults ctModelSave(ctModel& model,
                             ctFile& file,
                             ctModelCPUCompression compression) {
   ctWADReader wad = ctWADReader();
   ctWADSetupWrite(&wad);

   /* Skeleton */
   ctWADWriteSection(&wad,
                     "BXFORMS",
                     (uint8_t*)model.skeleton.transformArray,
                     model.skeleton.boneCount * sizeof(model.skeleton.transformArray[0]));
   ctWADWriteSection(&wad,
                     "BINVBIND",
                     (uint8_t*)model.skeleton.inverseBindArray,
                     model.skeleton.boneCount *
                       sizeof(model.skeleton.inverseBindArray[0]));
   ctWADWriteSection(&wad,
                     "BGRAPH",
                     (uint8_t*)model.skeleton.graphArray,
                     model.skeleton.boneCount * sizeof(model.skeleton.graphArray[0]));
   ctWADWriteSection(&wad,
                     "BHASHES",
                     (uint8_t*)model.skeleton.hashArray,
                     model.skeleton.boneCount * sizeof(model.skeleton.hashArray[0]));
   ctWADWriteSection(&wad,
                     "BNAMES",
                     (uint8_t*)model.skeleton.nameArray,
                     model.skeleton.boneCount * sizeof(model.skeleton.nameArray[0]));

   /* Mesh */
   ctWADWriteSection(&wad,
                     "MESHES",
                     (uint8_t*)model.geometry.meshes,
                     model.geometry.meshCount * sizeof(model.geometry.meshes[0]));
   ctWADWriteSection(&wad,
                     "SUBMESH",
                     (uint8_t*)model.geometry.submeshes,
                     model.geometry.submeshCount * sizeof(model.geometry.submeshes[0]));
   ctWADWriteSection(&wad,
                     "MORPHS",
                     (uint8_t*)model.geometry.morphTargets,
                     model.geometry.morphTargetCount *
                       sizeof(model.geometry.morphTargets[0]));
   ctWADWriteSection(&wad,
                     "MORPHMAP",
                     (uint8_t*)model.geometry.morphTargetMapping,
                     model.geometry.morphTargetMappingCount *
                     sizeof(model.geometry.morphTargetMapping[0]));

   /* Spline */
   ctWADWriteSection(&wad,
                     "SSEGS",
                     (uint8_t*)model.splines.segments,
                     model.splines.segmentCount * sizeof(model.splines.segments[0]));
   ctWADWriteSection(&wad,
                     "SPOS",
                     (uint8_t*)model.splines.positions,
                     model.splines.pointCount * sizeof(model.splines.positions[0]));
   ctWADWriteSection(&wad,
                     "STAN",
                     (uint8_t*)model.splines.tangents,
                     model.splines.pointCount * sizeof(model.splines.tangents[0]));
   ctWADWriteSection(&wad,
                     "SBITAN",
                     (uint8_t*)model.splines.bitangents,
                     model.splines.pointCount * sizeof(model.splines.bitangents[0]));

   /* Animations */
   ctWADWriteSection(&wad,
                     "ACLIPS",
                     (uint8_t*)model.animation.clips,
                     model.animation.clipCount * sizeof(model.animation.clips[0]));
   ctWADWriteSection(&wad,
                     "ACHANS",
                     (uint8_t*)model.animation.channels,
                     model.animation.channelCount * sizeof(model.animation.channels[0]));
   ctWADWriteSection(&wad,
                     "ASCALARS",
                     (uint8_t*)model.animation.scalars,
                     model.animation.scalarCount * sizeof(model.animation.scalars[0]));

   /* PhysX */
   ctWADWriteSection(&wad,
                     "PXBAKEG",
                     (uint8_t*)model.physxSerialGlobal.data,
                     model.physxSerialGlobal.size);
   ctWADWriteSection(&wad,
                     "PXBAKEI",
                     (uint8_t*)model.physxSerialInstance.data,
                     model.physxSerialInstance.size);

   /* Navmesh */
   ctWADWriteSection(
     &wad, "NAVMESH", (uint8_t*)model.navmeshData.data, model.navmeshData.size);

   /* Material Set */
   ctWADWriteSection(
     &wad, "MATSET", (uint8_t*)model.materialSet.data, model.materialSet.size);

   /* Object VM */
   ctWADWriteSection(
     &wad, "SCNCODE", (uint8_t*)model.sceneScript.data, model.sceneScript.size);

   /* Dump WAD */
   size_t wadSize;
   void* wadData;
   ctWADToBuffer(&wad, NULL, &wadSize);
   wadData = ctMalloc(wadSize);
   ctWADToBuffer(&wad, (uint8_t*)wadData, &wadSize);

   /* Align GPU Buffer */
   size_t gpuOffset = sizeof(ctModelHeader) + wadSize;
   size_t gpuAlignOffset = gpuOffset % CT_ALIGNMENT_MODEL_GPU;
   gpuOffset += gpuAlignOffset;

   /* Perform compression */
   void* cpuData = wadData;
   size_t cpuDataSize = wadSize;
   if (compression == CT_MODEL_CPU_COMPRESS_LZ4) {
      cpuDataSize = (size_t)LZ4_compressBound((int)wadSize);
      cpuData = ctMalloc(cpuDataSize); /* worst case */
      cpuDataSize = (size_t)LZ4_compress_default(
        (const char*)wadData, (char*)cpuData, (int)wadSize, (int)cpuDataSize);

      /* if compression grows file size, disable it */
      if (cpuDataSize > wadSize) {
         ctFree(cpuData);
         cpuData = wadData;
         cpuDataSize = wadSize;
         compression = CT_MODEL_CPU_COMPRESS_NONE;
      }
   }

   /* Setup Header */
   model.header.magic = CT_MODEL_MAGIC;
   model.header.version = CT_MODEL_VERSION;
   model.header.wadDataOffset = sizeof(ctModelHeader);
   model.header.wadDataSize = wadSize;
   model.header.cpuCompressionSize = cpuDataSize;
   if (model.inMemoryGeometryData) {
      model.header.gpuDataOffset = gpuOffset;
      model.header.gpuDataSize = model.inMemoryGeometryDataSize;
   } else {
      model.header.gpuDataOffset = 0;
      model.header.gpuDataSize = 0;
   }

   /* Write Data */
   file.WriteRaw(&model.header, sizeof(model.header), 1);
   file.WriteRaw(cpuData, cpuDataSize, 1);

   if (model.inMemoryGeometryData) {
      uint8_t padding[CT_ALIGNMENT_MODEL_GPU];
      memset(padding, 0, CT_ALIGNMENT_MODEL_GPU);
      file.WriteRaw(padding, gpuAlignOffset, 1);
      file.WriteRaw(model.inMemoryGeometryData, model.inMemoryGeometryDataSize, 1);
   }

   file.Close();
   if (compression != CT_MODEL_CPU_COMPRESS_NONE) { ctFree(cpuData); }
   ctFree(wadData);
   ctWADWriteFree(&wad);
   return CT_SUCCESS;
}

CT_API void ctModelReleaseGeometry(ctModel& model) {
   if (model.inMemoryGeometryData) { ctFree(model.inMemoryGeometryData); }
   model.inMemoryGeometryData = NULL;
}

CT_API void ctModelRelease(ctModel& model) {
   if (model.mappedCpuData) { ctFree(model.mappedCpuData); }
   if (model.inMemoryGeometryData) { ctFree(model.inMemoryGeometryData); }
   model = ctModel();
}