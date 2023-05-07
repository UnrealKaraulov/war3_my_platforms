#include "Общий заголовок.h"
#include "MemoryModule.h"
#include "Storm.h"
#include "AntiHack.h"
#include <cctype>
#include <filesystem>
#include <WinInet.h>
#pragma comment(lib,"Wininet.lib")
#include <commctrl.h>
#pragma comment(lib,"Comctl32.lib")
#pragma comment(lib,"Imm32.lib")


typedef FARPROC(WINAPI* pGetProcAddress)(HMODULE hModule, LPCSTR  lpProcName);
pGetProcAddress GetProcAddress_org;
pGetProcAddress GetProcAddress_ptr;
#include <intrin.h>


HIMC WINAPI ImmAssociateContex3(HWND a1, HIMC a2)
{
	return ImmAssociateContext(a1, a2);
}
BOOL WINAPI ImmAssociateContextE3(HWND a1, HIMC a2, DWORD a3)
{
	return ImmAssociateContextEx(a1, a2, a3);
}
void __declspec(naked) ImageList_Ad3()
{
	__asm jmp ImageList_Add;
}
void __declspec(naked) ImageList_BeginDra3()
{
	__asm jmp ImageList_BeginDrag;
}
void __declspec(naked) ImageList_Creat3()
{
	__asm jmp ImageList_Create;
}
void __declspec(naked) ImageList_Destro3()
{
	__asm jmp ImageList_Destroy;
}
void __declspec(naked) ImageList_DragEnte3()
{
	__asm jmp ImageList_DragEnter;
}

void __declspec(naked) ImageList_DragLeav3()
{
	__asm jmp ImageList_DragLeave;
}
void __declspec(naked) ImageList_DragMov3()
{
	__asm jmp ImageList_DragMove;
}
void __declspec(naked) ImageList_DragShowNoloc3()
{
	__asm jmp ImageList_DragShowNolock;
}
void __declspec(naked) ImageList_EndDra3()
{
	__asm jmp ImageList_EndDrag;
}
void __declspec(naked) ImageList_GetImageCoun3()
{
	__asm jmp ImageList_GetImageCount;
}
void __declspec(naked) ImageList_Replac3()
{
	__asm jmp ImageList_Replace;
}
void __declspec(naked) InitCommonControlsE3()
{
	__asm jmp InitCommonControlsEx;
}
void __declspec(naked) _TrackMouseEven3()
{
	__asm jmp _TrackMouseEvent;
}


BOOL WINAPI InternetCanonicalizeUrlB(LPCSTR a1, LPSTR a2, LPDWORD a3, DWORD a4)
{
	return InternetCanonicalizeUrlA(a1, a2, a3, a4);
}

DWORD FirstTickCount = GetTickCount();
DWORD  WINAPI  timeGetTime()
{
	return FirstTickCount - GetTickCount();
}

LSTATUS WINAPI RegQueryValueExB(
	HKEY    hKey,
	LPCSTR  lpValueName,
	LPDWORD lpReserved,
	LPDWORD lpType,
	LPBYTE  lpData,
	LPDWORD lpcbData
)
{
	return RegQueryValueExA(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
}

LSTATUS WINAPI RegOpenKeyExB(
	HKEY   hKey,
	LPCSTR lpSubKey,
	DWORD  ulOptions,
	REGSAM samDesired,
	PHKEY  phkResult
)
{
	return RegOpenKeyExA(hKey, lpSubKey, ulOptions, samDesired, phkResult);
}

LSTATUS WINAPI RegSetValueExB(
	HKEY       hKey,
	LPCSTR     lpValueName,
	DWORD      Reserved,
	DWORD      dwType,
	const BYTE* lpData,
	DWORD      cbData
)
{
	return RegSetValueExA(hKey, lpValueName, Reserved, dwType, lpData, cbData);
}
LSTATUS WINAPI  RegCreateKeyExB(
	HKEY                        hKey,
	LPCSTR                      lpSubKey,
	DWORD                       Reserved,
	LPSTR                       lpClass,
	DWORD                       dwOptions,
	REGSAM                      samDesired,
	const LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	PHKEY                       phkResult,
	LPDWORD                     lpdwDisposition
)
{
	return RegCreateKeyExA(hKey, lpSubKey, Reserved, lpClass, dwOptions, samDesired, lpSecurityAttributes, phkResult, lpdwDisposition);
}

BOOL WINAPI GetUserNameB(
	LPSTR   lpBuffer,
	LPDWORD pcbBuffer
)
{
	return GetUserNameA(lpBuffer, pcbBuffer);
}

LSTATUS WINAPI RegKloseKey(
	HKEY hKey
)
{
	return RegCloseKey(hKey);
}

int RealGameMain;
HMODULE GetCurrentModule = NULL;
PMEMORYMODULE GameMemDll = NULL;

typedef LONG NTSTATUS;
typedef NTSTATUS(WINAPI* NTQUERYINFORMATIONTHREAD)(
	HANDLE ThreadHandle,
	ULONG ThreadInformationClass,
	PVOID ThreadInformation,
	ULONG ThreadInformationLength,
	PULONG ReturnLength);
typedef enum _THREADINFOCLASS {
	ThreadBasicInformation,
	ThreadTimes,
	ThreadPriority,
	ThreadBasePriority,
	ThreadAffinityMask,
	ThreadImpersonationToken,
	ThreadDescriptorTableEntry,
	ThreadEnableAlignmentFaultFixup,
	ThreadEventPair_Reusable,
	ThreadQuerySetWin32StartAddress,
	ThreadZeroTlsCell,
	ThreadPerformanceCount,
	ThreadAmILastThread,
	ThreadIdealProcessor,
	ThreadPriorityBoost,
	ThreadSetTlsArrayAddress,   // Obsolete
	ThreadIsIoPending,
	ThreadHideFromDebugger,
	ThreadBreakOnTermination,
	ThreadSwitchLegacyState,
	ThreadIsTerminated,
	ThreadLastSystemCall,
	ThreadIoPriority,
	ThreadCycleTime,
	ThreadPagePriority,
	ThreadActualBasePriority,
	ThreadTebInformation,
	ThreadCSwitchMon,          // Obsolete
	ThreadCSwitchPmu,
	ThreadWow64Context,
	ThreadGroupInformation,
	ThreadUmsInformation,      // UMS
	ThreadCounterProfiling,
	ThreadIdealProcessorEx,
	MaxThreadInfoClass
} THREADINFOCLASS;


DWORD GetThreadStartAddr(DWORD dwThreadId)
{
	HMODULE hNtdll = LoadLibrary("ntdll.dll");
	if (!hNtdll)
	{
		return 0;
	}

	NTQUERYINFORMATIONTHREAD NtQueryInformationThread = NULL;
	NtQueryInformationThread = (NTQUERYINFORMATIONTHREAD)
		GetProcAddress_ptr(hNtdll, "NtQueryInformationThread");
	if (!NtQueryInformationThread)
	{
		return 0;
	}

	HANDLE ThreadHandle = NULL;
	ThreadHandle = OpenThread(THREAD_QUERY_INFORMATION, FALSE, dwThreadId);
	if (!ThreadHandle)
	{
		return 0;
	}

	DWORD dwStaAddr = NULL;
	DWORD dwReturnLength = 0;
	if (NtQueryInformationThread(ThreadHandle, ThreadQuerySetWin32StartAddress,
		&dwStaAddr, sizeof(dwStaAddr), &dwReturnLength))
	{
		return 0;
	}

	return dwStaAddr;
}


typedef unsigned short(FAR __stdcall* htons_p)(unsigned short hostshort);

htons_p htons_org;
htons_p htons_ptr;

unsigned short FAR __stdcall htons_my(unsigned short hostshort)
{
	if (hostshort == (unsigned short)6112)
	{
		return htons_ptr(6113);
	}
	else
		return htons_ptr(hostshort);
}

bool firstgamemain = false;

PMEMORYMODULE codecs[7]{ NULL ,NULL ,NULL ,NULL ,NULL ,NULL ,NULL };
DWORD codecs_size[7]{ NULL ,NULL ,NULL ,NULL ,NULL ,NULL ,NULL };

bool memory_readable(void* ptr)
{
	char bytes[4];
	DWORD readed = 0;
	if (ReadProcessMemory(GetCurrentProcess(), ptr, bytes, 4, &readed) && readed == 4)
	{
		return true;
	}
	//MEMORY_BASIC_INFORMATION mbi;
	//if (VirtualQuery(ptr, &mbi, sizeof(MEMORY_BASIC_INFORMATION)) == 0)
	//	return false;

	//if (mbi.State != MEM_COMMIT)
	//	return false;

	//if (mbi.Protect == PAGE_NOACCESS || mbi.Protect == PAGE_EXECUTE)
	//	return false;

	//// This checks that the start of memory block is in the same "region" as the
	//// end. If it isn't you "simplify" the problem into checking that the rest of 
	//// the memory is readable.
	//size_t blockOffset = (size_t)((char*)ptr - (char*)mbi.AllocationBase);
	//size_t blockBytesPostPtr = mbi.RegionSize - blockOffset;

	//if (blockBytesPostPtr < byteCount)
	//	return memory_readable((char*)ptr + blockBytesPostPtr,
	//		byteCount - blockBytesPostPtr);

	return false;
}

FARPROC WINAPI GetProcAddress_my(HMODULE hModule, LPCSTR  lpProcName)
{
	auto retaddr = _ReturnAddress();
	if ((DWORD)retaddr == ((DWORD)GetModuleHandle(0) + 0x13f9))
	{
		firstgamemain = true;
		return (FARPROC)GameMain;
	}
	/*if ((LPVOID)hModule == addlib_mod[0]->codeBase)
	{
		return MemoryGetProcAddress(addlib_mod[0], lpProcName);
	}*/

	//if (hModule && codecs[5] && hModule == (HMODULE)codecs[5]->codeBase)
	//{
	//	LogLog("[storm]");
	//	return MemoryGetProcAddress(codecs[5], lpProcName);
	//}

	if (hModule && codecs[6] && hModule == (HMODULE)codecs[6]->codeBase)
	{
		LogLog("[directx]");
		return MemoryGetProcAddress(codecs[6], lpProcName);
	}

	//if ((LPVOID)hModule == addlib_mod[1]->codeBase)
	//{
	//	MessageBoxA(0, "--11", "1", 0);
	//	return MemoryGetProcAddress(addlib_mod[1], lpProcName);
	//}
	//if ((LPVOID)hModule == addlib_mod[2]->codeBase)
	//{
	//	MessageBoxA(0, "--111", "1", 0);
	//	return MemoryGetProcAddress(addlib_mod[2], lpProcName);
	//}
	//if ((LPVOID)hModule == addlib_mod[3]->codeBase)
	//{
	//	MessageBoxA(0, "--1111", "1", 0);
	//	return MemoryGetProcAddress(addlib_mod[4], lpProcName);
	//}/*
	//if ((LPVOID)hModule == addlib_mod[4]->codeBase)
	//{
	//	return MemoryGetProcAddress(addlib_mod[4], lpProcName);
	//}*/

	return GetProcAddress_ptr(hModule, lpProcName);
}

typedef HMODULE(WINAPI* LoadLibraryA_)(LPCSTR modulename);
LoadLibraryA_ LoadLibraryA_org;
LoadLibraryA_ LoadLibraryA_ptr;


std::string str_tolower(std::string s) {
	std::transform(s.begin(), s.end(), s.begin(),
		// static_cast<int(*)(int)>(std::tolower)         // wrong
		// [](int c){ return std::tolower(c); }           // wrong
		// [](char c){ return std::tolower(c); }          // wrong
		[](unsigned char c) { return std::tolower(c); } // correct
	);
	return s;
}

int LoadedMssPlugins = 0;

bool firstd3d = false;

BOOL WINAPI GetOpenFileNameB(
	void* Arg1
)
{
	return FALSE;
}

BOOL WINAPI GetSaveFileNameB(
	void* Arg1
)
{
	return FALSE;
}

HMODULE WINAPI LoadLibraryA_my(LPCSTR modulename)
{
	if (!modulename || modulename[0] == '\0')
	{
		return 0;
	}
	//MessageBoxA(0, modulename, modulename, 0);

	std::string fullpath = str_tolower(modulename);
	std::string extension = str_tolower(std::filesystem::path(modulename).extension().string());
	if (extension.size() > 0)
	{
		if (extension.find("dll") != std::string::npos)
		{
			/*if (fullpath.find("ijl15.dll") != std::string::npos)
			{
				LogLog("Load [INTEL JPEG LIBRARY]");
				HRSRC hResource = FindResource(GetCurrentModule, MAKEINTRESOURCE(IDR_RCDATA11), RT_RCDATA);
				HGLOBAL hMemory = LoadResource(GetCurrentModule, hResource);
				codecs_size[5] = SizeofResource(GetCurrentModule, hResource);
				LPVOID lpAddress = LockResource(hMemory);
				codecs[5] = MemoryLoadLibrary(lpAddress, codecs_size[5]);
				LogLog("Load [OK]");
				return (HMODULE)codecs[5]->codeBase;
			}*/
			if (fullpath.find("d3d8.dll") != std::string::npos && codecs[6] == NULL)
			{
				LogLog("Load [DIRECTX JPEG LIBRARY]");
				HRSRC hResource = FindResource(GetCurrentModule, MAKEINTRESOURCE(IDR_RCDATA6), RT_RCDATA);
				HGLOBAL hMemory = LoadResource(GetCurrentModule, hResource);
				codecs_size[6] = SizeofResource(GetCurrentModule, hResource);
				LPVOID lpAddress = LockResource(hMemory);
				codecs[6] = MemoryLoadLibrary(lpAddress, codecs_size[6]);
				LogLog("Load [OK]");
				return (HMODULE)codecs[6]->codeBase;
			}
		}

		if (/*fullpath.find("redist") != std::string::npos || */(extension.find("mix") != std::string::npos ||
			extension.find("flt") != std::string::npos ||
			extension.find("m3d") != std::string::npos ||
			extension.find("asi") != std::string::npos))
		{
			LogLog("Load [MSS PLUGIN]");
			std::string filename = str_tolower(std::filesystem::path(modulename).filename().string());
			if (filename.find("mp3dec") != std::string::npos)
			{
				LoadedMssPlugins++;
				HRSRC hResource = FindResource(GetCurrentModule, MAKEINTRESOURCE(IDR_RCDATA1), RT_RCDATA);
				HGLOBAL hMemory = LoadResource(GetCurrentModule, hResource);
				codecs_size[0] = SizeofResource(GetCurrentModule, hResource);
				LPVOID lpAddress = LockResource(hMemory);
				codecs[0] = MemoryLoadLibrary(lpAddress, codecs_size[0]);
				LogLog("Load [1]");
				return (HMODULE)codecs[0]->codeBase;
			}
			if (filename.find("mssdolby") != std::string::npos)
			{
				LoadedMssPlugins++;
				HRSRC hResource = FindResource(GetCurrentModule, MAKEINTRESOURCE(IDR_RCDATA2), RT_RCDATA);
				HGLOBAL hMemory = LoadResource(GetCurrentModule, hResource);
				codecs_size[1] = SizeofResource(GetCurrentModule, hResource);
				LPVOID lpAddress = LockResource(hMemory);
				codecs[1] = MemoryLoadLibrary(lpAddress, codecs_size[1]);
				LogLog("Load [2]");
				return (HMODULE)codecs[1]->codeBase;
			}
			if (filename.find("msseax2") != std::string::npos)
			{
				LoadedMssPlugins++;
				HRSRC hResource = FindResource(GetCurrentModule, MAKEINTRESOURCE(IDR_RCDATA3), RT_RCDATA);
				HGLOBAL hMemory = LoadResource(GetCurrentModule, hResource);
				codecs_size[2] = SizeofResource(GetCurrentModule, hResource);
				LPVOID lpAddress = LockResource(hMemory);
				codecs[2] = MemoryLoadLibrary(lpAddress, codecs_size[2]);
				LogLog("Load [3]");
				return (HMODULE)codecs[2]->codeBase;
			}
			if (filename.find("mssfast") != std::string::npos)
			{
				LoadedMssPlugins++;
				HRSRC hResource = FindResource(GetCurrentModule, MAKEINTRESOURCE(IDR_RCDATA4), RT_RCDATA);
				HGLOBAL hMemory = LoadResource(GetCurrentModule, hResource);
				codecs_size[3] = SizeofResource(GetCurrentModule, hResource);
				LPVOID lpAddress = LockResource(hMemory);
				codecs[3] = MemoryLoadLibrary(lpAddress, codecs_size[3]);
				LogLog("Load [4]");
				return (HMODULE)codecs[3]->codeBase;
			}
			if (filename.find("reverb3") != std::string::npos)
			{
				LoadedMssPlugins++;
				HRSRC hResource = FindResource(GetCurrentModule, MAKEINTRESOURCE(IDR_RCDATA5), RT_RCDATA);
				HGLOBAL hMemory = LoadResource(GetCurrentModule, hResource);
				codecs_size[4] = SizeofResource(GetCurrentModule, hResource);
				LPVOID lpAddress = LockResource(hMemory);
				codecs[4] = MemoryLoadLibrary(lpAddress, codecs_size[4]);
				LogLog("Load [5]");
				return (HMODULE)codecs[4]->codeBase;
			}
			LoadedMssPlugins += 55;
			LogLog("Load [6]");
			return (HMODULE)MemoryLoadLibrary(codecs[0], codecs_size[0])->codeBase;;
		}
	}
	return LoadLibraryA_ptr(modulename);
}

DWORD LASTINITIALIZED = 0;
DWORD LASTINITIALIZED2 = 0;
HMODULE StormStormStorm = 0;

// Записать путь к папке Windows
// Проверять список модулей каждые ХЗСКОЛЬКО времени
// Если модуль не из Program Files, не из Windows, и не из папки с игрой...
// Послать его на***

double GameDll;

uint32_t Status[5000];

uint32_t StatusOffset = 5 + rand() % 4990;



void MapsBigger8MB()
{
	DWORD prot1;
	// 127a 0x87266B
	VirtualProtect((void*)((DWORD)GameDll + 0x6577E4), 4, PAGE_EXECUTE_READWRITE, &prot1);
	*(int*)((DWORD)GameDll + 0x6577E4) = 2130706432;
	VirtualProtect((void*)((DWORD)GameDll + 0x6577E4), 4, prot1, &prot1);
	FlushInstructionCache(GetCurrentProcess(), (void*)((DWORD)GameDll + 0x6577E4), 4);


	// 127a 0x85F9BB
	VirtualProtect((void*)((DWORD)GameDll + 0x66ED7F), 4, PAGE_EXECUTE_READWRITE, &prot1);
	*(int*)((DWORD)GameDll + 0x66ED7F) = 2130706432;
	VirtualProtect((void*)((DWORD)GameDll + 0x66ED7F), 4, prot1, &prot1);
	FlushInstructionCache(GetCurrentProcess(), (void*)((DWORD)GameDll + 0x66ED7F), 4);


	// 127a 0x84F535
	VirtualProtect((void*)((DWORD)GameDll + 0x67EC61), 4, PAGE_EXECUTE_READWRITE, &prot1);
	*(int*)((DWORD)GameDll + 0x67EC61) = 2130706432;
	VirtualProtect((void*)((DWORD)GameDll + 0x67EC61), 4, prot1, &prot1);
	FlushInstructionCache(GetCurrentProcess(), (void*)((DWORD)GameDll + 0x67EC61), 4);

	// 127a 0
	VirtualProtect((void*)((DWORD)GameDll + 0x401319), 4, PAGE_EXECUTE_READWRITE, &prot1);
	*(int*)((DWORD)GameDll + 0x401319) = 2130706432;
	VirtualProtect((void*)((DWORD)GameDll + 0x401319), 4, prot1, &prot1);
	FlushInstructionCache(GetCurrentProcess(), (void*)((DWORD)GameDll + 0x401319), 4);
}

//void InitAdditionalLibraries()
//{
//	for (int i = 1; i < 4; i++)
//	{
//		HRSRC hResource = FindResource(GetCurrentModule, MAKEINTRESOURCE(IDR_RCDATA6) + i, RT_RCDATA);
//		HGLOBAL hMemory = LoadResource(GetCurrentModule, hResource);
//		addlib_size[i] = SizeofResource(GetCurrentModule, hResource);
//		LPVOID lpAddress = LockResource(hMemory);
//		addlib[i] = lpAddress;
//	}
//}

void __cdecl ИнициализацияВсехФункций()
{
	LogLog("Antihack initialize all patches");
	std::wstring path = L"Patches";

	DWORD priority = 10;
	HANDLE mpq;

	//InitAdditionalLibraries();
	HRSRC hResource = FindResource(GetCurrentModule, MAKEINTRESOURCE(IDR_BINARY1), RT_RCDATA);
	HGLOBAL hMemory = LoadResource(GetCurrentModule, hResource);
	DWORD dwSize = SizeofResource(GetCurrentModule, hResource);
	LPVOID lpAddress = LockResource(hMemory);
	Status[StatusOffset] = crc32_16bytes(lpAddress, dwSize, 0);
	StormStormStorm = GetModuleHandleA("Storm.dll");
	LogLog("[STORM]");

	Storm::Init(StormStormStorm);
	GameMemDll = MemoryLoadLibrary(lpAddress, dwSize);
	LogLog("[GAME]");
	RealGameMain = (int)MemoryGetProcAddress(GameMemDll, "GameMain");
	GameDll = (double)(DWORD)(GameMemDll->codeBase);
	GameMemDll = NULL;
	LogLog("[MIX PATCHES]");
	for (const auto& entry : std::filesystem::directory_iterator(path))
	{
		Storm::FileOpenArchive(entry.path().string().c_str(), priority, 6, &mpq);
		priority += 2;
	}
	LogLog("[FRAMES]");
	NWar3Frame::CWar3Frame::Init(0x26a, (DWORD)GameDll);
	//NWar3Frame::CWar3Frame::InitCallbackHook( );
	MapsBigger8MB();
	ЗапускАвтоматическогоВхода();
	LASTINITIALIZED = GetTickCount();
	LogLog("[END]");
}

// 2 level error bypass
LONG WINAPI OurCrashHandler(EXCEPTION_POINTERS* pExcept)
{
	char addr[256];
	addr[0] = 'X';
	DWORD errcode;
	DWORD errcode2 = GetLastError();
	if (pExcept)
	{
		if (pExcept->ExceptionRecord)
		{
			errcode = pExcept->ExceptionRecord->ExceptionCode;
			sprintf_s(addr, "%X", pExcept->ExceptionRecord->ExceptionAddress);
		}
	}
	LogLog(std::string("\rОбнаружена ошибка природы:") + addr + std::string(" ") + std::to_string(errcode) + " " + std::to_string(errcode2));
	return ExceptionContinueExecution;
}

HMODULE mss32mdl = NULL;
HMODULE mswsockmdl = NULL;
HMODULE ntdllmdl = NULL;

BOOL __stdcall DllMain(HINSTANCE Module, unsigned int reason, LPVOID)
{
	if (reason == DLL_THREAD_ATTACH)
	{
		LogLog("Thread initialized");
		if (DestroyAutoJoinThreadHandle == GetCurrentThread())
		{
			LogLog("1");
			return TRUE;
		}
		char mdlname[256];
		sprintf_s(mdlname, "%s%s %s%s", "Unk", "nown", "mod", "ule");
		HMODULE ThreadModule = NULL;
		auto ThreadId = GetCurrentThreadId();
		if (ThreadId)
		{
			auto ThreadStartAddr = GetThreadStartAddr(ThreadId);
			if (ThreadStartAddr)
			{
				ThreadModule = GetModuleFromAddress(ThreadStartAddr);
				if (ThreadModule)
				{
					if (StormStormStorm == ThreadModule || ThreadModule == GetModuleHandleA(0))
					{
						LogLog("1");
						return TRUE;
					}
					GetModuleFileNameA(ThreadModule, mdlname, 256);
					DWORD oldprot;
					VirtualProtect((LPVOID)ThreadStartAddr, 4, PAGE_EXECUTE_READ, &oldprot);
					if (oldprot == PAGE_EXECUTE_READWRITE)
					{
						InjectFound = 4;
					}
					VirtualProtect((LPVOID)ThreadStartAddr, 4, oldprot, &oldprot);
					if (ThreadStartAddr == (DWORD)&LoadLibraryA)
					{
						InjectFound = 5;
					}
					if (ThreadStartAddr == (DWORD)&LoadLibraryW)
					{
						InjectFound = 6;
					}
					/*
					auto OffsetThreadModule = ThreadStartAddr - (DWORD)(ThreadModule);
					wchar_t PathOffs[512];
					GetModuleFileNameW(ThreadModule, PathOffs, 512);

					MessageBoxW(0, PathOffs, L"", 0);*/

					/*char AllIfooo[512];
					sprintf_s(AllIfooo, "%s+%X", PathOffs, OffsetThreadModule);
					MessageBoxA(0, AllIfooo, AllIfooo, 0);*/

				}
				else
				{
					InjectFound = 3;
				}
			}
			else
			{
				InjectFound = 2;
			}
		}
		else
		{
			InjectFound = 1;
		}

		if (InjectFound != 7)
		{
			LogLog("2");
			while (true)
			{
				Sleep(100);
			}
		}

		if (LoadedMssPlugins >= 0 && !LASTINITIALIZED && ThreadModule != ntdllmdl)
		{
			LogLog(mdlname + std::string(": c") + std::string("heat") + std::string("2 det") + std::string("ected"));
			//LogLog("3");
			while (true)
			{
				Sleep(100);
			}
		}
		else if (LoadedMssPlugins >= 0 && LASTINITIALIZED2 && ThreadModule != mss32mdl && ThreadModule != mswsockmdl)
		{
			LogLog(mdlname + std::string(": c") + std::string("heat") + std::string(" det") + std::string("ected"));
			while (true)
			{
				Sleep(100);
			}
		}
	}
	if (reason == DLL_PROCESS_ATTACH)
	{
		LogLog("Antihack attached");
		mss32mdl = GetModuleHandleA("Mss32.dll");
		ntdllmdl = GetModuleHandleA("ntdll");
		SetUnhandledExceptionFilter(OurCrashHandler);
		AddVectoredExceptionHandler(1, OurCrashHandler);
		GetCurrentModule = Module;
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
		SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
		MH_Initialize();
		DisableProcessWindowsGhosting();

		GetProcAddress_org = (pGetProcAddress)GetProcAddress(
			GetModuleHandle(TEXT("kernel32.dll")),
			"GetProcAddress");
		if (!GetProcAddress_org)
		{
			GetProcAddress_org = GetProcAddress;
		}
		MH_CreateHook(GetProcAddress_org, &GetProcAddress_my, reinterpret_cast<void**>(&GetProcAddress_ptr));
		MH_EnableHook(GetProcAddress_org);

		HMODULE ws32 = GetModuleHandle("Ws2_32.dll");
		if (!ws32)
		{
			ws32 = LoadLibraryW(L"Ws2_32.dll");
		}
		if (ws32)
		{
			htons_org = (htons_p)GetProcAddress_ptr(ws32, "htons");
			MH_CreateHook(htons_org, &htons_my, reinterpret_cast<void**>(&htons_ptr));
			MH_EnableHook(htons_org);
		}
		else
		{
			MessageBox(0, "ANAL ERROR 2", "LOAD ERROR", 0);
		}
		LoadLibraryA_org = (LoadLibraryA_)GetProcAddress_ptr(
			GetModuleHandle(TEXT("kernel32.dll")),
			"LoadLibraryA");
		if (!LoadLibraryA_org)
		{
			LoadLibraryA_org = LoadLibraryA;
		}
		MH_CreateHook(LoadLibraryA_org, &LoadLibraryA_my, reinterpret_cast<void**>(&LoadLibraryA_ptr));
		MH_EnableHook(LoadLibraryA_org);


		mswsockmdl = GetModuleHandleA("mswsock.dll");
		if (!mswsockmdl)
		{
			mswsockmdl = LoadLibraryA("mswsock.dll");
		}

		LogLog("Antihack initialize...");
		/*DisableThreadLibraryCalls(Module); FATAL ANAL ERROR IF ENABLE THIS!!!*/
	}
	if (reason == DLL_PROCESS_DETACH)
	{
		LogLog("Antihack detach and terminate process");
		ExitProcess(0);
	}
	return TRUE;
}

int oldretaddr;
int oldespvalue;
int newespvalue;
int oldebpvalue;



int save_eax, save_ebx, save_ecx, save_edx, save_esi, save_edi, save_ebp, save_esp;
short save_ax, save_cx, save_dx, save_bx, save_bp, save_si, save_di;

#define safepopad \
	__asm mov eax, save_eax \
	__asm mov ebx, save_ebx \
	__asm mov ecx, save_ecx \
	__asm mov edx, save_edx \
	__asm mov esi, save_esi \
	__asm mov edi, save_edi \
	__asm mov ax, save_ax \
	__asm mov cx, save_cx \
	__asm mov dx, save_dx \
	__asm mov bx, save_bx \
	__asm mov bp, save_bp \
	__asm mov si, save_si \
	__asm mov di, save_di \
	__asm mov esp, save_esp \
	__asm mov ebp, oldretaddr \
	__asm mov [esp], ebp \
	__asm mov esp, save_esp \
	__asm mov ebp, save_ebp 


#define safepushad \
	__asm mov save_eax, eax  \
	__asm mov save_ebx, ebx  \
	__asm mov save_ecx, ecx  \
	__asm mov save_edx, edx  \
	__asm mov save_esi, esi  \
	__asm mov save_edi, edi  \
	__asm mov save_ebp, ebp  \
	__asm mov save_esp, esp  \
	__asm mov save_ax, ax  \
	__asm mov save_cx, cx  \
	__asm mov save_dx, dx  \
	__asm mov save_bx, bx  \
	__asm mov save_bp, bp  \
	__asm mov save_si, si  \
	__asm mov save_di, di  \
	__asm mov eax, [esp] \
	__asm mov oldretaddr, eax 

__declspec(naked) void GameMain()
{
	safepushad;
	__asm call ИнициализацияВсехФункций;
	safepopad;

	__asm jmp RealGameMain;
}


