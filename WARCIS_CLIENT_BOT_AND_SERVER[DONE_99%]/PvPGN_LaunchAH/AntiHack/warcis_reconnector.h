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

//#ifdef WIN32
// #include "ms_stdint.h"
//
//#else
// #include <stdint.h>
//#endif

// STL
#include "voiceheaders.h"
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


void CONSOLE_Print( const std::string &  message );
void CONSOLE_Print( const std::wstring &  message );
void DEBUG_Print( const std::string & message );

//
// CGProxy
//

class CTCPServer;
class CTCPSocket;
class CTCPClient;
class CUDPSocket;
class CBNET;
class CGameProtocol;
class CIncomingGameHost;
class CGPSProtocol;
class CCommandPacket;
class CBNFTPConnection;
class CWC3;


class CGProxy
{
public:
	string m_Version;
	CTCPServer *m_LocalServer;
	CTCPSocket *m_LocalSocket;
	CTCPClient *m_RemoteSocket;
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
	string m_Server;
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
	CTCPServer *m_WC3Server;
	string m_GIndicator;
	vector<CWC3 *> m_Connections;

	CGProxy( const std::string &  nServer, uint16_t nPort );
	~CGProxy( );

	// processing functions

	bool Update( long usecBlock );

	void ExtractLocalPackets( );
	void ProcessLocalPackets( );
	void ExtractRemotePackets( );
	void ProcessRemotePackets( );


	void SendEmptyAction( );
	void SendLocalChat( string message );


};


extern CGProxy * gGProxy;
//
// CIncomingGameHost
//

class CIncomingGameHost
{
public:
	static uint32_t NextUniqueGameID;


	uint16_t m_GameType;
	uint16_t m_Parameter;
	uint32_t m_LanguageID;
	uint16_t m_Port;
	BYTEARRAY m_IP;
	uint32_t m_Status;
	uint32_t m_ElapsedTime;
	string m_GameName;
	unsigned char m_SlotsTotal;
	uint32_t m_HostCounter;
	BYTEARRAY m_StatString;
	uint32_t m_UniqueGameID;
	uint32_t m_ReceivedTime;
	BYTEARRAY m_GamePassword;
	BYTEARRAY m_HostCounterRAW;
	unsigned char m_SlotsTotalRAW;


	// decoded from stat string:

	uint32_t m_MapFlags;
	uint16_t m_MapWidth;
	uint16_t m_MapHeight;
	BYTEARRAY m_MapCRC;
	string m_MapPath;
	string m_HostName;

	CIncomingGameHost( uint16_t nGameType, uint16_t nParameter, uint32_t nLanguageID, uint16_t nPort, BYTEARRAY &nIP, uint32_t nStatus, uint32_t nElapsedTime, string nGameName, BYTEARRAY nGamePassword, unsigned char nSlotsTota, unsigned char nSlotsTotalRAW, uint32_t nHostCounter, BYTEARRAY nHostCounterRAW, BYTEARRAY &nStatString );
	~CIncomingGameHost( );

	uint16_t GetGameType( ) { return m_GameType; }
	uint16_t GetParameter( ) { return m_Parameter; }
	uint32_t GetLanguageID( ) { return m_LanguageID; }
	uint16_t GetPort( ) { return m_Port; }
	BYTEARRAY GetIP( ) { return m_IP; }
	string GetIPString( );
	uint32_t GetStatus( ) { return m_Status; }
	uint32_t GetElapsedTime( ) { return m_ElapsedTime; }
	string GetGameName( ) { return m_GameName; }
	unsigned char GetSlotsTotal( ) { return m_SlotsTotal; }
	uint32_t GetHostCounter( ) { return m_HostCounter; }
	BYTEARRAY GetStatString( ) { return m_StatString; }
	uint32_t GetUniqueGameID( ) { return m_UniqueGameID; }
	uint32_t GetReceivedTime( ) { return m_ReceivedTime; }
	uint32_t GetMapFlags( ) { return m_MapFlags; }
	uint16_t GetMapWidth( ) { return m_MapWidth; }
	uint16_t GetMapHeight( ) { return m_MapHeight; }
	BYTEARRAY GetMapCRC( ) { return m_MapCRC; }
	string GetMapPath( ) { return m_MapPath; }
	string GetHostName( ) { return m_HostName; }
	BYTEARRAY GetGamePassword( ) { return m_GamePassword; }
	BYTEARRAY GetHostCounterRAW( ) { return m_HostCounterRAW; }
	unsigned char GetSlotsTotalRAW( ) { return m_SlotsTotalRAW; }
	BYTEARRAY GetData( const std::string &  indicator );
};

class CWC3
{

public:
	enum Protocol
	{
		SID_NULL = 0,	// 0x0
		SID_STOPADV = 2,	// 0x2
		SID_GETADVLISTEX = 9,	// 0x9
		SID_ENTERCHAT = 10,	// 0xA
		SID_CHANNELLIST = 10,	// 0xA
		SID_JOINCHANNEL = 12,	// 0xC
		SID_CHATCOMMAND = 14,	// 0xE
		SID_CHATEVENT = 15,	// 0xF
		SID_CHECKAD = 21,	// 0x15
		SID_STARTADVEX3 = 28,	// 0x1C
		SID_DISPLAYAD = 33,	// 0x21
		SID_NOTIFYJOIN = 34,	// 0x22
		SID_PING = 37,	// 0x25
		SID_LOGONRESPONSE = 41,	// 0x29
		SID_NETGAMEPORT = 69,	// 0x45
		SID_AUTH_INFO = 80,	// 0x50
		SID_AUTH_CHECK = 81,	// 0x51
		SID_AUTH_ACCOUNTLOGON = 83,	// 0x53
		SID_AUTH_ACCOUNTLOGONPROOF = 84,	// 0x54
		SID_WARDEN = 94,	// 0x5E
		SID_FRIENDSLIST = 101,	// 0x65
		SID_FRIENDSUPDATE = 102,	// 0x66
		SID_CLANMEMBERLIST = 125,	// 0x7D
		SID_CLANMEMBERSTATUSCHANGE = 127,	// 0x7F
		SID_AH_PACKET = 0x6e	// 0x6e
	};
	enum IncomingChatEvent
	{
		EID_SHOWUSER = 1,	// received when you join a channel (includes users in the channel and their information)
		EID_JOIN = 2,	// received when someone joins the channel you're currently in
		EID_LEAVE = 3,	// received when someone leaves the channel you're currently in
		EID_WHISPER = 4,	// received a whisper message
		EID_TALK = 5,	// received when someone talks in the channel you're currently in
		EID_BROADCAST = 6,	// server broadcast
		EID_CHANNEL = 7,	// received when you join a channel (includes the channel's name, flags)
		EID_USERFLAGS = 9,	// user flags updates
		EID_WHISPERSENT = 10,	// sent a whisper message
		EID_CHANNELFULL = 13,	// channel is full
		EID_CHANNELDOESNOTEXIST = 14,	// channel does not exist
		EID_CHANNELRESTRICTED = 15,	// channel is restricted
		EID_INFO = 18,	// broadcast/information message
		EID_ERROR = 19,	// error message
		EID_EMOTE = 23	// emote

	};

	CTCPSocket *m_LocalSocket;
	CTCPClient *m_RemoteSocket;
	bool m_IsBNFTP;
	string m_GIndicator;
	bool m_FirstPacket;
	vector<CIncomingGameHost *> m_Games;
	queue<CCommandPacket *> m_LocalPackets;
	queue<CCommandPacket *> m_RemotePackets;


	void ExtractWC3Packets( );
	void ProcessWC3Packets( );

	void ExtractBNETPackets( );
	void ProcessBNETPackets( );

	bool AssignLength( BYTEARRAY &content );
	bool ValidateLength( BYTEARRAY &content );

	// packet handling

	void Handle_SID_GETADVLISTEX( BYTEARRAY data );
	void Handle_SID_CHATEVENT( BYTEARRAY & data );
	void Handle_SID_CHATCOMMAND( BYTEARRAY data );
	void Handle_SID_AUTH_ACCOUNTLOGONPROOF( BYTEARRAY data );
	
	void Handle_AH_PACKET( BYTEARRAY data );
	void Handle_VOICE_PACKET( BYTEARRAY data );

	void SendVoicePacket( unsigned int frameidx, unsigned int maxframeidx, SAMPLE * bytes );

	bool ProcessCommand( const std::string &  Command );
	void SendLocalChat( string message );
	void SendChatCommand( string message );

	CWC3( CTCPSocket *socket, string hostname, uint16_t port, string indicator );
	~CWC3( );
	unsigned int SetFD( void *fd, void *send_fd, int *nfds );
	bool Update( void *fd, void *send_fd );
	void SendAHPacket( uint32_t version, uint32_t command, uint32_t var1 = 0,
		uint32_t var2 = 0, uint32_t var3 = 0, uint32_t var4 = 0, std::string strvar = "", std::string strvar2 = "", std::string strvar3 = "", std::string strvar4 = "" );
	vector<CIncomingGameHost *> GetGames( ) { return m_Games; }
	string HelloWorld( ) { return "Hello World"; }
};


extern unsigned int __var1;
extern unsigned int __var2;
extern unsigned int __var3;
extern unsigned int __var4;

unsigned long __stdcall InitAll( void * );


#endif



