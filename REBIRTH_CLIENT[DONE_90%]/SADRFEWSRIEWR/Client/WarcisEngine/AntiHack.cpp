#include "����� ���������.h"
#include "AntiHack.h"
bool BlockAnyThreads = false;
BYTE InjectFound = 7;

// �������� HMODULE �� ������ � ������. (x86)
HMODULE GetModuleFromAddress(int addr)
{
	if (!addr)
		return 0;
	HMODULE hModule = NULL;
	GetModuleHandleExW(
		GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
		(LPCWSTR)addr,
		&hModule);
	return hModule;
}