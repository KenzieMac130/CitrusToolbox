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

# ---------- Physics Backend Files ----------
set(PHYSICS_INTERFACE_HEADER_FILES
${CMAKE_CURRENT_SOURCE_DIR}/Baking.hpp
${CMAKE_CURRENT_SOURCE_DIR}/Body.hpp
${CMAKE_CURRENT_SOURCE_DIR}/Character.hpp
${CMAKE_CURRENT_SOURCE_DIR}/Constraint.hpp
${CMAKE_CURRENT_SOURCE_DIR}/Physics.hpp
${CMAKE_CURRENT_SOURCE_DIR}/Query.hpp
${CMAKE_CURRENT_SOURCE_DIR}/Ragdoll.hpp
${CMAKE_CURRENT_SOURCE_DIR}/Shape.hpp
${CMAKE_CURRENT_SOURCE_DIR}/Surface.hpp)
set(PHYSICS_INTERFACE_SOURCE_FILES)

if(CITRUS_JOLT)
set(PHYSICS_BACKEND_SOURCE_FILES
${CMAKE_CURRENT_SOURCE_DIR}/jolt/JoltBaking.cpp
${CMAKE_CURRENT_SOURCE_DIR}/jolt/JoltBody.cpp
${CMAKE_CURRENT_SOURCE_DIR}/jolt/JoltCharacter.cpp
${CMAKE_CURRENT_SOURCE_DIR}/jolt/JoltConstraint.cpp
${CMAKE_CURRENT_SOURCE_DIR}/jolt/JoltContext.cpp
${CMAKE_CURRENT_SOURCE_DIR}/jolt/JoltDebugRender.cpp
${CMAKE_CURRENT_SOURCE_DIR}/jolt/JoltQuery.cpp
${CMAKE_CURRENT_SOURCE_DIR}/jolt/JoltRagdoll.cpp
${CMAKE_CURRENT_SOURCE_DIR}/jolt/JoltShape.cpp)

set(PHYSICS_BACKEND_HEADER_FILES
${CMAKE_CURRENT_SOURCE_DIR}/jolt/JoltBaking.hpp
${CMAKE_CURRENT_SOURCE_DIR}/jolt/JoltBody.hpp
${CMAKE_CURRENT_SOURCE_DIR}/jolt/JoltCharacter.hpp
${CMAKE_CURRENT_SOURCE_DIR}/jolt/JoltConstraint.hpp
${CMAKE_CURRENT_SOURCE_DIR}/jolt/JoltContext.hpp
${CMAKE_CURRENT_SOURCE_DIR}/jolt/JoltDebugRender.hpp
${CMAKE_CURRENT_SOURCE_DIR}/jolt/JoltQuery.hpp
${CMAKE_CURRENT_SOURCE_DIR}/jolt/JoltRagdoll.hpp
${CMAKE_CURRENT_SOURCE_DIR}/jolt/JoltShape.hpp)
endif()

list(APPEND PHYSICS_BACKEND_SOURCE_FILES
${CMAKE_CURRENT_SOURCE_DIR}/shared/Surface.cpp
${CMAKE_CURRENT_SOURCE_DIR}/shared/ConvexDecompose.cpp
)

list(APPEND PHYSICS_BACKEND_HEADER_FILES
${CMAKE_CURRENT_SOURCE_DIR}/shared/Surface.hpp
)

set(PHYSICS_FILES
${PHYSICS_INTERFACE_HEADER_FILES}
${PHYSICS_INTERFACE_SOURCE_FILES}
${PHYSICS_BACKEND_SOURCE_FILES}
${PHYSICS_BACKEND_HEADER_FILES}
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${PHYSICS_FILES})
add_library(physics ${PHYSICS_FILES})
set_target_properties(physics PROPERTIES FOLDER "engine")

target_link_libraries(physics utilities)
target_link_libraries(physics formats)

if(CITRUS_JOLT)
	target_link_libraries(physics debug ${JOLT_LIBRARY_DEBUG} optimized ${JOLT_LIBRARY_RELEASE})
	target_include_directories(physics PUBLIC ${JOLT_INCLUDE_DIR})
endif()