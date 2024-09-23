#
#   Copyright 2024 MacKenzie Strand
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
${CMAKE_CURRENT_SOURCE_DIR}/CodeGenerator.hpp
${CMAKE_CURRENT_SOURCE_DIR}/InitializeOutputs.py
)

set(CITRUS_MODEL_SOURCE_FILES
${CMAKE_CURRENT_SOURCE_DIR}/CodeGenerator.cpp
${CMAKE_CURRENT_SOURCE_DIR}/Main.cpp
)

add_executable(CitrusCodegen
${CITRUS_MODEL_HEADER_FILES}
${CITRUS_MODEL_SOURCE_FILES})
target_link_libraries(CitrusCodegen PUBLIC utilities)
set_target_properties(CitrusCodegen PROPERTIES FOLDER "tools")