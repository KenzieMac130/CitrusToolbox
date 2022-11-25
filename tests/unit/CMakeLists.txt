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

enable_testing()

#Utilities Test
add_executable(Test_Units UnitTestBase.cpp AllTests.h.in
utilities/UtilitiesTest.cpp
ecs/ECSBasics.cpp
reflect/ReflectTest.cpp
)

set_property(TARGET Test_Units PROPERTY FOLDER "tests")
target_link_libraries(Test_Units PUBLIC engine)
set(CT_ALL_TESTS "\\\n")

macro(ct_add_test test_name)
string(APPEND CT_ALL_TESTS "CT_TEST_ENTRY(${test_name}) \\\n")
add_test(${test_name} Test_Units ${test_name} Test_Units)
endmacro()

# --------------- Define All Tests Here ---------------
ct_add_test(array_test)
ct_add_test(dynamic_string_test)
ct_add_test(file_path_test)
ct_add_test(bloom_filter_test)
ct_add_test(hash_table_test)
ct_add_test(noise_test)

ct_add_test(kinnow_test_routine)
ct_add_test(kinnow_test_lots_of_component_types)
ct_add_test(kinnow_add_entities)
ct_add_test(kinnow_add_components)

ct_add_test(basic_reflect)

configure_file (
${CMAKE_CURRENT_SOURCE_DIR}/AllTests.h.in
${CITRUS_CODEGEN_DIR}/AllTests.h)