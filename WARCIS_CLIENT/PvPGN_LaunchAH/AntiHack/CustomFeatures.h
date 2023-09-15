#pragma once

#include "warcis_reconnector.h"
#include "voiceheaders.h"
#include "WarcraftFrameHelper.h"
int GetLocalPlayerId( );

extern int _BarVTable;
extern int UnitVtable;
extern int ConvertHandle( int handle );
int __stdcall Player( int number );
extern int ItemVtable;
BOOL IsPlayerEnemy( int hPlayer1, int hPlayer2 );
int GetPlayerByNumber( int number );
extern int pGameClass1;
typedef float *( __thiscall * _GetUnitFloatStat )( int unitaddr, float *a2, int a3 );
extern _GetUnitFloatStat GetUnitFloatState;
BOOL __stdcall SetManabarEnabled( BOOL enabled );
typedef int( __cdecl * IsPlayerEnemy_org )( unsigned int Player1, unsigned int Player2 );
int __stdcall InitManaBar( int );
extern BOOL SkipAllFiles;

int * FindUnitAbils( int unitaddr, unsigned int * count, int abilcode = 0, int abilbasecode = 0 );
int __stdcall GetUnitOwnerSlot( int unitaddr );
BOOL __stdcall IsEnemy( int UnitAddr );
BOOL __stdcall IsHero( int unitaddr );
BOOL __stdcall IsTower( int unitaddr );
BOOL __stdcall IsNotBadUnit( int unitaddr, BOOL onlymem = FALSE );
BOOL __stdcall IsUnitInvulnerable( int unitaddr );
BOOL __stdcall IsUnitIllusion( int unitaddr );
BOOL __stdcall IsNotBadItem( int itemaddr, BOOL extracheck = FALSE );
typedef int( __fastcall * pGetHeroInt )( int unitaddr, int unused, BOOL withbonus );
extern pGetHeroInt GetHeroInt;
int GetSelectedUnitCountBigger( int slot );
int GetSelectedUnit( int slot );
void GetItemLocation2D( int itemaddr, float * x, float * y );
void GetUnitLocation2D( int unitaddr, float * x, float * y );
int * GetUnitCountAndUnitArray( int ** unitarray );
int * GetItemCountAndItemArray( int ** itemarray );
float GetUnitMPregen( int unitaddr );
float GetUnitHPregen( int unitaddr );
float GetUnitMP( int unitaddr );
float GetUnitHP( int unitaddr );
int GetUnitAddressFloatsRelated( int unitaddr, int step );
float GetUnitX_real( int unitaddr );
float GetUnitY_real( int unitaddr );


void CustomFeaturesInitialize( const char * launcherpath );
void CustomFeaturesUninitialize( );

struct Matrix1//Matrix 4x4
{
	float flt1;//0
	float flt2;//4
	float flt3;//8
	float flt4;//12
	float flt5;//16
	float flt6;//20
	float flt7;//24
	float flt8;//28
	float flt9;//32
	float flt10;//36
	float flt11;//40
	float flt12;//44
	float flt13;//48
	float flt14;//52
	float flt15;//56
	float flt16;//60
};

FARPROC WINAPI  GetProcAddress_my( HMODULE hModule, LPCSTR  lpProcName );
typedef FARPROC( WINAPI * pGetProcAddress )( HMODULE hModule, LPCSTR  lpProcName );
extern pGetProcAddress GetProcAddress_org;
extern pGetProcAddress GetProcAddress_ptr;


void UpdateVoicePlayerList( );


extern LPARAM lpRShiftScanKeyUP;
extern LPARAM lpRShiftScanKeyDOWN;

extern LPARAM lpShiftScanKeyUP;
extern LPARAM lpShiftScanKeyDOWN;

extern LPARAM lpLShiftScanKeyUP;
extern LPARAM lpLShiftScanKeyDOWN;

extern int PressEnterChatButtonTimed;
void SetGameSpeed( float newspeed, bool enabledspeedhack = true );
extern BOOL NeedAutoJoin;
extern BOOL NeedBnetSpeedUp;
extern BOOL ShiftNumpadFix;
extern int WarKeyKeys[ 18 ];
extern BOOL WarKeyEnabled;
extern BOOL NeedUseNewD3dMode;
extern bool FixFieldOfView;

extern std::string LineForStats;

struct InitializeInfo
{
	char ServerAddr[ 128 ];
	int ReconnectPort;
	unsigned int var1;
	unsigned int var2;
	unsigned int var3;
	unsigned int var4;
	char Username[ 50 ];
	char Password[ 50 ];
	unsigned int launcher_version;
	int NeedBnetSpeedUp;
	int ShiftNumpadFix;
	int Bypass8MBlimit;
	int WarKeyEnabled;
	int WarKeyKeys[ 18 ];
	int WindowModeAltEnter;
	char StatsLine[ 1024 ];
	unsigned int StartTime;
	int UseNewD3dMode;
	int NeedSaveAutologin;
	int GrayScaleWorld;
	unsigned int NickNameColor;
	unsigned int ChatNickNameColor;
	unsigned int ChatTextColor;
	int Opengl;
	int EnableVSYNC;
	int MaxFps;
	int BugReportWithoutUser;
	int NeedClipCursor;
	int InWindowMode;
	int UseDefaultErrorCatch;
	int MinimapRightClickWithAlt;
	int UseCustomMpq;
	int MaxMapPreloadTime;
};

extern void PrintText( const char * text, float staytime = 10.0f );
extern void PrintText( std::string text, float staytime = 10.0f );


extern int CustomChatMessagesCount;
extern std::vector<int> CustomChatKeyCodes;

struct KeyChatActionStruct
{
	int VK;
	BOOL IsShift;
	BOOL IsCtrl;
	BOOL IsAlt;
	string Message;
};

extern std::vector<KeyChatActionStruct> KeyChatActionList;

BOOL IsCommandButton( int addr );
std::string ReturnStringWithoutWarcraftTags( std::string str );
void DumpFileToDisk( const void * filedata, const uint32_t filesize, const char * filename );
extern int pW3XGlobalClass;

extern HWND Warcraft3Window;
typedef LRESULT( __fastcall *  WarcraftRealWNDProc )( HWND hWnd, unsigned int Msg, WPARAM wParam, LPARAM lParam );
extern WarcraftRealWNDProc WarcraftRealWNDProc_org;
extern WarcraftRealWNDProc WarcraftRealWNDProc_ptr;
typedef int( __fastcall * pGameChatSetState )( int chat, int unused, BOOL IsOpened );
extern pGameChatSetState GameChatSetState;

typedef int( __fastcall * pSetChatTargetUsers/*sub_6F3412F0*/ )( int chataddr, int ecx, int valtype );
extern pSetChatTargetUsers pSetChatTargetUsers_org;
extern pSetChatTargetUsers pSetChatTargetUsers_ptr;
extern std::vector<NWar3Frame::CWar3Frame * > VoicePlayerFrameList[ 2 ];

int __fastcall SetChatTargetUsers_my( int chataddr, int ecx, int valtype );

extern InitializeInfo gInfo;
int __stdcall SendMessageToChat( const char * msg, BOOL toAll );
//BOOL IsChat( );
//BOOL IsGame( );

void DecreaseCameraOffset( float offset = 50);
void IncreaseCameraOffset( float offset = 50 );
void ResetCameraOffset(  );

typedef void( __fastcall * pGame_Wc3MessageBox ) ( int type, const char * text, BOOL IsUsedCallBack, int callbackaddr, int unk2, int unk3, int unk4 );
extern pGame_Wc3MessageBox Game_Wc3MessageBox;
int __stdcall Wc3MessageBox( const char * message, int type );

struct PlayerStatSlot
{
	char PlayerName[ 20 ];
	char PlayerNewName[ 256 ];
	std::string PlayerStats;
	std::string PlayerStats2;
	std::string PlayerStats3;
	std::string PlayerNewStats;
	std::string PlayerNewStats2;
	std::string PlayerNewStats3;
	bool NeedUpdate;
	unsigned int LobbyNickNameColor;
	unsigned int ChatNickNameColor;
	unsigned int ChatTextColor;
};
extern BOOL NeedFullScreenSwitcher;
extern std::vector<PlayerStatSlot> PlayerStatsList;

struct MapInfosStruct
{
	std::string mapfilename;
	std::string minimapfilename;
	std::string previewfilename;
	uint32_t mapcrc32;
	char * MiniMapData;
	size_t MiniMapDataSize;
	char * PreviewData;
	size_t PreviewDataSize;

	MapInfosStruct( )
	{
		mapfilename = "";
		MiniMapDataSize = 0;
		minimapfilename = "";
		PreviewDataSize = 0;
		previewfilename = "";
		mapcrc32 = 0;
		MiniMapData = 0;
		PreviewData = 0;
	}
};


extern std::vector<MapInfosStruct> ListMapInfos;

extern int FollowStatus;
extern int FollowTryCount;
extern std::string FollowGameName;
extern std::string LatestRequestGame;
typedef void( __thiscall * sub_6F29A640 )( int offset, unsigned int id );
extern sub_6F29A640 UpdatePlayerSlot;

std::string uint_to_hex( unsigned int i );

void AddNewPlayerStats( const  std::string & UserName, const  std::string & NewUserName, const  std::string & StatsString1, 
	const  std::string & StatsString2, const  std::string & StatsString3, unsigned int LobbyNickColor, unsigned int ChatNickColor, unsigned int ChatTextColor );
void ClearMapHostList( );
void AddNewMapHost( const  std::string & MapName, const  std::string & MapHost, const  std::string & MapGenre, const  std::string & MapPath, std::vector<std::string> MapModes, std::vector<std::string> MapPlayers, uint32_t crc32, bool forstats );

enum class Wc3Menu
{
	MAIN_MENU,
	BNET_LOGIN,
	BNET_MAIN,
	BNET_CHAT,
	GAME_LIST,
	GAME_LOBBY,
	GAME_GAME,
	GAME_NOGAME
};

typedef char *( __fastcall * p_GetPlayerName )( int id, int real );
extern p_GetPlayerName GetPlayerName;
bool IsPlayerSlotNotEnemy( const  std::string & PlayerName );
extern Wc3Menu current_menu;
extern bool BattleNetButtonPressed;


extern bool needshowmessage;
extern std::string messageforshow;
extern int messageshowtype;
extern int messagesleep;

extern std::string SelectedMapCode;
extern std::string SelectedMapMode;
extern std::string SelectedMapPlayers;
extern std::string SelectedGameName;
extern unsigned int SelectedMapHostType;

extern std::string LastUsername;
extern std::string LastPassword;


struct MapHostStruct
{
	std::string MapName;
	std::string MapNameForList;
	std::string MapHost;
	std::string MapCategory;
	std::string MapFileName;
	std::vector<std::string> MapModes;
	std::vector<std::string> MapPlayers;
	uint32_t crc32;
	bool ForStats;
	bool availabled;
};


extern std::vector<MapHostStruct>  MapHostList;
extern std::vector<MapHostStruct>  FilteredMapHostList;



void EnableMemHooks( );
void DisableMemHooks( );
void DoHookJmp( LPVOID srcaddr, LPVOID dstaddr );
void RestoreVtable( int addr, int vtableaddr );
void RestoreMemOffset( int addr, int memdata );
void CheckMaphackMems( );
void FixPossibledMaphackMems( );
void CheckIsVtableOk( int addr, int vtableaddr );
void CheckIfMemMhOk( int addr, int memdata );
void InitFpsFix( );
void UninitFpsFix( );
void InitVoiceClientThread( );
void UninitializeVoiceClient( );
bool PatchMemory( LPVOID addr, std::vector<unsigned char> data, std::vector<unsigned char> & olddata );
bool PatchMemory( LPVOID addr, int data, int fix );
bool PatchMemory( LPVOID addr, std::vector<unsigned char> data );
void PrintMemoryValueToLog( int addr );
void AddNewPaTestData( unsigned int frameidx, unsigned int maxframeidx, SAMPLE * samples, const std::string & playername );
void SendVoicePackets( CWC3 * gproxy_wc3 );