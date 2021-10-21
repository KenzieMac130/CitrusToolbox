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

#include "../engine/utilities/Common.h"

#include "../engine/formats/wad/WADCore.h"
#include "../engine/formats/wad/prototypes/MarkersAndBlobs.h"
#include "../engine/formats/wad/prototypes/Header.h"

#include "shaderc/shaderc.h"

struct WadWriteSection {
   ctWADLump lump;
   void* data;
};
ctDynamicArray<WadWriteSection> gWadSections;
ctDynamicArray<char> gStringsContent;

const char* gHelpString = "Example:\n\t<OPTIONS> inputGlslPath outputWadPath\n"
                          "Options:"
                          "\n\t-help: Show help";

int32_t gNextLumpOffset = 0;
int32_t MakeSection(const char name[8], size_t size, void* data) {
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

/* Includer */
struct DependencyData {
   ctStringUtf8 name;
   ctDynamicArray<char> contents;
};

shaderc_include_result* IncludeFetch(void* user_data,
                                     const char* requested_source,
                                     int type,
                                     const char* requesting_source,
                                     size_t include_depth) {
   shaderc_include_result* result = new shaderc_include_result();
   DependencyData* depData = new DependencyData();
   result->user_data = depData;

   depData->name = requesting_source;
   depData->name.FilePathPop();
   depData->name.FilePathAppend(requested_source);
   depData->name.FilePathLocalize();
   result->source_name = depData->name.CStr();
   result->source_name_length = depData->name.ByteLength();

   FILE* pFile = fopen(depData->name.CStr(), "rb");
   if (pFile) {
      fseek(pFile, 0, SEEK_END);
      size_t size = ftell(pFile);
      fseek(pFile, 0, SEEK_SET);

      depData->contents.Resize(size + 1);
      depData->contents.Memset(0);
      fread(depData->contents.Data(), 1, size, pFile);
      fclose(pFile);
      result->content = depData->contents.Data();
      result->content_length = size;
   } else {
      result->content = NULL;
      result->content_length = 0;
      result->source_name = NULL;
      result->source_name_length = 0;
   }
   return result;
}

void IncludeRelease(void* user_data, shaderc_include_result* include_result) {
   delete (DependencyData*)include_result->user_data;
   delete include_result;
}

/* Entries */
struct ShaderEntry {
   uint8_t* pData;
   size_t size;
};

struct stage {
   ctStringUtf8 blobName;
   ctStringUtf8 stageDefinition;
   shaderc_shader_kind kind;
   ShaderEntry entry;
};

struct fx {
   ctStringUtf8 fxDefinitions;
   ctDynamicArray<stage> stages;
};

ctDynamicArray<fx> gFxLevels;

int main(int argc, char* argv[]) {
   if (argc < 3) {
      ctDebugError("Not enough args!\n%s", gHelpString);
      return -1;
   }

   if (FindFlag("-help")) { ctDebugLog(gHelpString); }

   const char* glslPath = argv[1];
   const char* outPath = argv[2];

   /* Common Citrus Header */
   ctWADProtoHeader header = ctWADProtoHeader();
   header.magic = CT_WADPROTO_HEADER_MAGIC;
   header.revision = CT_WADPROTO_HEADER_INTERNAL_REV;
   MakeSection(CT_WADPROTO_NAME_HEADER, sizeof(header), &header);

   ctStringUtf8 relativePath = glslPath;
   relativePath.FilePathPop();

   shaderc_compiler_t compiler = shaderc_compiler_initialize();

   /* Get file data */
   FILE* pFile = fopen(glslPath, "rb");
   char* contentsData = NULL;
   size_t contentsSize = 0;
   if (!pFile) { return -1; }
   fseek(pFile, 0, SEEK_END);
   contentsSize = ftell(pFile);
   fseek(pFile, 0, SEEK_SET);
   contentsData = (char*)ctMalloc(contentsSize + 1);
   memset(contentsData, 0, contentsSize + 1);
   fread(contentsData, 1, contentsSize, pFile);
   fclose(pFile);

   bool canTess = false;
   bool wantVertFrag = false;
   bool wantCallable = false;
   bool wantTess = false;
   bool wantCompute = false;
   bool wantRaytrace = false;
   bool wantLod = false;
   if (FindFlag("-surface")) {
      wantVertFrag = true;
      canTess = true;
      wantCallable = true;
   } else if (FindFlag("-raster")) {
      wantVertFrag = true;
      canTess = true;
   } else if (FindFlag("-compute")) {
      wantCompute = true;
   } else if (FindFlag("-raytrace")) {
      wantRaytrace = true;
   }
   if (canTess && FindFlag("-tess")) { wantTess = true; }
   if (FindFlag("-lod")) { wantLod = true; }

   fx defaultFx = fx();
   defaultFx.fxDefinitions = "QUALITY_HIGH";
   defaultFx.stages.Reserve(16);

   if (wantVertFrag) {
      defaultFx.stages.Append(
        {CT_WADBLOB_NAME_SHADER_VERT_SPIRV, "VERTEX_SHADER", shaderc_vertex_shader, {0}});
      defaultFx.stages.Append(
        {CT_WADBLOB_NAME_SHADER_FRAG_SPIRV, "FRAGMENT_SHADER", shaderc_fragment_shader, {0}});
   }
   if (wantTess) {
      defaultFx.stages.Append({CT_WADBLOB_NAME_SHADER_TESS_CONTROL_SPIRV,
                               "TESS_CONTROL_SHADER",
                               shaderc_tess_control_shader,
                               {0}});
      defaultFx.stages.Append({CT_WADBLOB_NAME_SHADER_TESS_EVALUATION_SPIRV,
                               "TESS_EVALUATION_SHADER",
                               shaderc_tess_evaluation_shader,
                               {0}});
   }
   if (wantCompute) {
      defaultFx.stages.Append(
        {CT_WADBLOB_NAME_SHADER_COMPUTE_SPIRV, "COMPUTE_SHADER", shaderc_compute_shader, {0}});
   }
   if (wantCallable) {
      defaultFx.stages.Append({CT_WADBLOB_NAME_SHADER_CALLABLE_SPIRV,
                               "CALLABLE_SHADER",
                               shaderc_callable_shader,
                               {0}});
   }
   if (wantRaytrace) {
      defaultFx.stages.Append({CT_WADBLOB_NAME_SHADER_RAY_GENERATION_SPIRV,
                               "RAYGEN_SHADER",
                               shaderc_raygen_shader,
                               {0}});
      defaultFx.stages.Append({CT_WADBLOB_NAME_SHADER_RAY_ANY_HIT_SPIRV,
                               "ANY_HIT_SHADER",
                               shaderc_anyhit_shader,
                               {0}});
      defaultFx.stages.Append({CT_WADBLOB_NAME_SHADER_RAY_CLOSEST_HIT_SPIRV,
                               "CLOSEST_HIT_SHADER",
                               shaderc_closesthit_shader,
                               {0}});
      defaultFx.stages.Append(
        {CT_WADBLOB_NAME_SHADER_RAY_MISS_SPIRV, "MISS_SHADER", shaderc_miss_shader, {0}});
      defaultFx.stages.Append({CT_WADBLOB_NAME_SHADER_RAY_INTERSECTION_SPIRV,
                               "INTERSECTION_SHADER",
                               shaderc_intersection_shader,
                               {0}});
   }

   gFxLevels.Reserve(2);
   gFxLevels.Append(defaultFx);
   if (wantLod) {
      fx lowFx = defaultFx;
      lowFx.fxDefinitions = "QUALITY_LOW";
      gFxLevels.Append(lowFx);
   }

   for (int i = 0; i < gFxLevels.Count(); i++) {
      const fx fxLevel = gFxLevels[i];
      for (int j = 0; j < fxLevel.stages.Count(); j++) {
         const stage curStage = fxLevel.stages[j];
         /* Setup compiler */
         shaderc_compile_options_t compilerOptions = shaderc_compile_options_initialize();
         shaderc_compile_options_add_macro_definition(
           compilerOptions,
           curStage.stageDefinition.CStr(),
           curStage.stageDefinition.ByteLength(),
           "1",
           1);
         shaderc_compile_options_set_forced_version_profile(
           compilerOptions, 460, shaderc_profile_core);

         /* Includer */
         shaderc_compile_options_set_include_callbacks(
           compilerOptions, IncludeFetch, IncludeRelease, NULL);

         shaderc_compilation_result_t results = shaderc_compile_into_spv(compiler,
                                                                         contentsData,
                                                                         contentsSize,
                                                                         curStage.kind,
                                                                         glslPath,
                                                                         "main",
                                                                         compilerOptions);
         /* Handle errors */
         if (shaderc_result_get_num_errors(results)) {
            const char* error = shaderc_result_get_error_message(results);
            ctDebugLog(error);
            return -2;
         } else {
            ShaderEntry entry = ShaderEntry();
            entry.size = shaderc_result_get_length(results);
            entry.pData = (uint8_t*)ctMalloc(entry.size);
            memset(entry.pData, 0, entry.size);
            memcpy(entry.pData, shaderc_result_get_bytes(results), entry.size);
            gFxLevels[i].stages[j].entry = entry;
         }
         shaderc_compile_options_release(compilerOptions);
      }
   }
   shaderc_compiler_release(compiler);

   /* Write Shader Sections */
   for (int i = 0; i < gFxLevels.Count(); i++) {
      MakeSection(
        CT_WADMARKER_NAME_FX_START, gStringsContent.Count(), gStringsContent.Data());
      for (size_t j = 0; j < gFxLevels[i].stages.Count(); j++) {
         const stage curStage = gFxLevels[i].stages[j];
         MakeSection(curStage.blobName.CStr(), curStage.entry.size, curStage.entry.pData);
      }
      MakeSection(
        CT_WADMARKER_NAME_FX_START, gStringsContent.Count(), gStringsContent.Data());
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

   for (int i = 0; i < gFxLevels.Count(); i++) {
      for (size_t j = 0; j < gFxLevels[i].stages.Count(); j++) {
         ctFree(gFxLevels[i].stages[j].entry.pData);
      }
   }

   return 0;
}