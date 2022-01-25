#
#   Copyright 2021 MacKenzie Strand
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

enable_testing()

#Utilities Test
add_executable(Test_Units UnitTestBase.cpp UnitTestBase.hpp

)
set_property(TARGET Test_Units PROPERTY FOLDER "tests")
target_link_libraries(Test_Units PUBLIC engine)

#Helper Macro
macro(ct_add_test)
add_test(${arg1} Test_Units ${arg1} Test_Units)
endmacro()

#Run All Tests
ct_add_test(DynamicArray)