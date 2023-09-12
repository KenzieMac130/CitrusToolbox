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

#define CGLTF_IMPLEMENTATION
#include "CitrusModel.hpp"

// clang-format off
const char* gHelpString = "Example:\n\t<OPTIONS> inputFilePath outputFilePath\n"
                          "Options:"
                          "\n\t--help: Show help"
                          "\n\t--justload: Dont process input, just load the output"
                          "\n\t--viewer: Show viewer";
// clang-format on

#define FindFlag(_name) _FindFlag(_name, argc, argv)

bool _FindFlag(const char* name, int argc, char* argv[]) {
   for (int i = 4; i < argc; i++) {
      if (ctCStrEql(argv[i], name)) { return true; }
   }
   return false;
}

#define FindParamOccurance(_name, _occ) _FindParamOccurance(_name, _occ, argc, argv)

char* _FindParamOccurance(const char* name, int occurance, int argc, char* argv[]) {
   int j = 0;
   for (int i = 4; i < argc; i++) {
      if (ctCStrEql(argv[i], name)) {
         if (j == occurance) { return argv[i + 1]; }
         j++;
      }
   }
   return NULL;
}

#define FindParam(_name) FindParamOccurance(_name, 0)

int main(int argc, char* argv[]) {
   ZoneScoped;
   if (argc < 4) {
      ctDebugError("Not enough args!\n%s", gHelpString);
      return -1;
   }

   if (FindFlag("--help")) {
      ctDebugLog(gHelpString);
      return 0;
   }

   const char* inputFilePath = argv[1];
   const char* outputFilePath = argv[2];
   const char* assetCompilerPath = argv[3];

   ctGltf2Model exporter = ctGltf2Model();

   /* Preview Only */
   if (FindFlag("--just_load")) {
      CT_RETURN_ON_FAIL(exporter.LoadModel(outputFilePath), -1001);
      if (FindFlag("--viewer")) { exporter.ModelViewer(argc, argv); }
      return 0;
   }

   /* Load GLTF */
   CT_RETURN_ON_FAIL(exporter.LoadGltf(inputFilePath), -1000);

   /* Skeleton */
   CT_RETURN_ON_FAIL(exporter.ExtractSkeleton(), -2000);

   /* Material */
   exporter.ExtractMaterials();

   /* Geometry */
   CT_RETURN_ON_FAIL(exporter.ExtractGeometry(FindFlag("--skin")), -3000);
   if (FindFlag("--lod_generate")) {
      const char* paramStr = FindParam("--lod_drop");
      float lodDrop = 0.25f;
      if (paramStr) { lodDrop = (float)atof(paramStr); }

      paramStr = FindParam("--lod_count");
      uint32_t lodCount = 4;
      if (paramStr) { lodCount = (uint32_t)atoi(paramStr); }

      paramStr = FindParam("--lod_quality");
      ctGltf2ModelLodQuality quality = CT_GLTF2MODEL_LODQ_MED;
      if (paramStr) {
         if (ctCStrEql(paramStr, "high")) {
            quality = CT_GLTF2MODEL_LODQ_HIGH;
         } else if (ctCStrEql(paramStr, "medium")) {
            quality = CT_GLTF2MODEL_LODQ_MED;
         } else if (ctCStrEql(paramStr, "low")) {
            quality = CT_GLTF2MODEL_LODQ_LOW;
         }
      }

      CT_RETURN_ON_FAIL(exporter.GenerateLODs(quality, lodCount, lodDrop), -3010);
   }
   if (FindFlag("--merge_mesh")) {
      CT_RETURN_ON_FAIL(exporter.MergeMeshes(FindFlag("--skin")), -3020);
   }
   if (!FindFlag("--skip_tangents")) {
      CT_RETURN_ON_FAIL(exporter.GenerateTangents(), -3030);
   }
   const char* paramStr = FindParam("--overdraw_threshold");
   float param = 1.05f;
   if (paramStr) { param = (float)atof(paramStr); }
   if (!FindFlag("--skip_vertex_cache")) {
      CT_RETURN_ON_FAIL(exporter.OptimizeVertexCache(), -3040);
   }
   if (!FindFlag("--skip_overdraw")) {
      CT_RETURN_ON_FAIL(exporter.OptimizeOverdraw(param), -3050);
   }
   if (!FindFlag("--skip_vertex_fetch")) {
      CT_RETURN_ON_FAIL(exporter.OptimizeVertexFetch(), -3060);
   }
   CT_RETURN_ON_FAIL(exporter.ComputeBounds(), -3080);
   CT_RETURN_ON_FAIL(exporter.EncodeVertices(), -3100);
   CT_RETURN_ON_FAIL(exporter.CreateGeometryBlob(), -3200);

   /* Animations */
   if (FindFlag("--animations")) { exporter.ExtractAnimations(); }

   /* Splines */
   if (FindFlag("--splines")) { exporter.ExtractSplines(); }

   /* Physics */
   paramStr = FindParam("--physics");
   ctGltf2ModelPhysicsMode phys = CT_GLTF2MODEL_PHYS_COMPOUND;
   if (paramStr) {
      if (ctCStrEql(paramStr, "compound")) {
         phys = CT_GLTF2MODEL_PHYS_COMPOUND;
      } else if (ctCStrEql(paramStr, "convex")) {
         phys = CT_GLTF2MODEL_PHYS_CONVEX;
      } else if (ctCStrEql(paramStr, "mesh")) {
         phys = CT_GLTF2MODEL_PHYS_MESH;
      } else if (ctCStrEql(paramStr, "scene")) {
         phys = CT_GLTF2MODEL_PHYS_SCENE;
      } else if (ctCStrEql(paramStr, "ragdoll")) {
         phys = CT_GLTF2MODEL_PHYS_RAGDOLL;
      }
      if (!ctCStrEql(paramStr, "none")) {
         paramStr = FindParam("--surface_override");
         uint32_t surfaceOverride = 0;
         if (paramStr) { surfaceOverride = ctXXHash32(paramStr); }
         exporter.ExtractPhysics(phys, surfaceOverride);
      }
   }

   /* Scene */
   if (FindFlag("--scene")) { exporter.ExtractSceneScript(); }

   /* View Model */
   if (FindFlag("--viewer")) { exporter.ModelViewer(argc, argv); }

   exporter.SaveModel(outputFilePath);

   return 0;
}

ctResults ctGltf2Model::LoadGltf(const char* filepath) {
   ZoneScoped;
   cgltf_options options = {};
   cgltf_data* pGltf;
   if (cgltf_parse_file(&options, filepath, &pGltf) != cgltf_result_success) {
      ctDebugError("COULD NOT PARSE GLTF!");
      return CT_FAILURE_CORRUPTED_CONTENTS;
   }
   if (cgltf_load_buffers(&options, pGltf, filepath) != cgltf_result_success) {
      ctDebugError("COULD NOT LOAD GLTF BUFFERS!");
      return CT_FAILURE_CORRUPTED_CONTENTS;
   }
   gltf = *pGltf;
   model = ctModel();
   gltfRootPath = filepath;
   gltfRootPath.FilePathPop();
   return CT_SUCCESS;
}

ctResults ctGltf2Model::SaveModel(const char* filepath) {
   ZoneScoped;
   ctFile file;
   if (file.Open(filepath, CT_FILE_OPEN_WRITE) == CT_SUCCESS) {
      return ctModelSave(model, file, CT_MODEL_CPU_COMPRESS_LZ4);
   }
   return CT_FAILURE_INACCESSIBLE;
}

ctResults ctGltf2Model::LoadModel(const char* filepath) {
   ZoneScoped;
   ctFile file;
   if (file.Open(filepath, CT_FILE_OPEN_READ) == CT_SUCCESS) {
      return ctModelLoad(model, file, true);
   }
   return CT_FAILURE_INACCESSIBLE;
}