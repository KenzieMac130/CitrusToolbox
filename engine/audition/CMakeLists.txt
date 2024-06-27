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

# ---------- Audition  ---------- 
set(AUDITION_SOURCE_FILES
${CMAKE_CURRENT_SOURCE_DIR}/HotReloadDetection.cpp
${CMAKE_CURRENT_SOURCE_DIR}/AuditionEditor.cpp
${CMAKE_CURRENT_SOURCE_DIR}/AssetCompilerBootstrap.cpp
${CMAKE_CURRENT_SOURCE_DIR}/LiveSync.cpp
${CMAKE_CURRENT_SOURCE_DIR}/LiveSyncSocket.cpp
${CMAKE_CURRENT_SOURCE_DIR}/AssetUtilities.cpp
${CMAKE_CURRENT_SOURCE_DIR}/spaces/AssetBrowser.cpp
${CMAKE_CURRENT_SOURCE_DIR}/spaces/AssetActions.cpp
${CMAKE_CURRENT_SOURCE_DIR}/spaces/Compiler.cpp
${CMAKE_CURRENT_SOURCE_DIR}/spaces/ModuleInspector.cpp
${CMAKE_CURRENT_SOURCE_DIR}/spaces/HexViewer.cpp
${CMAKE_CURRENT_SOURCE_DIR}/spaces/asseteditors/AssetEditorBase.cpp
${CMAKE_CURRENT_SOURCE_DIR}/spaces/asseteditors/ModelImport.cpp
${CMAKE_CURRENT_SOURCE_DIR}/spaces/asseteditors/TextureImport.cpp
${CMAKE_CURRENT_SOURCE_DIR}/spaces/asseteditors/TextEditor.cpp
)

set(AUDITION_HEADER_FILES
${CMAKE_CURRENT_SOURCE_DIR}/HotReloadDetection.hpp
${CMAKE_CURRENT_SOURCE_DIR}/AuditionEditor.hpp
${CMAKE_CURRENT_SOURCE_DIR}/AssetCompilerBootstrap.hpp
${CMAKE_CURRENT_SOURCE_DIR}/LiveSync.hpp
${CMAKE_CURRENT_SOURCE_DIR}/AssetUtilities.hpp
${CMAKE_CURRENT_SOURCE_DIR}/spaces/AssetBrowser.hpp
${CMAKE_CURRENT_SOURCE_DIR}/spaces/AssetActions.hpp
${CMAKE_CURRENT_SOURCE_DIR}/spaces/Compiler.hpp
${CMAKE_CURRENT_SOURCE_DIR}/spaces/ModuleInspector.hpp
${CMAKE_CURRENT_SOURCE_DIR}/spaces/HexViewer.hpp
${CMAKE_CURRENT_SOURCE_DIR}/spaces/SpaceBase.hpp
${CMAKE_CURRENT_SOURCE_DIR}/spaces/asseteditors/AssetEditorBase.hpp
${CMAKE_CURRENT_SOURCE_DIR}/spaces/asseteditors/ModelImport.hpp
${CMAKE_CURRENT_SOURCE_DIR}/spaces/asseteditors/TextureImport.hpp
${CMAKE_CURRENT_SOURCE_DIR}/spaces/asseteditors/TextEditor.hpp
)

set(AUDITION_THIRD_PARTY
 ${THIRD_PARTY_DIRECTORY}/imguicolortextedit/TextEditor.cpp)

source_group(thirdparty FILES ${AUDITION_THIRD_PARTY})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${AUDITION_SOURCE_FILES} ${AUDITION_HEADER_FILES})
add_library(audition ${AUDITION_SOURCE_FILES} ${AUDITION_HEADER_FILES} ${AUDITION_THIRD_PARTY})
target_link_libraries(audition engine)
set_target_properties(audition PROPERTIES FOLDER "engine")