#
#   Copyright 2022 MacKenzie Strand
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#

# ---------- Project Config ----------
cmake_minimum_required(VERSION 3.15)
project(CitrusToolbox)

# ---------- Language Version ----------
set (CMAKE_CXX_STANDARD 17)

# ---------- Unicode ----------
add_definitions(-DUNICODE)
add_definitions(-D_UNICODE)

#---------- No Exceptions ----------
add_definitions(-D_HAS_EXCEPTIONS=0)
string (REGEX REPLACE "/EHsc *" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHs-c-")
if(!MSVC)
   set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
endif()

#---------- Static Runtime Library ----------
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

#---------- Unhelpful Warnings ----------
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

#---------- Config Escape Codes ----------
cmake_policy(SET CMP0005 NEW)

#---------- Global Pragmas ----------
set (CITRUS_ENGINE_PRAGMAS 
"#if defined(_MSC_VER)
    /* enum class nonsense */
    #pragma warning(disable: 26812)
    /* heap alignment to CT_ALIGNMENT_CACHE is guaranteed */
    #pragma warning(disable: 4316)
#endif"
)

# ---------- Limits ----------
set(CT_MAX_SMALL_STRING 40)
set(CT_MAX_HANDLED_BUCKET_SIZE 64)
set(CT_MAX_SPACIAL_QUERY_ENTRIES_PER_CELL 16)
set(CT_MAX_FILE_PATH_LENGTH 4096)
set(CT_MAX_LOG_LENGTH 4096)
set(CT_MAX_SETTINGS_SECTIONS 64)
set(CT_MAX_GFX_DYNAMIC_INTS 8)
set(CT_MAX_GFX_SAMPLERS 8)
set(CT_MAX_GFX_SAMPLED_IMAGES 2048)
set(CT_MAX_GFX_STORAGE_IMAGES 64)
set(CT_MAX_GFX_STORAGE_BUFFERS 16192)
set(CT_MAX_GFX_UNIFORM_BUFFERS 16192)
set(CT_MAX_INFLIGHT_FRAMES 3)
set(CT_MAX_GFX_TASK_DEPENDENCIES 64)
set(CT_MAX_GFX_TASK_BUFFERS 8)
set(CT_MAX_GFX_TASK_IMAGES 9)
set(CT_MAX_GFX_TASK_BARRIERS 4)
set(CT_MAX_INTERACT_PATH_SIZE 1024)
set(CT_MAX_JOB_GROUPS 1024)
set(CT_MAX_MIP_LEVELS 16)
set(CT_MAX_VERTEX_UV_CHANNELS 4)
set(CT_MAX_VERTEX_COLOR_CHANNELS 4)
set(CT_MAX_MESH_MATERIALS 32)
set(CT_MAX_KINNOW_REFLECT_ENUMS 128)
set(CT_MAX_KINNOW_REFLECT_STRUCTS 256)

# ---------- Alignment ----------
set(CT_ALIGNMENT_CACHE 64)
set(CT_ALIGNMENT_VEC2 8)
set(CT_ALIGNMENT_VEC3 16)
set(CT_ALIGNMENT_VEC4 16)
set(CT_ALIGNMENT_QUAT 16)
set(CT_ALIGNMENT_MAT4 32)
set(CT_ALIGNMENT_MODEL_GPU 16)

# ---------- Scene Engine ----------
set(CITRUS_SCENE_ENGINES HONEYBELL KINNOW)
set(CITRUS_SCENE_ENGINE KINNOW CACHE STRING "Scene engine")
set_property(CACHE CITRUS_SCENE_ENGINE PROPERTY STRINGS ${CITRUS_SCENE_ENGINES})

# ---------- Other Defines ----------
option(USE_STDOUT "Enable STD Out" ON)
if(USE_STDOUT)
set(CITRUS_USE_STDOUT 1)
else()
set(CITRUS_USE_STDOUT 0)
endif()
option(IS_PRODUCTION_BUILD "Production Build" OFF)
if(IS_PRODUCTION_BUILD)
set(CITRUS_IS_PRODUCTION 1)
else()
set(CITRUS_IS_PRODUCTION 0)
endif()

# ---------- Binary output directory ----------
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)

# ---------- Code Generation Output Directory ----------
set(CITRUS_CODEGEN_DIR ${PROJECT_BINARY_DIR}/generated)

# ---------- Built Asset Directory ----------
set(CITRUS_ASSET_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/assets)
set(CITRUS_ASSET_BUILD_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets)

#---------- Point to Libraries ----------
# The following should be set up per-platform in CMakePresets.json
#"BASE_LIBRARY_DIRECTORY": "${CMAKE_CURRENT_SOURCE_DIR}/libs/Win64"
#"SHARED_LIBRARY_FORMAT": ".dll"
#"EXECUTABLE_FORMAT": ".exe"
#"ASSET_BUILD_PLATFORM": "windows"
#"NATIVE_SOURCE_FILES": "path/XXX.c path/YYY.c path/ZZZ.c ..."

set(SHARED_LIBRARY_DIRECTORY ${BASE_LIBRARY_DIRECTORY}/Installs)
set(TOOLS_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/tools)
set(THIRD_PARTY_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty)

# ---------- Paths ----------
set(CT_AUDITION_PYTHON "python")
set(CT_AUDITION_WAF_PATH ${CITRUS_ASSET_SOURCE_DIR} CACHE FILEPATH "Waf root path")
set(CT_AUDITION_ASSET_PATH ${CITRUS_ASSET_SOURCE_DIR} CACHE FILEPATH "Asset path")

#---------- Build Config Feedback ----------
if (CMAKE_GENERATOR MATCHES "Visual Studio")
set(BUILD_CONFIGURATION_DIR $(Configuration))
else()
set(BUILD_CONFIGURATION_DIR ${CMAKE_BUILD_TYPE})
endif()

# ---------- Install Libraries ----------
if(NOT EXISTS ${BASE_LIBRARY_DIRECTORY})
	add_subdirectory(${TOOLS_INCLUDE}/installdeps)
endif()

# ---------- Find ISPC ----------
set(ISPC_COMPILER_PATH ${BASE_LIBRARY_DIRECTORY}/ISPC/ispc)

# ---------- Find SDL2 ----------
set(CITRUS_SDL 1)
find_package(SDL2 
NAMES sdl2
HINTS ${BASE_LIBRARY_DIRECTORY}/SDL2 REQUIRED)

# ---------- Find Vulkan ----------
set(CITRUS_GPU_VULKAN 1)
if(CITRUS_GPU_VULKAN)
   find_package(Vulkan)
   if(NOT Vulkan_FOUND)
      set(Vulkan_INCLUDE_DIRS ${BASE_LIBRARY_DIRECTORY}/Vulkan/Include)
      set(Vulkan_INCLUDE_DIR ${Vulkan_INCLUDE_DIRS}/vulkan)
      set(Vulkan_LIBRARY ${BASE_LIBRARY_DIRECTORY}/Vulkan/Lib/vulkan-1.lib)
      set(Vulkan_LIBRARIES ${Vulkan_LIBRARY})
   endif()
endif()

# ---------- Find Shaderc ----------
set(CITRUS_SHADERC 1)
if(CITRUS_SHADERC)
   set(SHADERC_INCLUDE_DIRS ${BASE_LIBRARY_DIRECTORY}/Shaderc/Include)
   set(SHADERC_LIBRARY ${BASE_LIBRARY_DIRECTORY}/Shaderc/Lib/shaderc_shared.lib)
endif()

# ---------- Find Jolt ----------
set(CITRUS_JOLT 1)
if(CITRUS_JOLT)
   set(JOLT_INCLUDE_DIR ${BASE_LIBRARY_DIRECTORY}/Jolt/include)
   set(JOLT_LIBRARY_DEBUG ${BASE_LIBRARY_DIRECTORY}/Jolt/lib/Debug/jolt.lib)
   if(IS_PRODUCTION_BUILD)
      set(JOLT_LIBRARY_RELEASE ${BASE_LIBRARY_DIRECTORY}/Jolt/lib/Distribution/jolt.lib)
   else()
      set(JOLT_LIBRARY_RELEASE ${BASE_LIBRARY_DIRECTORY}/Jolt/lib/Release/jolt.lib)
   endif()
endif()

# ---------- Find Recast ----------
set(CITRUS_RECAST 1)
if(CITRUS_RECAST)
    set(RECAST_INCLUDE_DIR ${BASE_LIBRARY_DIRECTORY}/Recast/include)
    set(RECAST_LIBRARIES 
        ${BASE_LIBRARY_DIRECTORY}/Recast/lib/Recast.lib)
endif()

# ---------- Find Detour ----------
set(CITRUS_DETOUR 1)
if(CITRUS_DETOUR)
    set(DETOUR_INCLUDE_DIR ${BASE_LIBRARY_DIRECTORY}/Detour/include)
    set(DETOUR_LIBRARIES 
        ${BASE_LIBRARY_DIRECTORY}/Detour/lib/Detour.lib
        ${BASE_LIBRARY_DIRECTORY}/Detour/lib/DetourCrowd.lib
        ${BASE_LIBRARY_DIRECTORY}/Detour/lib/DetourDebugUtils.lib
        ${BASE_LIBRARY_DIRECTORY}/Detour/lib/DetourTileCache.lib)
endif()

# ---------- Find Lua ----------
set(CITRUS_LUA 1)
if(CITRUS_LUA)
    set(LUA_INCLUDE_DIR ${BASE_LIBRARY_DIRECTORY}/LuaJIT)
    set(LUA_LIBRARIES 
        ${BASE_LIBRARY_DIRECTORY}/LuaJIT/lua51.lib)
endif()

# ---------- Find Angel Script ----------
set(CITRUS_ANGELSCRIPT 1)
if(CITRUS_ANGELSCRIPT)
    set(ANGELSCRIPT_INCLUDE_DIR ${BASE_LIBRARY_DIRECTORY}/AngelScript)
    set(ANGELSCRIPT_LIBRARIES 
        ${BASE_LIBRARY_DIRECTORY}/AngelScript/angelscript.lib)
endif()

# ---------- Tracy Profiler ---------- 
option(TRACY_DEBUG "Enable Tracy Debugger" ON)
if(TRACY_DEBUG)
   add_compile_definitions(TRACY_ENABLE)
   set(TRACY_FILES ${CMAKE_CURRENT_SOURCE_DIR}/tools/tracy/TracyClient.cpp)
   set(CITRUS_TRACY 1)
else()
   set(TRACY_FILES "")
   set(CITRUS_TRACY 0)
endif()

# ---------- Audition Live Development ----------
option(USE_AUDITION "Enable Audition Live Development" ON)
if(USE_AUDITION)
   set(CITRUS_INCLUDE_AUDITION 1)
else()
   set(CITRUS_INCLUDE_AUDITION 0)
endif()

# ---------- Interaction Backends ----------
set(CITRUS_INTERACT_INPUT_SDL 1)

# ---------- ImGui/Im3d ----------
set(CITRUS_IMGUI 1)
set(CITRUS_IM3D 1)

# ---------- Headlesss Mode ----------
option(IS_HEADLESS "Command line only" OFF)
if(IS_HEADLESS)
   set(CITRUS_HEADLESS 1)
   set(CITRUS_GPU_VULKAN 0)
   set(CITRUS_GPU_NULL 1)
   set(CITRUS_INTERACT_INPUT_SDL 0)
   set(CITRUS_INTERACT_INPUT_NULL 1)
   set(CITRUS_IMGUI 0)
   set(CITRUS_IM3D 0)
else()
   set(CITRUS_HEADLESS 0)
endif()

# ---------- Static vs Dynamic ----------
option(IS_STATIC_ENGINE "Static engine" ON)
if(IS_STATIC_ENGINE)
   set(CITRUS_STATIC_ENGINE 1)
else()
   set(CITRUS_STATIC_ENGINE 0)
endif()

option(IS_STATIC_GAME "Static game" OFF)
if(IS_STATIC_GAME)
   set(CITRUS_STATIC_GAME 1)
else()
   set(CITRUS_STATIC_GAME 0)
endif()

# ---------- Add Subdirectories ----------
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/engine)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/game)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tests)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tools)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/assets)