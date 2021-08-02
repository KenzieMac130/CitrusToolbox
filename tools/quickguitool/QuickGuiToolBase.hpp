/*
   Copyright 2021 MacKenzie Strand

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

#pragma once

#include "../../engine/utilities/Common.h"
#include "DearImGuiSDL/library/DearImGuiSDL.h"

class QuickGuiToolBase {
public:
	int Execute(int argc, char* argv[]);
	
	virtual int HandleArgs(int argc, char* argv[]);
	virtual int PumpGUI();
	
	bool wantsQuit;
};