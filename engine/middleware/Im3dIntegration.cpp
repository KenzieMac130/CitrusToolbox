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
#include "scene/SceneEngineBase.hpp"
#include "core/WindowManager.hpp"
#include "core/Translation.hpp"
#include "core/FileSystem.hpp"
#include "im3d/im3d_math.h"
#include "imgui/imgui.h"

#include "gpu/Device.h"
#include "gpu/Buffer.h"
#include "gpu/Pipeline.h"

ctResults ctIm3dIntegration::Startup() {
   return CT_SUCCESS;
}

ctResults ctIm3dIntegration::Shutdown() {
   return CT_SUCCESS;
}

ctResults ctIm3dIntegration::StartupGPU(struct ctGPUDevice* pGPUDevice,
                                        struct ctGPUBindlessManager* pBindless,
                                        struct ctGPUExternalBufferPool* pGPUBufferPool,
                                        size_t maxVerts,
                                        int32_t viewBind,
                                        int32_t vtxBind,
                                        enum TinyImageFormat colorFormat,
                                        enum TinyImageFormat depthFormat) {
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
   ctGPUPipelineBuilderSetFaceCull(pPipelineBuilder, CT_GPU_FACE_NONE);

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
      ctGPUPipelineBuilderSetTopology(pPipelineBuilder, fillToTopo[i], false);
      ctGPUPipelineBuilderClearShaders(pPipelineBuilder);
      ctGPUPipelineBuilderAddShader(pPipelineBuilder, CT_GPU_SHADER_VERT, vertShader);
      ctGPUPipelineBuilderAddShader(pPipelineBuilder, CT_GPU_SHADER_FRAG, fragShader);
      ctGPUPipelineCreate(pGPUDevice, pPipelineBuilder, &pPipelines[i], pBindless);

      ctGPUShaderSoftRelease(pGPUDevice, fragShader);
      ctGPUShaderSoftRelease(pGPUDevice, vertShader);
   }

   ctGPUPipelineBuilderDelete(pPipelineBuilder);
   return CT_SUCCESS;
}

ctResults ctIm3dIntegration::ShutdownGPU(ctGPUDevice* pGPUDevice,
                                         ctGPUExternalBufferPool* pGPUBufferPool) {
   for (int i = 0; i < Im3d::DrawPrimitive_Count; i++) {
      ctGPUPipelineDestroy(pGPUDevice, pPipelines[i]);
   }
   return CT_SUCCESS;
}

void ctIm3dIntegration::DrawImguiText(ctMat4 viewProj, ctCameraInfo cameraInfo) {
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
   Im3d::Mat4 camView = Im3d::Inverse(Im3d::LookAt(
     appData.m_viewOrigin, (appData.m_viewOrigin + appData.m_viewDirection)));
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

ctResults ctIm3dIntegration::NextFrame() {
   ZoneScoped;
   if (!Engine->SceneEngine || !Engine->SceneEngine->isStarted()) {
      Im3d::NewFrame();
      return CT_FAILURE_MODULE_NOT_INITIALIZED;
   }
   const ctCameraInfo cameraInfo = Engine->SceneEngine->GetCameraInfo(NULL);
   Im3d::AppData& appData = Im3d::GetAppData();
   appData.m_cursorRayOrigin = ctVec3ToIm3d(cameraInfo.cursorPosition);
   appData.m_cursorRayDirection = ctVec3ToIm3d(cameraInfo.cursorDirection);
   appData.m_worldUp = CT_UP;
   appData.m_viewOrigin = ctVec3ToIm3d(cameraInfo.position);
   appData.m_viewDirection = ctVec3ToIm3d(cameraInfo.rotation.getForward());
   int32_t ih, iw = 0;
   Engine->WindowManager->GetMainWindowDrawableSize(&iw, &ih);
   appData.m_viewportSize.x = (float)iw;
   appData.m_viewportSize.y = (float)ih;
   /* Todo: Feed me seymour */
   Im3d::NewFrame();
   return CT_SUCCESS;
}
