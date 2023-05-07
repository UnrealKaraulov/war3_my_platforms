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

#ifndef GGB_H
#define GGB_H

// standard integer sizes for 64 bit compatibility

#ifdef WIN32
 #include "ms_stdint.h"

#else
 #include <stdint.h>
#endif

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


void CONSOLE_Print( string message);
//
// CGHostGamesBroadcaster
//

class CTCPServer;
class CTCPSocket;
class CTCPClient;
class CUDPSocket;
class CGameProtocol;
class CGPSProtocol;
class CCommandPacket;
class CGPG;
class CIncomingGameHost;
class CGame;

class CGHostGamesBroadcaster
{
public:
	bool m_Exiting;                     // are we going to exit oeo? :P
	string m_Hostname;                  // remote ghost server hostname
	uint16_t m_Port;                    // remote ghost server port 
	string m_Version;			        // version string
	CUDPSocket *m_UDPSocket;		    // our socket for broadcasting games in the lan screen
	CTCPServer *m_GameServer;	        // server for accepting game connections
	CTCPClient *m_Socket;			    // socket for connecting to the ghost server
	vector<CGPG *> m_Cons;				// our game connections
	uint32_t m_LastConnectionAttemptTime;
	uint16_t m_ListeningPort;
	CGameProtocol *m_Protocol;
	queue<CCommandPacket *> m_Packets;
	vector<CGame * > m_Games;


	CGHostGamesBroadcaster(string nHostname,uint16_t nPort);
	~CGHostGamesBroadcaster( );

	// processing functions

	bool Update( long usecBlock );
	
	void ExtractPackets( );
	void ProcessPackets( );

	void Event_GameInfo( BYTEARRAY data );



};
class CGame
{
	string m_GameName;
	uint32_t m_UniqueID;
	uint16_t m_Port;
	bool m_SupportsGProxy;
public :
	CGame( string nGameName, uint32_t nUniqueID,uint16_t nPort,bool nGproxyGame);
	~CGame( );


	string GetGameName( )			   { return m_GameName; }
	void SetUniqueID( uint32_t value ) { m_UniqueID = value;}
	uint32_t GetUniqueID( )			   { return m_UniqueID; }
	uint16_t GetPort( )				   { return m_Port; }
	bool GetGProxySupport( )           { return m_SupportsGProxy;}
};
class CGPG
{
private:
	CTCPSocket *m_LocalSocket;
	CTCPClient *m_RemoteSocket;
	CGameProtocol *m_GameProtocol;
	CGPSProtocol *m_GPSProtocol;
	queue<CCommandPacket *> m_LocalPackets;
	queue<CCommandPacket *> m_RemotePackets;
	queue<CCommandPacket *> m_PacketBuffer;
	vector<unsigned char> m_Laggers;
	uint32_t m_TotalPacketsReceivedFromLocal;
	uint32_t m_TotalPacketsReceivedFromRemote;
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
	string m_GameName;


	void ExtractLocalPackets( );
	void ProcessLocalPackets( );
	void ExtractRemotePackets( );
	void ProcessRemotePackets( );
	
	void SendEmptyAction( );
	void SendLocalChat( string message );
public:

	CGPG( CTCPSocket *socket);
	~CGPG( );

	bool Update( void *fd , void *send_fd );
	unsigned int SetFD( void *fd ,void *send_fd , int *nfds );

	string GetGameName( ) { return m_GameName;}
};
#endif
