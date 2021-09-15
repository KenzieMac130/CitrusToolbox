#
#   Copyright 2021 MacKenzie Strand
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
cmake_minimum_required(VERSION 3.10)
project(CitrusToolbox)

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

#---------- Unhelpful Warnings ----------
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

#---------- Global Pragmas ----------
set (CITRUS_ENGINE_PRAGMAS 
"#if defined(_MSC_VER)
    /* enum class nonsense */
    #pragma warning(disable: 26812)
#endif"
)

# ---------- Limits ----------
set(CT_MAX_SMALL_STRING 40)
set(CT_MAX_SPACIAL_QUERY_ENTRIES_PER_CELL 16)
set(CT_MAX_FILE_PATH_LENGTH 4096)
set(CT_MAX_LOG_LENGTH 4096)
set(CT_MAX_SETTINGS_SECTIONS 64)
set(CT_MAX_GFX_SAMPLERS 8)
set(CT_MAX_GFX_SAMPLED_IMAGES 2048)
set(CT_MAX_GFX_STORAGE_IMAGES 64)
set(CT_MAX_GFX_STORAGE_BUFFERS 16192)
set(CT_MAX_INFLIGHT_FRAMES 3)
set(CT_MAX_INTERACT_PATH_SIZE 1024)
set(CT_MAX_JOB_GROUPS 1024)

# ---------- Alignment ----------
set(CT_ALIGNMENT_CACHE 64)
set(CT_ALIGNMENT_VEC2 8)
set(CT_ALIGNMENT_VEC3 16)
set(CT_ALIGNMENT_VEC4 16)
set(CT_ALIGNMENT_QUAT 16)
set(CT_ALIGNMENT_MAT4 32)

# ---------- Collision Mitigation ----------
set(CT_HASH_RESEED_ASSET_PATH 0)

# ---------- Other Defines ----------
option(USE_STDOUT "Enable STD Out" ON)
if(USE_STDOUT)
set(CITRUS_USE_STDOUT 1)
else()
set(CITRUS_USE_STDOUT 0)
endif()
option(IS_PRODUCTION_BUILD "Production Build" OFF)

# ---------- Binary output directory ----------
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)

# ---------- Code Generation Output Directory ----------
set(CITRUS_CODEGEN_DIR ${PROJECT_BINARY_DIR}/generated)

# ---------- Citrus Reflect Tool ----------
set(CITRUS_REFLECT_OUTPUT_DIR ${CITRUS_CODEGEN_DIR}/reflect)
set(CITRUS_REFLECT_PY ${CMAKE_CURRENT_SOURCE_DIR}/tools/reflect/CitrusReflect.py)
set(FILE_2_HEADER_PY ${CMAKE_CURRENT_SOURCE_DIR}/tools/file2header/File2Header.py)

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

# ---------- Find SWIG ----------
set(SWIG_EXECUTABLE_PATH ${BASE_LIBRARY_DIRECTORY}/SWIG/swig)

# ---------- Find SDL2 ----------
set(CITRUS_SDL 1)
find_package(SDL2 
NAMES sdl2
HINTS ${BASE_LIBRARY_DIRECTORY}/SDL2 REQUIRED)

# ---------- Find Vulkan ----------
set(CITRUS_GFX_VULKAN 1)
if(CITRUS_GFX_VULKAN)
   find_package(Vulkan REQUIRED)
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

# ---------- Find PhysX ----------
set(CITRUS_PHYSX 1)
set(CITRUS_PHYSX_CHECKED 1)
set(CITRUS_PHYSX_RUNTIMECOOK 1)
if(CITRUS_PHYSX)
    set(PHYSX_INCLUDE_DIR ${BASE_LIBRARY_DIRECTORY}/PhysX/include)

    if(CITRUS_PHYSX_CHECKED)
        set(PHYSX_PROFILE_NAME checked)
    else()
        set(PHYSX_PROFILE_NAME release)
    endif()

    set(PHYSX_SHARED_LIBRARIES 
            ${BASE_LIBRARY_DIRECTORY}/PhysX/bin/${PHYSX_PROFILE_NAME}/PhysX_64${SHARED_LIBRARY_FORMAT}
            ${BASE_LIBRARY_DIRECTORY}/PhysX/bin/${PHYSX_PROFILE_NAME}/PhysXCommon_64${SHARED_LIBRARY_FORMAT}
            ${BASE_LIBRARY_DIRECTORY}/PhysX/bin/${PHYSX_PROFILE_NAME}/PhysXCooking_64${SHARED_LIBRARY_FORMAT}
            ${BASE_LIBRARY_DIRECTORY}/PhysX/bin/${PHYSX_PROFILE_NAME}/PhysXFoundation_64${SHARED_LIBRARY_FORMAT})
    set(PHYSX_LIBRARIES 
            ${BASE_LIBRARY_DIRECTORY}/PhysX/lib/${BUILD_CONFIGURATION_DIR}/${PHYSX_PROFILE_NAME}/PhysX_64.lib
            ${BASE_LIBRARY_DIRECTORY}/PhysX/lib/${BUILD_CONFIGURATION_DIR}/${PHYSX_PROFILE_NAME}/PhysXCommon_64.lib
            ${BASE_LIBRARY_DIRECTORY}/PhysX/lib/${BUILD_CONFIGURATION_DIR}/${PHYSX_PROFILE_NAME}/PhysXCooking_64.lib
            ${BASE_LIBRARY_DIRECTORY}/PhysX/lib/${BUILD_CONFIGURATION_DIR}/${PHYSX_PROFILE_NAME}/PhysXFoundation_64.lib)
    set(PHYSX_STATIC_LIBRARIES
            ${BASE_LIBRARY_DIRECTORY}/PhysX/lib/${BUILD_CONFIGURATION_DIR}/${PHYSX_PROFILE_NAME}/PhysXTask_static_64.lib
            ${BASE_LIBRARY_DIRECTORY}/PhysX/lib/${BUILD_CONFIGURATION_DIR}/${PHYSX_PROFILE_NAME}/PhysXPvdSDK_static_64.lib
            ${BASE_LIBRARY_DIRECTORY}/PhysX/lib/${BUILD_CONFIGURATION_DIR}/${PHYSX_PROFILE_NAME}/PhysXExtensions_static_64.lib
            ${BASE_LIBRARY_DIRECTORY}/PhysX/lib/${BUILD_CONFIGURATION_DIR}/${PHYSX_PROFILE_NAME}/PhysXCharacterKinematic_static_64.lib
            ${BASE_LIBRARY_DIRECTORY}/PhysX/lib/${BUILD_CONFIGURATION_DIR}/${PHYSX_PROFILE_NAME}/PhysXVehicle_static_64.lib)
endif()

# ---------- Find Lua ----------
set(CITRUS_LUA 1)
if(CITRUS_LUA)
    set(LUA_INCLUDE_DIR ${BASE_LIBRARY_DIRECTORY}/LuaJIT)
    set(LUA_LIBRARIES 
        ${BASE_LIBRARY_DIRECTORY}/LuaJIT/lua51.lib)
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
   set(CITRUS_GFX_VULKAN 0)
   set(CITRUS_GFX_NULL 1)
   set(CITRUS_INTERACT_INPUT_SDL 0)
   set(CITRUS_INTERACT_INPUT_NULL 1)
   set(CITRUS_IMGUI 0)
   set(CITRUS_IM3D 0)
else()
   set(CITRUS_HEADLESS 0)
endif()

# ---------- Static vs Dynamic ----------
option(IS_STATIC "Static only" ON)
if(IS_STATIC)
   set(CITRUS_STATIC_ONLY 1)
else()
   set(CITRUS_STATIC_ONLY 0)
endif()

# ---------- Add Subdirectories ----------
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/engine)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/game)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tests)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tools)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/assets)