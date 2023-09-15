// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

// ^\w+[\s+\w+]+\(.*?\)[\n\r]*\{[\n\r]* 
// $1#ifndef  ANTIHACKNODEBUG\n AddLogFunctionCall( __FUNCSIGW__  );\n#endif\n


#define NOMINMAX


#pragma comment(lib,"PeLib.lib")
#include "..\PeLib\PeLib.h"

#include "warcis_reconnector.h"


#include "gpsprotocol.h"
#include "socket.h"



#include "Antihack.h"
#include "crc32.h"

#include "CustomFeatures.h"

#include "SimpleCrash.h"

#include <TlHelp32.h>

#include "Storm.h"
#include "BlpReadWrite.h"
#include <algorithm>

#include <ImageStone.h>
#include <.\include\effect\effect_ext.h>
#include "WarcraftFrameHelper.h"

unsigned int Antihack_magic_value = 0xFFFFFFFF;

namespace fs = std::filesystem;


std::vector<uint32_t> WhiteListDlls;
std::vector<uint32_t> WhiteListMaps;


DWORD AhExitThreadCode = 0x10000;


unsigned int LauncherVersion;

std::vector<AH_PACKET> PacketsToSend;

using namespace std;

int GameDll = 0;
int StormDll = 0;
HMODULE StormMdl = NULL;
#define GameDll127aCRC32 0x2466F9B1

unsigned int CurrentGameDllCRC32 = 0;
bool FoundModifiedGameDll = true;
int FoundModifiedLoader = 1;
bool FoundCodeInjection = false;
bool FoundFakeVtable = false;
bool FoundModifiedMemoryCode = false;
bool FoundUnknownModule = false;
bool FoundModifiedMemoryConstants = false;
bool FoundDebugger = false;
bool FoundNoGameDll = false;
bool FoundDebuggerProcess = false;
void * AntihackModule = NULL;
int FoundWhiteListMap = 0;
unsigned int CurrentCodeCrc32 = 1;
unsigned int CurrentConstantsCrc32 = 1;
bool FoundModifiedAhScanner = false;

//
//const char *szProcesses[ ] = {
//								"war3.exe",
//								"ollydbg.exe" ,			// OllyDebug debugger
//								"tcpview.exe" ,			// Part of Sysinternals Suite
//								"procmon.exe" ,			// Part of Sysinternals Suite
//								"procexp.exe" ,			// Part of Sysinternals Suite
//								"idaq.exe" ,				// IDA Pro Interactive Disassembler
//								"idaq64.exe" ,			// IDA Pro Interactive Disassembler
//								"ImmunityDebugger.exe" , // ImmunityDebugger
//								"Wireshark.exe" ,		// Wireshark packet sniffer
//								"dumpcap.exe" ,			// Network traffic dump tool
//								"HookExplorer.exe" ,		// Find various types of runtime hooks
//								"ImportREC.exe" ,		// Import Reconstructor
//								"PETools.exe" ,			// PE Tool
//								"LordPE.exe" ,			// LordPE
//								"dumpcap.exe" ,			// Network traffic dump tool
//								"SysInspector.exe" ,		// ESET SysInspector
//								"proc_analyzer.exe" ,	// Part of SysAnalyzer iDefense
//								"sysAnalyzer.exe" ,		// Part of SysAnalyzer iDefense
//								"sniff_hit.exe" ,		// Part of SysAnalyzer iDefense
//								"windbg.exe" ,			// Microsoft WinDbg
//								"joeboxcontrol.exe" ,	// Part of Joe Sandbox
//								"joeboxserver.exe" 		// Part of Joe Sandbox
//};
//


DWORD LastTimeFromWhiteEnd = 0;

// Если карта в белом списке вернет 1 иначе 0 
int GetFoundWhiteListMapValue( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif

	//CONSOLE_Print( "GetFoundWhiteListMapValue:" + to_string( FoundWhiteListMap ) );


	if ( FoundWhiteListMap == 2 )
	{
		if ( IsGame( ) )
		{
			//	//CONSOLE_Print( "[Warcis_Rec] Map in whitelist update" );
			LastTimeFromWhiteEnd = GetTickCount( );
			return 1;
		}

		if ( LastTimeFromWhiteEnd + 10000 > GetTickCount( ) )
			return 1;

		FoundWhiteListMap = 0;
		CONSOLE_Print( "[AMH] white game end." );
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif

		for ( auto mdl : LibrariesForUnloadAfterGame )
		{
			if ( mdl )
				FreeLibrary( mdl );
		}
		LibrariesForUnloadAfterGame.clear( );
		RestoreBackupGameDllMemory( );
		
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif

	}


	if ( FoundWhiteListMap == 3 )
	{
		return 0;
	}

	LastTimeFromWhiteEnd = GetTickCount( );
	return FoundWhiteListMap;
}

struct AHChecks
{
	int address;
	int size;
	DWORD protection;
	std::vector<unsigned char> buffer;
};

vector<AHChecks> ListOfAHChecks;

time_t _currentsecond = 1;
int scanscount = 0;
time_t lastsecond = 0;

int countchecks = 0;
int countchecks2 = 0;
int countchecks3 = 0;

// Один из вариантов проверки на наличие отладчика
BOOL _cdecl detectDebugger( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	__asm
	{
		mov eax, dword ptr fs : [0x18];
		mov eax, dword ptr[ eax + 0x30 ];
		cmp byte ptr[ eax + 2 ], 0;
		je blocEnd;
		XOR EAX, EAX;
		RET;
	blocEnd:;
		MOV EAX, 1;
		RET;
	}
}




LPCWSTR LastCalledFunctions[ MaxLogSize ];
int LastCodeLine[ MaxLogSize ];
DWORD LastThreadId[ MaxLogSize ];

wchar_t BufferForErrorLog[ 2048 ];
wchar_t BufferForErrorLog2[ 2048 ];

BOOL StopFuncLog = FALSE;

void AddLogFunctionCall( LPCWSTR func, int line )
{

	if ( StopFuncLog )
		return;
	//CONSOLE_Print( func );
	DWORD CurrentThreadId = GetCurrentThreadId( );
	// [0] [0] [0] [0] [0] [Test] [Test2]
	//  0   1   2   3   4     5       6    (7)
	// 
	// [5] = [6]
	// [4] = [5]



	for ( int i = 1; i < MaxLogSize; i++ )
	{
		LastCalledFunctions[ i - 1 ] = LastCalledFunctions[ i ];
		LastCodeLine[ i - 1 ] = LastCodeLine[ i ];
		LastThreadId[ i - 1 ] = LastThreadId[ i ];
	}

	LastCalledFunctions[ MaxLogSize - 1 ] = func;
	LastCodeLine[ MaxLogSize - 1 ] = line;
	LastThreadId[ MaxLogSize - 1 ] = CurrentThreadId;


}


typedef LONG    NTSTATUS;

typedef NTSTATUS( WINAPI *pNtQIT )( HANDLE, LONG, PVOID, ULONG, PULONG );

#define STATUS_SUCCESS    ((NTSTATUS)0x00000000L)

#define ThreadQuerySetWin32StartAddress 9


DWORD WINAPI GetThreadStartAddress( HANDLE hThread )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	NTSTATUS ntStatus;
	HANDLE hDupHandle;
	DWORD dwStartAddress;

	pNtQIT NtQueryInformationThread = ( pNtQIT )GetProcAddress_ptr( GetModuleHandle( "ntdll.dll" ), "NtQueryInformationThread" );

	if ( NtQueryInformationThread == NULL )
		return 0;

	HANDLE hCurrentProcess = GetCurrentProcess( );
	if ( !DuplicateHandle( hCurrentProcess, hThread, hCurrentProcess, &hDupHandle, THREAD_QUERY_INFORMATION, FALSE, 0 ) ) {
		SetLastError( ERROR_ACCESS_DENIED );

		return 0;
	}

	ntStatus = NtQueryInformationThread( hDupHandle, ThreadQuerySetWin32StartAddress, &dwStartAddress, sizeof( DWORD ), NULL );
	CloseHandle( hDupHandle );
	if ( ntStatus != STATUS_SUCCESS )
		return 0;

	return dwStartAddress;

}

BOOL ForceCrash = FALSE;

LPCWSTR GetGameMenuName( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	if ( IsGame( ) )
		return L"INGAME";

	NWar3Frame::CWar3Frame frame = NWar3Frame::CWar3Frame( );
	if ( frame.Load( "LoadingBarText" ) )
	{
		return L"LOADING";
	}

	switch ( current_menu )
	{
	case Wc3Menu::MAIN_MENU:
		return L"MAIN_MENU";
	case Wc3Menu::BNET_LOGIN:
		return L"BNET_LOGIN";
	case Wc3Menu::BNET_MAIN:
		return L"BNET_MAIN";
	case Wc3Menu::BNET_CHAT:
		return L"BNET_CHAT";
	case Wc3Menu::GAME_LIST:
		return L"GAME_LIST";
	case Wc3Menu::GAME_LOBBY:
		return L"GAME_LOBBY";
	case Wc3Menu::GAME_GAME:
		return L"GAME_GAME";
	case Wc3Menu::GAME_NOGAME:
		return L"GAME_NOGAME";
	default:
		break;
	}

	return L"UNKNOWN";
}

crash_rpt::CrashProcessingCallbackResult CALLBACK PFNCRASHPROCESSINGCALLBACK_MY(
	crash_rpt::CrashProcessingCallbackStage stage,	//!< Current crash processing stage.
	crash_rpt::ExceptionInfo* exceptionInfo,		//!< Information about exception being processed.
	LPVOID	userData					//!< Pointer to user-defined data.
)
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	StopFuncLog = TRUE;

	CONSOLE_Print( "Callback back!" );

	/*if ( crash_rpt::CrashProcessingCallbackStage::BeforeSendReport )
	{*/
	if ( ForceCrash )
		return crash_rpt::CrashProcessingCallbackResult::DoDefaultActions;
	ForceCrash = TRUE;

	if ( !exceptionInfo )
		exceptionInfo = new crash_rpt::ExceptionInfo( );
	if ( !exceptionInfo->ExceptionPointers )
		exceptionInfo->ExceptionPointers = new EXCEPTION_POINTERS( );


	CONSOLE_Print( "Crash thread id and info:" );
	swprintf_s( BufferForErrorLog, L"%X(NAME:%s)", exceptionInfo->ThreadId, GetThreadNameById( exceptionInfo->ThreadId ) );
	g_crashRpt.AddUserInfoToReport( L"CrashThreadId", BufferForErrorLog );
	CONSOLE_Print( BufferForErrorLog );
	swprintf_s( BufferForErrorLog, L"Module path: %s. Module start address:%X. Thread start address:%X", GetModuleFilePathW( GetModuleFromAddress( GetThreadStartAddress( GetCurrentThread( ) ) ) ).c_str( ), ( unsigned int )GetModuleFromAddress( GetThreadStartAddress( GetCurrentThread( ) ) ), GetThreadStartAddress( GetCurrentThread( ) ) );
	g_crashRpt.AddUserInfoToReport( L"CrashThreadStartAddress", BufferForErrorLog );
	CONSOLE_Print( BufferForErrorLog );

	CONSOLE_Print( "START" );
	CONSOLE_Print( "DEBUG" );
	CONSOLE_Print( "LOG:" );
	for ( int i = 0; i < MaxLogSize; i++ )
	{
		swprintf_s( BufferForErrorLog, L"LastError_%03X_%04X", i, LastThreadId[ i ] );
		swprintf_s( BufferForErrorLog2, L"%s(%i):(%s)", LastCalledFunctions[ i ], LastCodeLine[ i ], GetThreadNameById( LastThreadId[ i ] ) );

		g_crashRpt.AddUserInfoToReport( BufferForErrorLog, BufferForErrorLog2 );
		CONSOLE_Print( BufferForErrorLog );
		CONSOLE_Print( BufferForErrorLog2 );
	}


	g_crashRpt.AddUserInfoToReport( L"Current game menu:", GetGameMenuName( ) );

	if ( IsGame( ) )
	{
		if ( exceptionInfo->ThreadId != WarcraftThreadId )
		{
			if ( MessageBox( 0, "Ждать завершения игры? Waiting for game finish?", "Warcis Warcraft III Fatal Error.", MB_YESNO ) == IDYES )
			{

				while ( IsGame( ) )
				{
					Sleep( 100 );
				}

			}

		}
	}

	//try
	//{
	//	g_crashRpt.SendReport( exceptionInfo->ExceptionPointers );
	//}
	//catch ( ... )
	//{
	//	CONSOLE_Print( "Report not sent!" );
	//}

	return crash_rpt::CrashProcessingCallbackResult::DoDefaultActions;
}


// Проверка на наличие HardwareBreakpoints
// CheckHardwareBreakpoints returns the number of hardware 
// breakpoints detected and on failure it returns -1.
int CheckHardwareBreakpoints( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	unsigned int NumBps = 0;

	// This structure is key to the function and is the 
	// medium for detection and removal
	CONTEXT ctx;
	ZeroMemory( &ctx, sizeof( CONTEXT ) );

	// The CONTEXT structure is an in/out parameter therefore we have
	// to set the flags so Get/SetThreadContext knows what to set or get.
	ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;

	// Get a handle to our thread
	HANDLE hThread = GetCurrentThread( );

	// Get the registers
	if ( GetThreadContext( hThread, &ctx ) == 0 )
		return -1;

	// Now we can check for hardware breakpoints, its not 
	// necessary to check Dr6 and Dr7, however feel free to
	if ( ctx.Dr0 != 0 )
		++NumBps;
	if ( ctx.Dr1 != 0 )
		++NumBps;
	if ( ctx.Dr2 != 0 )
		++NumBps;
	if ( ctx.Dr3 != 0 )
		++NumBps;

	return NumBps;
}

DWORD LastVirtualProtectTest = GetTickCount( );

typedef int( __fastcall * pDrawUnitAtMainMap /*sub_6F60FCE0*/ )( void *a1, int unused, int a2, float a3, int a4, int a5, int a6 );
pDrawUnitAtMainMap pDrawUnitAtMainMap_org;
pDrawUnitAtMainMap pDrawUnitAtMainMap_ptr;
bool FirstDrawProtection = true;

int __fastcall pDrawUnitAtMainMap_my /*sub_6F60FCE0*/( void *a1, int unused, int a2, float a3, int a4, int a5, int a6 )
{
	int RetAddr = ( int )_ReturnAddress( );

#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	void * accessmodule = GetModuleFromAddress( RetAddr );
	if ( ( int )accessmodule != GameDll )
	{
		if ( FirstDrawProtection )
		{
			FirstDrawProtection = false;

			CONSOLE_Print( "Antihack neurolink protection found anime hacker, disale maphack." );
		}
		return 0;
	}

	return pDrawUnitAtMainMap_ptr( a1, unused, a2, a3, a4, a5, a6 );
}

// Сканирование памяти с дополнительной защитой от взлома
bool Ah_Start_Scan_Mem( BOOL skipchecks )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	bool NeedScanMemoryProtection = false;
	if ( LastVirtualProtectTest + 10000 < GetTickCount( ) )
	{
		LastVirtualProtectTest = GetTickCount( );
		NeedScanMemoryProtection = true;
	}

	if ( GetModuleHandle( "Game.dll" ) )
	{
		CONSOLE_Print( "Found bad Game.dll: " );
		FoundModifiedAhScanner = true;
	}

	if ( ( int )GetModuleHandle( "WarcisGame.dll" ) != GameDll )
	{
		CONSOLE_Print( "Found bad WarcisGame.dll: " );
		FoundModifiedAhScanner = true;
	}

	for ( AHChecks & ahcheck : ListOfAHChecks )
	{
		countchecks2++;


		if ( NeedScanMemoryProtection )
		{
			DWORD oldprotect = 0;
			VirtualProtect( ( LPVOID )ahcheck.address, ahcheck.size, ahcheck.protection, &oldprotect );
			if ( oldprotect != ahcheck.protection )
			{
				//CONSOLE_Print( "Found bad: " + to_string( ( DWORD )ahcheck.address ) );
				///FoundModifiedAhScanner = true;
				//return false;
			}
		}

		if ( memcmp( ( LPCVOID )ahcheck.address, ( LPCVOID )&ahcheck.buffer[ 0 ], ahcheck.size ) != 0 )
		{

			if ( GetFoundWhiteListMapValue( ) == 0 )
			{
				CONSOLE_Print( "Found bad 2: " + to_string( ( DWORD )ahcheck.address ) );
				FoundModifiedAhScanner = true;
				return false;
			}
		}
		/*else if ( !( ahcheck.protection & 0x2200000 ) )
		{
			if ( GetFoundWhiteListMapValue( ) == 0 )
			{
				FoundModifiedAhScanner = true;
				return false;
			}
		}*/
		else
		{
			if ( !skipchecks )
				countchecks++;
		}
		countchecks3++;
	}

	scanscount++;
	return true;
}

// Заменяет текст в строке 
bool replaceAll( std::string& str, const std::string& from, const std::string& to, int addtofrom ) {
	if ( from.empty( ) )
		return false;
	bool replaced = false;
	size_t start_pos = 0;
	while ( ( start_pos = str.find( from, start_pos ) ) != std::string::npos ) {
		replaced = true;
		str.replace( start_pos, from.length( ) + addtofrom, to );
		start_pos += to.length( ); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
	return replaced;
}

// Добавляет новый скан памяти
void AddNewAhChecks( LPVOID addr, int size )
{
	if ( !addr || !size )
		return;
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	AHChecks tmpah;
	tmpah.address = ( int )addr;
	tmpah.size = size;
	tmpah.protection = 0;
	DWORD oldprotect;
	//	DWORD oldprotect2;
	VirtualProtect( addr, size, PAGE_EXECUTE_READWRITE, &tmpah.protection );
	////CONSOLE_Print( "AH CHECK. #1:" + to_string( tmpah.protection ) );
	VirtualProtect( addr, size, tmpah.protection /*+ 0x2200000*/, &oldprotect );
	////CONSOLE_Print( "AH CHECK. #2:" + to_string( oldprotect ) );
	//oldprotect2 = oldprotect;
	//VirtualProtect( addr, size, oldprotect2, &oldprotect );
	////CONSOLE_Print( "AH CHECK. #3:" + to_string( oldprotect ) );
	tmpah.buffer = std::vector<unsigned char>( size );
	CopyMemory( &tmpah.buffer[ 0 ], addr, size );
	ListOfAHChecks.push_back( tmpah );
}


time_t _oldsecond = 0;
HANDLE hAH_Scanner_Thread = 0;
time_t scansecond = 0;


int OriginWar3World = 0x94157C;//126a
int OriginCFrameVtable = 0x96DEB4;//126a
int OriginCFrameFloatVtable = 0x96FAB4;//126a

bool CheckWorldFrame( ) //FIX ME , GET MODULE HANDLE FROM ADDR
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	if ( IsGame( ) )
	{
		int pOff1 = pW3XGlobalClass;
		pOff1 = *( int* )pOff1;
		if ( pOff1 > 0 )
		{
			pOff1 = *( int* )( pOff1 + 0x1C );
			if ( pOff1 > 0 )
			{
				pOff1 = *( int* )( pOff1 + 0xC );
				if ( pOff1 > 0 &&
					*( int* )pOff1 != OriginWar3World + GameDll &&
					*( int* )pOff1 != OriginCFrameVtable + GameDll  &&
					*( int* )pOff1 != OriginCFrameFloatVtable + GameDll )
				{
					char newvtable[ 100 ];
					sprintf_s( newvtable, 100, "New vtable addr: %X", *( int* )pOff1 );
					CONSOLE_Print( newvtable );
					return false;
				}
			}
		}
	}
	return true;
}

// Поток античит сканера
DWORD WINAPI AH_Scanner_Thread( LPVOID )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif

	SetThreadPriority( GetCurrentThread( ), THREAD_PRIORITY_BELOW_NORMAL );
	while ( true )
	{
		_currentsecond = time( NULL );
		int breakpoints = 0;


		breakpoints = CheckHardwareBreakpoints( );

		if ( breakpoints != 0 )
		{
			CONSOLE_Print( "??\n" );
			return 5;
		}



		if ( scansecond < _currentsecond )
		{
			scansecond = _currentsecond;
			ScanResult( );
		}

		if ( _oldsecond == 0 )
			_oldsecond = time( NULL );

		time_t diff = _currentsecond - _oldsecond;

		if ( diff > 2 )
		{
			CONSOLE_Print( "ОБнаружена критическая задержка. [Critical ah delay found! Request ban user! BAD!]" );
			return 4;
		}

		if ( !CheckWorldFrame( ) )
		{
			CONSOLE_Print( "??" );
			FoundFakeVtable = true;
			return 6;
		}


		Ah_Start_Scan_Mem( TRUE );

		breakpoints = CheckHardwareBreakpoints( );

		if ( breakpoints != 0 )
		{
			CONSOLE_Print( "^" );
			return 3;
		}

		Sleep( 10 );
		_oldsecond = time( NULL );
	}

	return 1;
}

void ForceAhScan( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	int breakpoints = CheckHardwareBreakpoints( );

	if ( breakpoints != 0 )
	{
		FoundDebugger = true;
	}

	//if ( detectDebugger( ) )
	//{
	//	FoundDebugger = true;
	//}
	Ah_Start_Scan_Mem( TRUE );
}

DWORD CurrentProcessID = GetCurrentProcessId( );

struct WhiteListModules
{
	int addr;
	DWORD size;
	HMODULE mdl;
	BOOL WhiteListed;
};

vector<WhiteListModules> mdllist;

void BuildModuleWhiteList( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	MODULEENTRY32 lpModuleEntry = { 0 };
	HANDLE hSnapShot = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE, CurrentProcessID );

	if ( !hSnapShot )
		return;
	lpModuleEntry.dwSize = sizeof( lpModuleEntry );

	BOOL bModule = Module32First( hSnapShot, &lpModuleEntry );
	while ( bModule )
	{
		WhiteListModules tmpWhiteListModules;
		tmpWhiteListModules.addr = ( int )lpModuleEntry.modBaseAddr;
		tmpWhiteListModules.size = lpModuleEntry.modBaseSize;
		tmpWhiteListModules.mdl = lpModuleEntry.hModule;
		tmpWhiteListModules.WhiteListed = TRUE;
		mdllist.push_back( tmpWhiteListModules );
		bModule = Module32Next( hSnapShot, &lpModuleEntry );
	}
	CloseHandle( hSnapShot );
}

BOOL IsWhiteList( int addr )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	for ( auto s : mdllist )
	{
		if ( s.WhiteListed && addr >= s.addr && addr < ( int )s.size + s.addr )
			return TRUE;
	}

	return FALSE;
}

HMODULE GetModuleFromAddress_( int addr )
{
	if ( !addr )
		return 0;
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	for ( auto s : mdllist )
	{
		if ( addr >= s.addr && addr < ( int )s.size + s.addr )
			return s.mdl;
	}

	MODULEENTRY32 lpModuleEntry = { 0 };
	HANDLE hSnapShot = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE, CurrentProcessID );

	if ( !hSnapShot )
		return NULL;
	lpModuleEntry.dwSize = sizeof( lpModuleEntry );

	BOOL bModule = Module32First( hSnapShot, &lpModuleEntry );
	while ( bModule )
	{
		if ( addr >= ( int )lpModuleEntry.modBaseAddr && addr < ( int )lpModuleEntry.modBaseAddr + ( int )lpModuleEntry.modBaseSize )
		{
			CloseHandle( hSnapShot );
			//CONSOLE_Print( "Ok module!" );
			return lpModuleEntry.hModule;
		}
		bModule = Module32Next( hSnapShot, &lpModuleEntry );
	}
	CloseHandle( hSnapShot );
	return NULL;
}


// Получает HMODULE из адреса в памяти. (x86)
HMODULE GetModuleFromAddress( int addr )
{
	if ( !addr )
		return 0;
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	for ( auto s : mdllist )
	{
		if ( addr >= s.addr && addr < ( int )s.size + s.addr )
			return s.mdl;
	}
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	int CheckReadMemory = 0;
	HMODULE hModule = NULL;
	GetModuleHandleExW(
		GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
		( LPCWSTR )addr,
		&hModule );
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	return hModule;
}

// Получает путь к DLL
string GetModuleFilePath( HMODULE mdl )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	char tempPath[ MAX_PATH ];
	GetModuleFileNameA( mdl, tempPath, MAX_PATH );
	return string( tempPath );
}

wstring GetModuleFilePathW( HMODULE mdl )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	wchar_t tempPath[ MAX_PATH ];
	GetModuleFileNameW( mdl, tempPath, MAX_PATH );
	return wstring( tempPath );
}
wchar_t TmpFileName[ MAX_PATH ];
size_t DllNameConvSize = 0;
char windir[ MAX_PATH ] = { '\0' };
string ScanningFile = "Scanning file ";
bool IsVerCheck( LPCWSTR name );
bool IsVerCheck( LPCSTR name );

bool IsVerCheck( LPCSTR name )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif

	if ( !FileExists( name ) )
	{
		return true;
	}

	PeLib::PeFile* pef = PeLib::openPeFile( name );
	if ( !pef )
		return false;


	pef->readMzHeader( );
	pef->readPeHeader( );
	if ( pef->readExportDirectory( ) )
	{
		return false;
	}

	if ( pef->readImportDirectory( ) )
	{
		return false;
	}

	bool FoundVer1 = false;

	auto exportdir = ( ( PeLib::PeFile32 * )( pef ) )->expDir( );
	for ( unsigned int i = 0; i < exportdir.getNumberOfFunctions( ); i++ )
	{
		if ( exportdir.getFunctionName( i ) == "CheckRevision" )
		{
			FoundVer1 = true;
		}
	}

	auto imp = ( ( PeLib::PeFile32 * )( pef ) )->impDir( );
	bool FoundVer2 = false;
	for ( unsigned int i = 0; i < imp.getNumberOfFiles( PeLib::OLDDIR ); i++ )
	{
#ifndef  ANTIHACKNODEBUG
		AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
		if ( ToLower( imp.getFileName( i, PeLib::OLDDIR ) ) == "version.dll" )
		{
			for ( unsigned int j = 0; j < imp.getNumberOfFunctions( i, PeLib::OLDDIR ); j++ )
			{
				if ( imp.getFunctionName( i, j, PeLib::OLDDIR ) == "GetFileVersionInfoSizeA" )
				{
					FoundVer2 = true;
				}
			}
		}
	}

	CONSOLE_Print( string( "IsOkayFile(ver): return :" ) + ( ( FoundVer1 && FoundVer2 ) ? "TRUE" : "FALSE" ) );

	return FoundVer1 && FoundVer2;
}

bool IsMssPlugin( const std::string & name )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	PeLib::PeFile* pef = PeLib::openPeFile( name );
	if ( !pef )
		return false;
	pef->readMzHeader( );
	pef->readPeHeader( );
	if ( pef->readImportDirectory( ) )
	{
		return false;
	}

	auto imp = ( ( PeLib::PeFile32 * )( pef ) )->impDir( );
	bool foundmss32 = false;
	for ( unsigned int i = 0; i < imp.getNumberOfFiles( PeLib::OLDDIR ); i++ )
	{
#ifndef  ANTIHACKNODEBUG
		AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
		if ( ToLower( imp.getFileName( i, PeLib::OLDDIR ) ) == "mss32.dll" )
		{
			for ( unsigned int j = 0; j < imp.getNumberOfFunctions( i, PeLib::OLDDIR ); j++ )
			{
				if ( imp.getFunctionName( i, j, PeLib::OLDDIR ) == "RIB_register_interface" )
				{
					//CONSOLE_Print( "Found mss" );
					foundmss32 = true;
				}
			}
		}
		//RIB_register_interface
	}
	CONSOLE_Print( string( "IsOkayFile(mss): return :" ) + ( foundmss32 ? "TRUE" : "FALSE" ) );
	return foundmss32;
}

// Проверяет тип файла и возможно ли загрузить его в память.
bool IsOkayFile( std::string name )
{


#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	CONSOLE_Print( ScanningFile + ( !name.empty( ) ? name : "BAD FILE!" ) );
	//CONSOLE_Print( string( "Win dir " ) + windir );


	name = ToLower( name );

	string CurPath1 = ToLower( CurrentPath_old );
	string CurPath2 = ToLower( CurrentPath_old );

	std::ifstream file( name, ifstream::in | ifstream::binary );


	if ( name.find( ":" ) != string::npos && name.find( CurPath1 ) != string::npos )
	{
		file.close( );

		return false;
	}

	if ( name.find( ":" ) != string::npos && name.find( CurPath1 ) == string::npos )
	{
		file.close( );

		return true;
	}

	if ( !file.is_open( ) || !file.good( ) )
	{
		file.close( );

		return true;
	}


	file.close( );

	if ( IsVerCheck( name.c_str( ) ) )
	{
		return true;
	}

	bool retvalue = false;

	uint32_t curcrc32 = GetFileXXHash( name );
	AH_PACKET tmpah;
	tmpah.AH_Version = ANTIHACK_VERSION;
	tmpah.COMMAND = 4;
	tmpah.var1 = curcrc32;



	for ( uint32_t i : WhiteListDlls )
	{
		if ( curcrc32 == i )
		{
			retvalue = true;
			break;
		}
	}


	if ( !retvalue )
	{
		//char windir[ MAX_PATH ];
		//GetWindowsDirectory( windir, MAX_PATH );

		//if ( strlen( name ) > strlen( windir ) )
		//{
		//	if ( ToLower(name ).find( ToLower( name ) ) != string::npos )
		//		return false;
		//}

		PacketsToSend.push_back( tmpah );
		DeleteFile( name.c_str( ) );

		std::ifstream file2( name, std::ifstream::binary | std::ifstream::in | std::ifstream::ate );
		if ( !file.is_open( ) )
		{
			LoadLibraryA_ptr( name.c_str( ) );
		}
		else
		{
			int id = 0;
			while ( true )
			{
				std::ifstream file3( name, std::ifstream::binary | std::ifstream::in | std::ifstream::ate );
				if ( file.is_open( ) )
				{
					file3.close( );
					MoveFile( name.c_str( ), ( name + ".bad" + to_string( id ) ).c_str( ) );
				}
				else
				{
					file3.close( );
					break;
				}

				id++;
			}
		}
		file2.close( );
		CONSOLE_Print( string( "[AMH] This dll want to be loaded:" ) + string( name ) + "(" + to_string( curcrc32 ) + ")" );
	}


	CONSOLE_Print( string( "[AMH] result:" ) + ( retvalue ? "SKIP" : "BLOCK" ) );
	return retvalue;
}

char DllNameBuffer[ MAX_PATH ];
// Проверяет тип файла и возможно ли загрузить его в память. 
bool IsOkayFile( LPCWSTR name )
{

#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	if ( IsVerCheck( name ) )
	{
		return true;
	}


	memset( DllNameBuffer, 0, MAX_PATH );
	DllNameConvSize = 0;
	wcstombs_s( &DllNameConvSize, DllNameBuffer, MAX_PATH, name, MAX_PATH );

	try
	{
		return IsOkayFile( DllNameBuffer );
	}
	catch ( ... )
	{
		CONSOLE_Print( "Loading this module cause fatal error. Please close game and send Warcis.log to warcis.com" );
		MessageBox( 0, "Loading this module cause fatal error. Please close game and send Warcis.log to warcis.com", "KARAUL!FATAL!", 0 );
		return false;
	}
}
bool IsMssPlugin( LPCWSTR name )
{

#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	if ( !name )
		return true;
	memset( DllNameBuffer, 0, MAX_PATH );
	DllNameConvSize = 0;
	wcstombs_s( &DllNameConvSize, DllNameBuffer, MAX_PATH, name, MAX_PATH );

	bool retval = IsMssPlugin( DllNameBuffer );
	return retval;
}


bool IsVerCheck( LPCWSTR name )
{

#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	if ( !name )
		return true;
	memset( DllNameBuffer, 0, MAX_PATH );
	DllNameConvSize = 0;
	wcstombs_s( &DllNameConvSize, DllNameBuffer, MAX_PATH, name, MAX_PATH );

	bool retval = IsVerCheck( DllNameBuffer );

	return retval;
}


// Проверяет crc32 хэш файла
uint32_t GetFileCrc32( const  string & filepath )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	std::ifstream file( filepath, std::ifstream::binary | std::ifstream::in | std::ifstream::ate );
	if ( file.is_open( ) )
	{
		std::streamsize size = file.tellg( );
		file.seekg( 0, std::ios::beg );
		std::vector<char> buffer( ( uint32_t )size );
		file.read( buffer.data( ), size );
		file.close( );
		return crc32_16bytes_prefetch( buffer.data( ), buffer.size( ) );
	}
	else
	{

	}
	return 0;
}

uint32_t GetFileCrc32( const wstring & filepath )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	std::ifstream file( filepath, std::ifstream::binary | std::ifstream::in | std::ifstream::ate );
	if ( file.is_open( ) )
	{
		std::streamsize size = file.tellg( );
		file.seekg( 0, std::ios::beg );
		std::vector<char> buffer( ( uint32_t )size );
		file.read( buffer.data( ), size );
		file.close( );
		return crc32_16bytes_prefetch( buffer.data( ), buffer.size( ) );
	}
	else
	{

	}
	return 0;
}

uint32_t GetFileXXHash( const  string & filepath )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	std::ifstream file( filepath, std::ifstream::binary | std::ifstream::in | std::ifstream::ate );
	if ( file.is_open( ) )
	{
		std::streamsize size = file.tellg( );
		file.seekg( 0, std::ios::beg );
		std::vector<char> buffer( ( uint32_t )size );
		file.read( buffer.data( ), size );
		file.close( );
		return GetXXHash( buffer.data( ), buffer.size( ) );
	}
	else
	{

	}
	return 0;
}

uint32_t GetFileXXHash( const wstring & filepath )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	std::ifstream file( filepath, std::ifstream::binary | std::ifstream::in | std::ifstream::ate );
	if ( file.is_open( ) )
	{
		std::streamsize size = file.tellg( );
		file.seekg( 0, std::ios::beg );
		std::vector<char> buffer( ( uint32_t )size );
		file.read( buffer.data( ), size );
		file.close( );
		return GetXXHash( buffer.data( ), buffer.size( ) );
	}
	else
	{

	}
	return 0;
}

std::time_t LastAHscan = std::time( NULL );

/*
extern int currentsecond;
extern int scanscount;
extern int lastsecond;*/


void * wc3classgproxy = nullptr;
int badcount = 0;

// Ответ сервера античита и отправка данных серверу античита
bool AH_Scan( void * _wc3classgproxy )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	int RetAddr = ( int )_ReturnAddress( );
	void * accessmodule = GetModuleFromAddress( RetAddr );
	if ( accessmodule != AntihackModule )
	{
		CONSOLE_Print( "BAD1:" );
		FoundModifiedLoader = 2;
	}

	if ( _wc3classgproxy != nullptr )
	{
		wc3classgproxy = _wc3classgproxy;
	}

	ForceAhScan( );

	if ( wc3classgproxy == nullptr )
	{
		//CONSOLE_Print( "ERROR: Bad wc3 protocol." );
		return false;
	}


	CWC3 * gproxy_wc3 = ( CWC3 * )wc3classgproxy;
	// проверку crc32 WarcisGame.dll файла
	// проверку crc32 WarcisGame.dll секции кода

	std::time( &CurrentAhTime );

	if ( LastAHscan + 5 < CurrentAhTime )
	{
		std::time( &LastAHscan );
		/*	if ( scanscount < currentsecond )
			{
				badcount++;
			}
			else
			{
				badcount = 0;
			}
			*/


		ScanResult( );

		if ( lastsecond == _currentsecond && GetFoundWhiteListMapValue( ) == 0 )
		{
			badcount = 0xC;
		}



		//avoid crash 



	}


	return true;
}


pBaseThreadInitThunk BaseThreadInitThunk_org = nullptr;
pBaseThreadInitThunk BaseThreadInitThunk_ptr = nullptr;

pLoadLibraryA LoadLibraryA_org;
pLoadLibraryA LoadLibraryA_ptr;


std::map<string, HMODULE> loadlibraryAcache;
std::map<wstring, HMODULE> loadlibraryWcache;

int curval = 0;

std::vector<HMODULE> LibrariesForUnloadAfterGame;

HMODULE __stdcall LoadLibraryA_my( LPCSTR name )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	/*CONSOLE_Print( string( "LoadLibrary:" ) + name );
	*/
	auto retaddr = _ReturnAddress( );
	bool NeedAddToUnloadList = false;
	if ( !GetModuleFromAddress( ( int )retaddr ) )
	{
		if ( GetFoundWhiteListMapValue( ) == 0 )
			return GetModuleHandleA( name );
		else
			NeedAddToUnloadList = true;
	}



	if ( !name || name[ 0 ] == '\0' )
	{
		return LoadLibraryA_ptr( name );
	}


	HMODULE retval = GetModuleHandleA( name );
	if ( retval )
		return retval;

	if ( name && name[ 0 ] != '\0' )
	{

		auto libraryext = fs::path( name ).extension( );
		auto fileext = ToLower( libraryext.string( ) );


		CONSOLE_Print( string( "LoadLibrary:" ) + name + ". Ext:" + libraryext.string( ).c_str( ) );


		if ( !fileext.empty( ) &&
			fileext != ".dll" &&
			fileext != ".drv"&&
			fileext != ".nls"&&
			fileext != ".dat" &&
			fileext != ".mui" &&
			fileext != ".ax" )
		{
			if ( IsMssPlugin( name ) )
			{
				CONSOLE_Print( string( "LOADOK[2]:" ) + name );
				HMODULE retval = LoadLibraryA_ptr( name );
				if ( NeedAddToUnloadList )
				{
					bool notfounddllinlist = true;
					for ( auto mdl : LibrariesForUnloadAfterGame )
					{
						if ( mdl == retval )
						{
							notfounddllinlist = false;
							break;
						}
					}
					if ( notfounddllinlist )
						LibrariesForUnloadAfterGame.push_back( retval );
				}
				return retval;
			}
			else
			{
				MoveFileA( name, ( name + string( ".bak" ) ).c_str( ) );
				CONSOLE_Print( string( "LOADBAD:" ) + name );
				return GetModuleHandleA( name );
			}
		}
		if ( IsOkayFile( name ) )
		{
			CONSOLE_Print( string( "LOADOK:" ) + name );
			HMODULE retval = LoadLibraryA_ptr( name );
			if ( NeedAddToUnloadList )
			{
				bool notfounddllinlist = true;
				for ( auto mdl : LibrariesForUnloadAfterGame )
				{
					if ( mdl == retval )
					{
						notfounddllinlist = false;
						break;
					}
				}
				if ( notfounddllinlist )
					LibrariesForUnloadAfterGame.push_back( retval );
			}
			return retval;
		}

		MoveFileA( name, ( name + string( ".bak" ) ).c_str( ) );
		CONSOLE_Print( string( "LOADBAD:" ) + name );
	}

	return GetModuleHandleA( name );
}

//// Перехваченная функция LoadLibaryA
//HMODULE __stdcall LoadLibraryA_my2( LPCSTR name )
//{
//#ifndef  ANTIHACKNODEBUG
//	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
//#endif
//	/*CONSOLE_Print( string( "LoadLibrary:" ) + name );
//*/
//	auto retaddr = _ReturnAddress( );
//
//	HMODULE retval = NULL;
//
//	if ( name &&  name[ 0 ] != '\0' )
//	{
//		CONSOLE_Print( string( "LoadLibrary:" ) + name );
//
//		//auto retsearch = loadlibraryAcache.find( name );
//
//		//if ( retsearch != loadlibraryAcache.end( ) )
//		//{
//		//	retval = retsearch->second;
//		//	CONSOLE_Print( string( "LoadLibraryOK:" ) + to_string( ( int )retval ) );
//
//		//	//CONSOLE_Print( string( "LoadLibrary:" ) + name );
//		//	return retval;
//		//}
//
//
//		retval = GetModuleHandleA( name );
//
//		if ( !retval )
//		{
//			if ( !GetModuleFromAddress( ( int )retaddr ) )
//				retval = GetModuleHandleA( name );
//			else
//			{
//				if ( IsOkayFile( name ) )
//				{
//					retval = LoadLibraryA_ptr( name );
//				}
//				else
//				{
//					//	MessageBox( 0, name, name, 0 );
//					CONSOLE_Print( "Warning. Detect virus or hack!." );
//					FoundUnknownModule = true;
//				}
//			}
//		}
//
//		//if ( name && strlen( name ) > 0 && retval )
//		//{
//		//	loadlibraryAcache[ name ] = retval;
//		//}
//
//	}
//	else CONSOLE_Print( "FOUND BADFILENAME!" );
//
//
//	CONSOLE_Print( string( "LoadLibrary:" ) + ( retval > 0 ? "OK:" : "BAD:" ) + to_string( ( int )retval ) );
//
//	return retval;
//}


pLoadLibraryW LoadLibraryW_ptr;
pLoadLibraryW LoadLibraryW_org;

HMODULE __stdcall LoadLibraryW_my( LPCWSTR name )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	/*CONSOLE_Print( string( "LoadLibrary:" ) + name );
	*/
	auto retaddr = _ReturnAddress( );

	bool NeedAddToUnloadList = false;
	if ( !GetModuleFromAddress( ( int )retaddr ) )
	{
		if ( GetFoundWhiteListMapValue( ) == 0 )
			return GetModuleHandleW( name );
		else
			NeedAddToUnloadList = true;
	}

	if ( !name || wcslen( name ) < 1 )
		return LoadLibraryW_ptr( name );

	//HMODULE retval = GetModuleHandleW( name );
	//if ( retval )
	//	return retval;

	if ( name && lstrlenW( name ) > 0 )
	{
		CONSOLE_Print( wstring( L"LoadLibraryw:" ) + name );


		auto libraryext = fs::path( name ).extension( );
		auto fileext = ToLower( libraryext.string( ) );
		if ( !fileext.empty( ) &&
			fileext != ".dll" &&
			fileext != ".drv"&&
			fileext != ".nls"&&
			fileext != ".dat" &&
			fileext != ".mui" )
		{
			if ( IsMssPlugin( name ) )
			{
				HMODULE retval = LoadLibraryW_ptr( name );
				if ( NeedAddToUnloadList )
				{
					bool notfounddllinlist = true;
					for ( auto mdl : LibrariesForUnloadAfterGame )
					{
						if ( mdl == retval )
						{
							notfounddllinlist = false;
							break;
						}
					}
					if ( notfounddllinlist )
						LibrariesForUnloadAfterGame.push_back( retval );
				}
				return retval;
			}
			else
			{
				CONSOLE_Print( wstring( L"LOADBAD[W]:" ) + name );
				return GetModuleHandleW( name );
			}
		}
		if ( IsOkayFile( name ) )
		{
			CONSOLE_Print( wstring( L"LOADBOK[W]:" ) + name );
			HMODULE retval = LoadLibraryW_ptr( name );
			if ( NeedAddToUnloadList )
			{
				bool notfounddllinlist = true;
				for ( auto mdl : LibrariesForUnloadAfterGame )
				{
					if ( mdl == retval )
					{
						notfounddllinlist = false;
						break;
					}
				}
				if ( notfounddllinlist )
					LibrariesForUnloadAfterGame.push_back( retval );
			}
			return retval;
		}
		CopyFileW( name, ( name + wstring( L".tmp" ) ).c_str( ), FALSE );
		CONSOLE_Print( wstring( L"LOADBAD[W]:" ) + name );
	}

	return GetModuleHandleW( name );
}



// Перехваченная функция LoadLibaryW
HMODULE __stdcall LoadLibraryW_my2( LPCWSTR name )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	//CONSOLE_Print( ( wstring( L"LoadLibrary:" ) + name ) );

	//	//CONSOLE_Print( "llw release" );
	auto retaddr = _ReturnAddress( );

	HMODULE retval = NULL;

	if ( name && wcslen( name ) > 0 )
	{
		CONSOLE_Print( ( wstring( L"WLoadLibrary:" ) + name ) );

		//auto retsearch = loadlibraryWcache.find( name );

		//if ( retsearch != loadlibraryWcache.end( ) )
		//{
		//	retval = retsearch->second;
		//	CONSOLE_Print( wstring( L"WLoadLibraryOK:" ) + to_wstring( ( int )retval ) );
		//	//CONSOLE_Print( ( wstring( L"LoadLibrary:" ) + name ) );
		//	return retval;
		//}


		retval = GetModuleHandleW( name );

		if ( !retval )
		{
			if ( !GetModuleFromAddress( ( int )retaddr ) )
				retval = GetModuleHandleW( name );
			else
			{
				if ( IsOkayFile( name ) )
				{
					retval = LoadLibraryW_ptr( name );
				}
				else
				{
					//	MessageBox( 0, name, name, 0 );
					CONSOLE_Print( "Warning. Detect virus or hack!." );
					FoundUnknownModule = true;
				}
			}
		}

		/*if ( name && wcslen( name ) > 0 && retval )
		{
			loadlibraryWcache[ name ] = retval;
		}*/
	}
	else CONSOLE_Print( "FOUND BADFILENAMEW!" );


	CONSOLE_Print( wstring( L"WLoadLibrary:" ) + ( retval > 0 ? L"OK:" : L"BAD:" ) + to_wstring( ( int )retval ) );

	return retval;
}



pLdrLoadDll pLdrLoadDll_org;
pLdrLoadDll pLdrLoadDll_ptr;
bool pLdrCalled = false;
LONG NTAPI pLdrLoadDll_my(
	PWCHAR          PathToFile,
	ULONG           Flags,
	PUNICODE_STRING ModuleFileName,
	PHANDLE        ModuleHandle )
{
	auto retaddr = _ReturnAddress( );
	/*bool needskipcheck = pLdrCalled;
	pLdrCalled = true;
	*/
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	if ( !GetModuleFromAddress( ( int )retaddr ) )
	{
		if ( ModuleFileName->Buffer && ModuleHandle )
			*ModuleHandle = GetModuleHandleW( ModuleFileName->Buffer );
		pLdrCalled = true;
		return 0;
	}


	/*	if ( ModuleFileName->Buffer && ( needskipcheck || IsOkayFile( ModuleFileName->Buffer ) ) )
		{
			pLdrCalled = false;*/
	return pLdrLoadDll_ptr( PathToFile, Flags, ModuleFileName, ModuleHandle );
	/*	}

		if ( ModuleHandle &&  ModuleFileName->Buffer )
			*ModuleHandle = GetModuleHandleW( ModuleFileName->Buffer );

		pLdrCalled = false;
		return 0;*/
}



typedef BOOL( __stdcall  * FreeLibrary_p )( HMODULE lib );
FreeLibrary_p FreeLibrary_org;
FreeLibrary_p FreeLibrary_ptr;

BOOL __stdcall FreeLibrary_my( HMODULE lib )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	//char mdbuf[ MAX_PATH ];

	//GetModuleFileNameA( lib, mdbuf, MAX_PATH );

	//CONSOLE_Print( string( "Unload:" ) + mdbuf );

	if ( lib )
	{
		/*	for ( auto & libw : loadlibraryWcache )
			{
				if ( libw.second == lib )
				{
					auto val = loadlibraryWcache.find( libw.first );
					if ( val != loadlibraryWcache.end( ) )
					{
						CONSOLE_Print( wstring( L"[W]Unload:" ) + libw.first );
						loadlibraryWcache.erase( val );
					}
				}
			}


			for ( auto & liba : loadlibraryAcache )
			{
				if ( liba.second == lib )
				{
					auto val = loadlibraryAcache.find( liba.first );
					if ( val != loadlibraryAcache.end( ) )
					{
						CONSOLE_Print( string( "[A]Unload:" ) + liba.first );
						loadlibraryAcache.erase( val );
					}
				}
			}
	*/
	}
	return FreeLibrary_ptr( lib );
}


DWORD WINAPI NULLTHREAD( LPVOID )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	while ( true )
	{
		Sleep( 10000 );
	}
	return 1;
}

BOOL TwoLineProtection = FALSE;


BOOL IsBadAddress( int addr )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	BOOL retval = TRUE;


	if ( addr >= ( int )LoadLibraryA_org &&
		addr <= ( int )LoadLibraryA_org + 100 )
	{
		return TRUE;
	}
	else
	{
		retval = FALSE;
	}

	if ( addr >= ( int )LoadLibraryW_org &&
		addr <= ( int )LoadLibraryW_org + 100 )
	{
		return TRUE;
	}
	else
	{
		retval = FALSE;
	}


	if ( addr >= ( int )pLdrLoadDll_org &&
		addr <= ( int )pLdrLoadDll_org + 100 )
	{
		return TRUE;
	}
	else
	{
		retval = FALSE;
	}


	return retval;
}



BOOL __stdcall IsBadBaseThreadInitThunk( PVOID StartAddress )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	//if ( IsWhiteList( ( int )StartAddress ) )
	//	return TRUE;

	if (/* TwoLineProtection && */!GetModuleFromAddress( ( int )StartAddress ) )
	{
		//CONSOLE_Print( "bad ." );

		FoundCodeInjection = true;
		return TRUE;
	}

	if ( IsBadAddress( ( int )StartAddress ) )
	{
		//CONSOLE_Print( "bad .." );

		return TRUE;
	}

	//CONSOLE_Print( "ok ..." );

	return FALSE;
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


void * __fastcall BaseThreadInitThunk_my( int unk1, PVOID StartAddress, PVOID ThreadParameter )
{
	//#ifndef  ANTIHACKNODEBUG
	//	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
	//#endif
	int retaddr = ( int )_ReturnAddress( );
	BOOL badval = FALSE;
	BOOL badval2 = FALSE;

#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	//CONSOLE_Print( string( "Caller:" ) + to_string( retaddr ) + string( ", target:" ) + to_string( ( int )StartAddress ) );
//#ifndef  ANTIHACKNODEBUG
//	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
//#endif

	//char caller[ 5000 ];
	//char target[ 5000 ];

	//	GetModuleFileNameA( GetModuleFromAddress( retaddr ), caller, 5000 );
	//	CONSOLE_Print( string( "Caller:" ) + caller );
	//	GetModuleFileNameA( GetModuleFromAddress( (int) StartAddress ), target, 5000 );
	//	CONSOLE_Print( string(", target:" ) + target  );

	//	__asm safepushad;
	badval = IsBadBaseThreadInitThunk( ( PVOID )retaddr );
	badval2 = IsBadBaseThreadInitThunk( StartAddress );
	//#ifndef  ANTIHACKNODEBUG
	//	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
	//#endif
		//	__asm safepopad;
	if ( badval || badval2 )
	{
		return BaseThreadInitThunk_ptr( unk1, NULLTHREAD, ThreadParameter );
	}
	else
	{
		return BaseThreadInitThunk_ptr( unk1, StartAddress, ThreadParameter );
	}

	/*if ( !GetModuleFromAddress( ( int )StartAddress ) )
	{
		//CONSOLE_Print( "bad 1" );

		FoundCodeInjection = true;
		return BaseThreadInitThunk_ptr( unk1, NULLTHREAD, ThreadParameter, unk2, unk3 );
	}
	else
	{
		if ( StartAddress == ( PVOID )LoadLibraryA_org ||
			StartAddress == ( PVOID )LoadLibraryW_org )
		{
			//CONSOLE_Print( "bad 2" );

			return BaseThreadInitThunk_ptr( unk1, NULLTHREAD, ThreadParameter, unk2, unk3 );
		}
		else
		{
			return BaseThreadInitThunk_ptr( unk1, StartAddress, ThreadParameter, unk2, unk3 );
		}

	}
*/
	return 0;
}



struct CallAhFunc
{
	int AH_VERSION;
};

bool IsGameDllFound( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	return GetModuleHandle( GetGameDllName( ).c_str( ) );
}
//1.27a снимает ограничение на размер карты.
void MapsBigger8MB( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	DWORD prot1;
	// 127a 0x87266B
	VirtualProtect( ( void * )( GameDll + 0x6577E4 ), 4, PAGE_EXECUTE_READWRITE, &prot1 );
	*( int* )( GameDll + 0x6577E4 ) = 2130706432;
	VirtualProtect( ( void * )( GameDll + 0x6577E4 ), 4, prot1, &prot1 );
	FlushInstructionCache( GetCurrentProcess( ), ( void * )( GameDll + 0x6577E4 ), 4 );


	// 127a 0x85F9BB
	VirtualProtect( ( void * )( GameDll + 0x66ED7F ), 4, PAGE_EXECUTE_READWRITE, &prot1 );
	*( int* )( GameDll + 0x66ED7F ) = 2130706432;
	VirtualProtect( ( void * )( GameDll + 0x66ED7F ), 4, prot1, &prot1 );
	FlushInstructionCache( GetCurrentProcess( ), ( void * )( GameDll + 0x66ED7F ), 4 );


	// 127a 0x84F535
	VirtualProtect( ( void * )( GameDll + 0x67EC61 ), 4, PAGE_EXECUTE_READWRITE, &prot1 );
	*( int* )( GameDll + 0x67EC61 ) = 2130706432;
	VirtualProtect( ( void * )( GameDll + 0x67EC61 ), 4, prot1, &prot1 );
	FlushInstructionCache( GetCurrentProcess( ), ( void * )( GameDll + 0x67EC61 ), 4 );

	// 127a 0
	VirtualProtect( ( void * )( GameDll + 0x401319 ), 4, PAGE_EXECUTE_READWRITE, &prot1 );
	*( int* )( GameDll + 0x401319 ) = 2130706432;
	VirtualProtect( ( void * )( GameDll + 0x401319 ), 4, prot1, &prot1 );
	FlushInstructionCache( GetCurrentProcess( ), ( void * )( GameDll + 0x401319 ), 4 );

}

char * gamedllconstantsectionmemory = NULL;
char * gamedllcodesectionmemory = NULL;

void InitGameDllBackupMemory( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	if ( gamedllconstantsectionmemory == NULL )
		gamedllconstantsectionmemory = new char[ GameDllConstantsSize ];
	if ( gamedllcodesectionmemory == NULL )
		gamedllcodesectionmemory = new char[ GameDllCodeSize ];
	memcpy( gamedllconstantsectionmemory, ( char* )( GameDll + GameDllConstantsOffset ), GameDllConstantsSize );
	memcpy( gamedllcodesectionmemory, ( char* )( GameDll + GameDllCodeOffset ), GameDllCodeSize );
}

void RestoreBackupGameDllMemory( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif

	for ( int i = 0; i < GameDllConstantsSize;)
	{
		int Memsize = 256;
		if ( GameDllConstantsSize - i < 256 )
			Memsize = GameDllConstantsSize - i;


		LPVOID srcaddr = ( LPVOID )( GameDll + GameDllConstantsOffset + i );
		LPVOID dstaddr = ( LPVOID )( ( int )gamedllconstantsectionmemory + i );

		if ( memcmp( srcaddr, dstaddr, Memsize ) == 0 )
		{
			i += Memsize;
			continue;
		}

		DWORD oldprot1, oldprot2;
		oldprot1 = 0;
		oldprot2 = 0;

		VirtualProtect( srcaddr, Memsize, PAGE_EXECUTE_READWRITE, &oldprot1 );
		memcpy( srcaddr, dstaddr, Memsize );
		VirtualProtect( srcaddr, Memsize, oldprot1, &oldprot2 );

		char addrinfo[ 100 ];
		sprintf_s( addrinfo, "Restore modification at:%X", srcaddr );
		CONSOLE_Print( addrinfo );

		i += Memsize;
	}

	for ( int i = 0; i < GameDllCodeSize; )
	{
		int Memsize = 256;
		if ( GameDllCodeSize - i < 256 )
			Memsize = GameDllCodeSize - i;


		LPVOID srcaddr = ( LPVOID )( GameDll + GameDllCodeOffset + i );
		LPVOID dstaddr = ( LPVOID )( ( int )gamedllcodesectionmemory + i );

		if ( memcmp( srcaddr, dstaddr, Memsize ) == 0 )
		{
			i += Memsize;
			continue;
		}

		DWORD oldprot1, oldprot2;
		oldprot1 = 0;
		oldprot2 = 0;

		VirtualProtect( srcaddr, Memsize, PAGE_EXECUTE_READWRITE, &oldprot1 );
		memcpy( srcaddr, dstaddr, Memsize );
		VirtualProtect( srcaddr, Memsize, oldprot1, &oldprot2 );

		char addrinfo[ 100 ];
		sprintf_s( addrinfo, "Restore modification at:%X", srcaddr );
		CONSOLE_Print( addrinfo );
		i += Memsize;
	}



	/*

		DWORD oldprot1, oldprot2;
		oldprot1 = 0;
		oldprot2 = 0;
		VirtualProtect( ( char* )( GameDll + GameDllConstantsOffset ), GameDllConstantsSize, PAGE_EXECUTE_READWRITE, &oldprot1 );
		memcpy( ( char* )( GameDll + GameDllConstantsOffset ), gamedllconstantsectionmemory, GameDllConstantsSize );
		FlushInstructionCache( GetCurrentProcess( ), ( char* )( GameDll + GameDllConstantsOffset ), GameDllConstantsSize );
		VirtualProtect( ( char* )( GameDll + GameDllConstantsOffset ), GameDllConstantsSize, oldprot1, &oldprot2 );

		oldprot1 = 0;
		oldprot2 = 0;
		VirtualProtect( ( char* )( GameDll + GameDllCodeOffset ), GameDllCodeSize, PAGE_EXECUTE_READWRITE, &oldprot1 );
		memcpy( ( char* )( GameDll + GameDllCodeOffset ), gamedllcodesectionmemory, GameDllCodeSize );
		FlushInstructionCache( GetCurrentProcess( ), ( char* )( GameDll + GameDllCodeOffset ), GameDllCodeSize );
		VirtualProtect( ( char* )( GameDll + GameDllCodeOffset ), GameDllCodeSize, oldprot1, &oldprot2 );


		*/

	CurrentConstantsCrc32 = 1;
	CurrentCodeCrc32 = 1;
}

// Возвращает результат сканирования
int ScanResult( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	int RetAddr = ( int )_ReturnAddress( );
	void * accessmodule = GetModuleFromAddress( RetAddr );

	if ( GameDll != 0 )
	{
		if ( AhExitThreadCode != STILL_ACTIVE && AhExitThreadCode != 0x10000
			)
		{
			//if ( IsGameDllFound( ) )
			//{
			char errorcode[ 25 ];
			sprintf_s( errorcode, "bad D->%i", AhExitThreadCode * 100 );
			CONSOLE_Print( errorcode );
			//AH_Scan( nullptr );
			//maincrash( );
			AhExitThreadCode = 0x200000;
			return ANTIHACK_VERSION + 1;
			//}
		}
		else
		{

			time_t diff = _currentsecond - _oldsecond;

			if ( diff > 4 )
			{
				CONSOLE_Print( "[Critical ah delay found! Request ban user! BAD!]" );
				return ANTIHACK_VERSION + 1;
			}

			/*char errorcode[ 25 ];
			sprintf_s( errorcode, "NOD->%i", AhExitThreadCode * 100 );
			CONSOLE_Print( errorcode );*/
		}

		if ( FoundModifiedLoader == 1 )
			FoundModifiedLoader = 0;

		CheckMaphackMems( );
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
		if ( GetFoundWhiteListMapValue( ) == 0 )
		{

#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
			vector<char> GameDllConstantSection( ( char* )( GameDll + GameDllConstantsOffset ), ( char* )( GameDll + GameDllConstantsOffset + GameDllConstantsSize ) );
			vector<char> GameDllCodeSection( ( char* )( GameDll + GameDllCodeOffset ), ( char* )( GameDll + GameDllCodeOffset + GameDllCodeSize ) );
			if ( CurrentConstantsCrc32 == 1 )
				CurrentConstantsCrc32 = GetXXHash( GameDllConstantSection.data( ), GameDllConstantSection.size( ) );
			else if ( CurrentConstantsCrc32 != GetXXHash( GameDllConstantSection.data( ), GameDllConstantSection.size( ) ) )
			{
				//CurrentConstantsCrc32 = 0;
				//if ( GetFoundWhiteListMapValue( ) == 0 )
					FoundModifiedMemoryConstants = true;
			}
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
			GameDllConstantSection.clear( );
			if ( CurrentCodeCrc32 == 1 )
				CurrentCodeCrc32 = GetXXHash( GameDllCodeSection.data( ), GameDllCodeSection.size( ) );
			else if ( CurrentCodeCrc32 != GetXXHash( GameDllCodeSection.data( ), GameDllCodeSection.size( ) ) )
			{
				//CurrentCodeCrc32 = 0;
				//if ( GetFoundWhiteListMapValue( ) == 0 )
					FoundModifiedMemoryCode = true;
			}
			GameDllCodeSection.clear( );
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
			if ( accessmodule != AntihackModule )
			{
				FoundModifiedLoader = 2;
				CONSOLE_Print( "BAD2:" );
			}
		}

#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
		if ( FoundModifiedLoader )
			return ANTIHACK_VERSION + 2;

		if ( FoundModifiedMemoryConstants )
			return ANTIHACK_VERSION + 3;

		if ( FoundModifiedGameDll )
			return ANTIHACK_VERSION + 4;

		if ( FoundModifiedMemoryCode )
			return ANTIHACK_VERSION + 5;

		if ( FoundModifiedAhScanner )
			return ANTIHACK_VERSION + 6;

		if ( FoundCodeInjection )
			return ANTIHACK_VERSION + 7;

		if ( TimeForInit > 15000 )
			return ANTIHACK_VERSION + 8;

		if ( FoundFakeVtable )
			return ANTIHACK_VERSION + 9;
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
		GetExitCodeThread( hAH_Scanner_Thread, &AhExitThreadCode );

		return ANTIHACK_VERSION;
	}
	else
	{
		CONSOLE_Print( "Bad game dll path" );
	}

	return 0;
}

char * gamedll_g = "G";
char * gamedll_w = "W";
char * gamedll_e = "e";
char * gamedll_l = "l";
char * gamedll_r = "r";
char * gamedll_c = "r";
char * gamedll_s = "r";
char * gamedll_i = "r";
char * gamedll_a = "a";
char * gamedll_po = ".";
char * gamedll_d = "d";
char * gamedll_m = "m";
char * gamedll__ = "_";

std::string GetGameDllName( )
{


	return
		string( gamedll_w ) +
		string( gamedll_a ) +
		string( gamedll_r ) +
		string( gamedll_c ) +
		string( gamedll_i ) +
		string( gamedll_s ) +
		string( gamedll_g ) +
		string( gamedll_a ) +
		string( gamedll_m ) +
		string( gamedll_e ) +
		string( gamedll_po ) +
		string( gamedll_d ) +
		string( gamedll_l ) +
		string( gamedll_l );
}


std::string GetGameDllNameOLD( )
{
	char * gamedll_g = "G";
	char * gamedll_l = "l";
	char * gamedll_a = "a";
	char * gamedll_po = ".";
	char * gamedll_d = "d";
	char * gamedll_m = "m";
	char * gamedll_e = "e";

	return string( gamedll_g ) +
		string( gamedll_a ) +
		string( gamedll_m ) +
		string( gamedll_e ) +
		string( gamedll_po ) +
		string( gamedll_d ) +
		string( gamedll_l ) +
		string( gamedll_l );
}

typedef BOOL( __stdcall * pSetCurrentDirectoryA )( LPCSTR path );
pSetCurrentDirectoryA pSetCurrentDirectoryA_org;
pSetCurrentDirectoryA pSetCurrentDirectoryA_ptr;

BOOL __stdcall pSetCurrentDirectoryA_my( LPCSTR path )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	CONSOLE_Print( "[NeedCurDirA]: " + string( path ) );
	return TRUE;
}

typedef BOOL( __stdcall * pSetCurrentDirectoryW )( LPCWSTR path );
pSetCurrentDirectoryW pSetCurrentDirectoryW_org;
pSetCurrentDirectoryW pSetCurrentDirectoryW_ptr;

BOOL __stdcall pSetCurrentDirectoryW_my( LPCWSTR path )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	CONSOLE_Print( L"[NeedCurDirA]: " + wstring( path ) );
	return TRUE;
}

typedef BOOL( __stdcall * pGetModuleFileNameA )( HMODULE mdl, LPSTR filename, DWORD nSize );
pGetModuleFileNameA pGetModuleFileNameA_org;
pGetModuleFileNameA pGetModuleFileNameA_ptr;

std::string ConvertFromUtf16ToUtf8( const std::wstring& wstr )
{
	std::string convertedString;
	int requiredSize = WideCharToMultiByte( CP_UTF8, 0, wstr.c_str( ), -1, 0, 0, 0, 0 );
	if ( requiredSize > 0 )
	{
		std::vector<char> buffer( requiredSize );
		WideCharToMultiByte( CP_UTF8, 0, wstr.c_str( ), -1, &buffer[ 0 ], requiredSize, 0, 0 );
		convertedString.assign( buffer.begin( ), buffer.end( ) - 1 );
	}
	return convertedString;
}


std::string ConvertFromUtf16ToUtf888( const std::wstring& wstr )
{
	std::string convertedString;
	int requiredSize = WideCharToMultiByte( CP_UTF8, 0, wstr.c_str( ), -1, 0, 0, "_", 0 );
	if ( requiredSize > 0 )
	{
		std::vector<char> buffer( requiredSize );
		WideCharToMultiByte( CP_UTF8, 0, wstr.c_str( ), -1, &buffer[ 0 ], requiredSize, "_", 0 );
		convertedString.assign( buffer.begin( ), buffer.end( ) - 1 );
	}
	return convertedString;
}

std::wstring ConvertFromUtf8ToUtf16( const std::string& str )
{
	std::wstring convertedString;
	int requiredSize = MultiByteToWideChar( CP_UTF8, 0, str.c_str( ), -1, 0, 0 );
	if ( requiredSize > 0 )
	{
		std::vector<wchar_t> buffer( requiredSize );
		MultiByteToWideChar( CP_UTF8, 0, str.c_str( ), -1, &buffer[ 0 ], requiredSize );
		convertedString.assign( buffer.begin( ), buffer.end( ) - 1 );
	}

	return convertedString;
}



//BOOL __stdcall pGetModuleFileNameA_my( HMODULE mdl, LPSTR filename, DWORD nSize )
//{
//	DWORD retval = pGetModuleFileNameA_ptr( mdl, filename, nSize );
//
//	//MessageBox( 0, filename, "0", 0 );
//	CONSOLE_Print( string( "In module filename:" ) + filename );
//
//	if ( filename[ 0 ] != '\0' )
//	{
//		//MessageBox( 0, filename, "1", 0 );
//		string outpath = filename;
//		CONSOLE_Print( string( "only filename:" ) + ToLower( fs::path( outpath ).filename( ).string( ) ) );
//
//		if ( ToLower( fs::path( outpath ).filename( ).string( ) ) == "wc3.exe" )
//		{
//			filename[ 0 ] = '\0';
//			retval =  pGetModuleFileNameA_ptr( StormMdl, filename, nSize );
//		}
//	}
//	CONSOLE_Print( string( "Out module filename:" ) + filename );
//
//	//MessageBox( 0, filename, "1", 0 );
//
//	return retval;
//}


bool FileExists( const std::string& name ) {
	ifstream f( name.c_str( ) );
	return f.good( );
}

bool FileExists( const std::wstring& name ) {
	ifstream f( name.c_str( ) );
	return f.good( );
}



BOOL __stdcall pGetModuleFileNameA_my( HMODULE mdl, LPSTR filename, DWORD nSize )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	DWORD retval = pGetModuleFileNameA_ptr( mdl, filename, nSize );

	//MessageBox( 0, filename, "0", 0 );
//	CONSOLE_Print( string( "In module filename:" ) + filename );

	if ( filename[ 0 ] != '\0' )
	{
		//MessageBox( 0, filename, "1", 0 );
		string outpath = filename;
		//	CONSOLE_Print( string( "only filename:" ) + ToLower( fs::path( outpath ).filename( ).string( ) ) );

		if ( ToLower( fs::path( outpath ).filename( ).string( ) ) == "wc3.exe" )
		{
			outpath = ".\\" + fs::path( outpath ).filename( ).string( );
			/*outpath = CurrentPath_old + "\\wc3.exe";
			CONSOLE_Print( "wc3.exe == yes. New path: " + outpath );
			if ( !FileExists( outpath ) )
			{
				outpath = CurrentPath + "\\wc3.exe";
				CONSOLE_Print( "BAAAD " + outpath );
			}
			if ( !FileExists( outpath ) )
			{
				CONSOLE_Print( "BAAAD2 " + outpath );
				outpath = ( fs::path( CurrentPathW ) / fs::path( "wc3.exe" ) ).string();
			}

			if ( !FileExists( outpath ) )
			{
				CONSOLE_Print( "BAAAD3 " + outpath );

			}*/
			//snprintf( filename, nSize, "%s", tmpoutsr.c_str( ) );
			//snprintf( filename, nSize, "%s\\wc3.exe", CurrentPath.c_str( ) );//, War3Path.c_str( ) );
		}
		else
			;//	CONSOLE_Print( "wc3.exe == no" );

		//string tmppath = outpath;

		if ( outpath.find( AMH_Path ) != string::npos )
		{
			outpath = ".\\" + fs::path( outpath ).filename( ).string( );
		}

		/*replaceAll( outpath, AMH_Path, CurrentPath_old );
		if ( !FileExists( outpath ) )
		{
			CONSOLE_Print( "BAAAD[2] 1 " + outpath );
			outpath = tmppath;
			replaceAll( outpath, AMH_Path, CurrentPath );
		}
		if ( !FileExists( outpath ) )
		{
			CONSOLE_Print( "BAAAD[2] 2 " + outpath );
			outpath = tmppath;
			replaceAll( outpath, AMH_Path_old, CurrentPath );
		}
		if ( !FileExists( outpath ) )
		{
			CONSOLE_Print( "BAAAD[2] 3 " + outpath );
			outpath = tmppath;
			replaceAll( outpath, AMH_Path_old, CurrentPath_old );
		}
		if ( !FileExists( outpath ) )
		{
			CONSOLE_Print( "BAAAD[2] 4 " + outpath );
			outpath = tmppath;
		}
*/
		memcpy( filename, outpath.c_str( ), outpath.size( ) + 1 );
		//snprintf( filename, nSize, "%s", outpath.c_str( ) );

	}
	//CONSOLE_Print( string( "Out module filename:" ) + filename );

	//MessageBox( 0, filename, "1", 0 );

	return retval;
}

unsigned long TimeForInit = 0;

typedef UINT( WINAPI * pGetTempFileName )(
	_In_  LPCTSTR lpPathName,
	_In_  LPCTSTR lpPrefixString,
	_In_  UINT    uUnique,
	_Out_ LPTSTR  lpTempFileName
	);

pGetTempFileName pGetTempFileName_org;
pGetTempFileName pGetTempFileName_ptr;
pGetTempFileName pGetTempFileName2_org;
pGetTempFileName pGetTempFileName2_ptr;

UINT WINAPI GetTempFileName_my(
	_In_  LPCTSTR lpPathName,
	_In_  LPCTSTR lpPrefixString,
	_In_  UINT    uUnique,
	_Out_ LPTSTR  lpTempFileName
)
{
	__asm xor eax, eax;
	return 0;
}

UINT WINAPI GetTempFileName2_my(
	_In_  LPCTSTR lpPathName,
	_In_  LPCTSTR lpPrefixString,
	_In_  UINT    uUnique,
	_Out_ LPTSTR  lpTempFileName
)
{
	__asm xor eax, eax;
	return 0;
}

// Запуск античита, все изменения лучше вносить в CustomFeaturesInitialize
void InitAh( const char * launcherpath )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif



	//	GetModuleFileNameA;
	int RetAddr = ( int )_ReturnAddress( );
	void * accessmodule = GetModuleFromAddress( RetAddr );
	if ( accessmodule != AntihackModule )
	{
		FoundModifiedLoader = 2;
		CONSOLE_Print( "BAD3:" );
	}


	CONSOLE_Print( "1" );

	if ( GameDll != 0 )
	{
		FoundModifiedGameDll = true;
		if ( ( CurrentGameDllCRC32 = GetFileXXHash( GetModuleFilePathW( GetModuleFromAddress( GameDll ) ) ) ) == GameDll127aCRC32 )
		{
			FoundModifiedGameDll = false;
		}
		else
		{
			// Тут снять коммент для получения CRC32 Game.dll новой при необходимости
			CONSOLE_Print( "GameDllBadCrc:" + to_string( GetFileXXHash( GetModuleFilePathW( GetModuleFromAddress( GameDll ) ) ) ) );
			FoundModifiedGameDll = true;
		}
		//CONSOLE_Print( "2" );

		if ( FoundModifiedGameDll )
		{
			if ( !GetModuleHandle( GetGameDllName( ).c_str( ) ) )
			{
				FoundModifiedGameDll = true;
				CONSOLE_Print( "bad2x1" );
			}
		}
		else
		{
			if ( GetModuleHandle( GetGameDllNameOLD( ).c_str( ) ) )
			{
				FoundModifiedGameDll = true;
				CONSOLE_Print( "bad2x2" );
			}
		}
		
		//CONSOLE_Print( "3" );

		CustomFeaturesInitialize( launcherpath );

		CONSOLE_Print( "x4:" );
		CONSOLE_Print( std::to_string( ( DWORD )BaseThreadInitThunk_org ) );
		CONSOLE_Print( std::to_string( ( DWORD )LoadLibraryA_org ) );
		CONSOLE_Print( std::to_string( ( DWORD )LoadLibraryW_org ) );
		CONSOLE_Print( std::to_string( ( DWORD )pLdrLoadDll_org ) );

		if ( BaseThreadInitThunk_org )
		{
			MH_CreateHook( BaseThreadInitThunk_org, &BaseThreadInitThunk_my, reinterpret_cast< void** >( &BaseThreadInitThunk_ptr ) );
			MH_EnableHook( BaseThreadInitThunk_org );
		}

		MH_CreateHook( LoadLibraryA_org, &LoadLibraryA_my, reinterpret_cast< void** >( &LoadLibraryA_ptr ) );
		MH_EnableHook( LoadLibraryA_org );

		MH_CreateHook( LoadLibraryW_org, &LoadLibraryW_my, reinterpret_cast< void** >( &LoadLibraryW_ptr ) );
		MH_EnableHook( LoadLibraryW_org );
		FreeLibrary_org = ( FreeLibrary_p )GetProcAddress_ptr( GetModuleHandle( "Kernel32.dll" ), "FreeLibrary" );
		MH_CreateHook( FreeLibrary_org, &FreeLibrary_my, reinterpret_cast< void** >( &FreeLibrary_ptr ) );
		MH_EnableHook( FreeLibrary_org );
		CONSOLE_Print( "x2:" );
		pGetTempFileName_org = ( pGetTempFileName )GetProcAddress_ptr( GetModuleHandle( "Kernel32.dll" ), "GetTempFileNameA" );

		CONSOLE_Print( std::to_string( ( DWORD )pGetTempFileName_org ) );

		MH_CreateHook( pGetTempFileName_org, &GetTempFileName_my, reinterpret_cast< void** >( &pGetTempFileName_ptr ) );
		MH_EnableHook( pGetTempFileName_org );

		pDrawUnitAtMainMap_org = ( pDrawUnitAtMainMap )( GameDll + 0x60FCE0 );
		MH_CreateHook( pDrawUnitAtMainMap_org, &pDrawUnitAtMainMap_my, reinterpret_cast< void** >( &pDrawUnitAtMainMap_ptr ) );
		MH_EnableHook( pDrawUnitAtMainMap_org );
		AddNewAhChecks( ( LPVOID )pDrawUnitAtMainMap_my, 20 );

		AddNewAhChecks( ( LPVOID )GetTempFileName_my, 5 );
		AddNewAhChecks( ( LPVOID )pGetTempFileName_org, 5 );


		pGetTempFileName2_org = ( pGetTempFileName )GetProcAddress_ptr( GetModuleHandle( "Kernel32.dll" ), "GetTempFileNameW" );
		CONSOLE_Print( std::to_string( ( DWORD )pGetTempFileName2_org ) );

		MH_CreateHook( pGetTempFileName2_org, &GetTempFileName2_my, reinterpret_cast< void** >( &pGetTempFileName2_ptr ) );
		MH_EnableHook( pGetTempFileName2_org );

		AddNewAhChecks( ( LPVOID )GetTempFileName2_my, 5 );
		AddNewAhChecks( ( LPVOID )pGetTempFileName2_org, 5 );



		pSetCurrentDirectoryA_org = ( pSetCurrentDirectoryA )( GetProcAddress_ptr( GetModuleHandle( "Kernel32.dll" ), "SetCurrentDirectoryA" ) );
		pSetCurrentDirectoryW_org = ( pSetCurrentDirectoryW )( GetProcAddress_ptr( GetModuleHandle( "Kernel32.dll" ), "SetCurrentDirectoryW" ) );
		MH_CreateHook( pSetCurrentDirectoryA_org, &pSetCurrentDirectoryA_my, reinterpret_cast< void** >( &pSetCurrentDirectoryA_ptr ) );
		MH_EnableHook( pSetCurrentDirectoryA_org );

		MH_CreateHook( pSetCurrentDirectoryW_org, &pSetCurrentDirectoryW_my, reinterpret_cast< void** >( &pSetCurrentDirectoryW_ptr ) );
		MH_EnableHook( pSetCurrentDirectoryW_org );

		pGetModuleFileNameA_org = ( pGetModuleFileNameA )( GetProcAddress_ptr( GetModuleHandle( "Kernel32.dll" ), "GetModuleFileNameA" ) );
		CONSOLE_Print( "modulefile:" + to_string( ( int )pGetModuleFileNameA_org ) );

		MH_CreateHook( pGetModuleFileNameA_org, &pGetModuleFileNameA_my, reinterpret_cast< void** >( &pGetModuleFileNameA_ptr ) );
		MH_EnableHook( pGetModuleFileNameA_org );

		//CONSOLE_Print( "lld:" + to_string( ( int )pLdrLoadDll_org ) );
		if ( pLdrLoadDll_org )
		{
			MH_CreateHook( pLdrLoadDll_org, &pLdrLoadDll_my, reinterpret_cast< void** >( &pLdrLoadDll_ptr ) );
			MH_EnableHook( pLdrLoadDll_org );

			AddNewAhChecks( ( LPVOID )pLdrLoadDll_org, 150 );
			AddNewAhChecks( ( LPVOID )pLdrLoadDll_my, 150 );

		}
		if ( BaseThreadInitThunk_org )
		{
			AddNewAhChecks( ( LPVOID )BaseThreadInitThunk_org, 150 );
			AddNewAhChecks( ( LPVOID )BaseThreadInitThunk_my, 150 );
		}

		AddNewAhChecks( ( LPVOID )LoadLibraryA_org, 150 );
		AddNewAhChecks( ( LPVOID )LoadLibraryA_my, 150 );

		AddNewAhChecks( ( LPVOID )LoadLibraryW_org, 150 );
		AddNewAhChecks( ( LPVOID )LoadLibraryW_my, 150 );




		AddNewAhChecks( ( LPVOID )ScanResult, 150 );
		GameGetFile_org = ( GameGetFile )( GameDll + 0x4C1550 ); // 127a 0x048C10

		MH_CreateHook( GameGetFile_org, &GameGetFile_my, reinterpret_cast< void** >( &GameGetFile_ptr ) );
		MH_EnableHook( GameGetFile_org );

		DWORD oldprot;
		VirtualProtect( InitAh, 1, PAGE_READWRITE, &oldprot );
		*( int* )InitAh = 0xC3;

		InitGameDllBackupMemory( );

		CurrentConstantsCrc32 = 1;
		CurrentCodeCrc32 = 1;


		CONSOLE_Print( "OK:" + to_string( TimeForInit ) );
	}
	else
	{
		FoundModifiedLoader = 2;
		DWORD oldprot;
		VirtualProtect( InitAh, 1, PAGE_READWRITE, &oldprot );
		*( int* )InitAh = 0xC3;
		CONSOLE_Print( "BAD:" + to_string( TimeForInit ) );
	}





}


string ToLower( string s )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	if ( !s.empty( ) && s.length( ) >= 1 )
	{
		std::transform( s.begin( ), s.end( ), s.begin( ), tolower );
		return s;
	}
	return "";
}

GameGetFile GameGetFile_org;
GameGetFile GameGetFile_ptr;

bool FoundBadMap = false;

DWORD LastBadMapFoundTime = 0;

BOOL InBattleNet = FALSE;

std::regex badmaptest1( "native\\s+MergeUnits" );
std::regex badmaptest2( "native\\s+IgnoredUnits" );



void ApplyFilterIfNeed( std::string filename, int * outdata, unsigned int * outsize )
{


#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	char * indata = ( char * )*outdata;
	if ( !indata )
		return;


	unsigned int insize = *outsize;

	if ( !insize || insize < 22 )
		return;

	std::string fileext = ToLower( fs::path( filename ).extension( ).string( ) );
	BOOL IsBlp = memcmp( ( LPCVOID )indata, "BLP1", 4 ) == 0;

	std::string filenamelower = ToLower( filename );

	if ( strstr( filenamelower.c_str( ), "abilities" ) == filenamelower.c_str( ) ||
		strstr( filenamelower.c_str( ), "teamcolor" ) != NULL ||
		strstr( filenamelower.c_str( ), "teamglow" ) != NULL )
	{
		return;
	}

	if ( gInfo.GrayScaleWorld == 5 || gInfo.GrayScaleWorld == 6 )
	{
		if ( strstr( filenamelower.c_str( ), "terrainart" ) == filenamelower.c_str( ) ||
			strstr( filenamelower.c_str( ), "replaceabletextures\\cliff" ) == filenamelower.c_str( ) )
		{


		}
		else
		{
			return;
		}
	}


	if ( !IsBlp )
	{
		StormBuffer outbuf;
		int w = 0;
		int h = 0;
		int bpp = 4;
		unsigned int outbufsize = TGA2Raw( StormBuffer( indata, insize ), outbuf, w, h, bpp, filename.c_str( ) );
		if ( outbufsize > 0 && w && h )
		{
			RGBAPix * pixels = ( RGBAPix* )outbuf.buf;

			if ( gInfo.GrayScaleWorld == 7 )
			{
				FCObjImage curimg;
				curimg.Create( w, h, bpp * 8 );
				memcpy( curimg.GetMemStart( ), outbuf.buf, outbuf.length );
				curimg.ApplyEffect( FCEffectPosterize( 3 ) );
				memcpy( outbuf.buf, curimg.GetMemStart( ), outbuf.length );
				curimg.Destroy( );
			}
			else if ( gInfo.GrayScaleWorld == 8 )
			{
				FCObjImage curimg;
				curimg.Create( w, h, bpp * 8 );
				memcpy( curimg.GetMemStart( ), outbuf.buf, outbuf.length );
				RGBQUAD color;
				color.rgbBlue = 255;
				color.rgbGreen = 100;
				color.rgbRed = 0;
				color.rgbReserved = 0;
				curimg.ApplyEffect( FCEffectColorTone( color, 240 ) );
				memcpy( outbuf.buf, curimg.GetMemStart( ), outbuf.length );
				curimg.Destroy( );
			}
			else if ( gInfo.GrayScaleWorld == 9 )
			{
				FCObjImage curimg;
				curimg.Create( w, h, bpp * 8 );
				memcpy( curimg.GetMemStart( ), outbuf.buf, outbuf.length );
				RGBQUAD color;
				color.rgbBlue = 10;
				color.rgbGreen = 10;
				color.rgbRed = 255;
				color.rgbReserved = 0;
				curimg.ApplyEffect( FCEffectColorTone( color, 240 ) );
				memcpy( outbuf.buf, curimg.GetMemStart( ), outbuf.length );
				curimg.Destroy( );
			}
			else if ( gInfo.GrayScaleWorld == 10 )
			{
				FCObjImage curimg;
				curimg.Create( w, h, bpp * 8 );
				memcpy( curimg.GetMemStart( ), outbuf.buf, outbuf.length );
				RGBQUAD color;
				color.rgbBlue = 10;
				color.rgbGreen = 255;
				color.rgbRed = 10;
				color.rgbReserved = 0;
				curimg.ApplyEffect( FCEffectColorTone( color, 240 ) );
				memcpy( outbuf.buf, curimg.GetMemStart( ), outbuf.length );
				curimg.Destroy( );
			}
			else if ( gInfo.GrayScaleWorld == 11 )
			{
				FCObjImage curimg;
				curimg.Create( w, h, bpp * 8 );
				memcpy( curimg.GetMemStart( ), outbuf.buf, outbuf.length );
				RGBQUAD color;
				color.rgbBlue = 10;
				color.rgbGreen = 255;
				color.rgbRed = 10;
				color.rgbReserved = 0;
				curimg.ApplyEffect( FCEffectSoftPortrait( 50, 200, 200 ) );
				memcpy( outbuf.buf, curimg.GetMemStart( ), outbuf.length );
				curimg.Destroy( );
			}
			else
			{
				for ( int i = 0; i < w*h; i++ )
				{
					unsigned char & Red = pixels[ i ].R;
					unsigned char & Green = pixels[ i ].G;
					unsigned char & Blue = pixels[ i ].B;

					if ( gInfo.GrayScaleWorld == 1 )
					{
						unsigned char graycolor = ( unsigned char )( ( float )( Red + Green + Blue ) / 3.0f );
						Red = Green = Blue = graycolor;
					}
					else if ( gInfo.GrayScaleWorld == 2 )
					{
						Green = NormalizeComponent( Green + ( Blue / 3 ) );
						Blue = Blue / 3;
					}
					else if ( gInfo.GrayScaleWorld == 3 )
					{
						Red = NormalizeComponent( Red + ( Blue / 3 ) );
						Blue = Blue / 3;
					}
					else if ( gInfo.GrayScaleWorld == 4 )
					{
						unsigned char outputRed = NormalizeComponent( ( Red * .393 ) + ( Green *.769 ) + ( Blue * .189 ) );
						unsigned char outputGreen = NormalizeComponent( ( Red * .349 ) + ( Green *.686 ) + ( Blue * .168 ) );
						unsigned char outputBlue = NormalizeComponent( ( Red * .272 ) + ( Green *.534 ) + ( Blue * .131 ) );
						Red = outputRed;
						Green = outputGreen;
						Blue = outputBlue;
					}
					else if ( gInfo.GrayScaleWorld == 5 )
					{
						if ( ( Green > 40 || Blue > 40 || Red > 40 ) )
						{
							if ( Red < 235 && Green < 235 && Blue < 235 )
							{
								Red += 25;
								Green += 25;
								Blue += 25;
							}
						}
						else if ( TRUE )
						{
							if ( Red > 0 && Red < 250 )
								Red += 5;
							if ( Green > 0 && Green < 250 )
								Green += 5;
							if ( Blue > 0 && Blue < 250 )
								Blue += 5;
						}

					}
					else if ( gInfo.GrayScaleWorld == 6 )
					{
						unsigned char graycolor = ( unsigned char )( ( float )( Red + Green + Blue ) / 3.0f / 3.0 );
						Red = Green = Blue = graycolor;
					}
				}
			}
			RAW2Tga( outbuf, outbuf, w, h, bpp, filename.c_str( ) );

			*outdata = ( int )outbuf.buf;
			*outsize = outbuf.length;
		}

	}
	else /*if ( IsBlp )*/
	{

		StormBuffer outbuf;
		int w = 0;
		int h = 0;
		int bpp = 4;
		int mipmaps = 0;
		int alpha = 0;
		int compresstype = 0;
		int pictype = 0;
		unsigned int outbufsize = Blp2Raw( StormBuffer( indata, insize ), outbuf, w, h, bpp, mipmaps, alpha, compresstype, pictype, filename.c_str( ) );
		if ( outbufsize > 0 && w && h )
		{
			BGRAPix * pixels = ( BGRAPix* )outbuf.buf;
			int alphax = 0;
			unsigned char graycolorlatest = 0;
			if ( gInfo.GrayScaleWorld == 7 )
			{
				FCObjImage curimg;
				curimg.Create( w, h, bpp * 8 );
				memcpy( curimg.GetMemStart( ), outbuf.buf, outbuf.length );
				curimg.ApplyEffect( FCEffectPosterize( 2 ) );
				memcpy( outbuf.buf, curimg.GetMemStart( ), outbuf.length );
				curimg.Destroy( );
			}
			else if ( gInfo.GrayScaleWorld == 8 )
			{
				FCObjImage curimg;
				curimg.Create( w, h, bpp * 8 );
				memcpy( curimg.GetMemStart( ), outbuf.buf, outbuf.length );
				RGBQUAD color;
				color.rgbBlue = 255;
				color.rgbGreen = 100;
				color.rgbRed = 0;
				color.rgbReserved = 0;
				curimg.ApplyEffect( FCEffectColorTone( color, 240 ) );
				memcpy( outbuf.buf, curimg.GetMemStart( ), outbuf.length );
				curimg.Destroy( );
			}
			else if ( gInfo.GrayScaleWorld == 9 )
			{
				FCObjImage curimg;
				curimg.Create( w, h, bpp * 8 );
				memcpy( curimg.GetMemStart( ), outbuf.buf, outbuf.length );
				RGBQUAD color;
				color.rgbBlue = 10;
				color.rgbGreen = 10;
				color.rgbRed = 255;
				color.rgbReserved = 0;
				curimg.ApplyEffect( FCEffectColorTone( color, 240 ) );
				memcpy( outbuf.buf, curimg.GetMemStart( ), outbuf.length );
				curimg.Destroy( );
			}
			else if ( gInfo.GrayScaleWorld == 10 )
			{
				FCObjImage curimg;
				curimg.Create( w, h, bpp * 8 );
				memcpy( curimg.GetMemStart( ), outbuf.buf, outbuf.length );
				RGBQUAD color;
				color.rgbBlue = 10;
				color.rgbGreen = 255;
				color.rgbRed = 10;
				color.rgbReserved = 0;
				curimg.ApplyEffect( FCEffectColorTone( color, 240 ) );
				memcpy( outbuf.buf, curimg.GetMemStart( ), outbuf.length );
				curimg.Destroy( );
			}
			else if ( gInfo.GrayScaleWorld == 11 )
			{
				FCObjImage curimg;
				curimg.Create( w, h, bpp * 8 );
				memcpy( curimg.GetMemStart( ), outbuf.buf, outbuf.length );
				RGBQUAD color;
				color.rgbBlue = 10;
				color.rgbGreen = 255;
				color.rgbRed = 10;
				color.rgbReserved = 0;
				curimg.ApplyEffect( FCEffectSoftPortrait( 50, 200, 200 ) );
				memcpy( outbuf.buf, curimg.GetMemStart( ), outbuf.length );
				curimg.Destroy( );
			}
			else
			{
				for ( int i = 0; i < w*h; i++ )
				{
					unsigned char & Red = pixels[ i ].R;
					unsigned char & Green = pixels[ i ].G;
					unsigned char & Blue = pixels[ i ].B;
					if ( gInfo.GrayScaleWorld == 1 )
					{
						unsigned char graycolor = ( unsigned char )( ( float )( Red + Green + Blue ) / 3.0f );
						Red = Green = Blue = graycolor;
					}
					else if ( gInfo.GrayScaleWorld == 2 )
					{
						Green = NormalizeComponent( Green + ( Blue / 3 ) );
						Blue = Blue / 3;
					}
					else if ( gInfo.GrayScaleWorld == 3 )
					{
						Red = NormalizeComponent( Red + ( Blue / 3 ) );
						Blue = Blue / 3;
					}
					else if ( gInfo.GrayScaleWorld == 4 )
					{
						unsigned char outputRed = NormalizeComponent( ( Red * .393 ) + ( Green *.769 ) + ( Blue * .189 ) );
						unsigned char outputGreen = NormalizeComponent( ( Red * .349 ) + ( Green *.686 ) + ( Blue * .168 ) );
						unsigned char outputBlue = NormalizeComponent( ( Red * .272 ) + ( Green *.534 ) + ( Blue * .131 ) );
						Red = outputRed;
						Green = outputGreen;
						Blue = outputBlue;
					}
					else if ( gInfo.GrayScaleWorld == 5 )
					{
						if ( ( Green > 40 || Blue > 40 || Red > 40 ) )
						{
							if ( Red < 235 && Green < 235 && Blue < 235 )
							{
								Red += 25;
								Green += 25;
								Blue += 25;
							}
						}
						else if ( TRUE )
						{
							if ( Red > 0 && Red < 250 )
								Red += 5;
							if ( Green > 0 && Green < 250 )
								Green += 5;
							if ( Blue > 0 && Blue < 250 )
								Blue += 5;
						}

					}
					else if ( gInfo.GrayScaleWorld == 6 )
					{
						unsigned char graycolor = ( unsigned char )( ( float )( Red + Green + Blue ) / 3.0f / 3.0 );
						Red = Green = Blue = graycolor;
					}
				}
			}

			//if ( alpha != 8 && alpha != 0 )
			//{
			//	for ( int i = 0; i < w*h; i++ )
			//	{
			//		if ( pixels[ i ].A != 255 )
			//			
			//	}
			//}
			alpha = 8;
			bpp = 4;
			/*if ( alpha == 8 )
			{
				for ( int i = 0; i < w*h; i++ )
				{
					pixels[ i ].A = 0;
				}
			}*/

			CreatePalettedBLP( outbuf, outbuf, 255, filename.c_str( ), w, h, bpp, alpha, mipmaps );

			/*		if ( compresstype == 0 && alpha == 6 )
					{
						outbuf.Clear( );
					}
					else
					{*/
			*outdata = ( int )outbuf.buf;
			*outsize = outbuf.length;/*


			if ( maximages >= 0 && maximages < 5 )
			{

				DumpFileToDisk( ( char* )*outdata, *outsize, ( to_string( *outdata ) + to_string( *outsize ) + "_alpha" + to_string( alpha ) + ".blp" ).c_str( ) );

			}*/
			/*}*/
		}
	}

}

BOOL SkipAllFiles = FALSE;

// Перехваченная функция GameGetFile которая считывает файлы из архива карты.
BOOL __fastcall GameGetFile_my( const char * filename, int * OutDataPointer, unsigned int * OutSize, BOOL unknown )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	int RetAddr = ( int )_ReturnAddress( );
	void * accessmodule = GetModuleFromAddress( RetAddr );

	/*	char buftest[ 200 ];
		sprintf_s( buftest, "Ret addr:%X, Module base:%X", RetAddr, accessmodule );
		*/
	if ( !filename || SkipAllFiles )
		return FALSE;

	std::string LowFileName = ToLower( filename );

	if ( !IsGame( ) )
	{
		/*if ( IsKeyPressed( '9' ) )
		{
			MessageBox( 0, filename, filename, 0 );
		}*/

		for ( auto s : ListMapInfos )
		{
#ifndef  ANTIHACKNODEBUG
			AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
			if ( LowFileName == ToLower( s.minimapfilename ) )
			{
				*OutDataPointer = ( int )s.MiniMapData;
				*OutSize = s.MiniMapDataSize;
				return TRUE;
			}

			if ( LowFileName == ToLower( s.previewfilename ) )
			{
				*OutDataPointer = ( int )s.PreviewData;
				*OutSize = s.PreviewDataSize;
				return TRUE;
			}

		}

	}

	BOOL retval = GameGetFile_ptr( filename, OutDataPointer, OutSize, unknown );

	if ( gInfo.GrayScaleWorld > 0 && retval && filename && strlen( filename ) > 4 )
	{
		ApplyFilterIfNeed( filename, OutDataPointer, OutSize );
	}

	if ( !IsGame( ) )
	{
#ifndef  ANTIHACKNODEBUG
		AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
		int filenamelen = strlen( filename );
		if ( retval && filenamelen > 2 && ToLower( filename ) == "war3map.j" )
		{
			//CONSOLE_Print( "Load script:" + string( filename ) );

			FoundBadMap = false;

			char * war3mapdata = ( char * )*OutDataPointer;
			if ( *OutSize > 25 )
			{
				if ( LastBadMapFoundTime + 3000 > GetTickCount( ) )
				{
					war3mapdata[ 0 ] = '\0';
					*OutSize = 1;
					retval = FALSE;
					CONSOLE_Print( "Ret bad" );
					return retval;
				}


				if ( strstr( war3mapdata, "native" ) && ( std::regex_search( war3mapdata, badmaptest1 ) || std::regex_search( war3mapdata, badmaptest2 ) ) )
				{
					FoundBadMap = true;

					uint32_t mapcrc32 = crc32_16bytes_prefetch( war3mapdata, *OutSize );

					CONSOLE_Print( "Found bad map. Crc32:" + to_string( mapcrc32 ) );

					bool map_white_listed = false;
					for ( auto mapcrc : WhiteListMaps )
					{
						if ( mapcrc == mapcrc32 )
						{
							map_white_listed = true;
							break;
						}
					}

					for ( auto maphost : MapHostList )
					{
						if ( maphost.crc32 == mapcrc32 )
						{
							map_white_listed = true;
							break;
						}
					}

					if ( !map_white_listed )
					{
						war3mapdata[ 0 ] = '\0';
						*OutSize = 1;
						retval = FALSE;
						CONSOLE_Print( "Ret bad 2" );
						LastBadMapFoundTime = GetTickCount( );
						FoundWhiteListMap = 0;
					}
					else
					{
						if ( !IsGame( ) )
							FoundWhiteListMap = 1;

						//if ( FoundWhiteListMap == 3 )
						CONSOLE_Print( "Whitelist!" );
					}

					if ( InBattleNet && wc3classgproxy != nullptr )
					{
						CWC3 * gproxy_wc3 = ( CWC3 * )wc3classgproxy;
						gproxy_wc3->SendAHPacket( ANTIHACK_VERSION, 0x33, mapcrc32, 0, 0, 0 );
					}
					else
					{
						*OutSize = 0;
					}

				}



			}

		}
	}
	return retval;
}


DWORD WarcraftThreadId = 0;

// Возвращает >0 если игра запущена
BOOL IsGame( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	int War3CurrentUIManager = *(int*)( GameDll + 0xACE66C ); 
	if ( War3CurrentUIManager )
	{
		BOOL retval = *( int* )War3CurrentUIManager == GameDll + 0x93631C;
		return retval;
	}

	return FALSE;
}


// Возвращает >0 если чат открыт
BOOL IsChat( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	return*( BOOL* )( GameDll + 0xAD15F0 );//1.27a 0xBDAA14
}

#include ".\..\WarcraftLoaderLib\functions.h"


static void hash_set_16( uint32_t * dst, unsigned char const * src, unsigned int count )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	unsigned int i;
	unsigned int pos;

	for ( pos = 0, i = 0; i < 16; i++ )
	{
		dst[ i ] = 0;
		if ( pos < count )
			dst[ i ] |= ( ( uint32_t )src[ pos ] );
		pos++;
		if ( pos < count )
			dst[ i ] |= ( ( uint32_t )src[ pos ] ) << 8;
		pos++;
		if ( pos < count )
			dst[ i ] |= ( ( uint32_t )src[ pos ] ) << 16;
		pos++;
		if ( pos < count )
			dst[ i ] |= ( ( uint32_t )src[ pos ] ) << 24;
		pos++;
	}
}


DLLEXPORT void w3l_do_hash( char *username, bnet_hash_ctx *ctx ) {
	int i;
	uint32_t     a, b, c, d, e, g;
	int32_t      tmp[ 16 + 64 ];
	const char  * password;
	char        *oldpassword;

	oldpassword = ( char * )( ctx + 1 );

	password = LastPassword.c_str( );

	memset( oldpassword, 0, 16 );
	memcpy( oldpassword, password, 15 );

	//

	hash_set_16( ( uint32_t * )tmp, ( const unsigned char * )oldpassword, strlen( oldpassword ) );

	for ( i = 0; i < 64; i++ )
		tmp[ i + 16 ] = ROTL32( 1, tmp[ i ] ^ tmp[ i + 8 ] ^ tmp[ i + 2 ] ^ tmp[ i + 13 ] );

	a = ctx->a;
	b = ctx->b;
	c = ctx->c;
	d = ctx->d;
	e = ctx->e;

	for ( i = 0; i < 20 * 1; i++ )
	{
		g = tmp[ i ] + ROTL32( a, 5 ) + e + ( ( b & c ) | ( ~b & d ) ) + 0x5a827999;
		e = d;
		d = c;
		c = ROTL32( b, 30 );
		b = a;
		a = g;
	}

	for ( ; i < 20 * 2; i++ )
	{
		g = ( d ^ c ^ b ) + e + ROTL32( g, 5 ) + tmp[ i ] + 0x6ed9eba1;
		e = d;
		d = c;
		c = ROTL32( b, 30 );
		b = a;
		a = g;
	}

	for ( ; i < 20 * 3; i++ )
	{
		g = tmp[ i ] + ROTL32( g, 5 ) + e + ( ( c & b ) | ( d & c ) | ( d & b ) ) - 0x70e44324;
		e = d;
		d = c;
		c = ROTL32( b, 30 );
		b = a;
		a = g;
	}

	for ( ; i < 20 * 4; i++ )
	{
		g = ( d ^ c ^ b ) + e + ROTL32( g, 5 ) + tmp[ i ] - 0x359d3e2a;
		e = d;
		d = c;
		c = ROTL32( b, 30 );
		b = a;
		a = g;
	}

	ctx->a += g;
	ctx->b += b;
	ctx->c += c;
	ctx->d += d;
	ctx->e += e;
}

DLLEXPORT void __fastcall w3l_hash_init( uint32_t *data ) {
	*( data + 0 ) = 0x67452301;
	*( data + 1 ) = 0xEFCDAB89;
	*( data + 2 ) = 0x98BADCFE;
	*( data + 3 ) = 0x10325476;
	*( data + 4 ) = 0xC3D2E1F0;
	*( data + 5 ) = 0;
	*( data + 6 ) = 0;
}



uint32_t GetXXHash( const char * bytes, size_t len )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	return XXH32( bytes, len, 0xABCD );
}
