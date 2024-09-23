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

# ---------- Utility Libraries ----------
set(UTILITIES_SOURCE_FILES
${CMAKE_CURRENT_SOURCE_DIR}/File.cpp
${CMAKE_CURRENT_SOURCE_DIR}/GUID.cpp
${CMAKE_CURRENT_SOURCE_DIR}/Hash.cpp
${CMAKE_CURRENT_SOURCE_DIR}/HandlePointer.cpp
${CMAKE_CURRENT_SOURCE_DIR}/Memory.cpp
${CMAKE_CURRENT_SOURCE_DIR}/Noise.cpp
${CMAKE_CURRENT_SOURCE_DIR}/Random.cpp
${CMAKE_CURRENT_SOURCE_DIR}/Reflect.cpp
${CMAKE_CURRENT_SOURCE_DIR}/SharedLogging.c
${CMAKE_CURRENT_SOURCE_DIR}/String.cpp
${CMAKE_CURRENT_SOURCE_DIR}/Sync.cpp
${CMAKE_CURRENT_SOURCE_DIR}/Time.cpp
${CMAKE_CURRENT_SOURCE_DIR}/Lexer.cpp
${CMAKE_CURRENT_SOURCE_DIR}/HardcodedReflection.cpp
)

set(UTILITIES_HEADER_FILES
${CMAKE_CURRENT_SOURCE_DIR}/Config.h.in
${CMAKE_CURRENT_SOURCE_DIR}/Common.h
${CMAKE_CURRENT_SOURCE_DIR}/BloomFilter.hpp
${CMAKE_CURRENT_SOURCE_DIR}/DynamicArray.hpp
${CMAKE_CURRENT_SOURCE_DIR}/File.hpp
${CMAKE_CURRENT_SOURCE_DIR}/GUID.hpp
${CMAKE_CURRENT_SOURCE_DIR}/HandleManager.hpp
${CMAKE_CURRENT_SOURCE_DIR}/HandledList.hpp
${CMAKE_CURRENT_SOURCE_DIR}/HandlePointer.hpp
${CMAKE_CURRENT_SOURCE_DIR}/Hash.hpp
${CMAKE_CURRENT_SOURCE_DIR}/HashTable.hpp
${CMAKE_CURRENT_SOURCE_DIR}/Math.hpp
${CMAKE_CURRENT_SOURCE_DIR}/Math3d.hpp
${CMAKE_CURRENT_SOURCE_DIR}/Noise.hpp
${CMAKE_CURRENT_SOURCE_DIR}/Random.hpp
${CMAKE_CURRENT_SOURCE_DIR}/ReflectForward.h
${CMAKE_CURRENT_SOURCE_DIR}/Reflect.hpp
${CMAKE_CURRENT_SOURCE_DIR}/RingBuffer.hpp
${CMAKE_CURRENT_SOURCE_DIR}/SharedLogging.h
${CMAKE_CURRENT_SOURCE_DIR}/SpacialQuery.hpp
${CMAKE_CURRENT_SOURCE_DIR}/StaticArray.hpp
${CMAKE_CURRENT_SOURCE_DIR}/String.hpp
${CMAKE_CURRENT_SOURCE_DIR}/Sync.hpp
${CMAKE_CURRENT_SOURCE_DIR}/Time.hpp
${CMAKE_CURRENT_SOURCE_DIR}/Lexer.hpp
${CMAKE_CURRENT_SOURCE_DIR}/HardcodedReflection.hpp
)

set (UTILITIES_EXTRA_FILES 
${CMAKE_CURRENT_SOURCE_DIR}/../../.clang-format
${CMAKE_CURRENT_SOURCE_DIR}/../../citrus.natvis)

set (UTILITIES_THIRD_PARTY
${THIRD_PARTY_DIRECTORY}/lz4/lz4.c
${THIRD_PARTY_DIRECTORY}/lz4/lz4frame.c)

# ---------- Generate Config Files ----------
configure_file (
${CMAKE_CURRENT_SOURCE_DIR}/Config.h.in
${CITRUS_CODEGEN_DIR}/Config.h)

add_library(utilities 
${UTILITIES_EXTRA_FILES}
${TRACY_FILES}
${UTILITIES_THIRD_PARTY}
${UTILITIES_SOURCE_FILES} ${UTILITIES_HEADER_FILES})

source_group(tracy FILES ${TRACY_FILES})
source_group(extra FILES ${UTILITIES_EXTRA_FILES})
source_group(thirdparty FILES ${UTILITIES_THIRD_PARTY})

target_include_directories(utilities PUBLIC ${CMAKE_SOURCE_DIR}/engine)
target_include_directories(utilities PUBLIC ${CITRUS_CODEGEN_DIR})
target_include_directories(utilities PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../thirdparty/)
target_include_directories(utilities PUBLIC ${TOOLS_INCLUDE})
target_link_libraries(utilities system)
set_target_properties(utilities PROPERTIES FOLDER "engine")

if(CITRUS_SDL)
	target_link_libraries(utilities ${SDL2_LIBRARIES})
	target_include_directories(utilities PUBLIC ${SDL2_INCLUDE_DIRS})
endif()