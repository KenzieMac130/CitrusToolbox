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

# ---------- Animation System ----------
set(ANIMATION_SOURCE_FILES
${CMAKE_CURRENT_SOURCE_DIR}/Bank.cpp
${CMAKE_CURRENT_SOURCE_DIR}/Canvas.cpp
${CMAKE_CURRENT_SOURCE_DIR}/CustomPropSet.cpp
${CMAKE_CURRENT_SOURCE_DIR}/LayerBase.cpp
${CMAKE_CURRENT_SOURCE_DIR}/LayerClip.cpp
${CMAKE_CURRENT_SOURCE_DIR}/MorphSet.cpp
${CMAKE_CURRENT_SOURCE_DIR}/Skeleton.cpp
${CMAKE_CURRENT_SOURCE_DIR}/Spline.cpp
)

set(ANIMATION_HEADER_FILES
${CMAKE_CURRENT_SOURCE_DIR}/Bank.hpp
${CMAKE_CURRENT_SOURCE_DIR}/Canvas.hpp
${CMAKE_CURRENT_SOURCE_DIR}/CustomPropSet.hpp
${CMAKE_CURRENT_SOURCE_DIR}/LayerBase.hpp
${CMAKE_CURRENT_SOURCE_DIR}/LayerClip.hpp
${CMAKE_CURRENT_SOURCE_DIR}/MorphSet.hpp
${CMAKE_CURRENT_SOURCE_DIR}/Skeleton.hpp
${CMAKE_CURRENT_SOURCE_DIR}/Spline.hpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ANIMATION_SOURCE_FILES} ${ANIMATION_HEADER_FILES})
add_library(animation ${ANIMATION_SOURCE_FILES} ${ANIMATION_HEADER_FILES})
target_link_libraries(animation utilities)
target_link_libraries(animation formats)
set_target_properties(animation PROPERTIES FOLDER "engine")