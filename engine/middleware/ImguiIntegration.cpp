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

#include "ImguiIntegration.hpp"

#include "imgui/backends/imgui_impl_sdl.h"
#include "core/EngineCore.hpp"
#include "core/WindowManager.hpp"
#include "core/OSEvents.hpp"
#include "core/FileSystem.hpp"
#include "core/Translation.hpp"
#include "interact/InteractionEngine.hpp"
#include "gpu/Device.h"
#include "gpu/Texture.h"
#include "gpu/Buffer.h"
#include "gpu/Bindless.h"

void processImguiEvent(SDL_Event* event, void* pData) {
   ZoneScoped;
   ctImguiIntegration* pIntegration = (ctImguiIntegration*)pData;
   ImGui_ImplSDL2_ProcessEvent(event);
}

void* ImguiCitrusMalloc(size_t sz, void* user_data) {
   return ctMalloc(sz);
}
void ImguiCitrusFree(void* ptr, void* user_data) {
   ctFree(ptr);
}

ctResults ctImguiIntegration::Startup() {
   ZoneScoped;
   ctDebugLog("Starting DearImgui...");
   ImGui::SetAllocatorFunctions(ImguiCitrusMalloc, ImguiCitrusFree);
   ImGui::CreateContext();
   ImGui::StyleColorsDark();
#if !CITRUS_HEADLESS
#ifdef CITRUS_GFX_VULKAN
   ImGui_ImplSDL2_InitForVulkan(Engine->WindowManager->mainWindow);
#endif
   ImGui::GetIO().Fonts->AddFontDefault();
   ImGui::GetIO().Fonts->Build();
   Engine->OSEventManager->MiscEventHandlers.Append({processImguiEvent, this});
   Engine->OSEventManager->WindowEventHandlers.Append({processImguiEvent, this});
   iniPath = Engine->FileSystem->GetPreferencesPath();
   iniPath += "imgui.ini";
   ImGui::GetIO().IniFilename = iniPath.CStr();

   ImGui::GetIO().DisplaySize.x = 640.0f;
   ImGui::GetIO().DisplaySize.y = 480.0f;
   ImGui::GetIO().DeltaTime = 1.0f / 60.0f;
   ImGui::NewFrame();
#else
   ImGui::GetIO().Fonts->AddFontDefault();
   ImGui::GetIO().Fonts->Build();
   ImGui::GetIO().DisplaySize.x = 640.0f;
   ImGui::GetIO().DisplaySize.y = 480.0f;
   ImGui::GetIO().DeltaTime = 1.0f / 60.0f;
   ImGui::NewFrame();
#endif

   return CT_SUCCESS;
}

ctResults ctImguiIntegration::Shutdown() {
#if !CITRUS_HEADLESS
   ImGui_ImplSDL2_Shutdown();
#endif
   ImGui::DestroyContext();
   return CT_SUCCESS;
}

void ctImguiUploadIndices(uint8_t* dest, size_t size, void* unused) {
   ImDrawData* pDrawData = ImGui::GetDrawData();
   if (!pDrawData) {
      memset(dest, 0, size);
      return;
   }
   size_t offset = 0;
   for (int i = 0; i < pDrawData->CmdListsCount; i++) {
      const ImDrawList* pCmd = pDrawData->CmdLists[i];
      const ImDrawIdx* pIndices = pCmd->IdxBuffer.Data;
      const int count = pCmd->IdxBuffer.Size;
      memcpy(&dest[offset], pIndices, sizeof(pIndices[0]) * count);
      offset += sizeof(pIndices[0]) * count;
   }
}

void ctImguiUploadVertices(uint8_t* dest, size_t size, void* unused) {
   ImDrawData* pDrawData = ImGui::GetDrawData();
   if (!pDrawData) {
      memset(dest, 0, size);
      return;
   }
   size_t offset = 0;
   for (int i = 0; i < pDrawData->CmdListsCount; i++) {
      const ImDrawList* pCmd = pDrawData->CmdLists[i];
      const ImDrawVert* pVertices = pCmd->VtxBuffer.Data;
      const int count = pCmd->VtxBuffer.Size;
      memcpy(&dest[offset], pVertices, sizeof(pVertices[0]) * count);
      offset += sizeof(pVertices[0]) * count;
   }
}

ctResults ctImguiIntegration::StartupGPU(struct ctGPUDevice* pGPUDevice,
                                         struct ctGPUBindlessManager* pBindless,
                                         struct ctGPUExternalBufferPool* pGPUBufferPool,
                                         struct ctGPUExternalTexturePool* pGPUTexturePool,
                                         size_t maxVerts,
                                         size_t maxIndices,
                                         int32_t fontBind,
                                         int32_t idxBind,
                                         int32_t vtxBind,
                                         enum TinyImageFormat colorFormat,
                                         enum TinyImageFormat depthFormat) {
#if !CITRUS_HEADLESS
   /* Font Texture */
   int32_t fontWidth;
   int32_t fontHeight;
   uint8_t* pixels = NULL;
   ImGui::GetIO().Fonts->GetTexDataAsRGBA32(&pixels, &fontWidth, &fontHeight);
   ctGPUExternalTextureCreateFuncInfo fontTexInfo = {};
   fontTexInfo.async = false;
   fontTexInfo.debugName = "Imgui Font";
   fontTexInfo.pPlaceholder = NULL;
   fontTexInfo.type = CT_GPU_EXTERN_TEXTURE_TYPE_2D;
   fontTexInfo.updateMode = CT_GPU_UPDATE_STATIC;
   fontTexInfo.format = TinyImageFormat_R8G8B8A8_UNORM;
   fontTexInfo.width = fontWidth;
   fontTexInfo.height = fontHeight;
   fontTexInfo.depth = 1;
   fontTexInfo.mips = 1;
   fontTexInfo.userData = pixels;
   fontTexInfo.generationFunction = ctGPUTextureGenerateFnQuickMemcpy;
   CT_RETURN_FAIL(ctGPUExternalTextureCreateFunc(
     pGPUDevice, pGPUTexturePool, &pFontTexture, &fontTexInfo));
   fontBind =
     ctGPUBindlessManagerMapTexture(pGPUDevice, pBindless, fontBind, pFontTexture);
   ImGui::GetIO().Fonts->SetTexID((ImTextureID)(size_t)fontBind);

   /* Index Buffer */
   ctGPUExternalBufferCreateFuncInfo iBufferInfo = {};
   iBufferInfo.debugName = "Imgui Indices";
   iBufferInfo.async = false;
   iBufferInfo.pPlaceholder = NULL;
   iBufferInfo.type = CT_GPU_EXTERN_BUFFER_TYPE_INDEX;
   iBufferInfo.updateMode = CT_GPU_UPDATE_STREAM;
   iBufferInfo.size = maxIndices * sizeof(ImDrawIdx);
   iBufferInfo.generationFunction = ctImguiUploadIndices;
   iBufferInfo.userData = NULL;
   ctGPUExternalBufferCreateFunc(pGPUDevice, pGPUBufferPool, &pIndexBuffer, &iBufferInfo);
   ctGPUBindlessManagerMapStorageBuffer(pGPUDevice, pBindless, idxBind, pIndexBuffer);

   /* Vertex Buffer */
   ctGPUExternalBufferCreateFuncInfo vBufferInfo = {};
   vBufferInfo.debugName = "Imgui Vertices";
   vBufferInfo.async = false;
   vBufferInfo.pPlaceholder = NULL;
   vBufferInfo.type = CT_GPU_EXTERN_BUFFER_TYPE_VERTEX;
   vBufferInfo.updateMode = CT_GPU_UPDATE_STREAM;
   vBufferInfo.size = maxVerts * sizeof(ImDrawVert);
   vBufferInfo.generationFunction = ctImguiUploadVertices;
   vBufferInfo.userData = NULL;
   ctGPUExternalBufferCreateFunc(
     pGPUDevice, pGPUBufferPool, &pVertexBuffer, &vBufferInfo);
   ctGPUBindlessManagerMapStorageBuffer(pGPUDevice, pBindless, vtxBind, pVertexBuffer);

   /* Pipeline */
   ctFile wadFile;
   ctDynamicArray<uint8_t> wadBytes;
   ctWADReader wadReader;
   CT_PANIC_FAIL(
     Engine->FileSystem->OpenDataFileByGUID(wadFile, CT_CDATA("Shader_ImGUI")),
     CT_NC("Failed to open shader file!"));
   wadFile.GetBytes(wadBytes);
   ctWADReaderBind(&wadReader, wadBytes.Data(), wadBytes.Count());

   ctGPUShaderModule vertShader;
   ctGPUShaderModule fragShader;
   CT_PANIC_FAIL(ctGPUShaderCreateFromWad(
                   pGPUDevice, &vertShader, &wadReader, NULL, CT_GPU_SHADER_VERT),
                 CT_NC("Failed to create imgui shader!"));
   CT_PANIC_FAIL(ctGPUShaderCreateFromWad(
                   pGPUDevice, &fragShader, &wadReader, NULL, CT_GPU_SHADER_FRAG),
                 CT_NC("Failed to create imgui shader!"));

   ctGPUPipelineBuilder* pPipelineBuilder =
     ctGPUPipelineBuilderNew(pGPUDevice, CT_GPU_PIPELINE_RASTER);
   ctGPUPipelineBuilderSetAttachments(pPipelineBuilder, depthFormat, 1, &colorFormat);
   ctGPUPipelineBuilderSetBlendMode(pPipelineBuilder,
                                    0,
                                    true,
                                    CT_COLOR_COMPONENT_RGBA,
                                    CT_GPU_BLEND_LERP,
                                    CT_GPU_BLEND_DISCARD);

   ctGPUPipelineBuilderAddShader(pPipelineBuilder, CT_GPU_SHADER_VERT, vertShader);
   ctGPUPipelineBuilderAddShader(pPipelineBuilder, CT_GPU_SHADER_FRAG, fragShader);

   ctGPUPipelineBuilderAddVertexBufferBinding(
     pPipelineBuilder, sizeof(ImDrawVert), false);
   ctGPUPipelineBuilderAddVertexBufferAttribute(pPipelineBuilder,
                                                CT_GPU_VERTEX_BUFFER_ATTRIBUTE_POSITION,
                                                TinyImageFormat_R32G32_SFLOAT,
                                                offsetof(ImDrawVert, pos),
                                                0);
   ctGPUPipelineBuilderAddVertexBufferAttribute(pPipelineBuilder,
                                                CT_GPU_VERTEX_BUFFER_ATTRIBUTE_TEXCOORD0,
                                                TinyImageFormat_R32G32_SFLOAT,
                                                offsetof(ImDrawVert, uv),
                                                0);
   ctGPUPipelineBuilderAddVertexBufferAttribute(pPipelineBuilder,
                                                CT_GPU_VERTEX_BUFFER_ATTRIBUTE_COLOR0,
                                                TinyImageFormat_R8G8B8A8_UNORM,
                                                offsetof(ImDrawVert, col),
                                                0);

   ctGPUPipelineCreate(pGPUDevice, pPipelineBuilder, &pPipeline, pBindless);

   ctGPUPipelineBuilderDelete(pPipelineBuilder);
   ctGPUShaderSoftRelease(pGPUDevice, fragShader);
   ctGPUShaderSoftRelease(pGPUDevice, vertShader);
#endif
   return CT_SUCCESS;
}

ctResults
ctImguiIntegration::ShutdownGPU(struct ctGPUDevice* pGPUDevice,
                                struct ctGPUExternalBufferPool* pGPUBufferPool,
                                struct ctGPUExternalTexturePool* pGPUTexturePool) {
#if !CITRUS_HEADLESS
   ctGPUPipelineDestroy(pGPUDevice, pPipeline);
   ctGPUExternalBufferRelease(pGPUDevice, pGPUBufferPool, pVertexBuffer);
   ctGPUExternalBufferRelease(pGPUDevice, pGPUBufferPool, pIndexBuffer);
   ctGPUExternalTextureRelease(pGPUDevice, pGPUTexturePool, pFontTexture);
#endif
   return CT_SUCCESS;
}

ctResults ctImguiIntegration::PrepareFrameGPU(ctGPUDevice* pGPUDevice,
                                              ctGPUExternalBufferPool* pGPUBufferPool) {
   ImGui::EndFrame();
   ctGPUExternalBuffer* externBuffers[2] = {pVertexBuffer, pIndexBuffer};
   ctGPUExternalBufferRebuild(pGPUDevice, pGPUBufferPool, 2, externBuffers);
   return CT_SUCCESS;
}

#include "gpu/Architect.h"

void ctImguiIntegration::DrawGPU(struct ctGPUArchitectExecutionContext* pCtx) {
#if !CITRUS_HEADLESS
   ctGPUCommandBuffer gpuCmd = pCtx->cmd;
   ImDrawData* pDrawData = ImGui::GetDrawData();
   ctAssert(pDrawData);
   uint32_t idxOffset = 0;
   uint32_t vtxOffset = 0;
   ctGPUCmdSetGraphicsPipeline(gpuCmd, pPipeline);

   ctGPUBufferAccessor vtxAccess, idxAccess;
   ctGPUExternalBufferGetCurrentAccessor(pCtx->pDevice, pVertexBuffer, &vtxAccess);
   ctGPUExternalBufferGetCurrentAccessor(pCtx->pDevice, pIndexBuffer, &idxAccess);
   ctGPUCmdSetIndexBuffer(gpuCmd, idxAccess, 0, CT_GPU_INDEX_TYPE_UINT32);
   ctGPUCmdSetVertexBuffers(gpuCmd, 1, &vtxAccess, NULL);
   ctGPUCmdSetDynamicInteger(
     gpuCmd, pCtx->pBindingModel, 0, (int32_t)ImGui::GetIO().DisplaySize.x);
   ctGPUCmdSetDynamicInteger(
     gpuCmd, pCtx->pBindingModel, 1, (int32_t)ImGui::GetIO().DisplaySize.y);

   ctGPUCmdSetViewport(
     gpuCmd, 0, 0, (float)pCtx->raster.width, (float)pCtx->raster.height, 0.0f, 1.0f);
   for (int i = 0; i < pDrawData->CmdListsCount; i++) {
      const ImDrawList* pList = pDrawData->CmdLists[i];
      ImVec2 clipMin = pList->GetClipRectMin();
      ImVec2 clipMax = pList->GetClipRectMax();
      for (int j = 0; j < pList->CmdBuffer.Size; j++) {
         const ImDrawCmd* pCmd = &pList->CmdBuffer.Data[j];
         const uint32_t width = (uint32_t)(((float)(pCmd->ClipRect.z - pCmd->ClipRect.x) /
                                            pDrawData->DisplaySize.x) *
                                           pCtx->raster.width);
         const uint32_t height =
           (uint32_t)(((float)(pCmd->ClipRect.w - pCmd->ClipRect.y) /
                       pDrawData->DisplaySize.y) *
                      pCtx->raster.height);
         const uint32_t offsetX =
           (uint32_t)((float)((pCmd->ClipRect.x) / pDrawData->DisplaySize.x) *
                      pCtx->raster.width);
         const uint32_t offsetY =
           (uint32_t)((float)((pCmd->ClipRect.y) / pDrawData->DisplaySize.y) *
                      pCtx->raster.height);
         ctGPUCmdSetScissor(gpuCmd, offsetX, offsetY, width, height);
         ctGPUCmdSetDynamicInteger(
           gpuCmd, pCtx->pBindingModel, 2, (int32_t)(size_t)pCmd->GetTexID());
         ctGPUCmdDrawIndexed(gpuCmd,
                             pCmd->ElemCount,
                             1,
                             pCmd->IdxOffset + idxOffset,
                             pCmd->VtxOffset + vtxOffset,
                             0);
      }
      idxOffset += pList->IdxBuffer.Size;
      vtxOffset += pList->VtxBuffer.Size;
   }
#endif
}

ctResults ctImguiIntegration::DrawCallback(ctGPUArchitectExecutionContext* pCtx,
                                           void* pUserData) {
   ((ctImguiIntegration*)pUserData)->DrawGPU(pCtx);
   return CT_SUCCESS;
}

ctResults ctImguiIntegration::NextFrame() {
#if CITRUS_HEADLESS
   ImGui::EndFrame();
   ImGui::NewFrame();
#else
   ImGui_ImplSDL2_NewFrame();
   ImGui::NewFrame();
   if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow)) {
      Engine->Interact->isFrameActive = false;
   }
#endif
   return CT_SUCCESS;
}
