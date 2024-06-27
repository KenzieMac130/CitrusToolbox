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

# ---------- Scripting Integration ---------- 
if(CITRUS_LUA)
set(SCRIPTING_SOURCE_FILES
${CMAKE_CURRENT_SOURCE_DIR}/lua/LuaScript.cpp
)

set(SCRIPTING_HEADER_FILES
${CMAKE_CURRENT_SOURCE_DIR}/lua/LuaScript.hpp
)
endif()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SCRIPTING_SOURCE_FILES} ${SCRIPTING_HEADER_FILES})
add_library(scripting ${SCRIPTING_SOURCE_FILES} ${SCRIPTING_HEADER_FILES})
target_link_libraries(scripting utilities)
set_target_properties(scripting PROPERTIES FOLDER "engine")

if(CITRUS_LUA)
	target_link_libraries(scripting ${LUA_LIBRARIES})
	target_include_directories(scripting PUBLIC ${LUA_INCLUDE_DIR})
endif()

if(CITRUS_ANGELSCRIPT)
	target_link_libraries(scripting ${ANGELSCRIPT_LIBRARIES})
	target_include_directories(scripting PUBLIC ${ANGELSCRIPT_INCLUDE_DIR})
endif()