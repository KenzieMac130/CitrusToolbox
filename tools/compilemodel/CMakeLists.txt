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
${CMAKE_CURRENT_SOURCE_DIR}/CitrusModel.hpp
)

set(CITRUS_MODEL_SOURCE_FILES
${CMAKE_CURRENT_SOURCE_DIR}/CitrusModel.cpp
${CMAKE_CURRENT_SOURCE_DIR}/SkeletonExport.cpp
${CMAKE_CURRENT_SOURCE_DIR}/MeshExport.cpp
${CMAKE_CURRENT_SOURCE_DIR}/AnimationExport.cpp
${CMAKE_CURRENT_SOURCE_DIR}/MaterialExport.cpp
${CMAKE_CURRENT_SOURCE_DIR}/PhysicsExport.cpp
${CMAKE_CURRENT_SOURCE_DIR}/Viewer.cpp
${THIRD_PARTY_DIRECTORY}/meshoptimizer/allocator.cpp
${THIRD_PARTY_DIRECTORY}/meshoptimizer/clusterizer.cpp
${THIRD_PARTY_DIRECTORY}/meshoptimizer/indexcodec.cpp
${THIRD_PARTY_DIRECTORY}/meshoptimizer/indexgenerator.cpp
${THIRD_PARTY_DIRECTORY}/meshoptimizer/overdrawanalyzer.cpp
${THIRD_PARTY_DIRECTORY}/meshoptimizer/overdrawoptimizer.cpp
${THIRD_PARTY_DIRECTORY}/meshoptimizer/simplifier.cpp
${THIRD_PARTY_DIRECTORY}/meshoptimizer/spatialorder.cpp
${THIRD_PARTY_DIRECTORY}/meshoptimizer/stripifier.cpp
${THIRD_PARTY_DIRECTORY}/meshoptimizer/vcacheanalyzer.cpp
${THIRD_PARTY_DIRECTORY}/meshoptimizer/vcacheoptimizer.cpp
${THIRD_PARTY_DIRECTORY}/meshoptimizer/vertexcodec.cpp
${THIRD_PARTY_DIRECTORY}/meshoptimizer/vertexfilter.cpp
${THIRD_PARTY_DIRECTORY}/meshoptimizer/vfetchanalyzer.cpp
${THIRD_PARTY_DIRECTORY}/meshoptimizer/vfetchoptimizer.cpp
${THIRD_PARTY_DIRECTORY}/mikkt/mikktspace.c
)

add_executable(CitrusModel
${CITRUS_MODEL_HEADER_FILES}
${CITRUS_MODEL_SOURCE_FILES})
target_include_directories(CitrusModel PUBLIC ${SHADERC_INCLUDE_DIRS})
target_link_libraries(CitrusModel PUBLIC engine)
target_include_directories(engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/)