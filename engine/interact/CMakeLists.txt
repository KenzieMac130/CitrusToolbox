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

# ---------- Interaction Engine ----------
set(INTERACT_SOURCE_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/InteractionEngine.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/DeviceBackendLayer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/backends/BackendManifest.cpp)

set(INTERACT_HEADER_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/InteractionEngine.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/DeviceBackendLayer.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/backends/BackendManifest.hpp)

# ---------- Interaction Engine All Active Backends ----------
if(CITRUS_INTERACT_INPUT_SDL)
list(APPEND INTERACT_SOURCE_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/backends/sdl/SDLGamepad.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/backends/sdl/SDLKeyboardMouse.cpp)

list(APPEND INTERACT_HEADER_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/backends/sdl/SDLGamepad.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/backends/sdl/SDLKeyboardMouse.hpp)
endif()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${INTERACT_SOURCE_FILES} ${INTERACT_HEADER_FILES})
add_library(interact ${INTERACT_SOURCE_FILES} ${INTERACT_HEADER_FILES})
target_link_libraries(interact utilities)
target_link_libraries(interact formats)
set_target_properties(interact PROPERTIES FOLDER "engine")