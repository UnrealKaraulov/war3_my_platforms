/*****************************************************************************/
/* Storm.cpp                              Copyright (c) Ladislav Zezula 2003 */
/*---------------------------------------------------------------------------*/
/* This is just a dummy module for building import library for Storm.dll     */
/*---------------------------------------------------------------------------*/
/*   Date    Ver   Who  Comment                                              */
/* --------  ----  ---  -------                                              */
/* 11.04.03  1.00  Lad  The first version of Storm.cpp                       */
/*****************************************************************************/

#include <windows.h>

#define BUILDING_STORM_CPP
#define STORM_ALTERNATE_NAMES
#include "storm_dll.h"

BOOL WINAPI SFILE(OpenArchive)(LPCSTR lpFileName, DWORD dwPriority, DWORD dwFlags, HANDLE *hMPQ)
{
    MessageBoxA(0, lpFileName, "OPEN ARCHIVE", 0);
    return TRUE;
}

BOOL WINAPI SFILE(CloseArchive)(HANDLE hMPQ)
{
    MessageBoxA(0, "", "CLOSE ARCHIVE", 0);
    return TRUE;
}

BOOL WINAPI SFILE(GetArchiveName)(HANDLE hMPQ, LPCSTR lpBuffer, DWORD dwBufferLength)
{
    MessageBoxA(0, "", "GET ARCHIVE NAME", 0);
    return TRUE;
}

BOOL WINAPI SFILE(OpenFile)(LPCSTR lpFileName, HANDLE *hFile)
{
    MessageBoxA(0, lpFileName, "OPEN FILE", 0);
    return TRUE;
}

BOOL WINAPI SFILE(OpenFileEx)(HANDLE hMPQ, LPCSTR lpFileName, DWORD dwSearchScope, HANDLE *hFile)
{
    MessageBoxA(0, lpFileName, "OPEN FILE EX", 0);
    return TRUE;
}

BOOL WINAPI SFILE(CloseFile)(HANDLE hFile)
{
    MessageBoxA(0, "", "CLOSE FILE", 0);
    return TRUE;
}

DWORD WINAPI SFILE(GetFileSize)(HANDLE hFile, LPDWORD lpFileSizeHigh)
{
    MessageBoxA(0, "", "GET FILE SIZE", 0);
    return TRUE;
}

BOOL WINAPI SFILE(GetFileArchive)(HANDLE hFile, HANDLE *hMPQ)
{
    MessageBoxA(0, "", "GET FILE OF ARCHIVE", 0);
    return TRUE;
}

BOOL WINAPI SFILE(GetFileName)(HANDLE hFile, LPCSTR lpBuffer, DWORD dwBufferLength)
{
    MessageBoxA(0, "", "GET FILE NAME", 0);
    return TRUE;
}

DWORD WINAPI SFILE(SetFilePointer)(HANDLE hFile, long lDistanceToMove, PLONG lplDistanceToMoveHigh, DWORD dwMoveMethod)
{
    MessageBoxA(0, "", "SET FILE POINTER", 0);
    return TRUE;
}

BOOL WINAPI SFILE(ReadFile)(HANDLE hFile,LPVOID lpBuffer,DWORD nNumberOfBytesToRead,LPDWORD lpNumberOfBytesRead,LPOVERLAPPED lpOverlapped)
{
    MessageBoxA(0, "", "READ FILE TO BUFFER", 0);
    return TRUE;
}

LCID WINAPI SFILE(SetLocale)(LCID nNewLocale)
{
    MessageBoxA(0, "", "SET LOCATE FILE", 0);
    LCID Locale = 0x0c01; //Arabic - Egypt
    int nchars = GetLocaleInfoA(Locale, LOCALE_SISO639LANGNAME, NULL, 0);
    char* LanguageCode = new char[nchars];
    GetLocaleInfoA(Locale, LOCALE_SISO639LANGNAME, LanguageCode, nchars);
    MessageBoxA(0, LanguageCode, "SET LOCATE FILE", 0);
    return TRUE;
}

BOOL WINAPI SFILE(GetBasePath)(LPCSTR lpBuffer, DWORD dwBufferLength)
{
    MessageBoxA(0, lpBuffer, "GET BASE FILE PATH", 0);
    return TRUE;
}

BOOL WINAPI SFILE(SetBasePath)(LPCSTR lpNewBasePath)
{
    MessageBoxA(0, lpNewBasePath, "SET BASE FILE PATH", 0);
    return TRUE;
}

BOOL WINAPI SFILE(Destroy)()
{
    MessageBoxA(0, "", "MAKE ANAL ERROR", 0);
    return TRUE;
}

BOOL WINAPI SCOMP(Compress)(char * pbOutBuffer, int * pdwOutLength, char * pbInBuffer, int dwInLength, int uCmp, int uCmpType, int nCmpLevel)
{
    MessageBoxA(0, "", "Compress data", 0);
    return TRUE;
}

BOOL WINAPI SCOMP(Decompress)(char * pbOutBuffer, int * pdwOutLength, char * pbInBuffer, int dwInLength)
{
    MessageBoxA(0, "", "Decompress data", 0);
    return TRUE;
}

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD reason, LPVOID)
{
    return TRUE;
}