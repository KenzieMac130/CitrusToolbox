#
#	Copyright 2022 MacKenzie Strand
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

#---------- Version ----------
set (CITRUS_ENGINE_VERSION_MAJOR 1)
set (CITRUS_ENGINE_VERSION_MINOR 0)
set (CITRUS_ENGINE_VERSION_PATCH 0)

# ---------- System Abstraction ---------- 
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/system)

# ---------- Utility Libraries ----------
add_subdirectory(utilities)

# ---------- File Formats  ---------- 
add_subdirectory(formats)

# ---------- GPU Abstraction  ---------- 
add_subdirectory(gpu)

# ---------- Engine Core Framework ----------
set(ENGINE_SOURCE_FILES_CORE
${CMAKE_CURRENT_SOURCE_DIR}/core/Application.cpp
${CMAKE_CURRENT_SOURCE_DIR}/core/AsyncTasks.cpp
${CMAKE_CURRENT_SOURCE_DIR}/core/EngineCore.cpp
${CMAKE_CURRENT_SOURCE_DIR}/core/FileSystem.cpp
${CMAKE_CURRENT_SOURCE_DIR}/core/JobSystem.cpp
${CMAKE_CURRENT_SOURCE_DIR}/core/Logging.cpp
${CMAKE_CURRENT_SOURCE_DIR}/core/ModuleBase.cpp
${CMAKE_CURRENT_SOURCE_DIR}/core/OSEvents.cpp
${CMAKE_CURRENT_SOURCE_DIR}/core/Settings.cpp
${CMAKE_CURRENT_SOURCE_DIR}/core/PhysicsEngine.cpp
${CMAKE_CURRENT_SOURCE_DIR}/core/Translation.cpp
${CMAKE_CURRENT_SOURCE_DIR}/core/WindowManager.cpp
${CMAKE_CURRENT_SOURCE_DIR}/core/GameLayer.cpp
)

set(ENGINE_HEADER_FILES_CORE
${CMAKE_CURRENT_SOURCE_DIR}/core/Application.hpp
${CMAKE_CURRENT_SOURCE_DIR}/core/AsyncTasks.hpp
${CMAKE_CURRENT_SOURCE_DIR}/core/EngineCore.hpp
${CMAKE_CURRENT_SOURCE_DIR}/core/FileSystem.hpp
${CMAKE_CURRENT_SOURCE_DIR}/core/JobSystem.hpp
${CMAKE_CURRENT_SOURCE_DIR}/core/Logging.hpp
${CMAKE_CURRENT_SOURCE_DIR}/core/ModuleBase.hpp
${CMAKE_CURRENT_SOURCE_DIR}/core/OSEvents.hpp
${CMAKE_CURRENT_SOURCE_DIR}/core/Settings.hpp
${CMAKE_CURRENT_SOURCE_DIR}/core/PhysicsEngine.hpp
${CMAKE_CURRENT_SOURCE_DIR}/core/Translation.hpp
${CMAKE_CURRENT_SOURCE_DIR}/core/WindowManager.hpp
${CMAKE_CURRENT_SOURCE_DIR}/core/GameLayer.hpp
)

# ---------- Audition  ---------- 
if(CITRUS_INCLUDE_AUDITION)
add_subdirectory(audition)
endif()

# ---------- Animation System ----------
add_subdirectory(animation)

# ---------- Resource Manager  ---------- 
add_subdirectory(resource)

# ---------- Renderer ---------- 
add_subdirectory(renderer)

# ---------- Physics Backend Files ----------
add_subdirectory(physics)

# ---------- Scene Engine ---------- 
add_subdirectory(scene)

# ---------- ImGUI Integration ---------- 
add_subdirectory(imgui)

# ---------- Scripting Integration ---------- 
add_subdirectory(scripting)

# ---------- Interaction Engine ----------
add_subdirectory(interact)

# ---------- All Engine Files ----------
set(ENGINE_FILES 
${ENGINE_SOURCE_FILES_CORE} ${ENGINE_HEADER_FILES_CORE}
${ENGINE_SOURCE_FILES_RENDERER} ${ENGINE_HEADER_FILES_RENDERER}
${ENGINE_SOURCE_FILES_SCENE} ${ENGINE_HEADER_FILES_SCENE}
${ENGINE_SOURCE_FILES_MIDDLEWARE} ${ENGINE_HEADER_FILES_MIDDLEWARE}
${ENGINE_SOURCE_FILES_SCRIPTING} ${ENGINE_HEADER_FILES_SCRIPTING}
${ENGINE_SOURCE_FILES_RESOURCE_MANAGER} ${ENGINE_HEADER_FILES_RESOURCE_MANAGER})

# ---------- Create Engine Library ----------
add_library(engine 
${ENGINE_FILES}
${THIRD_PARTY_IMPL_FILES}
${NATIVE_SOURCE_FILES})

# ---------- Setup Folders ----------
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ENGINE_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/../ FILES ${THIRD_PARTY_IMPL_FILES})
source_group(native FILES ${NATIVE_SOURCE_FILES})
source_group(generated FILES 
${CITRUS_REFLECT_OUTPUTS})
set_target_properties(engine PROPERTIES FOLDER "engine")

# ---------- Include and Link ----------
target_include_directories(engine PUBLIC ${CITRUS_CODEGEN_DIR})
target_include_directories(engine PUBLIC ${CITRUS_REFLECT_OUTPUT_DIR})
target_include_directories(engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/)
target_include_directories(engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(engine PUBLIC ${TOOLS_INCLUDE})
target_link_libraries(engine utilities)
target_link_libraries(engine formats)
target_link_libraries(engine gpu)
target_link_libraries(engine scripting)
target_link_libraries(engine interact)
target_link_libraries(engine imgui)
target_link_libraries(engine animation)
target_link_libraries(engine physics)
target_link_libraries(engine renderer)
target_link_libraries(engine scene)
target_link_libraries(engine resource)
if(CITRUS_INCLUDE_AUDITION)
target_link_libraries(engine audition)
endif()
if(IS_STATIC_GAME)
target_link_libraries(engine game)
endif()

# ---------- Copy Shared Libraries and Config ----------
add_custom_command(TARGET engine POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_if_different
"${CMAKE_SOURCE_DIR}/thirdparty/OpenSourceCredits.txt"
$<TARGET_FILE_DIR:engine>)
add_custom_command(TARGET engine POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_if_different
"${SHARED_LIBRARY_DIRECTORY}/data.redirect"
$<TARGET_FILE_DIR:engine>)

if(CITRUS_SDL)
	add_custom_command(TARGET engine POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
	"${SHARED_LIBRARY_DIRECTORY}/SDL2${SHARED_LIBRARY_FORMAT}"
	$<TARGET_FILE_DIR:engine>)
endif()
if(CITRUS_LUA)
	add_custom_command(TARGET engine POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
	"${SHARED_LIBRARY_DIRECTORY}/lua51${SHARED_LIBRARY_FORMAT}"
	$<TARGET_FILE_DIR:engine>)
endif()
if(CITRUS_SHADERC)
	add_custom_command(TARGET engine POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
	"${SHARED_LIBRARY_DIRECTORY}/shaderc_shared${SHARED_LIBRARY_FORMAT}"
	$<TARGET_FILE_DIR:engine>)
endif()