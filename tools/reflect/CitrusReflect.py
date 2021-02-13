import sys
import re
import io
#import pcpp

# ---------- Templates Typedefs ----------

# ---------- Templates JSON Write ----------
template_json_r_default = f"\
    %s.WriteJson(json);\n"

# container, object, write function
template_json_r_containers = {
}
template_json_r_types = {
    "bool": f"\
        json.WriteBool(%s);\n",
    "float": f"\
        json.WriteNumber(%s);\n",
    "double":  f"\
        json.WriteNumber(%s);\n",
    "int8_t":  f"\
        json.WriteNumber(%s);\n",
    "int16_t":  f"\
        json.WriteNumber(%s);\n",
    "int32_t":  f"\
        json.WriteNumber(%s);\n",
    "int64_t":  f"\
        json.WriteNumber(%s);\n",
    "uint8_t":  f"\
        json.WriteNumber(%s);\n",
    "uint16_t":  f"\
        json.WriteNumber(%s);\n",
    "uint32_t":  f"\
        json.WriteNumber(%s);\n",
    "uint64_t":  f"\
        json.WriteNumber((int64_t)%s);\n",
    "ctStringUtf8":  f"\
        json.WriteString(%s);\n",
}

# ---------- Templates JSON Read ----------
template_json_r_default = ""
template_json_r_types = ""

# ---------- Templates ImGui ----------
template_imgui_default = ""
template_imgui_types = ""

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
#preprocessor_io = io.StringIO()
#preprocessor = pcpp.Preprocessor()

# ---------- Find all Pragmas ----------
definitions = re.findall(r'\#pragma ct (\w+) {(.+)}[\n\s]+([<>,\w]+)[\n\s]+([\w]+)', input_contents)
print(definitions)

# ---------- Create Entries ----------
class VariableEntry:
    # Tuple
    json_write_info = None
    # Tuple
    json_read_info = None
    # Tuple
    imgui_info = None

    container_type = None
    var_type = None
    name = None

class ClassEntry:
    variable_entries = []

class_entries = []

for definition in definitions:
    if definition[0] == "class":
        class_entries.append(ClassEntry())
    elif definition[0] == "var":
        var = VariableEntry()
         #Todo: Options
        var.var_type = definition[2] #Todo: Container Type
        var.name = definition[3]
        class_entries[-1].variable_entries.append(var)

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