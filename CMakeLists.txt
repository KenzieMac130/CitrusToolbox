cmake_minimum_required(VERSION 3.10)
project(CitrusToolbox)

#Organization
set(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/assets)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/game)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/engine)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tests)