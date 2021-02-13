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

#include "core/Application.hpp"

class TestApp : public ctApplication {
   virtual ctResults OnStartup();
   virtual ctResults OnTick(const float deltatime);
   virtual ctResults OnShutdown();
};

ctResults TestApp::OnStartup() {
   ctStringUtf8 myString = "THIS_IS_A_FILE";
   ctFile file;
   Engine->FileSystem->OpenPreferencesFile(file, "Test.cfg", CT_FILE_OPEN_WRITE);
   file.WriteRaw(myString.CStr(), 1, myString.ByteLength());
   file.Close();

   ctFile asset;
   Engine->FileSystem->OpenAssetFile(asset, "core/shaders/vk/ABCDEFG.txt");
   char data[32];
   memset(data, 0, 32);
   asset.ReadRaw(data, 1, 32);
   Engine->Debug->Log(data);
   asset.Close();

   return CT_SUCCESS;
}

int loopvar = 0;
ctResults TestApp::OnTick(const float deltatime) {
   if (loopvar <= 5000) {
       Engine->Debug->Log("This Message %d", loopvar);
      loopvar++;
   }
   return CT_SUCCESS;
}

ctResults TestApp::OnShutdown() {
   return CT_SUCCESS;
}

int main(int argc, char* argv[]) {
   TestApp* pApplication = new TestApp();
   return pApplication->Execute(argc, argv);
}