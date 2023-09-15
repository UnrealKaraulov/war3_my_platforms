#pragma once
#include <stdio.h>     
#include <stdlib.h>     

#include <Windows.h>
#include <string>
#include <iostream>
#include <fstream>
#include "resource.h"
#include "MemoryModule.h"

#include <MinHook.h>
#include <fp_call.h>
#include <time.h>

#include "WarcraftFrameHelper.h"
using namespace NWar3Frame;

uint32_t crc32_16bytes( const void* data, size_t length, uint32_t previousCrc32 );

extern double GameDll;

void ЗапускАвтоматическогоВхода( );

void Доступ_К_WarcraftEngine( );

extern HWND Warcraft3Window;

typedef LRESULT( __fastcall *  WarcraftRealWNDProc )( HWND hWnd, unsigned int Msg, WPARAM wParam, LPARAM lParam );
extern WarcraftRealWNDProc WarcraftRealWNDProc_org;
extern WarcraftRealWNDProc WarcraftRealWNDProc_ptr;
void GameMain();
extern DWORD LASTINITIALIZED2;