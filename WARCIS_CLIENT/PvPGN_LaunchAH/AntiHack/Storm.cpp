
#include "Storm.h"
#include "FunctionTemplate.h"
#include <new>
#include <cstdlib>
#include "warcis_reconnector.h"
#include "CustomFeatures.h"
#include "fp_call.h"

static bool CustomMpqLoaded = false;

namespace Storm {
	bool StormAvailable = false;
	HMODULE StormModule = NULL;
	PROTOTYPE_FileArchiveOpen FileOpenArchive = NULL;
	PROTOTYPE_FileArchiveOpen FileOpenArchive_ptr = NULL;
	PROTOTYPE_Archive_OpenFile Archive_OpenFile = NULL;
	BOOL __stdcall FileOpenArchive_my( const char* mpqName, DWORD priority, DWORD flags, HANDLE *pMpqHandle )
	{
		char outbuftmp[ 256 ];
		sprintf_s( outbuftmp, "Open mpq \"%s\" with priority %X and flags %u", mpqName, priority, flags );
		CONSOLE_Print( outbuftmp );
		*pMpqHandle = NULL;
		BOOL retval = FileOpenArchive_ptr( mpqName, priority, flags, pMpqHandle );
		
		if ( !( *pMpqHandle ) || !retval )
		{
			//CONSOLE_Print( "Bad path... New path:" + ( War3Path_old + "\\" + string( mpqName ) ) );
			retval = FileOpenArchive_ptr( ( War3Path_old + "\\" + string( mpqName ) ).c_str( ), priority, flags, pMpqHandle );
		}
		if ( !( *pMpqHandle ) || !retval )
		{
			//CONSOLE_Print( "Bad path #2... New path:" + ( War3Path + "\\" + string( mpqName ) ) );
			retval = FileOpenArchive_ptr( ( War3Path + "\\" + string( mpqName ) ).c_str( ), priority, flags, pMpqHandle );
		}

		if (!CustomMpqLoaded && flags == 6)
		{
			CustomMpqLoaded = true;
			char WarcisMpqPath[MAX_PATH];
			sprintf_s(WarcisMpqPath, "%s\\%s", LauncherPath.c_str(), "warcis.mpq");

			char CustomMpqPath[MAX_PATH];
			sprintf_s(CustomMpqPath, "%s\\%s", LauncherPath.c_str(), "custom.mpq");


			BOOL AllOkay = TRUE;
			BOOL custommpqloaded = TRUE;


			HANDLE tmpval;


			AllOkay = FALSE;
			custommpqloaded = Storm::FileOpenArchive( WarcisMpqPath, 0x10000000, 6, &tmpval );
			if ( !custommpqloaded || !tmpval )
			{
				CONSOLE_Print( "Error loading warcis.mpq at path: " + ( string )WarcisMpqPath );
				custommpqloaded = Storm::FileOpenArchive( ( AMH_Path + "\\warcis.mpq" ).c_str( ), 0x10000000, 6, &tmpval );
				if ( !custommpqloaded || !tmpval )
				{
					CONSOLE_Print( "Error loading warcis.mpq #2 at path: " + ( string )WarcisMpqPath );
					custommpqloaded = Storm::FileOpenArchive( ( AMH_Path_old + "\\warcis.mpq" ).c_str( ), 0x10000000, 6, &tmpval );
					if ( !custommpqloaded || !tmpval )
					{
						CONSOLE_Print( "Error loading warcis.mpq #3 at path: " + ( string )WarcisMpqPath );
					}
					else AllOkay = TRUE;
				}
				else AllOkay = TRUE;
			}
			else AllOkay = TRUE;



			AllOkay = FALSE;
			custommpqloaded = FALSE;

			if ( gInfo.UseCustomMpq )
			{
				BOOL custommpqloaded = Storm::FileOpenArchive( CustomMpqPath, 0xF000000, 6, &tmpval );
				if ( !custommpqloaded || !tmpval )
				{
					CONSOLE_Print( "Error loading custom.mpq at path: " + ( string )CustomMpqPath );
					custommpqloaded = Storm::FileOpenArchive( ( AMH_Path + "\\custom.mpq" ).c_str( ), 0xF000000, 6, &tmpval );
					if ( !custommpqloaded || !tmpval )
					{
						CONSOLE_Print( "Error loading custom.mpq #2 at path: " + ( string )CustomMpqPath );
						custommpqloaded = Storm::FileOpenArchive( ( AMH_Path_old + "\\custom.mpq" ).c_str( ), 0xF000000, 6, &tmpval );
						if ( !custommpqloaded || !tmpval )
						{
							CONSOLE_Print( "Error loading custom.mpq #3 at path: " + ( string )CustomMpqPath );


						}
						else AllOkay = TRUE;
					}
					else AllOkay = TRUE;
				}
				else AllOkay = TRUE;
			}
			else
			{
				//AllOkay = TRUE;
				//custommpqloaded = TRUE;
			}





			if ( !AllOkay )
			{
				if ( FileExists( ( CurrentPathW + L"\\custom.mpq" ) ) )
				{
					DeleteFileW( ( CurrentPathW + L"\\custom.mpq" ).c_str( ) );
				}

				if ( FileExists( ( CurrentPathW + L"\\warcis.mpq" ) ) )
				{
					DeleteFileW( ( CurrentPathW + L"\\warcis.mpq" ).c_str( ) );
				}

				//DeleteFileW( ( CurrentPathW + L"\\warcis.mpq" ).c_str( ) );
				CopyFileW( ( AMH_PathW + L"\\warcis.mpq" ).c_str( ), ( CurrentPathW + L"\\warcis.mpq" ).c_str( ), FALSE );
				custommpqloaded = Storm::FileOpenArchive( "warcis.mpq", 0x10000000, 6, &tmpval );

				if ( gInfo.UseCustomMpq )
				{
					//DeleteFileW( ( CurrentPathW + L"\\custom.mpq" ).c_str( ) );
					CopyFileW( ( AMH_PathW + L"\\custom.mpq" ).c_str( ), ( CurrentPathW + L"\\custom.mpq" ).c_str( ), FALSE );
					custommpqloaded = Storm::FileOpenArchive( "custom.mpq", 0x10000000, 6, &tmpval );
				}

			}
		}

		return retval;
	}

	PROTOTYPE_FileArchiveClose FileCloseArchive = NULL;
	PROTOTYPE_StringGetHash StringGetHash = NULL;
	void* AddrMemAlloc;
	void* AddrMemFree;
	void* AddrMemGetSize;
	void* AddrMemReAlloc;
	void* AddrFileOpenFileEx;
	void* AddrFileGetFileSize;
	void* AddrFileReadFile;
	void* AddrFileCloseFile;
	void* AddrFileGetLocale;

	void Init( HMODULE module ) {
		StormModule = module;
		FileOpenArchive = ( PROTOTYPE_FileArchiveOpen )GetProcAddress( module, ( LPCSTR )266 );
		Archive_OpenFile = ( PROTOTYPE_Archive_OpenFile )GetProcAddress( module, ( LPCSTR )279 );
		FileCloseArchive = ( PROTOTYPE_FileArchiveClose )GetProcAddress( module, ( LPCSTR )252 );
		StringGetHash = ( PROTOTYPE_StringGetHash )GetProcAddress( module, ( LPCSTR )590 );
		AddrMemAlloc = ( void* )GetProcAddress( module, ( LPCSTR )401 );
		AddrMemFree = ( void* )GetProcAddress( module, ( LPCSTR )403 );
		AddrMemGetSize = ( void* )GetProcAddress( module, ( LPCSTR )404 );
		AddrMemReAlloc = ( void* )GetProcAddress( module, ( LPCSTR )405 );
		AddrFileOpenFileEx = ( void* )GetProcAddress( module, ( LPCSTR )268 );
		AddrFileGetFileSize = ( void* )GetProcAddress( module, ( LPCSTR )265 );
		AddrFileReadFile = ( void* )GetProcAddress( module, ( LPCSTR )269 );
		AddrFileCloseFile = ( void* )GetProcAddress( module, ( LPCSTR )253 );
		AddrFileGetLocale = ( void* )GetProcAddress( module, ( LPCSTR )294 );
		StormAvailable = true;
	}

	void Cleanup( ) {

	}

	std::map<void*, void*> MemUserDataMap;

	//294	SFileGetLocale() 
	LANGID FileGetLocale( ) {
		return aero::generic_c_call<uint16_t>( AddrFileGetLocale );
	}

	void * retvaladdr = 0;
	//TODO Debug版本加上调试信息
	void*  MemAlloc( uint32_t size ) {
		return aero::generic_std_call<void*>(
			AddrMemAlloc,
			size,
			"",
			0,
			0
			);
	}

	void* MemFree( void* addr ) {
		MemUserDataMap.erase( addr );
		return aero::generic_std_call<void*>(
			AddrMemFree,
			addr,
			"",
			0,
			0
			);
	}

	int MemGetSize( void* addr ) {
		return aero::generic_std_call<uint32_t>(
			AddrMemGetSize,
			addr,
			"",
			0
			);
	}

	void* MemReAlloc( void* addr, uint32_t size ) {
		return aero::generic_std_call<void*>(
			AddrMemReAlloc,
			addr,
			size,
			"",
			0,
			0
			);
	}

	HANDLE FileOpenFileEx( HANDLE hMpq, const char *szFileName, uint32_t dwSearchScope, HANDLE *phFile ) {
		return aero::generic_std_call<HANDLE>(
			AddrFileOpenFileEx,
			hMpq,
			szFileName,
			dwSearchScope,
			phFile
			);
	}

	uint32_t FileGetFileSize( HANDLE hFile, uint32_t *pdwFileSizeHigh ) {
		return aero::generic_std_call<uint32_t>(
			AddrFileGetFileSize,
			hFile,
			pdwFileSizeHigh
			);
	}

	bool FileReadFile( HANDLE hFile, void* lpBuffer, uint32_t dwToRead, uint32_t* pdwRead, LPOVERLAPPED lpOverlapped ) {
		return aero::generic_std_call<BOOL>(
			AddrFileReadFile,
			hFile,
			lpBuffer,
			dwToRead,
			pdwRead,
			lpOverlapped
			);
	}

	bool FileCloseFile( HANDLE hFile ) {
		return aero::generic_std_call<BOOL>(
			AddrFileCloseFile,
			hFile
			);
	}

}

/*
#ifndef _DEBUG
struct MemInfo {
	const char* FILE;
	int LINE;
};

std::map<void*, MemInfo> MemInfoMap;

void *operator new(size_t size) {
	void* rv;

	if (!Storm::StormAvailable) {
		rv = malloc(size);
		//OutputDebug("NonStormAlloc: 0x%X (%u)", rv, size);

		return rv;
	}

	if (size == 0)
		size = 1;

	while (1) {
		if ((rv = Storm::MemAlloc(size)) != NULL) {
			return rv;
		}

		// 分配不成功，找出当前出错处理函数
		new_handler globalhandler = std::set_new_handler(0);
		std::set_new_handler(globalhandler);

		if (globalhandler)
			(*globalhandler)();
		else throw std::bad_alloc();
	}
}

void operator delete(void *rawmemory) {
	if (rawmemory == 0)
		return;

	if (!Storm::StormAvailable) {
		free(rawmemory);
		//OutputDebug("NonStormFree: 0x%X", rawmemory);
		return;
	}

	Storm::MemFree(rawmemory);
	return;
}
#endif
*/