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

#pragma once

#include "utilities/Common.h"

#include "core/ModuleBase.hpp"
#include "core/Logging.hpp"
#include "core/WindowManager.hpp"

enum ctGfxBackend {
   CT_GFX_NULL = 0,
   CT_GFX_VULKAN = 1,
   CT_GFX_COUNT,
   CT_GFX_MAX = UINT32_MAX
};

class ctGfxShaderFactory {
public:
   virtual ctGfxBackend GetBackendId() = 0;
};

class ctGfxBufferBase {
public:
   virtual ctGfxBackend GetBackendId() = 0;
};

class ctGfxBufferManagerBase {
public:
   virtual ctGfxBackend GetBackendId() = 0;
};

class ctGfxTextureBase {
public:
   virtual ctGfxBackend GetBackendId() = 0;
};

class ctGfxTextureManagerBase {
public:
   virtual ctGfxBackend GetBackendId() = 0;
};

/* Abstracts threadsafe, sortable command buffer */
class ctGfxCommandBufferBase {
public:
   virtual ctGfxBackend GetBackendId() = 0;
};

/* Abstracts a renderpass or subpass */
class ctGfxPassBase {
public:
   virtual ctGfxBackend GetBackendId() = 0;
};

/* Manages framebuffers, render pass chains, secondary buffers, etc */
class ctGfxPassManagerBase {
public:
   virtual ctGfxBackend GetBackendId() = 0;
   virtual ctGfxPassBase* FindPass(const ctStringUtf8 name) = 0;
};

class ctGfxPipelineAdapterBase {
public:
   virtual ctGfxBackend GetBackendId() = 0;
};

class ctGfxResourceBindingManagerBase {
public:
   virtual ctGfxBackend GetBackendId() = 0;
};

class ctGfxCanvasBase {
public:
   virtual ctGfxBackend GetBackendId() = 0;
   virtual uint32_t GetWidth();
   virtual uint32_t GetHeight();
   virtual uint32_t GetMaxWidth();
   virtual uint32_t GetMaxHeight();

   virtual ctResults Resize(uint32_t width, uint32_t height) = 0;

   /* Window to render to (can be null) */
   virtual SDL_Window* GetOutputWindowPtr();
   /* Texture to composite render results to */
   virtual ctGfxTextureBase* GetOutputTexturePtr();

protected:
   uint32_t width;
   uint32_t height;
   uint32_t maxWidth;
   uint32_t maxHeight;
   SDL_Window* pWindow;
   ctGfxTextureBase* pTexture;
};

class ctGfxCanvasManagerBase {
public:
   virtual ctGfxBackend GetBackendId() = 0;

   virtual ctGfxCanvasBase* CreateWindowCanvas(ctWindow* pWindow) = 0;
   virtual ctResults DestroyWindowCanvas(ctGfxCanvasBase* pCanvas) = 0;
   virtual void AddWindowEvent(const SDL_WindowEvent* event) = 0;

   ctDynamicArray<ctGfxCanvasBase*> Canvases;
};

class ctGfxCoreBase : public ctModuleBase {
public:
   virtual ctGfxBackend GetBackendId() = 0;

   virtual ctResults RenderFrame() = 0;

   /* Canvases to draw to for onscreen/offscreen rendering */
   ctGfxCanvasManagerBase* CanvasManager;
   /* Describes different rendering path layouts */
   ctGfxPassManagerBase* PassManager;
   /* Allocates textures */
   ctGfxTextureManagerBase* TextureManager;
   /* Allocates buffers */
   ctGfxBufferManagerBase* BufferManager;
   /* Handles the reading and reflection of code */
   ctGfxShaderFactory* ShaderFactory;
   /* Adapter for pipelines which handles caching and permutations */
   ctGfxPipelineAdapterBase* PipelineAdapter;
   /* Bindless indexing and descriptor set management */
   ctGfxResourceBindingManagerBase* ResourceBindingManager;
};