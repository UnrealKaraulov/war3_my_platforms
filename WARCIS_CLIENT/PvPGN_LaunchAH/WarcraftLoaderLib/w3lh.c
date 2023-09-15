/*
	w3l: a PvPGN loader for Warcraft 3 1.27a+
	Copyright (C) 2008-2017 Rupan, Keres, Phatdeeva, Karaulov

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/


#ifdef _MSC_VER
#undef UNICODE
#if _MSC_VER > 1400
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif



#include <Windows.h>
#include <stdio.h>
#include "patches.h"
#include "dep.h"
#include "functions.h"
#include "w3lh.h"
#define DEBUG_FILE "debug.log"

void debug( char *message, ... ) {
	return;
	DWORD temp;
	HANDLE file;
	va_list args;
	char buf[ 1024 ];

	memset( buf, 0, sizeof( buf ) );
	va_start( args, message );
	wvsprintfA( buf, message, args );

	file = CreateFile( DEBUG_FILE, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	SetFilePointer( file, 0, 0, FILE_END );
	WriteFile( file, buf, ( DWORD )strlen( buf ), &temp, NULL );
	CloseHandle( file );
}


#ifndef USE_SRP3
//__declspec( dllexport ) void w3l_do_hash( char *username, bnet_hash_ctx *ctx );
//__declspec( dllexport ) int __stdcall w3l_logon_proof_hash_rev( char *arg1, void *arg2, void *arg3 );
//__declspec( dllexport ) int __fastcall w3l_lph_checked_rev( int *a1, int *a2, void *a3, void *a4 );
__declspec( dllexport ) int w3l_create_account3( );
#endif
//__declspec( dllexport ) void __fastcall w3l_hash_init( unsigned int *data );

char gamedllname[ ] = 
{ 
'W',
'a',
'r',
'c',
'i',
's',
'G',
'a',
'm',
'e',
'.',
'd',
'l',
'l',
'\0' 
};

//#define DEBUG /* undef to disable logging */
#define LATENCY_FILE "latency.txt"
#define GAME_DLL_NAMEold "WarcisGame.dll"
#define GAME_DLL_NAME gamedllname
#define GAME_MAIN "GameMain"
#define GAME_DLL_SIZE 0xb80000 /* size of real "Game.dll" in memory */

#define SUCCESS 0
#define ERR_GAME_DLL_LOAD 1
#define ERR_GAME_MAIN 2
#define ERR_SIG_NOT_FOUND 3
#define ERR_MEM_WRITE 3

#define DEFAULT_LATENCY 0x64

#define GAME_DLL_LOAD_ERROR "Could not open " ## GAME_DLL_NAMEold
#define GAME_MAIN_ERROR "Could not locate procedure " ## GAME_MAIN
#define SIG_NOT_FOUND_ERROR "Could not find location to patch."
#define MEMORY_WRITE_ERROR "Could not write to process memory."

#define LPH_HASH_INIT_CALL_OFFSET 0x9 /* offset of hash_init call in the logon proof password hashing routine */
#define LPH_DO_HASH_CALL_OFFSET 0x42  /* offset of do_hash call */

int called; /* used to stop multiple calls to DLL entry point */
FARPROC real_game_main; /* pointer to the real "Game.dll" GameMain function */
HMODULE game_dll_base; /* pointer to base of the real "Game.dll" */
HANDLE app_process; /* handle to the war3.exe process */

char read_latency( ) {
	HFILE file;
	OFSTRUCT ofFile;
	char buf[ 4 ];
	int lt, readNum;
	file = OpenFile( LATENCY_FILE, &ofFile, OF_READ );
	if ( file == HFILE_ERROR )
		return DEFAULT_LATENCY;
	ReadFile( file, buf, 3, &readNum, NULL );
	CloseHandle( file );
	buf[ 3 ] = '\0';
	lt = atoi( buf );
	if ( lt > 255 || lt < 30 )
		return DEFAULT_LATENCY;
	return ( char )lt;
}

/* write patch to memory */
int apply_patch( char *ptr, t_patch *patch ) {
	DWORD temp, oldprot, newprot = PAGE_EXECUTE_READWRITE;
	debug( "Patching (%s) 0x%08x: ", patch->name, ptr );

	VirtualProtectEx( app_process, ptr, patch->length, newprot, &oldprot );
	if ( WriteProcessMemory( app_process, ptr, patch->data, patch->length, &temp ) ) {
		debug( "%d of %d bytes written\r\n", temp, patch->length );
	}
	else {
		debug( "failed.\r\n" );
		return ERR_MEM_WRITE;
	}
	VirtualProtectEx( app_process, ptr, patch->length, oldprot, &newprot );
	return SUCCESS;
}

/* search buffer of size scan_len for sig */
int find_sig( char *base, t_sig *sig, int scan_len ) {
	int i, j, found = 0;

	for ( i = 0; i < scan_len; i++ ) {
		found = 1;
		for ( j = 0; j < sig->length; j++ ) {
			if ( ( ( base[ i + j ] ) & ( sig->data[ j * 2 ] ) ) != ( sig->data[ j * 2 + 1 ] ) ) {
				found = 0;
				break;
			}
		}
		if ( found )
			return i + sig->delta;
	}

	return -1;
}

/* apply a NULL terminated array of sig/patch pairs */
int apply_patches( char *base, void *patches[ ], int scan_len ) {
	int i = 0, patch_loc;
	int ret = SUCCESS; // return value
	t_sig *cur_sig;
	t_patch *cur_patch;
	while ( patches[ i ] != NULL ) {
		cur_sig = ( t_sig * )( patches[ i ] );
		cur_patch = ( t_patch * )( patches[ i + 1 ] );
		debug( "Searching for sig (%s), length 0x%x: ", cur_sig->name, cur_sig->length );
		patch_loc = find_sig( base, cur_sig, scan_len );
		if ( patch_loc == -1 ) {
			debug( "not found.\r\n" );
			ret = ERR_SIG_NOT_FOUND;
			i += 2;
			continue;
		}
		debug( "found at 0x%08x\r\n", base + patch_loc );
		if ( apply_patch( base + patch_loc, cur_patch ) == ERR_MEM_WRITE )
			ret = ERR_MEM_WRITE;
		i += 2;
	}

	return ret;
}

/* write len bytes of data to patch starting at offset */
void ammend_patch( char *patch, void *data, int offset, int len ) {
	char *patch_ptr = patch + offset;
	char *data_ptr = ( char * )data;

	while ( len > 0 ) {
		*patch_ptr = *data_ptr;
		patch_ptr++;
		data_ptr++;
		len--;
	}
}

int w3l_hash_init = 0;
int w3l_do_hash = 0;

#if ! defined USE_SRP3
/* Applies all patches to "Game.dll"
See patches.h for more information */
int patch_game( char *base ) {
	int hash_init_proc, do_hash_proc, logon_proof_hash_proc;
	int hash_init_call_loc, do_hash_call_loc, rel_call_addr;
	int create_account3_loc;

	debug( "Searching for hash_init: " );
	//hash_init_proc = find_sig(base, &hash_init_sig, GAME_DLL_SIZE);
	hash_init_proc = ( int )w3l_hash_init - ( int )base;
	if ( hash_init_proc == -1 ) {
		debug( "not found.\r\n" );
		return ERR_SIG_NOT_FOUND;
	}
	debug( "found at 0x%08x\r\n", base + hash_init_proc );

	debug( "Searching for do_hash: " );

	//do_hash_proc = DO_HASH_OFFSET; //use this line to write new do_hash into empty space
	//do_hash_proc = find_sig(base, &do_hash_sig, GAME_DLL_SIZE); //use this line to write new do_hash over old
	/*
	In version 1.1, do_hash function was moved to the library. In Game.dll there is indirect call to that function.
	That's why we count difference between pointers. May cause unknown errors, if star_do_hash will be lesser than
	base.
	*/
	do_hash_proc = ( int )w3l_do_hash - ( int )base;

	if ( do_hash_proc == -1 ) {
		debug( "not found.\r\n" );
		return ERR_SIG_NOT_FOUND;
	}
	debug( "found at 0x%08x\r\n", base + do_hash_proc );

	//if(apply_patch(base + do_hash_proc, &do_hash_patch) == ERR_MEM_WRITE)
	//	return ERR_MEM_WRITE;

	debug( "Searching for logon_proof_hash: " );
	logon_proof_hash_proc = find_sig( base, &logon_proof_hash_sig, GAME_DLL_SIZE );
	if ( logon_proof_hash_proc == -1 ) {
		debug( "not found.\r\n" );
		return ERR_SIG_NOT_FOUND;
	}
	debug( "found at 0x%08x\r\n", base + logon_proof_hash_proc );

	/* insert CALL offsets of hash_init and do_hash calls into the logon proof password hash routine */
	hash_init_call_loc = logon_proof_hash_proc + LPH_HASH_INIT_CALL_OFFSET;
	do_hash_call_loc = logon_proof_hash_proc + LPH_DO_HASH_CALL_OFFSET;

	/* 5 bytes = length of CALL addr instruction */
	rel_call_addr = hash_init_proc - hash_init_call_loc - 5;
	/* +1 to skip CALL instruction (only modifying the CALL address)
	rel_call_addr = 4 bytes */
	debug( "Setting LPH hash_init call addr at 0x%08x to 0x%08x\r\n", hash_init_call_loc + base, rel_call_addr );
	ammend_patch( logon_proof_hash_patch_data, &rel_call_addr, LPH_HASH_INIT_CALL_OFFSET + 1, 4 );

	/* do the same for do_hash */
	rel_call_addr = do_hash_proc - do_hash_call_loc - 5;
	debug( "Setting LPH do_hash call addr at 0x%08x to 0x%08x\r\n", do_hash_call_loc + base, rel_call_addr );
	ammend_patch( logon_proof_hash_patch_data, &rel_call_addr, LPH_DO_HASH_CALL_OFFSET + 1, 4 );

	if ( apply_patch( base + logon_proof_hash_proc, &logon_proof_hash_patch ) == ERR_MEM_WRITE )
		return ERR_MEM_WRITE;

	/* The main createaccount patch is too big to be placed at the main patch location.
	Instead, helper code redirects to the location. The patch is written right after
	where the logonproofhash patch is written
	TODO: ammend create_account2_patch_data to call create_account3_loc instead
	of hardcoded location OR redo create_account patches so relocation is not needed */
	create_account3_loc = logon_proof_hash_proc + logon_proof_hash_patch.length;
	if ( apply_patch( base + create_account3_loc, &create_account3_patch ) == ERR_MEM_WRITE )
		return ERR_MEM_WRITE;


	/* apply the other patches */
	return apply_patches( base, game_patches, GAME_DLL_SIZE );
}
#endif



pLoadLibraryA LoadLibraryA_org;
pLoadLibraryW LoadLibraryW_org;
pBaseThreadInitThunk BaseThreadInitThunk_org;
pLdrLoadDll pLdrLoadDll_org;

#pragma comment(lib,"WarcisGame.lib")

extern int GameMain( int );


/* Loads the real "WarcisGame.dll" and gets the address of the real "GameMain" function
   Applies patches */
int patch( ) {
	int rval;
	app_process = GetCurrentProcess( );
	auto kernel32 = LoadLibrary( "Kernel32.dll" );
	auto ntdll = LoadLibrary( "NTDLL.dll" );
	debug( "#1%X\r\n", game_dll_base );

	LoadLibraryA_org = ( pLoadLibraryA )GetProcAddress( kernel32, "LoadLibraryA" );
	LoadLibraryW_org = ( pLoadLibraryW )GetProcAddress( kernel32, "LoadLibraryW" );
	BaseThreadInitThunk_org = ( pBaseThreadInitThunk )GetProcAddress( kernel32, "BaseThreadInitThunk" );
	pLdrLoadDll_org = ( pLdrLoadDll )GetProcAddress( ntdll, "LdrLoadDll" );
	game_dll_base = GetModuleHandle( GAME_DLL_NAME );
	//MessageBox( 0, GAME_DLL_NAME, ":", 0 );
	/*char buf[ 256 ];
	GetCurrentDirectoryA( 256, buf );
	MessageBoxA( 0, buf, buf, 0 );
*/
	/*game_dll_base =*/
	debug( "#2-%s-%X\r\n", GAME_DLL_NAME, game_dll_base );
	debug( "#3-%s-%X\r\n","LoadLibraryA" , LoadLibraryA_org );

	if ( !game_dll_base ) {
		MessageBox( NULL, GAME_DLL_LOAD_ERROR, "[lib] Patch Error (w3lh.dll)", MB_OK );
		return ERR_GAME_DLL_LOAD;
	}

	debug( "base: 0x%08X\r\n", game_dll_base );
	debug( "base2: 0x%08X\r\n", GetModuleHandle( GAME_DLL_NAME ) );

	real_game_main = GetProcAddress( game_dll_base, GAME_MAIN );
	if ( !real_game_main ) {
		MessageBox( NULL, GAME_MAIN_ERROR, "[proc] Patch Error (w3lh.dll)", MB_OK );
		return ERR_GAME_MAIN;
	}
	/* apply delay reduction patches and adRemove patch */
	delay_reducer_patch_data[ 0 ] = 10;
	debug( "Latency set to: %hu ms\r\n", ( unsigned char )( delay_reducer_patch_data[ 0 ] ) );
	apply_patches( ( char * )game_dll_base, unimportant_patches, GAME_DLL_SIZE );

#if ! defined USE_SRP3
	rval = patch_game( ( char * )game_dll_base );
#else
	rval = apply_patches( ( char * )game_dll_base, game_patches_srp3, GAME_DLL_SIZE );
#endif
	if ( rval == ERR_SIG_NOT_FOUND )
		MessageBox( NULL, SIG_NOT_FOUND_ERROR, "Patch Error (w3lh.dll)", MB_OK );
	else if ( rval == ERR_MEM_WRITE )
		MessageBox( NULL, MEMORY_WRITE_ERROR, "Patch Error (w3lh.dll)", MB_OK );

	return rval;
}


int AllLoaded = 0;





/* DLL entry point - called when war3.exe calls LoadLibrary.
   Patches WarcisGame.dll memory on attach before GameMain is called.
   Also called when war3.exe calls FreeLibrary. Intercepts the war3.exe FreeLibrary
   call and frees the real "WarcisGame.dll" */
int __cdecl LoadGameDll( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved, pLoadLibraryA * LoadLibA, pLoadLibraryW * LoadLibW, pBaseThreadInitThunk * bBaseChunk, pLdrLoadDll * ldrloadlib, int hashinitfunc, int dohashfunc ) {

	w3l_hash_init = hashinitfunc;
	w3l_do_hash = dohashfunc;

	if ( fdwReason == DLL_PROCESS_DETACH && called == 1 && game_dll_base ) { /* FreeLibrary called */
		called = 2; /* prevent multiple FreeLibrary calls */
		FreeLibrary( game_dll_base );
		GameMain( 0 );
		return ( int )game_dll_base;
	}

	if ( called ) /* prevent multiple calls to this function */
		return  ( int )game_dll_base;
	called = 1;

	debug( "Start.\r\n" );
	if ( patch( ) == SUCCESS ) {
		debug( "Patches successful.\r\n" );
		if ( SetCurrentProcessDEP( DEP_DISABLED ) )
			debug( "DEP disabled\r\n" );
		else
			debug( "DEP can't be disabled\r\n" );
	}
	else { /* if everything did not go to plan, free the loaded DLL and terminate war3.exe */
		debug( "Some patches failed.\r\n" );
		FreeLibrary( game_dll_base );
		TerminateProcess( app_process, 1 );
	}

	*LoadLibA = LoadLibraryA_org;
	*LoadLibW = LoadLibraryW_org;
	*bBaseChunk = BaseThreadInitThunk_org;
	*ldrloadlib = pLdrLoadDll_org;
	return ( int )game_dll_base;
}

//BOOL WINAPI DllMain( HINSTANCE _hDLL, UINT reason, LPVOID reserved )
//{
//	if ( reason == DLL_PROCESS_DETACH )
//		LoadGameDll( _hDLL, reason, reserved, 0, 0, 0 );
//
//	return TRUE;
//}
//
//



/* This will be called by war3.exe. Inject the real "Game.dll" GameMain and
call it.
Must be defined with a naked storage-class attribute to prevent Visual
Studio from generating a prolog that will not be handled on return.
See __attribute__((naked)) for gcc (but not on x86).
*/
//DLLEXPORT __declspec( naked ) int GameMain( HMODULE hw3lhBase ) {
//	__asm {
//		mov eax, game_dll_base
//		push eax
//		call real_game_main
//		retn 4
//	}
//}
