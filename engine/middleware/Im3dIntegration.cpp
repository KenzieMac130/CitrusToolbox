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

#include "Im3dIntegration.hpp"
#include "core/EngineCore.hpp"
#include "core/WindowManager.hpp"
#include "core/Translation.hpp"
#include "core/FileSystem.hpp"
#include "im3d/im3d_math.h"
#include "imgui/imgui.h"

#include "gpu/Device.h"
#include "gpu/Buffer.h"
#include "gpu/Pipeline.h"
#include "gpu/Bindless.h"
#include "gpu/Commands.h"
#include "gpu/Architect.h"
#include "gpu/Struct.h"

ctResults ctIm3dIntegration::Startup() {
   return CT_SUCCESS;
}

ctResults ctIm3dIntegration::Shutdown() {
   return CT_SUCCESS;
}

const char* ctIm3dIntegration::GetModuleName() {
   return "Im3d";
}

void ctIm3dIntegration::ctIm3dUploadViewData(uint8_t* dest, size_t size, void* data) {
   ctIm3dIntegration* pIm3d = (ctIm3dIntegration*)data;

   ctMat4 viewProj = pIm3d->cameraInfo.GetViewInverseProjectionMatrix();
   ctVec2 viewportSize = ctVec2FromIm3d(Im3d::GetAppData().m_viewportSize);
   ctGPUStructSetVariable(
     pIm3d->pViewStructAssembler, &viewProj, dest, pIm3d->viewProj, 0);
   ctGPUStructSetVariable(
     pIm3d->pViewStructAssembler, &viewportSize, dest, pIm3d->viewportSize, 0);
}

void ctIm3dIntegration::ctIm3dUploadVertexData(uint8_t* dest, size_t size, void* data) {
   ctIm3dIntegration* pIm3d = (ctIm3dIntegration*)data;
   uint32_t drawListCount = Im3d::GetDrawListCount();

   const Im3d::DrawList* pDrawLists = Im3d::GetDrawLists();
   size_t vtxoffset = 0;
   for (uint32_t i = 0; i < drawListCount; i++) {
      const Im3d::DrawList& drawList = pDrawLists[i];
      ctGPUStructSetVariableMany(pIm3d->pVertexStructAssembler,
                                 (void*)drawList.m_vertexData,
                                 dest,
                                 pIm3d->positionScale,
                                 vtxoffset,
                                 drawList.m_vertexCount,
                                 offsetof(Im3d::VertexData, m_positionSize),
                                 sizeof(Im3d::VertexData));
      ctGPUStructSetVariableMany(pIm3d->pVertexStructAssembler,
                                 (void*)drawList.m_vertexData,
                                 dest,
                                 pIm3d->icolor,
                                 vtxoffset,
                                 drawList.m_vertexCount,
                                 offsetof(Im3d::VertexData, m_color),
                                 sizeof(Im3d::VertexData));
      vtxoffset += drawList.m_vertexCount;
   }
}

ctResults ctIm3dIntegration::StartupGPU(struct ctGPUDevice* pGPUDevice,
                                        struct ctGPUBindlessManager* pBindless,
                                        struct ctGPUExternalBufferPool* pGPUBufferPool,
                                        size_t maxVerts,
                                        int32_t viewBind,
                                        int32_t vtxBind,
                                        enum TinyImageFormat colorFormat,
                                        enum TinyImageFormat depthFormat) {
   /* Define Structs */
   pVertexStructAssembler =
     ctGPUStructAssemblerNew(pGPUDevice, CT_GPU_STRUCT_TYPE_STORAGE);
   positionScale =
     ctGPUStructDefineVariable(pVertexStructAssembler, CT_GPU_SVAR_FLOAT_VEC4);
   icolor = ctGPUStructDefineVariable(pVertexStructAssembler, CT_GPU_SVAR_UINT);

   pViewStructAssembler = ctGPUStructAssemblerNew(pGPUDevice, CT_GPU_STRUCT_TYPE_STORAGE);
   viewProj =
     ctGPUStructDefineVariable(pViewStructAssembler, CT_GPU_SVAR_FLOAT_MATRIX4x4);
   viewportSize = ctGPUStructDefineVariable(pViewStructAssembler, CT_GPU_SVAR_FLOAT_VEC2);

   ctFile wadFile;
   ctDynamicArray<uint8_t> wadBytes;
   ctWADReader wadReader;
   CT_PANIC_FAIL(Engine->FileSystem->OpenDataFileByGUID(wadFile, CT_CDATA("Shader_Im3d")),
                 CT_NC("Failed to open shader file!"));
   wadFile.GetBytes(wadBytes);
   ctWADReaderBind(&wadReader, wadBytes.Data(), wadBytes.Count());

   ctGPUPipelineBuilder* pPipelineBuilder =
     ctGPUPipelineBuilderNew(pGPUDevice, CT_GPU_PIPELINE_RASTER);
   ctGPUPipelineBuilderSetAttachments(pPipelineBuilder, depthFormat, 1, &colorFormat);

   ctGPUTopology fillToTopo[] = {CT_GPU_TOPOLOGY_TRIANGLE_LIST,
                                 CT_GPU_TOPOLOGY_LINE_LIST,
                                 CT_GPU_TOPOLOGY_POINT_LIST};
   const char* fxNames[] = {"TRIS", "LINES", "POINTS"};
   for (int i = 0; i < Im3d::DrawPrimitive_Count; i++) {
      ctGPUShaderModule vertShader;
      ctGPUShaderModule fragShader;
      CT_PANIC_FAIL(
        ctGPUShaderCreateFromWad(
          pGPUDevice, &vertShader, &wadReader, fxNames[i], CT_GPU_SHADER_VERT),
        CT_NC("Failed to create im3d shader!"));
      CT_PANIC_FAIL(
        ctGPUShaderCreateFromWad(
          pGPUDevice, &fragShader, &wadReader, fxNames[i], CT_GPU_SHADER_FRAG),
        CT_NC("Failed to create im3d shader!"));

      ctGPUPipelineBuilderSetFillMode(pPipelineBuilder, (ctGPUFillMode)i);
      ctGPUPipelineBuilderSetFaceCull(pPipelineBuilder, CT_GPU_FACE_NONE);
      ctGPUPipelineBuilderSetTopology(pPipelineBuilder, fillToTopo[i], false);
      ctGPUPipelineBuilderSetDepthTest(pPipelineBuilder, CT_GPU_DEPTHTEST_GREATER);
      ctGPUPipelineBuilderSetDepthWrite(pPipelineBuilder, true);
      ctGPUPipelineBuilderSetBlendMode(pPipelineBuilder,
                                       0,
                                       true,
                                       CT_COLOR_COMPONENT_RGBA,
                                       CT_GPU_BLEND_LERP,
                                       CT_GPU_BLEND_DISCARD);
      ctGPUPipelineBuilderClearShaders(pPipelineBuilder);
      ctGPUPipelineBuilderAddShader(pPipelineBuilder, CT_GPU_SHADER_VERT, vertShader);
      ctGPUPipelineBuilderAddShader(pPipelineBuilder, CT_GPU_SHADER_FRAG, fragShader);
      ctGPUPipelineCreate(pGPUDevice, pPipelineBuilder, &pPipelines[i], pBindless);

      ctGPUShaderSoftRelease(pGPUDevice, fragShader);
      ctGPUShaderSoftRelease(pGPUDevice, vertShader);
   }

   ctGPUPipelineBuilderDelete(pPipelineBuilder);

   /* View Buffer */
   ctGPUExternalBufferCreateFuncInfo viewBufferInfo = {};
   viewBufferInfo.debugName = "Im3d View";
   viewBufferInfo.async = false;
   viewBufferInfo.pPlaceholder = NULL;
   viewBufferInfo.type = CT_GPU_EXTERN_BUFFER_TYPE_STORAGE;
   viewBufferInfo.updateMode = CT_GPU_UPDATE_STREAM;
   viewBufferInfo.size = ctGPUStructGetBufferSize(pViewStructAssembler, 1);
   viewBufferInfo.generationFunction = ctIm3dIntegration::ctIm3dUploadViewData;
   viewBufferInfo.userData = this;
   ctGPUExternalBufferCreateFunc(
     pGPUDevice, pGPUBufferPool, &pViewBuffer, &viewBufferInfo);
   ctGPUBindlessManagerMapStorageBuffer(pGPUDevice, pBindless, viewBind, pViewBuffer);

   /* Vertex Buffer */
   ctGPUExternalBufferCreateFuncInfo vBufferInfo = {};
   vBufferInfo.debugName = "Im3d Vertices";
   vBufferInfo.async = false;
   vBufferInfo.pPlaceholder = NULL;
   vBufferInfo.type = CT_GPU_EXTERN_BUFFER_TYPE_STORAGE;
   vBufferInfo.updateMode = CT_GPU_UPDATE_STREAM;
   vBufferInfo.size = ctGPUStructGetBufferSize(pVertexStructAssembler, maxVerts);
   vBufferInfo.generationFunction = ctIm3dIntegration::ctIm3dUploadVertexData;
   vBufferInfo.userData = this;
   ctGPUExternalBufferCreateFunc(
     pGPUDevice, pGPUBufferPool, &pVertexBuffer, &vBufferInfo);
   ctGPUBindlessManagerMapStorageBuffer(pGPUDevice, pBindless, vtxBind, pVertexBuffer);
   return CT_SUCCESS;
}

ctResults ctIm3dIntegration::ShutdownGPU(ctGPUDevice* pGPUDevice,
                                         ctGPUExternalBufferPool* pGPUBufferPool) {
   for (int i = 0; i < Im3d::DrawPrimitive_Count; i++) {
      ctGPUPipelineDestroy(pGPUDevice, pPipelines[i]);
   }
   ctGPUExternalBufferRelease(pGPUDevice, pGPUBufferPool, pViewBuffer);
   ctGPUExternalBufferRelease(pGPUDevice, pGPUBufferPool, pVertexBuffer);
   ctGPUStructAssemblerDelete(pGPUDevice, pViewStructAssembler);
   ctGPUStructAssemblerDelete(pGPUDevice, pVertexStructAssembler);
   return CT_SUCCESS;
}

void ctIm3dIntegration::SkipGPU() {
   Im3d::EndFrame();
}

ctResults ctIm3dIntegration::PrepareFrameGPU(ctGPUDevice* pGPUDevice,
                                             ctGPUExternalBufferPool* pGPUBufferPool) {
   Im3d::EndFrame();
   ctGPUExternalBuffer* externBuffers[2] = {pVertexBuffer, pViewBuffer};
   ctGPUExternalBufferRebuild(pGPUDevice, pGPUBufferPool, 2, externBuffers);
   triangleCount = 0;
   lineCount = 0;
   pointCount = 0;
   textCount = 0;
   for (uint32_t i = 0; i < Im3d::GetTextDrawListCount(); ++i) {
      const Im3d::TextDrawList& textDrawList = Im3d::GetTextDrawLists()[i];
      textCount += textDrawList.m_textDataCount;
   }
   return CT_SUCCESS;
}

ctResults ctIm3dIntegration::DrawCallbackMain(ctGPUArchitectExecutionContext* pCtx,
                                              void* pUserData) {
   ctIm3dIntegration* pIntegration = (ctIm3dIntegration*)pUserData;
   pIntegration->DrawGPU(pCtx, false);
   return CT_SUCCESS;
}

ctResults ctIm3dIntegration::DrawCallbackXRay(ctGPUArchitectExecutionContext* pCtx,
                                              void* pUserData) {
   ctIm3dIntegration* pIntegration = (ctIm3dIntegration*)pUserData;
   pIntegration->DrawGPU(pCtx, true);
   return CT_SUCCESS;
}

void ctIm3dIntegration::DrawGPU(ctGPUArchitectExecutionContext* pCtx, bool xray) {
   drawListCount = Im3d::GetDrawListCount();
   const Im3d::DrawList* pDrawLists = Im3d::GetDrawLists();

   /* draw main */
   uint32_t offset = 0;
   for (uint32_t i = 0; i < drawListCount; i++) {
      const Im3d::DrawList& drawList = pDrawLists[i];
      if ((drawList.m_layerId != CT_IM3D_LAYER_XRAY && !xray) ||
          (drawList.m_layerId == CT_IM3D_LAYER_XRAY && xray)) {
         ctGPUCmdSetGraphicsPipeline(pCtx->cmd, pPipelines[(int)drawList.m_primType]);
         ctGPUCmdDraw(pCtx->cmd, drawList.m_vertexCount, 1, offset, 0);
         switch (drawList.m_primType) {
            case Im3d::DrawPrimitive_Triangles:
               triangleCount += drawList.m_vertexCount / 3;
               break;
            case Im3d::DrawPrimitive_Lines:
               lineCount += drawList.m_vertexCount / 2;
               break;
            case Im3d::DrawPrimitive_Points: pointCount += drawList.m_vertexCount; break;
            default: break;
         }
      }
      offset += drawList.m_vertexCount;
   }
   vertexCount = offset;
}

void ctIm3dIntegration::DrawImguiText() {
   ZoneScoped;
   Im3d::AppData& appData = Im3d::GetAppData();
   float n = 0.1f;
   float a = (float)appData.m_viewportSize.x / (float)appData.m_viewportSize.y;
   float scale = tanf(cameraInfo.fov * 0.5f) * n;
   float viewZ = 1.0f;
   float r = a * scale;
   float l = -r;
   float t = scale;
   float b = -t;
   // clang-format off
    Im3d::Mat4 m_camProj = Im3d::Mat4(2.0f * n / (r - l), 0.0f, -viewZ * (r + l) / (r - l), 0.0f,
                                      0.0f, 2.0f * n / (t - b), -viewZ * (t + b) / (t - b), 0.0f,
                                      0.0f, 0.0f, viewZ, -2.0f * n,
                                      0.0f, 0.0f, viewZ, 0.0f);
   // clang-format on
   Im3d::Mat4 camView =
     ctMat4ToIm3d(cameraInfo.GetViewMatrix()); /*Im3d::Inverse(Im3d::LookAt(
appData.m_viewOrigin, (appData.m_viewOrigin + appData.m_viewDirection)));*/
   Im3d::Mat4 camViewProj = m_camProj * camView;
   ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32_BLACK_TRANS);
   ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
   ImGui::SetNextWindowSize(ImVec2(appData.m_viewportSize.x, appData.m_viewportSize.y));
   ImGui::Begin("Im3dIntegration",
                nullptr,
                0 | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                  ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground |
                  ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoInputs |
                  ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
                  ImGuiWindowFlags_NoBringToFrontOnFocus);

   ImDrawList* imDrawList = ImGui::GetWindowDrawList();
   for (uint32_t i = 0; i < Im3d::GetTextDrawListCount(); ++i) {
      const Im3d::TextDrawList& textDrawList = Im3d::GetTextDrawLists()[i];
      for (uint32_t j = 0; j < textDrawList.m_textDataCount; ++j) {
         const Im3d::TextData& textData = textDrawList.m_textData[j];
         if (textData.m_positionSize.w == 0.0f || textData.m_color.getA() == 0.0f) {
            continue;
         }

         // Project world -> screen space.
         Im3d::Vec4 clip = camViewProj * Im3d::Vec4(textData.m_positionSize.x,
                                                    textData.m_positionSize.y,
                                                    textData.m_positionSize.z,
                                                    1.0f);
         Im3d::Vec2 screen = Im3d::Vec2(clip.x / clip.w, clip.y / clip.w);

         // Cull text which falls offscreen. Note that this doesn't take into account text
         // size but works well enough in practice.
         if (clip.w < 0.0f || screen.x >= 1.0f || screen.y >= 1.0f) { continue; }

         // Pixel coordinates for the ImGuiWindow ImGui.
         screen = screen * Im3d::Vec2(0.5f) + Im3d::Vec2(0.5f);
         screen.y =
           1.0f - screen.y;  // screen space origin is reversed by the projection.
         screen = screen * Im3d::Vec2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);

         // All text data is stored in a single buffer; each textData instance has an
         // offset into this buffer.
         const char* text = textDrawList.m_textBuffer + textData.m_textBufferOffset;

         // Calculate the final text size in pixels to apply alignment flags correctly.
         ImGui::SetWindowFontScale(
           textData.m_positionSize
             .w);  // NB no CalcTextSize API which takes a font/size directly...
         ImVec2 iTextSize = ImGui::CalcTextSize(text, text + textData.m_textLength);
         Im3d::Vec2 textSize = Im3d::Vec2(iTextSize.x, iTextSize.y);
         ImGui::SetWindowFontScale(1.0f);

         // Generate a pixel offset based on text flags.
         Im3d::Vec2 textOffset =
           Im3d::Vec2(-textSize.x * 0.5f, -textSize.y * 0.5f);  // default to center
         if ((textData.m_flags & Im3d::TextFlags_AlignLeft) != 0) {
            textOffset.x = -textSize.x;
         } else if ((textData.m_flags & Im3d::TextFlags_AlignRight) != 0) {
            textOffset.x = 0.0f;
         }

         if ((textData.m_flags & Im3d::TextFlags_AlignTop) != 0) {
            textOffset.y = -textSize.y;
         } else if ((textData.m_flags & Im3d::TextFlags_AlignBottom) != 0) {
            textOffset.y = 0.0f;
         }

         // Add text to the window draw list.
         screen = screen + textOffset;
         imDrawList->AddText(nullptr,
                             textData.m_positionSize.w * ImGui::GetFontSize(),
                             ImVec2(screen.x, screen.y),
                             textData.m_color.getABGR(),
                             text,
                             text + textData.m_textLength);
      }
   }

   ImGui::End();
   ImGui::PopStyleColor(1);
}

void ctIm3dIntegration::SetCamera(ctCameraInfo camera) {
   cameraInfo = camera;
}

void ctIm3dIntegration::SetSelection(ctVec3 origin, ctVec3 direction) {
   Im3d::AppData& appData = Im3d::GetAppData();
   appData.m_cursorRayOrigin = ctVec3ToIm3d(origin);
   appData.m_cursorRayDirection = ctVec3ToIm3d(direction);
}

ctResults ctIm3dIntegration::NextFrame() {
   ZoneScoped;
   Im3d::AppData& appData = Im3d::GetAppData();

   appData.m_worldUp = CT_UP;
   appData.m_viewOrigin = ctVec3ToIm3d(cameraInfo.position);
   appData.m_viewDirection = ctVec3ToIm3d(cameraInfo.rotation.getForward());
   int32_t ih, iw = 0;
   Engine->WindowManager->GetMainWindowDrawableSize(&iw, &ih);
   appData.m_viewportSize.x = (float)iw;
   appData.m_viewportSize.y = (float)ih;
   appData.m_deltaTime = Engine->FrameTime.GetDeltaTimeFloat();
   Im3d::NewFrame();
   return CT_SUCCESS;
}

void ctIm3dIntegration::DebugUI(bool useGizmos) {
   ImGui::Text("Im3d %s", IM3D_VERSION);
   ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
               Im3d::GetAppData().m_deltaTime * 1000.0f,
               1.0f / Im3d::GetAppData().m_deltaTime);
   ImGui::Separator();
   ImGui::Text("Draw Lists: %u", drawListCount);
   ImGui::Text("Vertices: %u", vertexCount);
   ImGui::Text("Triangles: %u", triangleCount);
   ImGui::Text("Lines: %u", lineCount);
   ImGui::Text("Points: %u", pointCount);
   ImGui::Text("Texts: %u", textCount);
   ImGui::Separator();
   ImGui::Text("Cursor Origin (%f,%f,%f)",
               Im3d::GetAppData().m_cursorRayOrigin.x,
               Im3d::GetAppData().m_cursorRayOrigin.y,
               Im3d::GetAppData().m_cursorRayOrigin.z);
   ImGui::Text("Cursor Direction (%f,%f,%f)",
               Im3d::GetAppData().m_cursorRayDirection.x,
               Im3d::GetAppData().m_cursorRayDirection.y,
               Im3d::GetAppData().m_cursorRayDirection.z);
}
