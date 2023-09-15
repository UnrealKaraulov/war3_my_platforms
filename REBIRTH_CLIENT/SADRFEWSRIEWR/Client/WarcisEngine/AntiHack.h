#pragma once

extern bool BlockAnyThreads;
HMODULE GetModuleFromAddress(int addr);

extern BYTE InjectFound;
LONG VerifyEmbeddedSignature(LPCWSTR pwszSourceFile);

extern int LoadedMssPlugins;

extern HANDLE DestroyAutoJoinThreadHandle;

void LogLogT(DWORD time); void LogLog(std::string s);