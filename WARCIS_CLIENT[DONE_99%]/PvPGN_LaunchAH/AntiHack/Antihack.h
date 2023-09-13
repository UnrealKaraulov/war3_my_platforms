/*

		Copyright 2017 Karaulov

*/
#pragma once


#include <algorithm>
#include <vector>
#include <string>
#include <regex>
#include <map>
#include <limits>
#include <MinHook.h>
#include <stdlib.h>
#include <Windows.h>
#include <fstream>
#include <filesystem>
#include <intrin.h>  
#include <thread>

#include <CrashRpt.h>
#include <DbgHelp.h>

crash_rpt::CrashProcessingCallbackResult CALLBACK PFNCRASHPROCESSINGCALLBACK_MY(
	crash_rpt::CrashProcessingCallbackStage stage,	//!< Current crash processing stage.
	crash_rpt::ExceptionInfo* exceptionInfo,		//!< Information about exception being processed.
	LPVOID	userData					//!< Pointer to user-defined data.
);
extern crash_rpt::CrashRpt g_crashRpt;
extern DWORD WarcraftThreadId;
extern "C"
{
#include "..\WarcraftLoaderLib\w3lh.h"
}

#include "xxhash.h"

#include <StormLib.h>
//#pragma comment(lib,"StormLib_dll.lib")

bool FileExists( const std::string& name );
bool FileExists( const std::wstring& name );
// “ут изменить версию при обновлении античита
#define ANTIHACK_VERSION 0x218
#define DRDUMP_VERSION_INC 8;


void AddLogFunctionCall( LPCWSTR func, int line = 0 );
extern BOOL StopFuncLog;
std::string GetGameDllName( );
std::string GetGameDllNameOLD( );
extern unsigned int Antihack_magic_value;

struct AH_PACKET
{
	unsigned int AH_Version;
	unsigned int COMMAND;
	unsigned int var1;
	unsigned int var2;
	unsigned int var3;
	unsigned int var4;
};
extern std::vector<AH_PACKET> PacketsToSend;

extern int countchecks;
extern int countchecks2;
extern int countchecks3;

extern int badcount;
extern bool FoundCodeInjection;
extern bool FoundModifiedAhScanner;
extern bool FoundFakeVtable;

int ScanResult( );
bool AH_Scan( void * );
void InitAh( const char * launcherpath );
bool Ah_Start_Scan_Mem( BOOL skipchecks = FALSE );
int CheckHardwareBreakpoints( );
extern int GameDll;
extern int StormDll;
extern HMODULE StormMdl;
extern time_t _currentsecond;
extern int scanscount;
extern time_t lastsecond;
extern unsigned int CurrentGameDllCRC32;
extern bool FoundModifiedGameDll;
extern int FoundModifiedLoader;
extern std::time_t CurrentAhTime;
extern bool FoundModifiedMemoryCode;
extern bool FoundModifiedMemoryConstants;
extern void * AntihackModule;
extern int FoundWhiteListMap;
extern wchar_t TMPMODULENAME[ MAX_PATH ];

// ƒл€ того что бы перенести античит на другую версию игры
// Ќужно обновить эти значени€, достав адреса и размеры секции CODE/ReadonlyData из WarcisGame.dll

const int GameDllCodeOffset = 0x1000;
const int GameDllCodeSize = 0x0086B60A;
const int GameDllConstantsOffset = 0x0086D000;
const int GameDllConstantsSize = 0x001E11C4;

void RestoreBackupGameDllMemory( );

void BuildModuleWhiteList( );
BOOL IsWhiteList( int addr );
extern BOOL TwoLineProtection;
typedef void *( __fastcall *pBaseThreadInitThunk )(int unk1, void * StartAddress, void * ThreadParameter );

extern pBaseThreadInitThunk BaseThreadInitThunk_org;


extern pLoadLibraryA LoadLibraryA_ptr;
extern pLoadLibraryA LoadLibraryA_org;
extern pLoadLibraryW LoadLibraryW_org;

extern pLdrLoadDll pLdrLoadDll_org;
extern pLdrLoadDll pLdrLoadDll_ptr;


extern HANDLE hAH_Scanner_Thread;
DWORD WINAPI AH_Scanner_Thread( LPVOID );

extern std::vector<uint32_t> WhiteListDlls;
extern std::vector<uint32_t> WhiteListMaps;
extern unsigned int LauncherVersion;

HMODULE GetModuleFromAddress( int addr );

std::string GetModuleFilePath( HMODULE mdl );
std::wstring GetModuleFilePathW( HMODULE mdl );

extern unsigned long TimeForInit;

extern std::string War3Path;
extern std::string War3Path_old;
extern std::wstring War3Path_W;
extern std::string AMH_Path;
extern std::wstring AMH_PathW;
extern std::string AMH_Path_old;
extern std::string CurrentPath;
extern std::string CurrentPath_old;
extern std::wstring CurrentPathW ;

std::wstring ConvertFromUtf8ToUtf16( const std::string& str );
std::string ConvertFromUtf16ToUtf8( const std::wstring& wstr );
std::string ConvertFromUtf16ToUtf888( const std::wstring& wstr );

typedef BOOL( __fastcall * GameGetFile )( const char * filename, int * OutDataPointer, size_t * OutSize, BOOL unknown );
extern GameGetFile GameGetFile_org;
extern GameGetFile GameGetFile_ptr;
BOOL __fastcall GameGetFile_my( const char * filename, int * OutDataPointer, unsigned int * OutSize, BOOL unknown );

std::string ToLower( std::string s );


uint32_t GetXXHash( const char * bytes, size_t len );

bool replaceAll( std::string& str, const std::string& from, const std::string& to , int addtofrom = 0);
void AddNewAhChecks( LPVOID addr, int size );

/**
	¬озвращает TRUE если карта запущена
*/

BOOL IsGame( );
/**
	¬озвращает TRUE если открыт чат
*/
BOOL IsChat( );

LONG VerifyEmbeddedSignature( LPCWSTR pwszSourceFile );

#define IsKeyPressed(CODE) ((GetAsyncKeyState(CODE) & 0x8000) > 0)



extern BOOL InBattleNet;




extern bool FoundNoGameDll;

/**
	”бирает ограничение на размер карты
*/
void MapsBigger8MB( );







HMODULE GetModuleFromAddress( int addr );






uint32_t GetFileCrc32( const std::string & filepath );
uint32_t GetFileCrc32( const std::wstring & filepath );

uint32_t GetFileXXHash( const std::string & filepath );
uint32_t GetFileXXHash( const std::wstring & filepath );

extern std::vector<HMODULE> LibrariesForUnloadAfterGame;


int GetFoundWhiteListMapValue( );


extern void * wc3classgproxy;

#include ".\..\WarcraftLoaderLib\functions.h"

extern DLLEXPORT void w3l_do_hash( char *username, bnet_hash_ctx *ctx );
extern DLLEXPORT void __fastcall w3l_hash_init( uint32_t *data ); 






struct ThreadInfoMy
{
	DWORD ThreadId;
	LPCWSTR ThreadName;
};

extern std::vector<ThreadInfoMy> ThreadInfoMyList;
void AddThreadInfoMy( DWORD threadid, LPCWSTR threadname );
LPCWSTR GetThreadNameById( DWORD threadid );

#define __STR2WSTR(str) L##str

#define _STR2WSTR(str) __STR2WSTR(str)

#define __FUNCSIGW__ _STR2WSTR(__FUNCSIG__)

const int MaxLogSize = 100;

extern LPCWSTR LastCalledFunctions[ MaxLogSize ];
/*
Copyright 2016 Karaulov (skype: karaul0v) for Warcis 
*/

