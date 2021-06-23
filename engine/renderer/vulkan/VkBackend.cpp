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
#include "core/Application.hpp"

#include "tracy/Tracy.hpp"

#ifdef _WIN32
#include <Windows.h>
#endif
#define VMA_IMPLEMENTATION
#include "vma/vk_mem_alloc.h"

#define PIPELINE_CACHE_FILE_PATH "VK_PIPELINE_CACHE"

/* ------------- Debug Callback ------------- */

VKAPI_ATTR VkBool32 VKAPI_CALL vDebugCallback(VkDebugReportFlagsEXT flags,
                                              VkDebugReportObjectTypeEXT objType,
                                              uint64_t srcObject,
                                              size_t location,
                                              int32_t msgCode,
                                              const char* pLayerPrefix,
                                              const char* pMsg,
                                              void* pUserData) {
   ZoneScoped;
   if ((flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) != 0) {
      ctDebugLog("VK Information: %s", pLayerPrefix, pMsg);
   } else {
      ctDebugError("VK Validation Layer: [%s] Code %u : %s", pLayerPrefix, msgCode, pMsg);
   }
   return VK_FALSE;
}

/* ------------- Helpers ------------- */

bool ctVkBackend::isValidationLayersAvailible() {
   ZoneScoped;
   uint32_t layerCount = 0;
   vkEnumerateInstanceLayerProperties(&layerCount, NULL);
   ctDynamicArray<VkLayerProperties> layerProps;
   layerProps.Resize(layerCount);
   vkEnumerateInstanceLayerProperties(&layerCount, layerProps.Data());
   for (uint32_t i = 0; i < layerCount; i++) {
      ctDebugLog("VK Layer: \"%s\" found", layerProps[i].layerName);
   }
   bool found;
   for (uint32_t i = 0; i < validationLayers.Count(); i++) {
      found = false;
      for (uint32_t j = 0; j < layerCount; j++) {
         if (strcmp(validationLayers[i], layerProps[j].layerName)) {
            found = true;
            break;
         }
      }
      if (!found) { return false; }
   }
   return true;
}

bool vIsQueueFamilyComplete(ctVkQueueFamilyIndices indices) {
   return (indices.graphicsIdx != UINT32_MAX && indices.transferIdx != UINT32_MAX &&
           indices.presentIdx != UINT32_MAX && indices.computeIdx != UINT32_MAX);
}

ctVkQueueFamilyIndices ctVkBackend::FindQueueFamilyIndices(VkPhysicalDevice gpu) {
   ZoneScoped;
   ctVkQueueFamilyIndices result = {UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX};
   uint32_t queueFamilyCount;
   vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, NULL);
   VkQueueFamilyProperties* queueFamilyProps = (VkQueueFamilyProperties*)ctMalloc(
     sizeof(VkQueueFamilyProperties) * queueFamilyCount);
   vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, queueFamilyProps);
   for (uint32_t i = 0; i < queueFamilyCount; i++) {
      if (queueFamilyProps[i].queueCount > 0 &&
          queueFamilyProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
         result.graphicsIdx = i;
      if (queueFamilyProps[i].queueCount > 0 &&
          queueFamilyProps[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
         result.computeIdx = i;
      if (queueFamilyProps[i].queueCount > 0 &&
          queueFamilyProps[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
         result.transferIdx = i;

      VkBool32 present = VK_FALSE;
      vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i, mainScreenResources.surface, &present);
      if (queueFamilyProps[i].queueCount > 0 && present) result.presentIdx = i;

      if (vIsQueueFamilyComplete(result)) break;
   }
   ctFree(queueFamilyProps);
   return result;
}

VkResult ctVkBackend::CreateCompleteImage(ctVkCompleteImage& fullImage,
                                          VkFormat format,
                                          VkImageUsageFlags usage,
                                          VmaAllocationCreateFlags allocFlags,
                                          VkImageAspectFlags aspect,
                                          uint32_t width,
                                          uint32_t height,
                                          uint32_t depth,
                                          uint32_t mip,
                                          uint32_t layers,
                                          VkSampleCountFlagBits samples,
                                          VkImageType imageType,
                                          VkImageViewType viewType,
                                          VkImageTiling tiling,
                                          VkImageLayout initialLayout,
                                          VmaMemoryUsage memUsage,
                                          int32_t imageFlags,
                                          VkSharingMode sharing,
                                          uint32_t queueFamilyIndexCount,
                                          uint32_t* pQueueFamilyIndices) {
   ZoneScoped;
   VkImageCreateInfo imageInfo {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
   imageInfo.format = format;
   imageInfo.usage = usage;
   imageInfo.extent.width = width;
   imageInfo.extent.height = height;
   imageInfo.extent.depth = depth;
   imageInfo.mipLevels = mip;
   imageInfo.samples = samples;
   imageInfo.imageType = imageType;
   imageInfo.sharingMode = sharing;
   imageInfo.tiling = tiling;
   imageInfo.initialLayout = initialLayout;
   imageInfo.flags = imageFlags;
   imageInfo.arrayLayers = layers;
   imageInfo.queueFamilyIndexCount = queueFamilyIndexCount;
   imageInfo.pQueueFamilyIndices = pQueueFamilyIndices;

   VmaAllocationCreateInfo allocInfo {};
   allocInfo.flags = allocFlags;
   allocInfo.usage = memUsage;

   VkResult result = vmaCreateImage(
     vmaAllocator, &imageInfo, &allocInfo, &fullImage.image, &fullImage.alloc, NULL);
   if (result != VK_SUCCESS) { return result; }

   VkImageViewCreateInfo viewInfo {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
   viewInfo.image = fullImage.image;
   viewInfo.viewType = viewType;
   viewInfo.format = format;
   viewInfo.subresourceRange.aspectMask = aspect;
   viewInfo.subresourceRange.baseArrayLayer = 0;
   viewInfo.subresourceRange.layerCount = layers;
   viewInfo.subresourceRange.baseMipLevel = 0;
   viewInfo.subresourceRange.levelCount = mip;
   return vkCreateImageView(vkDevice, &viewInfo, &vkAllocCallback, &fullImage.view);
}

VkResult ctVkBackend::CreateCompleteBuffer(ctVkCompleteBuffer& fullBuffer,
                                           VkBufferUsageFlags usage,
                                           VmaAllocationCreateFlags allocFlags,
                                           size_t size,
                                           VmaMemoryUsage memUsage,
                                           int32_t bufferFlags,
                                           VkSharingMode sharing,
                                           uint32_t queueFamilyIndexCount,
                                           uint32_t* pQueueFamilyIndices) {
   ZoneScoped;
   VkBufferCreateInfo bufferInfo {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
   bufferInfo.usage = usage;
   bufferInfo.size = size;
   bufferInfo.sharingMode = sharing;
   bufferInfo.queueFamilyIndexCount = queueFamilyIndexCount;
   bufferInfo.pQueueFamilyIndices = pQueueFamilyIndices;

   VmaAllocationCreateInfo allocInfo {};
   allocInfo.flags = allocFlags;
   allocInfo.usage = memUsage;
   return vmaCreateBuffer(
     vmaAllocator, &bufferInfo, &allocInfo, &fullBuffer.buffer, &fullBuffer.alloc, NULL);
}

VkResult ctVkBackend::CreateShaderModuleFromAsset(VkShaderModule& shader,
                                                  const char* path) {
   ZoneScoped;
   ctStringUtf8 fullPath = Engine->FileSystem->GetAssetPath();
   fullPath += path;
   fullPath.FilePathLocalize();
   uint32_t* shaderData = NULL;
   ctFile shaderFile;
   CT_RETURN_ON_FAIL(shaderFile.Open(fullPath.CStr(), CT_FILE_OPEN_READ),
                     VK_ERROR_UNKNOWN);
   size_t fSize = shaderFile.GetFileSize();
   shaderData = (uint32_t*)ctMalloc(fSize);
   CT_RETURN_ON_NULL(shaderData, VK_ERROR_UNKNOWN);
   shaderFile.ReadRaw(shaderData, fSize, 1);
   shaderFile.Close();
   VkShaderModuleCreateInfo info = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
   info.codeSize = fSize;
   info.pCode = shaderData;
   VkResult result = vkCreateShaderModule(vkDevice, &info, &vkAllocCallback, &shader);
   ctFree(shaderData);
   return result;
}

VkResult
ctVkBackend::CreateBindlessPipelineLayout(VkPipelineLayout& layout,
                                          uint32_t pushConstantRangeCount,
                                          VkPushConstantRange* pPushConstantRanges) {
   ZoneScoped;
   VkPipelineLayoutCreateInfo createInfo = {
     VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
   createInfo.setLayoutCount = 1;
   createInfo.pSetLayouts = &vkDescriptorSetLayout;
   createInfo.pushConstantRangeCount = pushConstantRangeCount;
   createInfo.pPushConstantRanges = pPushConstantRanges;
   return vkCreatePipelineLayout(vkDevice, &createInfo, &vkAllocCallback, &layout);
}

VkResult
ctVkBackend::CreateGraphicsPipeline(VkPipeline& pipeline,
                                    VkPipelineLayout layout,
                                    VkRenderPass renderpass,
                                    uint32_t subpass,
                                    VkShaderModule vertexShader,
                                    VkShaderModule fragShader,
                                    bool depthTest,
                                    bool depthWrite,
                                    VkFrontFace winding,
                                    VkCullModeFlags cullMode,
                                    VkPrimitiveTopology topology,
                                    bool blendEnable,
                                    uint32_t colorBlendCount,
                                    VkPipelineColorBlendAttachmentState* pCustomBlends,
                                    VkSampleCountFlagBits msaaSamples,
                                    uint32_t customDynamicCount,
                                    VkDynamicState* pDynamicStates) {
   ZoneScoped;
   ctAssert(colorBlendCount <= 8);
   VkGraphicsPipelineCreateInfo createInfo = {
     VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};

   VkPipelineColorBlendAttachmentState colorBlendsDefault[8];
   colorBlendsDefault[0] = VkPipelineColorBlendAttachmentState();
   colorBlendsDefault[0].blendEnable = blendEnable ? VK_TRUE : VK_FALSE;
   colorBlendsDefault[0].colorWriteMask =
     VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
     VK_COLOR_COMPONENT_A_BIT;
   for (int i = 1; i < 8; i++) {
      colorBlendsDefault[i] = colorBlendsDefault[0];
   }

   /* Shader stages */
   VkPipelineShaderStageCreateInfo shaderStages[2];
   createInfo.stageCount = ctCStaticArrayLen(shaderStages);
   createInfo.pStages = shaderStages;

   /* Vertex */
   shaderStages[0] = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
   shaderStages[0].module = vertexShader;
   shaderStages[0].pName = "main";
   shaderStages[0].pSpecializationInfo = NULL;
   shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;

   /* Fragment */
   shaderStages[1] = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
   shaderStages[1].module = fragShader;
   shaderStages[1].pName = "main";
   shaderStages[1].pSpecializationInfo = NULL;
   shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;

   /* Color Blend state */
   VkPipelineColorBlendStateCreateInfo colorBlendStateDefault = {
     VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
   colorBlendStateDefault.attachmentCount = colorBlendCount;
   colorBlendStateDefault.pAttachments =
     pCustomBlends ? pCustomBlends : colorBlendsDefault;

   /* Rasterization state */
   VkPipelineRasterizationStateCreateInfo rasterStateDefault = {
     VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
   rasterStateDefault.cullMode = cullMode;
   rasterStateDefault.frontFace = winding;
   rasterStateDefault.lineWidth = 1.0f;
   rasterStateDefault.polygonMode = VK_POLYGON_MODE_FILL;
   rasterStateDefault.rasterizerDiscardEnable = VK_FALSE;
   rasterStateDefault.depthBiasEnable = VK_FALSE;
   rasterStateDefault.depthClampEnable = VK_FALSE;
   rasterStateDefault.depthBiasConstantFactor = 0.0f;
   rasterStateDefault.depthBiasSlopeFactor = 0.0f;
   rasterStateDefault.depthBiasClamp = 0.0f;

   /* Viewport state */
   VkPipelineViewportStateCreateInfo viewportStateDefault = {
     VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
   VkViewport viewportDefault = {0.0f, 0.0f, 512.0f, 512.0f, 0.0f, 1.0f};
   VkRect2D scissorDefault = {0, 00, 512, 512};
   viewportStateDefault.viewportCount = 1;
   viewportStateDefault.pViewports = &viewportDefault;
   viewportStateDefault.scissorCount = 1;
   viewportStateDefault.pScissors = &scissorDefault;

   /* Vertex state */
   VkPipelineVertexInputStateCreateInfo vertexStateDefault = {
     VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
   vertexStateDefault.vertexAttributeDescriptionCount = 0;
   vertexStateDefault.vertexBindingDescriptionCount = 0;

   /* Depth Stencil state */
   VkPipelineDepthStencilStateCreateInfo depthStencilStateDefault = {
     VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
   depthStencilStateDefault.depthTestEnable = depthTest ? VK_TRUE : VK_FALSE;
   depthStencilStateDefault.depthWriteEnable = depthWrite ? VK_TRUE : VK_FALSE;
   depthStencilStateDefault.depthCompareOp = VK_COMPARE_OP_GREATER;
   depthStencilStateDefault.depthBoundsTestEnable = VK_FALSE;
   depthStencilStateDefault.minDepthBounds = 0.0f;
   depthStencilStateDefault.maxDepthBounds = 1.0f;
   depthStencilStateDefault.stencilTestEnable = VK_FALSE;

   /* Input Assembly state */
   VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateDefault = {
     VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
   inputAssemblyStateDefault.topology = topology;
   inputAssemblyStateDefault.primitiveRestartEnable = VK_FALSE;

   /* MSAA state */
   VkPipelineMultisampleStateCreateInfo msaaStateDefault = {
     VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
   msaaStateDefault.sampleShadingEnable =
     msaaSamples != VK_SAMPLE_COUNT_1_BIT ? VK_TRUE : VK_FALSE;
   msaaStateDefault.rasterizationSamples = msaaSamples;

   /* Tessellation state */
   VkPipelineTessellationStateCreateInfo tessStateDefault = {
     VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO, 0, 0, 0};

   /* Dynamic state */
   VkPipelineDynamicStateCreateInfo dynamicStateDefault = {
     VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
   VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_VIEWPORT};
   dynamicStateDefault.pDynamicStates = pDynamicStates ? pDynamicStates : dynamicStates;
   dynamicStateDefault.dynamicStateCount =
     pDynamicStates ? customDynamicCount : ctCStaticArrayLen(dynamicStates);

   /* Renderpass and Subpass */
   createInfo.renderPass = renderpass;
   createInfo.subpass = subpass;

   /* Pipeline derivitives */
   createInfo.basePipelineHandle = VK_NULL_HANDLE;
   createInfo.basePipelineIndex = 0;

   /* Assign states */
   createInfo.layout = layout;
   createInfo.pColorBlendState = &colorBlendStateDefault;
   createInfo.pViewportState = &viewportStateDefault;
   createInfo.pRasterizationState = &rasterStateDefault;
   createInfo.pVertexInputState = &vertexStateDefault;
   createInfo.pDepthStencilState = &depthStencilStateDefault;
   createInfo.pInputAssemblyState = &inputAssemblyStateDefault;
   createInfo.pMultisampleState = &msaaStateDefault;
   createInfo.pTessellationState = &tessStateDefault;
   createInfo.pDynamicState = &dynamicStateDefault;

   return vkCreateGraphicsPipelines(
     vkDevice, vkPipelineCache, 1, &createInfo, &vkAllocCallback, &pipeline);
}

void ctVkBackend::TryDestroyCompleteImage(ctVkCompleteImage& fullImage) {
   ZoneScoped;
   if (fullImage.view == VK_NULL_HANDLE) { return; }
   vkDestroyImageView(vkDevice, fullImage.view, &vkAllocCallback);
   if (fullImage.image == VK_NULL_HANDLE || fullImage.alloc == VK_NULL_HANDLE) { return; }
   vmaDestroyImage(vmaAllocator, fullImage.image, fullImage.alloc);
}

void ctVkBackend::TryDestroyCompleteBuffer(ctVkCompleteBuffer& fullBuffer) {
   ZoneScoped;
   if (fullBuffer.buffer == VK_NULL_HANDLE || fullBuffer.alloc == VK_NULL_HANDLE) {
      return;
   }
   vmaDestroyBuffer(vmaAllocator, fullBuffer.buffer, fullBuffer.alloc);
}

VkResult ctVkBackend::WaitForFrameAvailible() {
   ZoneScoped;
   VkResult result = vkWaitForFences(
     vkDevice, 1, &frameAvailibleFences[currentFrame], VK_TRUE, UINT64_MAX);
   if (result != VK_SUCCESS) { return result; }
   return vkResetFences(vkDevice, 1, &frameAvailibleFences[currentFrame]);
}

void ctVkBackend::ExposeBindlessStorageBuffer(uint32_t& outIdx,
                                              VkBuffer buffer,
                                              VkDeviceSize range,
                                              VkDeviceSize offset) {
   ZoneScoped;
   outIdx = descriptorsStorageBuffer.AllocateSlot();
   VkDescriptorBufferInfo buffInfo = {0};
   buffInfo.buffer = buffer;
   buffInfo.offset = offset;
   buffInfo.range = range;
   VkWriteDescriptorSet write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
   write.descriptorCount = 1;
   write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
   write.dstSet = vkGlobalDescriptorSet;
   write.dstBinding = 3;
   write.dstArrayElement = outIdx;
   write.pBufferInfo = &buffInfo;
   vkUpdateDescriptorSets(vkDevice, 1, &write, 0, NULL);
}

void ctVkBackend::ReleaseBindlessStorageBuffer(uint32_t idx) {
   descriptorsStorageBuffer.ReleaseSlot(idx);
}

void ctVkSwapchainSupport::GetSupport(VkPhysicalDevice gpu, VkSurfaceKHR surface) {
   ZoneScoped;
   vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &surfaceCapabilities);

   uint32_t formatCount;
   vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, NULL);
   if (formatCount) {
      surfaceFormats.Resize(formatCount);
      vkGetPhysicalDeviceSurfaceFormatsKHR(
        gpu, surface, &formatCount, surfaceFormats.Data());
   }

   uint32_t presentModeCount;
   vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &presentModeCount, NULL);
   if (presentModeCount) {
      presentModes.Resize(presentModeCount);
      vkGetPhysicalDeviceSurfacePresentModesKHR(
        gpu, surface, &presentModeCount, presentModes.Data());
   }
}

bool ctVkBackend::DeviceHasRequiredExtensions(VkPhysicalDevice gpu) {
   ZoneScoped;
   uint32_t extCount;
   vkEnumerateDeviceExtensionProperties(gpu, NULL, &extCount, NULL);
   VkExtensionProperties* availible =
     (VkExtensionProperties*)ctMalloc(sizeof(VkExtensionProperties) * extCount);
   vkEnumerateDeviceExtensionProperties(gpu, NULL, &extCount, availible);

   bool foundAll = true;
   bool foundThis;
   for (uint32_t i = 0; i < deviceExtensions.Count(); i++) {
      foundThis = false;
      for (uint32_t ii = 0; ii < extCount; ii++) {
         if (strcmp(deviceExtensions[i], availible[ii].extensionName) == 0) {
            foundThis = true;
            break;
         }
      }
      if (!foundThis) foundAll = false;
   }

   ctFree(availible);
   return foundAll;
}

bool vDeviceHasSwapChainSupport(VkPhysicalDevice gpu, VkSurfaceKHR surface) {
   ZoneScoped;
   ctVkSwapchainSupport support;
   support.GetSupport(gpu, surface);
   return !support.presentModes.isEmpty() && !support.surfaceFormats.isEmpty();
}

bool vDeviceHasRequiredFeatures(
  const VkPhysicalDeviceFeatures& features,
  const VkPhysicalDeviceDescriptorIndexingFeatures& descriptorIndexing) {
   if (!features.shaderFloat64 || !features.depthClamp || !features.depthBounds ||
       !features.shaderSampledImageArrayDynamicIndexing ||
       !features.shaderStorageBufferArrayDynamicIndexing ||
       !descriptorIndexing.descriptorBindingPartiallyBound ||
       !descriptorIndexing.runtimeDescriptorArray ||
       !descriptorIndexing.descriptorBindingSampledImageUpdateAfterBind ||
       !descriptorIndexing.descriptorBindingStorageBufferUpdateAfterBind ||
       !descriptorIndexing.descriptorBindingStorageImageUpdateAfterBind ||
       !descriptorIndexing.shaderSampledImageArrayNonUniformIndexing ||
       !descriptorIndexing.shaderStorageBufferArrayNonUniformIndexing ||
       !descriptorIndexing.shaderStorageImageArrayNonUniformIndexing) {
      return false;
   }
   return true;
}

VkPhysicalDevice ctVkBackend::PickBestDevice(VkPhysicalDevice* pGpus,
                                             uint32_t count,
                                             VkSurfaceKHR surface) {
   ZoneScoped;
   int64_t bestGPUIdx = -1;
   int64_t bestGPUScore = -1;
   int64_t currentGPUScore;
   for (uint32_t i = 0; i < count; i++) {
      currentGPUScore = 0;
      VkPhysicalDeviceProperties deviceProperties;
      VkPhysicalDeviceFeatures deviceFeatures;

      VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures {
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES};
      VkPhysicalDeviceFeatures2 deviceFeatures2 = {
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
      deviceFeatures2.pNext = &descriptorIndexingFeatures;

      vkGetPhysicalDeviceProperties(pGpus[i], &deviceProperties);
      vkGetPhysicalDeviceFeatures(pGpus[i], &deviceFeatures);
      vkGetPhysicalDeviceFeatures2(pGpus[i], &deviceFeatures2);

      /*Disqualifications*/
      if (!vDeviceHasRequiredFeatures(deviceFeatures, descriptorIndexingFeatures))
         continue; /*Device doesn't meet the minimum features spec*/
      if (!vIsQueueFamilyComplete(FindQueueFamilyIndices(pGpus[i])))
         continue; /*Queue families are incomplete*/
      if (!DeviceHasRequiredExtensions(pGpus[i]))
         continue; /*Doesn't have the required extensions*/
      if (!vDeviceHasSwapChainSupport(pGpus[i], surface))
         continue; /*Doesn't have swap chain support*/

      /*Benifits*/
      if (deviceProperties.deviceType ==
          VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) /* Discrete GPU */
         currentGPUScore += 10000;
      /* Favor well known vendors who have reliable drivers */
      if (deviceProperties.vendorID == 0x10DE || /* NVIDIA */
          deviceProperties.vendorID == 0x1002)   /* AMD */
         currentGPUScore += 5000;

      /*Set as GPU if its the best*/
      if (currentGPUScore > bestGPUScore) {
         bestGPUScore = currentGPUScore;
         bestGPUIdx = i;
      }
   }
   if (bestGPUIdx < 0) return VK_NULL_HANDLE;
   return pGpus[bestGPUIdx];
}

/* ------------- Alloc Functions ------------- */

void* vAllocFunction(void* pUserData,
                     size_t size,
                     size_t alignment,
                     VkSystemAllocationScope allocationScope) {
   ZoneScoped;
   return ctAlignedMalloc(size, alignment);
}

void* vReallocFunction(void* pUserData,
                       void* pOriginal,
                       size_t size,
                       size_t alignment,
                       VkSystemAllocationScope allocationScope) {
   ZoneScoped;
   return ctAlignedRealloc(pOriginal, size, alignment);
}

void vFreeFunction(void* pUserData, void* pMemory) {
   ZoneScoped;
   ctAlignedFree(pMemory);
}

/* ------------- Backend Core ------------- */

ctResults ctVkBackend::Startup() {
   ZoneScoped;
   if (!Engine->WindowManager->isStarted()) { return CT_FAILURE_DEPENDENCY_NOT_MET; }
   if (!Engine->OSEventManager->isStarted()) { return CT_FAILURE_DEPENDENCY_NOT_MET; }

   ctDebugLog("Starting Vulkan Backend...");
   /* Fill in AppInfo */
   {
      vkAppInfo = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
      vkAppInfo.apiVersion = VK_API_VERSION_1_2;
      vkAppInfo.pEngineName = "CitrusToolbox";
      vkAppInfo.engineVersion = VK_MAKE_VERSION(CITRUS_ENGINE_VERSION_MAJOR,
                                                CITRUS_ENGINE_VERSION_MINOR,
                                                CITRUS_ENGINE_VERSION_PATCH);
      vkAppInfo.pApplicationName = Engine->App->GetAppName();
      const ctAppVersion appVersion = Engine->App->GetAppVersion();
      ;
      vkAppInfo.applicationVersion =
        VK_MAKE_VERSION(appVersion.major, appVersion.minor, appVersion.patch);

      vkAllocCallback = VkAllocationCallbacks {};
      vkAllocCallback.pUserData = NULL;
      vkAllocCallback.pfnAllocation = (PFN_vkAllocationFunction)vAllocFunction;
      vkAllocCallback.pfnReallocation = (PFN_vkReallocationFunction)vReallocFunction;
      vkAllocCallback.pfnFree = (PFN_vkFreeFunction)vFreeFunction;

      validationLayers.Append("VK_LAYER_KHRONOS_validation");
   }
   /* Create Instance */
   {
      ctDebugLog("Getting Extensions...");
      if (validationEnabled && !isValidationLayersAvailible()) {
         ctFatalError(-1, CT_NC("Vulkan Validation layers requested but not avalible."));
      }

      unsigned int sdlExtCount;
      if (!SDL_Vulkan_GetInstanceExtensions(
            Engine->WindowManager->mainWindow.pSDLWindow, &sdlExtCount, NULL)) {
         ctFatalError(-1,
                      CT_NC("SDL_Vulkan_GetInstanceExtensions() Failed to get "
                            "instance extensions."));
      }
      instanceExtensions.Resize(sdlExtCount);
      SDL_Vulkan_GetInstanceExtensions(NULL, &sdlExtCount, instanceExtensions.Data());

      if (validationEnabled) {
         instanceExtensions.Append(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
      }

      deviceExtensions.Append(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
      deviceExtensions.Append("VK_EXT_descriptor_indexing");
      if (validationEnabled) {
         deviceExtensions.Append(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
      }

      for (uint32_t i = 0; i < instanceExtensions.Count(); i++) {
         ctDebugLog("VK Instance Extension Requested: \"%s\"", instanceExtensions[i]);
      }
      for (uint32_t i = 0; i < deviceExtensions.Count(); i++) {
         ctDebugLog("VK Device Extension Requested: \"%s\"", deviceExtensions[i]);
      }

      VkInstanceCreateInfo instanceInfo = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
      instanceInfo.pApplicationInfo = &vkAppInfo;
      if (validationEnabled) {
         instanceInfo.ppEnabledLayerNames = validationLayers.Data();
         instanceInfo.enabledLayerCount = (uint32_t)validationLayers.Count();
      } else {
         instanceInfo.enabledLayerCount = 0;
         instanceInfo.ppEnabledLayerNames = NULL;
      }

      instanceInfo.enabledExtensionCount = (uint32_t)instanceExtensions.Count();
      instanceInfo.ppEnabledExtensionNames = instanceExtensions.Data();
      ctDebugLog("Creating Instance...");
      CT_VK_CHECK(
        vkCreateInstance(&instanceInfo, &vkAllocCallback, &vkInstance),
        CT_NC("vkCreateInstance() Failed to create vulkan instance.\n"
              "Please update the graphics drivers to the latest version available.\n"
              "If this or other issues persist then upgrade the hardware or contact "
              "support."));
   }
   /*Setup Validation Debug Callback*/
   if (validationEnabled) {
      PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallback =
        (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(
          vkInstance, "vkCreateDebugReportCallbackEXT");

      VkDebugReportCallbackCreateInfoEXT createInfo = {
        VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT};
      createInfo.pfnCallback = (PFN_vkDebugReportCallbackEXT)vDebugCallback;
      createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;

      vkDebugCallback = VK_NULL_HANDLE;
      if (vkCreateDebugReportCallback) {
         vkCreateDebugReportCallback(
           vkInstance, &createInfo, &vkAllocCallback, &vkDebugCallback);
      } else {
         ctFatalError(-1, CT_NC("Failed to find vkCreateDebugReportCallbackEXT()."));
      }
   }
   /* Initialize a first surface to check for support */
   {
      mainScreenResources.CreateSurface(this,
                                        Engine->WindowManager->mainWindow.pSDLWindow);
   } /* Pick a GPU */
   {
      ctDebugLog("Finding GPU...");
      uint32_t gpuCount;
      CT_VK_CHECK(vkEnumeratePhysicalDevices(vkInstance, &gpuCount, NULL),
                  CT_NC("Failed to find devices with vkEnumeratePhysicalDevices()."));
      if (!gpuCount) {
         ctFatalError(-1,
                      CT_NC("No supported Vulkan compatible rendering device found.\n"
                            "Please upgrade the hardware."));
      }
      ctDynamicArray<VkPhysicalDevice> gpus;
      gpus.Resize(gpuCount);
      vkEnumeratePhysicalDevices(vkInstance, &gpuCount, gpus.Data());
      if (preferredDevice >= 0 && preferredDevice < (int32_t)gpuCount) {
         vkPhysicalDevice = gpus[preferredDevice];

         VkPhysicalDeviceFeatures deviceFeatures;
         VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures {
           VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES};
         VkPhysicalDeviceFeatures2 deviceFeatures2 = {
           VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
         deviceFeatures2.pNext = &descriptorIndexingFeatures;

         vkGetPhysicalDeviceFeatures(vkPhysicalDevice, &deviceFeatures);
         vkGetPhysicalDeviceFeatures2(vkPhysicalDevice, &deviceFeatures2);

         if (!vDeviceHasRequiredFeatures(deviceFeatures, descriptorIndexingFeatures) ||
             !DeviceHasRequiredExtensions(vkPhysicalDevice) ||
             !vDeviceHasSwapChainSupport(vkPhysicalDevice, mainScreenResources.surface)) {
            ctFatalError(-1, CT_NC("Rendering device does not meet requirements."));
         }
      } else {
         vkPhysicalDevice =
           PickBestDevice(gpus.Data(), gpuCount, mainScreenResources.surface);
         if (vkPhysicalDevice == VK_NULL_HANDLE) {
            ctFatalError(
              -1,
              CT_NC(
                "Could not find suitable rendering device.\n"
                "Please update the graphics drivers to the latest version available.\n"
                "If this or other issues persist then upgrade the hardware or contact "
                "support."));
         }
      }
   }
   /* Queues and Device */
   {
      ctDebugLog("Setting Queues...");
      VkDeviceCreateInfo deviceInfo = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
      /* Queue Creation */
      queueFamilyIndices = FindQueueFamilyIndices(vkPhysicalDevice);
      if (!vIsQueueFamilyComplete(queueFamilyIndices)) {
         ctFatalError(-1, CT_NC("Device doesn't have the necessary queues."));
      }

      uint32_t uniqueIdxCount = 0;
      uint32_t uniqueIndices[4] = {UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX};

      /*ONLY create a list of unique indices*/
      {
         uint32_t nonUniqueIndices[4] = {
           queueFamilyIndices.graphicsIdx,
           queueFamilyIndices.presentIdx,
           queueFamilyIndices.computeIdx,
           queueFamilyIndices.transferIdx,
         };
         bool found;
         for (uint32_t i = 0; i < 4; i++) { /*Add items*/
            found = false;
            for (uint32_t ii = 0; ii < uniqueIdxCount + 1;
                 ii++) {                                       /*Search previous items*/
               if (uniqueIndices[ii] == nonUniqueIndices[i]) { /*Only add if unique*/
                  found = true;
                  break;
               }
            }
            if (!found) {
               uniqueIndices[uniqueIdxCount] = nonUniqueIndices[i];
               uniqueIdxCount++;
            }
         }
      }

      VkDeviceQueueCreateInfo queueCreateInfos[4];
      float defaultPriority = 1.0f;
      for (uint32_t i = 0; i < uniqueIdxCount; i++) {
         queueCreateInfos[i] = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
         queueCreateInfos[i].queueCount = 1;
         queueCreateInfos[i].queueFamilyIndex = uniqueIndices[i];
         queueCreateInfos[i].pQueuePriorities = &defaultPriority;
      }
      deviceInfo.queueCreateInfoCount = uniqueIdxCount;
      deviceInfo.pQueueCreateInfos = queueCreateInfos;

      /* Validation */
      if (validationEnabled) {
         deviceInfo.ppEnabledLayerNames = validationLayers.Data();
         deviceInfo.enabledLayerCount = (uint32_t)validationLayers.Count();
      } else {
         deviceInfo.enabledLayerCount = 0;
         deviceInfo.ppEnabledLayerNames = NULL;
      }

      /* Enable Features */
      VkPhysicalDeviceFeatures features = VkPhysicalDeviceFeatures();
      features.imageCubeArray = VK_TRUE;
      features.shaderFloat64 = VK_TRUE;
      features.depthBounds = VK_TRUE;
      features.depthClamp = VK_TRUE;
      features.samplerAnisotropy = VK_TRUE;
      deviceInfo.pEnabledFeatures = &features;

      VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures = {
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES};
      indexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
      indexingFeatures.runtimeDescriptorArray = VK_TRUE;
      indexingFeatures.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
      indexingFeatures.shaderStorageBufferArrayNonUniformIndexing = VK_TRUE;
      indexingFeatures.shaderStorageImageArrayNonUniformIndexing = VK_TRUE;
      indexingFeatures.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
      indexingFeatures.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;
      indexingFeatures.descriptorBindingStorageImageUpdateAfterBind = VK_TRUE;
      deviceInfo.pNext = &indexingFeatures;

      deviceInfo.enabledExtensionCount = (uint32_t)deviceExtensions.Count();
      deviceInfo.ppEnabledExtensionNames = deviceExtensions.Data();
      ctDebugLog("Creating Device...");
      CT_VK_CHECK(
        vkCreateDevice(vkPhysicalDevice, &deviceInfo, &vkAllocCallback, &vkDevice),
        CT_NC("vkCreateDevice() failed to create the device."));

      ctDebugLog("Getting Queues...");
      /* Get Queues */
      vkGetDeviceQueue(vkDevice, queueFamilyIndices.graphicsIdx, 0, &graphicsQueue);
      vkGetDeviceQueue(vkDevice, queueFamilyIndices.presentIdx, 0, &presentQueue);
      vkGetDeviceQueue(vkDevice, queueFamilyIndices.computeIdx, 0, &computeQueue);
      vkGetDeviceQueue(vkDevice, queueFamilyIndices.transferIdx, 0, &transferQueue);
   }
   /* Swapchain and Present Resources */
   {
      mainScreenResources.CreatePresentResources(this);
      mainScreenResources.CreateSwapchain(
        this, queueFamilyIndices, vsync, VK_NULL_HANDLE);
   }
   /* Memory Allocator */
   {
      ctDebugLog("Creating Vulkan Memory Allocator...");
      VmaAllocatorCreateInfo allocatorInfo = {};
      /* Current version won't go further than Vulkan 1.1 */
      allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_1;
      allocatorInfo.physicalDevice = vkPhysicalDevice;
      allocatorInfo.device = vkDevice;
      allocatorInfo.instance = vkInstance;

      CT_VK_CHECK(vmaCreateAllocator(&allocatorInfo, &vmaAllocator),
                  CT_NC("vmaCreateAllocator() failed to create allocator."));
   }
   /* Frame Sync */
   {
      VkFenceCreateInfo fenceInfo {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
      fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
      for (int i = 0; i < CT_MAX_INFLIGHT_FRAMES; i++) {
         vkCreateFence(vkDevice, &fenceInfo, &vkAllocCallback, &frameAvailibleFences[i]);
      }
   }
   /* Pipeline Cache */
   {
      ctDebugLog("Loading Pipeline Cache...");
      ctFile cacheFile;
      Engine->FileSystem->OpenPreferencesFile(
        cacheFile, PIPELINE_CACHE_FILE_PATH, CT_FILE_OPEN_READ);
      size_t cacheSize = 0;
      void* cacheData = NULL;
      if (cacheFile.isOpen()) {
         cacheSize = (size_t)cacheFile.GetFileSize();
         cacheData = ctMalloc(cacheSize);
         cacheFile.ReadRaw(cacheData, cacheSize, 1);
         cacheFile.Close();
      }
      VkPipelineCacheCreateInfo cacheInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO};
      cacheInfo.pInitialData = cacheData;
      cacheInfo.initialDataSize = cacheSize;
      CT_VK_CHECK(
        vkCreatePipelineCache(vkDevice, &cacheInfo, &vkAllocCallback, &vkPipelineCache),
        CT_NC("vkCreatePipelineCache() failed to create cache."));
      if (cacheData) { ctFree(cacheData); }

      // Todo: (maybe later inside of key lime) :)
      // https://gpuopen.com/performance/#pso
      // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#pipelines-cache
      // Compile uber-shaders
   }
   /* Bindless System */
   {
      ctDebugLog("Starting Bindless System...");
      VkDescriptorSetLayoutBinding descriptorSetLayouts[4] = {};
      VkDescriptorPoolSize descriptorPoolSizes[4] = {};
      VkDescriptorBindingFlags bindFlags[4] = {};
      /* Sampler */
      descriptorSetLayouts[0].binding = 0;
      descriptorSetLayouts[0].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
      descriptorSetLayouts[0].stageFlags = VK_SHADER_STAGE_ALL;
      descriptorSetLayouts[0].descriptorCount = maxSamplers;
      descriptorPoolSizes[0].type = VK_DESCRIPTOR_TYPE_SAMPLER;
      descriptorPoolSizes[0].descriptorCount = maxSamplers;
      bindFlags[0] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
      descriptorsSamplers = ctVkDescriptorManager(maxSamplers);
      /* Sampled Image */
      descriptorSetLayouts[1].binding = 1;
      descriptorSetLayouts[1].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
      descriptorSetLayouts[1].stageFlags = VK_SHADER_STAGE_ALL;
      descriptorSetLayouts[1].descriptorCount = maxSampledImages;
      descriptorPoolSizes[1].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
      descriptorPoolSizes[1].descriptorCount = maxSampledImages;
      bindFlags[1] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
                     VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
      descriptorsSampledImage = ctVkDescriptorManager(maxSampledImages);
      /* Storage Image */
      descriptorSetLayouts[2].binding = 2;
      descriptorSetLayouts[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
      descriptorSetLayouts[2].stageFlags = VK_SHADER_STAGE_ALL;
      descriptorSetLayouts[2].descriptorCount = maxStorageImages;
      descriptorPoolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
      descriptorPoolSizes[2].descriptorCount = maxStorageImages;
      bindFlags[2] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
                     VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
      descriptorsStorageImage = ctVkDescriptorManager(maxStorageImages);
      /* Storage Buffer */
      descriptorSetLayouts[3].binding = 3;
      descriptorSetLayouts[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
      descriptorSetLayouts[3].stageFlags = VK_SHADER_STAGE_ALL;
      descriptorSetLayouts[3].descriptorCount = maxStorageBuffers;
      descriptorPoolSizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
      descriptorPoolSizes[3].descriptorCount = maxStorageBuffers;
      bindFlags[3] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
                     VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
      descriptorsStorageBuffer = ctVkDescriptorManager(maxStorageBuffers);

      VkDescriptorSetLayoutBindingFlagsCreateInfo bindingExtInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO};
      bindingExtInfo.pBindingFlags = bindFlags;
      bindingExtInfo.bindingCount = ctCStaticArrayLen(bindFlags);

      VkDescriptorSetLayoutCreateInfo descSetLayoutInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
      descSetLayoutInfo.pNext = &bindingExtInfo;
      descSetLayoutInfo.flags =
        VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
      descSetLayoutInfo.bindingCount = ctCStaticArrayLen(descriptorSetLayouts);
      descSetLayoutInfo.pBindings = descriptorSetLayouts;
      CT_VK_CHECK(
        vkCreateDescriptorSetLayout(
          vkDevice, &descSetLayoutInfo, &vkAllocCallback, &vkDescriptorSetLayout),
        CT_NC("vkCreateDescriptorSetLayout() failed to create descriptor set layout."));

      VkDescriptorPoolCreateInfo poolInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
      poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT |
                       VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
      poolInfo.poolSizeCount = ctCStaticArrayLen(descriptorPoolSizes);
      poolInfo.pPoolSizes = descriptorPoolSizes;
      poolInfo.maxSets = 1;
      CT_VK_CHECK(
        vkCreateDescriptorPool(vkDevice, &poolInfo, &vkAllocCallback, &vkDescriptorPool),
        CT_NC("vkCreateDescriptorPool() failed to create descriptor pool."));

      VkDescriptorSetAllocateInfo allocInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
      allocInfo.descriptorSetCount = 1;
      allocInfo.descriptorPool = vkDescriptorPool;
      allocInfo.pSetLayouts = &vkDescriptorSetLayout;
      CT_VK_CHECK(
        vkAllocateDescriptorSets(vkDevice, &allocInfo, &vkGlobalDescriptorSet),
        CT_NC("vkAllocateDescriptorSets() failed to allocate global descriptor set."));

      /* https://ourmachinery.com/post/moving-the-machinery-to-bindless/
       * https://roar11.com/2019/06/vulkan-textures-unbound/
       * https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VK_EXT_descriptor_indexing.html
       * https://gpuopen.com/performance/#descriptors */
   }
   /* Make Window Visible */
   {
      ctDebugLog("Showing Window...");
      Engine->WindowManager->ShowMainWindow();
   }

   ctDebugLog("Vulkan Backend has Started!");
   return CT_SUCCESS;
}

ctResults ctVkBackend::Shutdown() {
   ZoneScoped;
   ctDebugLog("Vulkan Backend is Shutting Down...");
   vkDeviceWaitIdle(vkDevice);

   /* Save Pipeline Cache */
   {
      size_t cacheSize = 0;
      void* cacheData = NULL;
      vkGetPipelineCacheData(vkDevice, vkPipelineCache, &cacheSize, NULL);
      if (cacheSize) {
         cacheData = ctMalloc(cacheSize);
         vkGetPipelineCacheData(vkDevice, vkPipelineCache, &cacheSize, cacheData);
         ctFile cacheFile;
         Engine->FileSystem->OpenPreferencesFile(
           cacheFile, PIPELINE_CACHE_FILE_PATH, CT_FILE_OPEN_WRITE);
         if (cacheFile.isOpen()) {
            cacheFile.WriteRaw(cacheData, cacheSize, 1);
            cacheFile.Close();
         }
         ctFree(cacheData);
      }
      vkDestroyPipelineCache(vkDevice, vkPipelineCache, &vkAllocCallback);
   }

   for (int i = 0; i < CT_MAX_INFLIGHT_FRAMES; i++) {
      vkDestroyFence(vkDevice, frameAvailibleFences[i], &vkAllocCallback);
   }

   vkDestroyDescriptorPool(vkDevice, vkDescriptorPool, &vkAllocCallback);
   vkDestroyDescriptorSetLayout(vkDevice, vkDescriptorSetLayout, &vkAllocCallback);
   mainScreenResources.DestroySwapchain(this);
   mainScreenResources.DestroySurface(this);
   mainScreenResources.DestroyPresentResources(this);
   vmaDestroyAllocator(vmaAllocator);
   vkDestroyDevice(vkDevice, &vkAllocCallback);

   if (vkDebugCallback != VK_NULL_HANDLE) {
      PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallback =
        (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(
          vkInstance, "vkDestroyDebugReportCallbackEXT");
      vkDestroyDebugReportCallback(vkInstance, vkDebugCallback, &vkAllocCallback);
   }

   vkDestroyInstance(vkInstance, &vkAllocCallback);
   return CT_SUCCESS;
}

/* ------------- Screen Resources ------------- */

ctResults ctVkScreenResources::CreateSurface(ctVkBackend* pBackend, SDL_Window* pWindow) {
   ZoneScoped;
   ctDebugLog("Creating Surface...");
   window = pWindow;
   if (SDL_Vulkan_CreateSurface(pWindow, pBackend->vkInstance, &surface)) {
      return CT_SUCCESS;
   }
   return CT_FAILURE_UNKNOWN;
}

VkSurfaceFormatKHR vPickBestSurfaceFormat(VkSurfaceFormatKHR* formats, size_t count) {
   ZoneScoped;
   for (uint32_t i = 0; i < count; i++) {
      const VkSurfaceFormatKHR fmt = formats[i];
      if (fmt.format == VK_FORMAT_B8G8R8A8_SRGB &&
          fmt.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
         return fmt;
      }
   }
   return formats[0];
}

VkPresentModeKHR vPickPresentMode(VkPresentModeKHR* modes, size_t count, bool vsync) {
   ZoneScoped;
   int64_t bestIdx = 0;
   int64_t bestScore = -1;
   int64_t currentScore;

   for (uint32_t i = 0; i < count; i++) {
      const VkPresentModeKHR mode = modes[i];
      currentScore = 0;

      /* Present to screen ASAP, no V-Sync */
      if (mode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
         if (vsync == false) {
            currentScore = 1000;
         } else {
            currentScore = 0;
         }
      }
      /* Present and swap to backbuffer. Needs to wait if full. */
      if (mode == VK_PRESENT_MODE_FIFO_KHR) {
         if (vsync == true) {
            currentScore = 500;
         } else {
            currentScore = 0;
         }
      }
      /* Present and swap to backbuffer. Can continue if full. */
      if (mode == VK_PRESENT_MODE_FIFO_RELAXED_KHR) {
         if (vsync == true) {
            currentScore = 1000;
         } else {
            currentScore = 0;
         }
      }

      if (currentScore > bestScore) {
         bestIdx = i;
         bestScore = currentScore;
      }
   }
   return modes[bestIdx];
}

ctResults ctVkScreenResources::CreateSwapchain(ctVkBackend* pBackend,
                                               ctVkQueueFamilyIndices indices,
                                               int32_t vsync,
                                               VkSwapchainKHR oldSwapchain) {
   ZoneScoped;
   ctDebugLog("Creating Swapchain...");

   swapChainSupport.GetSupport(pBackend->vkPhysicalDevice, surface);

   /* Select a Image and Color Format */
   surfaceFormat = vPickBestSurfaceFormat(swapChainSupport.surfaceFormats.Data(),
                                          swapChainSupport.surfaceFormats.Count());

   /* Select a Present Mode */
   presentMode = vPickPresentMode(
     swapChainSupport.presentModes.Data(), swapChainSupport.presentModes.Count(), vsync);

   /* Get Size */
   int w, h;
   SDL_Vulkan_GetDrawableSize(window, &w, &h);
   extent.width = w;
   extent.height = h;

   /* Get Image Count */
   imageCount = swapChainSupport.surfaceCapabilities.minImageCount + 1;
   if (swapChainSupport.surfaceCapabilities.maxImageCount > 0 &&
       imageCount > swapChainSupport.surfaceCapabilities.maxImageCount) {
      imageCount = swapChainSupport.surfaceCapabilities.maxImageCount;
   }

   /* Create swapchain */
   uint32_t sharedIndices[] {indices.graphicsIdx, indices.presentIdx};
   VkSwapchainCreateInfoKHR swapChainInfo = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
   swapChainInfo.surface = surface;
   swapChainInfo.imageFormat = surfaceFormat.format;
   swapChainInfo.imageColorSpace = surfaceFormat.colorSpace;
   swapChainInfo.presentMode = presentMode;
   swapChainInfo.imageExtent = extent;
   swapChainInfo.imageArrayLayers = 1;
   swapChainInfo.minImageCount = imageCount;
   swapChainInfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
   swapChainInfo.preTransform = swapChainSupport.surfaceCapabilities.currentTransform;
   swapChainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
   swapChainInfo.clipped = VK_TRUE;
   swapChainInfo.oldSwapchain = oldSwapchain;
   if (sharedIndices[0] == sharedIndices[1]) {
      swapChainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
      swapChainInfo.queueFamilyIndexCount = 0;
      swapChainInfo.pQueueFamilyIndices = NULL;
   } else {
      swapChainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
      swapChainInfo.queueFamilyIndexCount = 2;
      swapChainInfo.pQueueFamilyIndices = sharedIndices;
   }
   CT_VK_CHECK(
     vkCreateSwapchainKHR(
       pBackend->vkDevice, &swapChainInfo, &pBackend->vkAllocCallback, &swapchain),
     CT_NC("vkCreateSwapchainKHR() failed to create swapchain."));

   /* Get images */
   vkGetSwapchainImagesKHR(pBackend->vkDevice, swapchain, &imageCount, NULL);
   swapImages.Resize(imageCount);
   vkGetSwapchainImagesKHR(pBackend->vkDevice, swapchain, &imageCount, swapImages.Data());

   return CT_SUCCESS;
}

ctResults ctVkScreenResources::CreatePresentResources(ctVkBackend* pBackend) {
   ZoneScoped;
   /* Create image availible semaphore */
   VkSemaphoreCreateInfo semaphoreInfo {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
   for (int i = 0; i < CT_MAX_INFLIGHT_FRAMES; i++) {
      vkCreateSemaphore(pBackend->vkDevice,
                        &semaphoreInfo,
                        &pBackend->vkAllocCallback,
                        &imageAvailible[i]);
   }
   /* Blitting command buffer */
   VkCommandPoolCreateInfo poolInfo {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
   poolInfo.queueFamilyIndex = pBackend->queueFamilyIndices.transferIdx;
   poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
   vkCreateCommandPool(
     pBackend->vkDevice, &poolInfo, &pBackend->vkAllocCallback, &blitCommandPool);
   VkCommandBufferAllocateInfo allocInfo {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
   allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
   allocInfo.commandBufferCount = CT_MAX_INFLIGHT_FRAMES;
   allocInfo.commandPool = blitCommandPool;
   vkAllocateCommandBuffers(pBackend->vkDevice, &allocInfo, blitCommands);
   return CT_SUCCESS;
}

ctResults ctVkScreenResources::DestroySwapchain(ctVkBackend* pBackend) {
   ZoneScoped;
   vkDestroySwapchainKHR(pBackend->vkDevice, swapchain, &pBackend->vkAllocCallback);
   return CT_SUCCESS;
}

ctResults ctVkScreenResources::DestroyPresentResources(ctVkBackend* pBackend) {
   ZoneScoped;
   for (int i = 0; i < CT_MAX_INFLIGHT_FRAMES; i++) {
      vkDestroySemaphore(
        pBackend->vkDevice, imageAvailible[i], &pBackend->vkAllocCallback);
   }
   vkDestroyCommandPool(pBackend->vkDevice, blitCommandPool, &pBackend->vkAllocCallback);
   return CT_SUCCESS;
}

ctResults ctVkScreenResources::DestroySurface(ctVkBackend* pBackend) {
   ZoneScoped;
   vkDestroySurfaceKHR(pBackend->vkInstance, surface, NULL);
   return CT_SUCCESS;
}

VkResult ctVkScreenResources::BlitAndPresent(ctVkBackend* pBackend,
                                             uint32_t blitQueueIdx,
                                             VkQueue blitQueue,
                                             uint32_t semaphoreCount,
                                             VkSemaphore* pWaitSemaphores,
                                             VkImage srcImage,
                                             VkImageLayout srcLayout,
                                             VkPipelineStageFlags srcStageMask,
                                             VkAccessFlags srcAccess,
                                             uint32_t srcQueueFamily,
                                             VkImageBlit blit) {
   ZoneScoped;
   uint32_t imageIndex = 0;
   VkResult result = vkAcquireNextImageKHR(pBackend->vkDevice,
                                           swapchain,
                                           UINT64_MAX,
                                           imageAvailible[pBackend->currentFrame],
                                           VK_NULL_HANDLE,
                                           &imageIndex);
   if (resizeTriggered || result == VK_ERROR_OUT_OF_DATE_KHR) {
      resizeTriggered = true;
      vkDeviceWaitIdle(pBackend->vkDevice);
      ctFatalError(-1, "Resize Not Supported Yet");
   } else if (result != VK_SUCCESS) {
      return result;
   }

   /* Blit */
   {
      VkCommandBuffer cmd = blitCommands[pBackend->currentFrame];
      VkCommandBufferBeginInfo beginInfo {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
      /*[ERROR] VK Validation Layer: [Validation] Code 0 : Validation Error: [
       * VUID-vkResetCommandBuffer-commandBuffer-00045 ] Object 0: handle = 0x2a5679cc080,
       * type = VK_OBJECT_TYPE_COMMAND_BUFFER; | MessageID = 0x1e7883ea | Attempt to reset
       * VkCommandBuffer 0x2a5679cc080[] which is in use. The Vulkan spec states:
       * commandBuffer must not be in the pending state
       * (https://vulkan.lunarg.com/doc/view/1.2.148.0/windows/1.2-extensions/vkspec.html#VUID-vkResetCommandBuffer-commandBuffer-00045)*/
      vkResetCommandBuffer(cmd, 0);
      vkBeginCommandBuffer(cmd, &beginInfo);

      VkImageMemoryBarrier srcToTransfer {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
      srcToTransfer.image = srcImage;
      srcToTransfer.srcQueueFamilyIndex = srcQueueFamily;
      srcToTransfer.dstQueueFamilyIndex = blitQueueIdx;
      srcToTransfer.oldLayout = srcLayout;
      srcToTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
      srcToTransfer.srcAccessMask = srcAccess;
      srcToTransfer.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
      srcToTransfer.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      srcToTransfer.subresourceRange.baseArrayLayer = 0;
      srcToTransfer.subresourceRange.baseMipLevel = 0;
      srcToTransfer.subresourceRange.levelCount = 1;
      srcToTransfer.subresourceRange.layerCount = 1;
      vkCmdPipelineBarrier(cmd,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_TRANSFER_BIT,
                           VK_DEPENDENCY_BY_REGION_BIT,
                           0,
                           NULL,
                           0,
                           NULL,
                           1,
                           &srcToTransfer);

      VkImageMemoryBarrier dstToTransfer {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
      dstToTransfer.image = swapImages[imageIndex];
      dstToTransfer.srcQueueFamilyIndex = pBackend->queueFamilyIndices.presentIdx;
      dstToTransfer.dstQueueFamilyIndex = blitQueueIdx;
      dstToTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED; /* Don't care about previous */
      dstToTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
      dstToTransfer.srcAccessMask = 0;
      dstToTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      dstToTransfer.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      dstToTransfer.subresourceRange.baseArrayLayer = 0;
      dstToTransfer.subresourceRange.baseMipLevel = 0;
      dstToTransfer.subresourceRange.levelCount = 1;
      dstToTransfer.subresourceRange.layerCount = 1;
      vkCmdPipelineBarrier(cmd,
                           VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                           VK_PIPELINE_STAGE_TRANSFER_BIT,
                           VK_DEPENDENCY_BY_REGION_BIT,
                           0,
                           NULL,
                           0,
                           NULL,
                           1,
                           &dstToTransfer);
      vkCmdBlitImage(cmd,
                     srcImage,
                     VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                     swapImages[imageIndex],
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                     1,
                     &blit,
                     VK_FILTER_LINEAR);
      VkImageMemoryBarrier dstToPresent {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
      dstToPresent.image = swapImages[imageIndex];
      dstToPresent.srcQueueFamilyIndex = blitQueueIdx;
      dstToPresent.dstQueueFamilyIndex = pBackend->queueFamilyIndices.presentIdx;
      dstToPresent.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
      dstToPresent.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
      dstToPresent.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      dstToPresent.dstAccessMask = 0; /* Won't touch it */
      dstToPresent.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      dstToPresent.subresourceRange.baseArrayLayer = 0;
      dstToPresent.subresourceRange.baseMipLevel = 0;
      dstToPresent.subresourceRange.levelCount = 1;
      dstToPresent.subresourceRange.layerCount = 1;
      vkCmdPipelineBarrier(cmd,
                           VK_PIPELINE_STAGE_TRANSFER_BIT,
                           VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                           VK_DEPENDENCY_BY_REGION_BIT,
                           0,
                           NULL,
                           0,
                           NULL,
                           1,
                           &dstToPresent);
      vkEndCommandBuffer(cmd);
      VkSubmitInfo submitInfo {VK_STRUCTURE_TYPE_SUBMIT_INFO};
      VkPipelineStageFlags waitForStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
      submitInfo.commandBufferCount = 1;
      submitInfo.pCommandBuffers = &cmd;
      submitInfo.waitSemaphoreCount = semaphoreCount;
      submitInfo.pWaitSemaphores = pWaitSemaphores;
      submitInfo.pWaitDstStageMask = &waitForStage;
      vkQueueSubmit(blitQueue,
                    1,
                    &submitInfo,
                    pBackend->frameAvailibleFences[pBackend->currentFrame]);
   }

   VkPresentInfoKHR presentInfo {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
   presentInfo.waitSemaphoreCount = 1;
   presentInfo.pWaitSemaphores = &imageAvailible[pBackend->currentFrame];
   presentInfo.swapchainCount = 1;
   presentInfo.pSwapchains = &swapchain;
   presentInfo.pImageIndices = &imageIndex;
   return vkQueuePresentKHR(pBackend->presentQueue, &presentInfo);
}

/* ------------- Descriptor Manager ------------- */

ctVkDescriptorManager::ctVkDescriptorManager() {
   nextNewIdx = 0;
   _max = 0;
}

ctVkDescriptorManager::ctVkDescriptorManager(int32_t max) {
   nextNewIdx = 0;
   _max = max;
}

int32_t ctVkDescriptorManager::AllocateSlot() {
   int32_t result = nextNewIdx;
   if (!freedIdx.isEmpty()) {
      result = freedIdx.Last();
      freedIdx.RemoveLast();
   }
   return result;
}

void ctVkDescriptorManager::ReleaseSlot(const int32_t idx) {
   freedIdx.Append(idx);
}