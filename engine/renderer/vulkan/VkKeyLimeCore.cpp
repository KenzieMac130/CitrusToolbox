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

#include "VkKeyLimeCore.hpp"
#include "core/EngineCore.hpp"

void sendResizeSignal(SDL_Event* event, void* pData) {
   ctVkKeyLimeCore* pCore = (ctVkKeyLimeCore*)pData;
   if (SDL_GetWindowFromID(event->window.windowID) ==
       pCore->vkBackend.mainScreenResources.window) {
      if (event->window.event == SDL_WINDOWEVENT_RESIZED) {
         ctDebugLog("Window Resize: %d:%dx%d",
                    event->window.windowID,
                    event->window.data1,
                    event->window.data2);
         pCore->vkBackend.mainScreenResources.resizeTriggered = true;
      }
   }
}

ctResults ctVkKeyLimeCore::Startup() {
   ZoneScoped;
   vkBackend.preferredDevice = -1;
   vkBackend.vsync = Engine->WindowManager->mainWindowVSync;
   vkBackend.nextFrameTimeout = INT32_MAX;
#ifndef NDEBUG
   vkBackend.validationEnabled = 1;
#else
   vkBackend.validationEnabled = 0;
#endif
   vkBackend.maxSamplers = CT_MAX_GFX_SAMPLERS;
   vkBackend.maxSampledImages = CT_MAX_GFX_SAMPLED_IMAGES;
   vkBackend.maxStorageImages = CT_MAX_GFX_STORAGE_IMAGES;
   vkBackend.maxStorageBuffers = CT_MAX_GFX_STORAGE_BUFFERS;

   int w, h;
   Engine->WindowManager->GetMainWindowDrawableSize(&w, &h);
   internalResolutionWidth = w;
   internalResolutionHeight = h;

   ctSettingsSection* vkSettings = Engine->Settings->CreateSection("VulkanBackend", 32);
   vkSettings->BindInteger(&vkBackend.preferredDevice,
                           false,
                           true,
                           "PreferredDevice",
                           "Index of the preferred device to use for rendering.");
   vkSettings->BindInteger(&vkBackend.validationEnabled,
                           false,
                           true,
                           "ValidationEnabled",
                           "Use validation layers for debug testing.",
                           CT_SETTINGS_BOUNDS_BOOL);

   vkSettings->BindInteger(&vkBackend.maxSamplers,
                           false,
                           true,
                           "MaxSamplers",
                           "Max number of samplers.",
                           CT_SETTINGS_BOUNDS_UINT);
   vkSettings->BindInteger(&vkBackend.maxSampledImages,
                           false,
                           true,
                           "MaxSampledImages",
                           "Max number of sampled images.",
                           CT_SETTINGS_BOUNDS_UINT);
   vkSettings->BindInteger(&vkBackend.maxStorageImages,
                           false,
                           true,
                           "MaxStorageImages",
                           "Max number of storage images.",
                           CT_SETTINGS_BOUNDS_UINT);
   vkSettings->BindInteger(&vkBackend.maxStorageBuffers,
                           false,
                           true,
                           "MaxStorageBuffers",
                           "Max number of storage buffers.",
                           CT_SETTINGS_BOUNDS_UINT);
   vkSettings->BindInteger(&vkBackend.nextFrameTimeout,
                           false,
                           true,
                           "NextFrameTimeout",
                           "Amount of time until timeout.",
                           CT_SETTINGS_BOUNDS_BOOL);

   ctSettingsSection* settings = Engine->Settings->CreateSection("KeyLimeRenderer", 32);
   Engine->OSEventManager->WindowEventHandlers.Append({sendResizeSignal, this});

#if CITRUS_INCLUDE_AUDITION
   ShaderHotReload.RegisterPath("core/shaders/im3d_vert.spv");
   ShaderHotReload.RegisterPath("core/shaders/im3d_frag.spv");
   Engine->HotReload->RegisterAssetCategory(&ShaderHotReload);
#endif

   vkBackend.ModuleStartup(Engine);
   /* Commands and Sync */
   {
      /* Graphics Commands */
      {
         VkCommandPoolCreateInfo poolInfo {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
         poolInfo.queueFamilyIndex = vkBackend.queueFamilyIndices.graphicsIdx;
         poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
         vkCreateCommandPool(
           vkBackend.vkDevice, &poolInfo, &vkBackend.vkAllocCallback, &gfxCommandPool);
         VkCommandBufferAllocateInfo allocInfo {
           VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
         allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
         allocInfo.commandBufferCount = CT_MAX_INFLIGHT_FRAMES;
         allocInfo.commandPool = gfxCommandPool;
         vkAllocateCommandBuffers(vkBackend.vkDevice, &allocInfo, gfxCommandBuffers);
      }
      /* Transfer Commands */
      {
         VkCommandPoolCreateInfo poolInfo {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
         poolInfo.queueFamilyIndex = vkBackend.queueFamilyIndices.transferIdx;
         poolInfo.flags = 0;
         vkCreateCommandPool(vkBackend.vkDevice,
                             &poolInfo,
                             &vkBackend.vkAllocCallback,
                             &transferCommandPool);
         VkCommandBufferAllocateInfo allocInfo {
           VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
         allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
         allocInfo.commandBufferCount = CT_MAX_INFLIGHT_FRAMES;
         allocInfo.commandPool = transferCommandPool;
         vkAllocateCommandBuffers(
           vkBackend.vkDevice, &allocInfo, frameInitialUploadCommandBuffers);
      }
   }
   CreateScreenResources();
   /* Setup ImGUI */
   {
      VkCommandBuffer startupTransferCommands = frameInitialUploadCommandBuffers[0];
      VkCommandBufferBeginInfo beginInfo {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
      beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
      vkBeginCommandBuffer(startupTransferCommands, &beginInfo);
      vkImgui.SetDisplaySize(vkBackend.mainScreenResources.extent.width,
                             vkBackend.mainScreenResources.extent.height,
                             internalResolutionWidth,
                             internalResolutionHeight);
      vkImgui.Startup(
        &vkBackend, frameInitialUploadCommandBuffers[0], forwardRenderPass, 0);
      vkEndCommandBuffer(startupTransferCommands);
      VkSubmitInfo submitInfo {VK_STRUCTURE_TYPE_SUBMIT_INFO};
      submitInfo.commandBufferCount = 1;
      submitInfo.pCommandBuffers = &startupTransferCommands;
      vkQueueSubmit(vkBackend.transferQueue, 1, &submitInfo, VK_NULL_HANDLE);
      vkQueueWaitIdle(vkBackend.transferQueue);
   }
   /* Setup Im3d */
   {
      /*vkIm3d.SetDisplaySize(vkBackend.mainScreenResources.extent.width,
                            vkBackend.mainScreenResources.extent.height,
                            internalResolutionWidth,
                            internalResolutionHeight);*/
      vkIm3d.Startup(&vkBackend, forwardRenderPass, 0);
   }
   return CT_SUCCESS;
}

ctResults ctVkKeyLimeCore::Shutdown() {
   ZoneScoped;
   vkDeviceWaitIdle(vkBackend.vkDevice);
   vkDestroyCommandPool(vkBackend.vkDevice, gfxCommandPool, &vkBackend.vkAllocCallback);
   vkDestroyCommandPool(
     vkBackend.vkDevice, transferCommandPool, &vkBackend.vkAllocCallback);
   vkIm3d.Shutdown();
   vkImgui.Shutdown();
   DestroyScreenResources();
   vkBackend.ModuleShutdown();
   return CT_SUCCESS;
}

ctResults ctVkKeyLimeCore::CreateScreenResources() {
   /* GUI Renderpass and Framebuffer */
   {
      compositeFormat = VK_FORMAT_R8G8B8A8_UNORM;
      depthFormat = VK_FORMAT_D32_SFLOAT;
      VkAttachmentDescription attachments[2] {};
      attachments[0].format = compositeFormat;
      attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
      attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
      attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;

      attachments[1].format = depthFormat;
      attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
      attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
      attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
      attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;

      VkAttachmentReference colorAttachmentRef {};
      colorAttachmentRef.attachment = 0;
      colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      VkAttachmentReference depthAttachmentRef {};
      depthAttachmentRef.attachment = 1;
      depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

      VkSubpassDescription subpasses[1] {};
      subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
      subpasses[0].pColorAttachments = &colorAttachmentRef;
      subpasses[0].colorAttachmentCount = 1;
      subpasses[0].pDepthStencilAttachment = &depthAttachmentRef;

      VkRenderPassCreateInfo createInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
      createInfo.attachmentCount = ctCStaticArrayLen(attachments);
      createInfo.pAttachments = attachments;
      createInfo.subpassCount = ctCStaticArrayLen(subpasses);
      createInfo.pSubpasses = subpasses;
      CT_VK_CHECK(vkCreateRenderPass(vkBackend.vkDevice,
                                     &createInfo,
                                     &vkBackend.vkAllocCallback,
                                     &forwardRenderPass),
                  CT_NC("vkCreateRenderPass() failed to create gui renderpass"));
   }
   /* Depth/Composite and Framebuffers */
   {
      CT_VK_CHECK(vkBackend.CreateCompleteImage(
                    compositeBuffer,
                    compositeFormat,
                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                    VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
                    VK_IMAGE_ASPECT_COLOR_BIT,
                    internalResolutionWidth,
                    internalResolutionHeight),
                  CT_NC("Failed to create composite buffer."));
      CT_VK_CHECK(
        vkBackend.CreateCompleteImage(depthBuffer,
                                      depthFormat,
                                      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                      VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
                                      VK_IMAGE_ASPECT_DEPTH_BIT,
                                      internalResolutionWidth,
                                      internalResolutionHeight),
        CT_NC("Failed to create depth buffer."));
   }
   /* Render to image finished */
   {
      VkSemaphoreCreateInfo semaphoreInfo {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
      for (int i = 0; i < CT_MAX_INFLIGHT_FRAMES; i++) {
         vkCreateSemaphore(vkBackend.vkDevice,
                           &semaphoreInfo,
                           &vkBackend.vkAllocCallback,
                           &renderFinished[i]);
      }
   }
   /* GUI Framebuffer */
   {
      VkImageView fbAttachments[] {compositeBuffer.view, depthBuffer.view};
      VkFramebufferCreateInfo framebufferInfo {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
      framebufferInfo.attachmentCount = ctCStaticArrayLen(fbAttachments);
      framebufferInfo.pAttachments = fbAttachments;
      framebufferInfo.width = internalResolutionWidth;
      framebufferInfo.height = internalResolutionHeight;
      framebufferInfo.layers = 1;
      framebufferInfo.renderPass = forwardRenderPass;
      CT_VK_CHECK(vkCreateFramebuffer(vkBackend.vkDevice,
                                      &framebufferInfo,
                                      &vkBackend.vkAllocCallback,
                                      &forwardFramebuffer),
                  CT_NC("vkCreateFramebuffer() failed to create gui framebuffer."));
   }
   return CT_SUCCESS;
}

ctResults ctVkKeyLimeCore::DestroyScreenResources() {
   vkDestroyRenderPass(vkBackend.vkDevice, forwardRenderPass, &vkBackend.vkAllocCallback);
   vkDestroyFramebuffer(
     vkBackend.vkDevice, forwardFramebuffer, &vkBackend.vkAllocCallback);
   vkBackend.TryDestroyCompleteImage(depthBuffer);
   vkBackend.TryDestroyCompleteImage(compositeBuffer);
   for (int i = 0; i < CT_MAX_INFLIGHT_FRAMES; i++) {
      vkDestroySemaphore(
        vkBackend.vkDevice, renderFinished[i], &vkBackend.vkAllocCallback);
   }
   return CT_SUCCESS;
}

ctResults ctVkKeyLimeCore::UpdateCamera(ctKeyLimeCameraDesc cameraDesc) {
   ctAssert(viewBufferCount >= 1);
   return CT_SUCCESS;
}

ctResults ctVkKeyLimeCore::Render() {
   ZoneScoped;

   /* View Projection */
   ctCameraInfo camera = Engine->SceneEngine->GetCameraInfo(NULL);
   ctMat4 viewMatrix = ctMat4(1);
   ctMat4 projMatrix = ctMat4(1);
   ctMat4Rotate(viewMatrix, -camera.rotation);
   ctMat4Translate(viewMatrix, -camera.position);
   ctMat4PerspectiveInfinite(projMatrix,
                             camera.fov,
                             (float)vkBackend.mainScreenResources.extent.width /
                               (float)vkBackend.mainScreenResources.extent.height,
                             0.01f);

   /* Build Debug Internals */
   vkIm3d.BuildDrawLists();
   Engine->Im3dIntegration->DrawImguiText(ctMat4());
   vkImgui.BuildDrawLists();

   if (vkBackend.mainScreenResources.HandleResizeIfNeeded(&vkBackend)) {
      internalResolutionWidth = vkBackend.mainScreenResources.extent.width;
      internalResolutionHeight = vkBackend.mainScreenResources.extent.height;
      DestroyScreenResources();
      CreateScreenResources();
      vkImgui.SetDisplaySize(vkBackend.mainScreenResources.extent.width,
                             vkBackend.mainScreenResources.extent.height,
                             internalResolutionWidth,
                             internalResolutionHeight);
      vkBackend.RecreateSync();
      VkSemaphoreCreateInfo semaphoreInfo {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
   }
   if (vkBackend.WaitForFrameAvailible() != VK_SUCCESS) { return CT_SUCCESS; };

#if CITRUS_INCLUDE_AUDITION
   /* Check if shaders have been updated */
   if (ShaderHotReload.isContentUpdated()) {
      ctDebugLog("Shaders Updated...");
      vkDeviceWaitIdle(vkBackend.vkDevice);
      vkIm3d.LoadShaders(forwardRenderPass, 0);
      ShaderHotReload.ClearChanges();
   }
#endif

   VkCommandBuffer gfxCommands = gfxCommandBuffers[vkBackend.currentFrame];
   VkCommandBufferBeginInfo gfxBeginInfo {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
   vkResetCommandBuffer(gfxCommands, 0);
   vkBeginCommandBuffer(gfxCommands, &gfxBeginInfo);

   /* Render GUI */
   {
      VkClearValue clearValues[2];
      clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
      clearValues[1].depthStencil = {0.0f, 0};
      VkRenderPassBeginInfo passBeginInfo {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
      passBeginInfo.clearValueCount = ctCStaticArrayLen(clearValues);
      passBeginInfo.pClearValues = clearValues;
      passBeginInfo.renderPass = forwardRenderPass;
      passBeginInfo.framebuffer = forwardFramebuffer;
      passBeginInfo.renderArea =
        VkRect2D {{0, 0}, {internalResolutionWidth, internalResolutionHeight}};
      vkCmdBeginRenderPass(gfxCommands, &passBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

      /* Scissor/Viewport */
      VkViewport viewport = {0};
      viewport.maxDepth = 1.0f;
      viewport.width = (float)internalResolutionWidth;
      viewport.height = (float)internalResolutionHeight;
      VkRect2D scissor = {{0, 0}, {internalResolutionWidth, internalResolutionHeight}};
      vkCmdSetViewport(gfxCommands, 0, 1, &viewport);
      vkCmdSetScissor(gfxCommands, 0, 1, &scissor);

      vkIm3d.RenderCommands(
        gfxCommands,
        ctVec2((float)internalResolutionWidth, (float)internalResolutionHeight),
        viewMatrix,
        projMatrix);

      vkImgui.RenderCommands(gfxCommands);
      vkCmdEndRenderPass(gfxCommands);
   }
   /* Submit commands */
   {
      vkEndCommandBuffer(gfxCommands);
      VkSubmitInfo submitInfo {VK_STRUCTURE_TYPE_SUBMIT_INFO};
      submitInfo.commandBufferCount = 1;
      submitInfo.pCommandBuffers = &gfxCommands;
      submitInfo.signalSemaphoreCount = 1;
      submitInfo.pSignalSemaphores = &renderFinished[vkBackend.currentFrame];
      vkQueueSubmit(vkBackend.graphicsQueue, 1, &submitInfo, NULL);

      VkImageBlit blitInfo;
      blitInfo.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      blitInfo.dstSubresource.baseArrayLayer = 0;
      blitInfo.dstSubresource.mipLevel = 0;
      blitInfo.dstSubresource.layerCount = 1;
      blitInfo.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      blitInfo.srcSubresource.baseArrayLayer = 0;
      blitInfo.srcSubresource.mipLevel = 0;
      blitInfo.srcSubresource.layerCount = 1;
      blitInfo.srcOffsets[0] = {0};
      blitInfo.srcOffsets[1] = {
        (int32_t)internalResolutionWidth, (int32_t)internalResolutionHeight, 1};
      blitInfo.dstOffsets[0] = {0};
      blitInfo.dstOffsets[1] = {(int32_t)vkBackend.mainScreenResources.extent.width,
                                (int32_t)vkBackend.mainScreenResources.extent.height,
                                1};
      vkBackend.mainScreenResources.BlitAndPresent(
        &vkBackend,
        vkBackend.queueFamilyIndices.graphicsIdx,
        vkBackend.graphicsQueue,
        1,
        &renderFinished[vkBackend.currentFrame],
        compositeBuffer.image,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        vkBackend.queueFamilyIndices.graphicsIdx,
        blitInfo);
   }
   vkBackend.AdvanceNextFrame();
   return CT_SUCCESS;
}
