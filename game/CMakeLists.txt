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

# For user convenience just add any files in this or subdirectories to project
file(GLOB_RECURSE SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*) 

if(IS_STATIC_GAME)
set(GAME_LINK_MODE STATIC)
else()
set(GAME_LINK_MODE SHARED)
endif()
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../engine)
add_library(game ${GAME_LINK_MODE} ${SRC_FILES})
target_link_libraries(game PUBLIC engine)
set_property(TARGET game PROPERTY FOLDER "game")
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRC_FILES})