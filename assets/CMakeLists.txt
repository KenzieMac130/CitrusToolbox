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

# Configure asset build system
message("-------Configuring Asset Build System-------")
execute_process(
COMMAND python waf configure -o ${CMAKE_BINARY_DIR}/output/assets --libs ${BASE_LIBRARY_DIRECTORY}
WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
message("--------------------------------------------")

# Get core assets to add to IDE
set(ASSET_CORE_FILES
${CMAKE_CURRENT_SOURCE_DIR}/core/shaders/common.glsl
${CMAKE_CURRENT_SOURCE_DIR}/core/shaders/portability.glsl
${CMAKE_CURRENT_SOURCE_DIR}/input/manifest.json
${CMAKE_CURRENT_SOURCE_DIR}/input/devices/gamepad/default.json
${CMAKE_CURRENT_SOURCE_DIR}/input/devices/keyboard/default.json
${CMAKE_CURRENT_SOURCE_DIR}/input/devices/mouse/default.json
)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ASSET_CORE_FILES})

# Add asset build system as target
add_custom_target(assets 
COMMAND python waf build -v
SOURCES ${ASSET_CORE_FILES}
WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})