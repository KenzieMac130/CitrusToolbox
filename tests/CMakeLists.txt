enable_testing()

#Utilities Test
add_executable(UtilitiesTest UtilitiesTest.cpp)
set_property(TARGET UtilitiesTest PROPERTY FOLDER "tests")
target_link_libraries(UtilitiesTest PUBLIC engine)

#App Test
add_executable(AppTest AppTest.cpp)
set_property(TARGET AppTest PROPERTY FOLDER "tests")
target_link_libraries(AppTest PUBLIC engine)