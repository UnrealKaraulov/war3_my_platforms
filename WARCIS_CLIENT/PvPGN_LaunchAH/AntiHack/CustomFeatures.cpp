// GetTlsValue SetTlsValue optimize?
// WaitForSingleObject WaitForMultipleObject



// Game.dll+36196b NEED FIX COLOR
// [Game.dll+AB6214]+7C0


extern "C"
{
#include "bcrypt.h"
}
#include <Windows.h>
#include <MinHook.h>
#include <string>
#include <algorithm>
#include <vector>
#include <regex>
#include "Storm.h"
#include "crc32.h"
#include "CustomFeatures.h"
#include "Antihack.h"
#include <VSyncHeader.h>
#include "warcis_reconnector.h"
#include <TlHelp32.h>
using namespace std;
#include <filesystem>
namespace fs = std::filesystem;

#include "WarcraftFrameHelper.h"
using namespace NWar3Frame;

#include "buffer.h"
#include "BlpReadWrite.h"


std::string LastUsername = "";
std::string LastPassword = "";



#pragma pack(push, 1)

struct StringRep {
	void**				vtable;		//0x0
	uint32_t			refCount;	//0x4
	uint32_t			unk_8;		//0x8
	uint32_t			list_C;		//0xC
	uint32_t			unk_10;		//0x10
	uint32_t			unk_14;		//0x14
	StringRep*			next;		//0x18
	char*				text;		//0x1C
};//sizeof = 0x20

struct RCString {
	void**				vtable;		//0x0
	uint32_t			unk_4;		//0x4
	StringRep*			stringRep;	//0x8
};//sizeof = 0xC


#pragma pack(pop)


void PrintText( const char * text, float staytime )
{
	if ( IsGame( ) )
	{
		int pPrintText2 = GameDll + 0x2F69A0;
		__asm
		{
			push - 1;
			push staytime;
			push text;
			mov ecx, pW3XGlobalClass;
			mov ecx, [ ecx ];
			mov eax, pPrintText2;
			call eax;
		}
	}
}


void PrintText( std::string strtext, float staytime )
{
	const char * text = strtext.c_str( );
	if ( IsGame( ) )
	{
		int pPrintText2 = GameDll + 0x2F69A0;
		__asm
		{
			push - 1;
			push staytime;
			push text;
			mov ecx, pW3XGlobalClass;
			mov ecx, [ ecx ];
			mov eax, pPrintText2;
			call eax;
		}
	}
}





void MakeGreenFix( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	int offset = GameDll + 0xAB6214;
	if ( offset )
	{
		offset = *( int* )offset;
		if ( offset )
		{
			*( int* )( offset + 0x7C0 ) = 0xFF00FF00;
		}
	}
}


bool CompareByteArray( PBYTE Data, PBYTE Signature )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	for ( ; *Signature; ++Signature, ++Data )
	{
		if ( *Signature == '\xCC' )
		{
			continue;
		}
		if ( *Data != *Signature )
		{
			return false;
		}
	}

	return true;
}
// Поиск адреса в памяти по сигнатуре
PBYTE FindSignature( PBYTE BaseAddress, DWORD ImageSize, PBYTE Signature, size_t size = 0 )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	BYTE First = Signature[ 0 ];
	PBYTE Max = BaseAddress + ImageSize - ( size == 0 ? strlen( ( PCHAR )Signature ) : size );

	for ( ; BaseAddress < Max; ++BaseAddress )
	{
		if ( *BaseAddress != First )
		{
			continue;
		}
		if ( CompareByteArray( BaseAddress, Signature ) )
		{
			return BaseAddress;
		}
	}
	return NULL;
}







int GetGlobalPlayerData( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	return  *( int * )( 0xAB65F4 + GameDll );//1.27a 0xBE4238
}

// Получает адрес игрока по номеру слота
int GetPlayerByNumber( int number )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	int arg1 = GetGlobalPlayerData( );
	int result = 0;
	if ( arg1 > NULL )
	{
		result = ( int )arg1 + ( number * 4 ) + 0x58;
		if ( result )
		{
			result = *( int* )result;
		}
		else
		{
			return 0;
		}
	}
	return result;
}

// Получить ID игрока
int GetLocalPlayerId( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	int gldata = GetGlobalPlayerData( );
	if ( gldata > 0 )
	{
		short retval = *( short * )( gldata + 0x28 );
		return retval;
	}
	return 0;
}
// Получает количество выделенных юнитов
int GetSelectedUnitCountBigger( int slot )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	int plr = GetPlayerByNumber( slot );
	if ( plr > 0 )
	{
		int PlayerData1 = *( int* )( plr + 0x34 );
		if ( PlayerData1 > 0 )
		{
			int unitcount = *( int * )( PlayerData1 + 0x10 );
			int unitcount2 = *( int * )( PlayerData1 + 0x1D4 );

			if ( unitcount > unitcount2 )
				return unitcount;
			else
				return unitcount2;
		}
	}

	return NULL;
}
// Получает выделенного игроком юнита
int GetSelectedUnit( int slot )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( "GetSelectedUnit" );
#endif
	int plr = GetPlayerByNumber( slot );
	if ( plr > 0 )
	{
		int PlayerData1 = *( int* )( plr + 0x34 );
		if ( PlayerData1 > 0 )
		{
			return *( int * )( PlayerData1 + 0x1e0 );
		}
	}

	return NULL;
}

// Получает номер игрока-владельца юнита
int __stdcall GetUnitOwnerSlot( int unitaddr )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	if ( unitaddr > 0 )
		return *( int* )( unitaddr + 88 );
	return 15;
}




typedef int( __thiscall * pUpdateFrameFlags )( int FrameAddr, char unk );
pUpdateFrameFlags UpdateFrameFlags;



typedef BOOL( __fastcall * ReadGlobalWc3StringToBuffer )( const char * strname, char * buffer, int size );
ReadGlobalWc3StringToBuffer ReadGlobalWc3StringToBuffer_org;
ReadGlobalWc3StringToBuffer ReadGlobalWc3StringToBuffer_ptr;

// Перехваченная функция чтения строки из игровых файлов
BOOL __fastcall ReadGlobalWc3StringToBuffer_my( const char * strname, char * buffer, int size )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	BOOL retval = ReadGlobalWc3StringToBuffer_ptr( strname, buffer, size );
	if ( retval )
	{
		if ( buffer && buffer[ 0 ] != '\0' && size > 9 && strlen( buffer ) > 9 )
		{
			string TempWc3Str = string( buffer );
			if ( replaceAll( TempWc3Str, "Battle.net", "Warcis Gaming" ) )
			{
				sprintf_s( buffer, size, "%s", TempWc3Str.c_str( ) );
				//memset( buffer, 0, size );
				//CopyMemory( buffer, TempWc3Str.c_str( ), size - 1 );
			}
			else
			{
				if ( replaceAll( TempWc3Str, "Blizzard", "Warcis" ) )
				{
					sprintf_s( buffer, size, "%s", TempWc3Str.c_str( ) );
					//memset( buffer, 0, size );
					//CopyMemory( buffer, TempWc3Str.c_str( ), size - 1 );
				}
			}

		}

	}
	return retval;
}

typedef const char *( __fastcall*  GetWc3String )( const char * strname );
GetWc3String GetWc3String_org;
GetWc3String GetWc3String_ptr;

string TempWc3Str;
std::string LineForStats;

// Перехваченная функция чтения строки из fdf файлов
const char * __fastcall  GetWc3String_my( const char * strname )
{
	const char * retval = GetWc3String_ptr( strname );
	if ( retval && retval[ 0 ] != '\0' )
	{
		TempWc3Str = string( retval );
		if ( replaceAll( TempWc3Str, "Battle.net", "Warcis Gaming" ) )
		{
			return TempWc3Str.c_str( );
		}
		if ( replaceAll( TempWc3Str, "Blizzard", "Warcis" ) )
		{
			return TempWc3Str.c_str( );
		}
		else if ( replaceAll( TempWc3Str, "|CffffffffB|Rattle.net", "|Cffff0000Warcis|r Gaming" ) )
		{
			return  TempWc3Str.c_str( );
		}
	}
	return retval;
}


typedef int( __fastcall * SetGameAreaFOV )( Matrix1 * a1, int a2, float a3, float a4, float a5, float a6 );
SetGameAreaFOV SetGameAreaFOV_org = NULL;
SetGameAreaFOV SetGameAreaFOV_ptr;
float * GetWindowXoffset;
float * GetWindowYoffset;

float CustomFovFix = 1.0f;

void SimpleWc3Work( );

bool FixFieldOfView = true;

// Подмена так называемого  "Field of view"
int __fastcall SetGameAreaFOV_new( Matrix1 * a1, int _unused, float a3, float a4, float a5, float a6 )
{

	if ( FixFieldOfView )
	{
		float ScreenX = *GetWindowXoffset;
		float ScreenY = *GetWindowYoffset;

		float v1 = 1.0f / sqrt( a4 * a4 + 1.0f );
		float v2 = tan( v1 * a3 * 0.5f );

		float v3 = v2 * a5;
		float v4 = v3 * a4;


		a1->flt1 = ( ( a5 * ( 4.0f / 3.0f ) ) / ( ScreenX / ScreenY ) * CustomFovFix ) / v4; // Fix 4:3 to WindowX/WindowY
		a1->flt2 = 0.0f;
		a1->flt3 = 0.0f;
		a1->flt4 = 0.0f;
		a1->flt5 = 0.0f;


		a1->flt6 = a5 / v3;
		a1->flt7 = 0.0f;
		a1->flt8 = 0.0f;
		a1->flt9 = 0.0f;
		a1->flt10 = 0.0f;


		a1->flt11 = ( a5 + a6 ) / ( a6 - a5 );
		a1->flt12 = 1.0f;
		a1->flt13 = 0.0f;
		a1->flt14 = 0.0f;


		a1->flt15 = a5 * ( a6 * -2.0f ) / ( a6 - a5 );
		a1->flt16 = 0.0f;

	}
	else
	{
		SetGameAreaFOV_ptr( a1, _unused, a3, a4, a5, a6 );
	}
	//SimpleWc3Work( );

	return 0;
}

typedef int( __stdcall * IsBlizzardMap )( const char * name, int, size_t, int, int, int );
IsBlizzardMap IsBlizzardMap_org;
IsBlizzardMap IsBlizzardMap_ptr;

// Все карты будут считаться официальными! (можно закидывать любые карты в автопоиск игр через МЕЧ)
int __stdcall  IsBlizzardMap_my( const char * name, int a2, size_t a3, int a4, int a5, int a6 )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	return TRUE;
}


pGetProcAddress GetProcAddress_org;
pGetProcAddress GetProcAddress_ptr;

vector<string> BlackListProc;

// Запрет получения адреса функции из черного списка из неоткуда.
FARPROC WINAPI  GetProcAddress_my( HMODULE hModule, LPCSTR  lpProcName )
{
	int RetAddr = ( int )_ReturnAddress( );

#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif

	if ( !( int )lpProcName || !hModule )
		return  GetProcAddress_ptr( hModule, lpProcName );

	if ( GetFoundWhiteListMapValue( ) == 0 )
	{
		void * accessmodule = GetModuleFromAddress( RetAddr );
		if ( !accessmodule )
			return 0;
	}

	return  GetProcAddress_ptr( hModule, lpProcName );
	//
	//#ifndef  ANTIHACKNODEBUG
	//	AddLogFunctionCall( __FUNCSIGW__ );
	//	CONSOLE_Print( "GetProcAddr:" );
	//	CONSOLE_Print(lpProcName);
	//#endif
	//
	//
	//	if ( !accessmodule )
	//	{
	//
	//#ifndef  ANTIHACKNODEBUG
	//		AddLogFunctionCall( __FUNCSIGW__ );
	//#endif
	//		// с черным списком, 
	//		for ( string s : BlackListProc )
	//		{
	//			if ( s == lpProcName )
	//				return 0;
	//		}
	//
	//#ifndef  ANTIHACKNODEBUG
	//		AddLogFunctionCall( __FUNCSIGW__ );
	//#endif
	//		if ( GetFoundWhiteListMapValue( ) == 0 )
	//			return 0;
	//	}
	//
	//#ifndef  ANTIHACKNODEBUG
	//	AddLogFunctionCall( __FUNCSIGW__ );
	//#endif
	//	FARPROC retproc = GetProcAddress_ptr( hModule, lpProcName );
	//	//...
	//
	//#ifndef  ANTIHACKNODEBUG
	//	AddLogFunctionCall( __FUNCSIGW__ );
	//#endif
	//	return retproc;
}




typedef DWORD( WINAPI* pGetTickCount )( );
pGetTickCount GetTickCount_org;
pGetTickCount GetTickCount_ptr;

float GameSpeed = 1.0f;
bool SpeedUpEnabled = false;

double GameSpeedProtect = GameSpeed;

DWORD gtc_last_real, gtc_last_fake;

// Ускорение игры при необходимости
DWORD WINAPI GetTickCount_my( )
{


#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	static DWORD gtc_last_real, gtc_last_fake;
	DWORD ret = GetTickCount_ptr( );
	DWORD nReal = ret;
	DWORD dReal = nReal - gtc_last_real;
	DWORD dFake = ( DWORD )( GameSpeed * dReal );

	if ( SpeedUpEnabled )
	{
		ret = gtc_last_fake + dFake;
		gtc_last_fake += dFake;
	}
	else
	{
		ret = gtc_last_fake + dReal;
		gtc_last_fake += dReal;
	}
	gtc_last_real += dReal;


	return ret;
}

// Ускорение игры 
void SetGameSpeed( float newspeed, bool enablespeedhack )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	if ( GameSpeedProtect != GameSpeed )
		FoundModifiedLoader = 2;
	/*DWORD oldprot;
	VirtualProtect( &GameSpeed, 4, PAGE_READWRITE, &oldprot );
	if ( oldprot != PAGE_READONLY )
		FoundModifiedLoader = 2;*/
	GameSpeed = newspeed;
	GameSpeedProtect = GameSpeed;

	SpeedUpEnabled = enablespeedhack;
	//VirtualProtect( &GameSpeed, 4, oldprot, &oldprot );
}



BOOL NeedAutoJoin = FALSE;

int PressBnetButtonTimed = 0;
int PressLogonButtonTimed = 0;

CWar3Frame * LogonButton = NULL;
CWar3Frame * OKButton = NULL;


float SpeedUpValue = 2.0f;
int SpeedUpTime = -2000;
// Ускорение игры на время
void SpeedUpForTime( int time, float speed )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	SpeedUpValue = speed;
	SpeedUpTime = time;
}

typedef int( __fastcall * pGetFrameItemAddress )( const char * name, int id );
pGetFrameItemAddress GetFrameItemAddress;

//0BE300
typedef int( __fastcall * pSetFrameMaxBuffer ) ( int FrameAddr, int size );
pSetFrameMaxBuffer SetFrameMaxBuffer;

typedef int( __thiscall *  pWc3ControlClickButton )( int btnaddr, int unk );
pWc3ControlClickButton Wc3ControlClickButton_ptr;
pWc3ControlClickButton Wc3ControlClickButton_org;
int __fastcall Wc3ControlClickButton_my( int btnaddr, int, int unk );

typedef int( __thiscall *  pWc3ControlMouseBeforeClickButton )( int btnaddr, int unk );
pWc3ControlMouseBeforeClickButton Wc3ControlMouseBeforeClickButton_org;
pWc3ControlMouseBeforeClickButton Wc3ControlMouseBeforeClickButton_ptr;

typedef int( __thiscall * SetFrameText_p )( int frameaddr, const char * newtext );
SetFrameText_p SetFrameText;
SetFrameText_p EditBoxInsertText;


typedef int( __thiscall * SetFrameText2_p )( int frameaddr, const char * newtext, BOOL unk );
SetFrameText2_p SetEditBoxText;

//AF200
typedef int( __fastcall * InitFrameDef )( void *a1, int unused, int a2, int a3 );
InitFrameDef InitWc3MainMenu_org;
InitFrameDef InitWc3MainMenu_ptr;

InitFrameDef InitWc3BattleNetMenu_org;
InitFrameDef InitWc3BattleNetMenu_ptr;

InitFrameDef BattleNetChatPanel_org;
InitFrameDef BattleNetChatPanel_ptr;

InitFrameDef BattleNetCustomJoinPanel_org;
InitFrameDef BattleNetCustomJoinPanel_ptr;

int PressEnterChatButtonTimed = 0;


int FollowStatus;
// 0 follow завершен
// 1 требуется войти в игру
// 2 требуется выйти из игры

std::string LatestRequestGame = " ";
std::string FollowGameName = " ";

int FollowTryCount = 5;

int FollowEventLimitTime = 500;

bool BattleNetButtonPressed = false;


int BattleNetButtonAddr = 0;
int ConnectButtonAddr = 0;
int CustomGameButtonAddr = 0;
int CustomGameExitButtonAddr = 0;
int CustomGameNameButtonAddr = 0;
int CustomLoadGameButtonAddr = 0;
int CustomGameNameEditBox = 0;
int LobbyLeaveButtonAddr = 0;
int StandardGameButtonAddr = 0;
int QuickStandardGameButtonAddr = 0;
int StandardTeamGameButtonAddr = 0;


Wc3Menu current_menu = Wc3Menu::MAIN_MENU;


const float OPTIONMENU_WIDTH = 0.6f;
const float OPTIONMENU_HEIGHT = 0.375f;
const float OPTIONMENU_CENTER_X = 0.4f;
const float OPTIONMENU_CENTER_Y = 0.375f;
const float NAVIGATION_WIDTH = 0.125f;




CWar3Frame * autologinshowbtn = NULL;
CWar3Frame * autologinshowbtnitem = NULL;
CWar3Frame * SuperDebugButton = NULL;
CWar3Frame * SuperDebugButtonItem = NULL;


CWar3Frame * mainframe = NULL;
CWar3Frame * exitbutton = NULL;
CWar3Frame * autologin_save = NULL;
CWar3Frame * autologin_clear = NULL;
CWar3Frame * autologin_username = NULL;
CWar3Frame * autologin_password = NULL;
CWar3Frame * autologin_go = NULL;


CWar3Frame * hostmainframe = NULL;
CWar3Frame * host_mapscrollbar = NULL;
CWar3Frame * host_cancelbtn = NULL;
CWar3Frame * host_creategamebtn = NULL;
CWar3Frame * host_fastsearch = NULL;
CWar3Frame * host_gamename = NULL;
CWar3Frame * host_mapgenrefiltermenu = NULL;
CWar3Frame * host_modelist = NULL;
CWar3Frame * host_playerlist = NULL;

CWar3Frame * HostMenuMapStats = NULL;
CWar3Frame * HostMenuMapPublic = NULL;

CWar3Frame * mainmenumodel = NULL;

void InitCFrame( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
}

void ShowAutologinWindow( );
void ShowMapHostMenu( int startid = 0 );


int Pos = 0;
float scaletest = 0.05;

int AutoLoginMenuCallback( CWar3Frame*frame, int FrameAddr, uint32_t ClickType )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif

	if ( !frame )
		return 0;
	//	char checkboxaddr[ 100 ];
	//	sprintf_s( checkboxaddr, "1 CB:%X -> EV:%X -> FIX:%i", frame->FrameAddr, SuperDebugButtonItem ? SuperDebugButtonItem->FrameAddr : 0xFFFFFFFF, ClickType );
	//MessageBox( 0, checkboxaddr, "", 0 );

	if ( frame == exitbutton )
	{
		CWar3Frame::Wc3PlaySound( "T02Narrator027" );

		if ( mainframe )
		{
			mainframe->DestroyThisFrame( );
			delete mainframe;
			delete exitbutton;
			delete autologin_save;
			delete autologin_clear;
			delete autologin_username;
			delete autologin_password;
			delete autologin_go;
			//delete mainmenumodel;
			mainframe = NULL;
		}

	}
	else if ( frame == autologin_save )
	{
		if ( ClickType == CFrameEvents::FRAME_CHECKBOX_CHECKED )
		{		// checkbox checked
			gInfo.NeedSaveAutologin = true;
		}
		else
		{
			gInfo.NeedSaveAutologin = false;
		}
	}
	else if ( frame == autologin_clear )
	{
		// mouse click
		autologin_username->SetText( "" );
		autologin_password->SetText( "" );
		gInfo.Username[ 0 ] = '\0';
		gInfo.Password[ 0 ] = '\0';
		gInfo.NeedSaveAutologin = FALSE;
		autologin_save->SetChecked( false );

		CWar3Frame::Wc3PlaySound( "S08Tyrande55" );
	}
	else if ( frame == autologin_username )
	{
		//  text changed
		if ( frame->GetText( ) && frame->GetText( )[ 0 ] != '\0' )
		{
			memcpy( gInfo.Username, frame->GetText( ), 16 );
		}
		else
			gInfo.Username[ 0 ] = '\0';
		CWar3Frame::Wc3PlaySound( "WayPoint" );
	}
	else if ( frame == autologin_password )
	{
		//  text changed
		if ( frame->GetText( ) && frame->GetText( )[ 0 ] != '\0' )
		{
			size_t textcopylen = strlen( frame->GetText( ) ) > 49 ? 49 : strlen( frame->GetText( ) );
			memset( gInfo.Password, 0, sizeof( gInfo.Password ) );
			memcpy( gInfo.Password, frame->GetText( ), textcopylen );
		}
		else
			gInfo.Password[ 0 ] = '\0';

		CWar3Frame::Wc3PlaySound( "WayPoint" );
	}
	else if ( frame == autologinshowbtnitem )
	{
		ShowAutologinWindow( );
		CWar3Frame::Wc3PlaySound( "T02Grunt006" );
	}
	else if ( frame == autologin_go )
	{
		NeedAutoJoin = TRUE;
		PressBnetButtonTimed = 1;
		CWar3Frame::Wc3PlaySound( "A07Illidan20" );

		if ( mainframe )
		{
			mainframe->DestroyThisFrame( );
			delete mainframe;
			delete exitbutton;
			delete autologin_save;
			delete autologin_clear;
			delete autologin_username;
			delete autologin_password;
			delete autologin_go;
			//delete mainmenumodel;
			mainframe = NULL;
		}
	}
	else if ( frame == SuperDebugButtonItem )
	{
		CONSOLE_Print( "SuperDebugButtonItem 1" );
		autologinshowbtnitem->SetFrameScale( CFrameBackdropType::ControlFrame, scaletest, scaletest );
		//autologinshowbtn->SetFrameRelativePosition( CFramePosition::LEFT, GetFrameItemAddress( "BattleNetButton", 0 ), CFramePosition::BOTTOM_LEFT, -0.025f, 0.015f );


		scaletest -= 0.005;
		//int offset = 0;
		//*( int * )offset = 0;


		//CONSOLE_Print( "SuperDebugButtonItem 1" );
		///*CWar3Frame::Wc3PlaySound( "T02Narrator014" );
		//ShowMapHostMenu( );*/
		//SuperDebugButtonItem->SetText( SuperDebugButtonItem->GetFrameCustomValue( ) ? "Enabled" : "Disabled" );
		//CONSOLE_Print( "SuperDebugButtonItem 2" );
		//SuperDebugButtonItem->SetFrameCustomValue( !SuperDebugButtonItem->GetFrameCustomValue( ) );
		//CONSOLE_Print( "SuperDebugButtonItem 3" );
		//CWar3Frame backimg = CWar3Frame( );
		//backimg.CWar3FrameFromAddress( SuperDebugButtonItem->GetFrameBackdropAddress( CFrameBackdropType::ControlBackdrop ) );
		//backimg.SetFrameType( CFrameType::FRAMETYPE_BACKDROP );
		//CONSOLE_Print( "SuperDebugButtonItem 4" );
		//backimg.SetTexture( SuperDebugButtonItem->GetFrameCustomValue( ) ? "WarcisPictures\\UserCantSpeak.blp" : "WarcisPictures\\UserWantToSpeak.blp" );
		//CONSOLE_Print( "SuperDebugButtonItem 5" );

		//autologinshowbtn->Show( SuperDebugButtonItem->GetFrameCustomValue( ) > 0 );

	}
	else if ( frame == mainmenumodel )
	{
		frame->anim_offset += 0.001;
		/*if ( ClickType == CFrameEvents::FRAME_MOUSE_ENTER )
		{
			mainmenumodel->SetModel( "UI\\Glues\\BattleNet\\BattlenetWorkingExpansion\\BattlenetWorkingExp.mdl" );
		}
		else if ( ClickType == CFrameEvents::FRAME_MOUSE_LEAVE )
		{
			mainmenumodel->SetModel( Wc3GetSkinItemPath( "MainMenuLogo_V1", 0 ) );
		}*/
	}

	return 1;
}

CWar3Frame * MapListInfos[ 12 ];
CWar3Frame * MapListNames[ 12 ];
CWar3Frame * MapListHostCmd[ 12 ];
std::vector<MapHostStruct>  MapHostList;
std::vector<MapHostStruct>  FilteredMapHostList;
std::vector<std::string> GenresList;
int MapListStartedIndex = 0;

std::vector<MapInfosStruct> ListMapInfos;

std::vector<string> FileNamesOut;
void BuildFileList( const std::string &directory )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	std::string tmp = directory + "\\*";
	WIN32_FIND_DATA file;
	HANDLE search_handle = FindFirstFileA( tmp.c_str( ), &file );
	if ( search_handle != INVALID_HANDLE_VALUE )
	{
		std::vector<std::string> directories;

		do
		{
			if ( file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				if ( ( !lstrcmpA( file.cFileName, "." ) ) || ( !lstrcmpA( file.cFileName, ".." ) ) )
					continue;
			}

			tmp = directory + "\\" + std::string( file.cFileName );
			FileNamesOut.push_back( tmp );

			if ( file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
				directories.push_back( tmp );
		} while ( FindNextFileA( search_handle, &file ) );

		FindClose( search_handle );

		for ( std::vector<std::string>::iterator iter = directories.begin( ), end = directories.end( ); iter != end; ++iter )
			BuildFileList( *iter );
	}
}

string getFileExt( const string& s ) {

	size_t i = s.rfind( '.', s.length( ) );
	if ( i != string::npos ) {
		return( s.substr( i + 1, s.length( ) - i ) );
	}

	return( "" );
}

std::vector<string> get_file_list_windows( const std::string & path, std::vector<std::string> exts = std::vector<std::string>( ) )
{
	FileNamesOut.clear( );
	std::vector<string> tmp;

	BuildFileList( path );

	for ( auto fname : FileNamesOut )
	{
#ifndef  ANTIHACKNODEBUG
		AddLogFunctionCall( __FUNCSIGW__ );
#endif
		std::string curext = ToLower( getFileExt( fname ) );
		for ( auto s : exts )
		{
			if ( curext == ToLower( s ) )
			{
				tmp.push_back( fname );

				break;
			}
		}
	}
	FileNamesOut.clear( );
	return tmp;
}


std::vector<std::string> get_file_list( const fs::path & path, std::vector<std::string> exts = std::vector<std::string>( ) )
{
	std::vector<std::string> m_file_list;
	if ( !path.empty( ) )
	{
		fs::path apk_path( path );
		fs::recursive_directory_iterator end;

		for ( fs::recursive_directory_iterator i( apk_path ); i != end; ++i )
		{
			fs::path cp = ( *i );
			if ( exts.size( ) > 0 )
			{
#ifndef  ANTIHACKNODEBUG
				AddLogFunctionCall( __FUNCSIGW__ );
#endif
				std::string curext = ToLower( cp.filename( ).extension( ).string( ) );
				for ( auto s : exts )
				{
					if ( curext == ToLower( s ) )
					{
						string filename = cp.string( );
						if ( FileExists( filename ) )
							m_file_list.push_back( filename );
						else
						{
							CONSOLE_Print( "ERROR! FILE" + cp.string( ) + " NON EXISTS!" );

							filename = ConvertFromUtf16ToUtf888( cp.wstring( ) );

							if ( FileExists( filename ) )
							{
								CONSOLE_Print( "Super converted file exists! o_o..." );
								m_file_list.push_back( filename );
							}

						}
						break;
					}

				}
			}
			else
				m_file_list.push_back( cp.string( ) );
		}
	}
	return m_file_list;
}


void DumpFileToDisk( const void * filedata, const uint32_t filesize, const char * filename )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	FILE * f;
	fopen_s( &f, filename, "wb" );
	if ( f )
		fwrite( filedata, filesize, 1, f );
	fclose( f );
}


// Signed variables are for wimps 
#define uchar unsigned char 
#define uint unsigned int 

// DBL_INT_ADD treats two unsigned ints a and b as one 64-bit integer and adds c to it
#define ROTLEFT(a,b) ((a << b) | (a >> (32-b))) 
#define DBL_INT_ADD(a,b,c) if (a > 0xffffffff - c) ++b; a += c; 


typedef struct {
	uchar data[ 64 ];
	uint datalen;
	uint bitlen[ 2 ];
	uint state[ 5 ];
	uint k[ 4 ];
} SHA1_CTX;


void sha1_transform( SHA1_CTX *ctx, uchar data[ ] )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	uint a, b, c, d, e, i, j, t, m[ 80 ];

	for ( i = 0, j = 0; i < 16; ++i, j += 4 )
		m[ i ] = ( data[ j ] << 24 ) + ( data[ j + 1 ] << 16 ) + ( data[ j + 2 ] << 8 ) + ( data[ j + 3 ] );
	for ( ; i < 80; ++i ) {
		m[ i ] = ( m[ i - 3 ] ^ m[ i - 8 ] ^ m[ i - 14 ] ^ m[ i - 16 ] );
		m[ i ] = ( m[ i ] << 1 ) | ( m[ i ] >> 31 );
	}

	a = ctx->state[ 0 ];
	b = ctx->state[ 1 ];
	c = ctx->state[ 2 ];
	d = ctx->state[ 3 ];
	e = ctx->state[ 4 ];

	for ( i = 0; i < 20; ++i ) {
		t = ROTLEFT( a, 5 ) + ( ( b & c ) ^ ( ~b & d ) ) + e + ctx->k[ 0 ] + m[ i ];
		e = d;
		d = c;
		c = ROTLEFT( b, 30 );
		b = a;
		a = t;
	}
	for ( ; i < 40; ++i ) {
		t = ROTLEFT( a, 5 ) + ( b ^ c ^ d ) + e + ctx->k[ 1 ] + m[ i ];
		e = d;
		d = c;
		c = ROTLEFT( b, 30 );
		b = a;
		a = t;
	}
	for ( ; i < 60; ++i ) {
		t = ROTLEFT( a, 5 ) + ( ( b & c ) ^ ( b & d ) ^ ( c & d ) ) + e + ctx->k[ 2 ] + m[ i ];
		e = d;
		d = c;
		c = ROTLEFT( b, 30 );
		b = a;
		a = t;
	}
	for ( ; i < 80; ++i ) {
		t = ROTLEFT( a, 5 ) + ( b ^ c ^ d ) + e + ctx->k[ 3 ] + m[ i ];
		e = d;
		d = c;
		c = ROTLEFT( b, 30 );
		b = a;
		a = t;
	}

	ctx->state[ 0 ] += a;
	ctx->state[ 1 ] += b;
	ctx->state[ 2 ] += c;
	ctx->state[ 3 ] += d;
	ctx->state[ 4 ] += e;
}

void sha1_init( SHA1_CTX *ctx )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	ctx->datalen = 0;
	ctx->bitlen[ 0 ] = 0;
	ctx->bitlen[ 1 ] = 0;
	ctx->state[ 0 ] = 0x67452301;
	ctx->state[ 1 ] = 0xEFCDAB89;
	ctx->state[ 2 ] = 0x98BADCFE;
	ctx->state[ 3 ] = 0x10325476;
	ctx->state[ 4 ] = 0xc3d2e1f0;
	ctx->k[ 0 ] = 0x5a827999;
	ctx->k[ 1 ] = 0x6ed9eba1;
	ctx->k[ 2 ] = 0x8f1bbcdc;
	ctx->k[ 3 ] = 0xca62c1d6;
}

void sha1_update( SHA1_CTX *ctx, uchar data[ ], uint len )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	uint t, i;

	for ( i = 0; i < len; ++i ) {
		ctx->data[ ctx->datalen ] = data[ i ];
		ctx->datalen++;
		if ( ctx->datalen == 64 ) {
			sha1_transform( ctx, ctx->data );
			DBL_INT_ADD( ctx->bitlen[ 0 ], ctx->bitlen[ 1 ], 512 );
			ctx->datalen = 0;
		}
	}
}

void sha1_final( SHA1_CTX *ctx, uchar hash[ ] )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	uint i;

	i = ctx->datalen;

	// Pad whatever data is left in the buffer. 
	if ( ctx->datalen < 56 ) {
		ctx->data[ i++ ] = 0x80;
		while ( i < 56 )
			ctx->data[ i++ ] = 0x00;
	}
	else {
		ctx->data[ i++ ] = 0x80;
		while ( i < 64 )
			ctx->data[ i++ ] = 0x00;
		sha1_transform( ctx, ctx->data );
		memset( ctx->data, 0, 56 );
	}

	// Append to the padding the total message's length in bits and transform. 
	DBL_INT_ADD( ctx->bitlen[ 0 ], ctx->bitlen[ 1 ], 8 * ctx->datalen );
	ctx->data[ 63 ] = ctx->bitlen[ 0 ];
	ctx->data[ 62 ] = ctx->bitlen[ 0 ] >> 8;
	ctx->data[ 61 ] = ctx->bitlen[ 0 ] >> 16;
	ctx->data[ 60 ] = ctx->bitlen[ 0 ] >> 24;
	ctx->data[ 59 ] = ctx->bitlen[ 1 ];
	ctx->data[ 58 ] = ctx->bitlen[ 1 ] >> 8;
	ctx->data[ 57 ] = ctx->bitlen[ 1 ] >> 16;
	ctx->data[ 56 ] = ctx->bitlen[ 1 ] >> 24;
	sha1_transform( ctx, ctx->data );

	// Since this implementation uses little endian byte ordering and MD uses big endian, 
	// reverse all the bytes when copying the final state to the output hash. 
	for ( i = 0; i < 4; ++i ) {
		hash[ i ] = ( ctx->state[ 0 ] >> ( 24 - i * 8 ) ) & 0x000000ff;
		hash[ i + 4 ] = ( ctx->state[ 1 ] >> ( 24 - i * 8 ) ) & 0x000000ff;
		hash[ i + 8 ] = ( ctx->state[ 2 ] >> ( 24 - i * 8 ) ) & 0x000000ff;
		hash[ i + 12 ] = ( ctx->state[ 3 ] >> ( 24 - i * 8 ) ) & 0x000000ff;
		hash[ i + 16 ] = ( ctx->state[ 4 ] >> ( 24 - i * 8 ) ) & 0x000000ff;
	}
}

std::string GetFileSha1Hash( const std::string & filename )
{
	std::string retval = "";
	FILE  *f = NULL;
	fopen_s( &f, filename.c_str( ), "rb" );
	fseek( f, 0L, SEEK_END );
	size_t sz = ftell( f );
	fseek( f, 0L, SEEK_SET );
	unsigned char * filebuf = new unsigned char[ sz ];
	unsigned char hash[ 20 ];
	fread( filebuf, sz, 1, f );
	fclose( f );

	SHA1_CTX ctx;
	sha1_init( &ctx );
	sha1_update( &ctx, filebuf, sz );
	sha1_final( &ctx, hash );
	std::ostringstream oss;
	for ( int i = 0; i < 20; i++ )
		oss << std::hex << std::setw( 2 ) << std::setfill( '0' ) << static_cast< int >( hash[ i ] );
	retval = oss.str( );
	delete[ ] filebuf;
	return retval;
}
std::vector<MapInfosStruct> TmpListMapInfos;

DWORD WINAPI InitMapInfos( LPVOID )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	SetThreadPriority( GetCurrentThread( ), THREAD_PRIORITY_HIGHEST );
	CONSOLE_Print( "InitMapInfos" );

	auto MapsDir = "Maps";
	std::vector<std::string> MapsDirFiles = get_file_list_windows( MapsDir, { "w3x","w3m" } );
	//v6 = Storm_266( v1, 16, 4, &v10 );
	uint32_t fileid = 1;

	std::time_t StartLoadMapInfosTime = std::time( 0 );

	if ( gInfo.MaxMapPreloadTime < 5 && gInfo.MaxMapPreloadTime != -1)
		gInfo.MaxMapPreloadTime = 5;

	for ( auto s : MapsDirFiles )
	{
		DWORD StartCurrentMapLoadTime = GetTickCount( );

		if ( std::time( 0 ) - StartCurrentMapLoadTime > gInfo.MaxMapPreloadTime )
			break;

		MapInfosStruct  tmpMapInfo;

		fileid++;
		CONSOLE_Print( "Found file" );
		CONSOLE_Print( s );

		//CONSOLE_Print( "File hash:" + GetFileSha1Hash( s ) );

		//	CONSOLE_Print( "Map" );

		//	CONSOLE_Print( "Map2" );
#ifndef  ANTIHACKNODEBUG
		AddLogFunctionCall( __FUNCSIGW__ );
#endif
		std::string FileNameLower = ToLower( s );
		//	CONSOLE_Print( "Map3" );
		tmpMapInfo.mapfilename = FileNameLower;
		//	CONSOLE_Print( "Map4" );
		HANDLE tmpMap = NULL;
		char filenamebuf[ 256 ];
		HANDLE tmpFile = NULL;
		CONSOLE_Print( "Open archive..." );
		/*DumpFileToDisk( "Hello", 5, "Test.txt" );
*/
		if ( SFileOpenArchive( s.c_str( ), 0, MPQ_OPEN_FORCE_MPQ_V1, &tmpMap ) )
		{
			CONSOLE_Print( "Open ok..." );
			if ( SFileOpenFileEx( tmpMap, "war3mapMap.blp", 0, &tmpFile ) )
			{
				uint32_t FileSize = SFileGetFileSize( tmpFile, 0 );
				if ( FileSize )
				{
					char * filebuffer = new char[ FileSize ];

					if ( SFileReadFile( tmpFile, filebuffer, FileSize, 0, 0 ) )
					{
						BOOL IsBlp = memcmp( filebuffer, "BLP1", 4 ) == 0;

						sprintf_s( filenamebuf, "war3mapMap%u.%s", fileid, "blp" );

						if ( IsBlp )
						{
							tmpMapInfo.minimapfilename = filenamebuf;
							tmpMapInfo.MiniMapData = ( char * )filebuffer;
							tmpMapInfo.MiniMapDataSize = FileSize;
						}
						else
						{
							StormBuffer input;
							input.buf = ( char * )filebuffer;
							input.length = FileSize;
							StormBuffer output;
							int width = 0;
							int height = 0;
							int bpp = 4;

							if ( TGA2Raw( input, output, width, height, bpp, filenamebuf ) )
							{
								char * oldbuf = output.buf;
								ScaleImage( ( unsigned char * )oldbuf, width, height, 256, 256, 4, output );
								free( oldbuf );
								int mipmaps = 10;
								CreatePalettedBLP( output, input, 256, filenamebuf, 256, 256, 4, 1, mipmaps );
								delete[ ] output.buf;
								tmpMapInfo.minimapfilename = filenamebuf;
								tmpMapInfo.MiniMapData = ( char * )input.buf;
								tmpMapInfo.MiniMapDataSize = input.length;
							}

							delete[ ] filebuffer;
						}
						//DumpFileToDisk( filebuffer, FileSize, filenamebuf );
					}
				}
				SFileCloseFile( tmpFile );
			}
			else if ( SFileOpenFileEx( tmpMap, "war3mapMap.tga", 0, &tmpFile ) )
			{
				uint32_t FileSize = SFileGetFileSize( tmpFile, 0 );
				if ( FileSize )
				{
					char * filebuffer = new char[ FileSize ];
					if ( SFileReadFile( tmpFile, filebuffer, FileSize, 0, 0 ) )
					{
						BOOL IsBlp = memcmp( filebuffer, "BLP1", 4 ) == 0;

						sprintf_s( filenamebuf, "war3mapMap%u.%s", fileid, "blp" );

						if ( IsBlp )
						{
							tmpMapInfo.minimapfilename = filenamebuf;
							tmpMapInfo.MiniMapData = ( char * )filebuffer;
							tmpMapInfo.MiniMapDataSize = FileSize;
						}
						else
						{
							StormBuffer input;
							input.buf = ( char * )filebuffer;
							input.length = FileSize;
							StormBuffer output;
							int width = 0;
							int height = 0;
							int bpp = 4;

							if ( TGA2Raw( input, output, width, height, bpp, filenamebuf ) )
							{
								char * oldbuf = output.buf;
								ScaleImage( ( unsigned char * )oldbuf, width, height, 256, 256, 4, output );
								free( oldbuf );
								int mipmaps = 10;
								CreatePalettedBLP( output, input, 256, filenamebuf, 256, 256, 4, 1, mipmaps );
								delete[ ] output.buf;
								tmpMapInfo.minimapfilename = filenamebuf;
								tmpMapInfo.MiniMapData = ( char * )input.buf;
								tmpMapInfo.MiniMapDataSize = input.length;
							}
							delete[ ] filebuffer;
						}
						//	DumpFileToDisk( filebuffer, FileSize, tmpMapInfo.minimapfilename.c_str( ) );
					}
				}
				SFileCloseFile( tmpFile );
			}
			CONSOLE_Print( "Open minimap ok..." );

			if ( SFileOpenFileEx( tmpMap, "war3mapPreview.blp", 0, &tmpFile ) )
			{
				CONSOLE_Print( "found blp..." );

				uint32_t FileSize = SFileGetFileSize( tmpFile, 0 );
				if ( FileSize )
				{
					CONSOLE_Print( "found filesize..." );

					char * filebuffer = new char[ FileSize ];
					if ( SFileReadFile( tmpFile, filebuffer, FileSize, 0, 0 ) )
					{
						CONSOLE_Print( "read file ok" );

						BOOL IsBlp = memcmp( filebuffer, "BLP1", 4 ) == 0;

						sprintf_s( filenamebuf, "war3mapPreview%u.%s", fileid, "blp" );

						if ( IsBlp )
						{
							tmpMapInfo.previewfilename = filenamebuf;
							tmpMapInfo.PreviewData = ( char * )filebuffer;
							tmpMapInfo.PreviewDataSize = FileSize;
						}
						else
						{
							StormBuffer input;
							input.buf = ( char * )filebuffer;
							input.length = FileSize;
							StormBuffer output;
							int width = 0;
							int height = 0;
							int bpp = 4;

							if ( TGA2Raw( input, output, width, height, bpp, filenamebuf ) )
							{
								char * oldbuf = output.buf;
								ScaleImage( ( unsigned char * )oldbuf, width, height, 256, 256, 4, output );
								free( oldbuf );
								int mipmaps = 10;
								CreatePalettedBLP( output, input, 256, filenamebuf, 256, 256, 4, 1, mipmaps );
								delete[ ] output.buf;
								tmpMapInfo.previewfilename = filenamebuf;
								tmpMapInfo.PreviewData = ( char * )input.buf;
								tmpMapInfo.PreviewDataSize = input.length;
							}
							delete[ ] filebuffer;
						}
						//	DumpFileToDisk( filebuffer, FileSize, tmpMapInfo.minimapfilename.c_str( ) );
					}
				}
				SFileCloseFile( tmpFile );
			}
			else if ( SFileOpenFileEx( tmpMap, "war3mapPreview.tga", 0, &tmpFile ) )
			{
				CONSOLE_Print( "found tga..." );

				uint32_t FileSize = SFileGetFileSize( tmpFile, 0 );
				if ( FileSize )
				{
					CONSOLE_Print( "found filesize..." );

					char * filebuffer = new char[ FileSize ];
					if ( SFileReadFile( tmpFile, filebuffer, FileSize, 0, 0 ) )
					{
						CONSOLE_Print( "read file ok" );

						BOOL IsBlp = memcmp( filebuffer, "BLP1", 4 ) == 0;

						sprintf_s( filenamebuf, "war3mapPreview%u.%s", fileid, "blp" );
						if ( IsBlp )
						{
							CONSOLE_Print( "found blp..." );

							tmpMapInfo.previewfilename = filenamebuf;
							tmpMapInfo.PreviewData = ( char * )filebuffer;
							tmpMapInfo.PreviewDataSize = FileSize;
						}
						else
						{
							CONSOLE_Print( "found tga..." );

							StormBuffer input;
							input.buf = ( char * )filebuffer;
							input.length = FileSize;
							StormBuffer output;
							int width = 0;
							int height = 0;
							int bpp = 4;
							CONSOLE_Print( "buffer created... File size:" + to_string( FileSize ) );

							if ( TGA2Raw( input, output, width, height, bpp, filenamebuf ) )
							{
								CONSOLE_Print( "tga converted to raw... Width:" + to_string( width ) + ". Height:" + to_string( height ) );

								char * oldbuf = output.buf;
								ScaleImage( ( unsigned char * )oldbuf, width, height, 256, 256, 4, output );

								if ( output.buf )
									CONSOLE_Print( "raw image scaled" );
								else
									CONSOLE_Print( "fuck!" );
								free( oldbuf );
								CONSOLE_Print( "memory cleared" );

								int mipmaps = 10;
								CreatePalettedBLP( output, input, 256, filenamebuf, 256, 256, 4, 1, mipmaps );
								CONSOLE_Print( "Blp created" );

								delete[ ] output.buf;
								CONSOLE_Print( "buf cleared" );

								tmpMapInfo.previewfilename = filenamebuf;
								tmpMapInfo.PreviewData = ( char * )input.buf;
								tmpMapInfo.PreviewDataSize = input.length;

								CONSOLE_Print( "finish" );
							}

							delete[ ] filebuffer;


						}
						//DumpFileToDisk( filebuffer, FileSize, tmpMapInfo.minimapfilename.c_str( ) );
					}
				}
				SFileCloseFile( tmpFile );
			}

			CONSOLE_Print( "Open preview ok..." );
			if ( SFileOpenFileEx( tmpMap, "war3map.j", 0, &tmpFile ) )
			{
				uint32_t FileSize = SFileGetFileSize( tmpFile, 0 );
				if ( FileSize )
				{
					void * filebuffer = new char[ FileSize ];
					if ( SFileReadFile( tmpFile, filebuffer, FileSize, 0, 0 ) )
					{
						uint32_t mapcrc32 = crc32_16bytes_prefetch( filebuffer, FileSize );
						tmpMapInfo.mapcrc32 = mapcrc32;
						//	CONSOLE_Print( "Read crc32:" + to_string( mapcrc32 ) );
					}
				}
				SFileCloseFile( tmpFile );
			}
			else if ( SFileOpenFileEx( tmpMap, "scripts\\war3map.j", 0, &tmpFile ) )
			{
				uint32_t FileSize = SFileGetFileSize( tmpFile, 0 );
				if ( FileSize )
				{
					void * filebuffer = new char[ FileSize ];
					if ( SFileReadFile( tmpFile, filebuffer, FileSize, 0, 0 ) )
					{
						uint32_t mapcrc32 = crc32_16bytes_prefetch( filebuffer, FileSize );
						tmpMapInfo.mapcrc32 = mapcrc32;
						//	CONSOLE_Print( "Read crc32:" + to_string( mapcrc32 ) );
					}
				}
				SFileCloseFile( tmpFile );
			}
			else if ( SFileOpenFileEx( tmpMap, "Scripts\\war3map.j", 0, &tmpFile ) )
			{
				uint32_t FileSize = SFileGetFileSize( tmpFile, 0 );
				if ( FileSize )
				{
					void * filebuffer = new char[ FileSize ];
					if ( SFileReadFile( tmpFile, filebuffer, FileSize, 0, 0 ) )
					{
						uint32_t mapcrc32 = crc32_16bytes_prefetch( filebuffer, FileSize );
						tmpMapInfo.mapcrc32 = mapcrc32;
						//	CONSOLE_Print( "Read crc32:" + to_string( mapcrc32 ) );
					}
				}
				SFileCloseFile( tmpFile );
			}
			else
			{
				CONSOLE_Print( "no script ok ok" );
				SFileCloseArchive( tmpMap );
				continue;
			}
			SFileCloseArchive( tmpMap );

			CONSOLE_Print( "close ok" );
			TmpListMapInfos.push_back( tmpMapInfo );
			//CONSOLE_Print( "add ok" );
		}
		//CONSOLE_Print( "next1" );
		DWORD EndCurrentMapLoadTime = GetTickCount( );
		char EndCurrentMapLoadTimeText[ 128 ];
		sprintf_s( EndCurrentMapLoadTimeText, "Map loaded in %u ms.", ( EndCurrentMapLoadTime - StartCurrentMapLoadTime ) );
		CONSOLE_Print( "EndCurrentMapLoadTimeText" );
	}

	std::time_t EndLoadMapInfosTime = std::time( 0 );
	char EndLoadMapInfosTimeText[ 128 ];
	sprintf_s( EndLoadMapInfosTimeText, "Maps loaded in %u sec.", ( EndLoadMapInfosTime - StartLoadMapInfosTime ) );
	CONSOLE_Print( EndLoadMapInfosTimeText );


	ListMapInfos = TmpListMapInfos;
	TmpListMapInfos.clear( );
	return 0;
}

void ClearMapInfos( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	for ( auto s : ListMapInfos )
	{
		if ( s.MiniMapData )
			delete[ ] s.MiniMapData;;
		if ( s.PreviewData )
			delete[ ] s.PreviewData;
	}

	ListMapInfos.clear( );
}

void ClearMapHostList( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	MapHostList.clear( );
	GenresList.clear( );
}

void AddNewMapHost( const  std::string & MapName, const  std::string &  MapHost, const  std::string &  MapPath, const  std::string &  MapGenre, std::vector<std::string> MapModes, std::vector<std::string> MapPlayers, uint32_t crc32, bool forstats )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	MapHostStruct tmp = MapHostStruct( );
	tmp.availabled = true;
	tmp.crc32 = crc32;
	tmp.ForStats = forstats;
	tmp.MapCategory = MapGenre;
	tmp.MapFileName = MapPath;
	tmp.MapHost = MapHost;
	tmp.MapModes = MapModes;
	tmp.MapName = MapName;
	tmp.MapNameForList = MapName;
	replaceAll( tmp.MapNameForList, "|c", "", 8 );
	replaceAll( tmp.MapNameForList, "|n", "" );
	replaceAll( tmp.MapNameForList, "|r", "" );
	replaceAll( tmp.MapNameForList, "|", "" );
	tmp.MapPlayers = MapPlayers;
	MapHostList.push_back( tmp );

	bool NeedAddGenre = true;
	for ( auto s : GenresList )
	{
#ifndef  ANTIHACKNODEBUG
		AddLogFunctionCall( __FUNCSIGW__ );
#endif
		if ( ToLower( s ) == ToLower( MapGenre ) )
		{
			NeedAddGenre = false;
		}
	}

	if ( NeedAddGenre )
	{
		GenresList.push_back( MapGenre );
	}
	//MessageBoxA(0, ( tmp.MapName + ". Command:" + MapHost ).c_str( ), " asd", 0 );
}

bool firstinitmenu = true;
bool SkipAllEvents = false;

std::string GenreFilterStringLower = "";
std::string NameFilterStringLower = "";

std::string SelectedMapCode = "";
std::string SelectedMapMode = "";
std::string SelectedMapPlayers = "";
std::string SelectedGameName = "";
unsigned int SelectedMapHostType = 1;
bool OnlyStats = false;



void InitMapHostList( int startedfrom = 0 )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	if ( !hostmainframe )
		return;
	MapListStartedIndex = startedfrom;
	int MapListIndex = 1;

	for ( int i = MapListIndex; i < 12; i++ )
	{
		MapListNames[ i ]->SetText( 0 );
		MapListNames[ i ]->Enable( false );


		MapListHostCmd[ i ]->SetText( 0 );
		MapListHostCmd[ i ]->Enable( false );

	}
	CWar3Frame TmpFrameItem = CWar3Frame( );
	TmpFrameItem.Load( "HostMenuMapFilter" );
	TmpFrameItem.SetFrameType( CFrameType::FRAMETYPE_POPUPMENU );

	if ( !firstinitmenu )
	{
		TmpFrameItem.ClearMenuItems( );
	}
	TmpFrameItem.AddItem( 0 );
	TmpFrameItem.AddItem( "All" );


	TmpFrameItem.SetText( "All" );
	for ( auto s : GenresList )
	{
		TmpFrameItem.AddItem( s.c_str( ) );
	}

	if ( GenreFilterStringLower.length( ) )
	{
		TmpFrameItem.SetText( GenreFilterStringLower.c_str( ) );
	}

	firstinitmenu = false;

	for ( unsigned int i = startedfrom; i < FilteredMapHostList.size( ) && MapListIndex < 12; i++ )
	{

		TmpFrameItem.Load( "WarcisHostMenuMapItem_MapId", MapListIndex );
		TmpFrameItem.SetFrameType( CFrameType::FRAMETYPE_EDITBOX );
		TmpFrameItem.Enable( true );
		TmpFrameItem.SetText( to_string( i + 1 ).c_str( ) );
		TmpFrameItem.Enable( false );

		MapListNames[ MapListIndex ]->Enable( true );


		MapListNames[ MapListIndex ]->SetText( ( FilteredMapHostList[ i ].MapNameForList.length( ) > 6 ? FilteredMapHostList[ i ].MapNameForList : FilteredMapHostList[ i ].MapFileName ).c_str( ) );

		MapListHostCmd[ MapListIndex ]->Enable( true );
		MapListHostCmd[ MapListIndex ]->SetText( FilteredMapHostList[ i ].MapHost.c_str( ) );


		MapListNames[ MapListIndex ]->SetFocus( true );

		MapListNames[ MapListIndex ]->Click( );
		MapListNames[ MapListIndex ]->SetCursor( false );


		//MapListNames[ MapListIndex ]->SetFocus( false );


		MapListHostCmd[ MapListIndex ]->SetFocus( true );

		MapListHostCmd[ MapListIndex ]->Click( );
		MapListHostCmd[ MapListIndex ]->SetCursor( false );


		//MapListHostCmd[ MapListIndex ]->SetFocus( false );

		MapListIndex++;
	}

	for ( ; MapListIndex < 12; MapListIndex++ )
	{

		TmpFrameItem.Load( "WarcisHostMenuMapItem_MapId", MapListIndex );
		TmpFrameItem.SetFrameType( CFrameType::FRAMETYPE_EDITBOX );
		TmpFrameItem.Enable( true );
		TmpFrameItem.SetText( "" );
		TmpFrameItem.Enable( false );

		MapListNames[ MapListIndex ]->Enable( false );
		MapListNames[ MapListIndex ]->SetText( "" );

		MapListHostCmd[ MapListIndex ]->Enable( false );
		MapListHostCmd[ MapListIndex ]->SetText( "" );
	}

}

int scrolllastvalue = 0;


void UpdateFilteredMapList( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	if ( host_mapscrollbar )
	{
		FilteredMapHostList.clear( );
		for ( auto & s : MapHostList )
		{
#ifndef  ANTIHACKNODEBUG
			AddLogFunctionCall( __FUNCSIGW__ );
#endif
			bool categoryok = false;
			bool nameok = false;
			if ( GenreFilterStringLower.length( ) == 0 || ToLower( s.MapCategory ) == ToLower( GenreFilterStringLower ) )
			{
				categoryok = true;
			}
			std::string curnamelower = ToLower( s.MapNameForList );
			if ( NameFilterStringLower.length( ) == 0 || curnamelower.find( ToLower( NameFilterStringLower ) ) != std::string::npos )
			{
				nameok = true;
			}

			bool statsok = true;

			if ( OnlyStats || SelectedMapHostType == 0 )
			{
				if ( !s.ForStats )
					statsok = false;
			}
			else
			{
				if ( s.ForStats )
					statsok = false;
			}

			if ( nameok && categoryok && statsok )
			{
				FilteredMapHostList.push_back( s );
			}
		}


		int maphostcount = FilteredMapHostList.size( ) - 12;
		if ( maphostcount < 0 )
			maphostcount = 1;

		host_mapscrollbar->SetScrollBarValuesCount( maphostcount, 0 );
		scrolllastvalue = host_mapscrollbar->GetScrollBarValue( );
		//CWar3Frame::FrameEventCallback( host_mapscrollbar->FrameAddr, 0, CFrameEventsInternal::FRAME_EVENT_PRESSED ); 
		InitMapHostList( );
	}
}


void ResetFilterMapList( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	GenreFilterStringLower = "";
	NameFilterStringLower = "";
	UpdateFilteredMapList( );
}

bool OptionShow = false;
int dbgval = 1074135140;

void ShowAutologinShowButton( );


void DestroyHostMenu( );


void ChangeMenuCallback( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	CONSOLE_Print( "Change menu X" );


	if ( mainframe )
	{
		mainframe->DestroyThisFrame( );
		delete mainframe;
		mainframe = NULL;
		delete exitbutton;
		delete autologin_save;
		delete autologin_clear;
		delete autologin_username;
		delete autologin_password;
		delete autologin_go;
		//delete mainmenumodel;
	}

	DestroyHostMenu( );
}

void DestroyHostMenu( )
{
	if ( hostmainframe )
	{
		hostmainframe->DestroyThisFrame( );
		delete hostmainframe;
		hostmainframe = NULL;

		delete host_cancelbtn;
		delete host_mapscrollbar;
		delete host_fastsearch;
		delete host_gamename;
		delete host_mapgenrefiltermenu;
		delete host_modelist;
		delete host_playerlist;
		delete host_creategamebtn;
		delete HostMenuMapStats;
		delete HostMenuMapPublic;


		host_cancelbtn = NULL;
		host_mapscrollbar = NULL;
		host_fastsearch = NULL;
		host_gamename = NULL;
		host_mapgenrefiltermenu = NULL;
		host_modelist = NULL;
		host_playerlist = NULL;
		host_creategamebtn = NULL;
		HostMenuMapStats = NULL;
		HostMenuMapPublic = NULL;


		for ( int i = 0; i < 12; i++ )
		{
			MapListInfos[ i ]->DestroyThisFrame( );
			delete MapListInfos[ i ];
			MapListInfos[ i ] = 0;
			delete MapListNames[ i ];
			MapListNames[ i ] = 0;
			delete MapListHostCmd[ i ];
			MapListHostCmd[ i ] = 0;
		}

	}
}

int WarcisHostMenuCallback( CWar3Frame*frame, int FrameAddr, uint32_t ClickType )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	if ( !frame )
		return 0;

	if ( SkipAllEvents )
		return 1;

	if ( frame == host_cancelbtn )
	{
		int CancelAnonSearch = GetFrameItemAddress( "AnonSearchCancelButton", 0 );
		if ( CancelAnonSearch )
		{
			Wc3ControlClickButton_ptr( CancelAnonSearch, 1 );
		}
		DestroyHostMenu( );
		CWar3Frame::Wc3PlaySound( "H05Jaina01" );
	}
	else if ( frame == host_creategamebtn )
	{
		CWar3Frame::Wc3PlaySound( "CantPlaceUndead" );
		if ( wc3classgproxy )
		{
			CWC3 * gproxy_wc3 = ( CWC3 * )wc3classgproxy;
			gproxy_wc3->SendAHPacket( ANTIHACK_VERSION, 0x1020, SelectedMapHostType, 0, 0, 0, SelectedMapCode, SelectedMapPlayers, SelectedMapMode, SelectedGameName );
			CONSOLE_Print( "Host map:" + SelectedMapCode + ". Mode:" + SelectedMapMode + ". Players:" + SelectedMapPlayers + ". Game name:" + SelectedGameName + ".Type:" + to_string( SelectedMapHostType ) );
		}
		DestroyHostMenu( );
		//WarcisHostMenuCallback( host_cancelbtn, host_cancelbtn->FrameAddr,0 );
		//host_creategamebtn->SetFrameRelativePosition( CFramePosition::BOTTOM_CENTER, host_creategamebtn->FrameAddr, CFramePosition::TOP_CENTER, 0.0f, -0.025 );
	}
	else if ( frame->GetFrameCustomValue( 1 ) == 1 ) // Map name
	{
		//CWar3Frame::Wc3PlaySound( "CantPlaceUndead" );
		if ( ClickType == CFrameEvents::FRAME_EDITBOX_TEXT_CHANGED && hostmainframe )
		{
			SkipAllEvents = true;
			InitMapHostList( frame->GetFrameCustomValue( 2 ) + scrolllastvalue );
			SkipAllEvents = false;
		}
		else if ( ClickType == CFrameEvents::FRAME_MOUSE_UP
			|| ClickType == CFrameEvents::FRAME_MOUSE_DOWN )
		{
			MapHostStruct curhoststr = MapHostStruct( );
			curhoststr.availabled = false;
			int maphostid = frame->GetFrameCustomValue( 2 ) + scrolllastvalue - 1;
			if ( maphostid >= 0 && maphostid < FilteredMapHostList.size( ) )
			{
				curhoststr = FilteredMapHostList[ maphostid ];
				curhoststr.availabled = true;
			}
			else if ( FilteredMapHostList.size( ) )
			{
				curhoststr = FilteredMapHostList[ 0 ];
				curhoststr.availabled = true;
			}

			if ( curhoststr.availabled )
			{
				/*	char mapinfomsg[ 256 ];
					sprintf_s( mapinfomsg, "Map name:%s\nMap code:%s\nMap genre:%s\nCrc32: %u", curhoststr.MapName.c_str()
						, curhoststr.MapHost.c_str( ), curhoststr.MapCategory.c_str( ), curhoststr.crc32 );
					MessageBox( 0, mapinfomsg, mapinfomsg, 0 );
	*/
				MapInfosStruct curmapinfo = MapInfosStruct( );
				for ( auto s : ListMapInfos )
				{
					if ( s.mapcrc32 == curhoststr.crc32 )
					{
						curmapinfo = s;
					}
				}




				CWar3Frame minimap = CWar3Frame( );
				minimap.Load( "WarcisFastHostMenuPreviewImageBackdrop" );
				minimap.SetFrameType( CFrameType::FRAMETYPE_BACKDROP );
				minimap.SetTexture(
					curmapinfo.previewfilename.length( ) ?
					curmapinfo.previewfilename.c_str( )
					: "UI\\Widgets\\Glues\\Minimap-Unknown.blp"
				);


				minimap.Load( "WarcisFastHostMenuPreviewImageMinimapBackdrop" );
				minimap.SetFrameType( CFrameType::FRAMETYPE_BACKDROP );
				minimap.SetTexture(
					curmapinfo.minimapfilename.length( ) ?
					curmapinfo.minimapfilename.c_str( )
					: "UI\\Widgets\\Glues\\Minimap-Unknown.blp"
				);

				minimap.Load( "WarcisFastHostMenuPreviewLabel" );
				minimap.SetFrameType( CFrameType::FRAMETYPE_ITEM );
				minimap.SetText( curhoststr.MapName.c_str( ) );

				/*	char dumpmsg[ 512 ];
					sprintf_s( dumpmsg, "File preview:%s\nFile minimap:%s\nMap path:%s", ListMapInfos[ RandId ].previewfilename.length( ) ? ListMapInfos[ RandId ].previewfilename.c_str( ) : "no",
						ListMapInfos[ RandId ].minimapfilename.length( ) ? ListMapInfos[ RandId ].minimapfilename.c_str( ) : "no", ListMapInfos[ RandId ].mapfilename.c_str( ) );

					MessageBox( 0, dumpmsg, ListMapInfos[ RandId ].minimapfilename.c_str( ), 0 );*/



				host_playerlist->ClearMenuItems( );
				host_playerlist->AddItem( "Default" );
				for ( auto s : curhoststr.MapPlayers )
					host_playerlist->AddItem( s.c_str( ) );
				host_playerlist->SetText( "Default" );


				host_modelist->ClearMenuItems( );
				host_modelist->AddItem( "Default" );
				for ( auto s : curhoststr.MapModes )
					host_modelist->AddItem( s.c_str( ) );
				host_modelist->SetText( "Default" );

				SelectedMapCode = curhoststr.MapHost;
				//	MessageBox( 0, SelectedMapCode.c_str( ), "code:", 0 );
			}

		}
		else if ( ClickType == CFrameEvents::FRAME_MOUSE_DOUBLECLICK )
		{
			CWar3Frame::Wc3PlaySound( "CantPlaceUndead" );
			if ( wc3classgproxy )
			{
				CWC3 * gproxy_wc3 = ( CWC3 * )wc3classgproxy;
				gproxy_wc3->SendAHPacket( ANTIHACK_VERSION, 0x1020, SelectedMapHostType, 0, 0, 0, SelectedMapCode, SelectedMapPlayers, SelectedMapMode, SelectedGameName );
				CONSOLE_Print( "Host map:" + SelectedMapCode + ". Mode:" + SelectedMapMode + ". Players:" + SelectedMapPlayers + ". Game name:" + SelectedGameName + ".Type:" + to_string( SelectedMapHostType ) );
			}
		}
		SkipAllEvents = true;
		frame->SetCursor( false );
		//frame->SetFocus( false );
		SkipAllEvents = false;

	}
	else if ( frame->GetFrameCustomValue( 1 ) == 2 ) // Map code
	{
		//CWar3Frame::Wc3PlaySound( "CantPlaceUndead" );
		if ( ClickType == CFrameEvents::FRAME_EDITBOX_TEXT_CHANGED && hostmainframe )
		{
			SkipAllEvents = true;
			InitMapHostList( scrolllastvalue );
			SkipAllEvents = false;
		}
		SkipAllEvents = true;
		frame->SetCursor( false );
		//frame->SetFocus( false );
		SkipAllEvents = false;
	}
	else if ( frame == host_mapscrollbar && hostmainframe )
	{
		SkipAllEvents = true;
		InitMapHostList( frame->GetScrollBarValue( ) );
		SkipAllEvents = false;
	}
	else if ( frame == host_fastsearch && hostmainframe )
	{
		NameFilterStringLower = host_fastsearch->GetText( );
		UpdateFilteredMapList( );
		//InitMapHostList( );
	}
	else if ( frame == host_gamename )
	{
		SelectedGameName = host_gamename->GetText( );
	}
	else if ( frame == host_mapgenrefiltermenu && hostmainframe )
	{
		GenreFilterStringLower = host_mapgenrefiltermenu->GetText( );
		if ( ToLower( GenreFilterStringLower ) == "all" )
			GenreFilterStringLower = "";

		UpdateFilteredMapList( );
		//	InitMapHostList( );
	}
	else if ( frame == host_playerlist )
	{
		SelectedMapPlayers = frame->GetText( );
		if ( SelectedMapPlayers == "Default" )
			SelectedMapPlayers = "";
	}
	else if ( frame == host_modelist )
	{
		SelectedMapMode = frame->GetText( );
		if ( SelectedMapMode == "Default" )
			SelectedMapMode = "";
	}
	else if ( frame == HostMenuMapStats )
	{
		/*	HostMenuMapPublic->ClearMenuItems( );
			HostMenuMapPublic->AddItem( "Public" );
			HostMenuMapPublic->AddItem( "Private" );
			HostMenuMapPublic->SetText( "Public" );
			HostMenuMapStats->ClearMenuItems( );
			HostMenuMapStats->AddItem( "Stats" );
			HostMenuMapStats->AddItem( "No stats" );
			HostMenuMapStats->SetText( "Stats" );*/

		if ( HostMenuMapPublic->GetText( ) == string( "Private" ) )
		{
			SelectedMapHostType = 2;
			/*if ( HostMenuMapStats->GetText( ) == string( "Stats") )
			{
				HostMenuMapStats->SetText( "No stats" );
			}*/
		}
		else
		{
			if ( HostMenuMapStats->GetText( ) == string( "Stats" ) )
				SelectedMapHostType = 0;
			else
				SelectedMapHostType = 1;
		}

		UpdateFilteredMapList( );
		//InitMapHostList( );
	}
	else if ( frame == HostMenuMapPublic )
	{
		/*HostMenuMapPublic->ClearMenuItems( );
		HostMenuMapPublic->AddItem( "Public" );
		HostMenuMapPublic->AddItem( "Private" );
		HostMenuMapPublic->SetText( "Public" );
		HostMenuMapStats->ClearMenuItems( );
		HostMenuMapStats->AddItem( "Stats" );
		HostMenuMapStats->AddItem( "No stats" );
		HostMenuMapStats->SetText( "Stats" );*/


		if ( HostMenuMapPublic->GetText( ) == string( "Private" ) )
		{
			/*if ( HostMenuMapStats->GetText( ) == string( "Stats") )
			{
				HostMenuMapStats->SetText( "No stats" );
			}*/

			SelectedMapHostType = 2;
		}
		else
		{
			if ( HostMenuMapStats->GetText( ) == string( "Stats" ) )
			{
				SelectedMapHostType = 0;
			}
			else
			{
				SelectedMapHostType = 1;
			}

		}

		UpdateFilteredMapList( );
	}

	return 1;
}

void ChangeMapListScrollBarValMouseWheel( bool UP )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	if ( host_mapscrollbar && !IsGame( ) )
	{
		int curval = host_mapscrollbar->GetScrollBarValue( );
		curval += ( UP ? 1 : -1 );
		host_mapscrollbar->SetScrollBarValuesCount( -1, curval );
		scrolllastvalue = host_mapscrollbar->GetScrollBarValue( );
		CWar3Frame::FrameEventCallback( host_mapscrollbar->FrameAddr, 0, CFrameEventsInternal::FRAME_EVENT_PRESSED );
	}
}


void ShowMapHostMenu( int startid )
{

#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	if ( hostmainframe )
	{
		hostmainframe->DestroyThisFrame( );
		delete hostmainframe;
		delete host_cancelbtn;
		delete host_mapscrollbar;
		delete host_fastsearch;
		delete host_gamename;
		delete host_mapgenrefiltermenu;
		delete host_modelist;
		delete host_playerlist;
		delete host_creategamebtn;
		delete HostMenuMapStats;
		delete HostMenuMapPublic;

		hostmainframe = NULL;
		host_cancelbtn = NULL;
		host_mapscrollbar = NULL;
		host_fastsearch = NULL;
		host_gamename = NULL;
		host_mapgenrefiltermenu = NULL;
		host_modelist = NULL;
		host_playerlist = NULL;
		host_creategamebtn = NULL;
		HostMenuMapStats = NULL;
		HostMenuMapPublic = NULL;



		for ( int i = 0; i < 12; i++ )
		{
			MapListInfos[ i ]->DestroyThisFrame( );
			delete MapListInfos[ i ];
			MapListInfos[ i ] = 0;
			delete MapListNames[ i ];
			MapListNames[ i ] = 0;
			delete MapListHostCmd[ i ];
			MapListHostCmd[ i ] = 0;
		}

	}

	hostmainframe = new CWar3Frame( "WarcisFastHostMenu" );
	hostmainframe->SetFrameAbsolutePosition( CFramePosition::BOTTOM_LEFT, 0.0, 0.0 );
	hostmainframe->SetFrameFocused( );

	host_cancelbtn = new CWar3Frame( );
	host_cancelbtn->Load( "WarcisHostMenuExitButton" );
	host_cancelbtn->SetCallbackFunc( WarcisHostMenuCallback );
	host_cancelbtn->RegisterEventCallback( CFrameEvents::FRAME_EVENT_PRESSED );

	host_mapscrollbar = new CWar3Frame( );
	host_mapscrollbar->Load( "WarcisMapsScrollbar" );
	host_mapscrollbar->SetFrameType( CFrameType::FRAMETYPE_SCROLLBAR );
	host_mapscrollbar->SetCallbackFunc( WarcisHostMenuCallback );
	host_mapscrollbar->RegisterEventCallback( CFrameEvents::FRAME_MOUSE_UP );
	host_mapscrollbar->RegisterEventCallback( CFrameEvents::FRAME_MOUSE_DOWN );
	host_mapscrollbar->RegisterEventCallback( CFrameEvents::FRAME_FOCUS_LEAVE );
	host_mapscrollbar->RegisterEventCallback( CFrameEvents::FRAME_EVENT_PRESSED );

	host_fastsearch = new CWar3Frame( );
	host_fastsearch->Load( "HostMenuFastSearch" );
	host_fastsearch->SetFrameType( CFrameType::FRAMETYPE_EDITBOX );
	host_fastsearch->SetCallbackFunc( WarcisHostMenuCallback );
	host_fastsearch->RegisterEventCallback( CFrameEvents::FRAME_EDITBOX_TEXT_CHANGED );

	host_gamename = new CWar3Frame( );
	host_gamename->Load( "HostMenuGameName" );
	host_gamename->SetFrameType( CFrameType::FRAMETYPE_EDITBOX );
	host_gamename->SetCallbackFunc( WarcisHostMenuCallback );
	host_gamename->RegisterEventCallback( CFrameEvents::FRAME_EDITBOX_TEXT_CHANGED );
	SelectedGameName = "by " + LastUsername;
	host_gamename->SetText( SelectedGameName.c_str( ) );


	host_mapgenrefiltermenu = new CWar3Frame( );
	host_mapgenrefiltermenu->Load( "HostMenuMapFilter" );
	host_mapgenrefiltermenu->SetFrameType( CFrameType::FRAMETYPE_POPUPMENU );
	host_mapgenrefiltermenu->SetCallbackFunc( WarcisHostMenuCallback );
	//host_mapgenrefiltermenu->RegisterEventCallback( CFrameEvents::FRAME_POPUPMENU_ITEM_CHANGE_START );
	host_mapgenrefiltermenu->RegisterEventCallback( CFrameEvents::FRAME_POPUPMENU_ITEM_CHANGED );
	host_mapgenrefiltermenu->AddItem( "All" );
	host_mapgenrefiltermenu->SetText( "All" );

	//host_mapgenrefiltermenu->RegisterEventCallback( CFrameEvents::FRAME_EVENT_PRESSED );


	CWar3Frame minimap = CWar3Frame( );
	minimap.Load( "WarcisFastHostMenuPreviewImageBackdrop" );
	minimap.SetFrameType( CFrameType::FRAMETYPE_BACKDROP );
	minimap.SetTexture( "UI\\Widgets\\Glues\\Minimap-Unknown.blp" );
	minimap.Load( "WarcisFastHostMenuPreviewImageMinimapBackdrop" );
	minimap.SetFrameType( CFrameType::FRAMETYPE_BACKDROP );
	minimap.SetTexture( "UI\\Widgets\\Glues\\Minimap-Unknown.blp" );


	host_creategamebtn = new CWar3Frame( );
	host_creategamebtn->Load( "WarcisHostMenuCreateGameButton" );
	host_creategamebtn->SetCallbackFunc( WarcisHostMenuCallback );
	host_creategamebtn->RegisterEventCallback( CFrameEvents::FRAME_EVENT_PRESSED );

	CWar3Frame TmpFrameItem = CWar3Frame( );


	MapListInfos[ 0 ] = new CWar3Frame( "WarcisHostMenuMapItem", 0, false, GetFrameItemAddr( "WarcisMapsScrollbarBackdrop", 0 ) );
	MapListInfos[ 0 ]->SetFrameRelativePosition( CFramePosition::TOP_LEFT, GetFrameItemAddr( "WarcisMapsScrollbarBackdrop", 0 ), CFramePosition::TOP_LEFT, 0.005f, 0.026f );


	MapListNames[ 0 ] = new CWar3Frame( );
	MapListHostCmd[ 0 ] = new CWar3Frame( );

	SkipAllEvents = true;


	TmpFrameItem.Load( "WarcisHostMenuMapItem_MapId", 0 );
	TmpFrameItem.SetFrameType( CFrameType::FRAMETYPE_EDITBOX );
	TmpFrameItem.SetText( "#" );
	TmpFrameItem.Enable( false );

	TmpFrameItem.Load( "WarcisHostMenuMapItem_MapName", 0 );
	TmpFrameItem.SetFrameType( CFrameType::FRAMETYPE_EDITBOX );
	TmpFrameItem.SetText( "Map name" );
	TmpFrameItem.Enable( false );

	TmpFrameItem.Load( "WarcisHostMenuMapItem_MapCode", 0 );
	TmpFrameItem.SetFrameType( CFrameType::FRAMETYPE_EDITBOX );
	TmpFrameItem.SetText( "/host code" );
	TmpFrameItem.Enable( false );


	TmpFrameItem.Load( "WarcisHostMenuMapItemBackground", 0 );
	TmpFrameItem.SetFrameType( CFrameType::FRAMETYPE_HIGHLIGHT );
	TmpFrameItem.Show( false );


	for ( int i = 1; i < 12; i++ )
	{
		MapListInfos[ i ] = new CWar3Frame( "WarcisHostMenuMapItem", i, false, GetFrameItemAddr( "WarcisMapsScrollbarBackdrop", 0 ) );
		if ( i == 1 )
			MapListInfos[ i ]->SetFrameRelativePosition( CFramePosition::TOP_LEFT, MapListInfos[ i - 1 ]->FrameAddr, CFramePosition::BOTTOM_LEFT, 0.0f, -0.003f );
		else
			MapListInfos[ i ]->SetFrameRelativePosition( CFramePosition::TOP_LEFT, MapListInfos[ i - 1 ]->FrameAddr, CFramePosition::BOTTOM_LEFT, 0.0f, 0.0f );


		TmpFrameItem.Load( "WarcisHostMenuMapItem_MapId", i );
		TmpFrameItem.SetFrameType( CFrameType::FRAMETYPE_EDITBOX );
		TmpFrameItem.SetText( to_string( i ).c_str( ) );
		TmpFrameItem.Enable( false );

		MapListNames[ i ] = new CWar3Frame( );
		MapListNames[ i ]->Load( "WarcisHostMenuMapItem_MapName", i );
		MapListNames[ i ]->SetFrameType( CFrameType::FRAMETYPE_EDITBOX );
		MapListNames[ i ]->RegisterEventCallback( CFrameEvents::FRAME_EDITBOX_TEXT_CHANGED );
		MapListNames[ i ]->RegisterEventCallback( CFrameEvents::FRAME_MOUSE_UP );
		MapListNames[ i ]->RegisterEventCallback( CFrameEvents::FRAME_FOCUS_ENTER );
		MapListNames[ i ]->RegisterEventCallback( CFrameEvents::FRAME_FOCUS_LEAVE );
		MapListNames[ i ]->RegisterEventCallback( CFrameEvents::FRAME_EVENT_PRESSED );
		MapListNames[ i ]->RegisterEventCallback( CFrameEvents::FRAME_MOUSE_DOWN );

		MapListNames[ i ]->SetCallbackFunc( WarcisHostMenuCallback );
		MapListNames[ i ]->SetFrameCustomValue( 1, 1 );
		MapListNames[ i ]->SetFrameCustomValue( i, 2 );

		MapListNames[ i ]->SetText( "1" );
		//	MapListNames[ i ]->SetFlag( 0 );
		MapListNames[ i ]->Click( );
		MapListNames[ i ]->Click( );
		//	MapListNames[ i ]->SetFocus( true );
		//	MapListNames[ i ]->SetCursor( false );
		MapListNames[ i ]->SetText( "" );
		MapListNames[ i ]->SetFlag( 1 );
		//MapListNames[ i ]->SetSkipAnotherCallback( true );

		MapListHostCmd[ i ] = new CWar3Frame( );
		MapListHostCmd[ i ]->Load( "WarcisHostMenuMapItem_MapCode", i );
		MapListHostCmd[ i ]->SetFrameType( CFrameType::FRAMETYPE_EDITBOX );
		MapListHostCmd[ i ]->RegisterEventCallback( CFrameEvents::FRAME_EDITBOX_TEXT_CHANGED );
		MapListHostCmd[ i ]->RegisterEventCallback( CFrameEvents::FRAME_MOUSE_UP );
		MapListHostCmd[ i ]->RegisterEventCallback( CFrameEvents::FRAME_FOCUS_ENTER );
		MapListHostCmd[ i ]->RegisterEventCallback( CFrameEvents::FRAME_FOCUS_LEAVE );
		MapListHostCmd[ i ]->RegisterEventCallback( CFrameEvents::FRAME_EVENT_PRESSED );
		MapListHostCmd[ i ]->RegisterEventCallback( CFrameEvents::FRAME_MOUSE_DOWN );

		MapListHostCmd[ i ]->SetCallbackFunc( WarcisHostMenuCallback );
		MapListHostCmd[ i ]->SetFrameCustomValue( 2, 1 );
		MapListHostCmd[ i ]->SetFrameCustomValue( i, 2 );
		MapListHostCmd[ i ]->SetText( "1" );
		//	MapListHostCmd[ i ]->SetFlag( 0 );
		MapListHostCmd[ i ]->Click( );
		MapListHostCmd[ i ]->Click( );
		//	MapListHostCmd[ i ]->SetFocus( true );
			//MapListHostCmd[ i ]->SetCursor( false );
		MapListHostCmd[ i ]->SetText( "" );
		MapListHostCmd[ i ]->SetFlag( 1 );



		//	MapListHostCmd[ i ]->SetSkipAnotherCallback( true );
	}

	host_playerlist = new CWar3Frame( );
	host_playerlist->Load( "HostMenuMapPlayers" );
	host_playerlist->SetFrameType( CFrameType::FRAMETYPE_POPUPMENU );

	host_playerlist->SetText( "Select players:" );
	host_playerlist->AddItem( "Default" );
	host_playerlist->SetCallbackFunc( WarcisHostMenuCallback );
	host_playerlist->RegisterEventCallback( CFrameEvents::FRAME_POPUPMENU_ITEM_CHANGED );

	host_modelist = new CWar3Frame( );
	host_modelist->Load( "HostMenuMapMode" );
	host_modelist->SetFrameType( CFrameType::FRAMETYPE_POPUPMENU );

	host_modelist->SetText( "Select mode:" );
	host_modelist->AddItem( "Default" );
	host_modelist->SetCallbackFunc( WarcisHostMenuCallback );
	host_modelist->RegisterEventCallback( CFrameEvents::FRAME_POPUPMENU_ITEM_CHANGED );



	SelectedMapCode = "dota88";
	SelectedMapPlayers = "";
	SelectedMapMode = "";
	SelectedGameName = "by " + LastUsername;


	HostMenuMapStats = new CWar3Frame( );
	HostMenuMapStats->Load( "HostMenuMapStats" );
	HostMenuMapStats->SetFrameType( CFrameType::FRAMETYPE_POPUPMENU );
	HostMenuMapStats->AddItem( "No stats" );
	HostMenuMapStats->AddItem( "Stats" );
	HostMenuMapStats->SetText( "No stats" );
	HostMenuMapStats->RegisterEventCallback( CFrameEvents::FRAME_POPUPMENU_ITEM_CHANGED );
	HostMenuMapStats->SetCallbackFunc( WarcisHostMenuCallback );

	HostMenuMapPublic = new CWar3Frame( );
	HostMenuMapPublic->Load( "HostMenuMapPublic" );
	HostMenuMapPublic->SetFrameType( CFrameType::FRAMETYPE_POPUPMENU );
	HostMenuMapPublic->AddItem( "Public" );
	HostMenuMapPublic->AddItem( "Private" );
	HostMenuMapPublic->SetText( "Public" );
	HostMenuMapPublic->RegisterEventCallback( CFrameEvents::FRAME_POPUPMENU_ITEM_CHANGED );
	HostMenuMapPublic->SetCallbackFunc( WarcisHostMenuCallback );



	//MessageBox( 0, CWar3Frame::DumpAllFrames( ).c_str( ), "FrameS:", 0 );
	/*
	hostmainframe->DestroyThisFrame( );
	delete hostmainframe;
	delete host_cancelbtn;
	delete host_creategamebtn;
	hostmainframe = NULL;
	host_cancelbtn = NULL;
	host_creategamebtn = NULL;
	for ( int i = 0; i < 12; i++ )
	{
		MapListInfos[ i ]->DestroyThisFrame( );
		delete MapListInfos[ i ];
		MapListInfos[ i ] = 0;
	}
	*/

	SelectedMapHostType = 1;

	SkipAllEvents = true;
	UpdateFilteredMapList( );
	SkipAllEvents = false;


	//InitMapHostList( startid );

}


void ShowAutologinShowButton( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	if ( GetFrameItemAddress( "BattleNetButton", 0 ) )
	{

		if ( autologinshowbtn )
		{
			autologinshowbtn->DestroyThisFrame( );
			delete autologinshowbtn;
			delete autologinshowbtnitem;
			SuperDebugButton->DestroyThisFrame( );
			delete SuperDebugButton;
			delete SuperDebugButtonItem;
			autologinshowbtn = NULL;
			SuperDebugButton = NULL;
			autologinshowbtnitem = NULL;
			SuperDebugButtonItem = NULL;
		}

		autologinshowbtn = new CWar3Frame( "AutologinButton" );
		autologinshowbtn->SetFrameRelativePosition( CFramePosition::LEFT, GetFrameItemAddress( "BattleNetButton", 0 ), CFramePosition::BOTTOM_LEFT, -0.025f, 0.015f );

		autologinshowbtnitem = new CWar3Frame( );
		autologinshowbtnitem->Load( "AutologinItemButton" );
		autologinshowbtnitem->SetFrameType( CFrameType::FRAMETYPE_BUTTON );
		autologinshowbtnitem->SetText( "Autologin to Warcis" );
		autologinshowbtnitem->SetCallbackFunc( AutoLoginMenuCallback );
		autologinshowbtnitem->RegisterEventCallback( CFrameEvents::FRAME_EVENT_PRESSED );

		//	MessageBox( 0, CWar3Frame::DumpAllFrames( ).c_str( ), "FrameS:", 0 );

			/* DEBUG */
		SuperDebugButton = new CWar3Frame( "WarcisSpeakItem", 14 );
		SuperDebugButton->Show( true );
		SuperDebugButton->SetFrameType( CFrameType::FRAMETYPE_FRAME );
		SuperDebugButton->SetFrameAbsolutePosition( CFramePosition::BOTTOM_LEFT, 0.03f, 0.24 );

		SuperDebugButton = new CWar3Frame( "WarcisSpeakItem", 15 );
		SuperDebugButton->Show( true );
		SuperDebugButton->SetFrameType( CFrameType::FRAMETYPE_FRAME );
		SuperDebugButton->SetFrameAbsolutePosition( CFramePosition::BOTTOM_LEFT, 0.03f, 0.45 );


		SuperDebugButtonItem = new CWar3Frame( );
		SuperDebugButtonItem->Load( "WarcisSpeakFrameImg", 15 );
		SuperDebugButtonItem->SetFrameType( CFrameType::FRAMETYPE_BUTTON );
		SuperDebugButtonItem->SetText( "|c00FF0000This is the button that you need!|r" );
		SuperDebugButtonItem->SetCallbackFunc( AutoLoginMenuCallback );
		SuperDebugButtonItem->RegisterEventCallback( CFrameEvents::FRAME_EVENT_PRESSED );


		//	auto sSuperDebugButtonItem = new CWar3Frame( );
		//	sSuperDebugButtonItem->Load(
		//		"WarcisSpeakLoginText" );
		//	sSuperDebugButtonItem->SetFrameType( CFrameType::FRAMETYPE_ITEM );
		//		MessageBox( 0, CWar3Frame::DumpAllFrames( ).c_str( ), "All frames:", 0 );
			//	SuperDebugButtonItem->SetFrameScale( CFrameBackdropType::ControlFrame, 0.01, 0.01 );
				/*SuperDebugButtonItem->SetFrameScale( CFrameBackdropType::ControlDisabledBackdrop, 0.01, 0.01 );*/
				//SuperDebugButtonItem->FillToParentFrame( CFrameBackdropType::ControlFrame, false );
				//SuperDebugButtonItem->SetFrameScale( CFrameBackdropType::ControlFrame, 0.01, 0.01 );
				//SuperDebugButtonItem->Update( );

	}
}

void ShowAutologinWindow( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	CONSOLE_Print( "AutologinInit:0" );

	if ( !GetFrameItemAddress( "BattleNetButton", 0 ) )
		return;


	if ( mainframe )
	{
		mainframe->DestroyThisFrame( );
		delete mainframe;
		delete exitbutton;
		delete autologin_save;
		delete autologin_clear;
		delete autologin_username;
		delete autologin_password;
		delete autologin_go;
		//delete mainmenumodel;
		mainframe = NULL;
	}

	//CONSOLE_Print( "AutologinInit:1" );
	mainframe = new CWar3Frame( "WarcisAutoLogin" );
	mainframe->SetFrameRelativePosition( CFramePosition::LEFT, GetFrameItemAddress( "BattleNetButton", 0 ), CFramePosition::BOTTOM_LEFT, -0.26f, -0.037f );
	//CONSOLE_Print( "AutologinInit:2" );
	exitbutton = new CWar3Frame( );
	exitbutton->Load( "AutologinCloseButton" );
	exitbutton->SetCallbackFunc( AutoLoginMenuCallback );
	exitbutton->RegisterEventCallback( CFrameEvents::FRAME_EVENT_PRESSED );
	//CONSOLE_Print( "AutologinInit:3" );

	autologin_save = new CWar3Frame( );
	autologin_save->Load( "SaveThisLogin" );
	autologin_save->SetCallbackFunc( AutoLoginMenuCallback );
	autologin_save->SetChecked( gInfo.NeedSaveAutologin );
	autologin_save->RegisterEventCallback( CFrameEvents::FRAME_CHECKBOX_CHECKED );
	autologin_save->RegisterEventCallback( CFrameEvents::FRAME_CHECKBOX_UNCHECKED );
	//CONSOLE_Print( "AutologinInit:4" );
	autologin_clear = new CWar3Frame( );
	autologin_clear->Load( "AutoLogin_ClearButton" );
	autologin_clear->SetCallbackFunc( AutoLoginMenuCallback );
	autologin_clear->RegisterEventCallback( CFrameEvents::FRAME_EVENT_PRESSED );
	autologin_clear->SetFrameType( CFrameType::FRAMETYPE_TEXTBUTTON );
	autologin_clear->SetText( "Clear current login" );

	//CONSOLE_Print( "AutologinInit:5" );
	autologin_username = new CWar3Frame( );

	autologin_username->Load( "AutoLogin_Username" );
	autologin_username->SetFrameType( CFrameType::FRAMETYPE_EDITBOX );
	autologin_username->SetText( gInfo.Username );
	autologin_username->SetMaxLen( 16 );
	autologin_username->SetCallbackFunc( AutoLoginMenuCallback );
	autologin_username->RegisterEventCallback( CFrameEvents::FRAME_EDITBOX_TEXT_CHANGED );

	//	CONSOLE_Print( "AutologinInit:6" );
	autologin_password = new CWar3Frame( );

	autologin_password->Load( "AutoLogin_Password" );
	autologin_password->SetFrameType( CFrameType::FRAMETYPE_EDITBOX );
	autologin_password->SetText( gInfo.Password );
	autologin_password->SetMaxLen( 45 );
	autologin_password->SetCallbackFunc( AutoLoginMenuCallback );
	autologin_password->RegisterEventCallback( CFrameEvents::FRAME_EDITBOX_TEXT_CHANGED );


	autologin_go = new CWar3Frame( );
	autologin_go->Load( "AutoLogin_GoButton" );
	autologin_go->SetCallbackFunc( AutoLoginMenuCallback );
	autologin_go->RegisterEventCallback( CFrameEvents::FRAME_EVENT_PRESSED );
	autologin_go->SetFrameType( CFrameType::FRAMETYPE_BUTTON );
	autologin_go->SetText( "Start autologin" );


	//CONSOLE_Print( "AutologinInit:9" );

	//mainmenumodel = new CWar3Frame( );
	//mainmenumodel->Load( "WarCraftIIILogo" );
	//mainmenumodel->SetFrameType( CFrameType::FRAMETYPE_SPRITE );
	//mainmenumodel->SetCallbackFunc( AutoLoginMenuCallback );
	//mainmenumodel->RegisterEventCallback( CFrameEvents::FRAME_MOUSE_ENTER );
	//mainmenumodel->RegisterEventCallback( CFrameEvents::FRAME_MOUSE_LEAVE );

	//mainmenumodel = new CWar3Frame( /*GetCursorAddr( 1 )*/ );
	//mainmenumodel->Load( "Cursor" );
	//mainmenumodel->SetFrameRelativePosition( CFramePosition::LEFT, GetFrameItemAddress( "Cursor", 0 ), CFramePosition::RIGHT, 0.0f, 0.0f );

	//mainmenumodel = new CWar3Frame( );
	/*mainmenumodel->Load( "XXXX" );
	mainmenumodel->SetFrameType( CFrameType::FRAMETYPE_SPRITE );
	mainmenumodel->SetModel( "UI\\Glues\\Loading\\LoadBar\\LoadBar.mdl", -1 );
	mainmenumodel->SetCallbackFunc( AutoLoginMenuCallback );
	mainmenumodel->RegisterEventCallback( CFrameEvents::FRAME_SPRITE_ANIM_UPDATE );
	mainmenumodel->StartAnimate( );*/

	//CONSOLE_Print( "AutologinInit:10" );

	//char mainframeaddr[ 100 ];
	//sprintf_s( mainframeaddr, "Mainframe:%X", mainframe->FrameAddr );
	//CONSOLE_Print( mainframeaddr );

	//if ( !exitbutton )
	//{
	//	exitbutton = new CWar3Frame( );
	//	if ( exitbutton->Load( "WarcisMenu" ) )
	//	{
	//		exitbutton->SetFrameType( CFrameType::FRAMETYPE_POPUPMENU );
	//		/*exitbutton->SetText( "Hello World" );*/
	//		exitbutton->AddItem( "\"Test\"" );
	//		/*exitbutton->AddItem( "Test2" );*/
	//	}

	//}

	//mainframe->Show( true );
	//mainframe->Enable( false );

	/*
		if ( !exitbutton )
		{
			exitbutton = new CWar3Frame( );
			if ( exitbutton->Load( "xExitButton" ) )
			{
				exitbutton->SetCallbackFunc( MyFrameCallback );
				exitbutton->RegisterEventCallback( CFrameEvents::FRAME_EVENT_PRESSED );
			}
			else delete exitbutton;
		}*/
		//	new CWar3Frame( "GlyphButton", true );

}
//
//bool First = true;
//
//void ShowMainFrame( )
//{
//	if ( !First )
//	{
//		ShowAutologinWindow( );
//	}
//
//	First = false;
//}
//

bool InitializeMainFrames = false;

int NeedShowAutologinButtonShow = -1;

// Вход в главное меню
int __fastcall InitWc3MainMenu_my( void *a1, int unused, int a2, int a3 )
{


#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	TwoLineProtection = TRUE;
	PlayerStatsList.clear( );
	current_menu = Wc3Menu::MAIN_MENU;
	int retvalue = InitWc3MainMenu_ptr( a1, unused, a2, a3 );


	if ( NeedAutoJoin &&  gInfo.Username[ 0 ] != '\0' &&  gInfo.Password[ 0 ] != '\0' )
		PressBnetButtonTimed = 2;
	InBattleNet = FALSE;
	BattleNetButtonPressed = false;
	//CONSOLE_Print( "BattleNetButtonPressed: NO" );
	CustomGameButtonAddr = 0;
	CustomGameExitButtonAddr = 0;
	CustomGameNameButtonAddr = 0;
	CustomLoadGameButtonAddr = 0;
	CustomGameNameEditBox = 0;
	LobbyLeaveButtonAddr = 0;
	StandardGameButtonAddr = 0;
	QuickStandardGameButtonAddr = 0;
	StandardTeamGameButtonAddr = 0;
	BattleNetButtonAddr = GetFrameItemAddress( "BattleNetButton", 0 );
	ConnectButtonAddr = GetFrameItemAddress( "ConnectButton", 0 );
	CONSOLE_Print( "InitWc3MainMenu_my" );
	char printinfo[ 256 ];
	sprintf_s( printinfo, "BnetBtn:%X ConnectBtn:%X MainMenu:%X", BattleNetButtonAddr, ConnectButtonAddr, retvalue );
	CONSOLE_Print( printinfo );
	NeedShowAutologinButtonShow = 4;


	CONSOLE_Print( "Fps:" + to_string( gInfo.MaxFps ) );
	SetThreadPriority( GetCurrentThread( ), THREAD_PRIORITY_ABOVE_NORMAL );

	//ShowMainFrame( );
	return retvalue;
}

CWar3Frame * BattleNetUsernameEditbox = NULL;
CWar3Frame * BattleNePasswordEditbox = NULL;
CWar3Frame * BattleNeNewPasswordEditbox1 = NULL;
CWar3Frame * BattleNeNewPasswordEditbox2 = NULL;

CWar3Frame * BattleNeNewPasswordEditbox3 = NULL;
CWar3Frame * BattleNeNewPasswordEditbox4 = NULL;


bool SkipBnetActions = false;
int BattleNetLoginMenuCallback( CWar3Frame*frame, int FrameAddr, uint32_t ClickType )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	if ( !frame )
		return 0;
	if ( SkipBnetActions )
		return 1;
	SkipBnetActions = true;
	if ( frame == BattleNetUsernameEditbox )
	{
		if ( frame->GetText( ) && frame->GetText( )[ 0 ] != '\0' )
		{
			LastUsername = frame->GetText( );
			g_crashRpt->AddUserInfoToReport( L"Username", ConvertFromUtf8ToUtf16( LastUsername ).c_str( ) );
			SkipBnetActions = false;
			return 1;
		}
	}

	if ( frame == BattleNeNewPasswordEditbox1 || frame == BattleNeNewPasswordEditbox2
		|| frame == BattleNeNewPasswordEditbox3 || frame == BattleNeNewPasswordEditbox4 )
	{
		if ( frame->GetTextMaxLength( ) < 45 )
			frame->SetMaxLen( 45 );
		if ( frame->GetText( ) && frame->GetText( )[ 0 ] != '\0' )
			LastPassword = frame->GetText( );
		//CONSOLE_Print( "SetPwd2:" + LastPassword );
		SkipBnetActions = false;
		return 1;
	}

	if ( frame == BattleNePasswordEditbox )
	{
		if ( frame->GetText( ) && frame->GetText( )[ 0 ] != '\0' )
		{
			LastPassword = frame->GetText( );
			//CONSOLE_Print( "SetPwd3:" + LastPassword );
			if ( frame->GetTextMaxLength( ) < 45 )
				frame->SetMaxLen( 45 );
			SkipBnetActions = false;
			return 1;
		}
	}



	if ( frame == OKButton )
	{
		if ( wc3classgproxy )
		{
			CWC3 * gproxy_wc3 = ( CWC3 * )wc3classgproxy;
			char salt[ BCRYPT_HASHSIZE ];
			char hash[ BCRYPT_HASHSIZE ];
			bcrypt_gensalt( 10, salt );
			bcrypt_hashpw( LastPassword.c_str( ), salt, hash );
			//CONSOLE_Print( "Convert and send password:" + LastPassword );
			//	MessageBox( 0, LastPassword.c_str( ), "PasswordCreate:", 0 );
			gproxy_wc3->SendAHPacket( ANTIHACK_VERSION, 0xABCD, 0xA, 0xB, 0xC, 0xD, LastPassword, hash );
			gproxy_wc3->SendAHPacket( ANTIHACK_VERSION, 0xBCDE, gInfo.NickNameColor, gInfo.ChatNickNameColor, gInfo.ChatTextColor );

		}
		else
		{
			CONSOLE_Print( "Error,wc3classgproxy is null " );
		}
		SkipBnetActions = false;
		return 1;
	}


	if ( frame == LogonButton )
	{
		LastUsername = BattleNetUsernameEditbox->GetText( );
		g_crashRpt->AddUserInfoToReport( L"Username", ConvertFromUtf8ToUtf16( LastUsername ).c_str( ) );
		LastPassword = BattleNePasswordEditbox->GetText( );

		if ( wc3classgproxy )
		{
			CWC3 * gproxy_wc3 = ( CWC3 * )wc3classgproxy;
			char salt[ BCRYPT_HASHSIZE ];
			char hash[ BCRYPT_HASHSIZE ];
			bcrypt_gensalt( 10, salt );
			bcrypt_hashpw( LastPassword.c_str( ), salt, hash );
			//CONSOLE_Print( "Convert and send password 2:" + LastPassword);
			//	MessageBox( 0, LastPassword.c_str( ), "PasswordCreate:", 0 );
			gproxy_wc3->SendAHPacket( ANTIHACK_VERSION, 0xABCD, 0xA, 0xB, 0xC, 0xD, LastPassword, hash );
			gproxy_wc3->SendAHPacket( ANTIHACK_VERSION, 0xBCDE, gInfo.NickNameColor, gInfo.ChatNickNameColor, gInfo.ChatTextColor );
		}
		else
		{
			CONSOLE_Print( "Error,wc3classgproxy is null 2" );
		}

		//CONSOLE_Print( "SetPwd1:" + LastPassword );
		SkipBnetActions = false;
		return 1;
	}
	SkipBnetActions = false;
	return 0;
}

// Вход в BattleNet login меню
int __fastcall InitWc3BattleNetMenu_my( void *a1, int unused, int a2, int a3 )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	if ( BattleNetUsernameEditbox )
	{
		delete BattleNetUsernameEditbox;
		BattleNetUsernameEditbox = NULL;
	}

	if ( BattleNePasswordEditbox )
	{
		delete BattleNePasswordEditbox;
		BattleNePasswordEditbox = NULL;
	}

	if ( BattleNeNewPasswordEditbox1 )
	{
		delete BattleNeNewPasswordEditbox1;
		BattleNeNewPasswordEditbox1 = NULL;
	}

	if ( BattleNeNewPasswordEditbox2 )
	{
		delete BattleNeNewPasswordEditbox2;
		BattleNeNewPasswordEditbox2 = NULL;
	}


	if ( BattleNeNewPasswordEditbox3 )
	{
		delete BattleNeNewPasswordEditbox3;
		BattleNeNewPasswordEditbox3 = NULL;
	}

	if ( BattleNeNewPasswordEditbox4 )
	{
		delete BattleNeNewPasswordEditbox4;
		BattleNeNewPasswordEditbox4 = NULL;
	}

	PlayerStatsList.clear( );
	current_menu = Wc3Menu::BNET_MAIN;
	int retvalue = InitWc3BattleNetMenu_ptr( a1, unused, a2, a3 );
	if ( NeedAutoJoin &&  gInfo.Username[ 0 ] != '\0' &&  gInfo.Password[ 0 ] != '\0' )
	{
		PressLogonButtonTimed = 2;
	}
	CustomGameExitButtonAddr = 0;
	CustomGameNameButtonAddr = 0;
	CustomLoadGameButtonAddr = 0;
	CustomGameNameEditBox = 0;
	LobbyLeaveButtonAddr = 0;
	StandardGameButtonAddr = 0;
	QuickStandardGameButtonAddr = 0;
	StandardTeamGameButtonAddr = 0;
	BattleNetButtonAddr = 0;
	ConnectButtonAddr = 0;
	//CustomGameButtonAddr = GetFrameItemAddress( "CustomGameButton", 0 );

	//CONSOLE_Print( "InitWc3BattleNetMenu_my" );


	LogonButton = new CWar3Frame( );
	LogonButton->Load( "LogonButton" );
	LogonButton->SetFrameType( CFrameType::FRAMETYPE_EDITBOX );
	LogonButton->SetCallbackFunc( BattleNetLoginMenuCallback );
	//LogonButton->RegisterEventCallback( CFrameEvents::FRAME_MOUSE_UP );
	LogonButton->RegisterEventCallback( CFrameEvents::FRAME_EVENT_PRESSED );

	OKButton = new CWar3Frame( );
	OKButton->Load( "OKButton" );
	OKButton->SetFrameType( CFrameType::FRAMETYPE_EDITBOX );
	OKButton->SetCallbackFunc( BattleNetLoginMenuCallback );
	//LogonButton->RegisterEventCallback( CFrameEvents::FRAME_MOUSE_UP );
	OKButton->RegisterEventCallback( CFrameEvents::FRAME_EVENT_PRESSED );




	BattleNetUsernameEditbox = new CWar3Frame( );
	BattleNetUsernameEditbox->Load( "AccountName" );
	BattleNetUsernameEditbox->SetFrameType( CFrameType::FRAMETYPE_EDITBOX );
	BattleNetUsernameEditbox->SetCallbackFunc( BattleNetLoginMenuCallback );
	BattleNetUsernameEditbox->RegisterEventCallback( CFrameEvents::FRAME_EDITBOX_TEXT_CHANGED );
	LastUsername = BattleNetUsernameEditbox->GetText( ) ? BattleNetUsernameEditbox->GetText( ) : "";
	g_crashRpt->AddUserInfoToReport( L"Username", ConvertFromUtf8ToUtf16( LastUsername ).c_str( ) );

	BattleNePasswordEditbox = new CWar3Frame( );
	BattleNePasswordEditbox->Load( "Password" );
	BattleNePasswordEditbox->SetFrameType( CFrameType::FRAMETYPE_EDITBOX );
	BattleNePasswordEditbox->SetCallbackFunc( BattleNetLoginMenuCallback );
	BattleNePasswordEditbox->RegisterEventCallback( CFrameEvents::FRAME_EDITBOX_TEXT_CHANGED );

	LastPassword = BattleNePasswordEditbox->GetText( ) ? BattleNePasswordEditbox->GetText( ) : "";


	char pwdframeaddr[ 100 ];
	sprintf_s( pwdframeaddr, "Passw:%X->%p", BattleNePasswordEditbox->FrameAddr, BattleNePasswordEditbox->GetText( ) );
	CONSOLE_Print( pwdframeaddr );

	BattleNeNewPasswordEditbox1 = new CWar3Frame( );
	BattleNeNewPasswordEditbox1->Load( "CPNewPassword" );
	BattleNeNewPasswordEditbox1->SetFrameType( CFrameType::FRAMETYPE_EDITBOX );
	BattleNeNewPasswordEditbox1->SetCallbackFunc( BattleNetLoginMenuCallback );
	BattleNeNewPasswordEditbox1->RegisterEventCallback( CFrameEvents::FRAME_EDITBOX_TEXT_CHANGED );

	BattleNeNewPasswordEditbox2 = new CWar3Frame( );
	BattleNeNewPasswordEditbox2->Load( "CPRepeatNewPassword" );
	BattleNeNewPasswordEditbox2->SetFrameType( CFrameType::FRAMETYPE_EDITBOX );
	BattleNeNewPasswordEditbox2->SetCallbackFunc( BattleNetLoginMenuCallback );
	BattleNeNewPasswordEditbox2->RegisterEventCallback( CFrameEvents::FRAME_EDITBOX_TEXT_CHANGED );

	BattleNeNewPasswordEditbox3 = new CWar3Frame( );
	BattleNeNewPasswordEditbox3->Load( "NAPassword" );
	BattleNeNewPasswordEditbox3->SetFrameType( CFrameType::FRAMETYPE_EDITBOX );
	BattleNeNewPasswordEditbox3->SetCallbackFunc( BattleNetLoginMenuCallback );
	BattleNeNewPasswordEditbox3->RegisterEventCallback( CFrameEvents::FRAME_EDITBOX_TEXT_CHANGED );

	BattleNeNewPasswordEditbox4 = new CWar3Frame( );
	BattleNeNewPasswordEditbox4->Load( "NARepeatPassword" );
	BattleNeNewPasswordEditbox4->SetFrameType( CFrameType::FRAMETYPE_EDITBOX );
	BattleNeNewPasswordEditbox4->SetCallbackFunc( BattleNetLoginMenuCallback );
	BattleNeNewPasswordEditbox4->RegisterEventCallback( CFrameEvents::FRAME_EDITBOX_TEXT_CHANGED );

	BattleNePasswordEditbox->SetMaxLen( 45 );
	BattleNeNewPasswordEditbox1->SetMaxLen( 45 );
	BattleNeNewPasswordEditbox2->SetMaxLen( 45 );
	BattleNeNewPasswordEditbox3->SetMaxLen( 45 );
	BattleNeNewPasswordEditbox4->SetMaxLen( 45 );



	return retvalue;
}




// Вход в Chat меню
int __fastcall BattleNetChatPanel_my( void *a1, int unused, int a2, int a3 )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	FollowStatus = 0;
	PlayerStatsList.clear( );
	current_menu = Wc3Menu::BNET_CHAT;
	LobbyLeaveButtonAddr = 0;
	CustomGameNameEditBox = 0;
	CustomGameExitButtonAddr = 0;
	CustomGameNameButtonAddr = 0;
	CustomLoadGameButtonAddr = 0;
	BattleNetButtonAddr = 0;
	ConnectButtonAddr = 0;

	int retvalue = BattleNetChatPanel_ptr( a1, unused, a2, a3 );
	if ( NeedAutoJoin && gInfo.Username[ 0 ] != '\0' &&  gInfo.Password[ 0 ] != '\0' )
	{
		PressEnterChatButtonTimed = 8;
		NeedAutoJoin = FALSE;
	}
	InBattleNet = TRUE;
	CustomGameButtonAddr = GetFrameItemAddress( "CustomGameButton", 0 );
	StandardGameButtonAddr = GetFrameItemAddress( "StandardGameButton", 0 );
	QuickStandardGameButtonAddr = GetFrameItemAddress( "QuickStandardGameButton", 0 );
	StandardTeamGameButtonAddr = GetFrameItemAddress( "StandardTeamGameButton", 0 );
	//CONSOLE_Print( "BattleNetChatPanel_my" );


	return retvalue;
}


// Вход в список игр
int __fastcall BattleNetCustomJoinPanel_my( void *a1, int unused, int a2, int a3 )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	PlayerStatsList.clear( );
	current_menu = Wc3Menu::GAME_LIST;
	LobbyLeaveButtonAddr = 0;
	CustomGameButtonAddr = 0;
	BattleNetButtonAddr = 0;
	ConnectButtonAddr = 0;

	int retvalue = BattleNetCustomJoinPanel_ptr( a1, unused, a2, a3 );
	CustomGameExitButtonAddr = GetFrameItemAddress( "CancelButton", 0 );
	CustomGameNameEditBox = GetFrameItemAddress( "JoinGameNameEditBox", 0 );
	CustomGameNameButtonAddr = GetFrameItemAddress( "JoinGameButton", 0 );
	CustomLoadGameButtonAddr = GetFrameItemAddress( "LoadGameButton", 0 );


	//CONSOLE_Print( "BattleNetCustomJoinPanel_my" );

	return retvalue;
}



HWND Warcraft3Window = NULL;
WarcraftRealWNDProc WarcraftRealWNDProc_org = NULL;
WarcraftRealWNDProc WarcraftRealWNDProc_ptr = NULL;



struct KeyActionStruct
{
	int VK;
	int btnID;
	int altbtnID;
	BOOL IsSkill;
	BOOL IsShift;
	BOOL IsCtrl;
	BOOL IsAlt;
	BOOL IsRightClick;
};
vector<KeyActionStruct> KeyActionList;

int GetAltBtnID( int btnID )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	switch ( btnID )
	{
	case 2:
		return 0;
	case 5:
		return 3;
	case 8:
		return 6;
	case 11:
		return 9;
	case 4:
		return 1;
	case 7:
		return 4;
	}

	return -1;
}

// Проверяет пустая ли кнопка 
BOOL IsNULLButtonFound( int pButton )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	if ( pButton > 0 && *( int* )( pButton ) > 0 )
	{
		if ( *( int* )( pButton + 0x190 ) != 0 && *( int* )( *( int* )( pButton + 0x190 ) + 4 ) == 0 )
			return TRUE;
	}
	return FALSE;
}

typedef int( __fastcall * c_SimpleButtonClickEvent )( int pButton, int unused, int ClickEventType );
extern c_SimpleButtonClickEvent SimpleButtonClickEvent;
extern c_SimpleButtonClickEvent SimpleButtonClickEvent_ptr;

int pW3XGlobalClass = 0;

// Возвращает адрес pW3XGlobalClass
int GetGlobalClassAddr( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	return *( int* )pW3XGlobalClass;
}

#define flagsOffset 0x138
#define sizeOfCommandButtonObj 0x1c0

// Получает адрес SimpleКнопки по ID (панель способностей)
int __stdcall GetSkillPanelButton( int idx )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__ );
#endif
	int pclass = GetGlobalClassAddr( );
	if ( pclass > 0 )
	{
		int pGamePlayerPanelSkills = *( int* )( pclass + 0x3c8 );
		if ( pGamePlayerPanelSkills > 0 )
		{
			int topLeftCommandButton = *( int* )( pGamePlayerPanelSkills + 0x154 );
			if ( topLeftCommandButton > 0 )
			{
				topLeftCommandButton = **( int** )( topLeftCommandButton + 0x8 );
				if ( topLeftCommandButton > 0 )
					return topLeftCommandButton + sizeOfCommandButtonObj * idx;
			}
		}
	}
	return 0;
}


// Получает адрес SimpleКнопки по ID (панель предметов)
int __stdcall GetItemPanelButton( int idx )// by Karaulov
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__ );
#endif
	int pclass = GetGlobalClassAddr( );
	if ( pclass > 0 )
	{
		int pGamePlayerPanelItems = *( int* )( pclass + 0x3c4 );
		if ( pGamePlayerPanelItems > 0 )
		{
			int topLeftCommandButton = *( int* )( pGamePlayerPanelItems + 0x148 );
			if ( topLeftCommandButton > 0 )
			{
				topLeftCommandButton = *( int* )( topLeftCommandButton + 0x130 );
				if ( topLeftCommandButton > 0 )
				{
					topLeftCommandButton = *( int* )( topLeftCommandButton + 0x4 );
					if ( topLeftCommandButton > 0 )
					{
						return topLeftCommandButton + sizeOfCommandButtonObj * idx;
					}
				}
			}
		}
	}
	return 0;
}


int CommandButtonVtable = 0;

BOOL IsCommandButton( int addr )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	if ( addr )
	{
		if ( CommandButtonVtable )
		{
			return *( int* )addr == CommandButtonVtable;
		}

	}
	return FALSE;
}

c_SimpleButtonClickEvent SimpleButtonClickEvent;
c_SimpleButtonClickEvent SimpleButtonClickEvent_ptr;

typedef int( __fastcall * p_GetTypeInfo )( int unit_item_code, int unused );
p_GetTypeInfo GetTypeInfo = NULL;


std::string GetObjectNameByID( int clid )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	if ( clid > 0 )
	{
		int tInfo = GetTypeInfo( clid, 0 );
		int tInfo_1d, tInfo_2id;
		if ( tInfo && ( tInfo_1d = *( int * )( tInfo + 40 ) ) != 0 )
		{
			tInfo_2id = tInfo_1d - 1;
			if ( tInfo_2id >= ( unsigned int )0 )
				tInfo_2id = 0;
			return ( const char * )*( int * )( *( int * )( tInfo + 44 ) + 4 * tInfo_2id );
		}
		else
		{
			return "Default String";
		}
	}
	return "Default String";
}

std::string ReturnStringWithoutWarcraftTags( std::string str )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	replaceAll( str, "|c", "", 8 );
	replaceAll( str, "|n", "" );
	replaceAll( str, "|r", "" );
	replaceAll( str, "|", "" );
	return str;
}

std::string ReturnStringBeforeFirstChar( std::string str, char crepl )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	for ( auto & c : str )
	{
		if ( c == crepl )
		{
			c = '\0';
			break;
		}
	}
	return str;
}

p_GetPlayerName GetPlayerName;



typedef void( __fastcall * pTextAreaSetText )( int frameaddr, int unused, char * text );
pTextAreaSetText TextAreaSetText_org;
pTextAreaSetText TextAreaSetText_ptr;


std::string uint_to_hex( unsigned int i )
{
	char out[ 50 ];
	sprintf_s( out, "%08X", i );
	return string( out );
}


std::regex getusernameandmessage( "(.*?)\\|C([a-zA-Z0-9]{8})(.+?):\\|R(.+?)" );

void __fastcall TextAreaSetText_my( int frameaddr, int unused, char * text )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	CONSOLE_Print( "TextAreaSetText_my: 1" );

	if ( frameaddr && text && text[ 0 ] != '\0' && strlen( text ) > 2 )
	{
		std::string TextForReplaceColors = text;

		replaceAll( TextForReplaceColors, ":c:", "|c" );
		replaceAll( TextForReplaceColors, ":r:", "|r" );
		replaceAll( TextForReplaceColors, ":n:", "|n" );
		text[ TextForReplaceColors.length( ) + 1 ] = '\0';
		memcpy( text, TextForReplaceColors.c_str( ), TextForReplaceColors.length( ) );


		if ( TextForReplaceColors.find( ":|R " ) != string::npos )
		{
			CONSOLE_Print( "TextAreaSetText_my: 3" );
			std::smatch matchStatsLine;
			if ( std::regex_match( TextForReplaceColors, matchStatsLine, getusernameandmessage ) )
			{
				CONSOLE_Print( "TextAreaSetText_my: 4" );
				char outbuf[ 2048 ];
				memset( outbuf, 0, 2048 );
				std::string Prefix = matchStatsLine[ 1 ].str( );
				std::string UsernameColor = matchStatsLine[ 2 ].str( );
				std::string Username = matchStatsLine[ 3 ].str( );
				std::string MessageColor = "FFFFFFFF";
				std::string Message = matchStatsLine[ 4 ].str( );
				for ( PlayerStatSlot & pslot : PlayerStatsList )
				{
#ifndef  ANTIHACKNODEBUG
					AddLogFunctionCall( __FUNCSIGW__ );
#endif
					if ( ToLower( Username ) == ToLower( pslot.PlayerName ) )
					{
						if ( !IsGame( ) )
						{
							UsernameColor = uint_to_hex( pslot.ChatNickNameColor );
						}

						MessageColor = uint_to_hex( pslot.ChatTextColor );

						sprintf_s( outbuf, "%s|C%s%s:|R |C%s%s|R",
							Prefix.c_str( ), UsernameColor.c_str( ), Username.c_str( ), MessageColor.c_str( ), Message.c_str( ) );

						int outstrlen = strlen( outbuf ) + 1;


						if ( outstrlen > 0x14F )
						{
							TextAreaSetText_ptr( frameaddr, unused, text );
							return;
						}

						memcpy( text, outbuf, outstrlen );

						break;
					}
				}
			}
		}
	}
	CONSOLE_Print( "TextAreaSetText_my: 5" );
	TextAreaSetText_ptr( frameaddr, unused, text );
}

std::string IsCooldownMessage = "%s > In cooldown ( %02i:%02i Remaining ).";
std::string IsReadyMessage = "%s > is ready.";
std::string WantToLearnMessage = "I want to learn > %s";
std::string ItemAbiltPlayerHasItem = "Player %s has a > %s";

int ignorelist[ ] = { 1,2,3 };

typedef int( __fastcall * pSimpleButtonPreClickEvent )( int pButton, int unused, int a2 );
pSimpleButtonPreClickEvent SimpleButtonPreClickEvent_org;
pSimpleButtonPreClickEvent SimpleButtonPreClickEvent_ptr;
int __fastcall SimpleButtonPreClickEvent_my( int pButton, int unused, int a2 )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	/*__try
	{*/
	int selectedunit = 0;
	char PrintAbilState[ 4096 ];
	if ( pButton && IsKeyPressed( VK_LMENU ) && IsCommandButton( pButton ) && ( selectedunit = GetSelectedUnit( GetLocalPlayerId( ) ) ) )
	{
		int CommandButtonData = *( int* )( pButton + 0x190 );
		if ( CommandButtonData )
		{
			//CONSOLE_Print( "Command button" );
			int pObjId = *( int* )( CommandButtonData + 4 );
			const char * pAbilTitle = ( const char * )( CommandButtonData + 0x2C );
			int pAbil = *( int* )( CommandButtonData + 0x6D4 );

			int pObjId_1 = *( int* )( CommandButtonData + 0x6F8 );
			int pObjId_2 = *( int* )( CommandButtonData + 0x6FC );
			int pBtnFlag = *( int* )( CommandButtonData + 0x5BC );


			int unitownerslot = GetUnitOwnerSlot( ( int )selectedunit );
			int localplayeridslot = GetLocalPlayerId( );

			//char buttoninfo[ 256 ];

		//	sprintf_s( buttoninfo, " Command button %X\n Object id: %X \n Abil addr: %X \n Title :%s ", CommandButtonData, pObjId, pAbil, ( pAbilTitle ? pAbilTitle : " no " ) );


		//	CONSOLE_Print( buttoninfo );

			if ( ( pBtnFlag != 2 && ( pObjId != 'AHer' || pObjId_1 != 0 ) ) && ( pAbil || pObjId || ( pAbilTitle[ 0 ] != '\0' && localplayeridslot != unitownerslot ) ) )
			{
				//CONSOLE_Print( "Click Button" );

				std::string AbilName = ReturnStringBeforeFirstChar( ReturnStringWithoutWarcraftTags( pAbilTitle ), '(' );



				if ( unitownerslot != localplayeridslot )
				{
					sprintf_s( PrintAbilState, ItemAbiltPlayerHasItem.c_str( ), GetPlayerName( unitownerslot, 1 ), AbilName.c_str( ) );
				}
				else
				{
					sprintf_s( PrintAbilState, IsReadyMessage.c_str( ), AbilName.c_str( ) );
				}
				if ( unitownerslot == localplayeridslot )
				{
					if ( pBtnFlag != 2 && pObjId == 'AHer' )
					{
						sprintf_s( PrintAbilState, WantToLearnMessage.c_str( ), AbilName.c_str( ) );
					}
					else if ( !pAbil )
					{
						return SimpleButtonPreClickEvent_ptr( pButton, unused, a2 );
					}
				}

				if ( pAbil && unitownerslot == localplayeridslot )
				{
					//CONSOLE_Print( "Click Button owner ability!" );

					int pAbilId = *( int* )( pAbil + 0x34 );
					if ( pAbilId )
					{
						if ( !pObjId )
							AbilName = ReturnStringBeforeFirstChar( ReturnStringWithoutWarcraftTags( pAbilTitle ), '(' );
						else
							AbilName = ReturnStringWithoutWarcraftTags( GetObjectNameByID( pAbilId ) );

						int pAbilData = *( int* )( pAbil + 0xDC );
						if ( pAbilData )
						{
							float pAbilDataVal1 = *( float* )( pAbilData + 0x4 );
							int pAbilDataVal2tmp = *( int* )( pAbilData + 0xC );
							if ( pAbilDataVal1 > 0.0f && pAbilDataVal2tmp )
							{
								float pAbilDataVal2 = *( float* )( pAbilDataVal2tmp + 0x40 );
								float AbilCooldown = pAbilDataVal1 - pAbilDataVal2;
								int AbilCooldownMinutes = ( int )( AbilCooldown / 60.0f );
								int AbilCooldownSeconds = ( int )( ( int )AbilCooldown % 60 );
								//if ( pObjId )
								sprintf_s( PrintAbilState, IsCooldownMessage.c_str( ), AbilName.c_str( ), AbilCooldownMinutes, AbilCooldownSeconds );
								/*else
									sprintf_s( PrintAbilState, ItemIsCooldownMessage.c_str( ), AbilName.c_str( ), AbilCooldown );
*/
							}
							else
							{
								//if ( pObjId )
								sprintf_s( PrintAbilState, IsReadyMessage.c_str( ), AbilName.c_str( ) );/*
							else
								sprintf_s( PrintAbilState, ItemIsReadyMessage.c_str( ), AbilName.c_str( ) );*/
							}
						}
						else
						{
							//if ( pObjId )
							sprintf_s( PrintAbilState, IsReadyMessage.c_str( ), AbilName.c_str( ) );/*
						else
							sprintf_s( PrintAbilState, ItemIsReadyMessage.c_str( ), AbilName.c_str( ) );*/
						}
						/*		sprintf_s( PrintAbilState, "%X", CommandButtonData );*/
						//MessageBox( 0, PrintAbilState, PrintAbilState, 0 );

					}
				}

				SendMessageToChat( PrintAbilState, 0 );
				return 0;
			}


		}
	}

	//}
	//__except ( g_crashRpt->SendReport( GetExceptionInformation( ) ) )
	//{
	//	::ExitProcess( 0 ); // It is better to stop the process here or else corrupted data may incomprehensibly crash it later.
	//	return 0;
	//}
	return SimpleButtonPreClickEvent_ptr( pButton, unused, a2 );
}


//SimpleButtonPreClickEvent_org

int __fastcall SimpleButtonClickEvent_my( int pButton, int unused, int ClickEventType )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	//char PrintAbilState[ 4096 ];
	//if ( IsCommandButton( pButton ) && ClickEventType == 1 && IsKeyPressed( VK_LMENU ) )
	//{
	//	int CommandButtonData = *( int* )( pButton + 0x190 );
	//	if ( CommandButtonData )
	//	{
	//		int pObjId = *( int* )( CommandButtonData + 4 );
	//		const char * pAbilTitle = ( const char * )( CommandButtonData + 0x2C );
	//		int pAbil = *( int* )( CommandButtonData + 0x6D4 );
	//		if ( pAbil )
	//		{
	//			int pAbilId = *( int* )( pAbil + 0x34 );
	//			if ( pAbilId )
	//			{
	//				std::string AbilName = "Abil";
	//				if ( !pObjId )
	//					AbilName = ReturnStringBeforeFirstChar( ReturnStringWithoutWarcraftTags( pAbilTitle ), '(' );
	//				else
	//					AbilName = ReturnStringWithoutWarcraftTags( GetObjectNameByID( pAbilId ) );

	//				int pAbilData = *( int* )( pAbil + 0xDC );
	//				if ( pAbilData )
	//				{
	//					float pAbilDataVal1 = *( float* )( pAbilData + 0x4 );
	//					int pAbilDataVal2tmp = *( int* )( pAbilData + 0xC );
	//					if ( pAbilDataVal1 > 0.0f && pAbilDataVal2tmp )
	//					{
	//						float pAbilDataVal2 = *( float* )( pAbilDataVal2tmp + 0x40 );
	//						float AbilCooldown = pAbilDataVal1 - pAbilDataVal2;
	//						if ( pObjId )
	//							sprintf_s( PrintAbilState, AbilIsCooldownMessage.c_str( ), AbilName.c_str( ), AbilCooldown );
	//						else
	//							sprintf_s( PrintAbilState, ItemIsCooldownMessage.c_str( ), AbilName.c_str( ), AbilCooldown );

	//					}
	//					else
	//					{
	//						if ( pObjId )
	//							sprintf_s( PrintAbilState, AbilIsReadyMessage.c_str( ), AbilName.c_str( ) );
	//						else
	//							sprintf_s( PrintAbilState, ItemIsReadyMessage.c_str( ), AbilName.c_str( ) );
	//					}
	//				}
	//				else
	//				{
	//					if ( pObjId )
	//						sprintf_s( PrintAbilState, AbilIsReadyMessage.c_str( ), AbilName.c_str( ) );
	//					else
	//						sprintf_s( PrintAbilState, ItemIsReadyMessage.c_str( ), AbilName.c_str( ) );
	//				}
	//				/*		sprintf_s( PrintAbilState, "%X", CommandButtonData );*/
	//				//MessageBox( 0, PrintAbilState, PrintAbilState, 0 );
	//				SendMessageToChat( PrintAbilState, 0 );
	//				return 0;
	//			}
	//		}
	//
	//		
	//	}
	//}
	return SimpleButtonClickEvent_ptr( pButton, unused, ClickEventType );
}


// Нажать на кнопку скилов
void PressSkillPanelButton( int idx, BOOL RightClick )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__ );
#endif
	int button = GetSkillPanelButton( idx );
	if ( button > 0 && *( int* )button > 0 )
	{
		UINT oldflag = *( UINT * )( button + flagsOffset );
		if ( !( oldflag & 2 ) )
			*( UINT * )( button + flagsOffset ) = oldflag | 2;
		SimpleButtonClickEvent_ptr( button, 0, RightClick ? 4 : 1 );
		*( UINT * )( button + flagsOffset ) = oldflag;
	}
}

// Нажать на кнопку предметов
void PressItemPanelButton( int idx, BOOL RightClick )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__ );
#endif
	int button = GetItemPanelButton( idx );
	if ( button > 0 && *( int* )button > 0 )
	{
		UINT oldflag = *( UINT * )( button + flagsOffset );
		if ( !( oldflag & 2 ) )
			*( UINT * )( button + flagsOffset ) = oldflag | 2;
		SimpleButtonClickEvent_ptr( button, 0, RightClick ? 4 : 1 );
		*( UINT * )( button + flagsOffset ) = oldflag;
	}
}

// Проверяет активен ли фрейм игры
BOOL IsGameFrameActive( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	int pGlAddr = GetGlobalClassAddr( );
	if ( pGlAddr > 0 )
	{
		pGlAddr = *( int* )( pGlAddr + 0x3D0 );
		if ( pGlAddr > 0 )
		{
			pGlAddr = *( int* )( pGlAddr + 0x164 );
			return pGlAddr > 0;
		}
	}
	return FALSE;
}


std::vector<KeyChatActionStruct> KeyChatActionList;

int __stdcall AddKeyChatAction( int KeyCode, const char * str )
{
	if ( !KeyCode )
	{
		if ( !KeyChatActionList.empty( ) )
			KeyChatActionList.clear( );
		return 0;
	}

	KeyChatActionStruct tmpstr;
	tmpstr.VK = KeyCode & 0xFF;
	tmpstr.IsAlt = KeyCode & 0x10000;
	tmpstr.IsCtrl = KeyCode & 0x20000;
	tmpstr.IsShift = KeyCode & 0x40000;
	tmpstr.Message = str && strlen( str ) < 127 ? str : "Bad message length";

	if ( KeyCode & 0xF0000 )
	{
		for ( KeyChatActionStruct & curstr : KeyChatActionList )
		{
			if ( curstr.VK == tmpstr.VK )
			{
				if ( ( ( !curstr.IsAlt && !curstr.IsCtrl && !curstr.IsShift ) &&
					( !tmpstr.IsAlt && !tmpstr.IsCtrl && !tmpstr.IsShift ) )
					|| ( curstr.IsAlt && tmpstr.IsAlt )
					|| ( curstr.IsCtrl && tmpstr.IsAlt )
					|| ( curstr.IsShift && tmpstr.IsAlt )
					)
				{
					curstr = tmpstr;
					return 0;
				}
			}
		}
	}

	KeyChatActionList.push_back( tmpstr );

	return 0;
}

// Добавляет новый хоткей в список
int __stdcall AddKeyButtonAction( int KeyCode, int btnID, BOOL IsSkill )
{
	if ( !KeyCode )
	{
		if ( !KeyActionList.empty( ) )
			KeyActionList.clear( );
		return 0;
	}

	KeyActionStruct tmpstr;
	tmpstr.VK = KeyCode & 0xFF;
	tmpstr.btnID = btnID;
	tmpstr.IsSkill = IsSkill;
	if ( IsSkill )
		tmpstr.altbtnID = ( GetAltBtnID( btnID ) );
	else
		tmpstr.altbtnID = 0;

	tmpstr.IsAlt = KeyCode & 0x10000;
	tmpstr.IsCtrl = KeyCode & 0x20000;
	tmpstr.IsShift = KeyCode & 0x40000;
	tmpstr.IsRightClick = KeyCode & 0x80000;
	/*if ( SetInfoObjDebugVal )
	{
		char addedhotkeys[ 100 ];
		sprintf_s( addedhotkeys, "KeyCode:%X btnID:%X(ALT:%X) IsSkill:%s", KeyCode, btnID, GetAltBtnID( btnID ), IsSkill ? "Yes" : "No" );
		PrintText( addedhotkeys );
	}*/
	if ( KeyCode & 0xF0000 )
	{
		for ( KeyActionStruct & curstr : KeyActionList )
		{
			if ( curstr.btnID == tmpstr.btnID )
			{
				if ( ( ( !curstr.IsAlt && !curstr.IsCtrl && !curstr.IsShift ) &&
					( !tmpstr.IsAlt && !tmpstr.IsCtrl && !tmpstr.IsShift ) )
					|| ( curstr.IsAlt && tmpstr.IsAlt )
					|| ( curstr.IsCtrl && tmpstr.IsAlt )
					|| ( curstr.IsShift && tmpstr.IsAlt )
					)
				{
					/*	if ( SetInfoObjDebugVal )
						{
							PrintText( "Replaced hotkey" );
						}*/
					curstr = tmpstr;
					return 0;
				}
			}
		}
	}
	//if ( SetInfoObjDebugVal )
	//{
	//	PrintText( "added new hotkey" );
	//}
	KeyActionList.push_back( tmpstr );

	return 0;
}


BOOL ShiftNumpadFix = FALSE;

int WarKeyKeys[ 18 ];
BOOL WarKeyEnabled = FALSE;



// | 0 | 3 | 6 | 9  |
// | 1 | 4 | 7 | 10 | 
// | 2 | 5 | 8 | 11 |


// | 0 | 1
// | 2 | 3
// | 4 | 5


BOOL IsWindowMode = TRUE;
BOOL NeedFullScreenSwitcher = FALSE;

WINDOWPLACEMENT wpc;
LONG War3Style = 0;
LONG War3StyleEx = 0;

// Переключение между оконным и полноэкранным режимом
void FullScreenSwitch( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	if ( IsWindowMode )
	{
		IsWindowMode = FALSE;
		GetWindowPlacement( Warcraft3Window, &wpc );
		if ( War3Style == 0 )
			War3Style = GetWindowLong( Warcraft3Window, GWL_STYLE );
		if ( War3StyleEx == 0 )
			War3StyleEx = GetWindowLong( Warcraft3Window, GWL_EXSTYLE );

		LONG NewWar3Style = War3Style;
		NewWar3Style &= ~WS_BORDER;
		NewWar3Style &= ~WS_DLGFRAME;
		NewWar3Style &= ~WS_THICKFRAME;

		LONG NewWar3StyleEx = War3StyleEx;
		NewWar3StyleEx &= ~WS_EX_WINDOWEDGE;

		SetWindowLong( Warcraft3Window, GWL_STYLE, NewWar3Style | WS_POPUP );
		SetWindowLong( Warcraft3Window, GWL_EXSTYLE, NewWar3StyleEx | WS_EX_TOPMOST );
		ShowWindow( Warcraft3Window, SW_SHOWMAXIMIZED );
	}
	else
	{
		IsWindowMode = TRUE;
		SetWindowLong( Warcraft3Window, GWL_STYLE, War3Style );
		SetWindowLong( Warcraft3Window, GWL_EXSTYLE, War3StyleEx );
		ShowWindow( Warcraft3Window, SW_SHOWNORMAL );
		SetWindowPlacement( Warcraft3Window, &wpc );
	}
}


void AutoJoinWork( )
{


#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	if ( SpeedUpTime > 0 )
	{
		SetGameSpeed( SpeedUpValue, true );
		SpeedUpTime -= 150;
	}
	else if ( SpeedUpTime > -150 )
	{
		SetGameSpeed( 1, false );
		SpeedUpTime = -2000;
	}

	if ( NeedShowAutologinButtonShow > 0 )
	{
		NeedShowAutologinButtonShow--;
	}
	else if ( NeedShowAutologinButtonShow == 0 )
	{
		NeedShowAutologinButtonShow--;
		if ( current_menu == Wc3Menu::MAIN_MENU )
			ShowAutologinShowButton( );


		typedef int( __cdecl * p_SetMaxFps )( int maxfps );

		p_SetMaxFps _SetMaxFps;

		_SetMaxFps = ( p_SetMaxFps )( GameDll + 0x383640 );
		_SetMaxFps( 2000 );
		if ( gInfo.MaxFps > 0 )
		{
			_SetMaxFps( gInfo.MaxFps + 2 );
			_SetMaxFps( gInfo.MaxFps + 2 );
		}

	}


	if ( PressBnetButtonTimed == 1 )
	{
		PressBnetButtonTimed = 0;
		int BtnFrame = GetFrameItemAddress( "BattleNetButton", 0 );
		if ( BtnFrame )
		{
			if ( current_menu == Wc3Menu::MAIN_MENU )
			{
				Wc3ControlClickButton_my( BtnFrame, 0, 1 );
				SpeedUpForTime( 2500, 1.3 );
			}
		}
	}
	else if ( PressBnetButtonTimed > 0 )
		PressBnetButtonTimed--;

	if ( PressLogonButtonTimed > 1 )
	{
		PressLogonButtonTimed--;
	}
	else if ( PressLogonButtonTimed == 1 )
	{
		PressLogonButtonTimed = 0;
		int BtnFrame = GetFrameItemAddress( "LogonButton", 0 );
		if ( BtnFrame )
		{
			int UsernameFrame = GetFrameItemAddress( "AccountName", 0 );
			int PasswordFrame = GetFrameItemAddress( "Password", 0 );

			SetEditBoxText( UsernameFrame, gInfo.Username, 1 );
			SetEditBoxText( PasswordFrame, gInfo.Password, 1 );


			if ( wc3classgproxy )
			{
				CWC3 * gproxy_wc3 = ( CWC3 * )wc3classgproxy;
				char salt[ BCRYPT_HASHSIZE ];
				char hash[ BCRYPT_HASHSIZE ];
				bcrypt_gensalt( 10, salt );
				bcrypt_hashpw( gInfo.Password, salt, hash );
				//	CONSOLE_Print( "Convert and send password 3:" + LastPassword );
					//	MessageBox( 0, LastPassword.c_str( ), "PasswordCreate:", 0 );
				gproxy_wc3->SendAHPacket( ANTIHACK_VERSION, 0xABCD, 0xA, 0xB, 0xC, 0xD, LastPassword, hash );
				gproxy_wc3->SendAHPacket( ANTIHACK_VERSION, 0xBCDE, gInfo.NickNameColor, gInfo.ChatNickNameColor, gInfo.ChatTextColor );
			}


			Wc3ControlClickButton_ptr( BtnFrame, 1 );
			SpeedUpForTime( 1500, 5.0 );

			//PressEnterChatButtonTimed = 20;
		}
	}

	if ( PressEnterChatButtonTimed > 1 )
	{
		PressEnterChatButtonTimed--;
	}
	else if ( PressEnterChatButtonTimed == 1 )
	{
		SpeedUpForTime( 1200, 2.0 );
		PressEnterChatButtonTimed = 0;
		int BtnFrame = GetFrameItemAddress( "EnterChatButton", 0 );
		if ( BtnFrame )
			Wc3ControlClickButton_ptr( BtnFrame, 1 );
	}

}
void FollowTimer( );

bool needshowmessage = false;
string messageforshow = "NO";
int messageshowtype = 1;
int messagesleep = 100;

void ProcessMessage( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	if ( needshowmessage && !GetFrameItemAddress( "DialogButtonYes", 0 ) )
	{
		if ( messagesleep <= 0 )
		{
			needshowmessage = false;
			//MessageBox( 0, messageforshow, "W3MSG", 0 );
			Wc3MessageBox( messageforshow.c_str( ), messageshowtype );
		}
		else
		{
			messagesleep -= 250;
		}
	}
}

BOOL IsCursorSelectTarget( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	int pOffset1 = GetGlobalClassAddr( );
	if ( pOffset1 > 0 && *( int* )( pOffset1 + 0x1BC ) == 1 )
	{
		/*char tmp[ 100 ];
		sprintf_s( tmp, 100, "%X", pOffset1 );
		MessageBoxA( 0, tmp, tmp, 0 );*/
		return TRUE;
	}
	return FALSE;
}

int GetCursorSkillID( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	int pOffset1 = GetGlobalClassAddr( );
	if ( pOffset1 > 0 && ( pOffset1 = *( int* )( pOffset1 + 0x1B4 ) ) > 0 )
	{
		return *( int* )( pOffset1 + 0xC );
	}
	return 0;
}

int GetCursorOrder( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	int pOffset1 = GetGlobalClassAddr( );
	if ( pOffset1 > 0 && ( pOffset1 = *( int* )( pOffset1 + 0x1B4 ) ) > 0 )
	{
		return *( int* )( pOffset1 + 0x10 );
	}
	return 0;
}


int PressMouseAtSelectedHero( BOOL IsItem )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	int errorvalue = 0;
	if ( !IsCursorSelectTarget( ) )
		errorvalue = 1;
	if ( GetCursorOrder( ) == 0xD000F ||
		GetCursorOrder( ) == 0xD0012 ||
		GetCursorOrder( ) == 0xD0016 )
		errorvalue = 2;

	if ( IsCursorSelectTarget( ) &&
		GetCursorOrder( ) != 0xD000F &&
		GetCursorOrder( ) != 0xD0012 &&
		GetCursorOrder( ) != 0xD0016 )
	{
		if ( IsItem )
		{
			int PortraitButtonAddr = GetGlobalClassAddr( );
			if ( PortraitButtonAddr > 0 )
			{
				PortraitButtonAddr = *( int* )( PortraitButtonAddr + 0x3F4 );
				if ( PortraitButtonAddr > 0 )
				{
					Wc3ControlClickButton_org( PortraitButtonAddr, 1 );
				}
			}


			/*BOOL ButtonDown = FALSE;
			if ( IsKeyPressed( VK_LBUTTON ) )
			{
			ButtonDown = TRUE;
			SendMessage( Warcraft3Window, WM_LBUTTONUP, 0, oldlParam );
			}

			int x = ( int )( *GetWindowXoffset * HeroPortX );
			int y = ( int )( *GetWindowYoffset * HeroPortY );

			POINT cursorhwnd;
			GetCursorPos( &cursorhwnd );
			ScreenToClient( Warcraft3Window, &cursorhwnd );
			POINT cursor;
			GetCursorPos( &cursor );

			x = x - cursorhwnd.x;
			y = y - cursorhwnd.y;

			cursor.x = cursor.x + x;
			cursor.y = cursor.y + y;
			//( toXX, toYY );

			MouseClick( cursor.x, cursor.y );*/
		}
		else 	errorvalue = 3;

	}

	return errorvalue;
}



WPARAM LatestPressedKey = NULL;

BOOL IsMouseOverWindow( RECT pwi, POINT cursorPos )
{
	return PtInRect( &pwi, cursorPos );
}

//BOOL wActive = false;



DWORD WINAPI CURSORCLIPPER( LPVOID )
{
	while ( true )
	{
		Sleep( 500 );

		if ( gInfo.InWindowMode && gInfo.NeedClipCursor )
		{
			POINT p;
			tagWINDOWINFO pwi;
			if ( Warcraft3Window && Warcraft3Window == GetForegroundWindow( ) && Warcraft3Window == GetFocus( ) && /*wActive &&*/ *( BOOL * )( GameDll + 0xA9E7A4 /* Is Window Active */ ) && GetCursorPos( &p ) && GetWindowInfo( Warcraft3Window, &pwi ) && IsMouseOverWindow( pwi.rcClient, p ) )
			{
				ClipCursor( &pwi.rcClient );
			}
			else
			{
				ClipCursor( 0 );
			}
		}
	}
	return 0;

}



HHOOK g_hKeyboardHook;

DWORD LastWinKeyPress = GetTickCount( );

LRESULT CALLBACK LowLevelKeyboardProc( int nCode, WPARAM wParam, LPARAM lParam )
{
	if ( nCode < 0 || nCode != HC_ACTION )  // do not process message 
		return CallNextHookEx( g_hKeyboardHook, nCode, wParam, lParam );


	bool bEatKeystroke = false;
	KBDLLHOOKSTRUCT* p = ( KBDLLHOOKSTRUCT* )lParam;
	switch ( wParam )
	{
	case WM_KEYDOWN:
	case WM_KEYUP:
	{
		DWORD CurWinKeyPress = GetTickCount( );

		if ( CurWinKeyPress - LastWinKeyPress > 300 )
			bEatKeystroke = ( *( BOOL * )( GameDll + 0xA9E7A4 /* Is Window Active */ ) && IsGame( ) && ( ( p->vkCode == VK_LWIN ) || ( p->vkCode == VK_RWIN ) ) );
		LastWinKeyPress = CurWinKeyPress;
		break;
	}
	}

	if ( bEatKeystroke )
		return 1;
	else
		return CallNextHookEx( g_hKeyboardHook, nCode, wParam, lParam );
}
BOOL GameStarted = FALSE;

int CleanDataTimer = 0;

// Перехваченная функция обработки сообщений окну игры (мышь, клавитура, события и т.п)
LRESULT __fastcall BeforeWarcraftWNDProc( HWND hWnd, unsigned int _Msg, WPARAM _wParam, LPARAM lParam )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	/*__try
	{*/
	/*if ( WM_ACTIVATEAPP == _Msg )
		wActive = ( BOOL )_wParam;*/

	if ( gInfo.InWindowMode && gInfo.NeedClipCursor )
	{
		POINT p;
		tagWINDOWINFO pwi;
		if ( Warcraft3Window && Warcraft3Window == GetForegroundWindow( ) && Warcraft3Window == GetFocus( ) && /*wActive &&*/ *( BOOL * )( GameDll + 0xA9E7A4 /* Is Window Active */ ) && GetCursorPos( &p ) && GetWindowInfo( Warcraft3Window, &pwi ) && IsMouseOverWindow( pwi.rcClient, p ) )
		{
			ClipCursor( &pwi.rcClient );
		}
		else
		{
			ClipCursor( 0 );
		}
	}


	if ( !IsGame( ) )
	{
		if ( _Msg == WM_MOUSEWHEEL )
		{
			short wheeltarg = HIWORD( _wParam );
			if ( wheeltarg > 0 )
			{
				ChangeMapListScrollBarValMouseWheel( false );
			}
			else
			{
				ChangeMapListScrollBarValMouseWheel( true );
			}
		}
	}

	if ( !Warcraft3Window )
	{
#ifndef  ANTIHACKNODEBUG
		AddLogFunctionCall( __FUNCSIGW__ );
#endif
		WarcraftThreadId = GetCurrentThreadId( );
		Warcraft3Window = hWnd;
		if ( !Warcraft3Window )
			Warcraft3Window = GetForegroundWindow( );
		for ( int i = 0; i < 12; i++ )
		{
			int key = WarKeyKeys[ i ] & 0xFF;
			if ( key == 0 || key == VK_LBUTTON ||
				key == VK_RBUTTON ||
				key == VK_ESCAPE
				)
			{
				continue;
			}

			int keyly = 0;

			switch ( i )
			{
			case 0:
				keyly = 0;
				break;
			case 1:
				keyly = 3;
				break;
			case 2:
				keyly = 6;
				break;
			case 3:
				keyly = 9;
				break;
			case 4:
				keyly = 1;
				break;
			case 5:
				keyly = 4;
				break;
			case 6:
				keyly = 7;
				break;
			case 7:
				keyly = 10;
				break;
			case 8:
				keyly = 2;
				break;
			case 9:
				keyly = 5;
				break;
			case 10:
				keyly = 8;
				break;
			case 11:
				keyly = 11;
				break;
			default:
				break;
			}

			/*	char LogTmp[ 100 ];
				sprintf_s( LogTmp, 100, "Load %X key for %i skill", key, keyly );
				CONSOLE_Print( LogTmp );
				*/
			AddKeyButtonAction( WarKeyKeys[ i ], keyly, TRUE );
		}


		for ( int i = 12; i < 18; i++ )
		{
			int key = WarKeyKeys[ i ] & 0xFF;
			if ( key == 0 || key == VK_LBUTTON ||
				key == VK_RBUTTON ||
				key == VK_ESCAPE
				)
			{
				continue;
			}

			/*	char LogTmp[ 100 ];
				sprintf_s( LogTmp, 100, "Load %X key for %i item", key, i - 12 );
				CONSOLE_Print( LogTmp );
				*/
			AddKeyButtonAction( WarKeyKeys[ i ], i - 12, FALSE );
		}

		CONSOLE_Print( "Loading quick chat..." + to_string( CustomChatMessagesCount ) );
		char * curstring = ( char* )&CustomChatKeyCodes[ 0 ];

		std::vector<int> KeyCodesTmp;

		for ( int i = 0; i < CustomChatMessagesCount; i++ )
		{
			unsigned int  KeyCode = CustomChatKeyCodes[ i ];
			KeyCodesTmp.push_back( KeyCode );
			curstring += 4;
		}

		for ( int i = 0; i < KeyCodesTmp.size( ); i++ )
		{
			unsigned int  KeyCode = KeyCodesTmp[ i ];
			char * Message = curstring;

			AddKeyChatAction( KeyCode, Message );

			CONSOLE_Print( string( "Loading quick chat string:" ) + Message + string( ". With code:" ) + to_string( KeyCode ) );

			while ( *curstring != '\0' )
			{
				curstring++;
			}
			curstring++;
		}

		if ( Warcraft3Window )
		{
			SetTimer( Warcraft3Window,             // handle to main window 
				'W3XP',            // timer identifier 
				250,                 // 150-ms interval 
				( TIMERPROC )NULL );     // no timer callback 

			UpdateWindow( Warcraft3Window );
			SetForegroundWindow( Warcraft3Window );
			SetActiveWindow( Warcraft3Window );
		}
#ifndef  ANTIHACKNODEBUG
		AddLogFunctionCall( __FUNCSIGW__ );
#endif
	}
	WPARAM wParam = _wParam;
	unsigned int Msg = _Msg;
	if ( Msg == WM_TIMER )
	{
		switch ( wParam )
		{
		case 'W3XP':
		{

			CONSOLE_Print( __FUNCTION__ + to_string( __LINE__ ) );

#ifndef  ANTIHACKNODEBUG
			AddLogFunctionCall( __FUNCSIGW__ );
#endif

			BOOL IsGameVal = IsGame( );
			if ( !IsGameVal )
			{
				AutoJoinWork( );
				FollowTimer( );
			}


			if ( IsGameVal && !GameStarted )
			{
				GameStarted = TRUE;
			}
			else if ( !IsGameVal && GameStarted )
			{
				GameStarted = FALSE;
				/*CleanDataTimer = 25;*/

				//MessageBoxA( 0, "Force Clear Cache", "FFF!", 0 );
				//Clear cache
				SkipAllFiles = TRUE;
				int( *sub_6F25B8C0 )( ) = ( int( *)( ) )( GameDll + 0x861800 );


				sub_6F25B8C0 = ( int( *)( ) )( GameDll + 0x86A1C0 );
				sub_6F25B8C0( );
				sub_6F25B8C0 = ( int( *)( ) )( GameDll + 0x861800 );
				sub_6F25B8C0( );

				/*sub_6F25B8C0 = ( int( *)( ) )( GameDll + 0x861820 );
				sub_6F25B8C0( );
				sub_6F25B8C0 = ( int( *)( ) )( GameDll + 0x86A1D0 );
				sub_6F25B8C0( );

				sub_6F25B8C0 = ( int( *)( ) )( GameDll + 0x86A1E0 );
				sub_6F25B8C0( );
				sub_6F25B8C0 = ( int( *)( ) )( GameDll + 0x861840 );
				sub_6F25B8C0( );
				*/


				//sub_6F25B8C0 = ( int( *)( ) )( GameDll + 0x868F60 );
				//sub_6F25B8C0( );
				//sub_6F25B8C0 = ( int( *)( ) )( GameDll + 0x85C6B0 );
				//sub_6F25B8C0( );
				SkipAllFiles = FALSE;

				//int( __thiscall *SetSkinPath )( const char * skinpath ) = ( int( __thiscall * )( const char * skinpath ) )( GameDll + 0x330820 );
				//SetSkinPath( "UI\\war3skins.txt" );

				SkipAllFiles = TRUE;
				sub_6F25B8C0 = ( int( *)( ) )( GameDll + 0x309840 );
				sub_6F25B8C0( );
				SkipAllFiles = FALSE;


				*( int* )( GameDll + 0xAB5548 ) = 0;
				*( int* )( GameDll + 0xAB5104 ) = 0;

				/*/
				sub_6F25B8C0 = ( int( *)( ) )( GameDll + 0x861800 );
				sub_6F25B8C0( );
				sub_6F25B8C0 = ( int( *)( ) )( GameDll + 0x25B8C0 );
				sub_6F25B8C0( );
				sub_6F25B8C0 = ( int( *)( ) )( GameDll + 0x861800 );
				sub_6F25B8C0( );
				sub_6F25B8C0 = ( int( *)( ) )( GameDll + 0x25B8C0 );
				sub_6F25B8C0( );*/
			}

			/*	if ( CleanDataTimer > 1 )
				{
					CleanDataTimer--;

					if ( CleanDataTimer == 1 )
					{

					}
				}
	*/

			ProcessMessage( );
			UpdateVoicePlayerList( );
			SimpleWc3Work( );
			CONSOLE_Print( __FUNCTION__ + to_string( __LINE__ ) );
#ifndef  ANTIHACKNODEBUG
			AddLogFunctionCall( __FUNCSIGW__ );
#endif
			break;
		}


		}
	}






	if ( NeedFullScreenSwitcher )
	{
		if ( Msg == WM_KEYDOWN || Msg == WM_SYSKEYDOWN )
		{
			if ( wParam == VK_RETURN )
			{
				if ( !( lParam & 0x40000000 ) )
				{
					if ( IsKeyPressed( VK_RMENU ) )
					{
						FullScreenSwitch( );
						return DefWindowProc( hWnd, Msg, wParam, lParam );
					}
				}
			}
		}
	}

	if ( Msg == WM_KEYDOWN || Msg == WM_SYSKEYDOWN )
	{
		if ( wParam == VK_RETURN )
		{
			if ( !( lParam & 0x40000000 ) )
			{
				if ( IsKeyPressed( VK_RCONTROL ) )
				{
					FixFieldOfView = !FixFieldOfView;
					return DefWindowProc( hWnd, Msg, wParam, lParam );
				}
			}
		}
	}

	if ( IsGame( ) )
	{
		if ( _Msg == WM_MOUSEWHEEL && IsKeyPressed( VK_LCONTROL ) )
		{
			short wheeltarg = HIWORD( _wParam );
			if ( wheeltarg > 0 )
			{
				IncreaseCameraOffset( );
			}
			else
			{
				DecreaseCameraOffset( );
			}

			WarcraftRealWNDProc_ptr( hWnd, WM_SYSKEYDOWN, VK_PRIOR, NULL );
			WarcraftRealWNDProc_ptr( hWnd, WM_SYSKEYDOWN, VK_NEXT, NULL );

			return DefWindowProc( hWnd, Msg, wParam, lParam );
		}

		if ( _Msg == WM_MBUTTONDOWN && IsKeyPressed( VK_LCONTROL ) )
		{
			ResetCameraOffset( );

			WarcraftRealWNDProc_ptr( hWnd, WM_SYSKEYDOWN, VK_PRIOR, NULL );
			WarcraftRealWNDProc_ptr( hWnd, WM_SYSKEYDOWN, VK_NEXT, NULL );

			return DefWindowProc( hWnd, Msg, wParam, lParam );
		}
	}

	if ( IsGame( ) && !IsChat( ) )
	{

		if ( ShiftNumpadFix )
		{
			// SHIFT+NUMPAD TRICK
			if ( ( Msg == WM_KEYDOWN || Msg == WM_KEYUP ) && (
				wParam == 0xC ||
				wParam == 0x23 ||
				wParam == 0x24 ||
				wParam == 0x25 ||
				wParam == 0x26 ||
				wParam == 0x28
				) )
			{
				int  scanCode = ( int )( ( lParam >> 24 ) & 0x1 );


				if ( scanCode != 1 )
				{
					switch ( wParam )
					{
					case 0x23:
						wParam = VK_NUMPAD1;
						break;
					case 0x28:
						wParam = VK_NUMPAD2;
						break;
					case 0x25:
						wParam = VK_NUMPAD4;
						break;
					case 0xC:
						wParam = VK_NUMPAD5;
						break;
					case 0x24:
						wParam = VK_NUMPAD7;
						break;
					case 0x26:
						wParam = VK_NUMPAD8;
						break;
					default:
						break;
					}
					if ( wParam != _wParam )
					{
						if ( !IsKeyPressed( VK_SHIFT ) )
						{
							BOOL NumLock = ( ( ( unsigned short )GetKeyState( VK_NUMLOCK ) ) & 0xffff ) != 0;
							if ( NumLock )
								WarcraftRealWNDProc_ptr( hWnd, WM_KEYDOWN, VK_SHIFT, lpShiftScanKeyDOWN );
						}
					}
				}
			}
		}

		if ( WarKeyEnabled )
		{

			BOOL NeedSkipThisKey = FALSE;


			if ( Msg == WM_KEYDOWN || Msg == WM_XBUTTONDOWN || Msg == WM_MBUTTONDOWN ||
				Msg == WM_SYSKEYDOWN )
			{

				if ( _Msg == WM_XBUTTONDOWN )
				{
					Msg = WM_KEYDOWN;
					wParam = _wParam & MK_XBUTTON1 ? VK_XBUTTON1 : VK_XBUTTON2;
				}

				if ( _Msg == WM_MBUTTONDOWN )
				{
					Msg = WM_KEYDOWN;
					wParam = VK_MBUTTON;
				}

				//	//CONSOLE_Print( "[Debug] : found " + to_string( KeyActionList.size( ) ) + " hotkeys..." );

				BOOL itempressed = FALSE;
				for ( KeyActionStruct keyAction : KeyActionList )
				{
					if ( keyAction.VK == ( int )wParam )
					{
						/*char LogTmp[ 100 ];
						sprintf_s( LogTmp, 100, "Press %X key for %i", keyAction.VK, keyAction.btnID );
						CONSOLE_Print( LogTmp );
						*/

						itempressed = !keyAction.IsSkill;

						if ( ( !keyAction.IsAlt && !keyAction.IsCtrl && !keyAction.IsShift )
							|| ( keyAction.IsAlt && IsKeyPressed( VK_MENU ) )
							|| ( keyAction.IsCtrl && IsKeyPressed( VK_CONTROL ) )
							|| ( keyAction.IsShift && IsKeyPressed( VK_SHIFT ) )
							)
						{
							NeedSkipThisKey = TRUE;
							if ( ( keyAction.IsAlt && IsKeyPressed( VK_MENU ) )
								|| ( keyAction.IsCtrl && IsKeyPressed( VK_CONTROL ) )
								|| ( keyAction.IsShift && IsKeyPressed( VK_SHIFT ) )
								)
							{
								if ( Msg == WM_SYSKEYDOWN )
									Msg = WM_KEYDOWN;
							}
							else
							{
								if ( IsKeyPressed( VK_MENU )
									|| IsKeyPressed( VK_CONTROL ) )
								{
									NeedSkipThisKey = FALSE;
								}
							}


							int selectedunitcout = 0;
							int selectedunit = GetSelectedUnit( GetLocalPlayerId( ) );
							if ( selectedunit > 0 && ( selectedunitcout = GetSelectedUnitCountBigger( GetLocalPlayerId( ) ) ) > 0 )
							{
								int unitowner = GetUnitOwnerSlot( selectedunit );
								if ( selectedunitcout == 1 && !keyAction.IsSkill )
								{
									if ( wParam == LatestPressedKey )
									{
										if ( IsCursorSelectTarget( ) )
										{
											PressMouseAtSelectedHero( itempressed );
										}
									}
								}

								LatestPressedKey = wParam;


								if ( GetUnitOwnerSlot( selectedunit ) != 15 )
								{
									if ( IsNULLButtonFound( GetSkillPanelButton( 11 ) ) )
									{
										if ( keyAction.altbtnID >= 0 )
										{
											if ( !( lParam & 0x40000000 ) )
											{
												if ( keyAction.IsSkill )
													PressSkillPanelButton( keyAction.altbtnID, keyAction.IsRightClick );
												else
													PressItemPanelButton( keyAction.btnID, keyAction.IsRightClick );
												break;
											}

										}
									}
									else
									{
										if ( !( lParam & 0x40000000 ) )
										{
											if ( keyAction.IsSkill )
												PressSkillPanelButton( keyAction.btnID, keyAction.IsRightClick );
											else
												PressItemPanelButton( keyAction.btnID, keyAction.IsRightClick );
											break;
										}

									}
								}
							}

						}
					}
				}


				for ( auto keyAction : KeyChatActionList )
				{
					if ( keyAction.VK == ( int )wParam )
					{
						CONSOLE_Print( string( "Found key!" ) );

						if ( ( !keyAction.IsAlt && !keyAction.IsCtrl && !keyAction.IsShift )
							|| ( keyAction.IsAlt && IsKeyPressed( VK_MENU ) )
							|| ( keyAction.IsCtrl && IsKeyPressed( VK_CONTROL ) )
							|| ( keyAction.IsShift && IsKeyPressed( VK_SHIFT ) )
							)
						{

							NeedSkipThisKey = TRUE;
							if ( ( keyAction.IsAlt && IsKeyPressed( VK_MENU ) )
								|| ( keyAction.IsCtrl && IsKeyPressed( VK_CONTROL ) )
								|| ( keyAction.IsShift && IsKeyPressed( VK_SHIFT ) )
								)
							{
								if ( Msg == WM_SYSKEYDOWN )
									Msg = WM_KEYDOWN;
							}
							else
							{
								if ( IsKeyPressed( VK_MENU )
									|| IsKeyPressed( VK_CONTROL ) )
								{
									NeedSkipThisKey = FALSE;
								}
							}


							CONSOLE_Print( string( "need send key!" ) );

							SendMessageToChat( keyAction.Message.c_str( ), FALSE );
						}
					}
				}


				if ( _Msg == WM_XBUTTONDOWN
					|| _Msg == WM_MBUTTONDOWN )
				{
					Msg = _Msg;
					wParam = _wParam;
				}

			}


			if ( NeedSkipThisKey )
				return DefWindowProc( hWnd, Msg, wParam, lParam );
		}

	}

	return WarcraftRealWNDProc_ptr( hWnd, Msg, wParam, lParam );
	//}
	//__except ( g_crashRpt->SendReport( GetExceptionInformation( ) ) )
	//{
	//	::ExitProcess( 0 ); // It is better to stop the process here or else corrupted data may incomprehensibly crash it later.
	//	return false;
	//}
	//return WarcraftRealWNDProc_ptr( hWnd, _Msg, _wParam, lParam );
}

//
//typedef void( __fastcall * pSetChatMessage )( int FrameAddr, int unused, char * value );
//pSetChatMessage SetChatMessage_org;
//pSetChatMessage SetChatMessage_ptr;
//
//// Перехваченная функция установки текста в фрейм чата 
//void __fastcall SetChatMessage_my( int FrameAddr, int unused, char * value )
//{
//	TextAreaSetText_my( FrameAddr, unused, value );
//	if ( value && value[ 0 ] != '\0' && strlen( value ) > 2 )
//	{
//		string s( value );
//		replaceAll( s, ":c:", "|c" );
//		replaceAll( s, ":r:", "|r" );
//		replaceAll( s, ":n:", "|n" );
//		SetChatMessage_ptr( FrameAddr, unused, s.c_str( ) );
//	}
//}


BOOL NeedBnetSpeedUp = FALSE;

// Перехваченная функция нажятия на кнопки из фреймов(FrameDef файлов)

int __fastcall Wc3ControlMouseBeforeClickButton_my( int btnaddr, int, int unk )
{

#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	return Wc3ControlMouseBeforeClickButton_ptr( btnaddr, unk );
}

int __fastcall Wc3ControlClickButton_my( int btnaddr, int, int unk )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif

	//CONSOLE_Print( "Wc3ControlClickButton_my: " + to_string( btnaddr ) );
	if ( current_menu == Wc3Menu::GAME_LIST )
	{
		if ( btnaddr == CustomLoadGameButtonAddr
			)
		{
			Wc3MessageBox( "Not availabled at Warcis server!", 1 );
			return 0;
		}
	}

	if ( current_menu == Wc3Menu::MAIN_MENU )
	{
		if ( btnaddr == BattleNetButtonAddr
			)
		{
			CONSOLE_Print( "BattleNetButtonPressed: YES" );
			BattleNetButtonPressed = true;
			//CWar3Frame::Wc3PlaySound( "T02Narrator018" );
			//ShowAutologinWindow( );


		}
		else if ( btnaddr == ConnectButtonAddr
			)
		{



			CONSOLE_Print( "BattleNetButtonPressed: NO" );
			BattleNetButtonPressed = false;
		}

	}

	if ( current_menu == Wc3Menu::BNET_MAIN )
	{
		int LogonBtn = GetFrameItemAddress( "LogonButton", 0 );
		if ( LogonBtn == btnaddr )
		{
			/*CWar3Frame AccountNameFrame = CWar3Frame( );
			int AccountNameFrame_addr = AccountNameFrame.Load( "AccountName" );
			AccountNameFrame.SetFrameType( CFrameType::FRAMETYPE_EDITBOX );

			CWar3Frame PasswordFrame = CWar3Frame( );
			int PasswordFrame_addr = PasswordFrame.Load( "Password" );
			PasswordFrame.SetFrameType( CFrameType::FRAMETYPE_EDITBOX );

			if ( AccountNameFrame_addr &&  PasswordFrame_addr )
			{
				if ( AccountNameFrame.GetText( ) && AccountNameFrame.GetText( )[ 0 ] != '\0' )
				{
					MessageBox( 0, AccountNameFrame.GetText( ), "Account name:", 0 );
					*/if ( LastUsername.length( ) > 0 )
					{
						memset( gInfo.Username, 0, 50 );
						memcpy( gInfo.Username, LastUsername.c_str( ), LastUsername.length( ) );
					}
		/*	}
			if ( PasswordFrame.GetText( ) && PasswordFrame.GetText( )[ 0 ] != '\0' )
			{
				MessageBox( 0, AccountNameFrame.GetText( ), "Account Password:", 0 );
				*/if ( LastPassword.length( ) > 0 )
				{
					memset( gInfo.Password, 0, 50 );
					memcpy( gInfo.Password, LastPassword.c_str( ), LastPassword.length( ) > 49 ? 49 : LastPassword.length( ) );
				}
					/*	}
					}*/
		}



	}
	if ( current_menu == Wc3Menu::BNET_CHAT )
	{
		if ( btnaddr == QuickStandardGameButtonAddr )
		{
			OnlyStats = true;
			if ( TmpListMapInfos.size( ) == 0 )
				ShowMapHostMenu( );
			else
				Wc3MessageBox( "Please wait for load maplist and try again!", 1 );
			/*if ( wc3classgproxy )
			{
				CWC3 * gproxy_wc3 = ( CWC3 * )wc3classgproxy;
				gproxy_wc3->SendAHPacket( ANTIHACK_VERSION, 0x1020, SelectedMapHostType, 0, 0, 0,SelectedMapCode, SelectedMapPlayers, SelectedMapMode, SelectedGameName );
				CONSOLE_Print( "Host map #2: " + SelectedMapCode + ". Mode:" + SelectedMapMode + ". Players:" + SelectedMapPlayers + ". Game name:" + SelectedGameName +".Type:" + to_string( SelectedMapHostType ));
			}*/
		}
		else
		{
			if (
				btnaddr == StandardTeamGameButtonAddr
				)
			{
				Wc3MessageBox( "Not availabled at Warcis server!", 1 );
				return 0;
			}
			else if ( btnaddr == StandardGameButtonAddr )
			{
				OnlyStats = false;
				if ( TmpListMapInfos.size( ) == 0 )
					ShowMapHostMenu( );
				else
					Wc3MessageBox( "Please wait for load maplist and try again!", 1 );
				return 0;
			}
		}
	}




	if ( ( current_menu == Wc3Menu::BNET_CHAT || current_menu == Wc3Menu::BNET_MAIN ) && NeedBnetSpeedUp && ( btnaddr == CustomGameButtonAddr || btnaddr == CustomGameExitButtonAddr ) )
	{
		SpeedUpForTime( 800, 2.5 );
	}

	return Wc3ControlClickButton_ptr( btnaddr, unk );;
}





vector<PlayerStatSlot> PlayerStatsList;




typedef int( __stdcall  * pStorm_MemAlloc )( size_t sz );
pStorm_MemAlloc Storm_MemAlloc_org;
pStorm_MemAlloc Storm_MemAlloc_ptr;

int CurrentStatsId = 0;
int CurrentPlayerId = 0;


// перехваченная функция опирование строки (?)
//int __stdcall  Storm_MemAlloc_my(  size_t s )
//{
//	/*int retval = Storm_MemAlloc_ptr(  s );
//
//
//*/
//	
//	return retval;
//}


//sub_6F29A690 127a
typedef void( __fastcall * pSetPlayerSlotInfo )( UINT * a1, int unused, UINT * a2, unsigned __int8 *a3, int a4, unsigned int PlayerId );
pSetPlayerSlotInfo SetPlayerSlotInfo_org;
pSetPlayerSlotInfo SetPlayerSlotInfo_ptr;

// Получает текст фрейма типа "Menu"
const char * GetFrameMenuItemText( int frameaddr )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	int offset = *( int * )( frameaddr + 0x1E4 );
	if ( offset )
	{
		offset = *( int * )( offset + 0x1E4 );
		if ( offset )
		{
			offset = *( int * )( offset + 0x1EC ) ? offset + 0x1EC : 0;
			if ( offset )
			{
				const char * rettext = ( char * )*( int* )offset;
				if ( !rettext )
					return NULL;
				return rettext;
			}
		}
	}
	return "qremgoiqrmgoiqmergioqeirogqroigm";
}

int OldStatFrameValues[ 20 ];

//sub_6F155300

int GetPlayerTeam( int PlayerId )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	return *( unsigned int * )( *( unsigned int * )( GetGlobalPlayerData( ) + PlayerId * 4 + 0x58 ) + 0x278 );
}


sub_6F29A640 UpdatePlayerSlot;
std::regex getstatsregex( "\\{(.+?)\\}\\{(.+?)\\}\\{(.+?)\\}\\{(.+?)\\}\\{(.+?)\\}\\{(.+?)\\}\\{(.+?)\\}\\{(.+?)\\}\\{(.+?)\\}\\{(.+?)\\}" );


struct PlayerSlot
{
	int id;
	std::string playername;
	std::string teamname;
	int mmr;
	BOOL NeedUpdate;
};

std::vector<PlayerSlot> PlayerSlotList;

void AddNewSlot( unsigned int PlayerId, const  std::string &  PlayerName, const  std::string &  TeamName, int mmr )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	for ( auto & s : PlayerSlotList )
	{
		if ( PlayerId == s.id )
		{
			s.playername = PlayerName;
			s.teamname = TeamName;
			s.mmr = mmr;
			s.NeedUpdate = TRUE;
			//	CONSOLE_Print( "Update slot: " + to_string( PlayerId ) + ". Name:" + PlayerName + ". Team:" + TeamName );
			return;
		}
	}
	PlayerSlot tmpPlayerSlot = PlayerSlot( );
	tmpPlayerSlot.id = PlayerId;
	tmpPlayerSlot.playername = PlayerName;
	tmpPlayerSlot.teamname = TeamName;
	tmpPlayerSlot.mmr = mmr;
	tmpPlayerSlot.NeedUpdate = TRUE;
	PlayerSlotList.push_back( tmpPlayerSlot );

	//CONSOLE_Print( "Added slot: " + to_string( PlayerId ) + ". Name:" + PlayerName + ". Team:" + TeamName );
}

bool IsPlayerSlotNotEnemy( const  std::string &  PlayerName )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	std::string LocalPlayerUsername = ToLower( LastUsername );
	std::string TargetUsername = ToLower( PlayerName );

	//	CONSOLE_Print( "Check enemy:" + LocalPlayerUsername + " & " + TargetUsername );

	std::string LocalPlayerTeam = "Team -1";

	bool FoundLocalPlayer = false;
	for ( auto & s : PlayerSlotList )
	{
#ifndef  ANTIHACKNODEBUG
		AddLogFunctionCall( __FUNCSIGW__ );
#endif
		std::string CurrentUsername = ToLower( s.playername );
		if ( CurrentUsername == LocalPlayerUsername )
		{
			//CONSOLE_Print( "LocalPlayerTeam:" + s.teamname );
			LocalPlayerTeam = s.teamname;
			FoundLocalPlayer = true;
			//CONSOLE_Print( "FoundLocalPlayer" );
			break;
		}
	}

	if ( FoundLocalPlayer )
		for ( auto & s : PlayerSlotList )
		{
#ifndef  ANTIHACKNODEBUG
			AddLogFunctionCall( __FUNCSIGW__ );
#endif
			std::string CurrentUsername = ToLower( s.playername );
			if ( CurrentUsername == TargetUsername )
			{
				//CONSOLE_Print( "Target user Team:" + s.teamname );
				if ( ToLower( s.teamname ) == ToLower( LocalPlayerTeam ) )
					return true;
				return false;
			}
		}
	return false;
}

const char * GetRankPathByMMR( int mmr )
{
	if ( mmr == 0 )
		return "WarcisPictures\\Empty.blp";
	if ( mmr > 7000.0 )
		return "WarcisRankIcons\\base-icons\\master.blp";
	if ( mmr > 4000.0 )
		return "WarcisRankIcons\\base-icons\\provisional.blp";
	if ( mmr > 2500.0 )
		return "WarcisRankIcons\\base-icons\\challenger.blp";
	if ( mmr > 2450.0 )
		return "WarcisRankIcons\\tier-icons\\diamond_v.blp";
	if ( mmr > 2390.0 )
		return "WarcisRankIcons\\tier-icons\\diamond_iv.blp";
	if ( mmr > 2330.0 )
		return "WarcisRankIcons\\tier-icons\\diamond_iii.blp";
	if ( mmr > 2260.0 )
		return "WarcisRankIcons\\tier-icons\\diamond_ii.blp";
	if ( mmr > 2200.0 )
		return "WarcisRankIcons\\tier-icons\\diamond_i.blp";

	if ( mmr > 2090.0 )
		return "WarcisRankIcons\\tier-icons\\platinum_v.blp";
	if ( mmr > 2030.0 )
		return "WarcisRankIcons\\tier-icons\\platinum_iv.blp";
	if ( mmr > 1970.0 )
		return "WarcisRankIcons\\tier-icons\\platinum_iii.blp";
	if ( mmr > 1910.0 )
		return "WarcisRankIcons\\tier-icons\\platinum_ii.blp";
	if ( mmr > 1850.0 )
		return "WarcisRankIcons\\tier-icons\\platinum_i.blp";

	if ( mmr > 1740.0 )
		return "WarcisRankIcons\\tier-icons\\gold_v.blp";
	if ( mmr > 1680.0 )
		return "WarcisRankIcons\\tier-icons\\gold_iv.blp";
	if ( mmr > 1620.0 )
		return "WarcisRankIcons\\tier-icons\\gold_iii.blp";
	if ( mmr > 1560.0 )
		return "WarcisRankIcons\\tier-icons\\gold_ii.blp";
	if ( mmr > 1500.0 )
		return "WarcisRankIcons\\tier-icons\\gold_i.blp";

	if ( mmr > 1390.0 )
		return "WarcisRankIcons\\tier-icons\\silver_v.blp";
	if ( mmr > 1330.0 )
		return "WarcisRankIcons\\tier-icons\\silver_iv.blp";
	if ( mmr > 1270.0 )
		return "WarcisRankIcons\\tier-icons\\silver_iii.blp";
	if ( mmr > 1210.0 )
		return "WarcisRankIcons\\tier-icons\\silver_ii.blp";
	if ( mmr > 1150.0 )
		return "WarcisRankIcons\\tier-icons\\silver_i.blp";

	if ( mmr > 1050.0 )
		return "WarcisRankIcons\\tier-icons\\bronze_v.blp";
	if ( mmr > 1000.0 )
		return "WarcisRankIcons\\tier-icons\\bronze_iv.blp";
	if ( mmr > 950.0 )
		return "WarcisRankIcons\\tier-icons\\bronze_iii.blp";
	if ( mmr > 900.0 )
		return "WarcisRankIcons\\tier-icons\\bronze_ii.blp";
	if ( mmr > 850.0 )
		return "WarcisRankIcons\\tier-icons\\bronze_i.blp";

	return "WarcisRankIcons\\tier-icons\\diamond_i.blp";
}

const char * GetRankNameByMMR( int mmr )
{
	if ( mmr == 0 )
		return "Unnamed";
	if ( mmr > 7000.0 )
		return "MASTER";
	if ( mmr > 4000.0 )
		return "PROVISIONAL";
	if ( mmr > 2500.0 )
		return "CHALLENGER";

	if ( mmr > 2200.0 )
		return "DIAMOND";

	if ( mmr > 1850.0 )
		return "PLATINUM";

	if ( mmr > 1500.0 )
		return "GOLD";

	if ( mmr > 1150.0 )
		return "SILVER";

	if ( mmr > 850.0 )
		return "BRONZE";

	return "WarcisRankIcons\\tier-icons\\bronze_i.blpp";
}

void UpdatePlayerStatsIcons( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	std::string LocalPlayerUsername = ToLower( LastUsername );
	CONSOLE_Print( "Check :" + LocalPlayerUsername + ". Player data len:" + to_string( PlayerSlotList.size( ) ) );

	std::string LocalPlayerTeam = "Team -1";

	bool FoundLocalPlayer = false;
	for ( auto & s : PlayerSlotList )
	{
#ifndef  ANTIHACKNODEBUG
		AddLogFunctionCall( __FUNCSIGW__ );
#endif
		std::string CurrentUsername = ToLower( s.playername );
		if ( CurrentUsername == LocalPlayerUsername )
		{
			//	CONSOLE_Print( "LocalPlayerTeam:" + s.teamname );
			LocalPlayerTeam = s.teamname;
			FoundLocalPlayer = true;
			//	CONSOLE_Print( "FoundLocalPlayer" );
			break;
		}
	}

	if ( FoundLocalPlayer )
	{
		for ( auto & s : PlayerSlotList )
		{
#ifndef  ANTIHACKNODEBUG
			AddLogFunctionCall( __FUNCSIGW__ );
#endif
			//if ( s.NeedUpdate )
			{
				s.NeedUpdate = FALSE;
				if ( ToLower( s.teamname ) == ToLower( LocalPlayerTeam ) )
				{
					CWar3Frame PlayerRankIcon = CWar3Frame( );
					//CONSOLE_Print( "Set icon rank! For player" + s.playername + " id:" + to_string( s.id ) );
					if ( PlayerRankIcon.Load( "PlayerIcon", s.id ) )
					{
						if ( s.mmr == 0 )
						{
							PlayerRankIcon.SetFrameType( CFrameType::FRAMETYPE_BUTTON );
							PlayerRankIcon.SetText( "No stats" );
							if ( PlayerRankIcon.CWar3FrameFromAddress( PlayerRankIcon.GetFrameBackdropAddress( CFrameBackdropType::ControlBackdrop ) ) )
							{
								PlayerRankIcon.SetFrameType( CFrameType::FRAMETYPE_BACKDROP );
								PlayerRankIcon.SetTexture( "WarcisPictures\\Empty.blp" );
							}
						}
						else
						{
							PlayerRankIcon.SetFrameType( CFrameType::FRAMETYPE_BUTTON );
							PlayerRankIcon.SetText( ( "Rank " + string( GetRankNameByMMR( s.mmr ) ) + ". MMR: " + to_string( s.mmr ) ).c_str( ) );
							if ( PlayerRankIcon.CWar3FrameFromAddress( PlayerRankIcon.GetFrameBackdropAddress( CFrameBackdropType::ControlBackdrop ) ) )
							{
								PlayerRankIcon.SetFrameType( CFrameType::FRAMETYPE_BACKDROP );
								PlayerRankIcon.SetTexture( GetRankPathByMMR( s.mmr ) );
							}
						}
					}
				}
				else
				{
					CWar3Frame PlayerRankIcon = CWar3Frame( );
					//CONSOLE_Print( "Set no rank! For player" + s.playername + " id:" + to_string( s.id ) );
					if ( PlayerRankIcon.Load( "PlayerIcon", s.id ) )
					{
						PlayerRankIcon.SetFrameType( CFrameType::FRAMETYPE_BUTTON );
						PlayerRankIcon.SetText( "No stats" );
						if ( PlayerRankIcon.CWar3FrameFromAddress( PlayerRankIcon.GetFrameBackdropAddress( CFrameBackdropType::ControlBackdrop ) ) )
						{
							PlayerRankIcon.SetFrameType( CFrameType::FRAMETYPE_BACKDROP );
							PlayerRankIcon.SetTexture( "WarcisPictures\\Empty.blp" );
						}
					}
				}
			}
		}
	}
}

// Перехваченная функция установок PlayerSlot
void __fastcall SetPlayerSlotInfo_my( UINT * a1, int unused, UINT * a2, unsigned __int8 *a3, int a4, unsigned int PlayerId )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	CONSOLE_Print( "SetPlayerSlotInfo_my" );
	//int oldecx = unused;

	if ( IsGame( ) )
	{
		if ( current_menu != Wc3Menu::GAME_GAME )
		{
			current_menu = Wc3Menu::GAME_GAME;
			MakeGreenFix( );
		}

		SetPlayerSlotInfo_ptr( a1, unused, a2, a3, a4, PlayerId );
		return;
	}
	else
	{
		current_menu = Wc3Menu::GAME_LOBBY;
	}
	SetPlayerSlotInfo_ptr( a1, unused, a2, a3, a4, PlayerId );
	int UsernameFrame = GetFrameItemAddress( "NameMenu", PlayerId );
	LobbyLeaveButtonAddr = GetFrameItemAddress( "CancelButton", 0 );
	/*if ( LobbyLeaveButtonAddr )
	{
		Wc3ControlClickButton_ptr( LobbyLeaveButtonAddr, 1 );
		SpeedUpForTime( 2500, 1.3 );
	}*/


	CWar3Frame cStatsFrame = CWar3Frame( );
	int UserStatsFrame = cStatsFrame.Load( "STATSTEXT", PlayerId );
	//	char test[ 200 ];
		//sprintf_s( test, 200, "StatsMenuAddr:%X", UserStatsFrame );
		//CONSOLE_Print( test );

	CONSOLE_Print( "SetPlayerSlotInfo_my 2" );
	if ( UserStatsFrame && UsernameFrame )
	{
		cStatsFrame.SetFrameType( CFrameType::FRAMETYPE_POPUPMENU );
		cStatsFrame.Enable( false );


		const char * PlayerName = GetFrameMenuItemText( UsernameFrame );

		if ( !PlayerName )
			return;

		//MessageBox( 0, GetPlayerName( PlayerId, 1 ), PlayerName, 0 );
		//char playerTeam[ 100 ];
		//sprintf_s( playerTeam, "Team id:%u\n Local Player:%u", GetPlayerTeam( PlayerId ), GetLocalPlayerId( ) );

		//MessageBox( 0, playerTeam, "", 0 );

		bool found = false;
		bool foundplayer = false;
		std::string DefaultPlayerName = "";
		int mmr = 0;


		CONSOLE_Print( "SetPlayerSlotInfo_my 3" );


		for ( PlayerStatSlot & pslot : PlayerStatsList )
		{
			bool FoundNewName = _stricmp( pslot.PlayerNewName, PlayerName ) == 0;
			bool FoundOldName = _stricmp( pslot.PlayerName, PlayerName ) == 0;

			//if ( !pslot.NeedUpdate )
			//	continue;
			pslot.NeedUpdate = false;

			if ( FoundOldName || FoundNewName )
			{
				foundplayer = true;
				DefaultPlayerName = pslot.PlayerName;
			}

			if ( FoundOldName || FoundNewName )
			{
				if ( !FoundNewName )
					SetFrameText( *( int* )( *( int* )( UsernameFrame + 0x1E4 ) + 0x1E4 ), pslot.PlayerNewName );
				if ( _stricmp( pslot.PlayerStats.c_str( ), "no_stats" ) == 0 )
				{

				}
				else
				{
					found = true;
					if ( !( *( unsigned int * )( UserStatsFrame + 0x1D4 ) & 1 ) )
					{
						*( unsigned int * )( UserStatsFrame + 0x1D4 ) += 1;
					}

					if ( !( *( unsigned int * )( UserStatsFrame + 0xB0 ) & 4 ) )
					{
						*( unsigned int * )( UserStatsFrame + 0xB0 ) += 4;
					}

					cStatsFrame.Update( );

					std::smatch matchStatsLine;
					std::string PlayerOldStats = pslot.PlayerStats;
					std::string PlayerNewStats = LineForStats;

					if ( std::regex_match( PlayerOldStats, matchStatsLine, getstatsregex ) )
					{
						mmr = atoi( matchStatsLine[ 1 ].str( ).c_str( ) );

						replaceAll( PlayerNewStats, "{MMR}", "NO" );

						CONSOLE_Print( "SetStats:" + string( pslot.PlayerName ? pslot.PlayerName : "" ) );
						//CWar3Frame PlayerRankIcon = CWar3Frame( );
						//if ( PlayerRankIcon.Load( "PlayerIcon", PlayerId ) )
						//{
						//	PlayerRankIcon.SetFrameType( CFrameType::FRAMETYPE_BUTTON );
						//	if ( PlayerRankIcon.CWar3FrameFromAddress( PlayerRankIcon.GetFrameBackdropAddress( CFrameBackdropType::ControlBackdrop ) ) )
						//	{
						//		PlayerRankIcon.SetFrameType( CFrameType::FRAMETYPE_BACKDROP );
						//		PlayerRankIcon.SetTexture( "WarcisRankIcons\\tier-icons\\diamond_ii.blp" );
						//	}
						//}


						replaceAll( PlayerNewStats, "{WINS}", matchStatsLine[ 2 ].str( ) );




						replaceAll( PlayerNewStats, "{LOSES}", matchStatsLine[ 3 ].str( ) );
						replaceAll( PlayerNewStats, "{LEAVES}", matchStatsLine[ 4 ].str( ) );
						replaceAll( PlayerNewStats, "{STREAK}", matchStatsLine[ 5 ].str( ) );
						replaceAll( PlayerNewStats, "{MINSTREAK}", matchStatsLine[ 6 ].str( ) );
						replaceAll( PlayerNewStats, "{MAXSTREAK}", matchStatsLine[ 7 ].str( ) );
						replaceAll( PlayerNewStats, "{KILLS}", matchStatsLine[ 8 ].str( ) );
						replaceAll( PlayerNewStats, "{DEATHS}", matchStatsLine[ 9 ].str( ) );
						replaceAll( PlayerNewStats, "{ASSISTS}", matchStatsLine[ 10 ].str( ) );

						pslot.PlayerNewStats = PlayerNewStats;

						cStatsFrame.SetText( pslot.PlayerNewStats.c_str( ) );

					}
					else
					{

					}
					cStatsFrame.Enable( false );

				}
				break;
			}
		}

		CWar3Frame teamName = CWar3Frame( );
		int TeamTextAddress = teamName.Load( "TeamButtonTitle", PlayerId );

		if ( TeamTextAddress )
		{
			teamName.SetFrameType( CFrameType::FRAMETYPE_ITEM );
			const char * TeamNameText = teamName.GetText( );

			if ( foundplayer )
			{
				AddNewSlot( PlayerId, DefaultPlayerName, TeamNameText, mmr );
			}
			else
			{
				AddNewSlot( PlayerId, PlayerName, TeamNameText, mmr );
			}

		}
		if ( !found )
		{

			if ( ( *( unsigned int * )( UserStatsFrame + 0x1D4 ) & 1 ) )
			{
				*( unsigned int * )( UserStatsFrame + 0x1D4 ) -= 1;
			}

			if ( ( *( unsigned int * )( UserStatsFrame + 0xB0 ) & 4 ) )
			{
				*( unsigned int * )( UserStatsFrame + 0xB0 ) -= 4;
			}
			/*if ( PlayerName[ 0 ] != '|' )
			{

				SetFrameText( *( int* )( *( int* )( UsernameFrame + 0x1E4 ) + 0x1E4 ), ( ( string )( "|cFFFF0000NoStats:|r" ) + PlayerName ).c_str( ) );

			}*/

			cStatsFrame.Update( );
		}
	}

	if ( current_menu == Wc3Menu::GAME_LOBBY )
	{
		UpdatePlayerStatsIcons( );
	}
	//__asm mov ecx, oldecx;
}

// Добавление статистики игрока в список
void AddNewPlayerStats( const  std::string & UserName, const  std::string & NewUserName, const  std::string & StatsString1,
	const  std::string &  StatsString2, const  std::string &  StatsString3, unsigned int LobbyNickColor, unsigned int ChatNickColor, unsigned int ChatTextColor )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	CONSOLE_Print( "[Warcis_Rec] new stats found!: " + UserName + " -> "
		+ NewUserName + " -> "
		+ StatsString1 + " -> "
		+ StatsString2 + " -> "
		+ StatsString3 );

	bool FoundPlayer = false;
	for ( auto & pstr : PlayerStatsList )
	{
		if ( _stricmp( pstr.PlayerName, UserName.c_str( ) ) == 0 )
		{
			sprintf_s( pstr.PlayerNewName, "%s", NewUserName.c_str( ) );
			pstr.PlayerStats = StatsString1;
			pstr.PlayerStats2 = StatsString2;
			pstr.PlayerStats3 = StatsString3;
			pstr.LobbyNickNameColor = LobbyNickColor;
			pstr.ChatNickNameColor = ChatNickColor;
			pstr.ChatTextColor = ChatTextColor;
			pstr.NeedUpdate = true;
			FoundPlayer = true;
			break;
		}
	}

	if ( !FoundPlayer )
	{
		PlayerStatSlot pslot;
		sprintf_s( pslot.PlayerName, "%s", UserName.c_str( ) );
		sprintf_s( pslot.PlayerNewName, "%s", NewUserName.c_str( ) );
		pslot.PlayerStats = StatsString1;
		pslot.PlayerStats2 = StatsString2;
		pslot.PlayerStats3 = StatsString3;
		pslot.NeedUpdate = false;
		pslot.LobbyNickNameColor = LobbyNickColor;
		pslot.ChatNickNameColor = ChatNickColor;
		pslot.ChatTextColor = ChatTextColor;
		PlayerStatsList.push_back( pslot );
	}

}


pGame_Wc3MessageBox Game_Wc3MessageBox = NULL;
int __stdcall Wc3MessageBox( const char * message, int type )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	if ( Game_Wc3MessageBox )
		Game_Wc3MessageBox( type, message, 0, 0, 0, 0, 0 );
	return TRUE;
}


void __stdcall UpdateStatsLine( const char * line )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	LineForStats = line;

	for ( PlayerStatSlot & pslot : PlayerStatsList )
	{
		pslot.NeedUpdate = true;
	}
}


DWORD WarTickLatest = GetTickCount( ) + 12000;
DWORD WarTickLatest2 = GetTickCount( ) + 12000;

void FollowTimer( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	FollowEventLimitTime -= 250;


	if ( current_menu == Wc3Menu::GAME_LIST )
	{
		if ( !GetFrameItemAddress( "JoinGameNameEditBox", 0 ) )
		{

			PlayerStatsList.clear( );
			current_menu = Wc3Menu::BNET_CHAT;
			LobbyLeaveButtonAddr = 0;
			CustomGameNameEditBox = 0;
			CustomGameExitButtonAddr = 0;
			CustomGameNameButtonAddr = 0;
			CustomLoadGameButtonAddr = 0;
			InBattleNet = TRUE;
			CustomGameButtonAddr = GetFrameItemAddress( "CustomGameButton", 0 );
			//CONSOLE_Print( "BattleNetChatPanel_my#2" );
		}
	}

	if ( FollowEventLimitTime < 0 )
	{
		if ( FollowStatus == 2 || FollowStatus == 20 )
		{
			if ( FollowStatus == 2 )
			{
				FollowStatus = 20;
				FollowEventLimitTime = 300;
				return;
			}


			if ( current_menu == Wc3Menu::GAME_LOBBY )
			{
				LobbyLeaveButtonAddr = GetFrameItemAddress( "CancelButton", 0 );
				if ( LobbyLeaveButtonAddr )
				{
					Wc3ControlClickButton_ptr( LobbyLeaveButtonAddr, 1 );
					FollowEventLimitTime = 1000;
					SpeedUpForTime( 1500, 2.5 );
					FollowStatus = 0;
				}
			}
			if ( current_menu == Wc3Menu::GAME_LIST )
			{
				CustomGameExitButtonAddr = GetFrameItemAddress( "CancelButton", 0 );
				if ( CustomGameExitButtonAddr )
				{
					Wc3ControlClickButton_ptr( CustomGameExitButtonAddr, 1 );
					FollowEventLimitTime = 1000;
					SpeedUpForTime( 1500, 2.5 );
					FollowStatus = 0;
				}
			}

		}

		if ( ( FollowStatus == 1 || FollowStatus == 10 ) && !FollowGameName.empty( ) )
		{

			if ( FollowStatus == 1 )
			{
				FollowStatus = 10;
				FollowEventLimitTime = 300;
				return;
			}

			if ( current_menu == Wc3Menu::GAME_LOBBY )
			{
				LobbyLeaveButtonAddr = GetFrameItemAddress( "CancelButton", 0 );
				if ( LobbyLeaveButtonAddr )
				{
					if ( ToLower( LatestRequestGame ) != ToLower( FollowGameName ) )
					{
						Wc3ControlClickButton_ptr( LobbyLeaveButtonAddr, 1 );
						SpeedUpForTime( 1500, 2.0 );
						FollowEventLimitTime = 1000;
					}
				}
			}
			else if ( current_menu == Wc3Menu::BNET_CHAT )
			{
				CustomGameButtonAddr = GetFrameItemAddress( "CustomGameButton", 0 );
				if ( CustomGameButtonAddr )
				{
					int CancelAnonSearch = GetFrameItemAddress( "AnonSearchCancelButton", 0 );
					if ( CancelAnonSearch )
					{
						Wc3ControlClickButton_ptr( CancelAnonSearch, 1 );
					}
					Wc3ControlClickButton_ptr( CustomGameButtonAddr, 1 );
					FollowEventLimitTime = 1000;
					SpeedUpForTime( 1500, 2.0 );
				}
			}
			else if ( current_menu == Wc3Menu::GAME_LIST )
			{
				CustomGameNameEditBox = GetFrameItemAddress( "JoinGameNameEditBox", 0 );
				CustomGameNameButtonAddr = GetFrameItemAddress( "JoinGameButton", 0 );

				if ( GetFrameItemAddress( "DialogButtonOK", 0 ) )
				{
					Wc3ControlClickButton_ptr( GetFrameItemAddress( "DialogButtonOK", 0 ), 1 );
				}
				else if ( CustomGameNameEditBox &&  CustomGameNameButtonAddr )
				{
					SetEditBoxText( CustomGameNameEditBox, FollowGameName.c_str( ), 1 );
					Wc3ControlClickButton_ptr( CustomGameNameButtonAddr, 1 );
				}

				if ( FollowTryCount > 0 )
					FollowTryCount--;
				else
					FollowStatus = 0;
				FollowEventLimitTime = 250;
			}


		}

		if ( FollowStatus != 0 && FollowEventLimitTime <= 250 )
			FollowEventLimitTime = 251;
	}



}

// Таймер для работы с функциями игры без необходимости установки TLS
void SimpleWc3Work( )
{
	//return;
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	if ( !IsGame( ) )
	{
		if ( current_menu == Wc3Menu::GAME_GAME )
		{
			current_menu = Wc3Menu::GAME_NOGAME;
		}
	}

	if ( ( GetTickCount( ) - WarTickLatest ) > 1000 )
	{
		WarTickLatest = GetTickCount( );
		/*int offs1 = *( int* )( GameDll + 0xBB8790 );
		if ( offs1 )
		{
			offs1 = *( int* )( offs1 + 0x12C );
			if ( offs1 )
			{
				for ( int i = 0; i < 12; i++ )
				{
					UpdatePlayerSlot( offs1, i );
				}
			}
		}*/
		//[Game.dll + BB8790] + 12C

		if ( !IsGame( ) )
		{


			for ( int PlayerId = 0; PlayerId < 14; PlayerId++ )
			{
				int UsernameFrame = GetFrameItemAddress( "NameMenu", PlayerId );
				CWar3Frame cStatsFrame = CWar3Frame( );
				int UserStatsFrame = cStatsFrame.Load( "STATSTEXT", PlayerId );

				bool found = false;


				if ( UsernameFrame &&
					UserStatsFrame &&
					*( int* )( UsernameFrame + 0x1E4 ) &&
					*( int* )( *( int* )( UsernameFrame + 0x1E4 ) + 0x1E4 ) && GetFrameMenuItemText( UsernameFrame ) )
				{
					const char * PlayerName = GetFrameMenuItemText( UsernameFrame );
					cStatsFrame.SetFrameType( CFrameType::FRAMETYPE_POPUPMENU );
					cStatsFrame.Enable( false );


					for ( PlayerStatSlot pslot : PlayerStatsList )
					{
						bool FoundNewName = _stricmp( pslot.PlayerNewName, PlayerName ) == 0;
						bool FoundOldName = _stricmp( pslot.PlayerName, PlayerName ) == 0;

						if ( !pslot.NeedUpdate )
							continue;
						pslot.NeedUpdate = false;

						//if ( FoundOldName || FoundNewName )
						//{
						//	foundplayer = true;
						//	DefaultPlayerName = pslot.PlayerName;
						//}

						if ( FoundOldName || FoundNewName )
						{
							/*if ( pslot.NeedUpdate )
							{*/
							std::smatch matchStatsLine;
							std::string PlayerOldStats = pslot.PlayerStats;
							std::string PlayerNewStats = LineForStats;

							if ( std::regex_match( PlayerOldStats, matchStatsLine, getstatsregex ) )
							{
								replaceAll( PlayerNewStats, "{MMR}", matchStatsLine[ 1 ].str( ) );
								replaceAll( PlayerNewStats, "{WINS}", matchStatsLine[ 2 ].str( ) );
								replaceAll( PlayerNewStats, "{LOSES}", matchStatsLine[ 3 ].str( ) );
								replaceAll( PlayerNewStats, "{LEAVES}", matchStatsLine[ 4 ].str( ) );
								replaceAll( PlayerNewStats, "{STREAK}", matchStatsLine[ 5 ].str( ) );
								replaceAll( PlayerNewStats, "{MINSTREAK}", matchStatsLine[ 6 ].str( ) );
								replaceAll( PlayerNewStats, "{MAXSTREAK}", matchStatsLine[ 7 ].str( ) );
								replaceAll( PlayerNewStats, "{KILLS}", matchStatsLine[ 8 ].str( ) );
								replaceAll( PlayerNewStats, "{DEATHS}", matchStatsLine[ 9 ].str( ) );
								replaceAll( PlayerNewStats, "{ASSISTS}", matchStatsLine[ 10 ].str( ) );

								pslot.PlayerNewStats = PlayerNewStats;
							}
							//	}
							pslot.NeedUpdate = false;
							found = true;
							SetFrameText( *( int* )( *( int* )( UsernameFrame + 0x1E4 ) + 0x1E4 ), pslot.PlayerNewName );

							if ( _stricmp( pslot.PlayerStats.c_str( ), "no_stats" ) == 0 )
							{
								continue;
							}

							if ( !( *( unsigned int * )( UserStatsFrame + 0x1D4 ) & 1 ) )
							{
								*( unsigned int * )( UserStatsFrame + 0x1D4 ) += 1;
							}

							if ( !( *( unsigned int * )( UserStatsFrame + 0xB0 ) & 4 ) )
							{
								*( unsigned int * )( UserStatsFrame + 0xB0 ) += 4;
							}


							cStatsFrame.Update( );
							cStatsFrame.SetText( pslot.PlayerNewStats.c_str( ) );
							cStatsFrame.Enable( false );

							//UpdateFrameFlags( UserStatsFrame, 0 );
							//SetFrameText( *( int* )( *( int* )( UserStatsFrame + 0x1E4 ) + 0x1E4 ), pslot.PlayerNewStats.c_str( ) );

						}
					}
					if ( !found )
					{

						/*if ( ( *( unsigned int * )( UserStatsFrame + 0x1D4 ) & 1 ) )
						{
							*( unsigned int * )( UserStatsFrame + 0x1D4 ) -= 1;
						}

						if ( ( *( unsigned int * )( UserStatsFrame + 0xB0 ) & 4 ) )
						{
							*( unsigned int * )( UserStatsFrame + 0xB0 ) -= 4;
						}*/
						/*if ( PlayerName[ 0 ] != '|' )
						{

						SetFrameText( *( int* )( *( int* )( UsernameFrame + 0x1E4 ) + 0x1E4 ), ( ( string )( "|cFFFF0000NoStats:|r" ) + PlayerName ).c_str( ) );

						}*/

						cStatsFrame.Update( );

					}

				}

			}
		}
		else
		{
			if ( current_menu != Wc3Menu::GAME_GAME )
			{
				current_menu = Wc3Menu::GAME_GAME;
				MakeGreenFix( );
			}
		}
	}

}



#ifdef _MSC_VER
#pragma pack(push, 1)
#endif // _MSC_VER


#ifdef _MSC_VER
#pragma pack(pop)
#endif // _MSC_VER

typedef void( __cdecl * pPreloadGenEnd )( RCString* buf );
pPreloadGenEnd PreloadGenEnd_org;
pPreloadGenEnd PreloadGenEnd_ptr;


typedef void( __cdecl * pPreloader )( RCString* buf );
pPreloader Preloader_org;
pPreloader Preloader_ptr;


typedef int( __thiscall * pSavePreloadFile )( int val1, const char * filepath );
pSavePreloadFile SavePreloadFile_org;

int MapNameOffset1 = 0;
int MapNameOffset2 = 0;


const char* jStringAddrToStr( RCString* t ) {
	return t->stringRep->text;
}

typedef void( __fastcall * p_setString )( RCString* s, int unused, const char * str );
p_setString setString;

//void setString( RCString* t, const char* str1 ) {
//	int RCString_SETSTRING = 0x4C5CF0; // 126a
//	__asm
//	{
//		mov ecx, t;
//		push str1;
//		call RCString_SETSTRING;
//	}
//}


int GetWc3Class0( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	return *( int* )( GameDll + 0xAB675C ); // 127a 0xBE44B8
}

// Защита от Preload вирусов
void __cdecl Preloader_my( RCString * jString )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	//CONSOLE_Print( string( "Preload addr:" ) + to_string( ( int )jString ) );
	const char * filename = jStringAddrToStr( jString );
	if ( filename && filename[ 0 ] != '\0' )
	{
		CONSOLE_Print( string( "Preload file:" ) + filename );
		string filename_no_extension = fs::path( filename ).stem( ).string( );

		if ( filename_no_extension.length( ) > 0 )
		{
			for ( char & c : filename_no_extension )
			{
				// Оставить только цифры и буквы в пути
				if ( !isalnum( c ) )
				{
					c = '_';
				}
			}
			int addr1 = GetWc3Class0( );
			int offset1 = *( int* )MapNameOffset1;
			if ( offset1 > 0 )
			{
				string BuildPreloadFilePath = "CustomMapData\\";
				BuildPreloadFilePath += fs::path( ( const char * )( offset1 + MapNameOffset2 ) ).stem( ).string( );
				CreateDirectory( BuildPreloadFilePath.c_str( ), 0 );
				BuildPreloadFilePath += "\\";
				BuildPreloadFilePath += filename_no_extension + ".pld";
				setString( jString, 0, BuildPreloadFilePath.c_str( ) );
				CONSOLE_Print( "Preloader:" + BuildPreloadFilePath );
				Preloader_ptr( jString );
				CONSOLE_Print( "OK" );
			}
		}
	}
}

// Защита от Preload вирусов
void __cdecl PreloadGenEnd_my( RCString * jString )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	//CONSOLE_Print( string( "Preload gen addr:" ) + to_string( ( int )jString ) );
	const char * filename = jStringAddrToStr( jString );
	if ( filename && filename[ 0 ] != '\0' )
	{
		CONSOLE_Print( string( "Preload gen file:" ) + filename );

		//получить имя файла без расширения
		string filename_no_extension = fs::path( filename ).stem( ).string( );

		if ( filename_no_extension.length( ) > 0 )
		{
			for ( char & c : filename_no_extension )
			{
				// Оставить только цифры и буквы в пути
				if ( !isalnum( c ) )
				{
					c = '_';
				}
			}

			int addr1 = GetWc3Class0( );
			int offset1 = *( int* )MapNameOffset1;
			if ( offset1 > 0 )
			{
				string BuildPreloadFilePath = "CustomMapData\\";
				BuildPreloadFilePath += fs::path( ( const char * )( offset1 + MapNameOffset2 ) ).stem( ).string( );
				CreateDirectory( BuildPreloadFilePath.c_str( ), 0 );
				BuildPreloadFilePath += "\\";
				BuildPreloadFilePath += filename_no_extension + ".pld";
				CONSOLE_Print( "PreloadGenEnd:" + BuildPreloadFilePath );
				SavePreloadFile_org( addr1, BuildPreloadFilePath.c_str( ) );
				CONSOLE_Print( "OK" );
			}
		}
	}
}

typedef signed int( __cdecl * ijlInit_p )( );
ijlInit_p ijlInit_org;
ijlInit_p ijlInit_ptr;

bool ijInitCalled = false;
int retval = 0;
signed int __cdecl ijlInit_my( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	if ( !ijInitCalled )
	{
		retval = ijlInit_ptr( );
		ijInitCalled = true;
	}
	else
	{

	}
	return retval;
}

BOOL NeedUseNewD3dMode = FALSE;


typedef int( __thiscall * pWc3Error )( int error );
pWc3Error Wc3Error_org;
pWc3Error Wc3Error_ptr;/*sub_6F0068C0*/

int __fastcall Wc3Error_my( int error, int unused )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	int retaddr = ( int )_ReturnAddress( );
	char tmpd[ 100 ];
	sprintf_s( tmpd, "Error:%i RetAddr:Game.dll+0x%X", error, ( retaddr - GameDll ) );
	CONSOLE_Print( tmpd );
	return Wc3Error_ptr( error );
}

typedef int( *IsSomeOk )( );
IsSomeOk IsSomeOk_org;
IsSomeOk IsSomeOk_ptr;

int IsSomeOk_my( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	*( int * )( 0xA4F6EC + GameDll ) = 1;
	return 1;
}

typedef void( __cdecl * WarcraftInit )( );
WarcraftInit WarcraftInit_org;
WarcraftInit WarcraftInit_ptr;

void __cdecl WarcraftInit_my( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	WarcraftInit_ptr( );
	CWar3Frame::LoadFrameDefFiles( "WarcisFrames.txt" );

}


typedef void * ( __cdecl * my_memmove_func )( unsigned char * dst, unsigned char * src, size_t size );
my_memmove_func memmove_org;
my_memmove_func memmove_ptr;
my_memmove_func memcpy_org;
my_memmove_func memcpy_ptr;
__declspec( dllexport ) void * __cdecl my_memmove_my( unsigned char * dst, unsigned char  * src, size_t size )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	return std::copy( src, src + size, dst );
}


void * __cdecl my_memcpy_my( unsigned char * dst, unsigned char  * src, size_t size )
{
	return std::move( src, src + size, dst );
}

typedef void * ( __cdecl * my_memset_func )( unsigned char * dst, int val, size_t size );
my_memset_func memset_org;
my_memset_func memset_ptr;
void * __cdecl my_memset_my( unsigned char * dst, int val, size_t size )
{
	std::fill( dst, dst + size, val );
	return dst;
}

typedef LPVOID( WINAPI * pVirtualAlloc )( LPVOID addr, SIZE_T size, DWORD falloctype, DWORD fprotect );
pVirtualAlloc VirtualAlloc_org;
pVirtualAlloc VirtualAlloc_ptr;



LPVOID WINAPI VirtualAlloc_my( LPVOID addr, SIZE_T size, DWORD falloctype, DWORD fprotect )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	if ( falloctype == MEM_RESERVE && fprotect == PAGE_READWRITE )
		fprotect += MEM_PHYSICAL;

	//CONSOLE_Print( "Virtual alloc" );
	LPVOID retval = VirtualAlloc_ptr( addr, size, falloctype, fprotect );
	if ( fprotect & PAGE_NOACCESS )
		return retval;

	//CONSOLE_Print( "Virtual ok" );
	VirtualLock( addr, size );
	//CONSOLE_Print( "Virtual ok 2" );
	return retval;
}

typedef BOOL( WINAPI * pVirtualFree )( LPVOID addr, SIZE_T size, DWORD flags );
pVirtualFree VirtualFree_org;
pVirtualFree VirtualFree_ptr;
BOOL WINAPI VirtualFree_my( LPVOID addr, SIZE_T size, DWORD flags )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	//CONSOLE_Print( "Virtual free" );
	VirtualUnlock( addr, size );
	//CONSOLE_Print( "Virtual free ok" );
	BOOL retval = VirtualFree_ptr( addr, size, flags );
	//	CONSOLE_Print( "Virtual free ok 2" );
	return retval;
}
#define PAGE_EXECUTE_FLAGS \
    (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY)



typedef NTSTATUS( NTAPI* NtProtectVirtualMemory_t )( HANDLE, PVOID, PSIZE_T, ULONG, PULONG );
NtProtectVirtualMemory_t NtProtectVirtualMemory_ptr;
NtProtectVirtualMemory_t NtProtectVirtualMemory_org;

NTSTATUS
NTAPI
NtProtectVirtualMemory_my(
	IN      HANDLE      ProcessHandle,
	IN OUT  PVOID       *BaseAddress,
	IN OUT  PSIZE_T     NumberOfBytesToProtect,
	IN      ULONG       NewAccessProtection,
	OUT     PULONG      OldAccessProtection )
{
	if ( GetFoundWhiteListMapValue( ) == 0 )
	{
		/*if ( GetModuleFromAddress( ( int )*BaseAddress == GameDll ) )
		{
			FoundModifiedMemoryCode = true;
		}*/
	}

	return NtProtectVirtualMemory_ptr( ProcessHandle, BaseAddress, NumberOfBytesToProtect, NewAccessProtection, OldAccessProtection );
}


BOOL IsExecutableAddress2( LPVOID pAddress )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	DWORD oldprot, oldprot2;
	VirtualProtect( pAddress, 4, PAGE_EXECUTE_READWRITE, &oldprot );
	VirtualProtect( pAddress, 4, oldprot, &oldprot2 );
	return oldprot & PAGE_EXECUTE;
}
//-------------------------------------------------------------------------
BOOL IsExecutableAddress( LPVOID pAddress )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	MEMORY_BASIC_INFORMATION mi;
	VirtualQuery( pAddress, &mi, sizeof( mi ) );

	return mi.Protect & PAGE_EXECUTE_FLAGS;
}



typedef float( __fastcall * pGetCameraHeight/*sub_6F3019A0*/ )( unsigned int a1 );
pGetCameraHeight GetCameraHeight_org;
pGetCameraHeight GetCameraHeight_ptr;

float cameraoffset = 0;


void IncreaseCameraOffset( float offset )
{
	if ( cameraoffset > 3000 )
		return;
	cameraoffset += offset;
}

void DecreaseCameraOffset( float offset )
{
	if ( cameraoffset < -1000 )
		return;
	cameraoffset -= offset;
}

void ResetCameraOffset( )
{
	cameraoffset = 0;
}

float __fastcall GetCameraHeight_my( unsigned int a1 )
{
	return GetCameraHeight_ptr( a1 ) + cameraoffset;
}


typedef  int( __fastcall * sub_6F689FE0 )( int a1, int a2, int a3 );
sub_6F689FE0 sub_6F689FE0_org;
sub_6F689FE0 sub_6F689FE0_ptr;

int lastretval = 0;

int __fastcall sub_6F689FE0_my( int a1, int a2, int a3 )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	int retval = sub_6F689FE0_ptr( a1, a2, a3 );
	if ( retval == 40 )
		return lastretval;
	return retval;
}

void PrintNeededAddrs( )
{
	CONSOLE_Print( "MH HELPER:" );
	PrintMemoryValueToLog( GameDll + 0x3563E8 );
	PrintMemoryValueToLog( GameDll + 0x38B602 );
	PrintMemoryValueToLog( GameDll + 0x425C48 );
	PrintMemoryValueToLog( GameDll + 0x424C7C );
	PrintMemoryValueToLog( GameDll + 0x35FA2B );
	PrintMemoryValueToLog( GameDll + 0x3A15B2 );
	PrintMemoryValueToLog( GameDll + 0x42554F );
	//PrintMemoryValueToLog( GameDll + 0x425BCB );
	PrintMemoryValueToLog( GameDll + 0x28DF9B );
}



int _BarVTable;
int UnitVtable;
int ConvertHandle( int handleid )
{
	int offset = GetGlobalPlayerData( );
	if ( offset && handleid >= 0x100000 )
	{
		offset = *( int* )( offset + 28 );
		if ( offset )
		{
			offset = *( int* )( offset + 412 );
			if ( offset )
			{
				offset = *( int* )( offset + handleid * 12 - 0xBFFFFC );
				return offset;
			}
		}
	}
	return 0;
}

typedef int( __cdecl * pPlayer )( int number );
pPlayer _Player;

int __stdcall Player( int number )
{
	if ( number < 0 || number > 15 )
		return _Player( 0 );
	return _Player( number );
}
int IsPlayerEnemyOffset;

int ItemVtable;
BOOL IsPlayerEnemy( int hPlayer1, int hPlayer2 )
{
	return ( ( ( ( IsPlayerEnemy_org )( GameDll + IsPlayerEnemyOffset ) )( hPlayer1, hPlayer2 ) ) );
}
pGetHeroInt GetHeroInt;




int pGameClass1;
_GetUnitFloatStat GetUnitFloatState;


typedef int( __fastcall * pMinimapClickEvent/*sub_6F347C90*/ )( int pClass, int unused, int ClickData );

pMinimapClickEvent MinimapClickEvent_org;
pMinimapClickEvent MinimapClickEvent_ptr;


int __fastcall MinimapClickEvent_my/*sub_6F347C90*/( int pClass, int unused, int ClickData )
{
	if ( ClickData )
	{
		// 1 Left 2 Middle 4 Right
		int ClickType = *( int* )( ClickData + 0x14 );
		int ClickType2 = *( int* )( ClickData + 0x18 );
		//char testdata[ 256 ];
		//sprintf_s( testdata, 256, "ClickDataAddr: %X. ClickType: %X. ClickType2: %X", ClickData, ClickType, ClickType2 );
		if ( ClickType == 4 && gInfo.MinimapRightClickWithAlt )
		{


			if ( !IsKeyPressed( VK_LMENU ) )
			{
				return 0;
			}
		}

	}
	return MinimapClickEvent_ptr( pClass, unused, ClickData );
}

// Инициализация CustomFeatures 
void CustomFeaturesInitialize( const char * launcherpath )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	//if ( IsKeyPressed( '1' ) )
	//	return;
//	PrintNeededAddrs( );
	PBYTE TmpStrPath = 0;
	PBYTE GameDllName = 0;
	DWORD oldprotect;



	const char * servername = "WarcisServer";
	const char * d3d8libname = "d3d8.fix";

	CONSOLE_Print( "CustomFeaturesInitialize" );
	bool FoundServer = false;
	while ( TmpStrPath = FindSignature( ( LPBYTE )( GameDll + GameDllConstantsOffset ), GameDllConstantsSize, ( PBYTE )"Battle.net Gateways" ) )
	{
		FoundServer = true;
		VirtualProtect( TmpStrPath, 20, PAGE_READWRITE, &oldprotect );
		WriteProcessMemory( GetCurrentProcess( ), ( void * )TmpStrPath, servername, strlen( servername ) + 1, 0 );
		VirtualProtect( TmpStrPath, 20, oldprotect, &oldprotect );
	}

	if ( !FoundServer )
	{
		CONSOLE_Print( "Patch server list failed!" );
	}

	while ( TmpStrPath = FindSignature( ( LPBYTE )( GameDll + GameDllConstantsOffset ), GameDllConstantsSize, ( PBYTE )"Battle.net" ) )
	{
		VirtualProtect( TmpStrPath, 20, PAGE_READWRITE, &oldprotect );
		WriteProcessMemory( GetCurrentProcess( ), ( void * )TmpStrPath, servername, strlen( servername ) + 1, 0 );
		VirtualProtect( TmpStrPath, 20, oldprotect, &oldprotect );
	}


	if ( NeedUseNewD3dMode )
	{
		//CONSOLE_Print( "NeedUseNewD3dMode" );
		if ( GetModuleHandle( "d3d8.dll" ) )
		{
			FreeLibrary( GetModuleHandle( "d3d8.dll" ) );
			//CONSOLE_Print( "Bad d3d8.dll allready loaded!" );
		}
		//else
			//CONSOLE_Print( "All okay." );
		while ( TmpStrPath = FindSignature( ( LPBYTE )( GameDll + GameDllConstantsOffset ), GameDllConstantsSize, ( PBYTE )"d3d8.dll" ) )
		{
			//CONSOLE_Print( "d3d8 found" );
			VirtualProtect( TmpStrPath, 20, PAGE_READWRITE, &oldprotect );
			WriteProcessMemory( GetCurrentProcess( ), ( void * )TmpStrPath, d3d8libname, strlen( d3d8libname ) + 1, 0 );
			VirtualProtect( TmpStrPath, 20, oldprotect, &oldprotect );

		}

		LoadLibraryA( "d3d8.fix" );
	}


	//if ( IsKeyPressed( '2' ) )
	//	return;


	//else
		//CONSOLE_Print( "NeedUseNewD3dMode false" );


	//while ( ServerRegistryPath = FindSignature( ( LPBYTE )( GameDll + GameDllConstantsOffset ), GameDllConstantsSize, ( PBYTE )"Battle.net" ) )
	//{
	//	VirtualProtect( ServerRegistryPath, 20, PAGE_READWRITE, &oldprotect );
	//	WriteProcessMemory( GetCurrentProcess( ), ( void * )ServerRegistryPath, servername, strlen( servername ) + 1, 0 );
	//	VirtualProtect( ServerRegistryPath, 20, oldprotect, &oldprotect );
	//}


	memset( OldStatFrameValues, 0, 20 * 4 );

	//int ijladdr = (int)GetModuleHandle( "ijl20.dll" );
	//ijlInit_org = ( ijlInit_p )( ijladdr + 0x32240 );
	//auto err1 = MH_CreateHook( ijlInit_org, &ijlInit_my, reinterpret_cast< void** >( &ijlInit_ptr ) );
	//auto err2 = MH_EnableHook( ijlInit_org );

	//BOOL ijlInit_org_found1 = IsExecutableAddress( ijlInit_org );
	//BOOL ijlInit_org_found2 = IsExecutableAddress2( ijlInit_org );
	//BOOL ijlInit_my_found1 = IsExecutableAddress( &ijlInit_my );
	//BOOL ijlInit_my_found2 = IsExecutableAddress2( &ijlInit_my );

	//char tmperrcode[ 512 ];
	//sprintf_s( tmperrcode, " IJL Addr:%X \n Hook addr: %X\n ijlInit_my addr:%X\n MH_CreateHook = %s\n MH_EnableHook = %s\n %s %s %s %s",
	//	ijladdr, ( void * )ijlInit_org, (void *)&ijlInit_my, MH_StatusToString( err1 ), MH_StatusToString( err2 ),
	//	ijlInit_org_found1 ? "ijlInit_org_found1: found\n" : "ijlInit_org_found1: not found\n", 
	//	ijlInit_org_found2 ? "ijlInit_org_found2: found\n" : "ijlInit_org_found2: not found\n", 
	//	ijlInit_my_found1 ? "ijlInit_my_found1: found\n" : "ijlInit_my_found1: not found\n",
	//	ijlInit_my_found2 ? "ijlInit_my_found2: found\n" : "ijlInit_my_found2: not found\n");
	//MessageBox( 0, "Ok", "Error", 0 );

//	GetUnkTlsValue( );
	_BarVTable = GameDll + 0x93E604;
	pGameClass1 = GameDll + 0xAB7788;
	UnitVtable = GameDll + 0x931934;
	ItemVtable = GameDll + 0x9320B4;
	IsPlayerEnemyOffset = 0x3C9580;
	_Player = ( pPlayer )( GameDll + 0x3BBB30 );
	GetUnitFloatState = ( _GetUnitFloatStat )( GameDll + 0x27AE90 );  // 6F27B9B0 
	GetHeroInt = ( pGetHeroInt )( GameDll + 0x277850 );


	GetCameraHeight_org = pGetCameraHeight( GameDll + 0x3019A0 );
	MH_CreateHook( GetCameraHeight_org, &GetCameraHeight_my, reinterpret_cast< void** >( &GetCameraHeight_ptr ) );
	MH_EnableHook( GetCameraHeight_org );



	ReadGlobalWc3StringToBuffer_org = ( ReadGlobalWc3StringToBuffer )( GameDll + 0x5C9650 );//1.27a 0x090B00
	MH_CreateHook( ReadGlobalWc3StringToBuffer_org, &ReadGlobalWc3StringToBuffer_my, reinterpret_cast< void** >( &ReadGlobalWc3StringToBuffer_ptr ) );
	MH_EnableHook( ReadGlobalWc3StringToBuffer_org );



	MinimapClickEvent_org = ( pMinimapClickEvent )( GameDll + 0x347C90 );//1.27a 0x090B00
	MH_CreateHook( MinimapClickEvent_org, &MinimapClickEvent_my, reinterpret_cast< void** >( &MinimapClickEvent_ptr ) );
	MH_EnableHook( MinimapClickEvent_org );


	GetWc3String_org = ( GetWc3String )( GameDll + 0x5CE190 );//1.27a 0x096AC0
	MH_CreateHook( GetWc3String_org, &GetWc3String_my, reinterpret_cast< void** >( &GetWc3String_ptr ) );
	MH_EnableHook( GetWc3String_org );

	GetWindowXoffset = ( float * )( GameDll + 0xADE91C );//1.27a 0xBBA22C
	GetWindowYoffset = ( float * )( GameDll + 0xADE918 );//1.27a 0xBBA228

	SetGameAreaFOV_org = ( SetGameAreaFOV )( 0x7B66F0 + GameDll );//1.27a 0xD31D0
	MH_CreateHook( SetGameAreaFOV_org, &SetGameAreaFOV_new, reinterpret_cast< void** >( &SetGameAreaFOV_ptr ) );
	MH_EnableHook( SetGameAreaFOV_org );
	//if ( IsKeyPressed( '3' ) )
	//	return;

	IsBlizzardMap_org = ( IsBlizzardMap )( GameDll + 0x7E2EF0 );//1.27a 0x7BFFF0
	MH_CreateHook( IsBlizzardMap_org, &IsBlizzardMap_my, reinterpret_cast< void** >( &IsBlizzardMap_ptr ) );
	MH_EnableHook( IsBlizzardMap_org );



	SetPlayerSlotInfo_org = ( pSetPlayerSlotInfo )( GameDll + 0x560E80 );//1.27a 0x29A690
	MH_CreateHook( SetPlayerSlotInfo_org, &SetPlayerSlotInfo_my, reinterpret_cast< void** >( &SetPlayerSlotInfo_ptr ) );
	MH_EnableHook( SetPlayerSlotInfo_org );

	setString = ( p_setString )( GameDll + 0x4C5CF0 );

	//CONSOLE_Print( "2" );

	BlackListProc.push_back( "ShellExecuteA" );
	BlackListProc.push_back( "ShellExecuteW" );
	BlackListProc.push_back( "LdrLoadDll" );
	BlackListProc.push_back( "CreateProcessA" );
	BlackListProc.push_back( "CreateProcessW" );
	BlackListProc.push_back( "DbgBreakPoint" );
	BlackListProc.push_back( "DbgUserBreakPoint" );
	BlackListProc.push_back( "LdrGetDllHandle" );
	BlackListProc.push_back( "OpenProcess" );
	BlackListProc.push_back( "CreateProcessAsUserA" );
	BlackListProc.push_back( "CreateProcessAsUserW" );
	BlackListProc.push_back( "DbgUserBreakPoint" );
	BlackListProc.push_back( "RegOpenCurrentUser" );
	BlackListProc.push_back( "RegOpenKeyEx" );
	BlackListProc.push_back( "RegOpenKeyTransacted" );
	BlackListProc.push_back( "RegOpenUserClassesRoot" );


	//CONSOLE_Print( "3" );


	//if ( IsKeyPressed( '4' ) )
	//	return;
	Storm_MemAlloc_org = ( pStorm_MemAlloc )Storm::MemAlloc;
	/*MH_CreateHook( Storm_MemAlloc_org, &Storm_MemAlloc_my, reinterpret_cast< void** >( &Storm_MemAlloc_ptr ) );
	MH_EnableHook( Storm_MemAlloc_org );*/


	Wc3Error_org = ( pWc3Error )( GameDll + 0x068C0 );
	MH_CreateHook( Wc3Error_org, &Wc3Error_my, reinterpret_cast< void** >( &Wc3Error_ptr ) );
	MH_EnableHook( Wc3Error_org );

	IsSomeOk_org = ( IsSomeOk )( GameDll + 0x09920 );
	MH_CreateHook( IsSomeOk_org, &IsSomeOk_my, reinterpret_cast< void** >( &IsSomeOk_ptr ) );
	MH_EnableHook( IsSomeOk_org );



	char WarcisMpqPath[ MAX_PATH ];
	sprintf_s( WarcisMpqPath, "%s\\%s", launcherpath, "warcis.mpq" );

	char CustomMpqPath[ MAX_PATH ];
	sprintf_s( CustomMpqPath, "%s\\%s", launcherpath, "custom.mpq" );


	//if ( IsKeyPressed( '5' ) )
	//	return;

	HANDLE tmpval;

	int outdataaddr;
	size_t outdatasize;

	bool FoundArchives = false;


	/*	Storm::FileOpenArchive( (War3Path + "\\War3.mpq").c_str ( ), 0, 2, &tmpval );
		Storm::FileOpenArchive( (War3Path + "\\War3x.mpq" ).c_str( ), 1, 2, &tmpval );
		Storm::FileOpenArchive( (War3Path + "\\War3xlocal.mpq" ).c_str( ), 3, 2, &tmpval );
		Storm::FileOpenArchive( (War3Path + "\\War3Patch.mpq" ).c_str( ), 8, 6, &tmpval );

		
	if ( Storm::Archive_OpenFile( "UI\\SkinMetaData.slk", &outdataaddr, &outdatasize, 1,0 ) || outdatasize )
	{
		MessageBox( 0, "W3X LOADED", " ", 0 );
		FoundArchives = true;
	}

	if ( Storm::Archive_OpenFile( "UI\\HelpStrings.txt", &outdataaddr, &outdatasize, 1, 0 ) || outdatasize)
	{
		MessageBox( 0, "W3 LOADED", " ", 0 );
		FoundArchives = true;
	}*/

	BOOL AllOkay = TRUE;
	BOOL custommpqloaded = TRUE;


	//AllOkay = FALSE;
	//custommpqloaded = Storm::FileOpenArchive( WarcisMpqPath, 11, 6, &tmpval );
	//if ( !custommpqloaded || !tmpval )
	//{
	//	CONSOLE_Print( "Error loading warcis.mpq at path: " + ( string )WarcisMpqPath );
	//	custommpqloaded = Storm::FileOpenArchive( ( AMH_Path + "\\warcis.mpq" ).c_str( ), 11, 6, &tmpval );
	//	if ( !custommpqloaded || !tmpval )
	//	{
	//		CONSOLE_Print( "Error loading warcis.mpq #2 at path: " + ( string )WarcisMpqPath );
	//		custommpqloaded = Storm::FileOpenArchive( ( AMH_Path_old + "\\warcis.mpq" ).c_str( ), 11, 6, &tmpval );
	//		if ( !custommpqloaded || !tmpval )
	//		{
	//			CONSOLE_Print( "Error loading warcis.mpq #3 at path: " + ( string )WarcisMpqPath );
	//		}
	//		else AllOkay = TRUE;
	//	}
	//	else AllOkay = TRUE;
	//}
	//else AllOkay = TRUE;



	//AllOkay = FALSE;
	//custommpqloaded = FALSE;

	//if ( gInfo.UseCustomMpq )
	//{
	//	BOOL custommpqloaded = Storm::FileOpenArchive( CustomMpqPath, 10, 6, &tmpval );
	//	if ( !custommpqloaded || !tmpval )
	//	{
	//		CONSOLE_Print( "Error loading custom.mpq at path: " + ( string )CustomMpqPath );
	//		custommpqloaded = Storm::FileOpenArchive( ( AMH_Path + "\\custom.mpq" ).c_str( ), 10, 6, &tmpval );
	//		if ( !custommpqloaded || !tmpval )
	//		{
	//			CONSOLE_Print( "Error loading custom.mpq #2 at path: " + ( string )CustomMpqPath );
	//			custommpqloaded = Storm::FileOpenArchive( ( AMH_Path_old + "\\custom.mpq" ).c_str( ), 10, 6, &tmpval );
	//			if ( !custommpqloaded || !tmpval )
	//			{
	//				CONSOLE_Print( "Error loading custom.mpq #3 at path: " + ( string )CustomMpqPath );


	//			}
	//			else AllOkay = TRUE;
	//		}
	//		else AllOkay = TRUE;
	//	}
	//	else AllOkay = TRUE;
	//}
	//else
	//{
	//	//AllOkay = TRUE;
	//	//custommpqloaded = TRUE;
	//}





	//if ( !AllOkay )
	//{
	//	if ( FileExists( ( CurrentPathW + L"\\custom.mpq" ) ) )
	//	{
	//		DeleteFileW( ( CurrentPathW + L"\\custom.mpq" ).c_str( ) );
	//	}

	//	if ( FileExists( ( CurrentPathW + L"\\warcis.mpq" ) ) )
	//	{
	//		DeleteFileW( ( CurrentPathW + L"\\warcis.mpq" ).c_str( ) );
	//	}

	//	//DeleteFileW( ( CurrentPathW + L"\\warcis.mpq" ).c_str( ) );
	//	CopyFileW( ( AMH_PathW + L"\\warcis.mpq" ).c_str( ), ( CurrentPathW + L"\\warcis.mpq" ).c_str( ), FALSE );
	//	custommpqloaded = Storm::FileOpenArchive( "warcis.mpq", 10, 6, &tmpval );

	//	if ( gInfo.UseCustomMpq )
	//	{
	//		//DeleteFileW( ( CurrentPathW + L"\\custom.mpq" ).c_str( ) );
	//		CopyFileW( ( AMH_PathW + L"\\custom.mpq" ).c_str( ), ( CurrentPathW + L"\\custom.mpq" ).c_str( ), FALSE );
	//		custommpqloaded = Storm::FileOpenArchive( "custom.mpq", 11, 6, &tmpval );
	//	}

	//}

	MH_CreateHook( Storm::FileOpenArchive, &Storm::FileOpenArchive_my, reinterpret_cast< void** >( &Storm::FileOpenArchive_ptr ) );
	MH_EnableHook( Storm::FileOpenArchive );

	//if ( IsKeyPressed( '6' ) )
	//	return;

	AddNewAhChecks( GetProcAddress_org, 50 );
	AddNewAhChecks( GetProcAddress_my, 50 );

	//CONSOLE_Print( "4" );

	InitWc3MainMenu_org = ( InitFrameDef )( GameDll + 0x58BD20 );//1.27a 0x2BE270
	MH_CreateHook( InitWc3MainMenu_org, &InitWc3MainMenu_my, reinterpret_cast< void** >( &InitWc3MainMenu_ptr ) );
	MH_EnableHook( InitWc3MainMenu_org );


	InitWc3BattleNetMenu_org = ( InitFrameDef )( GameDll + 0x5AB960 );//1.27a 0x2B9D60
	MH_CreateHook( InitWc3BattleNetMenu_org, &InitWc3BattleNetMenu_my, reinterpret_cast< void** >( &InitWc3BattleNetMenu_ptr ) );
	MH_EnableHook( InitWc3BattleNetMenu_org );

	BattleNetChatPanel_org = ( InitFrameDef )( GameDll + 0x5B2570 );//1.27a 0x2B7560
	MH_CreateHook( BattleNetChatPanel_org, &BattleNetChatPanel_my, reinterpret_cast< void** >( &BattleNetChatPanel_ptr ) );
	MH_EnableHook( BattleNetChatPanel_org );

	BattleNetCustomJoinPanel_org = ( InitFrameDef )( GameDll + 0x575F60 );//1.27a 0x2B92C0
	MH_CreateHook( BattleNetCustomJoinPanel_org, &BattleNetCustomJoinPanel_my, reinterpret_cast< void** >( &BattleNetCustomJoinPanel_ptr ) );
	MH_EnableHook( BattleNetCustomJoinPanel_org );

	//if ( IsKeyPressed( '7' ) )
	//	return;
	GetFrameItemAddress = ( pGetFrameItemAddress )( GameDll + 0x5FA970 );//1.27a 0x09EF40

	WarcraftRealWNDProc_org = ( WarcraftRealWNDProc )( GameDll + 0x6C6AA0 );//1.27a  0x153710 

	MH_CreateHook( WarcraftRealWNDProc_org, &BeforeWarcraftWNDProc, reinterpret_cast< void** >( &WarcraftRealWNDProc_ptr ) );
	MH_EnableHook( WarcraftRealWNDProc_org );

	SetFrameText = ( SetFrameText_p )( GameDll + 0x611D40 );//1.27a 0xAA130 
	EditBoxInsertText = ( SetFrameText_p )( GameDll + 0x615700 );//1.27a 0xAF200
	SetEditBoxText = ( SetFrameText2_p )( GameDll + 0x615B50 );//1.27a 0xB0450 


	GetTickCount_org = ( pGetTickCount )( GetProcAddress_ptr( GetModuleHandle( "Kernel32.dll" ), "GetTickCount" ) );
	MH_CreateHook( GetTickCount_org, &GetTickCount_my, reinterpret_cast< void** >( &GetTickCount_ptr ) );
	//MH_EnableHook( GetTickCount_org );


	//SetChatMessage_org = ( pSetChatMessage )( GameDll + 0x61D640 );//1.27a 0xB5A40
	//MH_CreateHook( SetChatMessage_org, &SetChatMessage_my, reinterpret_cast< void** >( &SetChatMessage_ptr ) );
	//MH_EnableHook( SetChatMessage_org );

	Wc3ControlClickButton_org = ( pWc3ControlClickButton )( GameDll + 0x601F20 );//1.27a 0xBE3A0
	MH_CreateHook( Wc3ControlClickButton_org, &Wc3ControlClickButton_my, reinterpret_cast< void** >( &Wc3ControlClickButton_ptr ) );
	MH_EnableHook( Wc3ControlClickButton_org );

	//Wc3ControlMouseBeforeClickButton_org = ( pWc3ControlMouseBeforeClickButton )( GameDll + 0xAFAC0 );//1.27a
	//MH_CreateHook( Wc3ControlMouseBeforeClickButton_org, &Wc3ControlMouseBeforeClickButton_my, reinterpret_cast< void** >( &Wc3ControlMouseBeforeClickButton_ptr ) );
	//MH_EnableHook( Wc3ControlMouseBeforeClickButton_org );

	AddNewAhChecks( GetTickCount_org, 50 );
	AddNewAhChecks( GetTickCount_my, 50 );

	SimpleButtonClickEvent = ( c_SimpleButtonClickEvent )( GameDll + 0x603440 );//1.27a  0x0BB560
	MH_CreateHook( SimpleButtonClickEvent, &SimpleButtonClickEvent_my, reinterpret_cast< void** >( &SimpleButtonClickEvent_ptr ) );
	MH_EnableHook( SimpleButtonClickEvent );

	SimpleButtonPreClickEvent_org = ( pSimpleButtonPreClickEvent )( GameDll + 0x6033A0 );//1.27a  0x0BB560
	MH_CreateHook( SimpleButtonPreClickEvent_org, &SimpleButtonPreClickEvent_my, reinterpret_cast< void** >( &SimpleButtonPreClickEvent_ptr ) );
	MH_EnableHook( SimpleButtonPreClickEvent_org );
	//if ( IsKeyPressed( '8' ) )
	//	return;
	CommandButtonVtable = GameDll + 0x93EBC4; // 127a 0x98F6A8

	GetTypeInfo = ( p_GetTypeInfo )( GameDll + 0x32C880 ); // 127a 0x327020
	//

	pW3XGlobalClass = GameDll + 0xAB4F80;//1.27a 0xBE6350

	GameChatSetState = ( pGameChatSetState )( GameDll + 0x341460 );


	pSetChatTargetUsers_org = ( pSetChatTargetUsers )( GameDll + 0x3412F0 );
	MH_CreateHook( pSetChatTargetUsers_org, &SetChatTargetUsers_my, reinterpret_cast< void** >( &pSetChatTargetUsers_ptr ) );
	MH_EnableHook( pSetChatTargetUsers_org );

	UpdateFrameFlags = ( pUpdateFrameFlags )( GameDll + 0x602370 ); // 127a  0x0BEFD0 


	SetFrameMaxBuffer = ( pSetFrameMaxBuffer )( GameDll + 0x601D60 ); // 127a  0xBE300

	UpdatePlayerSlot = ( sub_6F29A640 )( GameDll + 0x5662D0 ); // 127a  0x29A640 

	GetPlayerName = ( p_GetPlayerName )( GameDll + 0x2F8F90 );

	PreloadGenEnd_org = ( pPreloadGenEnd )( GameDll + 0x3B52D0 ); // 127a  0x1F2110
	MH_CreateHook( PreloadGenEnd_org, &PreloadGenEnd_my, reinterpret_cast< void** >( &PreloadGenEnd_ptr ) );
	MH_EnableHook( PreloadGenEnd_org );

	Preloader_org = ( pPreloader )( GameDll + 0x3B5310 );
	MH_CreateHook( Preloader_org, &Preloader_my, reinterpret_cast< void** >( &Preloader_ptr ) );
	MH_EnableHook( Preloader_org );

	SavePreloadFile_org = ( pSavePreloadFile )( GameDll + 0x3F9060 ); // 127a 0x2124E0 

	MapNameOffset1 = GameDll + 0xAAE788; // 127a 0xBEE150
	MapNameOffset2 = 8;

	Game_Wc3MessageBox = ( pGame_Wc3MessageBox )( GameDll + 0x55CEB0 ); // 127a 0x29E8F0

	//if ( IsKeyPressed( '9' ) )
	//	return;
	CWar3Frame::Init( 0x26a, GameDll );

	CWar3Frame::InitCallbackHook( );

	Wc3SetDefaultSkinTheme( "Demon" );// "Human"  "Orc""Undead""NightElf""Demon" 
	CONSOLE_Print( "CustomFeatureInit" );

	SelectedMapCode = "dota88";
	SelectedMapMode = "ap";
	SelectedMapPlayers = "5x5";



	WarcraftInit_org = ( WarcraftInit )( GameDll + 0x8420 ); // 127a  0x1F2110
	MH_CreateHook( WarcraftInit_org, &WarcraftInit_my, reinterpret_cast< void** >( &WarcraftInit_ptr ) );
	MH_EnableHook( WarcraftInit_org );


	//if ( IsKeyPressed( '0' ) )
	//	return;
	TextAreaSetText_org = ( pTextAreaSetText )( GameDll + 0x61D640 );
	MH_CreateHook( TextAreaSetText_org, &TextAreaSetText_my, reinterpret_cast< void** >( &TextAreaSetText_ptr ) );
	MH_EnableHook( TextAreaSetText_org );



	//if ( IsKeyPressed( 'a' ))
	//	return;
	//EnableMemHooks( );
	InitFpsFix( );

	//if ( IsKeyPressed( 's' ) )
	//	return;
	int blplimitaddr = GameDll + 0x52D373;
	PatchMemory( ( LPVOID )blplimitaddr, { 0xEB } );
	//if ( IsKeyPressed( 'd' ) )
	//	return;
	CWar3Frame::SetChangeMenuEventCallback( ChangeMenuCallback );
	//if ( IsKeyPressed( 'f' ) )
	//	return;
	DWORD thread_id;
	auto tmpthreadhandle = CreateThread( 0, 0, InitMapInfos, 0, CREATE_SUSPENDED, &thread_id );
	AddThreadInfoMy( thread_id, L" INIT MAP INFOS THREAD" );
	ResumeThread( tmpthreadhandle );
	//if ( IsKeyPressed( 'e' ) )
	//	return;
	InitVoiceClientThread( );
	//	VirtualFree_org

	if ( gInfo.UseNewD3dMode && !gInfo.Opengl && gInfo.EnableVSYNC )
	{
		int fpslimitaddr = GameDll + 0x62D7FB;
		PatchMemory( ( LPVOID )fpslimitaddr, { 0xFF } );
		EnableVSync( TRUE, gInfo.MaxFps );
	}
	else
	{
		int fpslimitaddr = GameDll + 0x62D7F7;
		PatchMemory( ( LPVOID )fpslimitaddr, { 0x90,0x90 } );
	}

	FixPossibledMaphackMems( );


	/* FIX DEFAULT ANTIFLOOD */
	//PatchMemory( ( LPVOID )( GameDll + 0x5AE298 ), { 0xEB } );
	//PatchMemory( ( LPVOID )( GameDll + 0x5AF5F8 ), { 0xEB } );
	//PatchMemory( ( LPVOID )( GameDll + 0x5B0CEB ), { 0xEB } );
	PatchMemory( ( LPVOID )( GameDll + 0x6A3674 ), { 0xEB } );

	thread_id = 0;
	tmpthreadhandle = CreateThread( 0, 0, CURSORCLIPPER, 0, CREATE_SUSPENDED, &thread_id );

	g_hKeyboardHook = SetWindowsHookEx( WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle( NULL ), 0 );
	InitManaBar( GameDll );
	SetManabarEnabled( TRUE );
	//SetTlsForMe( );
}

void FixPossibledMaphackMems( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif

	RestoreVtable( 0x936328, 0x3012e0 );
	RestoreVtable( 0x9415A8, 0x39C090 );
	RestoreVtable( 0x931AB4, 0x2a5d50 );
	RestoreVtable( 0x940058, 0x36a660 );
	RestoreVtable( 0x940110, 0x36e8b0 );
	RestoreVtable( 0x9319E8, 0x29d880 );
	RestoreVtable( 0x93A470, 0x35d960 );
	RestoreVtable( 0x931A34, 0x285110 );
	RestoreVtable( 0x92A214, 0x2026a0 );
	RestoreVtable( 0x936348, 0x2fb0e0 );
	RestoreVtable( 0x93CF78, 0x35f940 );
	RestoreVtable( 0x9365B8, 0x308e70 );
	RestoreVtable( 0x93B098, 0x353820 );
	RestoreVtable( 0x93B110, 0x353e10 );
	RestoreVtable( 0x9402F4, 0x3625f0 );
	RestoreVtable( 0x93B2F0, 0x3548c0 );
	RestoreVtable( 0x93E678, 0x364a50 );
	RestoreVtable( 0x93FA98, 0x364a50 );
	RestoreVtable( 0x9582B4, 0x5375b0 );
	RestoreVtable( 0x969A78, 0x5c4450 );
	RestoreVtable( 0x962958, 0x5a02e0 );
	RestoreVtable( 0x9674E0, 0x59b630 );
	RestoreVtable( 0x960EF0, 0x5ba950 );
	RestoreVtable( 0x87D380, 0x4e3b0 );
	RestoreVtable( 0x87D6D0, 0x4e3b0 );
	RestoreVtable( 0x8A9F24, 0xb8510 );
	RestoreVtable( 0x8DDA6C, 0x118440 );
	RestoreVtable( 0x92958C, 0x1fd180 );
	RestoreVtable( 0x8F95FC, 0x162dc0 );
	RestoreVtable( 0x8F9A3C, 0x162dc0 );



	RestoreMemOffset( 0x3C639C, 65341 );
	RestoreMemOffset( 0x3C63A0, -1056606720 );
	RestoreMemOffset( 0x3CB870, 192151560 );
	RestoreMemOffset( 0x34DDA0, -2020931468 );
	RestoreMemOffset( 0x34DDA8, -2020931861 );
	RestoreMemOffset( 0x35FA48, 149624868 );
	RestoreMemOffset( 0x2851B0, 695582853 );
	RestoreMemOffset( 0x28519C, 1149971060 );
	RestoreMemOffset( 0x282A5C, -858993469 );
	RestoreMemOffset( 0x399A98, 1815684980 );
	RestoreMemOffset( 0x3A1598, 600880911 );
	RestoreMemOffset( 0x3A14F0, 1149962731 );
	RestoreMemOffset( 0x361174, 225821573 );
	RestoreMemOffset( 0x74CA18, 1284541183 );
	RestoreMemOffset( 0x356524, -2097051580 );
	RestoreMemOffset( 0x361438, -1194773758 );
	RestoreMemOffset( 0x43EE94, -1065025533 );
	RestoreMemOffset( 0x43EE98, 12616719 );
	RestoreMemOffset( 0x43EEA8, 264275200 );
	RestoreMemOffset( 0x43EEAC, 44420 );
	RestoreMemOffset( 0x38E9F0, 1985938600 );
	RestoreMemOffset( 0x04B7D0, 1958774016 );
	RestoreMemOffset( 0x3A14D8, 1963057795 );
	RestoreMemOffset( 0x2026DC, 23036943 );
	RestoreMemOffset( 0x0C838C, 16548879 );
	RestoreMemOffset( 0x28E1DC, 829800581 );
	RestoreMemOffset( 0x34F2A8, -1957296012 );
	RestoreMemOffset( 0x34F2E8, -1957296012 );
	RestoreMemOffset( 0x3a1560, 1711276032 );


	RestoreMemOffset( 0x3563E8, 0x1828C0F );
	RestoreMemOffset( 0x38B602, 0xC01BC83B );
	RestoreMemOffset( 0x425C48, 0x4B20974 );
	RestoreMemOffset( 0x424C7C, 0x7C833474 );
	RestoreMemOffset( 0x35FA2B, 0x448B1F75 );
	RestoreMemOffset( 0x3A15B2, 0xEAC1D08B );
	RestoreMemOffset( 0x42554F, 0x60247C8B );
	RestoreMemOffset( 0x28DF9B, 0x50247C83 );

}



void CheckMaphackMems( )
{


	CheckIsVtableOk( 0x936328, 0x3012e0 );
	CheckIsVtableOk( 0x9415A8, 0x39C090 );
	CheckIsVtableOk( 0x931AB4, 0x2a5d50 );
	CheckIsVtableOk( 0x940058, 0x36a660 );
	CheckIsVtableOk( 0x940110, 0x36e8b0 );
	CheckIsVtableOk( 0x9319E8, 0x29d880 );
	CheckIsVtableOk( 0x93A470, 0x35d960 );
	CheckIsVtableOk( 0x931A34, 0x285110 );
	CheckIsVtableOk( 0x92A214, 0x2026a0 );
	CheckIsVtableOk( 0x936348, 0x2fb0e0 );
	CheckIsVtableOk( 0x93CF78, 0x35f940 );
	CheckIsVtableOk( 0x9365B8, 0x308e70 );
	CheckIsVtableOk( 0x93B098, 0x353820 );
	CheckIsVtableOk( 0x93B110, 0x353e10 );
	CheckIsVtableOk( 0x9402F4, 0x3625f0 );
	CheckIsVtableOk( 0x93B2F0, 0x3548c0 );
	CheckIsVtableOk( 0x93E678, 0x364a50 );
	CheckIsVtableOk( 0x93FA98, 0x364a50 );
	CheckIsVtableOk( 0x9582B4, 0x5375b0 );
	CheckIsVtableOk( 0x969A78, 0x5c4450 );
	CheckIsVtableOk( 0x962958, 0x5a02e0 );
	CheckIsVtableOk( 0x9674E0, 0x59b630 );
	CheckIsVtableOk( 0x960EF0, 0x5ba950 );
	CheckIsVtableOk( 0x87D380, 0x4e3b0 );
	CheckIsVtableOk( 0x87D6D0, 0x4e3b0 );
	CheckIsVtableOk( 0x8A9F24, 0xb8510 );
	CheckIsVtableOk( 0x8DDA6C, 0x118440 );
	CheckIsVtableOk( 0x92958C, 0x1fd180 );
	CheckIsVtableOk( 0x8F95FC, 0x162dc0 );
	CheckIsVtableOk( 0x8F9A3C, 0x162dc0 );


	CheckIfMemMhOk( 0x3563E8, 0x1828C0F );
	CheckIfMemMhOk( 0x38B602, 0xC01BC83B );
	CheckIfMemMhOk( 0x425C48, 0x4B20974 );
	CheckIfMemMhOk( 0x424C7C, 0x7C833474 );
	CheckIfMemMhOk( 0x35FA2B, 0x448B1F75 );
	CheckIfMemMhOk( 0x3A15B2, 0xEAC1D08B );
	CheckIfMemMhOk( 0x42554F, 0x60247C8B );
	CheckIfMemMhOk( 0x28DF9B, 0x50247C83 );


	CheckIfMemMhOk( 0x3C639C, 65341 );
	CheckIfMemMhOk( 0x3C63A0, -1056606720 );
	CheckIfMemMhOk( 0x3CB870, 192151560 );
	CheckIfMemMhOk( 0x34DDA0, -2020931468 );
	CheckIfMemMhOk( 0x34DDA8, -2020931861 );
	CheckIfMemMhOk( 0x35FA48, 149624868 );
	CheckIfMemMhOk( 0x2851B0, 695582853 );
	CheckIfMemMhOk( 0x28519C, 1149971060 );
	CheckIfMemMhOk( 0x282A5C, -858993469 );
	CheckIfMemMhOk( 0x399A98, 1815684980 );
	CheckIfMemMhOk( 0x3A1598, 600880911 );
	CheckIfMemMhOk( 0x3A14F0, 1149962731 );
	CheckIfMemMhOk( 0x361174, 225821573 );
	CheckIfMemMhOk( 0x74CA18, 1284541183 );
	CheckIfMemMhOk( 0x356524, -2097051580 );
	CheckIfMemMhOk( 0x361438, -1194773758 );
	CheckIfMemMhOk( 0x43EE94, -1065025533 );
	CheckIfMemMhOk( 0x43EE98, 12616719 );
	CheckIfMemMhOk( 0x43EEA8, 264275200 );
	CheckIfMemMhOk( 0x43EEAC, 44420 );
	CheckIfMemMhOk( 0x38E9F0, 1985938600 );
	CheckIfMemMhOk( 0x04B7D0, 1958774016 );
	CheckIfMemMhOk( 0x3A14D8, 1963057795 );
	CheckIfMemMhOk( 0x2026DC, 23036943 );
	CheckIfMemMhOk( 0x0C838C, 16548879 );
	CheckIfMemMhOk( 0x28E1DC, 829800581 );
	CheckIfMemMhOk( 0x34F2A8, -1957296012 );
	CheckIfMemMhOk( 0x34F2E8, -1957296012 );
	CheckIfMemMhOk( 0x3a1560, 1711276032 );
}

void CheckIsVtableOk( int addr, int vtableaddr )
{
	if ( vtableaddr + GameDll != *( int* )( GameDll + addr ) )
		FoundFakeVtable = true;
}

int RestoredVtables = 0;
int RestoredMems = 0;

void RestoreVtable( int addr, int vtableaddr )
{
	if ( PatchMemory( ( LPVOID )( GameDll + addr ), GameDll + vtableaddr, 0 ) )
		RestoredVtables++;
}

void RestoreMemOffset( int addr, int memdata )
{
	if ( PatchMemory( ( LPVOID )( GameDll + addr ), memdata, 0 ) )
		RestoredMems++;
}

void PrintMemoryValueToLog( int addr )
{
	int memory = *( int* )addr;
	char tmp[ 100 ];
	sprintf_s( tmp, "Default value at %X = %X", addr, memory );
	CONSOLE_Print( tmp );
}
void PrintMemoryValueToLog2( int addr )
{
	int memory = *( int* )addr;
	char tmp[ 100 ];
	sprintf_s( tmp, "Default value at %X = %i", addr, memory );
	CONSOLE_Print( tmp );
}

void CheckIfMemMhOk( int addr, int memdata )
{
	if ( memdata != *( int* )( GameDll + addr ) )
	{
		//	PrintMemoryValueToLog2( GameDll + addr );
		FoundModifiedMemoryCode = true;
	}
}

void CustomFeaturesUninitialize( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	ClearMapInfos( );
	UninitFpsFix( );
	DisableMemHooks( );
	UninitializeVoiceClient( );
	//CWar3Frame::UninitializeAllFrames( );
	CWar3Frame::UninitializeCallbackHook( );
	KillTimer( Warcraft3Window,
		'W3XP' );

	UnhookWindowsHookEx( g_hKeyboardHook );
}

std::vector<LPVOID> ListOfHookedFunc;

void DisableMemHooks( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	for ( auto s : ListOfHookedFunc )
		MH_DisableHook( s );
	ListOfHookedFunc.clear( );

	if ( VirtualAlloc_org && VirtualFree_org )
	{
		MH_DisableHook( VirtualAlloc_org );
		MH_DisableHook( VirtualFree_org );
	}
}

void EnableMemHooks( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	std::vector<string> ListOfFuncHooks = { "terminate","_CIacos","_CIasin","_CIatan","_CIatan2","_CIcos","_CIexp",
		"_CIfmod","_CIlog10","_CIpow","_CIsin","_CIsqrt","_CItan","_HUGE","__CppXcptFilter","__CxxFrameHandler3",
		"__clean_type_info_names_internal","__dllonexit","__iob_func","_adjust_fdiv","_amsg_exit","_beginthreadex",
		"_clearfp","_control87","_crt_debugger_hook","_ctime64","_decode_pointer","_encode_pointer","_encoded_null",
		"_except_handler4_common","_initterm","_initterm_e","_ismbcspace","_itoa","_lock","_malloc_crt","_mbsstr",
		"_onexit","_purecall","_time64","_unlock","_vsnprintf","atof","atoi","atol","calloc","ceil","exit","fclose",
		"floor","fopen","fprintf","fputc","fputs","fread","free","fseek","ftell","fwrite","isalnum","isdigit","isprint",
		"isupper","iswspace","malloc","memcpy","memmove","memset","printf","putc","qsort","rand","realloc","setvbuf",
		"sprintf","srand","sscanf",	"strncmp","strncpy","strstr","strtol","strtoul","toupper","vfprintf" };

	HMODULE defaultmsvcr = GetModuleHandle( "MSVCR80.dll" );
	HMODULE LatestVCruntime = LoadLibraryA( "vcruntime140.dll" );
	HMODULE LatestVCredist = LoadLibraryA( "msvcr120.dll" );

	if ( !defaultmsvcr )
		return;

	int hookedfunctions = 0;

	for ( auto s : ListOfFuncHooks )
	{
		if ( ToLower( s ).find( "alloc" ) != std::string::npos ||
			ToLower( s ).find( "free" ) != std::string::npos )
			continue;
		/*if ( hookedfunctions > 30 )
			break;*/
		void * srcfunc = GetProcAddress_ptr( defaultmsvcr, s.c_str( ) );

		if ( srcfunc )
		{

			void * dstfunc = nullptr;
			if ( LatestVCruntime && ( dstfunc = GetProcAddress_ptr( LatestVCruntime, s.c_str( ) ) ) )
			{
				hookedfunctions++;
				DoHookJmp( srcfunc, dstfunc );
				CONSOLE_Print( "Hook " + s + " from  MSVCR80 to vcruntime140.dll" );
			}
			else if ( LatestVCruntime && ( dstfunc = GetProcAddress_ptr( LatestVCruntime, ( "_" + s ).c_str( ) ) ) )
			{
				hookedfunctions++;
				DoHookJmp( srcfunc, dstfunc );
				CONSOLE_Print( "Hook[2] _" + s + " from  MSVCR80 to vcruntime140.dll" );
			}
			else if ( LatestVCredist && ( dstfunc = GetProcAddress_ptr( LatestVCredist, s.c_str( ) ) ) )
			{
				hookedfunctions++;
				DoHookJmp( srcfunc, dstfunc );
				CONSOLE_Print( "Hook " + s + " from  MSVCR80 to msvcr120.dll" );
			}
			else if ( LatestVCredist && ( dstfunc = GetProcAddress_ptr( LatestVCredist, ( "_" + s ).c_str( ) ) ) )
			{
				hookedfunctions++;
				DoHookJmp( srcfunc, dstfunc );
				CONSOLE_Print( "Hook[2] _" + s + " from  MSVCR80 to msvcr120.dll" );
			}
		}
	}

	CONSOLE_Print( "Hooked " + to_string( hookedfunctions ) + " functions of " + to_string( ListOfFuncHooks.size( ) ) );

	//memmove_org = ( my_memmove_func )( GetProcAddress_ptr( GetModuleHandle( "MSVCR80.dll" ), "memmove" ) );
	//MH_CreateHook( memmove_org, &my_memmove_my/*( my_memmove_func )( GetProcAddress_ptr( GetModuleHandle( "MSVCR120.dll" ), "memmove" ) )*/, reinterpret_cast< void** >( &memmove_ptr ) );
	//MH_EnableHook( memmove_org );

	//memcpy_org = ( my_memmove_func )( GetProcAddress_ptr( GetModuleHandle( "MSVCR80.dll" ), "memcpy" ) );
	//MH_CreateHook( memcpy_org, &my_memcpy_my/*( my_memmove_func )( GetProcAddress_ptr( GetModuleHandle( "MSVCR120.dll" ), "memcpy" ) )*/, reinterpret_cast< void** >( &memcpy_ptr ) );
	//MH_EnableHook( memcpy_org );

	//memset_org = ( my_memset_func )( GetProcAddress_ptr( GetModuleHandle( "MSVCR80.dll" ), "memset" ) );
	//MH_CreateHook( memset_org, &my_memset_my/*( my_memmove_func )( GetProcAddress_ptr( GetModuleHandle( "MSVCR120.dll" ), "memcpy" ) )*/, reinterpret_cast< void** >( &memset_ptr ) );
	//MH_EnableHook( memset_org );


	VirtualAlloc_org = ( pVirtualAlloc )GetProcAddress_ptr( GetModuleHandle( "Kernel32.dll" ), "VirtualAlloc" );
	VirtualFree_org = ( pVirtualFree )GetProcAddress_ptr( GetModuleHandle( "Kernel32.dll" ), "VirtualFree" );

	if ( VirtualAlloc_org && VirtualFree_org )
	{
		MH_CreateHook( VirtualAlloc_org, &VirtualAlloc_my, reinterpret_cast< void** >( &VirtualAlloc_ptr ) );
		MH_EnableHook( VirtualAlloc_org );

		MH_CreateHook( VirtualFree_org, &VirtualFree_my, reinterpret_cast< void** >( &VirtualFree_ptr ) );
		MH_EnableHook( VirtualFree_org );
	}
}


void DoHookJmp( LPVOID srcaddr, LPVOID dstaddr ) {
	LPVOID oldaddr = 0;
	MH_CreateHook( srcaddr, dstaddr, reinterpret_cast< void** >( &oldaddr ) );
	MH_EnableHook( srcaddr );
	FlushInstructionCache( GetCurrentProcess( ), srcaddr, 0x100 );
	ListOfHookedFunc.push_back( srcaddr );
}

typedef int( __fastcall * pDrawBarForUnit )( int unitaddr );
pDrawBarForUnit DrawBarForUnit_org, DrawBarForUnit_ptr;

unsigned int( __thiscall  * DestroyUnitHpBar )( int HpBarAddr );


BOOL __stdcall IsUnitInvulnerable( int unitaddr )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	return ( *( unsigned int* )( unitaddr + 0x20 ) & 8 );
}

BOOL __stdcall IsUnitSelectable( int unitaddr )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	unsigned int unitflag = *( unsigned int* )( unitaddr + 0x20 );
	unsigned int unitflag2 = *( unsigned int* )( unitaddr + 0x5C );

	if ( unitflag & 1u )
		return FALSE;

	if ( !( unitflag & 2u ) )
		return FALSE;

	if ( unitflag2 & 0x100u )
		return FALSE;

	if ( unitflag2 == 0x1001u )
		return FALSE;
	return  TRUE;
}

BOOL __stdcall IsTower( int unitaddr )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	if ( unitaddr > 0 )
	{
		unsigned int istower = *( unsigned int* )( unitaddr + 0x5C );
		return ( istower & 0x10000 ) > 0;
	}
	return FALSE;
}


int __fastcall DrawBarForUnit_my( int unitaddr )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	int retval = 0;
	int hpbaraddr = *( int* )( unitaddr + 0x50 );
	BOOL needremove = FALSE;


	if ( hpbaraddr )
	{
		if ( ( IsTower( unitaddr ) && IsUnitInvulnerable( unitaddr ) ) || !IsUnitSelectable( unitaddr ) )
		{
			needremove = TRUE;
			*( int * )( hpbaraddr + 8 ) = 0;
		}
	}

	retval = DrawBarForUnit_ptr( unitaddr );

	if ( needremove )
	{
		*( int* )( unitaddr + 0x50 ) = 0;
	}

	return retval;
}


void UninitFpsFix( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	MH_DisableHook( DrawBarForUnit_org );
}


void InitFpsFix( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	DestroyUnitHpBar = ( unsigned int( __thiscall  * )( int ) )( GameDll + 0x2C7440 );
	DrawBarForUnit_org = ( pDrawBarForUnit )( GameDll + 0x2C74B0 );
	MH_CreateHook( DrawBarForUnit_org, &DrawBarForUnit_my, reinterpret_cast< void** >( &DrawBarForUnit_ptr ) );
	MH_EnableHook( DrawBarForUnit_org );
}

bool PatchMemory( LPVOID addr, std::vector<unsigned char> data )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	bool IsSame = memcmp( addr, &data[ 0 ], data.size( ) ) == 0;
	DWORD oldprot1, oldprot2;
	VirtualProtect( addr, data.size( ), PAGE_EXECUTE_READWRITE, &oldprot1 );
	memcpy( addr, &data[ 0 ], data.size( ) );
	VirtualProtect( addr, data.size( ), oldprot1, &oldprot2 );
	FlushInstructionCache( GetCurrentProcess( ), addr, data.size( ) );
	return IsSame;
}

bool PatchMemory( LPVOID addr, int data, int fix )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif

	/*char path[ 120 ];
	sprintf_s( path, "Addr: %X, value: %X, oldvalue: %X", addr, data, *( int* )addr );
	CONSOLE_Print( path );
*/
	bool IsSame = memcmp( addr, &data, 4 ) == 0;

	DWORD oldprot1, oldprot2;
	VirtualProtect( addr, 4, PAGE_EXECUTE_READWRITE, &oldprot1 );
	memcpy( addr, &data, 4 );
	VirtualProtect( addr, 4, oldprot1, &oldprot2 );
	FlushInstructionCache( GetCurrentProcess( ), addr, 4 );
	return IsSame;
}

bool PatchMemory( LPVOID addr, std::vector<unsigned char> data, std::vector<unsigned char> & olddata )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	bool IsSame = memcmp( addr, &data[ 0 ], data.size( ) ) == 0;

	DWORD oldprot1, oldprot2;
	VirtualProtect( addr, data.size( ), PAGE_EXECUTE_READWRITE, &oldprot1 );
	olddata = std::vector<unsigned char>( data.size( ) );
	memcpy( &olddata[ 0 ], addr, data.size( ) );
	memcpy( addr, &data[ 0 ], data.size( ) );
	VirtualProtect( addr, data.size( ), oldprot1, &oldprot2 );
	FlushInstructionCache( GetCurrentProcess( ), addr, data.size( ) );
	return IsSame;
}