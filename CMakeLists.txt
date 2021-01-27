# ---------- Project Config ----------
cmake_minimum_required(VERSION 3.10)
project(CitrusToolbox)

# ---------- Organization ----------
set(GLOBAL PROPERTY USE_FOLDERS ON)

# ---------- Unicode ----------
add_definitions(-DUNICODE)
add_definitions(-D_UNICODE)

#---------- No Exceptions ----------
add_definitions(-D_HAS_EXCEPTIONS=0)
string (REGEX REPLACE "/EHsc *" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHs-c-")
if(!MSVC)
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
endif()

#---------- Unhelpful Warnings ----------
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

# ---------- Binary output directory ----------
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)

# ---------- Code Generation Output Directory ----------
set(CITRUS_CODEGEN_DIR ${PROJECT_BINARY_DIR}/generated)

# ---------- Citrus Reflect Tool ----------
set(CITRUS_REFLECT_OUTPUT_DIR ${CITRUS_CODEGEN_DIR}/reflect)
set(CITRUS_REFLECT_PY ${CMAKE_CURRENT_SOURCE_DIR}/tools/reflect/CitrusReflect.py)

# ---------- Built Asset Directory ----------
set(CITRUS_ASSET_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/assets)
set(CITRUS_ASSET_BUILD_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets)

#---------- Point to Libraries ----------
if(WIN32)
	set(BASE_LIBRARY_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/libs/Win64)
	set(SHARED_LIBRARY_DIRECTORY ${BASE_LIBRARY_DIRECTORY}/Installs)
	set(SHARED_LIBRARY_FORMAT ".dll")
else()
	message(FATAL_ERROR "Unsupported platform, please configure.")
endif()

# ---------- Find ISPC ----------
set(ISPC_COMPILER_PATH ${BASE_LIBRARY_DIRECTORY}/ISPC/ispc)

# ---------- Find SDL2 ----------
find_package(SDL2 
NAMES sdl2
HINTS ${BASE_LIBRARY_DIRECTORY}/SDL2 REQUIRED)

# ---------- Find Vulkan ----------
find_package(Vulkan REQUIRED)

# ---------- Find PhysX ----------

# ---------- Tracy Profiler ---------- 
option(TRACY_DEBUG "Enable Tracy Debugger" ON)
if(TRACY_DEBUG)
	add_compile_definitions(TRACY_ENABLE)
	set(TRACY_FILES ${CMAKE_CURRENT_SOURCE_DIR}/tools/tracy/TracyClient.cpp)
	set(CITRUS_TRACY 1)
else()
	set(TRACY_FILES "")
	set(CITRUS_TRACY 0)
endif()
set(TRACY_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/tools/tracy)

# ---------- Add Subdirectories ----------
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/game)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/engine)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tests)