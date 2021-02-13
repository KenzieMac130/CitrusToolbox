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

# ----------Generate Config Files ----------
configure_file (
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/Config.h.in
    ${CITRUS_CODEGEN_DIR}/Config.h)
    
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

# ---------- Compile Shaders ----------
file(MAKE_DIRECTORY ${CITRUS_ASSET_BUILD_DIR}/core/shaders/vk)

set(SHADER_DEPENDENCIES
    #${CMAKE_CURRENT_SOURCE_DIR}/renderer/shaders/YYY.hlsl
)

set(SHADER_INPUTS
    #${CMAKE_CURRENT_SOURCE_DIR}/renderer/shaders/XXX.hlsl
)

set(SHADER_OUTPUTS
    #${CITRUS_ASSET_BUILD_DIR}/core/shaders/vk/XXX.spv
)

set(SHADER_OPTIONS
    #""
)

# Add build command for each file
foreach(SHADER_IN SHADER_OUT SHADER_OPTIONS
IN ZIP_LISTS 
SHADER_INPUTS
SHADER_OUTPUTS
SHADER_OPTIONS)
file(TOUCH ${SHADER_OUT})
add_custom_command(OUTPUT ${SHADER_OUT}
                   COMMAND
                   ${Vulkan_GLSLC_EXECUTABLE} 
                   ${SHADER_IN} 
                   -o ${SHADER_OUT} 
                   ${SHADER_OPTIONS}
                   DEPENDS
                   ${SHADER_IN} 
                   ${SHADER_DEPENDENCIES} 
				   ${LIBRARY_INSTALL_DEPENDENCY})

# Transpile to other languages here
endforeach()

# ---------- Citrus Reflect ----------
set(CITRUS_REFLECT_INPUTS
    /utilities/Reflect.hpp
)
set(CITRUS_REFLECT_OUTPUTS)

# for each file in relative inputs
foreach(FILE_IN IN LISTS CITRUS_REFLECT_INPUTS)

# get the file name without path
get_filename_component(FILE_IN_NAME ${FILE_IN} NAME_WLE)
get_filename_component(FILE_IN_REL_PATH ${FILE_IN} DIRECTORY)
# create the reflection output
string(CONCAT REFLECT_OUT 
    ${CITRUS_REFLECT_OUTPUT_DIR} 
    ${FILE_IN_REL_PATH} "/"
    ${FILE_IN_NAME}
    ${REFLECT_IN} 
    ".gen.hpp")

string(CONCAT FILE_IN ${CMAKE_CURRENT_SOURCE_DIR} ${FILE_IN})
get_filename_component(REFLECT_OUT_REL_PATH ${REFLECT_OUT} DIRECTORY)
file(MAKE_DIRECTORY ${REFLECT_OUT_REL_PATH})
add_custom_command(OUTPUT ${REFLECT_OUT}
                   COMMAND
                   python ${CITRUS_REFLECT_PY} 
                   "${FILE_IN}" 
                   "${REFLECT_OUT}" 
                   DEPENDS
                   ${FILE_IN}
                   ${CITRUS_REFLECT_PY} 
				   ${LIBRARY_INSTALL_DEPENDENCY})
list(APPEND CITRUS_REFLECT_OUTPUTS ${REFLECT_OUT})
endforeach()

# ---------- Source Files ----------
set(ENGINE_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/AABB.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/Color.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/Hash.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/JSON.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/Math.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/Matrix.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/Quaternion.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/Random.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/String.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/Sync.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/Vector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/Application.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/EngineCore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/FileSystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/Logging.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/OSEvents.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/Settings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/WindowManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/renderer/Renderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/renderer/lowlevel/GfxBase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/renderer/lowlevel/vulkan/GfxCoreVk.cpp
)

# ----------Header Files ----------
set(ENGINE_HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/../.clang-format
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/Config.h.in
    ${CMAKE_CURRENT_SOURCE_DIR}/ispc/IspcKernels.h
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/AABB.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/Color.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/Common.h
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/DynamicArray.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/Hash.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/IndirectionTable.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/JSON.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/Math.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/Matrix.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/Quaternion.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/Random.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/Reflect.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/SparseArray.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/StaticArray.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/String.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/Sync.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/Vector.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/Application.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/EngineCore.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/FileSystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/Logging.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/ModuleBase.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/OSEvents.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/Settings.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/WindowManager.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/renderer/Renderer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/renderer/lowlevel/GfxBase.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/renderer/lowlevel/vulkan/GfxCoreVk.hpp
)

# ---------- Create Library ----------
add_library(engine 
    ${ENGINE_SOURCE_FILES} ${ENGINE_HEADER_FILES}
    ${ISPC_KERNEL_INPUTS} ${ISPC_KERNEL_OUTPUTS}
    ${SHADERC_SHADER_DEPENDENCIES} ${SHADERC_SHADER_INPUTS}
    ${SHADERC_SHADER_OUTPUTS}
    ${CITRUS_REFLECT_OUTPUTS}
    ${TRACY_FILES})
set_property(TARGET engine PROPERTY FOLDER "engine")
target_include_directories(engine PUBLIC ${CITRUS_CODEGEN_DIR})
target_include_directories(engine PUBLIC ${CITRUS_REFLECT_OUTPUT_DIR})
target_include_directories(engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/)
target_include_directories(engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(engine PUBLIC ${TRACY_INCLUDE})


target_link_libraries(engine ${SDL2_LIBRARIES})
target_include_directories(engine PUBLIC ${SDL2_INCLUDE_DIRS})

target_link_libraries(engine ${Vulkan_LIBRARIES})
target_include_directories(engine PUBLIC ${Vulkan_INCLUDE_DIRS})

# ---------- Copy Shared Libraries and Config ----------
add_custom_command(TARGET engine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/thirdparty/OpenSourceCredits.txt"
        $<TARGET_FILE_DIR:engine>)
add_custom_command(TARGET engine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SHARED_LIBRARY_DIRECTORY}/assets.redirect"
        $<TARGET_FILE_DIR:engine>)
add_custom_command(TARGET engine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SHARED_LIBRARY_DIRECTORY}/SDL2${SHARED_LIBRARY_FORMAT}"
        $<TARGET_FILE_DIR:engine>)