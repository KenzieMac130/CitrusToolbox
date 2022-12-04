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

#include "gpu/DebugUI.h"
#include "DeviceVulkan.hpp"
#include "ArchitectVulkan.hpp"
#include "PresentVulkan.hpp"
#include "TextureVulkan.hpp"
#include "BufferVulkan.hpp"

#if CITRUS_IMGUI
#include "imgui/imgui.h"

void ctGPUDebugUIDevice(ctGPUDevice* pDevice) {
   /* Device Info */
   uint32_t* propMultiPtr = NULL;
   float* propMultiFltPtr = NULL;
   // clang-format off
   ImGui::Text("Device Name: %s", pDevice->vPhysicalDeviceProperties.deviceName);
   ImGui::Text("Vendor ID: %d", pDevice->vPhysicalDeviceProperties.vendorID);
   ImGui::Text("Device ID: %d", pDevice->vPhysicalDeviceProperties.deviceID);
   ImGui::Text("API Version: %d", pDevice->vPhysicalDeviceProperties.apiVersion);
   ImGui::Text("Driver Version: %d", pDevice->vPhysicalDeviceProperties.driverVersion);
   ImGui::Text("Device Type: %d", (int)pDevice->vPhysicalDeviceProperties.deviceType);
   ImGui::Text("Validation Enabled: %s", pDevice->validationEnabled ? "true" : "false");
   ImGui::Text("Debug Markers: %s", pDevice->useMarkers ? "true" : "false");
   ImGui::Text("Preferred Device: %d", pDevice->preferredDevice);
   ImGui::Text("Next Frame Timeout: %d", pDevice->nextFrameTimeout);
   ImGui::Text("Default Anisotropy: %d", pDevice->defaultAnisotropyLevel);
   ImGui::Text("Queue Family Indices: (GFX: %d) (PRESENT: %d) (COMPUTE: %d) (TRANSFER: %d)",
               pDevice->queueFamilyIndices.graphicsIdx,
               pDevice->queueFamilyIndices.presentIdx,
               pDevice->queueFamilyIndices.computeIdx,
               pDevice->queueFamilyIndices.transferIdx);
   ImGui::Separator();
   VmaBudget mbudget;
   vmaGetBudget(pDevice->vmaAllocator, &mbudget);
   ImGui::Text("VMA Memory Usage: %d", mbudget.usage);
   ImGui::Text("VMA Alloc Bytes: %d", mbudget.allocationBytes);
   ImGui::Text("VMA Block Bytes: %d", mbudget.blockBytes);
   ImGui::Text("VMA Budget: %d", mbudget.budget);
   ImGui::Text("Staging Buffer Count: %d", pDevice->stagingBuffers.Count());
   if (!pDevice->isDynamicRenderingSupported()) {
       ctSpinLockEnterCritical(pDevice->jitUsableRenderInfoLock);
       ImGui::Text("JIT Renderpass Pool Count: %d", pDevice->usableRenderInfo.Count());
       ctSpinLockExitCritical(pDevice->jitPipelineRenderpassLock);
   }
   else {
       ImGui::Text("Using Dynamic Rendering");
   }
   // clang-format on
}

char g_architectDumpPath[4096] = {"ARCHITECT_DUMP.png"};
void ctGPUDebugUIArchitect(ctGPUDevice* pDevice,
                           ctGPUArchitect* pArchitect,
                           bool allowGVisDump) {
   if (allowGVisDump) {
      ImGui::InputText("Dump File Path", g_architectDumpPath, 4096);
      if (ImGui::Button("Dump GraphVis")) {
         ctGPUArchitectDumpGraphVis(pArchitect, g_architectDumpPath, true, true);
      }
   }
}

void ctGPUDebugUIPresent(ctGPUDevice* pDevice, ctGPUPresenter* pPresenter) {
}

void ctGPUDebugUIBindless(ctGPUDevice* pDevice, ctGPUBindlessManager* pBindless) {
}

void ctGPUDebugUIExternalBufferPool(ctGPUDevice* pDevice,
                                    ctGPUExternalBufferPool* pPool) {
}

void ctGPUDebugUIExternalTexturePool(ctGPUDevice* pDevice,
                                     ctGPUExternalTexturePool* pPool) {
}
#else
void ctGPUDebugUIDevice(ctGPUDevice* pDevice) {
}

void ctGPUDebugUIArchitect(ctGPUDevice* pDevice, ctGPUArchitect* pArchitect) {
}

void ctGPUDebugUIPresent(ctGPUDevice* pDevice, ctGPUPresenter* pPresenter) {
}

void ctGPUDebugUIBindless(ctGPUDevice* pDevice, ctGPUBindlessManager* pBindless) {
}

void ctGPUDebugUIExternalBufferPool(ctGPUDevice* pDevice,
                                    ctGPUExternalBufferPool* pPool) {
}

void ctGPUDebugUIExternalTexturePool(ctGPUDevice* pDevice,
                                     ctGPUExternalTexturePool* pPool) {
}
#endif