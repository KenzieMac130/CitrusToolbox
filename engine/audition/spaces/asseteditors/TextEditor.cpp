/*
   Copyright 2023 MacKenzie Strand

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

#include "TextEditor.hpp"
#include "imguicolortextedit/TextEditor.h"

ctAuditionSpaceAssetEditorTextEditor::ctAuditionSpaceAssetEditorTextEditor() {
   pTextEditor = new TextEditor();
   pTextEditor->SetShowWhitespaces(false);
}

ctAuditionSpaceAssetEditorTextEditor::~ctAuditionSpaceAssetEditorTextEditor() {
   delete pTextEditor;
}

const char* ctAuditionSpaceAssetEditorTextEditor::GetAssetTypeName() {
   return "Text";
}

ctResults ctAuditionSpaceAssetEditorTextEditor::OnLoad(ctJSONReader& configFile) {
   ctFile file = ctFile(assetFilePath, CT_FILE_OPEN_READ);
   if (!file.isOpen()) { return CT_FAILURE_INACCESSIBLE; }
   file.GetText(contents);
   file.Close();
   canSave = true;
   ctStringUtf8 ext = assetFilePath.FilePathGetExtension();
   if (ext == ".c" || ext == ".h") {
      pTextEditor->SetLanguageDefinition(TextEditor::LanguageDefinition::C());
      pTextEditor->SetColorizerEnable(true);
   } else if (ext == ".json") {
      auto langDef = TextEditor::LanguageDefinition::C(); /* good enough */
      langDef.mKeywords.insert("true");
      langDef.mKeywords.insert("false");
      langDef.mKeywords.insert("null");
      pTextEditor->SetLanguageDefinition(langDef);
      pTextEditor->SetColorizerEnable(true);
   } else if (ext == ".cpp" || ext == ".hpp") {
      pTextEditor->SetLanguageDefinition(TextEditor::LanguageDefinition::CPlusPlus());
      pTextEditor->SetColorizerEnable(true);
   } else if (ext == ".lua") {
      pTextEditor->SetLanguageDefinition(TextEditor::LanguageDefinition::Lua());
      pTextEditor->SetColorizerEnable(true);
   } else if (ext == ".ctsh" || ext == ".ctsi") {
      pTextEditor->SetLanguageDefinition(TextEditor::LanguageDefinition::HLSL());
      pTextEditor->SetColorizerEnable(true);
   } else if (ext == ".as") {
      pTextEditor->SetLanguageDefinition(TextEditor::LanguageDefinition::AngelScript());
      pTextEditor->SetColorizerEnable(true);
   } else if (ext == ".po") {
      auto langDef = TextEditor::LanguageDefinition::C(); /* good enough */
      langDef.mKeywords.insert("msgid");
      langDef.mKeywords.insert("msgstr");
      langDef.mSingleLineComment = "#";
      pTextEditor->SetLanguageDefinition(langDef);
      pTextEditor->SetColorizerEnable(true);
   } else {
      pTextEditor->SetLanguageDefinition(TextEditor::LanguageDefinition());
      pTextEditor->SetColorizerEnable(false);
   }
   pTextEditor->SetText(contents.CStr());
   return CT_SUCCESS;
}

ctResults ctAuditionSpaceAssetEditorTextEditor::OnSave(ctJSONWriter& configFile) {
   if (!canSave) { return CT_FAILURE_UNKNOWN; }
   ctFile file = ctFile(assetFilePath, CT_FILE_OPEN_WRITE);
   contents = pTextEditor->GetText().c_str();
   file.WriteRaw(contents.Data(), 1, contents.ByteLength());
   file.Close();
   return CT_SUCCESS;
}

void ctAuditionSpaceAssetEditorTextEditor::OnEditor() {
   pTextEditor->Render("##contents", ImVec2(-FLT_MIN, -FLT_MIN));
}
