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

#App Test
add_executable(Test_App AppTest.cpp)
set_property(TARGET Test_App PROPERTY FOLDER "tests")
target_link_libraries(Test_App PUBLIC engine)
target_link_libraries(Test_App PUBLIC game)

#Scene Test
add_executable(Test_Scene SceneTest.cpp)
set_property(TARGET Test_Scene PROPERTY FOLDER "tests")
target_link_libraries(Test_Scene PUBLIC engine)
target_link_libraries(Test_Scene PUBLIC game)

#Physics Test
add_subdirectory(physics)