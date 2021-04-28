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
#ifndef NDEBUG
   vkBackend.validationEnabled = 1;
#else
   vkBackend.validationEnabled = 0;
#endif
   vkBackend.maxSamplers = CT_MAX_GFX_SAMPLERS;
   vkBackend.maxSampledImages = CT_MAX_GFX_SAMPLED_IMAGES;
   vkBackend.maxStorageImages = CT_MAX_GFX_STORAGE_IMAGES;
   vkBackend.maxStorageBuffers = CT_MAX_GFX_STORAGE_BUFFERS;
   vkBackend.maxGraphicsCommandBuffers = CT_MAX_GFX_GRAPHICS_COMMAND_BUFFERS;
   vkBackend.maxComputeCommandBuffers = CT_MAX_GFX_COMPUTE_COMMAND_BUFFERS;
   vkBackend.maxTransferCommandBuffers = CT_MAX_GFX_TRANSFER_COMMAND_BUFFERS;

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

   vkSettings->BindInteger(&vkBackend.maxGraphicsCommandBuffers,
                           false,
                           true,
                           "MaxGraphicsCommandBuffers",
                           "Max number of graphics command buffers per-frame.",
                           CT_SETTINGS_BOUNDS_UINT);
   vkSettings->BindInteger(&vkBackend.maxComputeCommandBuffers,
                           false,
                           true,
                           "MaxComputeCommandBuffers",
                           "Max number of compute command buffers per-frame.",
                           CT_SETTINGS_BOUNDS_UINT);
   vkSettings->BindInteger(&vkBackend.maxTransferCommandBuffers,
                           false,
                           true,
                           "MaxTransferCommandBuffers",
                           "Max number of transfer command buffers per-frame.",
                           CT_SETTINGS_BOUNDS_UINT);

   ctSettingsSection* settings = Engine->Settings->CreateSection("KeyLimeRenderer", 32);
   Engine->OSEventManager->WindowEventHandlers.Append({sendResizeSignal, this});

   vkBackend.ModuleStartup(Engine);
   /* Depth/Composite buffers */
   {
      compositeFormat = VK_FORMAT_R8G8B8A8_UNORM;
      depthFormat = VK_FORMAT_D32_SFLOAT;
      vkBackend.CreateCompleteImage(compositeBuffer,
                                    compositeFormat,
                                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                      VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                    VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
                                    VK_IMAGE_ASPECT_COLOR_BIT,
                                    1920,
                                    1080);
      vkBackend.CreateCompleteImage(depthBuffer,
                                    depthFormat,
                                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                    VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
                                    VK_IMAGE_ASPECT_DEPTH_BIT,
                                    1920,
                                    1080);
   }
   /* GUI Renderpass */
   {
      VkAttachmentDescription attachments[2] {};
      attachments[0].format = compositeFormat;
      attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      attachments[0].finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
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
      CT_VK_CHECK(
        vkCreateRenderPass(
          vkBackend.vkDevice, &createInfo, &vkBackend.vkAllocCallback, &guiRenderPass),
        CT_NC("vkCreateRenderPass() failed to create gui renderpass"));
   }

   vkImgui.Startup(&vkBackend, guiRenderPass, 0);
   return CT_SUCCESS;
}

ctResults ctVkKeyLimeCore::Shutdown() {
   vkImgui.Shutdown(&vkBackend);
   vkBackend.TryDestroyCompleteImage(depthBuffer);
   vkBackend.TryDestroyCompleteImage(compositeBuffer);
   vkBackend.ModuleShutdown();
   return CT_SUCCESS;
}

ctResults ctVkKeyLimeCore::Render() {
   return CT_SUCCESS;
}
