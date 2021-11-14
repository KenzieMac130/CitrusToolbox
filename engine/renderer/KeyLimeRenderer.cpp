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

#include "KeyLimeRenderer.hpp"

#include "imgui/imgui.h"
#include "gpu/Device.h"
#include "gpu/Pipeline.h"
#include "gpu/Architect.h"

#include "core/EngineCore.hpp"
#include "core/Application.hpp"
#include "core/WindowManager.hpp"
#include "core/FileSystem.hpp"

#define CT_GPU_BIND_BUFFER_SCENE_OCCLUSION 0

#define CT_GPU_BIND_TEXTURE_SCENE_DEPTH    0
#define CT_GPU_BIND_TEXTURE_GBUFFER_COLOR  1
#define CT_GPU_BIND_TEXTURE_GBUFFER_NORMAL 2
#define CT_GPU_BIND_TEXTURE_GBUFFER_PBR    3
#define CT_GPU_BIND_TEXTURE_LIGHTING       4

ctGPUArchitectBufferPayloadDesc occlusionFeedbackDesc = {
  CT_GPU_BIND_BUFFER_SCENE_OCCLUSION, "Occlusion List", CT_GPU_PAYLOAD_FEEDBACK, 2048};

ctGPUArchitectImagePayloadDesc depthDesc = {CT_GPU_BIND_TEXTURE_SCENE_DEPTH,
                                            "Scene Depth",
                                            0,
                                            1.0f,
                                            1.0f,
                                            1,
                                            1,
                                            TinyImageFormat_D24_UNORM_S8_UINT};
ctGPUArchitectImagePayloadDesc gbufferColorDesc = {CT_GPU_BIND_TEXTURE_GBUFFER_COLOR,
                                                   "GBuffer Color",
                                                   0,
                                                   1.0f,
                                                   1.0f,
                                                   1,
                                                   1,
                                                   TinyImageFormat_R8G8B8A8_UNORM};
ctGPUArchitectImagePayloadDesc gbufferNormalDesc = {CT_GPU_BIND_TEXTURE_GBUFFER_NORMAL,
                                                    "GBuffer Normal",
                                                    0,
                                                    1.0f,
                                                    1.0f,
                                                    1,
                                                    1,
                                                    TinyImageFormat_B10G10R10A2_UNORM};
ctGPUArchitectImagePayloadDesc gbufferPBRDesc = {CT_GPU_BIND_TEXTURE_GBUFFER_PBR,
                                                 "GBuffer PBR",
                                                 0,
                                                 1.0f,
                                                 1.0f,
                                                 1,
                                                 1,
                                                 TinyImageFormat_R8G8B8A8_UNORM};
ctGPUArchitectImagePayloadDesc lightBufferDesc = {CT_GPU_BIND_TEXTURE_LIGHTING,
                                                  "GBuffer Light",
                                                  0,
                                                  1.0f,
                                                  1.0f,
                                                  1,
                                                  1,
                                                  TinyImageFormat_B10G10R10A2_UNORM};

// clang-format off
ctResults DefineDepthPrepass(ctGPUArchitectDefinitionContext* pCtx, void* pUserData) {
   ctGPUTaskDeclareDepthTarget(pCtx, CT_ARCH_ID("D_SceneDepth"), &depthDesc);
   ctGPUTaskDeclareStorageBuffer(pCtx, CT_ARCH_ID("S_OcclusionList"), &occlusionFeedbackDesc);

   ctGPUTaskUseStorageBuffer(pCtx, CT_ARCH_ID("S_OcclusionList"), CT_GPU_ACCESS_READ);
   ctGPUTaskUseDepthTarget(pCtx, CT_ARCH_ID("D_SceneDepth"), CT_GPU_ACCESS_WRITE);
   ctGPUTaskWaitBarrier(pCtx, CT_ARCH_ID("B_OcclusionBuilt"));
   return CT_SUCCESS;
}
ctResults DefineGBufferPass(ctGPUArchitectDefinitionContext* pCtx, void* pUserData) {
   ctGPUTaskDeclareColorTarget(pCtx, CT_ARCH_ID("C_GBuffer_Color"), &gbufferColorDesc);
   ctGPUTaskDeclareColorTarget(pCtx, CT_ARCH_ID("C_GBuffer_Normal"), &gbufferNormalDesc);
   ctGPUTaskDeclareColorTarget(pCtx, CT_ARCH_ID("C_GBuffer_PBR"), &gbufferPBRDesc);
   ctGPUTaskDeclareColorTarget(pCtx, CT_ARCH_ID("C_LightBuffer"), &lightBufferDesc);

   ctGPUTaskUseDepthTarget(pCtx, CT_ARCH_ID("D_SceneDepth"), CT_GPU_ACCESS_READ);
   ctGPUTaskUseColorTarget(pCtx, CT_ARCH_ID("C_GBuffer_Color"), CT_GPU_ACCESS_WRITE, 0);
   ctGPUTaskUseColorTarget(pCtx, CT_ARCH_ID("C_GBuffer_Normal"), CT_GPU_ACCESS_WRITE, 1);
   ctGPUTaskUseColorTarget(pCtx, CT_ARCH_ID("C_GBuffer_PBR"), CT_GPU_ACCESS_WRITE, 2);
   ctGPUTaskUseColorTarget(pCtx, CT_ARCH_ID("C_LightBuffer"), CT_GPU_ACCESS_WRITE, 3);

   ctGPUTaskUseStorageBuffer(pCtx, CT_ARCH_ID("S_OcclusionList"), CT_GPU_ACCESS_READ);
   return CT_SUCCESS;
}
ctResults DefineLightPass(ctGPUArchitectDefinitionContext* pCtx, void* pUserData) {
   ctGPUTaskUseTexture(pCtx, CT_ARCH_ID("D_SceneDepth"));
   ctGPUTaskUseTexture(pCtx, CT_ARCH_ID("C_GBuffer_Color"));
   ctGPUTaskUseTexture(pCtx, CT_ARCH_ID("C_GBuffer_Normal"));
   ctGPUTaskUseTexture(pCtx, CT_ARCH_ID("C_GBuffer_PBR"));
   ctGPUTaskUseColorTarget(pCtx, CT_ARCH_ID("C_LightBuffer"), CT_GPU_ACCESS_READ_WRITE, 0);
   return CT_SUCCESS;
}
ctResults DefineObjectOcclusionPass(ctGPUArchitectDefinitionContext* pCtx,
                                    void* pUserData) {
   ctGPUTaskCreateBarrier(pCtx, CT_ARCH_ID("B_OcclusionBuilt"), "Occlusion Built", 0);
   ctGPUTaskUseStorageBuffer(pCtx, CT_ARCH_ID("S_OcclusionList"), CT_GPU_ACCESS_WRITE);
   ctGPUTaskSignalBarrier(pCtx, CT_ARCH_ID("B_OcclusionBuilt"));
   return CT_SUCCESS;
}
// clang-format on

ctResults OpenCacheFileCallback(ctFile* pFileOut,
                                const char* path,
                                ctFileOpenMode fileMode,
                                ctFileSystem* pFileSystem) {
   ctAssert(pFileSystem);
   ctAssert(pFileOut);
   return pFileSystem->OpenPreferencesFile(*pFileOut, path, fileMode);
}

ctResults ctKeyLimeRenderer::Startup() {
#if !CITRUS_HEADLESS
   ImGui::GetIO().Fonts->AddFontDefault();
   ImGui::GetIO().Fonts->Build();
   ImGui::GetIO().DisplaySize.x = 640.0f;
   ImGui::GetIO().DisplaySize.y = 480.0f;
   ImGui::GetIO().DeltaTime = 1.0f / 60.0f;
   ImGui::NewFrame();
   ctGPUDeviceCreateInfo deviceCreateInfo = {};
   deviceCreateInfo.appName = Engine->App->GetAppName();
   deviceCreateInfo.version[0] = Engine->App->GetAppVersion().major;
   deviceCreateInfo.version[1] = Engine->App->GetAppVersion().minor;
   deviceCreateInfo.version[2] = Engine->App->GetAppVersion().patch;
   deviceCreateInfo.pMainWindow = Engine->WindowManager->mainWindow;
   deviceCreateInfo.fpOpenCacheFileCallback = (ctGPUOpenCacheFileFn)OpenCacheFileCallback;
   deviceCreateInfo.pCacheCallbackCustomData = Engine->FileSystem;
   deviceCreateInfo.useVSync = false;
   deviceCreateInfo.validationEnabled = true;
   ctGPUDeviceStartup(&pGPUDevice, &deviceCreateInfo, NULL);
   Engine->WindowManager->ShowMainWindow();

   /* Temporary Test Render Graph */
   ctGPUArchitectCreateInfo architectInfo = {};
   ctGPUArchitectStartup(pGPUDevice, &pGPUArchitect, &architectInfo);

   ctGPUArchitectTaskInfo lightPass = {
     "Lighting Pass", CT_GPU_TASK_RASTER, DefineLightPass, NULL, NULL};
   ctGPUArchitectAddTask(pGPUDevice, pGPUArchitect, &lightPass);
   ctGPUArchitectTaskInfo depthPrepass = {
     "Depth Prepass", CT_GPU_TASK_RASTER, DefineDepthPrepass, NULL, NULL};
   ctGPUArchitectAddTask(pGPUDevice, pGPUArchitect, &depthPrepass);
   ctGPUArchitectTaskInfo gbufferPass = {
     "GBuffer Pass", CT_GPU_TASK_RASTER, DefineGBufferPass, NULL, NULL};
   ctGPUArchitectAddTask(pGPUDevice, pGPUArchitect, &gbufferPass);
   ctGPUArchitectTaskInfo occlusionBuildPass = {
     "Build Occlusion", CT_GPU_TASK_COMPUTE, DefineObjectOcclusionPass, NULL, NULL};
   ctGPUArchitectAddTask(pGPUDevice, pGPUArchitect, &occlusionBuildPass);
   ctGPUArchitectSetOutput(pGPUDevice, pGPUArchitect, CT_ARCH_ID("C_LightBuffer"), 0);

   /* Build and Display Outputs */
   ctGPUArchitectBuild(pGPUDevice, pGPUArchitect);
   ctStringUtf8 dotPath = Engine->FileSystem->GetPreferencesPath();
   dotPath.FilePathAppend("RENDERGRAPH_DUMP");
   ctGPUArchitectDumpGraphVis(pGPUArchitect, dotPath.CStr(), true, true);
#endif
   return CT_SUCCESS;
}

ctResults ctKeyLimeRenderer::Shutdown() {
#if !CITRUS_HEADLESS
   ctGPUArchitectShutdown(pGPUDevice, pGPUArchitect);
   ctGPUDeviceShutdown(pGPUDevice);
#endif
   return CT_SUCCESS;
}

ctResults ctKeyLimeRenderer::RenderFrame() {
#if !CITRUS_HEADLESS
   ImGui::Render();
#endif
   ctGPUArchitectExecute(pGPUDevice, pGPUArchitect);
   return CT_SUCCESS;
}

ctResults ctKeyLimeRenderer::UpdateCamera(const ctCameraInfo& cameraInfo) {
   return CT_SUCCESS;
}