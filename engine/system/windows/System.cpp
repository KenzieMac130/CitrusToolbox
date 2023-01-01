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

#include "../System.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <winsock.h>
#pragma comment(lib, "ws2_32.lib")
#include <Windows.h>
#include <stdio.h>

int ctSystemCreateGUID(void* guidPtr) {
   const HRESULT result = CoCreateGuid((GUID*)guidPtr);
   if (result != S_OK) { return -1; };
   return 0;
}

int ctSystemFilePathLocalize(char* str) {
   for (int i = 0; i < strlen(str); i++) {
      if (str[i] == '/') { str[i] = '\\'; }
   }
   return 0;
}

int ctSystemInitialGetLanguage(char* buff, size_t max) {
   wchar_t data[LOCALE_NAME_MAX_LENGTH];
   GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, LOCALE_SNAME, data, LOCALE_NAME_MAX_LENGTH);
   WideCharToMultiByte(
     CP_UTF8, 0, data, LOCALE_NAME_MAX_LENGTH, buff, (int)max, NULL, NULL);
   return 0;
}

int ctSystemExecuteCommand(const char* commandAlias,
                           int argc,
                           const char* argv[],
                           void (*outputCallback)(const char* output, void* userData),
                           void* userData,
                           const char* workingDirectory) {
   size_t argStrSize = strlen(commandAlias) + 1;
   for (int i = 0; i < argc; i++) {
      argStrSize += strlen(argv[i]) + 3;
   }
   char* argStr = (char*)malloc(argStrSize);
   if (!argStr) { return -1000000; }
   memset(argStr, 0, argStrSize);
   size_t pos = 0;
   pos = snprintf(&argStr[pos], argStrSize - pos, "%s", commandAlias);
   for (int i = 0; i < argc; i++) {
      pos += snprintf(&argStr[pos], argStrSize - pos, " \"%s\"", argv[i]);
   }

   /* https://learn.microsoft.com/en-us/windows/win32/procthread/creating-a-child-process-with-redirected-input-and-output
    */
   SECURITY_ATTRIBUTES saAttr;
   saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
   saAttr.bInheritHandle = TRUE;
   saAttr.lpSecurityDescriptor = NULL;

   HANDLE g_hChildStd_IN_Rd = NULL;
   HANDLE g_hChildStd_IN_Wr = NULL;
   HANDLE g_hChildStd_OUT_Rd = NULL;
   HANDLE g_hChildStd_OUT_Wr = NULL;

   if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0)) {
      return -1000001;
   }
   if (!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0)) {
      return -1000002;
   }
   if (!CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0)) {
      return -1000003;
   }
   if (!SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0)) {
      return -1000004;
   }

   PROCESS_INFORMATION piProcInfo;
   STARTUPINFOA siStartInfo;
   BOOL bProcessSuccess = FALSE;
   ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

   ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
   siStartInfo.cb = sizeof(STARTUPINFO);
   siStartInfo.hStdError = g_hChildStd_OUT_Wr;
   siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
   siStartInfo.hStdInput = NULL;
   siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

   if (!CreateProcessA(NULL,
                       argStr,
                       NULL,
                       NULL,
                       TRUE,
                       0,
                       NULL,
                       workingDirectory,
                       &siStartInfo,
                       &piProcInfo)) {
      DWORD err = GetLastError();
      return err;
   }

   CloseHandle(piProcInfo.hProcess);
   CloseHandle(piProcInfo.hThread);

   CloseHandle(g_hChildStd_IN_Rd);
   CloseHandle(g_hChildStd_OUT_Wr);

   DWORD dwRead;
#define BUFSIZE 10000
   CHAR chBuf[BUFSIZE];
   BOOL bSuccess = FALSE;

   for (;;) { /* assumes a program will never ask for input */
      bSuccess = ReadFile(g_hChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, NULL);
      if (!bSuccess || dwRead == 0) { break; }

      chBuf[dwRead] = '\0';
      if (outputCallback) { outputCallback(chBuf, userData); }
   }
   WaitForSingleObject(piProcInfo.hProcess, INFINITE);
   DWORD result = 0;
   GetExitCodeProcess(piProcInfo.hProcess, &result);

   CloseHandle(g_hChildStd_IN_Wr);
   CloseHandle(g_hChildStd_OUT_Rd);
   free(argStr);
   return (int)result;
}

int ctSystemShowFileToDeveloper(const char* path) {
   return system(path);
}

int ctSystemPositionalPrintToString(char* dest,
                                    size_t capacity,
                                    const char* format,
                                    ...) {
   va_list vl;
   va_start(vl, format);
   const int result = _vsprintf_p(dest, capacity, format, vl);
   va_end(vl);
   return result;
}

void* ctSystemMapVirtualFile(const char* path,
                             bool write,
                             size_t reserve,
                             size_t* pSize) {
   return NULL;
}

int ctSystemUnmapVirtualFile(void* buff, size_t length) {
   return -1;
}

void WinSocketCleanup(void) {
   WSACleanup();
}

bool g_wslStarted = false;
int ctSystemEnsurePosixSocket() {
   if (g_wslStarted) { return 0; }
   WSADATA wsaData;
   int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
   if (result != 0) { return result; }
   g_wslStarted = true;
   atexit(WinSocketCleanup);
   return 0;
}

int ctSystemHostSocket(void* handle, int port, int timeout) {
   ctSystemEnsurePosixSocket();
   SOCKET sock;
   sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
   if (sock < 0) { return -1; }

   /* get local host info */
   hostent* localHost;
   char* localIP;
   localHost = gethostbyname("");
   localIP = inet_ntoa(*(struct in_addr*)*localHost->h_addr_list);

   int reuseval = 1;
   setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuseval, sizeof(reuseval));
   if (timeout > 0) {
      setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof timeout);
   }

   /* bind a socket */
   sockaddr_in sadr;
   sadr.sin_family = AF_INET;
   sadr.sin_addr.s_addr = inet_addr(localIP);
   sadr.sin_port = port;
   int result = bind(sock, (sockaddr*)&sadr, sizeof(sadr));
   if (result == SOCKET_ERROR) { return -2; }
   handle = (void*)sock;
   return 0;
}

int ctSystemSocketRecv(void* handle, void* buff, int length) {
   return recv((SOCKET)handle, (char*)buff, length, 0);
}

int ctSystemSocketSend(void* handle, void* buff, int length) {
   return send((SOCKET)handle, (char*)buff, length, 0);
}

void ctSystemCloseSocket(void* handle) {
   closesocket((SOCKET)handle);
}
