#include "Общий заголовок.h"


#include <TlHelp32.h>


LPVOID TlsValue;
DWORD TlsIndex;
DWORD _W3XTlsIndex;

DWORD GetIndex( )
{
	return *( DWORD* )( _W3XTlsIndex );
}

DWORD GetW3TlsForIndex( DWORD index )
{
	DWORD pid = GetCurrentProcessId( );
	THREADENTRY32 te32;
	HANDLE hSnap = CreateToolhelp32Snapshot( TH32CS_SNAPTHREAD, pid );
	te32.dwSize = sizeof( THREADENTRY32 );

	if ( Thread32First( hSnap, &te32 ) )
	{
		do
		{
			if ( te32.th32OwnerProcessID == pid )
			{
				HANDLE hThread = OpenThread( THREAD_ALL_ACCESS, false, te32.th32ThreadID );
				if ( hThread )
				{
					CONTEXT ctx = { CONTEXT_SEGMENTS };
					LDT_ENTRY ldt;
					GetThreadContext( hThread, &ctx );
					GetThreadSelectorEntry( hThread, ctx.SegFs, &ldt );
					DWORD dwThreadBase = ldt.BaseLow | ( ldt.HighWord.Bytes.BaseMid <<
						16u ) | ( ldt.HighWord.Bytes.BaseHi << 24u );
					CloseHandle( hThread );
					if ( dwThreadBase == NULL )
						continue;
					DWORD* dwTLS = *( DWORD** )( dwThreadBase + 0xE10 + 4 * index );
					if ( dwTLS == NULL )
						continue;
					return ( DWORD )dwTLS;
				}
			}
		} while ( Thread32Next( hSnap, &te32 ) );
	}

	return NULL;
}

void SetTlsForMe( )
{
	TlsIndex = GetIndex( );
	LPVOID tls = ( LPVOID )GetW3TlsForIndex( TlsIndex );
	TlsValue = tls;
	LPVOID oldtlsvalue = TlsGetValue( TlsIndex );
	if ( oldtlsvalue != TlsValue )
		TlsSetValue( TlsIndex, TlsValue );
}


void GiveMeTlsAccess( )
{
	_W3XTlsIndex = 0xAB7BF4 + (DWORD)GameDll;
	
	SetTlsForMe( );
}