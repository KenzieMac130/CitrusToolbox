import sys
import re
import io
import pcpp

# ---------- Initialize ----------
input_file = open(sys.argv[1], 'rt')
output_file = open(sys.argv[2], 'wt')

input_contents = input_file.read()
output_contents = ""

output_comments = ""
output_json_read = ""
output_json_write = ""
output_imgui = ""

# ---------- Preprocessor ----------
preprocessor_io = io.StringIO()
preprocessor = pcpp.Preprocessor()

# ---------- Find all Pragmas ----------
defs = re.findall(r'\#pragma ct (\w+) {(.+)}[\n\s]+([*\w]+)[\n\s]+([*\w\[\]]+)', input_contents)
print(defs)

# ---------- Go through  ----------

# ---------- Comments ----------
output_comments = "/* DO NOT MANUALLY EDIT!!! */"

# ---------- JSON Read ----------

# ---------- JSON Write ----------

# ---------- Imgui ----------


# ---------- Write ----------
output_contents = "#pragma once\n"
output_contents += output_comments
output_contents += output_json_read
output_contents += output_json_write 
output_contents += output_imgui
output_file.write(output_contents)