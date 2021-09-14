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

#include "VkBackend.hpp"
#include "VkImgui.hpp"

#include "imgui/backends/imgui_impl_vulkan.h"

void checkVkResult(VkResult err) {CT_VK_CHECK(
  err, CT_NCT("FAIL:VkDearImgui", "DearImgui Vulkan backend encountered an error."))}

ctResults ctVkImgui::Startup(ctVkBackend* pBackend,
                             VkCommandBuffer textureUploadCmd,
                             VkRenderPass guiRenderpass,
                             uint32_t subpass) {
   _pBackend = pBackend;
   /* Create descriptor pool for Imgui */
   VkDescriptorPoolSize poolSizes[] = {{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}};
   VkDescriptorPoolCreateInfo poolInfo = {};
   poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
   poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
   poolInfo.maxSets = 32;
   poolInfo.poolSizeCount = (uint32_t)ctCStaticArrayLen(poolSizes);
   poolInfo.pPoolSizes = poolSizes;
   CT_VK_CHECK(
     vkCreateDescriptorPool(
       pBackend->vkDevice, &poolInfo, &pBackend->vkAllocCallback, &_vkDescriptorPool),
     CT_NCT("FAIL:vkCreateDescriptorPool",
            "vkCreateDescriptorPool() could not create descriptor set."));

   ImGui_ImplVulkan_InitInfo initInfo = {};
   initInfo.Instance = pBackend->vkInstance;
   initInfo.PhysicalDevice = pBackend->vkPhysicalDevice;
   initInfo.Device = pBackend->vkDevice;
   initInfo.QueueFamily = pBackend->queueFamilyIndices.graphicsIdx;
   initInfo.Queue = pBackend->graphicsQueue;
   initInfo.DescriptorPool = _vkDescriptorPool;
   initInfo.Subpass = subpass;
   initInfo.MinImageCount =
     pBackend->mainScreenResources.swapChainSupport.surfaceCapabilities.minImageCount;
   initInfo.ImageCount = pBackend->mainScreenResources.imageCount;
   initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
   initInfo.Allocator = &pBackend->vkAllocCallback;
   initInfo.CheckVkResultFn = checkVkResult;
   initInfo.PipelineCache = pBackend->vkPipelineCache;
   ImGui_ImplVulkan_Init(&initInfo, guiRenderpass);

   ImGui_ImplVulkan_CreateFontsTexture(textureUploadCmd);
   ImGui::NewFrame();

   return CT_SUCCESS;
}

ctResults ctVkImgui::Shutdown() {
   ImGui_ImplVulkan_Shutdown();
   vkDestroyDescriptorPool(
     _pBackend->vkDevice, _vkDescriptorPool, &_pBackend->vkAllocCallback);
   return CT_SUCCESS;
}

void ctVkImgui::BuildDrawLists() {
   int width, height;
   SDL_Vulkan_GetDrawableSize(_pBackend->mainScreenResources.window, &width, &height);
   ImGui::GetIO().DisplaySize.x = (float)width;
   ImGui::GetIO().DisplaySize.y = (float)height;
   ImGui::Render();
}

void ctVkImgui::SetDisplaySize(int32_t windowWidth,
                               int32_t windowHeight,
                               int32_t internalWidth,
                               int32_t internalHeight) {
   ImGui::GetIO().DisplaySize.x = (float)windowWidth;
   ImGui::GetIO().DisplaySize.y = (float)windowHeight;
   ImGui::GetIO().DisplayFramebufferScale =
     ImVec2((float)internalWidth / windowWidth, (float)internalHeight / windowHeight);
}

void ctVkImgui::RenderCommands(VkCommandBuffer cmd) {
   ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd, 0);
}
