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

#include "ExportPhaseBase.hpp"

#include "SkeletonExport.hpp"
#include "MeshExport.hpp"

#define CGLTF_IMPLEMENTATION
#include "cgltf/cgltf.h"

// clang-format off
const char* gHelpString = "Example:\n\t<OPTIONS> inputFilePath outputFilePath assetCompilerPath\n"
                          "Options:"
                          "\n\t--help: Show help"
                          "\n\t--mode: Model mode: (single, articulation, level, destructable, vat)"
                          "\n\t--curves: Import curves"
                          "\n\t--physics: Export physics shapes and bakes (always for destructibles)"
                          "\n\t--mass_scale: Scale applied to object mass"
                          "\n\t--physmat: Physics material override"
                          "\n\t--bone_anim_list: Comma separated animation list (NAME:OUTPUT_FILE)"
                          "\n\t--blend_shape: Import blend shapes"
                          "\n\t--lods: Import lods"
                          "\n\t--keep_topology: Preserve topology"
                          "\n\t--destructable_original: Original destructable model"
                          "\n\t--destructable_core: Core model left exposed on destruction"
                          "\n\t--destructable_preset: Destruction preset name"
                          "\n\t--blast_bond_mode: Blast bond mode (always/exact)"
                          "\n\t--blast_max_separation: Blast max separation distance";
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
   if (argc < 4) {
      ctDebugError("Not enough args!\n%s", gHelpString);
      return -1;
   }

   if (FindFlag("--help")) { ctDebugLog(gHelpString); }

   const char* inputFilePath = argv[1];
   const char* outputFilePath = argv[2];
   const char* assetCompilerPath = argv[3];

   cgltf_options options = {};
   cgltf_data* pInput;
   if (cgltf_parse_file(&options, inputFilePath, &pInput) != cgltf_result_success) {
      ctDebugError("Could not parse gltf");
      return -2;
   }

   ctModel output = ctModelCreateEmpty();

   ctModelExportContext ctx;
   ctx.singleBone = false;
   ctx.singleMesh = true;

   ctx.pSkeletonExport = new ctModelExportSkeleton();
   ctx.pMeshExport = new ctModelExportMesh();

   ctx.pSkeletonExport->Export(*pInput, output, ctx);
   ctx.pMeshExport->Export(*pInput, output, ctx);

   ctFile file;
   if (file.Open(outputFilePath, CT_FILE_OPEN_WRITE) == CT_SUCCESS) {
      ctModelSave(output, &file);
   }
   return 0;
}