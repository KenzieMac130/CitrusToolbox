#
#	Copyright 2022 MacKenzie Strand
#
#	Licensed under the Apache License, Version 2.0 (the "License");
#	you may not use this file except in compliance with the License.
#	You may obtain a copy of the License at
#
#		http://www.apache.org/licenses/LICENSE-2.0
#
#	Unless required by applicable law or agreed to in writing, software
#	distributed under the License is distributed on an "AS IS" BASIS,
#	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#	See the License for the specific language governing permissions and
#	limitations under the License.
#

#---------- Version ----------
set (CITRUS_ENGINE_VERSION_MAJOR 1)
set (CITRUS_ENGINE_VERSION_MINOR 0)
set (CITRUS_ENGINE_VERSION_PATCH 0)

# ---------- Endian Test ----------
include (TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)
if(IS_BIG_ENDIAN)
set (CITRUS_ENDIAN 0xFF)
else()
set (CITRUS_ENDIAN 0x00)
endif()

#---------- Compile ISPC Kernels ----------
file(MAKE_DIRECTORY ${CITRUS_CODEGEN_DIR}/ispc)

set(ISPC_KERNEL_INPUTS
#${CMAKE_CURRENT_SOURCE_DIR}/XXX.ispc
)

set(ISPC_KERNEL_OUTPUTS
#${CITRUS_CODEGEN_DIR}/ispc/XXX.o
)

# Configure ispc options
set(ISPC_ARCH "x86-64")
set(ISPC_TARGET "sse2")
option(ISPC_DEBUG "Output ISPC debug symbols" ON)
if(ISPC_DEBUG)
set(ISPC_DEBUG_FLAGS "-g")
else()
set(ISPC_DEBUG_FLAGS "")
endif()
set(ISPC_OPT_FLAGS "-O2")

# Add build command for each file
foreach(ISPC_IN ISPC_OUT 
IN ZIP_LISTS
ISPC_KERNEL_INPUTS ISPC_KERNEL_OUTPUTS)
file(TOUCH ${ISPC_OUT})
add_custom_command(OUTPUT ${ISPC_OUT}
COMMAND 
${ISPC_COMPILER_PATH} 
${ISPC_DEBUG_FLAGS} ${ISPC_IN} 
-o ${ISPC_OUT} 
${ISPC_OPT_FLAGS} 
--arch=${ISPC_ARCH} 
--target=${ISPC_TARGET} 
DEPENDS ${ISPC_IN} 
${LIBRARY_INSTALL_DEPENDENCY})
endforeach()

# ---------- Utility Libraries ----------
set(ENGINE_SOURCE_FILES_UTILITIES
${CMAKE_CURRENT_SOURCE_DIR}/utilities/File.cpp
${CMAKE_CURRENT_SOURCE_DIR}/utilities/GUID.cpp
${CMAKE_CURRENT_SOURCE_DIR}/utilities/Hash.cpp
${CMAKE_CURRENT_SOURCE_DIR}/utilities/JSON.cpp
${CMAKE_CURRENT_SOURCE_DIR}/utilities/Memory.cpp
${CMAKE_CURRENT_SOURCE_DIR}/utilities/Noise.cpp
${CMAKE_CURRENT_SOURCE_DIR}/utilities/Random.cpp
${CMAKE_CURRENT_SOURCE_DIR}/utilities/SharedLogging.c
${CMAKE_CURRENT_SOURCE_DIR}/utilities/String.cpp
${CMAKE_CURRENT_SOURCE_DIR}/utilities/Sync.cpp
${CMAKE_CURRENT_SOURCE_DIR}/utilities/Time.cpp
)

set(ENGINE_HEADER_FILES_UTILITIES
${CMAKE_CURRENT_SOURCE_DIR}/utilities/Config.h.in
${CMAKE_CURRENT_SOURCE_DIR}/utilities/Common.h
${CMAKE_CURRENT_SOURCE_DIR}/utilities/BloomFilter.hpp
${CMAKE_CURRENT_SOURCE_DIR}/utilities/DynamicArray.hpp
${CMAKE_CURRENT_SOURCE_DIR}/utilities/File.hpp
${CMAKE_CURRENT_SOURCE_DIR}/utilities/GUID.hpp
${CMAKE_CURRENT_SOURCE_DIR}/utilities/HandleManager.hpp
${CMAKE_CURRENT_SOURCE_DIR}/utilities/HandledList.hpp
${CMAKE_CURRENT_SOURCE_DIR}/utilities/Hash.hpp
${CMAKE_CURRENT_SOURCE_DIR}/utilities/HashTable.hpp
${CMAKE_CURRENT_SOURCE_DIR}/utilities/JSON.hpp
${CMAKE_CURRENT_SOURCE_DIR}/utilities/Math.hpp
${CMAKE_CURRENT_SOURCE_DIR}/utilities/Math3d.hpp
${CMAKE_CURRENT_SOURCE_DIR}/utilities/Noise.hpp
${CMAKE_CURRENT_SOURCE_DIR}/utilities/Random.hpp
${CMAKE_CURRENT_SOURCE_DIR}/utilities/Reflect.hpp
${CMAKE_CURRENT_SOURCE_DIR}/utilities/RingBuffer.hpp
${CMAKE_CURRENT_SOURCE_DIR}/utilities/SharedLogging.h
${CMAKE_CURRENT_SOURCE_DIR}/utilities/SpacialQuery.hpp
${CMAKE_CURRENT_SOURCE_DIR}/utilities/StaticArray.hpp
${CMAKE_CURRENT_SOURCE_DIR}/utilities/String.hpp
${CMAKE_CURRENT_SOURCE_DIR}/utilities/Sync.hpp
${CMAKE_CURRENT_SOURCE_DIR}/utilities/Time.hpp
)


# ---------- Engine Core Framework ----------
set(ENGINE_SOURCE_FILES_CORE
${CMAKE_CURRENT_SOURCE_DIR}/core/Application.cpp
${CMAKE_CURRENT_SOURCE_DIR}/core/AsyncTasks.cpp
${CMAKE_CURRENT_SOURCE_DIR}/core/EngineCore.cpp
${CMAKE_CURRENT_SOURCE_DIR}/core/FileSystem.cpp
${CMAKE_CURRENT_SOURCE_DIR}/core/JobSystem.cpp
${CMAKE_CURRENT_SOURCE_DIR}/core/Logging.cpp
${CMAKE_CURRENT_SOURCE_DIR}/core/ModuleBase.cpp
${CMAKE_CURRENT_SOURCE_DIR}/core/OSEvents.cpp
${CMAKE_CURRENT_SOURCE_DIR}/core/Settings.cpp
${CMAKE_CURRENT_SOURCE_DIR}/core/Translation.cpp
${CMAKE_CURRENT_SOURCE_DIR}/core/WindowManager.cpp
${CMAKE_CURRENT_SOURCE_DIR}/core/GameLayer.cpp
)

set(ENGINE_HEADER_FILES_CORE
${CMAKE_CURRENT_SOURCE_DIR}/core/Application.hpp
${CMAKE_CURRENT_SOURCE_DIR}/core/AsyncTasks.hpp
${CMAKE_CURRENT_SOURCE_DIR}/core/EngineCore.hpp
${CMAKE_CURRENT_SOURCE_DIR}/core/FileSystem.hpp
${CMAKE_CURRENT_SOURCE_DIR}/core/JobSystem.hpp
${CMAKE_CURRENT_SOURCE_DIR}/core/Logging.hpp
${CMAKE_CURRENT_SOURCE_DIR}/core/ModuleBase.hpp
${CMAKE_CURRENT_SOURCE_DIR}/core/OSEvents.hpp
${CMAKE_CURRENT_SOURCE_DIR}/core/Settings.hpp
${CMAKE_CURRENT_SOURCE_DIR}/core/Translation.hpp
${CMAKE_CURRENT_SOURCE_DIR}/core/WindowManager.hpp
${CMAKE_CURRENT_SOURCE_DIR}/core/GameLayer.hpp
)

# ---------- System Abstraction ---------- 
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/system)

# ---------- Audition  ---------- 
if(CITRUS_INCLUDE_AUDITION)
set(ENGINE_SOURCE_FILES_AUDITION
${CMAKE_CURRENT_SOURCE_DIR}/audition/HotReloadDetection.cpp
${CMAKE_CURRENT_SOURCE_DIR}/audition/AuditionEditor.cpp
${CMAKE_CURRENT_SOURCE_DIR}/audition/AssetCompilerBootstrap.cpp
${CMAKE_CURRENT_SOURCE_DIR}/audition/LiveSync.cpp
${CMAKE_CURRENT_SOURCE_DIR}/audition/LiveSyncSocket.cpp
${CMAKE_CURRENT_SOURCE_DIR}/audition/spaces/AssetBrowser.cpp
${CMAKE_CURRENT_SOURCE_DIR}/audition/spaces/AssetActions.cpp
${CMAKE_CURRENT_SOURCE_DIR}/audition/spaces/Compiler.cpp
${CMAKE_CURRENT_SOURCE_DIR}/audition/spaces/ModuleInspector.cpp
${CMAKE_CURRENT_SOURCE_DIR}/audition/spaces/HexViewer.cpp
${CMAKE_CURRENT_SOURCE_DIR}/audition/spaces/asseteditors/AssetEditorBase.cpp
${CMAKE_CURRENT_SOURCE_DIR}/audition/spaces/asseteditors/ModelImport.cpp
${CMAKE_CURRENT_SOURCE_DIR}/audition/spaces/asseteditors/TextureImport.cpp
${CMAKE_CURRENT_SOURCE_DIR}/audition/spaces/asseteditors/TextEditor.cpp
)

set(ENGINE_HEADER_FILES_AUDITION
${CMAKE_CURRENT_SOURCE_DIR}/audition/HotReloadDetection.hpp
${CMAKE_CURRENT_SOURCE_DIR}/audition/AuditionEditor.hpp
${CMAKE_CURRENT_SOURCE_DIR}/audition/AssetCompilerBootstrap.hpp
${CMAKE_CURRENT_SOURCE_DIR}/audition/LiveSync.hpp
${CMAKE_CURRENT_SOURCE_DIR}/audition/spaces/AssetBrowser.hpp
${CMAKE_CURRENT_SOURCE_DIR}/audition/spaces/AssetActions.hpp
${CMAKE_CURRENT_SOURCE_DIR}/audition/spaces/Compiler.hpp
${CMAKE_CURRENT_SOURCE_DIR}/audition/spaces/ModuleInspector.hpp
${CMAKE_CURRENT_SOURCE_DIR}/audition/spaces/HexViewer.hpp
${CMAKE_CURRENT_SOURCE_DIR}/audition/spaces/SpaceBase.hpp
${CMAKE_CURRENT_SOURCE_DIR}/audition/spaces/asseteditors/AssetEditorBase.hpp
${CMAKE_CURRENT_SOURCE_DIR}/audition/spaces/asseteditors/ModelImport.hpp
${CMAKE_CURRENT_SOURCE_DIR}/audition/spaces/asseteditors/TextureImport.hpp
${CMAKE_CURRENT_SOURCE_DIR}/audition/spaces/asseteditors/TextEditor.hpp
)
endif()

# ---------- Animation System ----------
set(ENGINE_SOURCE_FILES_ANIMATION
${CMAKE_CURRENT_SOURCE_DIR}/animation/AnimationSystem.cpp
${CMAKE_CURRENT_SOURCE_DIR}/animation/Bank.cpp
${CMAKE_CURRENT_SOURCE_DIR}/animation/Canvas.cpp
${CMAKE_CURRENT_SOURCE_DIR}/animation/CustomPropSet.cpp
${CMAKE_CURRENT_SOURCE_DIR}/animation/LayerBase.cpp
${CMAKE_CURRENT_SOURCE_DIR}/animation/LayerClip.cpp
${CMAKE_CURRENT_SOURCE_DIR}/animation/MorphSet.cpp
${CMAKE_CURRENT_SOURCE_DIR}/animation/Skeleton.cpp
${CMAKE_CURRENT_SOURCE_DIR}/animation/Spline.cpp
)

set(ENGINE_HEADER_FILES_ANIMATION
${CMAKE_CURRENT_SOURCE_DIR}/animation/AnimationSystem.hpp
${CMAKE_CURRENT_SOURCE_DIR}/animation/Bank.hpp
${CMAKE_CURRENT_SOURCE_DIR}/animation/Canvas.hpp
${CMAKE_CURRENT_SOURCE_DIR}/animation/CustomPropSet.hpp
${CMAKE_CURRENT_SOURCE_DIR}/animation/LayerBase.hpp
${CMAKE_CURRENT_SOURCE_DIR}/animation/LayerClip.hpp
${CMAKE_CURRENT_SOURCE_DIR}/animation/MorphSet.hpp
${CMAKE_CURRENT_SOURCE_DIR}/animation/Skeleton.hpp
${CMAKE_CURRENT_SOURCE_DIR}/animation/Spline.hpp
)

# ---------- File Formats  ---------- 
set(ENGINE_SOURCE_FILES_FORMAT
${CMAKE_CURRENT_SOURCE_DIR}/formats/wad/WADCore.c
${CMAKE_CURRENT_SOURCE_DIR}/formats/texture/TextureLoad.cpp
${CMAKE_CURRENT_SOURCE_DIR}/formats/package/CitrusPackage.cpp
${CMAKE_CURRENT_SOURCE_DIR}/formats/mo/MO.cpp
${CMAKE_CURRENT_SOURCE_DIR}/formats/model/Model.cpp
)

set(ENGINE_HEADER_FILES_FORMAT
${CMAKE_CURRENT_SOURCE_DIR}/formats/wad/WADCore.h
${CMAKE_CURRENT_SOURCE_DIR}/formats/wad/WADCore.h
${CMAKE_CURRENT_SOURCE_DIR}/formats/wad/prototypes/Header.h
${CMAKE_CURRENT_SOURCE_DIR}/formats/wad/prototypes/MarkersAndBlobs.h
${CMAKE_CURRENT_SOURCE_DIR}/formats/texture/TextureLoad.h
${CMAKE_CURRENT_SOURCE_DIR}/formats/package/CitrusPackage.h
${CMAKE_CURRENT_SOURCE_DIR}/formats/mo/MO.h
${CMAKE_CURRENT_SOURCE_DIR}/formats/model/Model.hpp
)

# ---------- Renderer Frontend ---------- 
set(ENGINE_SOURCE_FILES_RENDERER
${CMAKE_CURRENT_SOURCE_DIR}/renderer/KeyLimeRenderer.cpp
)

set(ENGINE_HEADER_FILES_RENDERER
${CMAKE_CURRENT_SOURCE_DIR}/renderer/KeyLimeRenderer.hpp
)

# ---------- Graphics Backend Files ----------
set(ENGINE_HEADER_FILES_GPU_INTERFACE
${CMAKE_CURRENT_SOURCE_DIR}/gpu/Architect.h
${CMAKE_CURRENT_SOURCE_DIR}/gpu/Bindless.h
${CMAKE_CURRENT_SOURCE_DIR}/gpu/Buffer.h
${CMAKE_CURRENT_SOURCE_DIR}/gpu/Commands.h
${CMAKE_CURRENT_SOURCE_DIR}/gpu/Device.h
${CMAKE_CURRENT_SOURCE_DIR}/gpu/DebugUI.h
${CMAKE_CURRENT_SOURCE_DIR}/gpu/Pipeline.h
${CMAKE_CURRENT_SOURCE_DIR}/gpu/Present.h
${CMAKE_CURRENT_SOURCE_DIR}/gpu/Texture.h
${CMAKE_CURRENT_SOURCE_DIR}/gpu/Struct.h)
set(ENGINE_SOURCE_FILES_GPU_INTERFACE)

if(CITRUS_GPU_NULL)
set(ENGINE_SOURCE_FILES_GPU_BACKEND
${CMAKE_CURRENT_SOURCE_DIR}/gpu/null/DeviceNull.cpp)
set(ENGINE_HEADER_FILES_GPU_BACKEND)
endif()

if(CITRUS_GPU_VULKAN)
set(ENGINE_SOURCE_FILES_GPU_BACKEND
${CMAKE_CURRENT_SOURCE_DIR}/gpu/vulkan/DeviceVulkan.cpp
${CMAKE_CURRENT_SOURCE_DIR}/gpu/vulkan/DebugUIVulkan.cpp
${CMAKE_CURRENT_SOURCE_DIR}/gpu/vulkan/PipelineVulkan.cpp
${CMAKE_CURRENT_SOURCE_DIR}/gpu/vulkan/PresentVulkan.cpp
${CMAKE_CURRENT_SOURCE_DIR}/gpu/vulkan/ArchitectVulkan.cpp
${CMAKE_CURRENT_SOURCE_DIR}/gpu/vulkan/BindlessVulkan.cpp
${CMAKE_CURRENT_SOURCE_DIR}/gpu/vulkan/CommandsVulkan.cpp
${CMAKE_CURRENT_SOURCE_DIR}/gpu/vulkan/TextureVulkan.cpp
${CMAKE_CURRENT_SOURCE_DIR}/gpu/vulkan/BufferVulkan.cpp
${CMAKE_CURRENT_SOURCE_DIR}/gpu/vulkan/StructVulkan.cpp)

set(ENGINE_HEADER_FILES_GPU_BACKEND
${CMAKE_CURRENT_SOURCE_DIR}/gpu/vulkan/DeviceVulkan.hpp
${CMAKE_CURRENT_SOURCE_DIR}/gpu/vulkan/PipelineVulkan.hpp
${CMAKE_CURRENT_SOURCE_DIR}/gpu/vulkan/PresentVulkan.hpp
${CMAKE_CURRENT_SOURCE_DIR}/gpu/vulkan/ArchitectVulkan.hpp
${CMAKE_CURRENT_SOURCE_DIR}/gpu/vulkan/BindlessVulkan.hpp
${CMAKE_CURRENT_SOURCE_DIR}/gpu/vulkan/CommandsVulkan.hpp
${CMAKE_CURRENT_SOURCE_DIR}/gpu/vulkan/TextureVulkan.hpp
${CMAKE_CURRENT_SOURCE_DIR}/gpu/vulkan/BufferVulkan.hpp
${CMAKE_CURRENT_SOURCE_DIR}/gpu/vulkan/StructVulkan.hpp)
endif()

list(APPEND ENGINE_SOURCE_FILES_GPU_BACKEND
${CMAKE_CURRENT_SOURCE_DIR}/gpu/shared/ArchitectGraphBuilder.cpp
${CMAKE_CURRENT_SOURCE_DIR}/gpu/shared/ExternalLoading.cpp)
list(APPEND ENGINE_HEADER_FILES_GPU_BACKEND
${CMAKE_CURRENT_SOURCE_DIR}/gpu/shared/DeviceBase.hpp
${CMAKE_CURRENT_SOURCE_DIR}/gpu/shared/ArchitectGraphBuilder.hpp
${CMAKE_CURRENT_SOURCE_DIR}/gpu/shared/ExternalLoading.hpp)

# ---------- Physics Backend Files ----------
set(ENGINE_HEADER_FILES_PHYSICS_INTERFACE
${CMAKE_CURRENT_SOURCE_DIR}/physics/Archive.hpp
${CMAKE_CURRENT_SOURCE_DIR}/physics/Baking.hpp
${CMAKE_CURRENT_SOURCE_DIR}/physics/Body.hpp
${CMAKE_CURRENT_SOURCE_DIR}/physics/Character.hpp
${CMAKE_CURRENT_SOURCE_DIR}/physics/Constraint.hpp
${CMAKE_CURRENT_SOURCE_DIR}/physics/Module.hpp
${CMAKE_CURRENT_SOURCE_DIR}/physics/Physics.hpp
${CMAKE_CURRENT_SOURCE_DIR}/physics/Query.hpp
${CMAKE_CURRENT_SOURCE_DIR}/physics/Ragdoll.hpp
${CMAKE_CURRENT_SOURCE_DIR}/physics/Shape.hpp
${CMAKE_CURRENT_SOURCE_DIR}/physics/Surface.hpp)
set(ENGINE_SOURCE_FILES_PHYSICS_INTERFACE)

if(CITRUS_JOLT)
set(ENGINE_SOURCE_FILES_PHYSICS_BACKEND
${CMAKE_CURRENT_SOURCE_DIR}/physics/jolt/JoltArchive.cpp
${CMAKE_CURRENT_SOURCE_DIR}/physics/jolt/JoltBaking.cpp
${CMAKE_CURRENT_SOURCE_DIR}/physics/jolt/JoltBody.cpp
${CMAKE_CURRENT_SOURCE_DIR}/physics/jolt/JoltCharacter.cpp
${CMAKE_CURRENT_SOURCE_DIR}/physics/jolt/JoltConstraint.cpp
${CMAKE_CURRENT_SOURCE_DIR}/physics/jolt/JoltContext.cpp
${CMAKE_CURRENT_SOURCE_DIR}/physics/jolt/JoltDebugRender.cpp
${CMAKE_CURRENT_SOURCE_DIR}/physics/jolt/JoltQuery.cpp
${CMAKE_CURRENT_SOURCE_DIR}/physics/jolt/JoltRagdoll.cpp
${CMAKE_CURRENT_SOURCE_DIR}/physics/jolt/JoltShape.cpp)

set(ENGINE_HEADER_FILES_PHYSICS_BACKEND
${CMAKE_CURRENT_SOURCE_DIR}/physics/jolt/JoltArchive.hpp
${CMAKE_CURRENT_SOURCE_DIR}/physics/jolt/JoltBaking.hpp
${CMAKE_CURRENT_SOURCE_DIR}/physics/jolt/JoltBody.hpp
${CMAKE_CURRENT_SOURCE_DIR}/physics/jolt/JoltCharacter.hpp
${CMAKE_CURRENT_SOURCE_DIR}/physics/jolt/JoltConstraint.hpp
${CMAKE_CURRENT_SOURCE_DIR}/physics/jolt/JoltContext.hpp
${CMAKE_CURRENT_SOURCE_DIR}/physics/jolt/JoltDebugRender.hpp
${CMAKE_CURRENT_SOURCE_DIR}/physics/jolt/JoltQuery.hpp
${CMAKE_CURRENT_SOURCE_DIR}/physics/jolt/JoltRagdoll.hpp
${CMAKE_CURRENT_SOURCE_DIR}/physics/jolt/JoltShape.hpp)
endif()

list(APPEND ENGINE_SOURCE_FILES_GPU_BACKEND
${CMAKE_CURRENT_SOURCE_DIR}/physics/shared/Module.cpp
${CMAKE_CURRENT_SOURCE_DIR}/physics/shared/Surface.cpp
)

list(APPEND ENGINE_HEADER_FILES_PHYSICS_BACKEND
${CMAKE_CURRENT_SOURCE_DIR}/physics/shared/Surface.hpp
)

# ---------- Scene Engine ---------- 
set(ENGINE_SOURCE_FILES_SCENE
${CMAKE_CURRENT_SOURCE_DIR}/scene/SceneEngine.cpp
)

set(ENGINE_HEADER_FILES_SCENE
${CMAKE_CURRENT_SOURCE_DIR}/scene/SceneEngine.hpp
)

# ---------- Third Party Implementation ---------- 
set(THIRD_PARTY_IMPL_FILES
${THIRD_PARTY_DIRECTORY}/imgui/imgui_demo.cpp
${THIRD_PARTY_DIRECTORY}/imgui/imgui_draw.cpp
${THIRD_PARTY_DIRECTORY}/imgui/imgui_tables.cpp
${THIRD_PARTY_DIRECTORY}/imgui/imgui_widgets.cpp
${THIRD_PARTY_DIRECTORY}/imgui/imgui.cpp
${THIRD_PARTY_DIRECTORY}/imnodes/imnodes.cpp
${THIRD_PARTY_DIRECTORY}/im3d/im3d.cpp
${THIRD_PARTY_DIRECTORY}/lz4/lz4.c
${THIRD_PARTY_DIRECTORY}/lz4/lz4frame.c
)

if(CITRUS_GFX_VULKAN)
list(APPEND THIRD_PARTY_IMPL_FILES ${THIRD_PARTY_DIRECTORY}/imgui/backends/imgui_impl_vulkan.cpp)
endif()
if(CITRUS_SDL)
list(APPEND THIRD_PARTY_IMPL_FILES ${THIRD_PARTY_DIRECTORY}/imgui/backends/imgui_impl_sdl.cpp)
endif()

# ---------- Middleware Integration ---------- 
set(ENGINE_SOURCE_FILES_MIDDLEWARE
${CMAKE_CURRENT_SOURCE_DIR}/middleware/Im3dIntegration.cpp
${CMAKE_CURRENT_SOURCE_DIR}/middleware/ImguiIntegration.cpp
)

set(ENGINE_HEADER_FILES_MIDDLEWARE
${CMAKE_CURRENT_SOURCE_DIR}/middleware/Im3dIntegration.hpp
${CMAKE_CURRENT_SOURCE_DIR}/middleware/ImguiIntegration.hpp
)

# ---------- Scripting Integration ---------- 
if(CITRUS_LUA)
set(ENGINE_SOURCE_FILES_SCRIPTING
${CMAKE_CURRENT_SOURCE_DIR}/scripting/lua/LuaScript.cpp
)

set(ENGINE_HEADER_FILES_SCRIPTING
${CMAKE_CURRENT_SOURCE_DIR}/scripting/lua/LuaScript.hpp
)
endif()

# ---------- Interaction Engine ----------
set(ENGINE_SOURCE_FILES_INTERACT
	${CMAKE_CURRENT_SOURCE_DIR}/interact/InteractionEngine.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/interact/DeviceBackendLayer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/interact/backends/BackendManifest.cpp)

set(ENGINE_HEADER_FILES_INTERACT
	${CMAKE_CURRENT_SOURCE_DIR}/interact/InteractionEngine.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/interact/DeviceBackendLayer.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/interact/backends/BackendManifest.hpp)

# ---------- Interaction Engine All Active Backends ----------
if(CITRUS_INTERACT_INPUT_SDL)
list(APPEND ENGINE_SOURCE_FILES_INTERACT
	${CMAKE_CURRENT_SOURCE_DIR}/interact/backends/sdl/SDLGamepad.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/interact/backends/sdl/SDLKeyboardMouse.cpp)

list(APPEND ENGINE_HEADER_FILES_INTERACT
	${CMAKE_CURRENT_SOURCE_DIR}/interact/backends/sdl/SDLGamepad.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/interact/backends/sdl/SDLKeyboardMouse.hpp)
endif()

# ---------- All Engine Files ----------
set(ENGINE_FILES 
${ENGINE_SOURCE_FILES_UTILITIES} ${ENGINE_HEADER_FILES_UTILITIES}
${ENGINE_SOURCE_FILES_CORE} ${ENGINE_HEADER_FILES_CORE}
${ENGINE_SOURCE_FILES_ANIMATION} ${ENGINE_HEADER_FILES_ANIMATION}
${ENGINE_SOURCE_FILES_AUDITION} ${ENGINE_HEADER_FILES_AUDITION}
${ENGINE_SOURCE_FILES_RENDERER} ${ENGINE_HEADER_FILES_RENDERER}
${ENGINE_SOURCE_FILES_GPU_INTERFACE} ${ENGINE_HEADER_FILES_GPU_INTERFACE}
${ENGINE_SOURCE_FILES_GPU_BACKEND} ${ENGINE_HEADER_FILES_GPU_BACKEND}
${ENGINE_SOURCE_FILES_PHYSICS_INTERFACE} ${ENGINE_HEADER_FILES_PHYSICS_INTERFACE}
${ENGINE_SOURCE_FILES_PHYSICS_BACKEND} ${ENGINE_HEADER_FILES_PHYSICS_BACKEND}
${ENGINE_SOURCE_FILES_INTERACT} ${ENGINE_HEADER_FILES_INTERACT}
${ENGINE_SOURCE_FILES_INTERACT_BACKEND} ${ENGINE_SOURCE_FILES_INTERACT_BACKEND}
${ENGINE_SOURCE_FILES_SCENE} ${ENGINE_HEADER_FILES_SCENE}
${ENGINE_SOURCE_FILES_MIDDLEWARE} ${ENGINE_HEADER_FILES_MIDDLEWARE}
${ENGINE_SOURCE_FILES_SCRIPTING} ${ENGINE_HEADER_FILES_SCRIPTING}
${ENGINE_SOURCE_FILES_ASSET_SYSTEM} ${ENGINE_HEADER_FILES_ASSET_SYSTEM}
${ENGINE_SOURCE_FILES_FORMAT} ${ENGINE_HEADER_FILES_FORMAT}
${ISPC_KERNEL_INPUTS})

# ---------- Generate Config Files ----------
configure_file (
${CMAKE_CURRENT_SOURCE_DIR}/utilities/Config.h.in
${CITRUS_CODEGEN_DIR}/Config.h)

# ---------- Asset System Codegen ----------
if(EXISTS ${CITRUS_CODEGEN_DIR}/DataNicknames.cpp)
else()
write_file(${CITRUS_CODEGEN_DIR}/DataNicknames.cpp "#include <stdint.h>\nconst char* ctGetDataGuidFromHash(size_t hash){return NULL;}")
endif()
add_library(data ${CITRUS_CODEGEN_DIR}/DataNicknames.cpp)

# ---------- Create Engine Library ----------
add_library(engine 
${CMAKE_CURRENT_SOURCE_DIR}/../.clang-format
${CMAKE_CURRENT_SOURCE_DIR}/../citrus.natvis
${ENGINE_FILES}
${ASSET_SYSTEM_CODEGEN}
${TRACY_FILES}
${THIRD_PARTY_IMPL_FILES}
${NATIVE_SOURCE_FILES}
${ISPC_KERNEL_OUTPUTS}
${SWIG_OUTPUTS})

# ---------- Setup Folders ----------
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ENGINE_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/../ FILES ${THIRD_PARTY_IMPL_FILES})
source_group(tracy FILES ${TRACY_FILES})
source_group(native FILES ${NATIVE_SOURCE_FILES})
source_group(generated FILES 
${CITRUS_REFLECT_OUTPUTS}
${SWIG_OUTPUTS}
${ISPC_KERNEL_OUTPUTS}
${ASSET_SYSTEM_CODEGEN})

# ---------- Include and Link ----------
target_include_directories(engine PUBLIC ${CITRUS_CODEGEN_DIR})
target_include_directories(engine PUBLIC ${CITRUS_REFLECT_OUTPUT_DIR})
target_include_directories(engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/)
target_include_directories(engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(engine PUBLIC ${TOOLS_INCLUDE})
target_link_libraries(engine data)
target_link_libraries(engine system)
if(IS_STATIC_GAME)
target_link_libraries(engine game)
endif()

if(CITRUS_SDL)
	target_link_libraries(engine ${SDL2_LIBRARIES})
	target_include_directories(engine PUBLIC ${SDL2_INCLUDE_DIRS})
endif()

if(CITRUS_GPU_VULKAN)
	target_link_libraries(engine ${Vulkan_LIBRARIES})
	target_include_directories(engine PUBLIC ${Vulkan_INCLUDE_DIRS})
endif()

if(CITRUS_LUA)
	target_link_libraries(engine ${LUA_LIBRARIES})
	target_include_directories(engine PUBLIC ${LUA_INCLUDE_DIR})
endif()

if(CITRUS_JOLT)
	target_link_libraries(engine debug ${JOLT_LIBRARY_DEBUG} optimized ${JOLT_LIBRARY_RELEASE})
	target_include_directories(engine PUBLIC ${JOLT_INCLUDE_DIR})
endif()

if(CITRUS_RECAST)
	target_link_libraries(engine ${RECAST_LIBRARIES})
	target_include_directories(engine PUBLIC ${RECAST_INCLUDE_DIR})
endif()

if(CITRUS_DETOUR)
	target_link_libraries(engine ${DETOUR_LIBRARIES})
	target_include_directories(engine PUBLIC ${DETOUR_INCLUDE_DIR})
endif()

if(CITRUS_ANGELSCRIPT)
	target_link_libraries(engine ${ANGELSCRIPT_LIBRARIES})
	target_include_directories(engine PUBLIC ${ANGELSCRIPT_INCLUDE_DIR})
endif()

# ---------- Copy Shared Libraries and Config ----------
add_custom_command(TARGET engine POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_if_different
"${CMAKE_SOURCE_DIR}/thirdparty/OpenSourceCredits.txt"
$<TARGET_FILE_DIR:engine>)
add_custom_command(TARGET engine POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_if_different
"${SHARED_LIBRARY_DIRECTORY}/data.redirect"
$<TARGET_FILE_DIR:engine>)

if(CITRUS_SDL)
	add_custom_command(TARGET engine POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
	"${SHARED_LIBRARY_DIRECTORY}/SDL2${SHARED_LIBRARY_FORMAT}"
	$<TARGET_FILE_DIR:engine>)
endif()
if(CITRUS_LUA)
	add_custom_command(TARGET engine POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
	"${SHARED_LIBRARY_DIRECTORY}/lua51${SHARED_LIBRARY_FORMAT}"
	$<TARGET_FILE_DIR:engine>)
endif()
if(CITRUS_SHADERC)
	add_custom_command(TARGET engine POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
	"${SHARED_LIBRARY_DIRECTORY}/shaderc_shared${SHARED_LIBRARY_FORMAT}"
	$<TARGET_FILE_DIR:engine>)
endif()