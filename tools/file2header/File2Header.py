#
#  Copyright 2021 MacKenzie Strand
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#

import sys
import os.path

if len(sys.argv) < 4:
	print("Usage: [input] [output] [b/t](binary or text)")
	exit()

if sys.argv[3] == 'b':
    input_file = open(sys.argv[1], 'rb')
else:
    input_file = open(sys.argv[1], 'rt')
output_file = open(sys.argv[2], 'wt')

input_contents = input_file.read()
output_contents = ""

output_contents += '/* GENERATED FROM "' + sys.argv[2] +\
"""" BY FILE2HEADER
INCLUDE IN ONLY ONE IMPLIMENTATION!!!
DO NOT MANUALLY EDIT!!! */\n"""

var_name = os.path.basename(sys.argv[1]).replace('.', '_').upper()

if sys.argv[3] == 'b':
    output_contents += "unsigned char " + var_name + "[] = {"
    for byte in input_contents:
        output_contents += hex(byte) + ","
    output_contents += "};"
else:
    output_contents += "const char* " + var_name + " = "
    output_contents += '"' + input_contents + '";'

output_file.write(output_contents)
print("Wrote file: " + sys.argv[2])