cmake_minimum_required(VERSION 3.10)
project(CitrusToolbox)

#Organization
set(GLOBAL PROPERTY USE_FOLDERS ON)

#Unicode
add_definitions(-DUNICODE)
add_definitions(-D_UNICODE)

#No Exceptions
add_definitions (-D_HAS_EXCEPTIONS=0)
string (REGEX REPLACE "/EHsc *" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHs-c-")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")

#Binary output directory
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#Point to Libraries
if(WIN32)
	set(SHARED_LIBRARY_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/libs/Win64/SharedLibraries)
	set(SHARED_LIBRARY_FORMAT ".dll")
else()
	message(FATAL_ERROR "Unsupported platform, please configure.")
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/game)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/engine)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tests)