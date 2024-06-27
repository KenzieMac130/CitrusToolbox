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

# ---------- Scene Engine ---------- 
set(SCENE_SOURCE_FILES
${CMAKE_CURRENT_SOURCE_DIR}/SceneEngine.cpp
${CMAKE_CURRENT_SOURCE_DIR}/DebugCamera.cpp
)

set(SCENE_HEADER_FILES
${CMAKE_CURRENT_SOURCE_DIR}/SceneEngine.hpp
${CMAKE_CURRENT_SOURCE_DIR}/DebugCamera.hpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SCENE_SOURCE_FILES} ${SCENE_HEADER_FILES})
add_library(scene ${SCENE_SOURCE_FILES} ${SCENE_HEADER_FILES})
target_link_libraries(scene utilities)
target_link_libraries(scene engine)
set_target_properties(scene PROPERTIES FOLDER "engine")