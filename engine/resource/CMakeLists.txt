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

# ---------- Resource Manager  ---------- 
set(RESOURCE_MANAGER_SOURCE_FILES
${CMAKE_CURRENT_SOURCE_DIR}/ResourceManager.cpp
${CMAKE_CURRENT_SOURCE_DIR}/ResourceTypeBase.cpp
${CMAKE_CURRENT_SOURCE_DIR}/JSONResource.cpp
${CMAKE_CURRENT_SOURCE_DIR}/TextResource.cpp
${CMAKE_CURRENT_SOURCE_DIR}/BinaryResource.cpp
${CMAKE_CURRENT_SOURCE_DIR}/ShaderResource.cpp
${CMAKE_CURRENT_SOURCE_DIR}/TranslationResource.cpp
${CMAKE_CURRENT_SOURCE_DIR}/TextureResource.cpp
${CMAKE_CURRENT_SOURCE_DIR}/ModelResource.cpp
)

set(RESOURCE_MANAGER_HEADER_FILES
${CMAKE_CURRENT_SOURCE_DIR}/ResourceManager.hpp
${CMAKE_CURRENT_SOURCE_DIR}/ResourceTypeBase.hpp
${CMAKE_CURRENT_SOURCE_DIR}/JSONResource.hpp
${CMAKE_CURRENT_SOURCE_DIR}/TextResource.hpp
${CMAKE_CURRENT_SOURCE_DIR}/BinaryResource.hpp
${CMAKE_CURRENT_SOURCE_DIR}/ShaderResource.hpp
${CMAKE_CURRENT_SOURCE_DIR}/TranslationResource.hpp
${CMAKE_CURRENT_SOURCE_DIR}/TextureResource.hpp
${CMAKE_CURRENT_SOURCE_DIR}/ModelResource.hpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${RESOURCE_MANAGER_SOURCE_FILES} ${RESOURCE_MANAGER_HEADER_FILES})
add_library(resource ${RESOURCE_MANAGER_SOURCE_FILES} ${RESOURCE_MANAGER_HEADER_FILES})
target_link_libraries(resource utilities)
target_link_libraries(resource engine)
set_target_properties(resource PROPERTIES FOLDER "engine")