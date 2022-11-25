from io import TextIOWrapper
from lib2to3.pgen2.parse import ParseError
import sys
import json
import os
import pathlib
import re
import math

# Documentation of different markups:
# CT_MODULE: applies globally to the file and determines a name group
# CT_HIDDEN: hide the element should be hidden from being edited or selected the editor (or atleast read-only)
# CT_EDITOR_READ_ONLY: do not allow the editor to edit this data
# CT_EMBED_BLOB: embed the blob of a pointer into the file and share a reference to it
# CT_COPY_BLOB: embed the blob of a pointer into the file and allocate copy of it on load
# CT_LENGTH_VAR: defines the length variable of a pointer or fixed buffer
# CT_DEFAULT: the default for a variable defined as a single value or valid json
# CT_OLD_NAME: defines an alias to use if the programmer changed a variable name (can have multiple)
# CT_COMPONENT_NAME: marks a struct as a component with this name
# CT_ENUM: use on a integer typedef to assign a enum name as a exclusive field
# CT_BITS: use on a integer typedef to assign a enum name as a bitfield
# CT_COMPONENT_GROUPS: declares a editor friendly group of components (ex: {'groupName':['comA','comB']})
# CT_LABEL: the text to be drawn for this item in the user interface
# CT_DOCUMENTATION: the tooltip to be associated with this variable in the user interface
# CT_UNITS: units to show in editor (METERS,RADIANS,SECONDS,COLOR,BLACKBODY)
# CT_MIN: minimum number allowed to be assigned in editor (if applicable)
# CT_MAX: maximum number allowed to be assigned in editor (if applicable)
# CT_SOFT_MIN: minimum number allowed to be assigned in editor without manual input (if applicable)
# CT_SOFT_MAX: maximum number allowed to be assigned in editor without manual input (if applicable)

# Auto assigned:
# CT_VALUE: value of an enumerator
# CT_TYPE: type of a variable
# CT_POINTER: specifies this variable is a pointer
# CT_CONST: specifies this variable is const
# CT_CONSTANTS: values of a enumerator
# CT_MEMBERS: members of a structure
# CT_LENGTH: length of a fixed buffer
# CT_TYPEDEFS: list of typedefs
# CT_ENUMS: list of enums
# CT_STRUCTS: list of structs
# CT_COMPONENT_GROUPS list of component groups

markup_variables = ["CT_HIDDEN",
"CT_MODULE:",
"CT_LABEL:",
"CT_DOCUMENTATION:",
"CT_DEFAULT:",
"CT_OLD_NAME:",
"CT_ENUM:",
"CT_BITS:",
"CT_COMPONENT_NAME:",
"CT_COMPONENT_GROUPS:",
"CT_UNITS:",
"CT_MIN:",
"CT_MAX:",
"CT_SOFT_MIN:",
"CT_SOFT_MAX:",
"CT_LENGTH_VAR:",
"CT_EDITOR_READ_ONLY",
"CT_EMBED_BLOB",
"CT_COPY_BLOB"]

number_variables = ["CT_MIN:","CT_MAX:","CT_SOFT_MIN:","CT_SOFT_MAX:"]

flag_variables = ["CT_HIDDEN","CT_EDITOR_READ_ONLY","CT_NO_LOAD","CT_EMBED_BLOB","CT_COPY_BLOB"]

class ParseContext:
    options = dict()
    groups = dict()
    module_name = "default"

def numberify_text(text):
    if '0x' in text:
        return int(text[2:], 16)
    elif '.' in text:
        return float(text)
    else:
        return int(text)

def is_string_float(str):
    try:
        float(str)
        return True
    except ValueError:
        return False

def jsonify_text(jsonstr):
    jsonstr = re.sub(r"([^\\])'", r'\1"', jsonstr)
    if is_string_float(jsonstr):
        jsonstr = f'[{numberify_text(jsonstr)}]'
    elif jsonstr == "true" or jsonstr == "false":
        jsonstr = f"[{jsonstr}]"
    elif jsonstr[0] != "[" and jsonstr[0] != "{":
        # todo: deconstruct and textify
        jsonstr = f'["{jsonstr}"]'
    return jsonstr

def digest_block_comment(curdict, tokenit, context):
    commentstr = ""
    while True: 
        token = next(tokenit)
        if token == "*/":
            break
        if any(token == e for e in markup_variables):
            if token == "CT_MODULE:":
                context.module_name
            elif token == "CT_COMPONENT_GROUPS:":
                try:
                    jsonstr = jsonify_text(next(tokenit))
                    jsondict = json.loads(jsonstr)
                    context.groups = context.groups | jsondict
                except ParseError:
                    raise ParseError(f"Failed to parse component groups: {jsonstr}")
            elif token == "CT_DEFAULT:":
                try:
                    jsonstr = jsonify_text(next(tokenit))
                    jsondict = json.loads(jsonstr)
                    curdict["CT_DEFAULT"] = jsondict
                except ParseError:
                    raise ParseError(f"Failed to parse default value: {jsonstr}")
            elif token == "CT_OLD_NAME:":
                if "CT_OLD_NAMES" not in curdict:
                    curdict["CT_OLD_NAMES"] = list()
                curdict["CT_OLD_NAMES"].append(next(tokenit))
            elif any(token == e for e in number_variables):
                curdict[token[0:-1]] = numberify_text(next(tokenit))
            elif any(token == e for e in flag_variables):
                curdict[token] = True
            else:
                curdict[token[0:-1]] = next(tokenit)
    return tokenit

def finish_block_for_line(curdict, tokenit, context):
    token = next(tokenit)
    while token != '\n':
        if token == "/*":
            tokenit = digest_block_comment(curdict, tokenit, context)
        token = next(tokenit)
    return tokenit

def digest_typedef_enum(curdict, tokenit, context):
    name = next(tokenit)
    curdict[name] = dict()
    curdict[name] = curdict[name] | context.options
    constdict = dict()
    while next(tokenit) != '{':
        pass
    token = next(tokenit)
    incr = 0
    while token != '}':
        if token[0].isalnum():
            vardict = dict()
            cname = token
            if next(tokenit) == "=":
                value = numberify_text(next(tokenit))
            else:
                value = incr
            incr = value + 1
            tokenit = finish_block_for_line(vardict, tokenit, context)
            vardict["CT_VALUE"] = value
            constdict[cname] = vardict.copy()

        token = next(tokenit)
    curdict[name]["CT_CONSTANTS"] = constdict.copy()      
    return finish_block_for_line(curdict[name], tokenit, context)

def digest_typedef_struct(curdict, tokenit, context):
    name = next(tokenit)
    curdict[name] = dict()
    curdict[name] = curdict[name] | context.options
    while next(tokenit) != '{':
        pass
    token = next(tokenit)

    # Digest variables
    memberdict = dict()
    vardict = dict()
    while token != '}':
        if token == "/*":
            digest_block_comment(vardict, tokenit, context)
        elif "CT_KINNOW_STRUCT" in token:
            next(tokenit)
            next(tokenit)
            next(tokenit)
            token = next(tokenit)
            continue
        elif token[0].isalnum():
            if token == "const":
                vardict["CT_CONST"] = True
                token = next(tokenit)
            aftertype = next(tokenit)
            pointerlevel = 0
            while aftertype == '*':
                aftertype = next(tokenit)
                pointerlevel = pointerlevel + 1
            if pointerlevel > 0:
                vardict["CT_POINTER"] = True
            if pointerlevel > 1:
                token = "void" # don't handle nested pointers
            varname = aftertype
            aftervname = next(tokenit)
            vardict["CT_TYPE"] = token
            if aftervname == "[":
                vardict["CT_LENGTH"] = next(tokenit)
            tokenit = finish_block_for_line(vardict, tokenit, context)
            memberdict[varname] = vardict.copy()
            vardict.clear()
            
        token = next(tokenit)
    curdict[name]["CT_MEMBERS"] = memberdict.copy()
    return finish_block_for_line(curdict[name], tokenit, context)

def digest_typedef_generic(curdict, ctype, tokenit, context):
    name = next(tokenit)
    while next(tokenit) != ';':
        pass
    curdict[name] = { "CT_TYPE": ctype }
    curdict[name] = curdict[name] | context.options
    return finish_block_for_line(curdict[name], tokenit, context)

def get_structures_as_dict(source : str):
    token_raw = re.split(r"([,\\\"#;\[\]\(\)\{\}\s]|/\*|\*/|\*)", source)
    tokens = []

    # Remove unnecesary blank space
    ignore_next = False
    in_text = False
    stringbuff = ""
    for token in token_raw:
        if token is None or token == '':
            continue
        if not ignore_next and token == '"':
            if in_text:
                tokens.append(stringbuff)
                stringbuff = ""
            in_text = not in_text
            ignore_next = False
            continue

        if token[-1] == '\\':
            ignore_next = True
            continue
        else:
            ignore_next = False

        if in_text:
            stringbuff += token
        elif not token.isspace() or token == "\n":
            tokens.append(token)

    # Setup content
    content = dict()
    content["CT_TYPEDEFS"] = dict()
    content["CT_ENUMS"] = dict()
    content["CT_STRUCTS"] = dict()
    content["CT_COMPONENT_GROUPS"] = dict()
    context = ParseContext()

    # Iterate through top level
    try:
        tokenit = iter(tokens)
        while True:
            token = next(tokenit)
            if token == "/*":
                tokenit = digest_block_comment(context.options, tokenit, context)
            elif token == "typedef":
                ctype = next(tokenit)
                if ctype == "struct":
                    tokenit = digest_typedef_struct(content["CT_STRUCTS"], tokenit, context)
                elif ctype == "enum":
                    tokenit = digest_typedef_enum(content["CT_ENUMS"], tokenit, context)
                else:
                    tokenit = digest_typedef_generic(content["CT_TYPEDEFS"], ctype, tokenit, context)
                context.options = dict()
    except StopIteration:
        pass
    content["CT_COMPONENT_GROUPS"] = context.groups
    return content, context.module_name

def generate_json_data(nodes):
    return json.dumps(nodes, indent=1)

# Write Header
def generate_head_data(nodes):
    result = \
"""#pragma once
#include "utilities/Common.h"
#include "scene/kinnow/KinnowReflection.h"

const struct ctKinnowReflectReferenceEnumField* ctKinnowReflectGetAllEnums();
const struct ctKinnowReflectReferenceStructField* ctKinnowReflectGetAllStructs();
"""
    for module in nodes.values():
        for enum in module["CT_ENUMS"].keys():
            result += f"const struct ctKinnowReflectReferenceEnumField* {enum}_GetFields();\n\n"
        for struct in module["CT_STRUCTS"].keys():
            result += f"const struct ctKinnowReflectReferenceStructField* {struct}_GetFields();\nstatic void {struct}_Init(struct {struct}* ptr);\n\n"
    return result

def generate_flags(entry):
    return 0

def generate_oldnames(entry):
    if "CT_OLD_NAMES" not in entry[1]:
        return f"const char* _{entry[0]}_OldNames[] = {{NULL}};\n"
    result = f"const char* _{entry[0]}_OldNames[] = {{"
    for name in entry[1]["CT_OLD_NAMES"]:
        result += f'"{name}",'
    result += "NULL};\n"
    return result

def generate_enum_entry(entry):
    label = entry[0]
    if "CT_LABEL" in entry[1]:
        label = entry[1]["CT_LABEL"]
    return f'{{"{entry[0]}","{label}",_{entry[0]}_OldNames,{generate_flags(entry[1])},{entry[1]["CT_VALUE"]}}},\n'

def generate_enum_ref(entry):
    return f'ctKinnowReflectReferenceEnumField _{entry[0]}_RefField = {{"{entry[0]}",_{entry[0]}_OldNames,{generate_flags(entry[1])},_{entry[0]}_Fields}},\n'

# Write Implementation
def generate_impl_data(nodes):
    result_includes = '#include "KinnowComponentGen.h"\n'
    result_oldnames = ""
    result_fields = ""
    result_all_enums = "const struct ctKinnowReflectReferenceEnumField* _CT_KINNOW_ALL_ENUMS[] = {\n"
    result_all_structs = "const struct ctKinnowReflectReferenceStructField* _CT_KINNOW_ALL_STRUCTS[] = {\n"
    result_initializers = ""
    for module in nodes.values():
        for enum in module["CT_ENUMS"].items():
            result_fields += f"const struct ctKinnowReflectEnumField* _{enum[0]}_Fields[] = {{\n"
            result_oldnames += generate_oldnames(enum)
            for val in enum[1]["CT_CONSTANTS"].items():
                result_oldnames += generate_oldnames(val)
                result_fields += generate_enum_entry(val)
            result_fields += "NULL};\n"
            result_fields += generate_enum_ref(enum)
            result_all_enums += f"&_{enum[0]}_RefField,\n"
            result_fields += f"const struct ctKinnowReflectReferenceEnumField* {enum[0]}_GetFields() {{ return &_{enum[0]}_RefField; }}\n\n"
    result_all_enums += "NULL};\n "
    # Create fields for each value

    # For each struct
    # Create fields for each member
    result_all_structs += "NULL};\n"

    # Create a reference to each enum
    # Create a reference to each value
    return f"{result_includes}\n{result_oldnames}\n{result_fields}\n{result_all_enums}\n{result_all_structs}\n{result_initializers}"

def main():
    try:
        base_path = pathlib.Path("C:\\Users\\Kenzie\\Documents\\GitHub\\CitrusToolbox")#sys.argv[1]
        codegen_path = pathlib.Path("C:\\Users\\Kenzie\\Documents\\GitHub\\CitrusToolbox\\build\\win64-development-vs19\\generated")#sys.argv[2]
    except IndexError:
        return -1

    kinnow_gen_json_path = codegen_path / "KinnowComponentGen.json"
    kinnow_gen_impl_path = codegen_path / "KinnowComponentGen.cpp"
    kinnow_gen_head_path = codegen_path / "KinnowComponentGen.h"
    try:
        editor_data_file = open(base_path / "EditorData.json", "rt")
        editor_data = json.load(editor_data_file)
        gen_json_file = open(kinnow_gen_json_path, "wt")
        gen_header_file = open(kinnow_gen_head_path, "wt")
        gen_source_file = open(kinnow_gen_impl_path, "wt")
    except: # screw it
        print(f"Could not open files...")
        return -1

    allnodes = dict()
    for subdir in editor_data["kinnow"]:
        directory = base_path / pathlib.Path(subdir)
        for path in directory.glob('*.h'):
            if(path.is_file()):
                input_file = open(path, "rt")
                try:
                    nodes, modulename = get_structures_as_dict(input_file.read())
                    if modulename in allnodes:
                        allnodes[modulename] = allnodes[modulename] | nodes.copy()
                    else:
                        allnodes[modulename] = nodes.copy()
                except ValueError as e:
                    print(f"Internal value error: {path}: {e}")
                except ParseError as e:
                    print(f"Bad file format: {path}: {e}")

    gen_json_file.write(generate_json_data(allnodes))
    gen_header_file.write(generate_head_data(allnodes))
    gen_source_file.write(generate_impl_data(allnodes))

if __name__ == "__main__":
    main()