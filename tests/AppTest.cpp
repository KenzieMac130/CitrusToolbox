#include "core/Application.hpp"

class TestApp : public ctApplication {
	virtual ctResults OnStartup();
	virtual ctResults OnTick(const float deltatime);
	virtual ctResults OnShutdown();
};

ctResults TestApp::OnStartup()
{
	ctStringUtf8 myString = "THIS_IS_A_FILE";
	ctFile file;
	FileSystem->OpenPreferencesFile(file, "Test.cfg", CT_FILE_OPEN_WRITE);
	file.WriteRaw(myString.CStr(), 1, myString.ByteLength());
	file.Close();

	ctFile asset;
	FileSystem->OpenAssetFile(asset, "core/shaders/vk/ABCDEFG.txt");
	char data[32];
	memset(data, 0, 32);
	asset.ReadRaw(data, 1, 32);
	Debug->Log(data);
	asset.Close();

	return CT_SUCCESS;
}

int loopvar = 0;
ctResults TestApp::OnTick(const float deltatime)
{
	Debug->Log("This Message %d", loopvar);
	if (loopvar == 5000) { Exit(); }
	loopvar++;
	return CT_SUCCESS;
}

ctResults TestApp::OnShutdown()
{
	return CT_SUCCESS;
}

int main(int argc, char* argv[])
{
	TestApp* pApplication = new TestApp();
	pApplication->Ignite();
	pApplication->EnterLoop();
	return 0;
}