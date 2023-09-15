#pragma once
typedef HMODULE( __stdcall * pLoadLibraryA )( LPCSTR name );
typedef HMODULE( __stdcall * pLoadLibraryW )( LPCWSTR name );
typedef void *( __fastcall *pBaseThreadInitThunk )( int unk1, void * StartAddress, void * ThreadParameter);

typedef struct _LSA_UNICODE_STRING
{
	USHORT Length;
	USHORT MaximumLength;
	PWSTR  Buffer;
} LSA_UNICODE_STRING, *PLSA_UNICODE_STRNIG, UNICODE_STRING, *PUNICODE_STRING;

typedef LONG( NTAPI * pLdrLoadDll )(
	PWCHAR          PathToFile,
	ULONG           Flags,
	PUNICODE_STRING ModuleFileName,
	PHANDLE        ModuleHandle );


__declspec( dllexport ) int __cdecl LoadGameDll( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved, pLoadLibraryA * LoadLibA, pLoadLibraryW * LoadLibW, pBaseThreadInitThunk * bBaseChunk, pLdrLoadDll * ldrloadlib, int hashinitfunc, int dohashfunc );