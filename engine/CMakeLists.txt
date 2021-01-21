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
                   DEPENDS ${ISPC_IN})
endforeach()

# ---------- Compile Shaders ----------
file(MAKE_DIRECTORY ${CITRUS_ASSET_BUILD_DIR}/core)
file(MAKE_DIRECTORY ${CITRUS_ASSET_BUILD_DIR}/core/shaders)
file(MAKE_DIRECTORY ${CITRUS_ASSET_BUILD_DIR}/core/shaders/vk)

set(SHADERC_SHADER_DEPENDENCIES
	#${CMAKE_CURRENT_SOURCE_DIR}/renderer/shaders/YYY.glsl
)

set(SHADERC_SHADER_INPUTS
	#${CMAKE_CURRENT_SOURCE_DIR}/renderer/shaders/XXX.glsl
)

set(SHADERC_SHADER_OUTPUTS
	#${CITRUS_ASSET_BUILD_DIR}/core/shaders/vk/XXX.spv
)

set(SHADERC_SHADER_OPTIONS
	#""
)

# Add build command for each file
foreach(SHADER_IN SHADER_OUT SHADER_OPTIONS
IN ZIP_LISTS 
SHADERC_SHADER_INPUTS
SHADERC_SHADER_OUTPUTS
SHADERC_SHADER_OPTIONS)
file(TOUCH ${SHADER_OUT})
add_custom_command(OUTPUT ${SHADER_OUT}
                   COMMAND
				   ${Vulkan_GLSLC_EXECUTABLE} 
				   ${SHADER_IN} 
				   -o ${SHADER_OUT} 
				   ${SHADER_OPTIONS}
				   DEPENDS
				   ${SHADER_IN} 
				   ${SHADERC_SHADER_DEPENDENCIES})

# Transpile to other languages here
endforeach()

# ---------- Source Files ----------
set(ENGINE_SOURCE_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/utilities/AABB.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/utilities/Color.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/utilities/Hash.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/utilities/HashTable.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/utilities/JSONParse.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/utilities/JSONWrite.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/utilities/Math.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/utilities/Matrix.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/utilities/Quaternion.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/utilities/String.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/utilities/Sync.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/utilities/Vector.cpp
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
	${CMAKE_CURRENT_SOURCE_DIR}/utilities/HashTable.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/utilities/JSONParse.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/utilities/JSONWrite.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/utilities/Math.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/utilities/Matrix.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/utilities/Quaternion.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/utilities/StaticArray.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/utilities/String.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/utilities/Sync.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/utilities/Vector.hpp
)

# ---------- Create Library ----------
add_library(engine 
	${ENGINE_SOURCE_FILES} ${ENGINE_HEADER_FILES}
	${ISPC_KERNEL_INPUTS} ${ISPC_KERNEL_OUTPUTS}
	${SHADERC_SHADER_DEPENDENCIES} ${SHADERC_SHADER_INPUTS}
	${SHADERC_SHADER_OUTPUTS}
	${TRACY_FILES})
set_property(TARGET engine PROPERTY FOLDER "engine")
target_include_directories(engine PUBLIC ${CITRUS_CODEGEN_DIR})
target_include_directories(engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/)
target_include_directories(engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

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
        "${SHARED_LIBRARY_DIRECTORY}/data.redirect"
        $<TARGET_FILE_DIR:engine>)
add_custom_command(TARGET engine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SHARED_LIBRARY_DIRECTORY}/SDL2${SHARED_LIBRARY_FORMAT}"
        $<TARGET_FILE_DIR:engine>)