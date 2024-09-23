/*
   Copyright 2024 MacKenzie Strand

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

#include "Common.h"
#include "Lexer.hpp"
// clang-format off

/* -------------------------------- Results  -------------------------------- */

CT_REFLECT_ENUM_DEFINE_START(ctResults, CT_REFLECT_ENUM_NUMBERED)
CT_REFLECT_ENUM_DEFINE_ENTRY(ctResults, CT_SUCCESS, "Success")
CT_REFLECT_ENUM_DEFINE_ENTRY(ctResults, CT_FAILURE_UNKNOWN, "Unknown")
CT_REFLECT_ENUM_DEFINE_ENTRY(ctResults, CT_FAILURE_OUT_OF_MEMORY, "Out of memory")
CT_REFLECT_ENUM_DEFINE_ENTRY(ctResults, CT_FAILURE_INVALID_PARAMETER, "Invalid parameter")
CT_REFLECT_ENUM_DEFINE_ENTRY(ctResults, CT_FAILURE_UNSUPPORTED_HARDWARE, "Unsupported hardware")
CT_REFLECT_ENUM_DEFINE_ENTRY(ctResults, CT_FAILURE_UNKNOWN_FORMAT, "Unknown format")
CT_REFLECT_ENUM_DEFINE_ENTRY(ctResults, CT_FAILURE_OUT_OF_BOUNDS, "Out of bounds")
CT_REFLECT_ENUM_DEFINE_ENTRY(ctResults, CT_FAILURE_PARSE_ERROR, "Parse error")
CT_REFLECT_ENUM_DEFINE_ENTRY(ctResults, CT_FAILURE_DECOMPRESSION_ERROR, "Decompression error")
CT_REFLECT_ENUM_DEFINE_ENTRY(ctResults, CT_FAILURE_FILE_NOT_FOUND, "File not found")
CT_REFLECT_ENUM_DEFINE_ENTRY(ctResults, CT_FAILURE_INACCESSIBLE, "Inaccessible")
CT_REFLECT_ENUM_DEFINE_ENTRY(ctResults, CT_FAILURE_DATA_DOES_NOT_EXIST, "Data does not exist")
CT_REFLECT_ENUM_DEFINE_ENTRY(ctResults, CT_FAILURE_DUPLICATE_ENTRY, "Duplicate entry")
CT_REFLECT_ENUM_DEFINE_ENTRY(ctResults, CT_FAILURE_NOT_UPDATABLE, "Not updatable")
CT_REFLECT_ENUM_DEFINE_ENTRY(ctResults, CT_FAILURE_COULD_NOT_SHRINK, "Could not shrink")
CT_REFLECT_ENUM_DEFINE_ENTRY(ctResults, CT_FAILURE_CORRUPTED_CONTENTS, "Corrupted contents")
CT_REFLECT_ENUM_DEFINE_ENTRY(ctResults, CT_FAILURE_DEPENDENCY_NOT_MET, "Dependency not met")
CT_REFLECT_ENUM_DEFINE_ENTRY(ctResults, CT_FAILURE_MODULE_NOT_INITIALIZED, "Uninitialized")
CT_REFLECT_ENUM_DEFINE_ENTRY(ctResults, CT_FAILURE_NOT_FOUND, "Not found")
CT_REFLECT_ENUM_DEFINE_ENTRY(ctResults, CT_FAILURE_SYNTAX_ERROR, "Syntax error")
CT_REFLECT_ENUM_DEFINE_ENTRY(ctResults, CT_FAILURE_RUNTIME_ERROR, "Runtime error")
CT_REFLECT_ENUM_DEFINE_ENTRY(ctResults, CT_FAILURE_TYPE_ERROR, "Type error")
CT_REFLECT_ENUM_DEFINE_ENTRY(ctResults, CT_FAILURE_NOT_FINISHED, "Not finished")
CT_REFLECT_ENUM_DEFINE_ENTRY(ctResults, CT_FAILURE_SKIPPED, "Skipped")
CT_REFLECT_ENUM_DEFINE_ENTRY(ctResults, CT_FAILURE_INCORRECT_VERSION, "Incorrect version")
CT_REFLECT_ENUM_DEFINE_ENTRY(ctResults, CT_FAILURE_END_OF_STREAM, "End of stream")
CT_REFLECT_ENUM_DEFINE_END(ctResults)

/* -------------------------------- Files -------------------------------- */

CT_REFLECT_ENUM_DEFINE_START(ctFileSeekMode, CT_REFLECT_ENUM_NUMBERED)
CT_REFLECT_ENUM_DEFINE_ENTRY(ctFileSeekMode, CT_FILE_SEEK_SET, "Set")
CT_REFLECT_ENUM_DEFINE_ENTRY(ctFileSeekMode, CT_FILE_SEEK_CUR, "Current")
CT_REFLECT_ENUM_DEFINE_ENTRY(ctFileSeekMode, CT_FILE_SEEK_END, "End")
CT_REFLECT_ENUM_DEFINE_END(ctFileSeekMode)

CT_REFLECT_ENUM_DEFINE_START(ctFileOpenMode, CT_REFLECT_ENUM_NUMBERED)
CT_REFLECT_ENUM_DEFINE_ENTRY(ctFileOpenMode, CT_FILE_OPEN_READ, "Read")
CT_REFLECT_ENUM_DEFINE_ENTRY(ctFileOpenMode, CT_FILE_OPEN_WRITE, "Write")
CT_REFLECT_ENUM_DEFINE_ENTRY(ctFileOpenMode, CT_FILE_OPEN_READ_TEXT, "Read Text")
CT_REFLECT_ENUM_DEFINE_ENTRY(ctFileOpenMode, CT_FILE_OPEN_WRITE_TEXT, "Write Text")
CT_REFLECT_ENUM_DEFINE_ENTRY(ctFileOpenMode, CT_FILE_OPEN_READ_VIRTUAL, "Read Virtual File")
CT_REFLECT_ENUM_DEFINE_ENTRY(ctFileOpenMode, CT_FILE_OPEN_WRITE_VIRTUAL, "Write Virtual File")
CT_REFLECT_ENUM_DEFINE_END(ctFileOpenMode)

/* -------------------------------- Lexer -------------------------------- */

CT_REFLECT_ENUM_DEFINE_START(ctLexerTokenType, CT_REFLECT_ENUM_NUMBERED)
CT_REFLECT_ENUM_DEFINE_ENTRY(ctLexerTokenType, CT_LEXER_TOKEN_IDENT, "Identifier")
CT_REFLECT_ENUM_DEFINE_ENTRY(ctLexerTokenType, CT_LEXER_TOKEN_SYMBOLS, "Symbols")
CT_REFLECT_ENUM_DEFINE_ENTRY(ctLexerTokenType, CT_LEXER_TOKEN_STRING, "String")
CT_REFLECT_ENUM_DEFINE_ENTRY(ctLexerTokenType, CT_LEXER_TOKEN_CHAR, "Char")
CT_REFLECT_ENUM_DEFINE_ENTRY(ctLexerTokenType, CT_LEXER_TOKEN_COMMENT, "Comment")
CT_REFLECT_ENUM_DEFINE_ENTRY(ctLexerTokenType, CT_LEXER_TOKEN_NUMBER, "Number")
CT_REFLECT_ENUM_DEFINE_END(ctLexerTokenType)

/* -------------------------------- 3D Math -------------------------------- */

/* vectors, matrices, colors, and quaternions are treated as base types */

CT_REFLECT_ENUM_DEFINE_START(ctAxis, CT_REFLECT_ENUM_NUMBERED)
CT_REFLECT_ENUM_DEFINE_ENTRY(ctAxis, CT_AXIS_X, "X")
CT_REFLECT_ENUM_DEFINE_ENTRY(ctAxis, CT_AXIS_Y, "Y")
CT_REFLECT_ENUM_DEFINE_ENTRY(ctAxis, CT_AXIS_Z, "Z")
CT_REFLECT_ENUM_DEFINE_ENTRY(ctAxis, CT_AXIS_UP, "Up-Down")
CT_REFLECT_ENUM_DEFINE_ENTRY(ctAxis, CT_AXIS_NS, "North-South")
CT_REFLECT_ENUM_DEFINE_ENTRY(ctAxis, CT_AXIS_EW, "East-West")
CT_REFLECT_ENUM_DEFINE_END(ctAxis)

CT_REFLECT_ENUM_DEFINE_START(ctColorComponents, CT_REFLECT_ENUM_BITMASK)
CT_REFLECT_ENUM_DEFINE_ENTRY(ctColorComponents, CT_COLOR_COMPONENT_NONE, "None")
CT_REFLECT_ENUM_DEFINE_ENTRY(ctColorComponents, CT_COLOR_COMPONENT_R, "Red")
CT_REFLECT_ENUM_DEFINE_ENTRY(ctColorComponents, CT_COLOR_COMPONENT_G, "Green")
CT_REFLECT_ENUM_DEFINE_ENTRY(ctColorComponents, CT_COLOR_COMPONENT_B, "Blue")
CT_REFLECT_ENUM_DEFINE_ENTRY(ctColorComponents, CT_COLOR_COMPONENT_A, "Alpha")
CT_REFLECT_ENUM_DEFINE_ENTRY(ctColorComponents, CT_COLOR_COMPONENT_RG, "RG")
CT_REFLECT_ENUM_DEFINE_ENTRY(ctColorComponents, CT_COLOR_COMPONENT_RGB, "RGB")
CT_REFLECT_ENUM_DEFINE_ENTRY(ctColorComponents, CT_COLOR_COMPONENT_RGBA, "RGBA")
CT_REFLECT_ENUM_DEFINE_END(ctColorComponents)

CT_IMPLEMENT_REFLECTOR(ctBoundBox) {
   CT_REFLECT_BASIC_TYPE(ctBoundBox, ctVec3, min, "");
   CT_REFLECT_BASIC_TYPE(ctBoundBox, ctVec3, max, "");
}

CT_IMPLEMENT_REFLECTOR(ctBoundBox2D) {
   CT_REFLECT_BASIC_TYPE(ctBoundBox2D, ctVec2, min, "");
   CT_REFLECT_BASIC_TYPE(ctBoundBox2D, ctVec2, max, "");
}

CT_IMPLEMENT_REFLECTOR(ctBoundSphere) {
   CT_REFLECT_BASIC_TYPE(ctBoundSphere, ctVec3, position, "");
   CT_REFLECT_BASIC_TYPE(ctBoundSphere, float, radius, "");
}

CT_IMPLEMENT_REFLECTOR(ctTransform) {
   CT_REFLECT_BASIC_TYPE(ctTransform, ctVec3, translation, "");
   CT_REFLECT_BASIC_TYPE(ctTransform, ctQuat, rotation, "");
   CT_REFLECT_BASIC_TYPE(ctTransform, ctVec3, scale, "");
}

CT_IMPLEMENT_REFLECTOR(ctCameraInfo) {
   CT_REFLECT_BASIC_TYPE(ctCameraInfo, ctVec3, position, "");
   CT_REFLECT_BASIC_TYPE(ctCameraInfo, ctQuat, rotation, "");
   CT_REFLECT_BASIC_TYPE(ctCameraInfo, float, fov, "");
   CT_REFLECT_BASIC_TYPE(ctCameraInfo, float, aspectRatio, "");
   CT_REFLECT_BASIC_TYPE(ctCameraInfo, float, nearClip, "");
}

// clang-format on