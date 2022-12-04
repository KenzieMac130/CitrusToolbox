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

#include "KeyLimeRenderer.hpp"

#include "imgui/imgui.h"
#include "gpu/Device.h"
#include "gpu/Present.h"
#include "gpu/Pipeline.h"
#include "gpu/Architect.h"
#include "gpu/Bindless.h"
#include "gpu/Buffer.h"
#include "gpu/Texture.h"
#include "gpu/Commands.h"

#include "core/EngineCore.hpp"
#include "core/Application.hpp"
#include "core/OSEvents.hpp"
#include "core/WindowManager.hpp"
#include "core/FileSystem.hpp"
#include "core/Translation.hpp"
#include "core/AsyncTasks.hpp"

#include "middleware/ImguiIntegration.hpp"
#include "middleware/Im3dIntegration.hpp"

#define CT_KEYLIME_BIND_BUFFER_IMGUI_INDICES  0
#define CT_KEYLIME_BIND_BUFFER_IMGUI_VERTICES 1

#define CT_KEYLIME_BIND_BUFFER_IM3D_VIEW     2
#define CT_KEYLIME_BIND_BUFFER_IM3D_VERTICES 3

#define CT_KEYLIME_BIND_BUFFER_SCENE_OCCLUSION 0 + 8

#define CT_KEYLIME_BIND_TEXTURE_IMGUI_FONT     0
#define CT_KEYLIME_BIND_TEXTURE_SCENE_DEPTH    0 + 8
#define CT_KEYLIME_BIND_TEXTURE_GBUFFER_COLOR  1 + 8
#define CT_KEYLIME_BIND_TEXTURE_GBUFFER_NORMAL 2 + 8
#define CT_KEYLIME_BIND_TEXTURE_GBUFFER_PBR    3 + 8
#define CT_KEYLIME_BIND_TEXTURE_LIGHTING       4 + 8
#define CT_KEYLIME_BIND_TEXTURE_TONEMAP        5 + 8

#define CT_KEYLIME_FORMAT_COLOR  TinyImageFormat_R8G8B8A8_UNORM
#define CT_KEYLIME_FORMAT_DEPTH  TinyImageFormat_D24_UNORM_S8_UINT
#define CT_KEYLIME_FORMAT_NORMAL TinyImageFormat_B10G10R10A2_UNORM
#define CT_KEYLIME_FORMAT_HDR    TinyImageFormat_B10G10R10A2_UNORM

ctGPUArchitectClearContents depthClear = {{0.0f}, 0.0f, 0};
ctGPUArchitectClearContents baseColorClear = {{0.0f, 0.0f, 0.0f, 1.0f}};
ctGPUArchitectClearContents normalRoughClear = {{0.5f, 0.5f, 1.0f, 1.0f}};
ctGPUArchitectClearContents pbrClear = {{0.0f, 0.0f, 0.0f, 0.0f}};
ctGPUArchitectClearContents lightClear = {{0.0f, 0.0f, 0.0f, 1.0f}};
ctGPUArchitectClearContents tonemapClear = {{0.0f, 0.0f, 0.0f, 1.0f}};

/*
----- GBuffer Layout -----
C0: Base Color, Thickness
C1: Normal XY, Roughness, Normal Z Sign
C2: Occlusion, Metal/Translucent, Fresnel F0, Profile ID
C3: Illumination
DS: Reverse Depth, Draw Flag

----- Draw Flag Layout -----
0: Recieves Decals
1: Unused
2: Unused
3: Unused
4: Unused
5: Unused
6: Unused
7: Unused
*/

// clang-format off
ctGPUArchitectBufferPayloadDesc occlusionFeedbackDesc = {CT_KEYLIME_BIND_BUFFER_SCENE_OCCLUSION,
                                                         "Occlusion List",
                                                         CT_GPU_PAYLOAD_FEEDBACK,
                                                         2048};
                                                        
ctGPUArchitectImagePayloadDesc depthDesc =              {CT_KEYLIME_BIND_TEXTURE_SCENE_DEPTH,
                                                         "Scene Depth",
                                                         0,
                                                         1.0f,
                                                         1.0f,
                                                         1,
                                                         1,
                                                         CT_KEYLIME_FORMAT_DEPTH};

ctGPUArchitectImagePayloadDesc gbufferColorDesc =       {CT_KEYLIME_BIND_TEXTURE_GBUFFER_COLOR,
                                                         "GBuffer Color",
                                                         0,
                                                         1.0f,
                                                         1.0f,
                                                         1,
                                                         1,
                                                         CT_KEYLIME_FORMAT_COLOR};

ctGPUArchitectImagePayloadDesc gbufferNormalDesc =      {CT_KEYLIME_BIND_TEXTURE_GBUFFER_NORMAL,
                                                         "GBuffer Normal",
                                                         0,
                                                         1.0f,
                                                         1.0f,
                                                         1,
                                                         1,
                                                         CT_KEYLIME_FORMAT_NORMAL};

ctGPUArchitectImagePayloadDesc gbufferPBRDesc =         {CT_KEYLIME_BIND_TEXTURE_GBUFFER_PBR,
                                                         "GBuffer PBR",
                                                         0,
                                                         1.0f,
                                                         1.0f,
                                                         1,
                                                         1,
                                                         CT_KEYLIME_FORMAT_COLOR};

ctGPUArchitectImagePayloadDesc lightBufferDesc =        {CT_KEYLIME_BIND_TEXTURE_LIGHTING,
                                                         "GBuffer Light",
                                                         0,
                                                         1.0f,
                                                         1.0f,
                                                         1,
                                                         1,
                                                         CT_KEYLIME_FORMAT_HDR};
ctGPUArchitectImagePayloadDesc tonemapBufferDesc =      {CT_KEYLIME_BIND_TEXTURE_TONEMAP,
                                                         "Tonemap Output",
                                                         0,
                                                         1.0f,
                                                         1.0f,
                                                         1,
                                                         1,
                                                         CT_KEYLIME_FORMAT_COLOR };
// clang-format on

void GenerateTestBuffer(uint8_t* dest, size_t size, void* userData) {
   memset(dest, 0, size);
   ctWait(100);
   ctDebugLog("Loaded!");
}

void GenerateTestTexture(uint8_t* dest,
                         ctGPUExternalGenerateContext* ctx,
                         void* userData) {
}

ctResults OpenCacheFileCallback(ctFile* pFileOut,
                                const char* path,
                                ctFileOpenMode fileMode,
                                ctFileSystem* pFileSystem) {
   ctAssert(pFileSystem);
   ctAssert(pFileOut);
   return pFileSystem->OpenPreferencesFile(*pFileOut, path, fileMode);
}

ctResults
OpenAssetFileCallback(ctFile* pFileOut, ctGUID* pGuid, ctFileSystem* pFileSystem) {
   ctAssert(pFileSystem);
   ctAssert(pFileOut);
   return pFileSystem->OpenDataFileByGUID(*pFileOut, *pGuid);
}

ctResults
AsyncSchedulerCallback(ctGPUAsyncWorkFn fpWork, void* data, ctAsyncManager* pAsync) {
   pAsync->CreateTask("Async Render Work", fpWork, data, 0);
   return CT_SUCCESS;
}

ctResults ctKeyLimeRenderer::Startup() {
   ZoneScoped;
#if !CITRUS_HEADLESS
   /* OS Events */
   Engine->OSEventManager->WindowEventHandlers.Append(
     {(ctOSEventCallback)HandleWindowEvent, this});

   /* Device */
   ctGPUDeviceCreateInfo deviceCreateInfo = {};
   deviceCreateInfo.appName = Engine->App->GetAppName();
   deviceCreateInfo.version[0] = Engine->App->GetAppVersion().major;
   deviceCreateInfo.version[1] = Engine->App->GetAppVersion().minor;
   deviceCreateInfo.version[2] = Engine->App->GetAppVersion().patch;
   deviceCreateInfo.pMainWindow = Engine->WindowManager->mainWindow;
   deviceCreateInfo.fpOpenCacheFileCallback = (ctGPUOpenCacheFileFn)OpenCacheFileCallback;
   deviceCreateInfo.pCacheCallbackCustomData = Engine->FileSystem;
   deviceCreateInfo.validationEnabled = true;
   ctGPUDeviceStartup(&pGPUDevice, &deviceCreateInfo, NULL);

   /* Presentation */
   ctGPUPresenterCreateInfo presenterCreateInfo = {};
   presenterCreateInfo.pWindow = Engine->WindowManager->mainWindow;
   presenterCreateInfo.useVSync = true;
   ctGPUPresenterStartup(pGPUDevice, &pGPUPresenter, &presenterCreateInfo);

   /* Bindless */
   ctGPUBindlessManagerCreateInfo bindlessCreateInfo = {};
   bindlessCreateInfo.fixedStorageBufferBindUpperBound = 512;
   bindlessCreateInfo.fixedTextureBindUpperBound = 512;
   ctGPUBindlessManagerStartup(pGPUDevice, &pGPUBindless, &bindlessCreateInfo);

   /* Buffer Pool */
   ctGPUExternalBufferPoolCreateInfo bufferPoolInfo = {};
   bufferPoolInfo.fpAsyncScheduler = (ctGPUAsyncSchedulerFn)AsyncSchedulerCallback;
   bufferPoolInfo.pAsyncUserData = ctGetAsyncManager();
   ctGPUExternalBufferPoolCreate(pGPUDevice, &pGPUBufferPool, &bufferPoolInfo);

   /* Texture Pool */
   ctGPUExternalTexturePoolCreateInfo texturePoolInfo = {};
   texturePoolInfo.fpAsyncScheduler = (ctGPUAsyncSchedulerFn)AsyncSchedulerCallback;
   texturePoolInfo.pAsyncUserData = ctGetAsyncManager();
   ctGPUExternalTexturePoolCreate(pGPUDevice, &pGPUTexturePool, &texturePoolInfo);

   /* Setup Imgui */
   Engine->ImguiIntegration->StartupGPU(pGPUDevice,
                                        pGPUBindless,
                                        pGPUBufferPool,
                                        pGPUTexturePool,
                                        10000,
                                        30000,
                                        CT_KEYLIME_BIND_TEXTURE_IMGUI_FONT,
                                        CT_KEYLIME_BIND_BUFFER_IMGUI_INDICES,
                                        CT_KEYLIME_BIND_BUFFER_IMGUI_VERTICES,
                                        CT_KEYLIME_FORMAT_COLOR,
                                        CT_KEYLIME_FORMAT_DEPTH);
   Engine->Im3dIntegration->StartupGPU(pGPUDevice,
                                       pGPUBindless,
                                       pGPUBufferPool,
                                       30000,
                                       CT_KEYLIME_BIND_BUFFER_IM3D_VIEW,
                                       CT_KEYLIME_BIND_BUFFER_IM3D_VERTICES,
                                       CT_KEYLIME_FORMAT_COLOR,
                                       CT_KEYLIME_FORMAT_DEPTH);

   /* Render Graph */
   ctGPUArchitectCreateInfo architectInfo = {};
   ctGPUArchitectStartup(pGPUDevice, &pGPUArchitect, &architectInfo);

   /* Resource Upload */
   ctGPUArchitectTaskInfo resourceUploadPass = {
     "Upload Resources",
     0,
     CT_GPU_TASK_TRANSFER,
     (ctGPUArchitectTaskDefinitionFn)ResourceUpload.Define,
     (ctGPUArchitectTaskExecutionFn)ResourceUpload.Execute,
     this};
   ctGPUArchitectAddTask(pGPUDevice, pGPUArchitect, &resourceUploadPass);

   /* Occlusion Build */
   ctGPUArchitectTaskInfo occlusionBuildPass = {
     "Build Occlusion",
     0,
     CT_GPU_TASK_COMPUTE,
     (ctGPUArchitectTaskDefinitionFn)Cull.Define,
     NULL,
     this};
   ctGPUArchitectAddTask(pGPUDevice, pGPUArchitect, &occlusionBuildPass);

   /* Depth Prepass */
   ctGPUArchitectTaskInfo depthPrepass = {
     "Depth Prepass",
     0,
     CT_GPU_TASK_RASTER,
     (ctGPUArchitectTaskDefinitionFn)Deferred.DefineDepthPrepass,
     NULL,
     this};
   ctGPUArchitectAddTask(pGPUDevice, pGPUArchitect, &depthPrepass);

   /* GBuffer */
   ctGPUArchitectTaskInfo gbufferPass = {
     "GBuffer Pass",
     0,
     CT_GPU_TASK_RASTER,
     (ctGPUArchitectTaskDefinitionFn)Deferred.DefineGBufferPass,
     NULL,
     this};
   ctGPUArchitectAddTask(pGPUDevice, pGPUArchitect, &gbufferPass);

   /* Lighting */
   ctGPUArchitectTaskInfo lightPass = {
     "Lighting Pass",
     0,
     CT_GPU_TASK_RASTER,
     (ctGPUArchitectTaskDefinitionFn)Deferred.DefineLightPass,
     NULL,
     this};
   ctGPUArchitectAddTask(pGPUDevice, pGPUArchitect, &lightPass);

   /* Tonemapping */
   ctGPUArchitectTaskInfo tonemapPass = {
     "Tonemap Pass",
     0,
     CT_GPU_TASK_RASTER,
     (ctGPUArchitectTaskDefinitionFn)PostProcess.DefineTonemapPass,
     NULL,
     this};
   ctGPUArchitectAddTask(pGPUDevice, pGPUArchitect, &tonemapPass);

   /* Gizmo Main */
   ctGPUArchitectTaskInfo gizmoPassMain = {
     "Gizmo Pass Main",
     0,
     CT_GPU_TASK_RASTER,
     (ctGPUArchitectTaskDefinitionFn)PostProcess.DefineGizmoPassMain,
     Engine->Im3dIntegration->DrawCallbackMain,
     Engine->Im3dIntegration};
   ctGPUArchitectAddTask(pGPUDevice, pGPUArchitect, &gizmoPassMain);

   /* Gizmo XRay */
   ctGPUArchitectTaskInfo gizmoPassXRay = {
     "Gizmo Pass XRay",
     0,
     CT_GPU_TASK_RASTER,
     (ctGPUArchitectTaskDefinitionFn)PostProcess.DefineGizmoPassXray,
     Engine->Im3dIntegration->DrawCallbackXRay,
     Engine->Im3dIntegration};
   ctGPUArchitectAddTask(pGPUDevice, pGPUArchitect, &gizmoPassXRay);

   /* GUI */
   ctGPUArchitectTaskInfo guiPass = {
     "GUI Pass",
     0,
     CT_GPU_TASK_RASTER,
     (ctGPUArchitectTaskDefinitionFn)PostProcess.DefineGUIPass,
     Engine->ImguiIntegration->DrawCallback,
     Engine->ImguiIntegration};
   ctGPUArchitectAddTask(pGPUDevice, pGPUArchitect, &guiPass);

   /* Output */
   ctGPUArchitectSetOutput(pGPUDevice, pGPUArchitect, CT_ARCH_ID("C_Tonemap_Color"), 0);

   /* Show window */
   Engine->WindowManager->ShowMainWindow();
   rebuildRequired = true;
#endif
   return CT_SUCCESS;
}

ctResults ctKeyLimeRenderer::Shutdown() {
   ZoneScoped;
#if !CITRUS_HEADLESS
   ctGPUDeviceWaitForIdle(pGPUDevice);
   Engine->Im3dIntegration->ShutdownGPU(pGPUDevice, pGPUBufferPool);
   Engine->ImguiIntegration->ShutdownGPU(pGPUDevice, pGPUBufferPool, pGPUTexturePool);
   ctGPUExternalTexturePoolDestroy(pGPUDevice, pGPUTexturePool);
   ctGPUExternalBufferPoolDestroy(pGPUDevice, pGPUBufferPool);
   ctGPUArchitectShutdown(pGPUDevice, pGPUArchitect);
   ctGPUBindlessManagerShutdown(pGPUDevice, pGPUBindless);
   ctGPUPresenterShutdown(pGPUDevice, pGPUPresenter);
   ctGPUDeviceShutdown(pGPUDevice);
#endif
   return CT_SUCCESS;
}

const char* ctKeyLimeRenderer::GetModuleName() {
   return "Renderer";
}

ctResults ctKeyLimeRenderer::RenderFrame() {
   ZoneScoped;
#if !CITRUS_HEADLESS

   /* Handle Presentation State */
   uint32_t width;
   uint32_t height;
   ctGPUPresenterState presenterState =
     ctGPUPresenterHandleState(pGPUDevice, pGPUPresenter, &width, &height);

   Engine->Im3dIntegration->SetCamera(mainCamera);
   Engine->Im3dIntegration->PrepareFrameGPU(pGPUDevice, pGPUBufferPool);
   Engine->Im3dIntegration->DrawImguiText();
   Engine->ImguiIntegration->PrepareFrameGPU(pGPUDevice, pGPUBufferPool);

   if (presenterState == CT_GPU_PRESENTER_INVALID) {
      return CT_FAILURE_SKIPPED;
   } else if (presenterState == CT_GPU_PRESENTER_RESIZED) {
      rebuildRequired = true;
   }

   /* Check for new Uploads */
   if (ResourceUpload.needsUpload) /* no-longer needs upload */ {
      ResourceUpload.needsUpload = false;
      rebuildRequired = true;
   }
   if (ctGPUExternalTexturePoolNeedsDispatch(pGPUDevice, pGPUTexturePool) ||
       ctGPUExternalBufferPoolNeedsDispatch(pGPUDevice, pGPUBufferPool)) {
      ResourceUpload.needsUpload = true;
      rebuildRequired = true;
   }

   /* Rebuild Rendergraph as necessary */
   if (rebuildRequired) {
      ctGPUArchitectBuildInfo archBuildInfo = {};
      archBuildInfo.width = width;
      archBuildInfo.height = height;
      ctGPUArchitectBuild(pGPUDevice, pGPUArchitect, &archBuildInfo);
      rebuildRequired = false;
      ctDebugLog("Rebuilt Rendergraph!");
   }
   ctGPUBindlessManagerPrepareFrame(pGPUDevice,
                                    pGPUBindless,
                                    1,
                                    &pGPUBufferPool,
                                    1,
                                    &pGPUTexturePool,
                                    1,
                                    &pGPUArchitect,
                                    NULL);
   ctGPUArchitectExecute(pGPUDevice, pGPUArchitect, pGPUBindless);
   ctGPUPresenterExecute(pGPUDevice, pGPUPresenter, pGPUArchitect);
#endif
   return CT_SUCCESS;
}

ctResults ctKeyLimeRenderer::UpdateCamera(const ctCameraInfo& cameraInfo) {
   mainCamera = cameraInfo;
   return CT_SUCCESS;
}

void ctKeyLimeRenderer::HandleWindowEvent(SDL_Event* event,
                                          ctKeyLimeRenderer* pRenderer) {
   if (event->type == SDL_WINDOWEVENT) {
      if (event->window.event == SDL_WINDOWEVENT_RESIZED) {
         ctGPUPresenterSignalStateChange(
           pRenderer->pGPUDevice, pRenderer->pGPUPresenter, CT_GPU_PRESENTER_RESIZED);
      }
   }
}

ctResults
ctKeyLimeRenderer::ResourceUploadS::Define(struct ctGPUArchitectDefinitionContext* pCtx,
                                           ctKeyLimeRenderer* pRenderer) {
   if (!pRenderer->ResourceUpload.needsUpload) { return CT_FAILURE_SKIPPED; }
   // clang-format off
   ctGPUTaskCreateBarrier(pCtx, CT_ARCH_ID("B_ResourcesUploaded"), "Resources Uploaded", 0);
   ctGPUTaskSignalBarrier(pCtx, CT_ARCH_ID("B_ResourcesUploaded"));
   // clang-format on
   return CT_SUCCESS;
}

ctResults
ctKeyLimeRenderer::ResourceUploadS::Execute(struct ctGPUArchitectExecutionContext* pCtx,
                                            ctKeyLimeRenderer* pRenderer) {
   if (ctGPUExternalBufferPoolNeedsDispatch(pCtx->pDevice, pRenderer->pGPUBufferPool)) {
      ctGPUExternalBufferPoolDispatch(
        pCtx->pDevice, pRenderer->pGPUBufferPool, pCtx->cmd);
   }
   if (ctGPUExternalTexturePoolNeedsDispatch(pCtx->pDevice, pRenderer->pGPUTexturePool)) {
      ctGPUExternalTexturePoolDispatch(
        pCtx->pDevice, pRenderer->pGPUTexturePool, pCtx->cmd);
   }
   return CT_SUCCESS;
}

ctResults ctKeyLimeRenderer::CullS::Define(ctGPUArchitectDefinitionContext* pCtx,
                                           ctKeyLimeRenderer* pRenderer) {
   // clang-format off
    ctGPUTaskCreateBarrier(pCtx, CT_ARCH_ID("B_OcclusionBuilt"), "Occlusion Built", 0);
    ctGPUTaskUseStorageBuffer(pCtx, CT_ARCH_ID("S_OcclusionList"), CT_GPU_ACCESS_WRITE);
    if (pRenderer->ResourceUpload.needsUpload) {
        ctGPUTaskWaitBarrier(pCtx, CT_ARCH_ID("B_ResourcesUploaded"));
    }
    ctGPUTaskSignalBarrier(pCtx, CT_ARCH_ID("B_OcclusionBuilt"));
   // clang-format on
   return CT_SUCCESS;
}

ctResults
ctKeyLimeRenderer::DeferredS::DefineDepthPrepass(ctGPUArchitectDefinitionContext* pCtx,
                                                 ctKeyLimeRenderer* pRenderer) {
   // clang-format off
    ctGPUTaskDeclareDepthTarget(pCtx, CT_ARCH_ID("D_SceneDepth"), &depthDesc);
    ctGPUTaskDeclareStorageBuffer(pCtx, CT_ARCH_ID("S_OcclusionList"), &occlusionFeedbackDesc);

    ctGPUTaskUseStorageBuffer(pCtx, CT_ARCH_ID("S_OcclusionList"), CT_GPU_ACCESS_READ);
    ctGPUTaskUseDepthTarget(pCtx, CT_ARCH_ID("D_SceneDepth"), CT_GPU_ACCESS_WRITE, &depthClear);
    ctGPUTaskWaitBarrier(pCtx, CT_ARCH_ID("B_OcclusionBuilt"));
   // clang-format on
   return CT_SUCCESS;
}

ctResults
ctKeyLimeRenderer::DeferredS::DefineGBufferPass(ctGPUArchitectDefinitionContext* pCtx,
                                                ctKeyLimeRenderer* pRenderer) {
   // clang-format off
    ctGPUTaskDeclareColorTarget(pCtx, CT_ARCH_ID("C_GBuffer_Color"), &gbufferColorDesc);
    ctGPUTaskDeclareColorTarget(pCtx, CT_ARCH_ID("C_GBuffer_Normal"), &gbufferNormalDesc);
    ctGPUTaskDeclareColorTarget(pCtx, CT_ARCH_ID("C_GBuffer_PBR"), &gbufferPBRDesc);
    ctGPUTaskDeclareColorTarget(pCtx, CT_ARCH_ID("C_LightBuffer"), &lightBufferDesc);

    ctGPUTaskUseDepthTarget(pCtx, CT_ARCH_ID("D_SceneDepth"), CT_GPU_ACCESS_READ, NULL);
    ctGPUTaskUseColorTarget(pCtx, CT_ARCH_ID("C_GBuffer_Color"), CT_GPU_ACCESS_WRITE, 0, &baseColorClear);
    ctGPUTaskUseColorTarget(pCtx, CT_ARCH_ID("C_GBuffer_Normal"), CT_GPU_ACCESS_WRITE, 1, &normalRoughClear);
    ctGPUTaskUseColorTarget(pCtx, CT_ARCH_ID("C_GBuffer_PBR"), CT_GPU_ACCESS_WRITE, 2, &pbrClear);
    ctGPUTaskUseColorTarget(pCtx, CT_ARCH_ID("C_LightBuffer"), CT_GPU_ACCESS_WRITE, 3, &lightClear);

    ctGPUTaskUseStorageBuffer(pCtx, CT_ARCH_ID("S_OcclusionList"), CT_GPU_ACCESS_READ);
   // clang-format on
   return CT_SUCCESS;
}

ctResults
ctKeyLimeRenderer::DeferredS::DefineLightPass(ctGPUArchitectDefinitionContext* pCtx,
                                              ctKeyLimeRenderer* pRenderer) {
   // clang-format off
    ctGPUTaskUseTexture(pCtx, CT_ARCH_ID("D_SceneDepth"));
    ctGPUTaskUseTexture(pCtx, CT_ARCH_ID("C_GBuffer_Color"));
    ctGPUTaskUseTexture(pCtx, CT_ARCH_ID("C_GBuffer_Normal"));
    ctGPUTaskUseTexture(pCtx, CT_ARCH_ID("C_GBuffer_PBR"));
    ctGPUTaskUseColorTarget(pCtx, CT_ARCH_ID("C_LightBuffer"), CT_GPU_ACCESS_READ_WRITE, 0, NULL);

    ctGPUTaskCreateBarrier(pCtx, CT_ARCH_ID("B_LightFinished"), "Light Finished", 0);
    ctGPUTaskSignalBarrier(pCtx, CT_ARCH_ID("B_LightFinished"));
   // clang-format on
   return CT_SUCCESS;
}

ctResults
ctKeyLimeRenderer::PostProcessS::DefineTonemapPass(ctGPUArchitectDefinitionContext* pCtx,
                                                   ctKeyLimeRenderer* pRenderer) {
   // clang-format off
    ctGPUTaskDeclareColorTarget(pCtx, CT_ARCH_ID("C_Tonemap_Color"), &tonemapBufferDesc);

    ctGPUTaskUseTexture(pCtx, CT_ARCH_ID("C_LightBuffer"));
    ctGPUTaskUseColorTarget(pCtx, CT_ARCH_ID("C_Tonemap_Color"), CT_GPU_ACCESS_WRITE, 0, &tonemapClear);

    ctGPUTaskWaitBarrier(pCtx, CT_ARCH_ID("B_LightFinished"));
   // clang-format on
   return CT_SUCCESS;
}

ctResults ctKeyLimeRenderer::PostProcessS::DefineGizmoPassMain(
  ctGPUArchitectDefinitionContext* pCtx, ctKeyLimeRenderer* pRenderer) {
   // clang-format off
    ctGPUTaskUseDepthTarget(pCtx, CT_ARCH_ID("D_SceneDepth"), CT_GPU_ACCESS_READ_WRITE, NULL);
    ctGPUTaskUseColorTarget(pCtx, CT_ARCH_ID("C_Tonemap_Color"), CT_GPU_ACCESS_READ_WRITE, 0, NULL);

    ctGPUTaskCreateBarrier(pCtx, CT_ARCH_ID("B_GizmoDrawnMain"), "Gizmo Drawn Main", 0);
    ctGPUTaskSignalBarrier(pCtx, CT_ARCH_ID("B_GizmoDrawnMain"));
   // clang-format on
   return CT_SUCCESS;
}

ctResults ctKeyLimeRenderer::PostProcessS::DefineGizmoPassXray(
  ctGPUArchitectDefinitionContext* pCtx, ctKeyLimeRenderer* pRenderer) {
   // clang-format off
    ctGPUTaskUseDepthTarget(pCtx, CT_ARCH_ID("D_SceneDepth"), CT_GPU_ACCESS_WRITE, &depthClear);
    ctGPUTaskUseColorTarget(pCtx, CT_ARCH_ID("C_Tonemap_Color"), CT_GPU_ACCESS_READ_WRITE, 0, NULL);

    ctGPUTaskCreateBarrier(pCtx, CT_ARCH_ID("B_GizmoDrawnXRay"), "Gizmo Drawn XRay", 0);
    ctGPUTaskSignalBarrier(pCtx, CT_ARCH_ID("B_GizmoDrawnXRay"));
    ctGPUTaskWaitBarrier(pCtx, CT_ARCH_ID("B_GizmoDrawnMain"));
   // clang-format on
   return CT_SUCCESS;
}

ctResults
ctKeyLimeRenderer::PostProcessS::DefineGUIPass(ctGPUArchitectDefinitionContext* pCtx,
                                               ctKeyLimeRenderer* pRenderer) {
   // clang-format off
    ctGPUTaskUseDepthTarget(pCtx, CT_ARCH_ID("D_SceneDepth"), CT_GPU_ACCESS_WRITE, &depthClear);
    ctGPUTaskUseColorTarget(pCtx, CT_ARCH_ID("C_Tonemap_Color"), CT_GPU_ACCESS_READ_WRITE, 0, NULL);
    ctGPUTaskWaitBarrier(pCtx, CT_ARCH_ID("B_GizmoDrawnXRay"));
   // clang-format on
   return CT_SUCCESS;
}