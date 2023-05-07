#pragma once
#include "Storm.h"
#include <Windows.h>
#pragma pack(push,1)


#pragma region ErrorHandler.cpp
#ifdef DOTA_HELPER_LOG
void  __stdcall AddNewLineToJassLog( std::string s );
void __stdcall  AddNewLineToDotaChatLog( std::string s );
void __stdcall  AddNewLineToDotaHelperLog( std::string s, int line );
void __stdcall  AddNewLineToJassNativesLog( std::string s );
void __stdcall EnableErrorHandler( int );
void __stdcall DisableErrorHandler( int );
void DumpExceptionInfoToFile( _EXCEPTION_POINTERS *ExceptionInfo );

extern void ResetTopLevelExceptionFilter( );
extern LPTOP_LEVEL_EXCEPTION_FILTER OriginFilter;
extern BOOL bDllLogEnable;
typedef LONG( __fastcall * StormErrorHandler )( int a1, void( *a2 )( int, std::string, ... ), int a3, BYTE *a4, LPSYSTEMTIME a5 );
extern StormErrorHandler StormErrorHandler_org;
typedef int( __fastcall *LookupNative )( int global, int unused, std::string name );
extern LookupNative LookupNative_org;
typedef signed int( __fastcall * LookupJassFunc )( int global, int unused, std::string funcname );
extern LookupJassFunc LookupJassFunc_org;
typedef void( __fastcall * ProcessNetEvents )( void * data, int unused_, int Event );
extern ProcessNetEvents ProcessNetEvents_org;
typedef void( __fastcall * BlizzardDebug1 ) ( const char*str );
extern BlizzardDebug1 BlizzardDebug1_org;
typedef void( __cdecl * BlizzardDebug2 )( std::string src, int lineid, std::string classname );
extern BlizzardDebug2 BlizzardDebug2_org;
typedef void( __cdecl * BlizzardDebug3 )( std::stringformat, ... );
extern BlizzardDebug3 BlizzardDebug3_org;
typedef void( __cdecl * BlizzardDebug4 )( BOOL type1, std::stringformat, ... );
extern BlizzardDebug4 BlizzardDebug4_org;
typedef void( __cdecl * BlizzardDebug5 )( std::stringformat, ... );
extern BlizzardDebug5 BlizzardDebug5_org;
typedef void( __cdecl * BlizzardDebug6 )( std::stringformat, ... );
extern BlizzardDebug6 BlizzardDebug6_org;
#endif
#pragma endregion

//extern int memoryleakcheck;



//extern int memoryleakcheck;

class StormBuffer
{
private:

public:
	char *buf;
	unsigned long length;
	/*~StormBuffer( )
	{
	Clear( );
	}*/
	StormBuffer( )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
		buf = 0;
		length = 0;
	}
	StormBuffer( unsigned long l )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
		//	memoryleakcheck++;
		length = l;
#ifdef STORMMEM
		buf = ( char * )Storm::MemAlloc( l + 1 );
#else 
		buf = ( char * )new char[ l + 1 ];
#endif
		buf[ l ] = '\0';
	}
	StormBuffer( char* b, unsigned long l )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
		buf = b;
		length = l;
	}
	void Resize( unsigned long l )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
		Clear( );
#ifdef STORMMEM
		buf = ( char * )Storm::MemAlloc( l + 1 );
#else 
		buf = ( char * )new char[ l + 1 ];
#endif
		buf[ l ] = '\0';
		length = l;
	}

	char * GetData( )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
		return buf;
	}
	char * GetData( int offset )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
		return buf + offset;
	}

	unsigned long GetSize( )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
		return length;
	}

	void Clear( )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
		//	memoryleakcheck--;
		length = 0;
		
#ifdef STORMMEM
		Storm::MemFree( buf );
#else 
		delete[ ] buf;
#endif
		buf = NULL;
	}

	StormBuffer&  Clone( StormBuffer& CopyObject )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
		Resize( CopyObject.length );
		std::memcpy( buf, CopyObject.GetData( ), length );
		return ( *this );
	}

	StormBuffer& operator =( StormBuffer& CopyObject )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
		/*Resize( CopyObject.length );
		std::memcpy( buf, CopyObject.GetData( ), length );*/
		length = CopyObject.length;
		buf = CopyObject.buf;
		return ( *this );
	}
	StormBuffer& operator =( std::string& CopyString )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
		Resize( static_cast< INT >( CopyString.size( ) ) );
		std::memcpy( buf, CopyString.c_str( ), length );
		return ( *this );
	}

	CHAR& operator []( INT Index )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
		return buf[ Index ];
	}


};

typedef struct StormBufferList
{
	char **buf;
	unsigned long length;
	StormBufferList( )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
		buf = 0;
		length = 0;
	}
	StormBufferList( unsigned long l )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
#ifdef STORMMEM
		buf = ( char ** )Storm::MemAlloc( l + 1 );
#else 
		buf = ( char ** )new char[ l + 1 ];
#endif
		length = l;
	}
	StormBufferList( char** b, unsigned long l )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
		buf = b;
		length = l;
	}
} StormBufferList;


#pragma pack(pop)