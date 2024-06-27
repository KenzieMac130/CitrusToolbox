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

# ---------- File Formats  ---------- 
set(FORMAT_SOURCE_FILES
${CMAKE_CURRENT_SOURCE_DIR}/json/JSON.cpp
${CMAKE_CURRENT_SOURCE_DIR}/wad/WADCore.c
${CMAKE_CURRENT_SOURCE_DIR}/texture/TextureLoad.cpp
${CMAKE_CURRENT_SOURCE_DIR}/mo/MO.cpp
${CMAKE_CURRENT_SOURCE_DIR}/model/Model.cpp
${CMAKE_CURRENT_SOURCE_DIR}/archive/CitrusArchive.cpp
)

set(FORMAT_HEADER_FILES
${CMAKE_CURRENT_SOURCE_DIR}/json/JSON.hpp
${CMAKE_CURRENT_SOURCE_DIR}/wad/WADCore.h
${CMAKE_CURRENT_SOURCE_DIR}/texture/TextureLoad.h
${CMAKE_CURRENT_SOURCE_DIR}/mo/MO.h
${CMAKE_CURRENT_SOURCE_DIR}/model/Model.hpp
${CMAKE_CURRENT_SOURCE_DIR}/archive/CitrusArchive.hpp
)

add_library(formats 
${FORMAT_SOURCE_FILES} ${FORMAT_SOURCE_FILES})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${FORMAT_SOURCE_FILES} ${FORMAT_SOURCE_FILES})
target_link_libraries(formats utilities)
set_target_properties(formats PROPERTIES FOLDER "engine")