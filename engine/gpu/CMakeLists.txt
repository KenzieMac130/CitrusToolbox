#
#	Copyright 2024 MacKenzie Strand
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

# ---------- Graphics Backend Files ----------
set(GPU_INTERFACE_HEADER_FILES
${CMAKE_CURRENT_SOURCE_DIR}/Architect.h
${CMAKE_CURRENT_SOURCE_DIR}/Bindless.h
${CMAKE_CURRENT_SOURCE_DIR}/Buffer.h
${CMAKE_CURRENT_SOURCE_DIR}/Commands.h
${CMAKE_CURRENT_SOURCE_DIR}/Device.h
${CMAKE_CURRENT_SOURCE_DIR}/DebugUI.h
${CMAKE_CURRENT_SOURCE_DIR}/Pipeline.h
${CMAKE_CURRENT_SOURCE_DIR}/Present.h
${CMAKE_CURRENT_SOURCE_DIR}/Texture.h
${CMAKE_CURRENT_SOURCE_DIR}/Struct.h)
set(GPU_INTERFACE_SOURCE_FILES)

if(CITRUS_GPU_VULKAN)
set(GPU_BACKEND_SOURCE_FILES
${CMAKE_CURRENT_SOURCE_DIR}/vulkan/DeviceVulkan.cpp
${CMAKE_CURRENT_SOURCE_DIR}/vulkan/DebugUIVulkan.cpp
${CMAKE_CURRENT_SOURCE_DIR}/vulkan/PipelineVulkan.cpp
${CMAKE_CURRENT_SOURCE_DIR}/vulkan/PresentVulkan.cpp
${CMAKE_CURRENT_SOURCE_DIR}/vulkan/ArchitectVulkan.cpp
${CMAKE_CURRENT_SOURCE_DIR}/vulkan/BindlessVulkan.cpp
${CMAKE_CURRENT_SOURCE_DIR}/vulkan/CommandsVulkan.cpp
${CMAKE_CURRENT_SOURCE_DIR}/vulkan/TextureVulkan.cpp
${CMAKE_CURRENT_SOURCE_DIR}/vulkan/BufferVulkan.cpp
${CMAKE_CURRENT_SOURCE_DIR}/vulkan/StructVulkan.cpp)

set(GPU_BACKEND_HEADER_FILES
${CMAKE_CURRENT_SOURCE_DIR}/vulkan/DeviceVulkan.hpp
${CMAKE_CURRENT_SOURCE_DIR}/vulkan/PipelineVulkan.hpp
${CMAKE_CURRENT_SOURCE_DIR}/vulkan/PresentVulkan.hpp
${CMAKE_CURRENT_SOURCE_DIR}/vulkan/ArchitectVulkan.hpp
${CMAKE_CURRENT_SOURCE_DIR}/vulkan/BindlessVulkan.hpp
${CMAKE_CURRENT_SOURCE_DIR}/vulkan/CommandsVulkan.hpp
${CMAKE_CURRENT_SOURCE_DIR}/vulkan/TextureVulkan.hpp
${CMAKE_CURRENT_SOURCE_DIR}/vulkan/BufferVulkan.hpp
${CMAKE_CURRENT_SOURCE_DIR}/vulkan/StructVulkan.hpp)
endif()

list(APPEND GPU_BACKEND_SOURCE_FILES
${CMAKE_CURRENT_SOURCE_DIR}/shared/ArchitectGraphBuilder.cpp
${CMAKE_CURRENT_SOURCE_DIR}/shared/TextureUpload.cpp)
list(APPEND GPU_BACKEND_HEADER_FILES
${CMAKE_CURRENT_SOURCE_DIR}/shared/DeviceBase.hpp
${CMAKE_CURRENT_SOURCE_DIR}/shared/ArchitectGraphBuilder.hpp)

set(GPU_LIBRARY_FILES
${GPU_INTERFACE_HEADER_FILES}
${GPU_INTERFACE_SOURCE_FILES}
${GPU_BACKEND_SOURCE_FILES}
${GPU_BACKEND_HEADER_FILES})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${GPU_LIBRARY_FILES})
add_library(gpu ${GPU_LIBRARY_FILES})
set_target_properties(gpu PROPERTIES FOLDER "engine")

target_link_libraries(gpu utilities)
target_link_libraries(gpu formats)

if(${CITRUS_GPU_VULKAN})
	target_link_libraries(gpu ${Vulkan_LIBRARIES})
	target_include_directories(gpu PUBLIC ${Vulkan_INCLUDE_DIRS})
endif()