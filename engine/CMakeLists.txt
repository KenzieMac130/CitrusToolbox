#
#	Copyright 2021 MacKenzie Strand
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
#${CMAKE_CURRENT_SOURCE_DIR}/ispc/XXX.ispc
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

#---------- Generate Scripting Bindings ----------
set(SWIG_INPUTS
${CMAKE_CURRENT_SOURCE_DIR}/scripting/SwigWrap.i
)
set(SWIG_DEPENDS
${CMAKE_CURRENT_SOURCE_DIR}/scripting/api/HoneybellScript.hpp
)
set(SWIG_OUTPUTS
${CITRUS_CODEGEN_DIR}/bind_lua/Bind_Lua.gen.cpp
)

add_custom_command(OUTPUT ${SWIG_OUTPUTS}
COMMAND
${SWIG_EXECUTABLE_PATH} -lua -c++
-outdir ${CITRUS_CODEGEN_DIR}/bind_lua
-o \"${CITRUS_CODEGEN_DIR}/bind_lua/Bind_Lua.gen.cpp\"
\"${CMAKE_CURRENT_SOURCE_DIR}/scripting/SwigWrap.i\" 
DEPENDS
${SWIG_INPUTS}
${SWIG_DEPENDS}
${LIBRARY_INSTALL_DEPENDENCY})


#file(MAKE_DIRECTORY ${CITRUS_CODEGEN_DIR}/bind_lua)
#foreach(FILE_IN IN LISTS SWIG_INPUTS)
#get_filename_component(FILE_IN_NAME ${FILE_IN} NAME_WLE)
#get_filename_component(FILE_IN_REL_PATH ${FILE_IN} DIRECTORY)
#string(CONCAT SWIG_OUT 
#	${CITRUS_CODEGEN_DIR}/bind_lua/${FILE_IN_NAME}_Lua.gen.cpp)
#string(CONCAT SWIG_OUT_H 
#	${CITRUS_CODEGEN_DIR}/bind_lua/${FILE_IN_NAME}_Lua.gen.hpp)
#add_custom_command(OUTPUT ${SWIG_OUT}
#COMMAND
#${SWIG_EXECUTABLE_PATH} -lua
#-outdir ${CITRUS_CODEGEN_DIR}/bind_lua
#-o \""${SWIG_OUT}"\"
#\"${FILE_IN}\" 
#DEPENDS
#${FILE_IN}
#${LIBRARY_INSTALL_DEPENDENCY})
#list(APPEND SWIG_OUTPUTS ${SWIG_OUT})
#endforeach()

# ---------- Utility Libraries ----------
set(ENGINE_SOURCE_FILES_UTILITIES
${CMAKE_CURRENT_SOURCE_DIR}/utilities/File.cpp
${CMAKE_CURRENT_SOURCE_DIR}/utilities/GUID.cpp
${CMAKE_CURRENT_SOURCE_DIR}/utilities/Hash.cpp
${CMAKE_CURRENT_SOURCE_DIR}/utilities/JSON.cpp
${CMAKE_CURRENT_SOURCE_DIR}/utilities/Memory.cpp
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
${CMAKE_CURRENT_SOURCE_DIR}/utilities/Meta.hpp
${CMAKE_CURRENT_SOURCE_DIR}/utilities/Noise.hpp
${CMAKE_CURRENT_SOURCE_DIR}/utilities/Random.hpp
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
${CMAKE_CURRENT_SOURCE_DIR}/gamelayer/GameLayer.cpp
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
${CMAKE_CURRENT_SOURCE_DIR}/scene/SceneEngineBase.hpp
${CMAKE_CURRENT_SOURCE_DIR}/gamelayer/GameLayer.hpp
)

# ---------- System Abstraction ---------- 
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/system)

# ---------- Audition  ---------- 
if(CITRUS_INCLUDE_AUDITION)
set(ENGINE_SOURCE_FILES_AUDITION
${CMAKE_CURRENT_SOURCE_DIR}/audition/HotReloadDetection.cpp
)

set(ENGINE_HEADER_FILES_AUDITION
${CMAKE_CURRENT_SOURCE_DIR}/audition/HotReloadDetection.hpp
)
endif()

# ---------- File Formats  ---------- 
set(ENGINE_SOURCE_FILES_FORMAT
${CMAKE_CURRENT_SOURCE_DIR}/formats/wad/WADCore.c
${CMAKE_CURRENT_SOURCE_DIR}/formats/texture/TextureLoad.cpp
${CMAKE_CURRENT_SOURCE_DIR}/formats/package/CitrusPackage.cpp
)

set(ENGINE_HEADER_FILES_FORMAT
${CMAKE_CURRENT_SOURCE_DIR}/formats/wad/WADCore.h
${CMAKE_CURRENT_SOURCE_DIR}/formats/wad/WADCore.h
${CMAKE_CURRENT_SOURCE_DIR}/formats/wad/prototypes/Header.h
${CMAKE_CURRENT_SOURCE_DIR}/formats/wad/prototypes/Bones.h
${CMAKE_CURRENT_SOURCE_DIR}/formats/wad/prototypes/Camera.h
${CMAKE_CURRENT_SOURCE_DIR}/formats/wad/prototypes/Light.h
${CMAKE_CURRENT_SOURCE_DIR}/formats/wad/prototypes/Material.h
${CMAKE_CURRENT_SOURCE_DIR}/formats/wad/prototypes/RenderMesh.h
${CMAKE_CURRENT_SOURCE_DIR}/formats/wad/prototypes/Spawner.h
${CMAKE_CURRENT_SOURCE_DIR}/formats/texture/TextureLoad.h
${CMAKE_CURRENT_SOURCE_DIR}/formats/package/CitrusPackage.h
)

# ---------- Renderer Frontend ---------- 
set(ENGINE_SOURCE_FILES_RENDERER
${CMAKE_CURRENT_SOURCE_DIR}/renderer/KeyLimeShared.cpp
)

set(ENGINE_HEADER_FILES_RENDERER
${CMAKE_CURRENT_SOURCE_DIR}/renderer/KeyLime.hpp
${CMAKE_CURRENT_SOURCE_DIR}/renderer/KeyLimeDataTypes.hpp
)

# ---------- Scene Engine ---------- 

set(CITRUS_SCENE_ENGINE_HEADER
${CMAKE_CURRENT_SOURCE_DIR}/scene/honeybell/HoneybellSceneEngine.hpp
)

set(ENGINE_SOURCE_FILES_SCENE
${CMAKE_CURRENT_SOURCE_DIR}/scene/honeybell/HoneybellSceneEngine.cpp
${CMAKE_CURRENT_SOURCE_DIR}/scene/honeybell/Component.cpp
${CMAKE_CURRENT_SOURCE_DIR}/scene/honeybell/Scene.cpp
${CMAKE_CURRENT_SOURCE_DIR}/scene/honeybell/Signal.cpp
${CMAKE_CURRENT_SOURCE_DIR}/scene/honeybell/Toy.cpp
${CMAKE_CURRENT_SOURCE_DIR}/scene/honeybell/BuiltinToys.cpp
${CMAKE_CURRENT_SOURCE_DIR}/scene/honeybell/components/DebugShapeComponent.cpp
${CMAKE_CURRENT_SOURCE_DIR}/scene/honeybell/components/PhysXActorComponent.cpp
${CMAKE_CURRENT_SOURCE_DIR}/scene/honeybell/components/PhysXControllerComponent.cpp
${CMAKE_CURRENT_SOURCE_DIR}/scene/honeybell/components/MeshRendererComponent.cpp
${CMAKE_CURRENT_SOURCE_DIR}/scene/honeybell/toys/TestShape.cpp
${CMAKE_CURRENT_SOURCE_DIR}/scene/honeybell/toys/GroundPlane.cpp
${CMAKE_CURRENT_SOURCE_DIR}/scene/honeybell/toys/SceneChunk.cpp
)

set(ENGINE_HEADER_FILES_SCENE
${CMAKE_CURRENT_SOURCE_DIR}/scene/honeybell/HoneybellSceneEngine.hpp
${CMAKE_CURRENT_SOURCE_DIR}/scene/honeybell/Component.hpp
${CMAKE_CURRENT_SOURCE_DIR}/scene/honeybell/Scene.hpp
${CMAKE_CURRENT_SOURCE_DIR}/scene/honeybell/Signal.hpp
${CMAKE_CURRENT_SOURCE_DIR}/scene/honeybell/Toy.hpp
${CMAKE_CURRENT_SOURCE_DIR}/scene/honeybell/components/DebugShapeComponent.hpp
${CMAKE_CURRENT_SOURCE_DIR}/scene/honeybell/components/PhysXActorComponent.hpp
${CMAKE_CURRENT_SOURCE_DIR}/scene/honeybell/components/PhysXControllerComponent.hpp
${CMAKE_CURRENT_SOURCE_DIR}/scene/honeybell/components/MeshRendererComponent.hpp
${CMAKE_CURRENT_SOURCE_DIR}/scene/honeybell/toys/TestShape.hpp
${CMAKE_CURRENT_SOURCE_DIR}/scene/honeybell/toys/GroundPlane.hpp
${CMAKE_CURRENT_SOURCE_DIR}/scene/honeybell/toys/SceneChunk.hpp
)

# ---------- Third Party Implementation ---------- 
set(THIRD_PARTY_IMPL_FILES
${THIRD_PARTY_DIRECTORY}/imgui/imgui_demo.cpp
${THIRD_PARTY_DIRECTORY}/imgui/imgui_draw.cpp
${THIRD_PARTY_DIRECTORY}/imgui/imgui_tables.cpp
${THIRD_PARTY_DIRECTORY}/imgui/imgui_widgets.cpp
${THIRD_PARTY_DIRECTORY}/imgui/imgui.cpp
${THIRD_PARTY_DIRECTORY}/im3d/im3d.cpp
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

if(CITRUS_PHYSX)
list(APPEND ENGINE_SOURCE_FILES_MIDDLEWARE
${CMAKE_CURRENT_SOURCE_DIR}/middleware/PhysXIntegration.cpp)
list(APPEND ENGINE_HEADER_FILES_MIDDLEWARE
${CMAKE_CURRENT_SOURCE_DIR}/middleware/PhysXIntegration.hpp)
endif()

# ---------- Scripting Integration ---------- 
if(CITRUS_LUA)
set(ENGINE_SOURCE_FILES_SCRIPTING
${CMAKE_CURRENT_SOURCE_DIR}/scripting/LuaScript.cpp
)

set(ENGINE_HEADER_FILES_SCRIPTING
${CMAKE_CURRENT_SOURCE_DIR}/scripting/LuaScript.hpp
${CMAKE_CURRENT_SOURCE_DIR}/scripting/ObjTypes.h
${CMAKE_CURRENT_SOURCE_DIR}/scripting/api/CommonApi.hpp
${CMAKE_CURRENT_SOURCE_DIR}/scripting/api/HoneybellScript.hpp
)
endif()

# ---------- Graphics Backend Files ----------
if(CITRUS_GFX_NULL)
set(ENGINE_SOURCE_FILES_GFX_BACKEND
${CMAKE_CURRENT_SOURCE_DIR}/renderer/null/KeyLimeAdapterNull.cpp)

set(ENGINE_HEADER_FILES_GFX_BACKEND)
endif()

if(CITRUS_GFX_VULKAN)
set(ENGINE_SOURCE_FILES_GFX_BACKEND
${CMAKE_CURRENT_SOURCE_DIR}/renderer/vulkan/KeyLimeAdapterVk.cpp
${CMAKE_CURRENT_SOURCE_DIR}/renderer/vulkan/VkKeyLimeCore.cpp
${CMAKE_CURRENT_SOURCE_DIR}/renderer/vulkan/VkBackend.cpp
${CMAKE_CURRENT_SOURCE_DIR}/renderer/vulkan/VkIm3d.cpp
${CMAKE_CURRENT_SOURCE_DIR}/renderer/vulkan/VkImgui.cpp)

set(ENGINE_HEADER_FILES_GFX_BACKEND
${CMAKE_CURRENT_SOURCE_DIR}/renderer/vulkan/VkKeyLimeCore.hpp
${CMAKE_CURRENT_SOURCE_DIR}/renderer/vulkan/VkBackend.hpp
${CMAKE_CURRENT_SOURCE_DIR}/renderer/vulkan/VkIm3d.hpp
${CMAKE_CURRENT_SOURCE_DIR}/renderer/vulkan/VkImgui.hpp)
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
${CMAKE_CURRENT_SOURCE_DIR}/ispc/IspcKernels.h
${ENGINE_SOURCE_FILES_UTILITIES} ${ENGINE_HEADER_FILES_UTILITIES}
${ENGINE_SOURCE_FILES_CORE} ${ENGINE_HEADER_FILES_CORE}
${ENGINE_SOURCE_FILES_AUDITION} ${ENGINE_HEADER_FILES_AUDITION}
${ENGINE_SOURCE_FILES_RENDERER} ${ENGINE_HEADER_FILES_RENDERER}
${ENGINE_SOURCE_FILES_INTERACT} ${ENGINE_HEADER_FILES_INTERACT}
${ENGINE_SOURCE_FILES_INTERACT_BACKEND} ${ENGINE_SOURCE_FILES_INTERACT_BACKEND}
${ENGINE_SOURCE_FILES_SCENE} ${ENGINE_HEADER_FILES_SCENE}
${ENGINE_SOURCE_FILES_GFX_BACKEND} ${ENGINE_HEADER_FILES_GFX_BACKEND}
${ENGINE_SOURCE_FILES_MIDDLEWARE} ${ENGINE_HEADER_FILES_MIDDLEWARE}
${ENGINE_SOURCE_FILES_SCRIPTING} ${ENGINE_HEADER_FILES_SCRIPTING}
${ENGINE_SOURCE_FILES_ASSET_SYSTEM} ${ENGINE_HEADER_FILES_ASSET_SYSTEM}
${ENGINE_SOURCE_FILES_FORMAT} ${ENGINE_HEADER_FILES_FORMAT}
${ISPC_KERNEL_INPUTS}
${SWIG_INPUTS})

# ----------Generate Config Files ----------
configure_file (
${CMAKE_CURRENT_SOURCE_DIR}/utilities/Config.h.in
${CITRUS_CODEGEN_DIR}/Config.h)

# ---------- Create Engine Library ----------
add_library(engine 
${CMAKE_CURRENT_SOURCE_DIR}/../.clang-format
${ENGINE_FILES}
${TRACY_FILES}
${THIRD_PARTY_IMPL_FILES}
${NATIVE_SOURCE_FILES}
${ISPC_KERNEL_OUTPUTS}
${SWIG_OUTPUTS})

# ---------- Setup Folders ----------
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ENGINE_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/../ FILES ${THIRD_PARTY_IMPL_FILES})
source_group(tracy FILES ${TRACY_FILES})
source_group(generated FILES ${CITRUS_REFLECT_OUTPUTS} ${SWIG_OUTPUTS} ${ISPC_KERNEL_OUTPUTS})
source_group(native FILES ${NATIVE_SOURCE_FILES})

# ---------- Include and Link ----------
target_include_directories(engine PUBLIC ${CITRUS_CODEGEN_DIR})
target_include_directories(engine PUBLIC ${CITRUS_REFLECT_OUTPUT_DIR})
target_include_directories(engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/)
target_include_directories(engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(engine PUBLIC ${TOOLS_INCLUDE})
target_link_libraries(engine system)

if(CITRUS_SDL)
	target_link_libraries(engine ${SDL2_LIBRARIES})
	target_include_directories(engine PUBLIC ${SDL2_INCLUDE_DIRS})
endif()

if(CITRUS_GFX_VULKAN)
	target_link_libraries(engine ${Vulkan_LIBRARIES})
	target_include_directories(engine PUBLIC ${Vulkan_INCLUDE_DIRS})
endif()

if(CITRUS_PHYSX)
	target_link_libraries(engine ${PHYSX_LIBRARIES})
	target_link_libraries(engine ${PHYSX_STATIC_LIBRARIES})
	target_include_directories(engine PUBLIC ${PHYSX_INCLUDE_DIR})
endif()

if(CITRUS_LUA)
	target_link_libraries(engine ${LUA_LIBRARIES})
	target_include_directories(engine PUBLIC ${LUA_INCLUDE_DIR})
endif()

# ---------- Copy Shared Libraries and Config ----------
add_custom_command(TARGET engine POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_if_different
"${CMAKE_SOURCE_DIR}/thirdparty/OpenSourceCredits.txt"
$<TARGET_FILE_DIR:engine>)
add_custom_command(TARGET engine POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_if_different
"${SHARED_LIBRARY_DIRECTORY}/assets.redirect"
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
if(CITRUS_PHYSX)
	foreach(LIB IN LISTS PHYSX_SHARED_LIBRARIES)
		add_custom_command(TARGET engine POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		"${LIB}"
		$<TARGET_FILE_DIR:engine>)
	endforeach()
endif()