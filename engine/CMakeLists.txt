#Config File Options
set (CITRUS_ENGINE_VERSION_MAJOR 1)
set (CITRUS_ENGINE_VERSION_MINOR 0)
set (NE_ENGINE_VERSION_PATCH 0)

include (TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)
if(IS_BIG_ENDIAN)
	set (CITRUS_ENDIAN 0xFF)
else()
	set (CITRUS_ENDIAN 0x00)
endif()

configure_file (
	${CMAKE_CURRENT_SOURCE_DIR}/utilities/Config.h.in
	${PROJECT_BINARY_DIR}/generated/Config.h)

#Source Files
set(ENGINE_SOURCE_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/utilities/AABB.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/utilities/Color.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/utilities/Common.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/utilities/DynamicArray.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/utilities/Hash.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/utilities/HashTable.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/utilities/Math.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/utilities/Matrix.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/utilities/Quaternion.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/utilities/String.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/utilities/Vector.cpp
)

#Header Files
set(ENGINE_HEADER_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/../.clang-format
	${CMAKE_CURRENT_SOURCE_DIR}/utilities/Config.h.in
	${CMAKE_CURRENT_SOURCE_DIR}/utilities/AABB.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/utilities/Color.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/utilities/Common.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/utilities/DynamicArray.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/utilities/Hash.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/utilities/HashTable.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/utilities/Math.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/utilities/Matrix.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/utilities/Quaternion.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/utilities/String.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/utilities/Vector.hpp
)

#Create Library
add_library(engine ${ENGINE_SOURCE_FILES} ${ENGINE_HEADER_FILES})
set_property(TARGET engine PROPERTY FOLDER "engine")
target_include_directories(engine PUBLIC ${PROJECT_BINARY_DIR}/generated/)
target_include_directories(engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

#Link Libraries
#target_link_libraries()