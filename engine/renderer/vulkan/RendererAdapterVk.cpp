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

#include "utilities/Common.h"
#include "renderer/Renderer.hpp"
#include "core/EngineCore.hpp"
#include "renderer/vulkan/VkBackend.hpp"

ctResults ctRenderer::Startup() {
   ZoneScoped;
   vkBackend = new ctVkBackend();
   vkBackend->preferredDevice = -1;
#ifndef NDEBUG
   vkBackend->validationEnabled = 1;
#else
   vkBackend->validationEnabled = 0;
#endif
   vkBackend->maxSamplers = CT_MAX_GFX_SAMPLERS;
   vkBackend->maxSampledImages = CT_MAX_GFX_SAMPLED_IMAGES;
   vkBackend->maxStorageImages = CT_MAX_GFX_STORAGE_IMAGES;
   vkBackend->maxStorageBuffers = CT_MAX_GFX_STORAGE_BUFFERS;

   ctSettingsSection* vkSettings = Engine->Settings->CreateSection("VulkanBackend", 2);
   vkSettings->BindInteger(&vkBackend->preferredDevice,
                           false,
                           true,
                           "PreferredDevice",
                           "Index of the preferred device to use for rendering.");
   vkSettings->BindInteger(&vkBackend->validationEnabled,
                           false,
                           true,
                           "ValidationEnabled",
                           "Use validation layers for debug testing.");

   vkSettings->BindInteger(&vkBackend->maxSamplers,
                           false,
                           true,
                           "MaxSamplers",
                           "Max number of samplers.");
   vkSettings->BindInteger(&vkBackend->maxSampledImages,
                           false,
                           true,
                           "MaxSampledImages",
                           "Max number of sampled images.");
   vkSettings->BindInteger(&vkBackend->maxStorageImages,
                           false,
                           true,
                           "MaxStorageImages",
                           "Max number of storage images.");
   vkSettings->BindInteger(&vkBackend->maxStorageBuffers,
                           false,
                           true,
                           "MaxStorageBuffers",
                           "Max number of storage buffers.");

   return vkBackend->ModuleStartup(Engine);
};

ctResults ctRenderer::Shutdown() {
   const ctResults results = vkBackend->ModuleShutdown();
   delete vkBackend;
   return results;
};