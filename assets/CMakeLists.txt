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

# Get core assets to add to IDE
set(ASSET_CORE_FILES
${CMAKE_CURRENT_SOURCE_DIR}/shaders/utilities/common.ctsh
${CMAKE_CURRENT_SOURCE_DIR}/shaders/utilities/bindless.ctsh
${CMAKE_CURRENT_SOURCE_DIR}/shaders/utilities/portability.ctsh
${CMAKE_CURRENT_SOURCE_DIR}/shaders/im3d.ctsi
${CMAKE_CURRENT_SOURCE_DIR}/input/actions.json
${CMAKE_CURRENT_SOURCE_DIR}/input/gamepad.json
${CMAKE_CURRENT_SOURCE_DIR}/input/keyboard.json
${CMAKE_CURRENT_SOURCE_DIR}/input/mouse.json
${CMAKE_CURRENT_SOURCE_DIR}/configs/Audition.json
${CMAKE_CURRENT_SOURCE_DIR}/configs/DebugSystem.json
${CMAKE_CURRENT_SOURCE_DIR}/configs/Honeybell.json
${CMAKE_CURRENT_SOURCE_DIR}/configs/JobSystem.json
${CMAKE_CURRENT_SOURCE_DIR}/configs/PhysX.json
${CMAKE_CURRENT_SOURCE_DIR}/configs/Renderer.json
${CMAKE_CURRENT_SOURCE_DIR}/configs/Translation.json
${CMAKE_CURRENT_SOURCE_DIR}/configs/Window.json
${CMAKE_CURRENT_SOURCE_DIR}/configs/VulkanBackend.json
)

if(${IS_PRODUCTION_BUILD})
set(ASSET_IS_PRODUCTION True)
else()
set(ASSET_IS_PRODUCTION False)
endif()

# Configure asset build system
message("-------Configuring Asset Build System-------")
execute_process(
COMMAND python waf configure 
--platform ${ENGINE_PLATFORM_NAME}
--final ${ASSET_IS_PRODUCTION}
-o ${CMAKE_BINARY_DIR}/output/data
--libs ${BASE_LIBRARY_DIRECTORY}
--build ${CMAKE_BINARY_DIR}
WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
message("--------------------------------------------")
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ASSET_CORE_FILES})

# Add asset build system as target
add_custom_target(assets_build 
COMMAND python waf build -v
DEPENDS ${ASSET_CORE_FILES} assets_configure
SOURCES wscript ${ASSET_CORE_FILES}
WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_custom_target(assets_configure 
COMMAND python waf configure 
--platform ${ENGINE_PLATFORM_NAME}
--final ${ASSET_IS_PRODUCTION}
-o ${CMAKE_BINARY_DIR}/output/assets
--libs ${BASE_LIBRARY_DIRECTORY}
--build ${CMAKE_BINARY_DIR}
DEPENDS Gltf2Citrus
WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_custom_target(assets_clean 
COMMAND python waf distclean
WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})