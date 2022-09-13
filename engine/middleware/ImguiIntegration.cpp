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

ctResults ctImguiIntegration::Startup() {
   ZoneScoped;
   ctDebugLog("Starting DearImgui...");
   ImGui::CreateContext();
#if !CITRUS_HEADLESS
#ifdef CITRUS_GFX_VULKAN
   ImGui_ImplSDL2_InitForVulkan(Engine->WindowManager->mainWindow);
#endif
   Engine->OSEventManager->MiscEventHandlers.Append({processImguiEvent, this});
   Engine->OSEventManager->WindowEventHandlers.Append({processImguiEvent, this});
   iniPath = Engine->FileSystem->GetPreferencesPath();
   iniPath += "imgui.ini";
   ImGui::GetIO().IniFilename = iniPath.CStr();
   ImGui::GetIO().Fonts->AddFontDefault();
   ImGui::GetIO().Fonts->Build();
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
   ctAssert(pDrawData);
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
   ctAssert(pDrawData);
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
                                         TinyImageFormat colorFormat,
                                         TinyImageFormat depthFormat) {
#if !CITRUS_HEADLESS
   ImGui::Render();

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
   ImGui::GetIO().Fonts->SetTexID(fontBind);
   ctGPUBindlessManagerMapTexture(pGPUDevice, pBindless, fontBind, pFontTexture);

   /* Index Buffer */
   ctGPUExternalBufferCreateFuncInfo iBufferInfo = {};
   iBufferInfo.debugName = "Imgui Indices";
   iBufferInfo.async = false;
   iBufferInfo.pPlaceholder = NULL;
   iBufferInfo.type = CT_GPU_EXTERN_BUFFER_TYPE_STORAGE;
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
   vBufferInfo.type = CT_GPU_EXTERN_BUFFER_TYPE_STORAGE;
   vBufferInfo.updateMode = CT_GPU_UPDATE_STREAM;
   vBufferInfo.size = maxVerts * sizeof(ImDrawVert);
   vBufferInfo.generationFunction = ctImguiUploadIndices;
   vBufferInfo.userData = NULL;
   ctGPUExternalBufferCreateFunc(
     pGPUDevice, pGPUBufferPool, &pVertexBuffer, &iBufferInfo);
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

   ctGPUPipelineBuilderAddShader(pPipelineBuilder, CT_GPU_SHADER_VERT, vertShader);
   ctGPUPipelineBuilderAddShader(pPipelineBuilder, CT_GPU_SHADER_FRAG, fragShader);
   ctGPUPipelineCreate(pGPUDevice, pPipelineBuilder, &pPipeline, pBindless);

   ctGPUPipelineBuilderDelete(pPipelineBuilder);
   ctGPUShaderSoftRelease(pGPUDevice, fragShader);
   ctGPUShaderSoftRelease(pGPUDevice, vertShader);

   ImGui::NewFrame();
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
   ctGPUExternalBuffer* externBuffers[2] = {pVertexBuffer, pIndexBuffer};
   ctGPUExternalBufferRebuild(pGPUDevice, pGPUBufferPool, 2, externBuffers);
   return CT_SUCCESS;
}

#include "gpu/Architect.h"

void ctImguiIntegration::_DrawGPU(struct ctGPUArchitectExecutionContext* pCtx) {
#if !CITRUS_HEADLESS
   ctGPUCommandBuffer gpuCmd = pCtx->cmd;
   ImDrawData* pDrawData = ImGui::GetDrawData();
   ctAssert(pDrawData);
   uint32_t offset = 0;
   ctGPUCmdSetGraphicsPipeline(gpuCmd, pPipeline);
   ctGPUCmdSetViewport(
     gpuCmd, 0, 0, (float)pCtx->raster.width, (float)pCtx->raster.height, 0.0f, 1.0f);
   for (int i = 0; i < pDrawData->CmdListsCount; i++) {
      const ImDrawList* pList = pDrawData->CmdLists[i];
      ImVec2 clipMin = pList->GetClipRectMin();
      ImVec2 clipMax = pList->GetClipRectMax();
      for (int j = 0; j < pList->CmdBuffer.Size; j++) {
         const ImDrawCmd* pCmd = &pList->CmdBuffer.Data[j];
         const uint32_t width = (uint32_t)(
           ((float)(pCmd->ClipRect.z - pCmd->ClipRect.x) / pDrawData->DisplaySize.x) *
           pCtx->raster.width);
         const uint32_t height = (uint32_t)(
           ((float)(pCmd->ClipRect.w - pCmd->ClipRect.y) / pDrawData->DisplaySize.y) *
           pCtx->raster.height);
         const uint32_t offsetX = (uint32_t)(
           (float)((pCmd->ClipRect.x) / pDrawData->DisplaySize.x) * pCtx->raster.width);
         const uint32_t offsetY = (uint32_t)(
           (float)((pCmd->ClipRect.y) / pDrawData->DisplaySize.y) * pCtx->raster.height);
         ctGPUCmdSetScissor(gpuCmd, offsetX, offsetY, width, height);
         ctGPUCmdSetDynamicInteger(
           gpuCmd, pCtx->pBindingModel, 0, (int32_t)pCmd->TextureId);
         ctGPUCmdDraw(gpuCmd, pCmd->ElemCount, 1, pCmd->IdxOffset + offset, 0);
      }
      offset += pList->IdxBuffer.Size;
   }
#endif
}

ctResults ctImguiIntegration::DrawCallback(ctGPUArchitectExecutionContext* pCtx,
                                           void* pUserData) {
   ((ctImguiIntegration*)pUserData)->_DrawGPU(pCtx);
   return CT_SUCCESS;
}

ctResults ctImguiIntegration::NextFrame() {
#if CITRUS_HEADLESS
   ImGui::EndFrame();
   ImGui::NewFrame();
#else
   ImGui_ImplSDL2_NewFrame(Engine->WindowManager->mainWindow);
   ImGui::NewFrame();
   if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow)) {
      Engine->Interact->isFrameActive = false;
   }
#endif
   return CT_SUCCESS;
}
