/*
   Copyright 2022 MacKenzie Strand

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include "UnitTestBase.hpp"
#define CATCH_CONFIG_MAIN
#include "acutest/acutest.h"

/* ----------------------- Tests -------------------------- */

#define CT_TEST_ENTRY(NAME)
#define CT_TESTS                                                                         \
   CT_TEST_ENTRY(dynamic_array_test)                                                     \
   CT_TEST_ENTRY(static_array_test)                                                      \
   CT_TEST_ENTRY(dynamic_string_test)                                                    \
   CT_TEST_ENTRY(file_path_test)                                                         \
   CT_TEST_ENTRY(bloom_filter_test)                                                      \
   CT_TEST_ENTRY(spacial_query_test) CT_TEST_ENTRY(hash_table_test)                      \
     CT_TEST_ENTRY(json_test) CT_TEST_ENTRY(math_3d_test)                                \
       CT_TEST_ENTRY(handled_list_test)

/* -------------------------------------------------------- */
#undef CT_TEST_ENTRY
#define CT_TEST_ENTRY(NAME) void NAME(void);
CT_TESTS

#undef CT_TEST_ENTRY
#define CT_TEST_ENTRY(NAME) {#NAME, NAME},

TEST_LIST = { CT_TESTS {NULL, NULL} };