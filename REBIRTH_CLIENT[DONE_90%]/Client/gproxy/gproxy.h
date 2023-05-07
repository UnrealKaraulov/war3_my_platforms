/*

   Copyright 2010 Trevor Hogan

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

	   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

*/

#ifndef GPROXY_H
#define GPROXY_H

// standard integer sizes for 64 bit compatibility

#include <stdint.h>
extern bool NeedAnnounce;

enum PvPGN_Login_Status : unsigned int
{
	Login_Status_NONE = 0,
	Login_Status_FAILED = 1,
	Login_Status_OK = 2
};

extern PvPGN_Login_Status m_login_status;


// STL

#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <vector>
#include <concurrent_vector.h>

#define safevector concurrency::concurrent_vector

#include "Antihack.h"


#include <signal.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>        


#include <time.h>

extern "C"
{
#include "bcrypt.h"
}
#include "xxhash.h"

#ifndef WIN32
#include <sys/time.h>
#endif

extern std::string GlobalUsername;
extern std::string GlobalPassword;
extern bool IsRegisterAccount;
extern bool ShutDownDOOOOWN;

using namespace std;

typedef vector<unsigned char> BYTEARRAY;

// time

uint32_t GetTime( );		// seconds
uint32_t GetTicks( );		// milliseconds

#ifdef WIN32
#define MILLISLEEP( x ) Sleep( x )
#else
#define MILLISLEEP( x ) usleep( ( x ) * 1000 )
#endif

// network

#undef FD_SETSIZE
#define FD_SETSIZE 512

// output

void LOG_Print( string message );
void CONSOLE_Print( string message, bool log = true );
void CONSOLE_PrintNoCRLF( string message, bool log = true );
void CONSOLE_ChangeChannel( string channel );
void CONSOLE_AddChannelUser( string name );
void CONSOLE_RemoveChannelUser( string name );
void CONSOLE_RemoveChannelUsers( );
void CONSOLE_Resize( );


struct MapHostStruct
{
	string MapName;
	string MapNameForList;
	string MapHost;
	string MapCategory;
	string MapFileName;
	string crc32;
	bool ForStats;
	bool availabled;
};

extern std::vector<MapHostStruct> MapHostStructList;

typedef void( __stdcall * TextCallback )( const char* text );
typedef void( __stdcall * pProcessCommandsCallback )( const char* text, const char* arg1,
	const char* arg2, const char* arg3, const char* arg4, const char* arg5, const char* arg6,
	const char* arg7, const char* arg8, const char* arg9, const char* arg10, const char* arg11);

bool __stdcall ProcessCommandsCallback( const char* text, const char* arg1,
	const char* arg2 = "", const char* arg3 = "", const char* arg4 = "", const char* arg5 = "", const char* arg6 = "",
	const char* arg7 = "", const char* arg8 = "", const char* arg9 = "", const char* arg10 = "", const char* arg11 = "");


extern pProcessCommandsCallback CommandsCallback;
void SendAHPackets( );

//
// CGProxy
//

class CTCPServer;
class CTCPSocket;
class CTCPClient;
class CUDPSocket;
class CBNET;
class CIncomingGameHost;
class CGameProtocol;
class CGPSProtocol;
class CCommandPacket;

class CGProxy;
extern CGProxy * gGProxy;

class CGProxy
{
public:
	string m_Version;
	CTCPServer *m_LocalServer;
	CTCPSocket *m_LocalSocket;
	CTCPClient *m_RemoteSocket;
	CUDPSocket *m_UDPSocket;
	CBNET *m_BNET;
	vector<CIncomingGameHost *> m_Games;
	CGameProtocol *m_GameProtocol;
	CGPSProtocol *m_GPSProtocol;
	queue<CCommandPacket *> m_LocalPackets;
	queue<CCommandPacket *> m_RemotePackets;
	queue<CCommandPacket *> m_PacketBuffer;
	vector<unsigned char> m_Laggers;
	uint32_t m_TotalPacketsReceivedFromLocal;
	uint32_t m_TotalPacketsReceivedFromRemote;
	bool m_Exiting;
	bool m_TFT;
	string m_War3Path;
	string m_CDKeyROC;
	string m_CDKeyTFT;
	string m_Server;
	string m_Username;
	string m_Password;
	string m_Channel;
	uint32_t m_War3Version;
	uint16_t m_Port;
	uint32_t m_LastConnectionAttemptTime;
	uint32_t m_LastRefreshTime;
	string m_RemoteServerIP;
	uint16_t m_RemoteServerPort;
	bool m_GameIsReliable;
	bool m_GameStarted;
	bool m_LeaveGameSent;
	bool m_ActionReceived;
	bool m_Synchronized;
	uint16_t m_ReconnectPort;
	unsigned char m_PID;
	unsigned char m_ChatPID;
	uint32_t m_ReconnectKey;
	unsigned char m_NumEmptyActions;
	unsigned char m_NumEmptyActionsUsed;
	uint32_t m_LastAckTime;
	uint32_t m_LastActionTime;
	string m_JoinedName;
	string m_HostName;

	CGProxy( bool nTFT, string nWar3Path, string nCDKeyROC, string nCDKeyTFT, string nServer, string nUsername, string nPassword, string nChannel, uint32_t nWar3Version, uint16_t nPort, BYTEARRAY nEXEVersion, BYTEARRAY nEXEVersionHash, string nPasswordHashType );
	~CGProxy( );

	// processing functions

	bool Update( long usecBlock );

	void ExtractLocalPackets( );
	void ProcessLocalPackets( );
	void ExtractRemotePackets( );
	void ProcessRemotePackets( );

	bool AddGame( CIncomingGameHost *game );
	void SendLocalChat( string message );
	void SendEmptyAction( );
};

#endif

#pragma once
