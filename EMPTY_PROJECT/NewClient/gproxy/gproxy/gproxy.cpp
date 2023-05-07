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

// todotodo: GHost++ may drop the player even after they reconnect if they run out of time and haven't caught up yet

#include "gproxy.h"
#include "util.h"
#include "socket.h"
#include "commandpacket.h"
#include "bnetprotocol.h"
#include "bnet.h"
#include "gameprotocol.h"
#include "gpsprotocol.h"
#include "ohsystem.h"


#include <signal.h>
#include <stdlib.h>

#ifdef WIN32
#include <windows.h>
#include <winsock.h>
#endif

#include <time.h>

#ifndef WIN32
#include <sys/time.h>
#endif

#ifdef __APPLE__
#include <mach/mach_time.h>
#endif

#include <queue>
using namespace std;

string gLogFile;
CGProxy *gGProxy = NULL;

uint32_t GetTime( )
{
	return GetTicks( ) / 1000;
}

uint32_t GetTicks( )
{
#ifdef WIN32
	return timeGetTime( );
#elif __APPLE__
	uint64_t current = mach_absolute_time( );
	static mach_timebase_info_data_t info = { 0, 0 };
	// get timebase info
	if ( info.denom == 0 )
		mach_timebase_info( &info );
	uint64_t elapsednano = current * ( info.numer / info.denom );
	// convert ns to ms
	return elapsednano / 1e6;
#else
	uint32_t ticks;
	struct timespec t;
	clock_gettime( CLOCK_MONOTONIC, &t );
	ticks = t.tv_sec * 1000;
	ticks += t.tv_nsec / 1000000;
	return ticks;
#endif
}

void LOG_Print( string message )
{
	if ( !gLogFile.empty( ) )
	{
		ofstream Log;
		Log.open( gLogFile.c_str( ), ios::app );

		if ( !Log.fail( ) )
		{
			time_t Now = time( NULL );
			string Time = asctime( localtime( &Now ) );

			// erase the newline

			Time.erase( Time.size( ) - 1 );
			Log << "[" << Time << "] " << message << endl;
			Log.close( );
		}
	}
}

void CONSOLE_Print(string message, bool toMainBuffer)
{

	//cout << message << endl;

	// logging
	LOG_Print(message);
}


void hexdump_string(unsigned char* data, unsigned int datalen, char* dst, unsigned int counter)
{
	unsigned int c;
	int tlen = 0;
	unsigned char* datatmp;

	datatmp = data;
	tlen += std::sprintf((dst + tlen), "%04X:   ", counter);

	for (c = 0; c < 8; c++) /* left half of hex dump */
		if (c < datalen)
			tlen += std::sprintf((dst + tlen), "%02X ", *(datatmp++));
		else
			tlen += std::sprintf((dst + tlen), "   "); /* pad if short line */

	tlen += std::sprintf((dst + tlen), "  ");

	for (c = 8; c < 16; c++) /* right half of hex dump */
		if (c < datalen)
			tlen += std::sprintf((dst + tlen), "%02X ", *(datatmp++));
		else
			tlen += std::sprintf((dst + tlen), "   "); /* pad if short line */

	tlen += std::sprintf((dst + tlen), "   ");

	for (c = 0, datatmp = data; c < 16; c++, datatmp++) /* ASCII dump */
		if (c < datalen) {
			if (*datatmp >= 32 && *datatmp < 127)
				tlen += std::sprintf((dst + tlen), "%c", *datatmp);
			else
				tlen += std::sprintf((dst + tlen), "."); /* put this for non-printables */
		}

}

void CONSOLE_Print(BYTEARRAY message)
{
	unsigned int i;
	char dst[100];
	char dst2[100];
	unsigned char* datac;

	if (!message.size()) {
		return;
	}

	for (i = 0, datac = (unsigned char*)&message[0]; i < message.size(); i += 16, datac += 16)
	{
		hexdump_string(datac, (message.size() - i < 16) ? (message.size() - i) : 16, dst, i);

		CONSOLE_Print(dst);
	}
}

void CONSOLE_PrintNoCRLF( string message )
{
	//cout << message;
}

void CONSOLE_ChangeChannel( string channel )
{
	ProcessCommandsCallback( "/channel", channel.c_str( ) );
}

void CONSOLE_AddChannelUser( string name, int flag )
{
	ProcessCommandsCallback( "/addchatuser", name.c_str( ) );
}

void CONSOLE_UpdateChannelUser( string name, int flag )
{
	//
}

void CONSOLE_RemoveChannelUser( string name )
{
	ProcessCommandsCallback( "/removechatuser", name.c_str( ) );
}

void CONSOLE_RemoveChannelUsers( )
{
	ProcessCommandsCallback( "/clearchatusers", "" );
}


string c_joinedGameName;

//
// main
//

void restartApp( )
{
	SHELLEXECUTEINFO ShExecInfo = { 0 };
	ShExecInfo.cbSize = sizeof( SHELLEXECUTEINFO );
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpVerb = NULL;
	ShExecInfo.lpFile = L"update.bat";
	ShExecInfo.lpParameters = L"";
	ShExecInfo.lpDirectory = NULL;
	ShExecInfo.nShow = SW_HIDE;
	ShExecInfo.hInstApp = NULL;
	ShellExecuteEx( &ShExecInfo );
	WaitForSingleObject( ShExecInfo.hProcess, INFINITE );
}


std::string War3path, Server, Username, Password, Channel;


HANDLE PROXYTHREADHANDLE = NULL;

DWORD WINAPI PROXYTHREAD( LPVOID )
{
	CONSOLE_Print( "[GPROXY] starting winsock" );
	WSADATA wsadata;
	if ( WSAStartup( MAKEWORD( 2, 2 ), &wsadata ) != 0 )
	{
		CONSOLE_Print( "[GPROXY] error starting winsock" );
		return 0;
	}


#ifndef WIN32
	// disable SIGPIPE since some systems like OS X don't define MSG_NOSIGNAL

	signal( SIGPIPE, SIG_IGN );
#endif

#ifdef WIN32
	// initialize winsock



	// increase process priority

	CONSOLE_Print( "[GPROXY] setting process priority to \"above normal\"" );
	SetPriorityClass( GetCurrentProcess( ), ABOVE_NORMAL_PRIORITY_CLASS );
#endif

	gGProxy = new CGProxy( true, War3path, "FFFFFFFFFFFFFFFFFFFFFFFFFF", "FFFFFFFFFFFFFFFFFFFFFFFFFF", Server, Username, Password, Channel, 26, 6125, BYTEARRAY( ), BYTEARRAY( ), "pvpgn" );

	while ( 1 )
	{
		if ( gGProxy->Update( 40000 ) )
			break;
	}

	MessageBoxA( 0, "GPROXY END", "ALL", 0 );

	// shutdown gproxy

	CONSOLE_Print( "[GPROXY] shutting down" );
	delete gGProxy;
	gGProxy = NULL;


#ifdef WIN32
	// shutdown winsock

	CONSOLE_Print( "[GPROXY] shutting down winsock" );
	WSACleanup( );
#endif
	return 0;
}

//new
void WINAPI RegisterNewAccount( const char * username, const char * password, const char * email )
{
	if ( gGProxy )
	{
		if ( gGProxy->m_BNET && gGProxy->m_BNET->m_Socket)
		{
			if ( !gGProxy->m_BNET->m_Socket->HasError( ) && gGProxy->m_BNET->m_Socket->GetConnected( ) )
			{
			//	MessageBoxA( 0, "SendRegisterPacket 4", "ALL", 0 );

				BYTEARRAY buildpacket = BYTEARRAY( );
				uint32_t packetid = 0;
				UTIL_AppendByteArray( buildpacket, packetid, 4 );
				UTIL_AppendByteArrayFast( buildpacket, string( username ) );		// Account Name
				UTIL_AppendByteArrayFast( buildpacket, string( password ) );		// Account Name
				UTIL_AppendByteArrayFast( buildpacket, string( email ) );		// Account Name
				gGProxy->m_BNET->m_Socket->PutBytes( gGProxy->m_BNET->m_Protocol->SEND_SID_WC3_CLIENT( buildpacket ) );
			}
			else if ( gGProxy->m_BNET->m_Socket->HasError( ) )
			{
				MessageBoxA( 0, "SendRegisterPacket ERROR 1", "ALL", 0 );
			}
			else if ( !gGProxy->m_BNET->m_Socket->GetConnected( ) )
			{
				MessageBoxA( 0, "SendRegisterPacket ERROR 2", "ALL", 0 );
			}
		}
		else
		{
			MessageBoxA( 0, "SendRegisterPacket ERROR 4", "ALL", 0 );
		}
	}
	else
	{
		MessageBoxA( 0, "SendRegisterPacket ERROR 3", "ALL", 0 );
	}
}

void WINAPI InitializeW3Proxy( const char * war3path, const char * server, const char * username, const char * password, const char * channel )
{
	if ( PROXYTHREADHANDLE != NULL )
	{
		TerminateThread( PROXYTHREADHANDLE, 0 );
		//.
		PROXYTHREADHANDLE = NULL;
	}


	if ( gGProxy )
	{
		// shutdown gproxy

		CONSOLE_Print( "[GPROXY] shutting down" );
		delete gGProxy;
		gGProxy = NULL;


#ifdef WIN32
		// shutdown winsock

		CONSOLE_Print( "[GPROXY] shutting down winsock" );
		WSACleanup( );
#endif
	}
	// read config file

	gLogFile = "NewPlatform_proxy.log";

	UTIL_Construct_UTF8_Latin1_Map( );

	CONSOLE_Print( "[GPROXY] starting up" );

	War3path = war3path;
	Server = server;
	Username = username;
	Password = password;
	Channel = channel;

	LoginStatus = 0;
	PROXYTHREADHANDLE = CreateThread( 0, 0, PROXYTHREAD, 0, 0, 0 );
}

//
// CGProxy
//

CGProxy::CGProxy( bool nTFT, string nWar3Path, string nCDKeyROC, string nCDKeyTFT, string nServer, string nUsername, string nPassword, string nChannel, uint32_t nWar3Version, uint16_t nPort, BYTEARRAY nEXEVersion, BYTEARRAY nEXEVersionHash, string nPasswordHashType )
{

	//CONFIGURATION NEEDED!
	c_BotName = "ChannelBotName";
	m_Version = "OHSystem GProxy++ Mod v1.01";
	m_LocalServer = new CTCPServer( );
	m_LocalSocket = NULL;
	m_RemoteSocket = new CTCPClient( );
	m_RemoteSocket->SetNoDelay( true );
	m_UDPSocket = new CUDPSocket( );
	m_UDPSocket->SetBroadcastTarget( "127.0.0.1" );
	m_GameProtocol = new CGameProtocol( this );
	m_GPSProtocol = new CGPSProtocol( );
	m_TotalPacketsReceivedFromLocal = 0;
	m_TotalPacketsReceivedFromRemote = 0;
	m_Exiting = false;
	m_TFT = nTFT;
	m_War3Path = nWar3Path;
	m_CDKeyROC = nCDKeyROC;
	m_CDKeyTFT = nCDKeyTFT;
	m_Server = nServer;
	m_Username = nUsername;
	m_Password = nPassword;
	m_Channel = nChannel;
	m_War3Version = nWar3Version;
	m_Port = nPort;
	m_LastConnectionAttemptTime = 0;
	m_LastRefreshTime = 0;
	m_RemoteServerPort = 0;
	m_GameIsReliable = false;
	m_GameStarted = false;
	m_LeaveGameSent = false;
	m_ActionReceived = false;
	m_Synchronized = true;
	m_ReconnectPort = 0;
	m_PID = 255;
	m_ChatPID = 255;
	m_ReconnectKey = 0;
	m_NumEmptyActions = 0;
	m_NumEmptyActionsUsed = 0;
	m_LastAckTime = 0;
	m_LastActionTime = 0;
	m_BNET = new CBNET( this, m_Server, string( ), 0, 0, m_CDKeyROC, m_CDKeyTFT, "USA", "United States", m_Username, m_Password, m_Channel, m_War3Version, nEXEVersion, nEXEVersionHash, nPasswordHashType, 200 );
	m_LocalServer->Listen( string( ), m_Port );
	m_SendInfo = false;
	CONSOLE_Print( m_Version );
}

CGProxy :: ~CGProxy( )
{
	for ( vector<CIncomingGameHost *> ::iterator i = m_Games.begin( ); i != m_Games.end( ); i++ )
		m_UDPSocket->Broadcast( 6112, m_GameProtocol->SEND_W3GS_DECREATEGAME( ( *i )->GetUniqueGameID( ) ) );

	delete m_LocalServer;
	delete m_LocalSocket;
	delete m_RemoteSocket;
	delete m_UDPSocket;
	delete m_BNET;

	for ( vector<CIncomingGameHost *> ::iterator i = m_Games.begin( ); i != m_Games.end( ); i++ )
		delete *i;

	delete m_GameProtocol;
	delete m_GPSProtocol;

	while ( !m_LocalPackets.empty( ) )
	{
		delete m_LocalPackets.front( );
		m_LocalPackets.pop( );
	}

	while ( !m_RemotePackets.empty( ) )
	{
		delete m_RemotePackets.front( );
		m_RemotePackets.pop( );
	}

	while ( !m_PacketBuffer.empty( ) )
	{
		delete m_PacketBuffer.front( );
		m_PacketBuffer.pop( );
	}
}
bool CGProxy::Update( long usecBlock )
{
	unsigned int NumFDs = 0;

	// take every socket we own and throw it in one giant select statement so we can block on all sockets

	int nfds = 0;
	fd_set fd;
	fd_set send_fd;
	FD_ZERO( &fd );
	FD_ZERO( &send_fd );

	// 1. the battle.net socket

	NumFDs += m_BNET->SetFD( &fd, &send_fd, &nfds );

	// 2. the local server

	m_LocalServer->SetFD( &fd, &send_fd, &nfds );
	NumFDs++;

	// 3. the local socket

	if ( m_LocalSocket )
	{
		m_LocalSocket->SetFD( &fd, &send_fd, &nfds );
		NumFDs++;
	}

	// 4. the remote socket

	if ( !m_RemoteSocket->HasError( ) && m_RemoteSocket->GetConnected( ) )
	{
		m_RemoteSocket->SetFD( &fd, &send_fd, &nfds );
		NumFDs++;
	}

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = usecBlock;

	struct timeval send_tv;
	send_tv.tv_sec = 0;
	send_tv.tv_usec = 0;

#ifdef WIN32
	select( 1, &fd, NULL, NULL, &tv );
	select( 1, NULL, &send_fd, NULL, &send_tv );
#else
	select( nfds + 1, &fd, NULL, NULL, &tv );
	select( nfds + 1, NULL, &send_fd, NULL, &send_tv );
#endif

	if ( NumFDs == 0 )
		MILLISLEEP( 50 );

	if ( m_BNET->Update( &fd, &send_fd ) )
		return true;

	//
	// accept new connections
	//

	CTCPSocket *NewSocket = m_LocalServer->Accept( &fd );

	if ( NewSocket )
	{
		if ( m_LocalSocket )
		{
			// someone's already connected, reject the new connection
			// we only allow one person to use the proxy at a time

			delete NewSocket;
		}
		else
		{
			CONSOLE_Print( "[GPROXY] local player connected" );
			std::queue<BYTEARRAY> empty;
			std::swap( m_BNET->m_OutPackets, empty );
			m_LocalSocket = NewSocket;
			m_LocalSocket->SetNoDelay( true );
			m_TotalPacketsReceivedFromLocal = 0;
			m_TotalPacketsReceivedFromRemote = 0;
			m_GameIsReliable = false;
			m_GameStarted = false;
			m_LeaveGameSent = false;
			m_ActionReceived = false;
			m_Synchronized = true;
			m_ReconnectPort = 0;
			m_PID = 255;
			m_ChatPID = 255;
			m_ReconnectKey = 0;
			m_NumEmptyActions = 0;
			m_NumEmptyActionsUsed = 0;
			m_LastAckTime = 0;
			m_LastActionTime = 0;
			m_JoinedName.clear( );
			m_HostName.clear( );

			while ( !m_PacketBuffer.empty( ) )
			{
				delete m_PacketBuffer.front( );
				m_PacketBuffer.pop( );
			}
		}
	}

	if ( m_LocalSocket )
	{
		//
		// handle proxying (reconnecting, etc...)
		//

		if ( m_LocalSocket->HasError( ) || !m_LocalSocket->GetConnected( ) )
		{
			CONSOLE_Print( "[GPROXY] local player disconnected" );

			if ( m_BNET->GetInGame( ) )
				m_BNET->QueueEnterChat( );

			delete m_LocalSocket;
			m_LocalSocket = NULL;

			// ensure a leavegame message was sent, otherwise the server may wait for our reconnection which will never happen
			// if one hasn't been sent it's because Warcraft III exited abnormally

			if ( m_GameIsReliable && !m_LeaveGameSent )
			{
				// note: we're not actually 100% ensuring the leavegame message is sent, we'd need to check that DoSend worked, etc...

				BYTEARRAY LeaveGame;
				LeaveGame.push_back( 0xF7 );
				LeaveGame.push_back( 0x21 );
				LeaveGame.push_back( 0x08 );
				LeaveGame.push_back( 0x00 );
				UTIL_AppendByteArray( LeaveGame, ( uint32_t )PLAYERLEAVE_GPROXY, false );
				m_RemoteSocket->PutBytes( LeaveGame );
				m_RemoteSocket->DoSend( &send_fd );
			}

			m_RemoteSocket->Reset( );
			m_RemoteSocket->SetNoDelay( true );
			m_RemoteServerIP.clear( );
			m_RemoteServerPort = 0;
		}
		else
		{
			m_LocalSocket->DoRecv( &fd );
			ExtractLocalPackets( );
			ProcessLocalPackets( );

			if ( !m_RemoteServerIP.empty( ) )
			{
				if ( m_GameIsReliable && m_ActionReceived && GetTime( ) - m_LastActionTime >= 60 )
				{
					if ( m_NumEmptyActionsUsed < m_NumEmptyActions )
					{
						SendEmptyAction( );
						m_NumEmptyActionsUsed++;
					}
					else
					{
						SendLocalChat( "GProxy++ ran out of time to reconnect, Warcraft III will disconnect soon." );
						CONSOLE_Print( "[GPROXY] ran out of time to reconnect" );
					}

					m_LastActionTime = GetTime( );
				}

				if ( m_RemoteSocket->HasError( ) )
				{
					CONSOLE_Print( "[GPROXY] disconnected from remote server due to socket error" );

					if ( m_GameIsReliable && m_ActionReceived && m_ReconnectPort > 0 )
					{
						SendLocalChat( "You have been disconnected from the server due to a socket error." );
						uint32_t TimeRemaining = ( m_NumEmptyActions - m_NumEmptyActionsUsed + 1 ) * 60 - ( GetTime( ) - m_LastActionTime );

						if ( GetTime( ) - m_LastActionTime > ( m_NumEmptyActions - m_NumEmptyActionsUsed + 1 ) * 60 )
							TimeRemaining = 0;

						SendLocalChat( "GProxy++ is attempting to reconnect... (" + UTIL_ToString( TimeRemaining ) + " seconds remain)" );
						CONSOLE_Print( "[GPROXY] attempting to reconnect" );
						m_RemoteSocket->Reset( );
						m_RemoteSocket->SetNoDelay( true );
						m_RemoteSocket->Connect( string( ), m_RemoteServerIP, m_ReconnectPort );
						m_LastConnectionAttemptTime = GetTime( );
					}
					else
					{
						if ( m_BNET->GetInGame( ) )
							m_BNET->QueueEnterChat( );

						m_LocalSocket->Disconnect( );
						delete m_LocalSocket;
						m_LocalSocket = NULL;
						m_RemoteSocket->Reset( );
						m_RemoteSocket->SetNoDelay( true );
						m_RemoteServerIP.clear( );
						m_RemoteServerPort = 0;
						return false;
					}
				}

				if ( !m_RemoteSocket->GetConnecting( ) && !m_RemoteSocket->GetConnected( ) )
				{
					CONSOLE_Print( "[GPROXY] disconnected from remote server" );

					if ( m_GameIsReliable && m_ActionReceived && m_ReconnectPort > 0 )
					{
						SendLocalChat( "You have been disconnected from the server." );
						uint32_t TimeRemaining = ( m_NumEmptyActions - m_NumEmptyActionsUsed + 1 ) * 60 - ( GetTime( ) - m_LastActionTime );

						if ( GetTime( ) - m_LastActionTime > ( m_NumEmptyActions - m_NumEmptyActionsUsed + 1 ) * 60 )
							TimeRemaining = 0;

						SendLocalChat( "GProxy++ is attempting to reconnect... (" + UTIL_ToString( TimeRemaining ) + " seconds remain)" );
						CONSOLE_Print( "[GPROXY] attempting to reconnect" );
						m_RemoteSocket->Reset( );
						m_RemoteSocket->SetNoDelay( true );
						m_RemoteSocket->Connect( string( ), m_RemoteServerIP, m_ReconnectPort );
						m_LastConnectionAttemptTime = GetTime( );
					}
					else
					{
						if ( m_BNET->GetInGame( ) )
							m_BNET->QueueEnterChat( );

						m_LocalSocket->Disconnect( );
						delete m_LocalSocket;
						m_LocalSocket = NULL;
						m_RemoteSocket->Reset( );
						m_RemoteSocket->SetNoDelay( true );
						m_RemoteServerIP.clear( );
						m_RemoteServerPort = 0;
						return false;
					}
				}

				if ( m_RemoteSocket->GetConnected( ) )
				{
					if ( m_GameIsReliable && m_ActionReceived && m_ReconnectPort > 0 && GetTime( ) - m_RemoteSocket->GetLastRecv( ) >= 20 )
					{
						CONSOLE_Print( "[GPROXY] disconnected from remote server due to 20 second timeout" );
						SendLocalChat( "You have been timed out from the server." );
						uint32_t TimeRemaining = ( m_NumEmptyActions - m_NumEmptyActionsUsed + 1 ) * 60 - ( GetTime( ) - m_LastActionTime );

						if ( GetTime( ) - m_LastActionTime > ( m_NumEmptyActions - m_NumEmptyActionsUsed + 1 ) * 60 )
							TimeRemaining = 0;

						SendLocalChat( "GProxy++ is attempting to reconnect... (" + UTIL_ToString( TimeRemaining ) + " seconds remain)" );
						CONSOLE_Print( "[GPROXY] attempting to reconnect" );
						m_RemoteSocket->Reset( );
						m_RemoteSocket->SetNoDelay( true );
						m_RemoteSocket->Connect( string( ), m_RemoteServerIP, m_ReconnectPort );
						m_LastConnectionAttemptTime = GetTime( );
					}
					else
					{
						m_RemoteSocket->DoRecv( &fd );
						ExtractRemotePackets( );
						ProcessRemotePackets( );

						if ( m_GameIsReliable && m_ActionReceived && m_ReconnectPort > 0 && GetTime( ) - m_LastAckTime >= 10 )
						{
							m_RemoteSocket->PutBytes( m_GPSProtocol->SEND_GPSC_ACK( m_TotalPacketsReceivedFromRemote ) );
							m_LastAckTime = GetTime( );
						}

						m_RemoteSocket->DoSend( &send_fd );
					}
				}

				if ( m_RemoteSocket->GetConnecting( ) )
				{
					// we are currently attempting to connect

					if ( m_RemoteSocket->CheckConnect( ) )
					{
						// the connection attempt completed

						if ( m_GameIsReliable && m_ActionReceived )
						{
							// this is a reconnection, not a new connection
							// if the server accepts the reconnect request it will send a GPS_RECONNECT back requesting a certain number of packets

							SendLocalChat( "GProxy++ reconnected to the server!" );
							SendLocalChat( "==================================================" );
							CONSOLE_Print( "[GPROXY] reconnected to remote server" );

							// note: even though we reset the socket when we were disconnected, we haven't been careful to ensure we never queued any data in the meantime
							// therefore it's possible the socket could have data in the send buffer
							// this is bad because the server will expect us to send a GPS_RECONNECT message first
							// so we must clear the send buffer before we continue
							// note: we aren't losing data here, any important messages that need to be sent have been put in the packet buffer
							// they will be requested by the server if required

							m_RemoteSocket->ClearSendBuffer( );
							m_RemoteSocket->PutBytes( m_GPSProtocol->SEND_GPSC_RECONNECT( m_PID, m_ReconnectKey, m_TotalPacketsReceivedFromRemote ) );

							// we cannot permit any forwarding of local packets until the game is synchronized again
							// this will disable forwarding and will be reset when the synchronization is complete

							m_Synchronized = false;
						}
						else
							CONSOLE_Print( "[GPROXY] connected to remote server" );
					}
					else if ( GetTime( ) - m_LastConnectionAttemptTime >= 10 )
					{
						// the connection attempt timed out (10 seconds)

						CONSOLE_Print( "[GPROXY] connect to remote server timed out" );

						if ( m_GameIsReliable && m_ActionReceived && m_ReconnectPort > 0 )
						{
							uint32_t TimeRemaining = ( m_NumEmptyActions - m_NumEmptyActionsUsed + 1 ) * 60 - ( GetTime( ) - m_LastActionTime );

							if ( GetTime( ) - m_LastActionTime > ( m_NumEmptyActions - m_NumEmptyActionsUsed + 1 ) * 60 )
								TimeRemaining = 0;

							SendLocalChat( "GProxy++ is attempting to reconnect... (" + UTIL_ToString( TimeRemaining ) + " seconds remain)" );
							CONSOLE_Print( "[GPROXY] attempting to reconnect" );
							m_RemoteSocket->Reset( );
							m_RemoteSocket->SetNoDelay( true );
							m_RemoteSocket->Connect( string( ), m_RemoteServerIP, m_ReconnectPort );
							m_LastConnectionAttemptTime = GetTime( );
						}
						else
						{
							if ( m_BNET->GetInGame( ) )
								m_BNET->QueueEnterChat( );

							m_LocalSocket->Disconnect( );
							delete m_LocalSocket;
							m_LocalSocket = NULL;
							m_RemoteSocket->Reset( );
							m_RemoteSocket->SetNoDelay( true );
							m_RemoteServerIP.clear( );
							m_RemoteServerPort = 0;
							return false;
						}
					}
				}
			}

			m_LocalSocket->DoSend( &send_fd );
		}
	}
	else
	{
		//
		// handle game listing
		//
		//CIncomingGameHost * game=NULL;
		if ( GetTime( ) - m_LastRefreshTime >= 2 )
		{
			for ( vector<CIncomingGameHost *> ::iterator i = m_Games.begin( ); i != m_Games.end( ); )
			{
				// expire games older than 60 seconds

				if ( GetTime( ) - ( *i )->GetReceivedTime( ) >= 60 )
				{
					// don't forget to remove it from the LAN list first

					m_UDPSocket->Broadcast( 6112, m_GameProtocol->SEND_W3GS_DECREATEGAME( ( *i )->GetUniqueGameID( ) ) );
					delete *i;
					i = m_Games.erase( i );
					continue;
				}

				BYTEARRAY MapGameType;
				UTIL_AppendByteArray( MapGameType, ( *i )->GetGameType( ), false );
				UTIL_AppendByteArray( MapGameType, ( *i )->GetParameter( ), false );
				BYTEARRAY MapFlags = UTIL_CreateByteArray( ( *i )->GetMapFlags( ), false );
				BYTEARRAY MapWidth = UTIL_CreateByteArray( ( *i )->GetMapWidth( ), false );
				BYTEARRAY MapHeight = UTIL_CreateByteArray( ( *i )->GetMapHeight( ), false );
				string GameName = ( *i )->GetGameName( );

				// colour reliable game names so they're easier to pick out of the list

				if ( ( *i )->GetMapWidth( ) == 1984 && ( *i )->GetMapHeight( ) == 1984 )
				{
					GameName = "|cFF4080C0" + GameName;

					// unfortunately we have to truncate them
					// is this acceptable?

					if ( GameName.size( ) > 31 )
						GameName = GameName.substr( 0, 31 );
				}


				m_UDPSocket->Broadcast( 6112, m_GameProtocol->SEND_W3GS_GAMEINFO( m_TFT, m_War3Version, MapGameType, MapFlags, MapWidth, MapHeight, GameName, ( *i )->GetHostName( ), ( *i )->GetElapsedTime( ), ( *i )->GetMapPath( ), ( *i )->GetMapCRC( ), 12, 12, m_Port, ( *i )->GetUniqueGameID( ), ( *i )->GetUniqueGameID( ) ) );

				i++;
			}

			m_LastRefreshTime = GetTime( );
		}
	}
	return m_Exiting;
}

void CGProxy::ExtractLocalPackets( )
{
	if ( !m_LocalSocket )
		return;

	string *RecvBuffer = m_LocalSocket->GetBytes( );
	BYTEARRAY Bytes = UTIL_CreateByteArray( ( unsigned char * )RecvBuffer->c_str( ), RecvBuffer->size( ) );

	// a packet is at least 4 bytes so loop as long as the buffer contains 4 bytes

	while ( Bytes.size( ) >= 4 )
	{
		// byte 0 is always 247

		if ( Bytes[ 0 ] == W3GS_HEADER_CONSTANT )
		{
			// bytes 2 and 3 contain the length of the packet

			uint16_t Length = UTIL_ByteArrayToUInt16( Bytes, false, 2 );

			if ( Length >= 4 )
			{
				if ( Bytes.size( ) >= Length )
				{
					// we have to do a little bit of packet processing here
					// this is because we don't want to forward any chat messages that start with a "/" as these may be forwarded to battle.net instead
					// in fact we want to pretend they were never even received from the proxy's perspective

					bool Forward = true;
					BYTEARRAY Data = BYTEARRAY( Bytes.begin( ), Bytes.begin( ) + Length );

					if ( Bytes[ 1 ] == CGameProtocol::W3GS_CHAT_TO_HOST )
					{
						if ( Data.size( ) >= 5 )
						{
							unsigned int i = 5;
							unsigned char Total = Data[ 4 ];

							if ( Total > 0 && Data.size( ) >= i + Total )
							{
								i += Total;
								unsigned char Flag = Data[ i + 1 ];
								i += 2;

								string MessageString;

								if ( Flag == 16 && Data.size( ) >= i + 1 )
								{
									BYTEARRAY Message = UTIL_ExtractCString( Data, i );
									MessageString = string( Message.begin( ), Message.end( ) );
								}
								else if ( Flag == 32 && Data.size( ) >= i + 5 )
								{
									BYTEARRAY Message = UTIL_ExtractCString( Data, i + 4 );
									MessageString = string( Message.begin( ), Message.end( ) );
								}

								string Command = MessageString;
								transform( Command.begin( ), Command.end( ), Command.begin( ), ( int( *)( int ) )tolower );

								if ( Command.size( ) >= 1 && Command.substr( 0, 1 ) == "/" )
								{
									Forward = false;

									if ( Command.size( ) >= 5 && Command.substr( 0, 4 ) == "/re " )
									{
										if ( m_BNET->GetLoggedIn( ) )
										{
											if ( !m_BNET->GetReplyTarget( ).empty( ) )
											{
												m_BNET->QueueChatCommand( MessageString.substr( 4 ), m_BNET->GetReplyTarget( ), true );
											}
											else
												SendLocalChat( "Nobody has whispered you yet." );
										}
										else
											SendLocalChat( "You are not connected to battle.net." );
									}
									else if ( Command == "/sc" || Command == "/spoof" || Command == "/spoofcheck" || Command == "/spoof check" )
									{
										if ( m_BNET->GetLoggedIn( ) )
										{
											if ( !m_GameStarted )
											{
												m_BNET->QueueChatCommand( "spoofcheck", m_HostName, true );
											}
											else
												SendLocalChat( "The game has already started." );
										}
										else
											SendLocalChat( "You are not connected to battle.net." );
									}
									else if ( Command == "/status" )
									{
										if ( m_LocalSocket )
										{
											if ( m_GameIsReliable && m_ReconnectPort > 0 )
												SendLocalChat( "GProxy++ disconnect protection: Enabled" );
											else
												SendLocalChat( "GProxy++ disconnect protection: Disabled" );

											if ( m_BNET->GetLoggedIn( ) )
												SendLocalChat( "battle.net: Connected" );
											else
												SendLocalChat( "battle.net: Disconnected" );
										}
									}
									else if ( Command.size( ) >= 4 && Command.substr( 0, 3 ) == "/w " )
									{
										if ( m_BNET->GetLoggedIn( ) )
										{
											m_BNET->QueueChatCommand( MessageString );
										}
										else
											SendLocalChat( "You are not connected to battle.net." );
									}

								}
							}
						}
					}

					if ( Forward )
					{
						m_LocalPackets.push( new CCommandPacket( W3GS_HEADER_CONSTANT, Bytes[ 1 ], Data ) );
						m_PacketBuffer.push( new CCommandPacket( W3GS_HEADER_CONSTANT, Bytes[ 1 ], Data ) );
						m_TotalPacketsReceivedFromLocal++;
					}

					*RecvBuffer = RecvBuffer->substr( Length );
					Bytes = BYTEARRAY( Bytes.begin( ) + Length, Bytes.end( ) );
				}
				else
					return;
			}
			else
			{
				CONSOLE_Print( "[GPROXY] received invalid packet from local player (bad length)" );
				m_Exiting = true;
				return;
			}
		}
		else
		{
			CONSOLE_Print( "[GPROXY] received invalid packet from local player (bad header constant)" );
			m_Exiting = true;
			return;
		}
	}
}

void CGProxy::ProcessLocalPackets( )
{
	if ( !m_LocalSocket )
		return;

	while ( !m_LocalPackets.empty( ) )
	{
		CCommandPacket *Packet = m_LocalPackets.front( );
		m_LocalPackets.pop( );
		BYTEARRAY Data = Packet->GetData( );

		if ( Packet->GetPacketType( ) == W3GS_HEADER_CONSTANT )
		{
			if ( Packet->GetID( ) == CGameProtocol::W3GS_REQJOIN )
			{
				if ( Data.size( ) >= 20 )
				{
					// parse

					uint32_t HostCounter = UTIL_ByteArrayToUInt32( Data, false, 4 );
					uint32_t EntryKey = UTIL_ByteArrayToUInt32( Data, false, 8 );
					unsigned char Unknown = Data[ 12 ];
					uint16_t ListenPort = UTIL_ByteArrayToUInt16( Data, false, 13 );
					uint32_t PeerKey = UTIL_ByteArrayToUInt32( Data, false, 15 );
					BYTEARRAY Name = UTIL_ExtractCString( Data, 19 );
					string NameString = string( Name.begin( ), Name.end( ) );
					BYTEARRAY Remainder = BYTEARRAY( Data.begin( ) + Name.size( ) + 20, Data.end( ) );

					if ( Remainder.size( ) == 18 )
					{
						// lookup the game in the main list

						bool GameFound = false;

						for ( vector<CIncomingGameHost *> ::iterator i = m_Games.begin( ); i != m_Games.end( ); i++ )
						{
							if ( ( *i )->GetUniqueGameID( ) == EntryKey )
							{
								CONSOLE_Print( "[GPROXY] local player requested game name [" + ( *i )->GetGameName( ) + "]" );

								if ( NameString != m_Username )
									CONSOLE_Print( "[GPROXY] using battle.net name [" + m_Username + "] instead of requested name [" + NameString + "]" );

								CONSOLE_Print( "[GPROXY] connecting to remote server [" + ( *i )->GetIPString( ) + "] on port " + UTIL_ToString( ( *i )->GetPort( ) ) );
								m_RemoteServerIP = ( *i )->GetIPString( );
								m_RemoteServerPort = ( *i )->GetPort( );
								m_RemoteSocket->Reset( );
								m_RemoteSocket->SetNoDelay( true );
								m_RemoteSocket->Connect( string( ), m_RemoteServerIP, m_RemoteServerPort );
								m_LastConnectionAttemptTime = GetTime( );
								m_GameIsReliable = ( ( *i )->GetMapWidth( ) == 1984 && ( *i )->GetMapHeight( ) == 1984 );
								m_GameStarted = false;

								// rewrite packet

								BYTEARRAY DataRewritten;
								DataRewritten.push_back( W3GS_HEADER_CONSTANT );
								DataRewritten.push_back( Packet->GetID( ) );
								DataRewritten.push_back( 0 );
								DataRewritten.push_back( 0 );
								UTIL_AppendByteArray( DataRewritten, ( *i )->GetHostCounter( ), false );
								UTIL_AppendByteArray( DataRewritten, ( uint32_t )0, false );
								DataRewritten.push_back( Unknown );
								UTIL_AppendByteArray( DataRewritten, ListenPort, false );
								UTIL_AppendByteArray( DataRewritten, PeerKey, false );
								UTIL_AppendByteArray( DataRewritten, m_Username );
								UTIL_AppendByteArrayFast( DataRewritten, Remainder );
								BYTEARRAY LengthBytes;
								LengthBytes = UTIL_CreateByteArray( ( uint16_t )DataRewritten.size( ), false );
								DataRewritten[ 2 ] = LengthBytes[ 0 ];
								DataRewritten[ 3 ] = LengthBytes[ 1 ];
								Data = DataRewritten;

								// tell battle.net we're joining a game (for automatic spoof checking)

								m_BNET->QueueJoinGame( ( *i )->GetGameName( ) );

								// save the hostname for later (for manual spoof checking)

								m_JoinedName = NameString;
								c_joinedGameName = ( *i )->GetGameName( );
								m_HostName = ( *i )->GetHostName( );
								GameFound = true;
								break;
							}
							else
							{
							}
						}

						if ( !GameFound )
						{
							CONSOLE_Print( "[GPROXY] local player requested unknown game (expired?)" );
							m_LocalSocket->Disconnect( );
						}
					}
					else
						CONSOLE_Print( "[GPROXY] received invalid join request from local player (invalid remainder)" );
				}
				else
					CONSOLE_Print( "[GPROXY] received invalid join request from local player (too short)" );
			}
			else if ( Packet->GetID( ) == CGameProtocol::W3GS_LEAVEGAME )
			{
				m_LeaveGameSent = true;
				m_LocalSocket->Disconnect( );
			}
			else if ( Packet->GetID( ) == CGameProtocol::W3GS_CHAT_TO_HOST )
			{
				// handled in ExtractLocalPackets (yes, it's ugly)
			}
		}

		// warning: do not forward any data if we are not synchronized (e.g. we are reconnecting and resynchronizing)
		// any data not forwarded here will be cached in the packet buffer and sent later so all is well

		if ( m_RemoteSocket && m_Synchronized )
			m_RemoteSocket->PutBytes( Data );

		delete Packet;
	}
}

void CGProxy::ExtractRemotePackets( )
{
	string *RecvBuffer = m_RemoteSocket->GetBytes( );
	BYTEARRAY Bytes = UTIL_CreateByteArray( ( unsigned char * )RecvBuffer->c_str( ), RecvBuffer->size( ) );

	// a packet is at least 4 bytes so loop as long as the buffer contains 4 bytes

	while ( Bytes.size( ) >= 4 )
	{
		if ( Bytes[ 0 ] == W3GS_HEADER_CONSTANT || Bytes[ 0 ] == GPS_HEADER_CONSTANT )
		{
			// bytes 2 and 3 contain the length of the packet

			uint16_t Length = UTIL_ByteArrayToUInt16( Bytes, false, 2 );

			if ( Length >= 4 )
			{
				if ( Bytes.size( ) >= Length )
				{
					m_RemotePackets.push( new CCommandPacket( Bytes[ 0 ], Bytes[ 1 ], BYTEARRAY( Bytes.begin( ), Bytes.begin( ) + Length ) ) );

					if ( Bytes[ 0 ] == W3GS_HEADER_CONSTANT )
						m_TotalPacketsReceivedFromRemote++;

					*RecvBuffer = RecvBuffer->substr( Length );
					Bytes = BYTEARRAY( Bytes.begin( ) + Length, Bytes.end( ) );
				}
				else
					return;
			}
			else
			{
				CONSOLE_Print( "[GPROXY] received invalid packet from remote server (bad length)" );
				m_Exiting = true;
				return;
			}
		}
		else
		{
			CONSOLE_Print( "[GPROXY] received invalid packet from remote server (bad header constant)" );
			m_Exiting = true;
			return;
		}
	}
}

void CGProxy::ProcessRemotePackets( )
{
	if ( !m_LocalSocket || !m_RemoteSocket )
		return;

	while ( !m_RemotePackets.empty( ) )
	{
		CCommandPacket *Packet = m_RemotePackets.front( );
		m_RemotePackets.pop( );

		if ( Packet->GetPacketType( ) == W3GS_HEADER_CONSTANT )
		{
			if ( Packet->GetID( ) == CGameProtocol::W3GS_SLOTINFOJOIN )
			{
				BYTEARRAY Data = Packet->GetData( );

				if ( Data.size( ) >= 6 )
				{
					uint16_t SlotInfoSize = UTIL_ByteArrayToUInt16( Data, false, 4 );

					if ( Data.size( ) >= 7 + SlotInfoSize )
						m_ChatPID = Data[ 6 + SlotInfoSize ];
				}

				// send a GPS_INIT packet
				// if the server doesn't recognize it (e.g. it isn't GHost++) we should be kicked

				CONSOLE_Print( "[GPROXY] join request accepted by remote server" );

				if ( m_GameIsReliable )
				{
					CONSOLE_Print( "[GPROXY] detected reliable game, starting GPS handshake" );
					m_RemoteSocket->PutBytes( m_GPSProtocol->SEND_GPSC_INIT( 1 ) );
				}
				else
					CONSOLE_Print( "[GPROXY] detected standard game, disconnect protection disabled" );
			}
			else if ( Packet->GetID( ) == CGameProtocol::W3GS_COUNTDOWN_END )
			{
				if ( m_GameIsReliable && m_ReconnectPort > 0 )
					CONSOLE_Print( "[GPROXY] game started, disconnect protection enabled" );
				else
				{
					if ( m_GameIsReliable )
						CONSOLE_Print( "[GPROXY] game started but GPS handshake not complete, disconnect protection disabled" );
					else
						CONSOLE_Print( "[GPROXY] game started" );
				}
				int number = rand( ) % 2;
				/*if ( number == 1 )
					PlaySound( L"sounds\\game.wav", NULL, SND_FILENAME );
				else
					PlaySound( L"sounds\\blood.wav", NULL, SND_FILENAME );

*/
				m_GameStarted = true;
			}
			else if ( Packet->GetID( ) == CGameProtocol::W3GS_INCOMING_ACTION )
			{
				if ( m_GameIsReliable )
				{
					// we received a game update which means we can reset the number of empty actions we have to work with
					// we also must send any remaining empty actions now
					// note: the lag screen can't be up right now otherwise the server made a big mistake, so we don't need to check for it

					BYTEARRAY EmptyAction;
					EmptyAction.push_back( 0xF7 );
					EmptyAction.push_back( 0x0C );
					EmptyAction.push_back( 0x06 );
					EmptyAction.push_back( 0x00 );
					EmptyAction.push_back( 0x00 );
					EmptyAction.push_back( 0x00 );

					for ( unsigned char i = m_NumEmptyActionsUsed; i < m_NumEmptyActions; i++ )
						m_LocalSocket->PutBytes( EmptyAction );

					m_NumEmptyActionsUsed = 0;
				}

				m_ActionReceived = true;
				m_LastActionTime = GetTime( );
			}
			else if ( Packet->GetID( ) == CGameProtocol::W3GS_START_LAG )
			{
				if ( m_GameIsReliable )
				{
					BYTEARRAY Data = Packet->GetData( );

					if ( Data.size( ) >= 5 )
					{
						unsigned char NumLaggers = Data[ 4 ];

						if ( Data.size( ) == 5 + NumLaggers * 5 )
						{
							for ( unsigned char i = 0; i < NumLaggers; i++ )
							{
								bool LaggerFound = false;

								for ( vector<unsigned char> ::iterator j = m_Laggers.begin( ); j != m_Laggers.end( ); j++ )
								{
									if ( *j == Data[ 5 + i * 5 ] )
										LaggerFound = true;
								}

								if ( LaggerFound )
									CONSOLE_Print( "[GPROXY] warning - received start_lag on known lagger" );
								else
									m_Laggers.push_back( Data[ 5 + i * 5 ] );
							}
						}
						else
							CONSOLE_Print( "[GPROXY] warning - unhandled start_lag (2)" );
					}
					else
						CONSOLE_Print( "[GPROXY] warning - unhandled start_lag (1)" );
				}
			}
			else if ( Packet->GetID( ) == CGameProtocol::W3GS_STOP_LAG )
			{
				if ( m_GameIsReliable )
				{
					BYTEARRAY Data = Packet->GetData( );

					if ( Data.size( ) == 9 )
					{
						bool LaggerFound = false;

						for ( vector<unsigned char> ::iterator i = m_Laggers.begin( ); i != m_Laggers.end( ); )
						{
							if ( *i == Data[ 4 ] )
							{
								i = m_Laggers.erase( i );
								LaggerFound = true;
							}
							else
								i++;
						}

						if ( !LaggerFound )
							CONSOLE_Print( "[GPROXY] warning - received stop_lag on unknown lagger" );
					}
					else
						CONSOLE_Print( "[GPROXY] warning - unhandled stop_lag" );
				}
			}
			else if ( Packet->GetID( ) == CGameProtocol::W3GS_INCOMING_ACTION2 )
			{
				if ( m_GameIsReliable )
				{
					// we received a fractured game update which means we cannot use any empty actions until we receive the subsequent game update
					// we also must send any remaining empty actions now
					// note: this means if we get disconnected right now we can't use any of our buffer time, which would be very unlucky
					// it still gives us 60 seconds total to reconnect though
					// note: the lag screen can't be up right now otherwise the server made a big mistake, so we don't need to check for it

					BYTEARRAY EmptyAction;
					EmptyAction.push_back( 0xF7 );
					EmptyAction.push_back( 0x0C );
					EmptyAction.push_back( 0x06 );
					EmptyAction.push_back( 0x00 );
					EmptyAction.push_back( 0x00 );
					EmptyAction.push_back( 0x00 );

					for ( unsigned char i = m_NumEmptyActionsUsed; i < m_NumEmptyActions; i++ )
						m_LocalSocket->PutBytes( EmptyAction );

					m_NumEmptyActionsUsed = m_NumEmptyActions;
				}
			}

			// forward the data

			m_LocalSocket->PutBytes( Packet->GetData( ) );

			// we have to wait until now to send the status message since otherwise the slotinfojoin itself wouldn't have been forwarded

			if ( Packet->GetID( ) == CGameProtocol::W3GS_SLOTINFOJOIN )
			{
				if ( m_JoinedName != m_Username )
					SendLocalChat( "Using battle.net name \"" + m_Username + "\" instead of LAN name \"" + m_JoinedName + "\"." );

				if ( m_GameIsReliable ) {
					SendLocalChat( "This is a reliable game. Requesting GProxy++ disconnect protection from server..." );

				}
				else
					SendLocalChat( "This is an unreliable game. GProxy++ disconnect protection is disabled." );

				m_GameStarted = true;
			}
		}
		else if ( Packet->GetPacketType( ) == GPS_HEADER_CONSTANT )
		{
			if ( m_GameIsReliable )
			{
				BYTEARRAY Data = Packet->GetData( );

				if ( Packet->GetID( ) == CGPSProtocol::GPS_INIT && Data.size( ) == 12 )
				{
					m_ReconnectPort = UTIL_ByteArrayToUInt16( Data, false, 4 );
					m_PID = Data[ 6 ];
					m_ReconnectKey = UTIL_ByteArrayToUInt32( Data, false, 7 );
					m_NumEmptyActions = Data[ 11 ];
					SendLocalChat( "GProxy++ disconnect protection is ready (" + UTIL_ToString( ( m_NumEmptyActions + 1 ) * 60 ) + " second buffer)." );

					//			m_GameStarted = true;
					CONSOLE_Print( "[GPROXY] handshake complete, disconnect protection ready (" + UTIL_ToString( ( m_NumEmptyActions + 1 ) * 60 ) + " second buffer)" );
				}
				else if ( Packet->GetID( ) == CGPSProtocol::GPS_RECONNECT && Data.size( ) == 8 )
				{
					uint32_t LastPacket = UTIL_ByteArrayToUInt32( Data, false, 4 );
					uint32_t PacketsAlreadyUnqueued = m_TotalPacketsReceivedFromLocal - m_PacketBuffer.size( );

					if ( LastPacket > PacketsAlreadyUnqueued )
					{
						uint32_t PacketsToUnqueue = LastPacket - PacketsAlreadyUnqueued;

						if ( PacketsToUnqueue > m_PacketBuffer.size( ) )
						{
							CONSOLE_Print( "[GPROXY] received GPS_RECONNECT with last packet > total packets sent" );
							PacketsToUnqueue = m_PacketBuffer.size( );
						}

						while ( PacketsToUnqueue > 0 )
						{
							delete m_PacketBuffer.front( );
							m_PacketBuffer.pop( );
							PacketsToUnqueue--;
						}
					}

					// send remaining packets from buffer, preserve buffer
					// note: any packets in m_LocalPackets are still sitting at the end of this buffer because they haven't been processed yet
					// therefore we must check for duplicates otherwise we might (will) cause a desync

					queue<CCommandPacket *> TempBuffer;

					while ( !m_PacketBuffer.empty( ) )
					{
						if ( m_PacketBuffer.size( ) > m_LocalPackets.size( ) )
							m_RemoteSocket->PutBytes( m_PacketBuffer.front( )->GetData( ) );

						TempBuffer.push( m_PacketBuffer.front( ) );
						m_PacketBuffer.pop( );
					}

					m_PacketBuffer = TempBuffer;

					// we can resume forwarding local packets again
					// doing so prior to this point could result in an out-of-order stream which would probably cause a desync

					m_Synchronized = true;
				}
				else if ( Packet->GetID( ) == CGPSProtocol::GPS_ACK && Data.size( ) == 8 )
				{
					uint32_t LastPacket = UTIL_ByteArrayToUInt32( Data, false, 4 );
					uint32_t PacketsAlreadyUnqueued = m_TotalPacketsReceivedFromLocal - m_PacketBuffer.size( );

					if ( LastPacket > PacketsAlreadyUnqueued )
					{
						uint32_t PacketsToUnqueue = LastPacket - PacketsAlreadyUnqueued;

						if ( PacketsToUnqueue > m_PacketBuffer.size( ) )
						{
							CONSOLE_Print( "[GPROXY] received GPS_ACK with last packet > total packets sent" );
							PacketsToUnqueue = m_PacketBuffer.size( );
						}

						while ( PacketsToUnqueue > 0 )
						{
							delete m_PacketBuffer.front( );
							m_PacketBuffer.pop( );
							PacketsToUnqueue--;
						}
					}
				}
				else if ( Packet->GetID( ) == CGPSProtocol::GPS_REJECT && Data.size( ) == 8 )
				{
					uint32_t Reason = UTIL_ByteArrayToUInt32( Data, false, 4 );

					if ( Reason == REJECTGPS_INVALID )
						CONSOLE_Print( "[GPROXY] rejected by remote server: invalid data" );
					else if ( Reason == REJECTGPS_NOTFOUND )
						CONSOLE_Print( "[GPROXY] rejected by remote server: player not found in any running games" );

					m_LocalSocket->Disconnect( );
				}
			}

		}

		delete Packet;
	}
}

bool CGProxy::AddGame( CIncomingGameHost *game )
{
	// check for duplicates and rehosted games
	bool DuplicateFound = false;
	uint32_t OldestReceivedTime = GetTime( );

	for ( vector<CIncomingGameHost *> ::iterator i = m_Games.begin( ); i != m_Games.end( ); i++ )
	{
		if ( ( game->GetIP( ) == ( *i )->GetIP( ) && game->GetPort( ) == ( *i )->GetPort( ) ) )
		{
			// duplicate or rehosted game, delete the old one and add the new one
			// don't forget to remove the old one from the LAN list first

			m_UDPSocket->Broadcast( 6112, m_GameProtocol->SEND_W3GS_DECREATEGAME( ( *i )->GetUniqueGameID( ) ) );
			delete *i;
			*i = game;
			DuplicateFound = true;
			break;
		}

		if ( game->GetReceivedTime( ) < OldestReceivedTime ) {
			vector<string> games = m_BNET->GetSearchGameName( );
			for ( vector<string> ::iterator i = games.begin( ); i != games.end( ); i++ ) {
				std::string Game = ( *i );
				std::string GameName = Game.substr( 0, Game.find_first_of( ":" ) );
				if ( GameName != game->GetGameName( ) ) {
					OldestReceivedTime = game->GetReceivedTime( );
				}
			}
		}
	}

	if ( !DuplicateFound )
		m_Games.push_back( game );

	// the game list cannot hold more than 20 games (warcraft 3 doesn't handle it properly and ignores any further games)
	// if this game puts us over the limit, remove the oldest game
	// don't remove the "search game" since that's probably a pretty important game
	// note: it'll get removed automatically by the 60 second timeout in the main loop when appropriate

	if ( m_Games.size( ) > 20 )
	{
		for ( vector<CIncomingGameHost *> ::iterator i = m_Games.begin( ); i != m_Games.end( ); i++ )
		{
			if ( game->GetReceivedTime( ) == OldestReceivedTime )
			{
				vector<string> games = m_BNET->GetSearchGameName( );
				for ( vector<string> ::iterator j = games.begin( ); j != games.end( ); j++ ) {
					std::string Game = ( *j );
					std::string GameName = Game.substr( 0, Game.find_first_of( ":" ) );
					if ( GameName != game->GetGameName( ) ) {
						m_UDPSocket->Broadcast( 6112, m_GameProtocol->SEND_W3GS_DECREATEGAME( ( *i )->GetUniqueGameID( ) ) );
						delete *i;
						m_Games.erase( i );
						break;
					}
				}
			}
		}
	}
	return !DuplicateFound;
}

void CGProxy::SendLocalChat( string message )
{
	if ( m_LocalSocket )
	{
		if ( m_GameStarted )
		{
			if ( message.size( ) > 127 )
				message = message.substr( 0, 127 );

			m_LocalSocket->PutBytes( m_GameProtocol->SEND_W3GS_CHAT_FROM_HOST( m_ChatPID, UTIL_CreateByteArray( m_ChatPID ), 32, UTIL_CreateByteArray( ( uint32_t )0, false ), message ) );
		}
		else
		{
			if ( message.size( ) > 254 )
				message = message.substr( 0, 254 );

			m_LocalSocket->PutBytes( m_GameProtocol->SEND_W3GS_CHAT_FROM_HOST( m_ChatPID, UTIL_CreateByteArray( m_ChatPID ), 16, BYTEARRAY( ), message ) );
		}
	}
}

void CGProxy::SendEmptyAction( )
{
	// we can't send any empty actions while the lag screen is up
	// so we keep track of who the lag screen is currently showing (if anyone) and we tear it down, send the empty action, and put it back up

	for ( vector<unsigned char> ::iterator i = m_Laggers.begin( ); i != m_Laggers.end( ); i++ )
	{
		BYTEARRAY StopLag;
		StopLag.push_back( 0xF7 );
		StopLag.push_back( 0x11 );
		StopLag.push_back( 0x09 );
		StopLag.push_back( 0 );
		StopLag.push_back( *i );
		UTIL_AppendByteArray( StopLag, ( uint32_t )60000, false );
		m_LocalSocket->PutBytes( StopLag );
	}

	BYTEARRAY EmptyAction;
	EmptyAction.push_back( 0xF7 );
	EmptyAction.push_back( 0x0C );
	EmptyAction.push_back( 0x06 );
	EmptyAction.push_back( 0x00 );
	EmptyAction.push_back( 0x00 );
	EmptyAction.push_back( 0x00 );
	m_LocalSocket->PutBytes( EmptyAction );

	if ( !m_Laggers.empty( ) )
	{
		BYTEARRAY StartLag;
		StartLag.push_back( 0xF7 );
		StartLag.push_back( 0x10 );
		StartLag.push_back( 0 );
		StartLag.push_back( 0 );
		StartLag.push_back( m_Laggers.size( ) );

		for ( vector<unsigned char> ::iterator i = m_Laggers.begin( ); i != m_Laggers.end( ); i++ )
		{
			// using a lag time of 60000 ms means the counter will start at zero
			// hopefully warcraft 3 doesn't care about wild variations in the lag time in subsequent packets

			StartLag.push_back( *i );
			UTIL_AppendByteArray( StartLag, ( uint32_t )60000, false );
		}

		BYTEARRAY LengthBytes;
		LengthBytes = UTIL_CreateByteArray( ( uint16_t )StartLag.size( ), false );
		StartLag[ 2 ] = LengthBytes[ 0 ];
		StartLag[ 3 ] = LengthBytes[ 1 ];
		m_LocalSocket->PutBytes( StartLag );
	}
}


//new 

pProcessCommandsCallback CommandsCallback = NULL;


bool __stdcall ProcessCommandsCallback( const char* text , const char* arg1 ,
	const char* arg2 , const char* arg3, const char* arg4, const char* arg5, const char* arg6 , const char* arg7, const char* arg8, const char* arg9, const char* arg10)
{
	if ( CommandsCallback != NULL )
	{
		CommandsCallback( text, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10 );
		return true;
	}

	return false;
}


void __stdcall SetCmdCallback( pProcessCommandsCallback handler )
{
	CommandsCallback = handler;

	ProcessCommandsCallback( "/addtext", " TEST :) !!! :D " );
}

void __stdcall SendServerChatMessage( const char * message )
{
	if ( gGProxy && message && message[0] != '\0' )
	{
		if (gGProxy->m_BNET )
			gGProxy->m_BNET->QueueChatCommand( message );
	}
}