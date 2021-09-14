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
int32_t MakeSection(char name[8], size_t size, void* data) {
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
   for (int i = 1; i < argc - 3; i++) {
      if (ctCStrEql(argv[i], name)) { return true; }
   }
   return false;
}

#define FindParamOccurance(_name, _occ) _FindParamOccurance(_name, _occ, argc, argv)
char* _FindParamOccurance(const char* name, int occurance, int argc, char* argv[]) {
   int j = 0;
   for (int i = 1; i < argc - 3; i++) {
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
ctDynamicArray<ShaderEntry> gShaderEntries;

int main(int argc, char* argv[]) {
   if (argc < 3) {
      ctDebugError("Not enough args!\n%s", gHelpString);
      return -1;
   }

   if (FindFlag("-help")) { ctDebugLog(gHelpString); }

   const char* gltfPath = argv[argc - 2];
   const char* outPath = argv[argc - 1];

   /* Common Citrus Header */
   ctWADProtoHeader header = ctWADProtoHeader();
   header.magic = CT_WADPROTO_HEADER_MAGIC;
   header.revision = CT_WADPROTO_HEADER_INTERNAL_REV;
   MakeSection(CT_WADPROTO_NAME_HEADER, sizeof(header), &header);

   ctStringUtf8 relativePath = gltfPath;
   relativePath.FilePathPop();

   shaderc_compiler_t compiler = shaderc_compiler_initialize();

   /* Get file data */
   FILE* pFile = fopen(gltfPath, "rb");
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

   const char* domainDefinitions[] = {"VERTEX_SHADER", "FRAGMENT_SHADER"};
   shaderc_shader_kind domainTypes[] = {shaderc_vertex_shader, shaderc_fragment_shader};
   for (int i = 0; i < 2; i++) {
      /* Setup compiler */
      shaderc_compile_options_t compilerOptions = shaderc_compile_options_initialize();
      shaderc_compile_options_add_macro_definition(
        compilerOptions, domainDefinitions[i], strlen(domainDefinitions[i]), "1", 1);
      shaderc_compile_options_set_forced_version_profile(
        compilerOptions, 450, shaderc_profile_core);

      /* Includer */
      shaderc_compile_options_set_include_callbacks(
        compilerOptions, IncludeFetch, IncludeRelease, NULL);

      shaderc_compilation_result_t results = shaderc_compile_into_spv(compiler,
                                                                      contentsData,
                                                                      contentsSize,
                                                                      domainTypes[i],
                                                                      gltfPath,
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
         gShaderEntries.Append(entry);
      }
      shaderc_compile_options_release(compilerOptions);
   }
   shaderc_compiler_release(compiler);

   /* Write Shader Sections */
   MakeSection(
     CT_WADMARKER_NAME_FX_START, gStringsContent.Count(), gStringsContent.Data());
   char* domainLumps[] = {CT_WADBLOB_NAME_VERT_SPIRV, CT_WADBLOB_NAME_FRAG_SPIRV};
   for (size_t i = 0; i < gShaderEntries.Count(); i++) {
      MakeSection(domainLumps[i], gShaderEntries[i].size, gShaderEntries[i].pData);
   }
   MakeSection(
     CT_WADMARKER_NAME_FX_START, gStringsContent.Count(), gStringsContent.Data());

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

   for (size_t i = 0; i < gShaderEntries.Count(); i++) {
      ctFree(gShaderEntries[i].pData);
   }

   return 0;
}