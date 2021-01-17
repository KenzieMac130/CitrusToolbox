enable_testing()

#Utilities Test
add_executable(UtilitiesTest UtilitiesTest.cpp)
set_property(TARGET UtilitiesTest PROPERTY FOLDER "tests")
target_link_libraries(UtilitiesTest PUBLIC engine)