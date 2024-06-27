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

# ---------- ImGUI Integration ---------- 
set(IMGUI_SOURCE_FILES
${CMAKE_CURRENT_SOURCE_DIR}/Im3dIntegration.cpp
${CMAKE_CURRENT_SOURCE_DIR}/ImguiIntegration.cpp
)

set(IMGUI_HEADER_FILES
${CMAKE_CURRENT_SOURCE_DIR}/Im3dIntegration.hpp
${CMAKE_CURRENT_SOURCE_DIR}/ImguiIntegration.hpp
)

set (IMGUI_THIRD_PARTY
${THIRD_PARTY_DIRECTORY}/imgui/imgui_demo.cpp
${THIRD_PARTY_DIRECTORY}/imgui/imgui_draw.cpp
${THIRD_PARTY_DIRECTORY}/imgui/imgui_tables.cpp
${THIRD_PARTY_DIRECTORY}/imgui/imgui_widgets.cpp
${THIRD_PARTY_DIRECTORY}/imgui/imgui.cpp
${THIRD_PARTY_DIRECTORY}/imnodes/imnodes.cpp
${THIRD_PARTY_DIRECTORY}/implot/implot.cpp
${THIRD_PARTY_DIRECTORY}/implot/implot_items.cpp
${THIRD_PARTY_DIRECTORY}/implot/implot_demo.cpp
${THIRD_PARTY_DIRECTORY}/im3d/im3d.cpp)

if(CITRUS_GFX_VULKAN)
list(APPEND IMGUI_THIRD_PARTY ${THIRD_PARTY_DIRECTORY}/imgui/backends/imgui_impl_vulkan.cpp)
endif()
if(CITRUS_SDL)
list(APPEND IMGUI_THIRD_PARTY ${THIRD_PARTY_DIRECTORY}/imgui/backends/imgui_impl_sdl.cpp)
endif()

source_group(thirdparty FILES ${IMGUI_THIRD_PARTY})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${IMGUI_SOURCE_FILES} ${IMGUI_HEADER_FILES})
add_library(imgui ${IMGUI_SOURCE_FILES} ${IMGUI_HEADER_FILES} ${IMGUI_THIRD_PARTY})
set_target_properties(imgui PROPERTIES FOLDER "engine")

target_link_libraries(imgui utilities)
target_link_libraries(imgui engine)