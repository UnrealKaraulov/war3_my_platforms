#include <Windows.h>
#include <iostream>

#include <string>
#include <vector>
#include "CustomFeatures.h"
#include <MinHook.h>
#include "warcis_reconnector.h"
#include <thread>
#include "Antihack.h"
#include "Storm.h"

#pragma comment(lib,"Loader.lib")
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"libMinHook.x86.lib")


#include <filesystem>
namespace fs = std::filesystem;





void HideModule( HMODULE module_handle )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	DWORD p_peb_ldr_data = 0;
	__asm
	{
		pushad;
		pushfd;
		mov eax, fs:[30h];			// PEB
		mov eax, [ eax + 0Ch ];			// PEB->ProcessModuleInfo
		mov p_peb_ldr_data, eax;	// Save ProcessModuleInfo

									//in_load_order_module_list:
		mov esi, [ eax + 0Ch ];			// ProcessModuleInfo->in_load_order_module_list[FORWARD]
		mov edx, [ eax + 10h ];			//  ProcessModuleInfo->in_load_order_module_list[BACKWARD]

	loop_in_load_order_module_list:
		lodsd;						//  Load First Module
		mov esi, eax;		    	//  ESI points to Next Module
		mov ecx, [ eax + 18h ];			//  LDR_MODULE->BaseAddress
		cmp ecx, module_handle;		//  Is it Our Module ?
		jne skip_a;		    		//  If Not, Next Please (@f jumps to nearest Unamed Lable @@:)
		mov ebx, [ eax ];				//  [FORWARD] Module 
		mov ecx, [ eax + 4 ];    		//  [BACKWARD] Module
		mov[ ecx ], ebx;				//  Previous Module's [FORWARD] Notation, Points to us, Replace it with, Module++
		mov[ ebx + 4 ], ecx;			//  Next Modules, [BACKWARD] Notation, Points to us, Replace it with, Module--
		jmp in_memory_order_module_list;		//  Hidden, so Move onto Next Set
	skip_a:
		cmp edx, esi;							//  Reached End of Modules ?
		jne loop_in_load_order_module_list;		//  If Not, Re Loop

	in_memory_order_module_list:
		mov eax, p_peb_ldr_data;	  //  PEB->ProcessModuleInfo
		mov esi, [ eax + 14h ];			  //  ProcessModuleInfo->in_memory_order_module_list[START]
		mov edx, [ eax + 18h ];			  //  ProcessModuleInfo->in_memory_order_module_list[FINISH]

	loop_in_memory_order_module_list:
		lodsd;
		mov esi, eax;
		mov ecx, [ eax + 10h ];
		cmp ecx, module_handle;
		jne skip_b;
		mov ebx, [ eax ];
		mov ecx, [ eax + 4 ];
		mov[ ecx ], ebx;
		mov[ ebx + 4 ], ecx;
		jmp in_initialization_order_module_list;
	skip_b:
		cmp edx, esi;
		jne loop_in_memory_order_module_list;

	in_initialization_order_module_list:
		mov eax, p_peb_ldr_data;		//  PEB->ProcessModuleInfo
		mov esi, [ eax + 1Ch ];				//  ProcessModuleInfo->in_initialization_order_module_list[START]
		mov edx, [ eax + 20h ];				//  ProcessModuleInfo->in_initialization_order_module_list[FINISH]

	loop_in_initialization_order_module_list:
		lodsd;
		mov esi, eax;
		mov ecx, [ eax + 08h ];
		cmp ecx, module_handle;
		jne skip_c;
		mov ebx, [ eax ];
		mov ecx, [ eax + 4 ];
		mov[ ecx ], ebx;
		mov[ ebx + 4 ], ecx;
		jmp Finished;
	skip_c:
		cmp edx, esi;
		jne loop_in_initialization_order_module_list;

	Finished:
		popfd;
		popad;
	}
}

inline bool HideThread( HANDLE hThread )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	typedef NTSTATUS( NTAPI *pNtSetInformationThread )
		( HANDLE, UINT, PVOID, ULONG );
	NTSTATUS Status;

	// Get NtSetInformationThread
	pNtSetInformationThread NtSIT = ( pNtSetInformationThread )
		GetProcAddress_ptr( GetModuleHandle( "ntdll.dll" ),
			"NtSetInformationThread" );

	// Shouldn't fail
	if ( NtSIT == NULL )
		return false;

	// Set the thread info
	if ( hThread == NULL )
		Status = NtSIT( GetCurrentThread( ),
			0x11, // HideThreadFromDebugger
			0, 0 );
	else
		Status = NtSIT( hThread, 0x11, 0, 0 );

	if ( Status != 0x00000000 )
		return false;
	else
		return true;
}


HANDLE GPROXYTHREAD;

// Считывание конфига
void InitAllConfig( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	if ( gInfo.Username[ 0 ] != '\0' && gInfo.Password[ 0 ] != '\0' )
	{
		NeedAutoJoin = TRUE;
	}
	__var1 = gInfo.var1;
	__var2 = gInfo.var2;
	__var3 = gInfo.var3;
	__var4 = gInfo.var4;
	LauncherVersion = gInfo.launcher_version;
	NeedBnetSpeedUp = gInfo.NeedBnetSpeedUp;
	ShiftNumpadFix = gInfo.ShiftNumpadFix;
	WarKeyEnabled = gInfo.WarKeyEnabled;
	CopyMemory( WarKeyKeys, gInfo.WarKeyKeys, 18 * 4 );

	if ( gInfo.Bypass8MBlimit )
	{
		MapsBigger8MB( );
	}

	NeedFullScreenSwitcher = gInfo.WindowModeAltEnter;
	NeedUseNewD3dMode = gInfo.UseNewD3dMode;
	//CONSOLE_Print( "gInfo.UseNewD3dMode:" + to_string( NeedUseNewD3dMode ) );

	if ( gInfo.StatsLine[ 0 ] != '\0' )
		LineForStats = gInfo.StatsLine;
	else
		LineForStats = "PTS:{MMR} WIN:{WINS} LOSE:{LOSES}";
}


HINSTANCE hDLL;
BOOL FirstInit = TRUE;


wstring WarcisConfigPath = L"";
string War3Path = "";
string War3Path_old = "";
wstring War3Path_W = L"";
string AMH_Path = "";
wstring AMH_PathW = L"";
string AMH_Path_old = "";
string CurrentPath = "";
string CurrentPath_old = "";
wstring CurrentPathW = L"";

typedef LPTOP_LEVEL_EXCEPTION_FILTER( __stdcall * pSetUnhandledExceptionFilter )( LPTOP_LEVEL_EXCEPTION_FILTER handler );
typedef PVOID( __stdcall * pAddVectoredExceptionHandler )( ULONG first, PVECTORED_EXCEPTION_HANDLER handler );
pSetUnhandledExceptionFilter pSetUnhandledExceptionFilter_org;
pSetUnhandledExceptionFilter pSetUnhandledExceptionFilter_ptr;

pAddVectoredExceptionHandler pAddVectoredExceptionHandler_org;
pAddVectoredExceptionHandler pAddVectoredExceptionHandler_ptr;



LPTOP_LEVEL_EXCEPTION_FILTER __stdcall pSetUnhandledExceptionFilter_my( LPTOP_LEVEL_EXCEPTION_FILTER handler )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	MessageBox( 0, "1", "1", 0 );
	return 0;
}

PVOID __stdcall pAddVectoredExceptionHandler_my( ULONG first, PVECTORED_EXCEPTION_HANDLER handler )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	MessageBox( 0, "2", "2", 0 );
	return 0;
}

FARPROC GetSetUnhandledExceptionFilterPointer( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	FARPROC suef = NULL;
	// In Windows 8 SetUnhandledExceptionFilter implementation is in kernelbase
	if ( HMODULE kernelbase = GetModuleHandle( _T( "kernel32" ) ) )
		suef = GetProcAddress_ptr( kernelbase, "SetUnhandledExceptionFilter" );
	if ( !suef )
	{
		if ( HMODULE kernel32 = GetModuleHandle( _T( "kernelbase" ) ) )
			suef = GetProcAddress_ptr( kernel32, "SetUnhandledExceptionFilter" );
	}
	return suef;
}



FARPROC GetAddVectoredExceptionHandlerPointer( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	FARPROC suef = NULL;
	// In Windows 8 SetUnhandledExceptionFilter implementation is in kernelbase
	if ( HMODULE kernelbase = GetModuleHandle( _T( "kernel32" ) ) )
		suef = GetProcAddress_ptr( kernelbase, "AddVectoredExceptionHandler" );
	if ( !suef )
	{
		if ( HMODULE kernel32 = GetModuleHandle( _T( "kernelbase" ) ) )
			suef = GetProcAddress_ptr( kernel32, "AddVectoredExceptionHandler" );
	}
	return suef;
}

std::vector<ThreadInfoMy> ThreadInfoMyList;
void AddThreadInfoMy( DWORD threadid, LPCWSTR threadname )
{
	for ( auto & s : ThreadInfoMyList )
	{
		if ( threadid == s.ThreadId )
		{
			s.ThreadName = threadname;
			return;
		}
	}

	ThreadInfoMy tmpThreadInfoMy = ThreadInfoMy( );
	tmpThreadInfoMy.ThreadId = threadid;
	tmpThreadInfoMy.ThreadName = threadname;
	ThreadInfoMyList.push_back( tmpThreadInfoMy );
}

LPCWSTR GetThreadNameById( DWORD threadid )
{
	for ( auto & s : ThreadInfoMyList )
	{
		if ( threadid == s.ThreadId )
		{
			return s.ThreadName;
		}
	}

	return L"UNNAMED THREAD";
}
int CustomChatMessagesCount = 0;
std::vector<int> CustomChatKeyCodes;

// запустить всё
void __stdcall InitAllFunctions( )
{

#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	if ( !FirstInit )
	{
		CONSOLE_Print( "[Warcis_Rec] FirstInit problem." );
		return;
	}

	////CONSOLE_Print( "[Warcis_Rec] call InitAllFunctions." );

	FirstInit = FALSE;

	char TmpCurDirA[ 4096 ];
	GetCurrentDirectoryA( 4096, TmpCurDirA );


	wchar_t TmpCurDirW[ 4096 ];
	GetCurrentDirectoryW( 4096, TmpCurDirW );
	CurrentPathW = TmpCurDirW;


	CurrentPath = ConvertFromUtf16ToUtf8( TmpCurDirW );
	CurrentPath_old = TmpCurDirA;

	CONSOLE_Print( "[Warcis_Rec] CurrentPath:" + CurrentPath );
	CONSOLE_Print( "[Warcis_Rec] CurrentPath_old:" + CurrentPath_old );


	if ( !GetModuleHandle( "wc3.exe" ) )
	{
		CONSOLE_Print( "[Warcis_Rec] no main code found." );
		return;
	}
	if ( GetModuleHandle( "war3.exe" ) )
	{
		CONSOLE_Print( "[Warcis_Rec] bad main code found." );
		return;
	}

	if ( !GameDll )
	{
		FoundNoGameDll = true;
		GameDll = ( int )LoadLibraryA( GetGameDllName( ).c_str( ) );
		CONSOLE_Print( "Found no game dll" );
	}

	auto AMH_Path_PATH = fs::path( GetModuleFilePathW( hDLL ) ).parent_path( );
	AMH_PathW = AMH_Path_PATH.wstring( );

	//if ( FileExists( ".\\AMH.dll" ) )
	//{
	//	if ( !DeleteFileA( ".\\AMH.dll" ) )
	//	{
	//		FoundModifiedAhScanner = true;
	//		CONSOLE_Print( "Neurolink protection found anime attacker! " );
	//	}
	//}


	/*if ( FileExists( ".\\WarcisGame.dll" ) )
	{
		if ( !DeleteFileA( ".\\WarcisGame.dll" ) )
		{
			FoundModifiedAhScanner = true;
			CONSOLE_Print( "Neurolink protection found anime attacker #2! " );
		}
		MoveFileA( ".\\WarcisGame.dll", ".\\WarcisGame.dll.bad" );
	}
*/
	if ( !GetModuleHandle( "WarcisGame.dll" ) )
	{
		FoundModifiedAhScanner = true;
		CONSOLE_Print( "Neurolink protection found anime attacker #3! " );
	}



	if ( GetModuleHandle( "w3lh.dll" ) )
	{
		FoundModifiedAhScanner = true;
		CONSOLE_Print( "Neurolink protection found anime attacker #4! " );
	}

	//bool CanCheckSign = true;
	//if ( FileExists( ".\\AMH.dll.check" ) )
	//{
	//	if ( !DeleteFileA( ".\\AMH.dll.check"  ) )
	//	{
	//		CanCheckSign = false; 
	//	}
	//}


	//if ( CanCheckSign )
	//{
	//	CopyFileW( GetModuleFilePathW( hDLL ).c_str( ), L".\\AMH.dll.check" , FALSE);
	//	if ( FileExists( ".\\AMH.dll.check" ) )
	//	{
	//		CONSOLE_Print( "Sign check:" + to_string( VerifyEmbeddedSignature( L".\\AMH.dll.check" ) ) );
	//	}
	//}
	//else
	//{
	//	CONSOLE_Print( "Bad sign" );
	//}

	//if ( FileExists( ".\\AMH.dll.check" ) )
	//{
	//	if ( !DeleteFileA( ".\\AMH.dll.check" ) )
	//	{
	//	
	//	}
	//}


	AMH_Path = ConvertFromUtf16ToUtf8( AMH_PathW );
	AMH_Path_old = AMH_Path_PATH.string( );
	auto War3Path_PATH = fs::path( GetModuleFilePath( GetModuleFromAddress( StormDll ) ) ).parent_path( );


	War3Path_W = War3Path_PATH.wstring( );
	War3Path_old = War3Path_PATH.string( );

	War3Path = ConvertFromUtf16ToUtf8( War3Path_W );

	//
	//SetCurrentDirectoryW( fs::path( War3Path ).wstring( ).c_str( ) );

	wstring LauncherPath = AMH_PathW;


	WarcisConfigPath = ( LauncherPath + L"\\StartupInfo.bin" );

	CONSOLE_Print( L"Init config:" + WarcisConfigPath );


	FILE *f;
	_wfopen_s( &f, WarcisConfigPath.c_str( ), L"rb" );
	if ( !f )
	{
		DeleteFileW( WarcisConfigPath.c_str( ) );
		return;
	}
	fseek( f, 0L, SEEK_END );
	size_t sz = ftell( f );
	fseek( f, 0L, SEEK_SET );

	fread( &gInfo, sizeof( InitializeInfo ), 1, f );
	fseek( f, sizeof( InitializeInfo ), SEEK_SET );
	sz -= sizeof( InitializeInfo );

	fread( (void *)&CustomChatMessagesCount, 4, 1, f );
	sz -= 4;

	CustomChatKeyCodes.reserve( sz / 4 + 4 );
	fseek( f, sizeof( InitializeInfo ) + 4, SEEK_SET );

	fread( &CustomChatKeyCodes[ 0 ], sz, 1, f );


	fclose( f );
	DeleteFileW( WarcisConfigPath.c_str( ) );


	InitAllConfig( );
	CONSOLE_Print( "OK" );



	HMODULE debugmodule = LoadLibrary( "dbghelp.dll" );
	CONSOLE_Print( "[Warcis_Rec] Init crash report...15%" );


	if ( debugmodule )
	{
		crash_rpt::ApplicationInfo appInfo;
		memset( &appInfo, 0, sizeof( appInfo ) );
		appInfo.ApplicationInfoSize = sizeof( appInfo );
		appInfo.ApplicationGUID = "a412d4ca-9dde-469e-8029-35279e3f810e";
		appInfo.Prefix = "";
		appInfo.AppName = L"Warcis Wc3 Client";
		appInfo.Company = L"Warcis Gaming";
		appInfo.V[ 0 ] = ANTIHACK_VERSION;
		appInfo.V[ 1 ] = DRDUMP_VERSION_INC;
		CONSOLE_Print( "[Warcis_Rec] Init crash report...50%" );
		crash_rpt::HandlerSettings handlerSettings;
		memset( &handlerSettings, 0, sizeof( handlerSettings ) );
		handlerSettings.HandlerSettingsSize = sizeof( handlerSettings );
		handlerSettings.SendAdditionalDataWithoutApproval = gInfo.BugReportWithoutUser > 0;
		handlerSettings.OpenProblemInBrowser = TRUE;
		handlerSettings.FullDumpType =
			MiniDumpWithIndirectlyReferencedMemory |
			MiniDumpFilterMemory |
			MiniDumpWithoutOptionalData |
			MiniDumpWithHandleData |
			MiniDumpIgnoreInaccessibleMemory;
		//MiniDumpNormal | MiniDumpWithHandleData | MiniDumpFilterMemory | MiniDumpWithoutOptionalData | MiniDumpIgnoreInaccessibleMemory;
		//handlerSettings.

		handlerSettings.CrashProcessingCallback = PFNCRASHPROCESSINGCALLBACK_MY;
		handlerSettings.OverrideDefaultFullDumpType = TRUE;
		CONSOLE_Print( "[Warcis_Rec] Init crash report...70%" );

		g_crashRpt = new crash_rpt::CrashRpt( &appInfo, &handlerSettings, TRUE );

		g_crashRpt->AddFileToReport( L".\\warcis.log", L"Warcis Log File.txt" );
		g_crashRpt->AddFileToReport( L".\\tempreplay.w3g", L"Warcraft Temp Replay.w3g" );
		g_crashRpt->AddUserInfoToReport( L"CommandLine", GetCommandLineW( ) );

		CONSOLE_Print( "[Warcis_Rec] Init crash report...100" );
	}
	else
	{
		CONSOLE_Print( "[Warcis_Rec] Init crash report...10%" );
		MessageBox( 0, "dbghelp not loaded, crash report disabled.", "Warning", 0 );
	}


	CONSOLE_Print( "CrashReport Initialized." );

	DWORD thread_id;
	GPROXYTHREAD = CreateThread( NULL, 0, InitAll, 0, CREATE_SUSPENDED, &thread_id );
	SetThreadPriority( GPROXYTHREAD, THREAD_PRIORITY_NORMAL );
	AddThreadInfoMy( thread_id, L"GPROXY THREAD" );

	//SetThreadPriority( GetCurrentThread( ), THREAD_PRIORITY_TIME_CRITICAL );
	ResumeThread( GPROXYTHREAD );
	//CONSOLE_Print( "OK" );

	//CONSOLE_Print( "Init HOOK library..." );

	//GPROXYTHREAD = CreateThread( 0, 0, InitAll, 0, 0, 0 );
	


	hAH_Scanner_Thread = CreateThread( 0, 0, AH_Scanner_Thread, 0, 0, &thread_id );
	HideThread( hAH_Scanner_Thread );
	AddThreadInfoMy( thread_id, L"ANTIHACK THREAD" );

	AddNewAhChecks( AH_Scanner_Thread, 300 );

	AddNewAhChecks( ScanResult, 300 );

	//HMODULE k32 = GetModuleHandle( "Kernel32.dll" );
	//if ( k32 )
	//{
	//
	//	pSetUnhandledExceptionFilter_org = ( pSetUnhandledExceptionFilter )( GetSetUnhandledExceptionFilterPointer ( ));
	//	MH_CreateHook( pSetUnhandledExceptionFilter_org, &pSetUnhandledExceptionFilter_my, reinterpret_cast< void** >( &pSetUnhandledExceptionFilter_ptr ) );
	//	MH_EnableHook( pSetUnhandledExceptionFilter_org );
	//	pAddVectoredExceptionHandler_org = ( pAddVectoredExceptionHandler )( GetProcAddress_ptr( k32, "AddVectoredExceptionHandler" ) );
	//	MH_CreateHook( pAddVectoredExceptionHandler_org, &pAddVectoredExceptionHandler_my, reinterpret_cast< void** >( &pAddVectoredExceptionHandler_ptr ) );
	//	MH_EnableHook( pAddVectoredExceptionHandler_org );

	//}


	//CONSOLE_Print( "OK" );

	/*DWORD oldprotect;
	VirtualProtect( ( LPVOID )( ( int )hDLL + 3 ), 1, PAGE_EXECUTE_READWRITE, &oldprotect );
	*( unsigned char* )( ( int )hDLL + 3 ) = 0xC3;
	VirtualProtect( ( LPVOID )( ( int )hDLL + 3 ), 1, PAGE_EXECUTE_READ, &oldprotect );
	FlushInstructionCache( GetCurrentProcess( ), ( LPVOID )( ( int )hDLL + 3 ), 1 );
	*/
	CONSOLE_Print( "Init another features" );
	TimeForInit = GetTickCount( ) - gInfo.StartTime;

	if ( FileExists( AMH_Path_old + "\\AMH.dll" ) )
		InitAh( AMH_Path_old.c_str( ) );
	else
		InitAh( AMH_Path.c_str( ) );
}

void WriteConfigBack( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	CONSOLE_Print( "Write config back" );
	FILE *f;
	_wfopen_s( &f, WarcisConfigPath.c_str( ), L"wb" );
	if ( !f )
	{
		CONSOLE_Print( "OPEN: BAD" );
		DeleteFileW( WarcisConfigPath.c_str( ) );
		return;
	}
	CONSOLE_Print( "OPEN: OK" );
	fwrite( &gInfo, sizeof( InitializeInfo ), 1, f );
	CONSOLE_Print( "WRITE: OK" );
	fclose( f );
}


/*
int __stdcall GameMain( HMODULE hw3lhBase )
{
#ifndef  ANTIHACKNODEBUG
 AddLogFunctionCall( __FUNCSIGW__  );
#endif
	InitAllFunctions( );
	return _GameMain( hw3lhBase );
}*/

FARPROC real_game_main; /* pointer to the real GetGameDllName( ).c_str() GameMain function */

HMODULE game_dll_base;

int AMH_Thread_id = 0;
auto WarcisAMHEvent = CreateEvent( NULL, TRUE, FALSE, "Warcis Protection" );



void __stdcall DllGameCall( int val )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	if ( val == 0 )
	{
		if ( WarcisAMHEvent )
			CloseHandle( WarcisAMHEvent );

		CONSOLE_Print( "[Warcis_Rec] shutting down winsock" );
		TerminateThread( hAH_Scanner_Thread, 2 );
		TerminateThread( GPROXYTHREAD, 2 );


		WSACleanup( );
		//CONSOLE_Print( "1" );
		CustomFeaturesUninitialize( );
		//CONSOLE_Print( "2" );
		MH_Uninitialize( );
		//CONSOLE_Print( "3" );
		WriteConfigBack( );


		typedef int( __cdecl * p_SetMaxFps )( int maxfps );
		p_SetMaxFps _SetMaxFps;
		_SetMaxFps = ( p_SetMaxFps )( GameDll + 0x383640 );
		_SetMaxFps( 2000 );

		//CONSOLE_Print( "5" );
		TerminateProcess( GetCurrentProcess( ), 0 );
	}
	//CONSOLE_Print( "Game Start call:" + to_string( val ) );
}


wchar_t TMPMODULENAME[ MAX_PATH ];


void InitGameProtection( )
{

#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
}

DWORD __stdcall TimeGetTime( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	return timeGetTime( );
}

inline void ErasePEHeaderFromMemory( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	DWORD OldProtect = 0;

	// Get base address of module
	char *pBaseAddr = ( char* )AntihackModule;

	// Change memory protection
	VirtualProtect( pBaseAddr, 256, // Assume x86 page size
		PAGE_READWRITE, &OldProtect );

	// Erase the header
	ZeroMemory( pBaseAddr, 256 );
}


crash_rpt::CrashRpt * g_crashRpt;

void RemoveBadFiles( )
{
	for ( int i = 0; i < 23; i++ )
	{
		if ( FileExists( "FoundCheat" + to_string( i ) + ".delete" ) )
		{
			DeleteFileA( ( "FoundCheat" + to_string( i ) + ".delete" ).c_str( ) );
		}
	}

	if ( FileExists( "VORT_DLS.DLL" ) )
	{
		for ( int i = 0; i < 23; i++ )
		{
			if ( MoveFileA( "VORT_DLS.DLL", ( "FoundCheat" + to_string( i ) + ".delete" ).c_str( ) ) )
			{
				break;
			}
		}
	}

	if ( FileExists( "S3BASE.DLL" ) )
	{
		for ( int i = 0; i < 23; i++ )
		{
			if ( MoveFileA( "S3BASE.DLL", ( "FoundCheat" + to_string( i ) + ".delete" ).c_str( ) ) )
			{
				break;
			}
		}
	}

}

BOOL WINAPI DllMain( HINSTANCE _hDLL, UINT reason, LPVOID reserved )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	//CONSOLE_Print( "[Warcis_Rec] call DllMain:" + to_string( reason ) );

	if ( reason == DLL_PROCESS_ATTACH )
	{
		if ( GetModuleHandle( "Game.dll" ) )
		{
			CONSOLE_Print( "Fatal error!" );

			return FALSE;
		}
		DisableProcessWindowsGhosting( );

		AddThreadInfoMy( GetCurrentThreadId( ), L"MAIN THREAD..." );


		hDLL = _hDLL;
		AntihackModule = hDLL;
		CONSOLE_Print( "[Warcis_Rec] Init crash report...10%" );


		

		SetPriorityClass( GetCurrentProcess( ), HIGH_PRIORITY_CLASS );



		MH_Initialize( );




#ifndef  ANTIHACKNODEBUG
		AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif // ! NODEBUG

		RemoveBadFiles( );

		for ( auto & val : LastCalledFunctions )
		{
			val = L"No func";
		}

	

		//g_crashRpt = crash_rpt::CrashRpt(/*
		//							   (AMH_PathW + L"\\crashrpt.dll").c_str( ),*/
		//	"a412d4ca-9dde-469e-8029-35279e3f810e", // GUID assigned to this application.
		//	"",
		//	L"WC3Crashes",                  // Application name that will be used in message box.
		//	L"Warcis Gaming"                      // Company name that will be used in message box.
		//	, FALSE
		//);


#ifndef  ANTIHACKNODEBUG
		AddLogFunctionCall( __FUNCSIGW__ " 2" );
#endif // ! NODEBUG


		OutputDebugStringA( TEXT( "%s%s%s%s%s%s%s%s%s%s%s" )
			TEXT( "%s%s%s%s%s%s%s%s%s%s%s%s%s" )
			TEXT( "%s%s%s%s%s%s%s%s%s%s%s%s%s" )
			TEXT( "%s%s%s%s%s%s%s%s%s%s%s%s%s" ) );
		OutputDebugStringW( L"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s" );

		DisableThreadLibraryCalls( _hDLL );
		//HideModule( _hDLL );
		if ( !WarcisAMHEvent )
		{
			MessageBox( 0, "Wc3 allready running", "Wc3 уже запущен!", 0 );
			return FALSE;
		}

		HANDLE hOlly = FindWindow( TEXT( "OLLYDBG" ), NULL );

		if ( hOlly )
		{
			MessageBox( 0, "OllyDbg found", "Обнаружен OllyDbg", 0 );
			return FALSE;
		}

		hOlly = FindWindow( TEXT( "Themida" ), NULL );

		if ( hOlly )
		{
			MessageBox( 0, "Debugger found", "Обнаружен Debugger", 0 );
			return FALSE;
		}
		hOlly = FindWindow( TEXT( "Armadillo" ), NULL );

		if ( hOlly )
		{
			MessageBox( 0, "Debugger 2 found", "Обнаружен Debugger 2", 0 );
			return FALSE;
		}



		/*CONSOLE_Print( "Sound1:" + to_string((int)LoadLibraryA( "WINMM.dll" )));
		CONSOLE_Print( "Sound2:" + to_string( ( int )LoadLibraryA( "wdmaud.drv" ) ) );
		CONSOLE_Print( "Sound3:" + to_string( ( int )LoadLibraryA( "DSOUND.dll" ) ) );*/

	
		GetModuleFileNameW( hDLL, TMPMODULENAME, MAX_PATH );
		//LoadLibraryW( L"d3d8.dll" );

		//DisableThreadLibraryCalls( hDLL );

		AMH_Thread_id = GetCurrentThreadId( );


#ifndef  ANTIHACKNODEBUG
		AddLogFunctionCall( __FUNCSIGW__ " 3" );
#endif // ! NODEBUG



		CONSOLE_Print( "Initialization ... :" + to_string( AMH_Thread_id ) );

		//if ( GetModuleHandle( GetGameDllName( ).c_str() ) )
		//{
		//	FoundModifiedLoader = 2;
		////CONSOLE_Print( "ban1" );

		//}

		if ( GetModuleHandle( GetGameDllNameOLD( ).c_str( ) ) )
		{
			FoundModifiedLoader = 2;
			CONSOLE_Print( "ban2" );
		}

		CONSOLE_Print( "#1" );



		//DWORD OldProtect = 0;
		//char *pBaseAddr = ( char* )hDLL;
		//VirtualProtect( pBaseAddr, 4096,
		//	PAGE_EXECUTE_READWRITE, &OldProtect );
		//// Erase the header
		//SecureZeroMemory( pBaseAddr, 4096 );
		//VirtualProtect( pBaseAddr, 4096,
		//	OldProtect, &OldProtect );
		//

		//



		CONSOLE_Print( "Hash 1:" + to_string( ( int )w3l_hash_init ) + " hash2 :" + to_string( ( int )w3l_do_hash ) );

		if ( GameDll == 0 )
			GameDll = LoadGameDll( hDLL, reason, reserved, &LoadLibraryA_org, &LoadLibraryW_org, &BaseThreadInitThunk_org,
				&pLdrLoadDll_org, ( int )w3l_hash_init, ( int )w3l_do_hash );

		StormMdl = GetModuleHandle( "Storm.dll" );
		StormDll = ( int )StormMdl;
		if ( StormDll )
		{
			DisableThreadLibraryCalls( ( HMODULE )StormDll );
		}

#ifndef  ANTIHACKNODEBUG
		AddLogFunctionCall( __FUNCSIGW__ " 3" );
#endif // ! NODEBUG

	
		Storm::Init( StormMdl );
		CONSOLE_Print( "#2" );

		if ( GameDll )
		{
			DisableThreadLibraryCalls( ( HMODULE )GameDll );
		}


		if ( GameDll )
			//CONSOLE_Print( "Ok. Game.Dll found" );


#ifndef  ANTIHACKNODEBUG
			AddLogFunctionCall( __FUNCSIGW__ " 1" );
#endif // ! NODEBUG



		BuildModuleWhiteList( );

		CONSOLE_Print( "#3" );



		game_dll_base = GetModuleFromAddress( GameDll );

		GetProcAddress_org = ( pGetProcAddress )GetProcAddress(
			GetModuleHandle( TEXT( "kernel32.dll" ) ),
			"GetProcAddress" );
		MH_CreateHook( GetProcAddress_org, &GetProcAddress_my, reinterpret_cast< void** >( &GetProcAddress_ptr ) );
		MH_EnableHook( GetProcAddress_org );



		real_game_main = GetProcAddress_ptr( game_dll_base, "GameMain" );

		CONSOLE_Print( "Init another functions" );


#ifndef  ANTIHACKNODEBUG
		AddLogFunctionCall( __FUNCSIGW__ " 4" );
#endif // ! NODEBUG


		InitAllFunctions( );

		CONSOLE_Print( "Init another functions: OK" );

		//ErasePEHeaderFromMemory( );

	}
	else if ( reason == DLL_PROCESS_DETACH )
	{
		if ( WarcisAMHEvent )
		{
			WarcisAMHEvent = NULL;
			CloseHandle( WarcisAMHEvent );
		}
		DllGameCall( 0 );
	}
	else if ( reason == DLL_THREAD_DETACH )
	{
		//CONSOLE_Print( "Thread id:" + to_string( GetCurrentThreadId( ) ) );

	}

	//CONSOLE_Print( "[Warcis_Rec] call DllMain#2:" + to_string( reason ) );

	return TRUE;
}



/* This will be called by war3.exe. Inject the real GetGameDllName( ).c_str() GameMain and
call it.
Must be defined with a naked storage-class attribute to prevent Visual
Studio from generating a prolog that will not be handled on return.
See __attribute__((naked)) for gcc (but not on x86).
*/
__declspec( dllexport ) __declspec( naked ) int GameMain( HMODULE hw3lhBase ) {
	__asm {
		mov eax, game_dll_base
		push eax
		call real_game_main
		push eax
		call DllGameCall
		retn 4
	}
}