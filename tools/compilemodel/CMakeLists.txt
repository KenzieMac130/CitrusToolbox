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

set(CITRUS_MODEL_HEADER_FILES
${CMAKE_CURRENT_SOURCE_DIR}/ExportPhaseBase.hpp
${CMAKE_CURRENT_SOURCE_DIR}/AnimationExport.hpp
${CMAKE_CURRENT_SOURCE_DIR}/CurveExport.hpp
${CMAKE_CURRENT_SOURCE_DIR}/DestructionExport.hpp
${CMAKE_CURRENT_SOURCE_DIR}/LightExport.hpp
${CMAKE_CURRENT_SOURCE_DIR}/MaterialExport.hpp
${CMAKE_CURRENT_SOURCE_DIR}/MeshExport.hpp
${CMAKE_CURRENT_SOURCE_DIR}/MetadataExport.hpp
${CMAKE_CURRENT_SOURCE_DIR}/PhysicsExport.hpp
${CMAKE_CURRENT_SOURCE_DIR}/SkeletonExport.hpp
${CMAKE_CURRENT_SOURCE_DIR}/SpawnerExport.hpp
${CMAKE_CURRENT_SOURCE_DIR}/TextureSetExport.hpp
)

set(CITRUS_MODEL_SOURCE_FILES
${CMAKE_CURRENT_SOURCE_DIR}/CitrusModel.cpp
${CMAKE_CURRENT_SOURCE_DIR}/AnimationExport.cpp
${CMAKE_CURRENT_SOURCE_DIR}/CurveExport.cpp
${CMAKE_CURRENT_SOURCE_DIR}/DestructionExport.cpp
${CMAKE_CURRENT_SOURCE_DIR}/LightExport.cpp
${CMAKE_CURRENT_SOURCE_DIR}/MaterialExport.cpp
${CMAKE_CURRENT_SOURCE_DIR}/MeshExport.cpp
${CMAKE_CURRENT_SOURCE_DIR}/MetadataExport.cpp
${CMAKE_CURRENT_SOURCE_DIR}/PhysicsExport.cpp
${CMAKE_CURRENT_SOURCE_DIR}/SkeletonExport.cpp
${CMAKE_CURRENT_SOURCE_DIR}/SpawnerExport.cpp
${CMAKE_CURRENT_SOURCE_DIR}/TextureSetExport.cpp
)

add_executable(CitrusModel
${CITRUS_MODEL_HEADER_FILES}
${CITRUS_MODEL_SOURCE_FILES})
target_include_directories(CitrusModel PUBLIC ${SHADERC_INCLUDE_DIRS})
target_link_libraries(CitrusModel PUBLIC engine)
target_include_directories(engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/)