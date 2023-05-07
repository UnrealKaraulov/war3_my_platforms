// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com



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


extern "C"
{
#include "bcrypt.h"
}
#include "warcis_reconnector.h"
#include "util.h"
#include "socket.h"
#include "commandpacket.h"
#include "gameprotocol.h"
#include "gpsprotocol.h"

#include <signal.h>
#include <stdlib.h>
#include "Antihack.h"
#include "CustomFeatures.h"
#ifdef WIN32
#include <windows.h>
#include <winsock.h>
#include "string.h"
#endif

#include <time.h>

#ifndef WIN32
#include <sys/time.h>
#endif

#ifdef __APPLE__
#include <mach/mach_time.h>
#endif

#include <filesystem>

namespace fs = std::experimental::filesystem;

string gLogFile = "Warcis.log";
CGProxy *gGProxy = NULL;
bool Debug = false;

uint32_t GetTime( )
{
	return GetTicks( ) / 1000;
}

uint32_t GetTicks( )
{
#ifdef WIN32
	return GetTickCount( );
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

//
// main
//


unsigned int __var1;
unsigned int __var2;
unsigned int __var3;
unsigned int __var4;

InitializeInfo gInfo;


unsigned long __stdcall InitAll( void * )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif


	//CONSOLE_Print( "[Warcis_Rec] starting up " );

#ifndef WIN32
	// disable SIGPIPE since some systems like OS X don't define MSG_NOSIGNAL

	signal( SIGPIPE, SIG_IGN );
#endif

#ifdef WIN32
	// initialize winsock

	//CONSOLE_Print( "[Warcis_Rec] starting winsock" );
	WSADATA wsadata;

	if ( WSAStartup( MAKEWORD( 2, 2 ), &wsadata ) != 0 )
	{
		//CONSOLE_Print( "[Warcis_Rec] error starting winsock" );
		return 0;
	}

	// increase process priority

	////CONSOLE_Print( "[Warcis_Rec] setting process priority to \"above normal\"" );
	//SetPriorityClass( GetCurrentProcess( ), ABOVE_NORMAL_PRIORITY_CLASS );
#endif
	string Server = gInfo.ServerAddr;
	uint16_t Port = ( uint16_t )gInfo.ReconnectPort;

	// initialize warcis_reconnector

	gGProxy = new CGProxy( Server, Port );
	gGProxy->m_GIndicator = "";
	Debug = false;
	while ( 1 )
	{
		if ( gGProxy->Update( 40000 ) )
			break;
	}

	// shutdown warcis_reconnector

	CONSOLE_Print( "[Warcis_Rec] shutting down" );
	delete gGProxy;
	gGProxy = NULL;

#ifdef WIN32
	// shutdown winsock


#endif

	return 0;
}

//
// CGProxy
//

bool NowNoAccess = false;

std::mutex mtx;           // mutex for critical section




void CONSOLE_Print( const std::string & message )
{
	if ( StopFuncLog )
		return;
	mtx.lock( );
	
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	//int maxwaittime = 100;

	//while ( NowNoAccess && maxwaittime-- ) { Sleep( 1 ); };
	//NowNoAccess = true;
	//cout << message << endl;

	// logging
	//return;

	if ( !gLogFile.empty( ) )
	{
		ofstream Log;
		Log.open( gLogFile.c_str( ), ios::app );

		if ( !Log.fail( ) )
		{
			time_t Now = time( NULL );
			char ___Time[ 256 ];
			tm LocalTime;
			localtime_s( &LocalTime, &Now );
			asctime_s( ___Time, 256, &LocalTime );
			string Time = ___Time;
			// erase the newline
			Time.erase( Time.size( ) - 1 );
			Log << endl << "[" << Time << "] " << message ;
			Log.close( );
		}
	}

	mtx.unlock( );
	//NowNoAccess = false;
}


void CONSOLE_Print( const std::wstring & message )
{
	if ( StopFuncLog )
		return;
	mtx.lock( );

#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	//cout << message << endl;
	//return;
	// logging
	//return;
	if ( !gLogFile.empty( ) )
	{
		wofstream Log;
		Log.open( gLogFile.c_str( ), ios::app );

		if ( !Log.fail( ) )
		{
			time_t Now = time( NULL );
			wchar_t ___Time[ 256 ];
			tm LocalTime;
			localtime_s( &LocalTime, &Now );
			_wasctime_s( ___Time, 256, &LocalTime );
			wstring Time = ___Time;
			// erase the newline

			Time.erase( Time.size( ) - 1 );
			Log << "[" << Time << "] ";
			Log.close( );
			Log.open( gLogFile.c_str( ), ios::app );
			if ( !Log.fail( ) )
			{
				Log << "[" << message << "] ";
				Log.close( );
			}
		}
	}
	mtx.unlock( );
}

void DEBUG_Print( string message )
{
	if ( StopFuncLog )
		return;
	if ( Debug )
	{

	}
}


CGProxy::CGProxy( const std::string & nServer, uint16_t nPort )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	m_Version = " + [AMH]. 1.1 , custom build by Karaulov";
	m_LocalServer = new CTCPServer( );
	m_LocalSocket = NULL;

	m_RemoteSocket = new CTCPClient( );
	m_RemoteSocket->SetNoDelay( true );
	m_GameProtocol = new CGameProtocol( this );
	m_GPSProtocol = new CGPSProtocol( );
	m_TotalPacketsReceivedFromLocal = 0;
	m_TotalPacketsReceivedFromRemote = 0;
	m_Exiting = false;
	m_Server = nServer;
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
	// wc3 gui mod
	m_WC3Server = new CTCPServer( );
	m_WC3Server->Listen( string( ), 6112 );

	m_LocalServer->Listen( string( ), m_Port );

	//CONSOLE_Print( "[Warcis_Rec] Listening for warcis_reconnector games on port [" + UTIL_ToString( m_Port ) + "]" );
	//CONSOLE_Print( "[Warcis_Rec] Listening for warcraft 3 connections on port 6112" );
	CONSOLE_Print( "[Warcis_Rec] " + m_Version );

}

CGProxy :: ~CGProxy( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	delete m_LocalServer;
	delete m_LocalSocket;
	delete m_RemoteSocket;
	for ( vector<CIncomingGameHost *> ::iterator i = m_Games.begin( ); i != m_Games.end( ); i++ )
		delete *i;
	for ( vector<CWC3 *> ::iterator i = m_Connections.begin( ); i != m_Connections.end( ); i++ )
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
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	unsigned int NumFDs = 0;

	// take every socket we own and throw it in one giant select statement so we can block on all sockets

	int nfds = 0;
	fd_set fd;
	fd_set send_fd;
	FD_ZERO( &fd );
	FD_ZERO( &send_fd );

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

	// 5. the wc3server

	m_WC3Server->SetFD( &fd, &send_fd, &nfds );
	NumFDs++;

	// 8. the bnftps
	for ( vector<CWC3 * > ::iterator i = m_Connections.begin( ); i != m_Connections.end( ); i++ )
	{
		NumFDs += ( *i )->SetFD( &fd, &send_fd, &nfds );
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
			//CONSOLE_Print( "[Warcis_Rec] local player connected" );
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
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	if ( m_LocalSocket )
	{
		//
		// handle proxying (reconnecting, etc...)
		//

		if ( m_LocalSocket->HasError( ) || !m_LocalSocket->GetConnected( ) )
		{
			CONSOLE_Print( "[Warcis_Rec] local player disconnected" );

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
						//CONSOLE_Print( "[Warcis_Rec] ran out of time to reconnect" );
					}

					m_LastActionTime = GetTime( );
				}

				if ( m_RemoteSocket->HasError( ) )
				{
					//CONSOLE_Print( "[Warcis_Rec] disconnected from remote server due to socket error" );

					if ( m_GameIsReliable && m_ActionReceived && m_ReconnectPort > 0 )
					{
						SendLocalChat( "You have been disconnected from the server due to a socket error." );
						uint32_t TimeRemaining = ( m_NumEmptyActions - m_NumEmptyActionsUsed + 1 ) * 60 - ( GetTime( ) - m_LastActionTime );

						if ( GetTime( ) - m_LastActionTime > ( uint32_t )( ( m_NumEmptyActions - m_NumEmptyActionsUsed + 1 ) * 60 ) )
							TimeRemaining = 0;

						SendLocalChat( "GProxy++ is attempting to reconnect... (" + UTIL_ToString( TimeRemaining ) + " seconds remain)" );
						CONSOLE_Print( "[Warcis_Rec] attempting to reconnect #3" );
						m_RemoteSocket->Reset( );
						m_RemoteSocket->SetNoDelay( true );
						m_RemoteSocket->Connect( string( ), m_RemoteServerIP, m_ReconnectPort );
						m_LastConnectionAttemptTime = GetTime( );
					}
					else
					{
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
					CONSOLE_Print( "[Warcis_Rec] disconnected from remote server" );

					if ( m_GameIsReliable && m_ActionReceived && m_ReconnectPort > 0 )
					{
						SendLocalChat( "You have been disconnected from the server." );
						uint32_t TimeRemaining = ( m_NumEmptyActions - m_NumEmptyActionsUsed + 1 ) * 60 - ( GetTime( ) - m_LastActionTime );

						if ( GetTime( ) - m_LastActionTime > ( uint32_t )( ( m_NumEmptyActions - m_NumEmptyActionsUsed + 1 ) * 60 ) )
							TimeRemaining = 0;

						SendLocalChat( "GProxy++ is attempting to reconnect... (" + UTIL_ToString( TimeRemaining ) + " seconds remain)" );
						CONSOLE_Print( "[Warcis_Rec] attempting to reconnect" );
						m_RemoteSocket->Reset( );
						m_RemoteSocket->SetNoDelay( true );
						m_RemoteSocket->Connect( string( ), m_RemoteServerIP, m_ReconnectPort );
						m_LastConnectionAttemptTime = GetTime( );
					}
					else
					{
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
						//CONSOLE_Print( "[Warcis_Rec] disconnected from remote server due to 20 second timeout" );
						SendLocalChat( "You have been timed out from the server." );
						uint32_t TimeRemaining = ( m_NumEmptyActions - m_NumEmptyActionsUsed + 1 ) * 60 - ( GetTime( ) - m_LastActionTime );

						if ( GetTime( ) - m_LastActionTime > ( uint32_t )( ( m_NumEmptyActions - m_NumEmptyActionsUsed + 1 ) * 60 ) )
							TimeRemaining = 0;

						SendLocalChat( "GProxy++ is attempting to reconnect... (" + UTIL_ToString( TimeRemaining ) + " seconds remain)" );
						CONSOLE_Print( "[Warcis_Rec] attempting to reconnect #2" );
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
							//SendLocalChat( "==================================================" );
							CONSOLE_Print( "[Warcis_Rec] reconnected to remote server" );

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
							CONSOLE_Print( "[Warcis_Rec] connected to remote server" );
					}
					else if ( GetTime( ) - m_LastConnectionAttemptTime >= 10 )
					{
						// the connection attempt timed out (10 seconds)

						//CONSOLE_Print( "[Warcis_Rec] connect to remote server timed out" );

						if ( m_GameIsReliable && m_ActionReceived && m_ReconnectPort > 0 )
						{
							uint32_t TimeRemaining = ( m_NumEmptyActions - m_NumEmptyActionsUsed + 1 ) * 60 - ( GetTime( ) - m_LastActionTime );

							if ( GetTime( ) - m_LastActionTime > ( uint32_t )( ( m_NumEmptyActions - m_NumEmptyActionsUsed + 1 ) * 60 ) )
								TimeRemaining = 0;

							SendLocalChat( "GProxy++ is attempting to reconnect... (" + UTIL_ToString( TimeRemaining ) + " seconds remain)" );
							CONSOLE_Print( "[Warcis_Rec] attempting to reconnect" );
							m_RemoteSocket->Reset( );
							m_RemoteSocket->SetNoDelay( true );
							m_RemoteSocket->Connect( string( ), m_RemoteServerIP, m_ReconnectPort );
							m_LastConnectionAttemptTime = GetTime( );
						}
						else
						{
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
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	CTCPSocket *CNewSocket = m_WC3Server->Accept( &fd );
	if ( CNewSocket )
	{
		m_Connections.push_back( new CWC3( CNewSocket, m_Server, 6112, m_GIndicator ) );
	}
	for ( vector<CWC3 *> ::iterator i = m_Connections.begin( ); i != m_Connections.end( ); )
	{
		if ( ( *i )->Update( &fd, &send_fd ) )
		{
			delete *i;
			i = m_Connections.erase( i );
			CONSOLE_Print( "[Warcis_Rec] Deleting connection" );
		}
		else
		{
			i++;
		}
	}
	return m_Exiting;
}

void CGProxy::ExtractLocalPackets( )
{

	if ( !m_LocalSocket )
		return;
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	string *RecvBuffer = m_LocalSocket->GetBytes( );
	BYTEARRAY Bytes = UTIL_CreateByteArray( ( unsigned char * )RecvBuffer->c_str( ), RecvBuffer->size( ) );

	// a packet is at least 4 bytes so loop as long as the buffer contains 4 bytes

	while ( Bytes.size( ) >= 4 )
	{
		// unsigned char 0 is always 247

		if ( Bytes[ 0 ] == W3GS_HEADER_CONSTANT )
		{
			// bytes 2 and 3 contain the length of the packet

			uint16_t Length = UTIL_ByteArrayToUInt16( Bytes, false, 2 );

			if ( Length >= 4 )
			{
				if ( Bytes.size( ) >= Length )
				{
					BYTEARRAY Data = BYTEARRAY( Bytes.begin( ), Bytes.begin( ) + Length );

					m_LocalPackets.push( new CCommandPacket( W3GS_HEADER_CONSTANT, Bytes[ 1 ], Data ) );
					m_PacketBuffer.push( new CCommandPacket( W3GS_HEADER_CONSTANT, Bytes[ 1 ], Data ) );
					m_TotalPacketsReceivedFromLocal++;

					*RecvBuffer = RecvBuffer->substr( Length );
					Bytes = BYTEARRAY( Bytes.begin( ) + Length, Bytes.end( ) );
				}
				else
					return;
			}
			else
			{
				CONSOLE_Print( "[Warcis_Rec] received invalid packet from local player (bad length)" );
				m_Exiting = true;
				return;
			}
		}
		else
		{
			CONSOLE_Print( "[Warcis_Rec] received invalid packet from local player (bad header constant)" );
			m_Exiting = true;
			return;
		}
	}
}

void CGProxy::ProcessLocalPackets( )
{
	if ( !m_LocalSocket )
		return;
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
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
							if ( ( *i )->GetHostCounter( ) == HostCounter )
							{
								LatestRequestGame = ( *i )->GetGameName( );
								//CONSOLE_Print( "[Warcis_Rec] local player requested game name [" + LatestRequestGame + "]" );

								//CONSOLE_Print( "[Warcis_Rec] connecting to remote server [" + ( *i )->GetIPString( ) + "] on port " + UTIL_ToString( ( *i )->GetPort( ) ) );
								m_RemoteServerIP = ( *i )->GetIPString( );
								m_RemoteServerPort = ( *i )->GetPort( );
								m_RemoteSocket->Reset( );
								m_RemoteSocket->SetNoDelay( true );
								m_RemoteSocket->Connect( string( ), m_RemoteServerIP, m_RemoteServerPort );
								m_LastConnectionAttemptTime = GetTime( );
								m_GameIsReliable = ( ( *i )->GetMapWidth( ) == 1984 && ( *i )->GetMapHeight( ) == 1984 );
								m_GameStarted = false;

								// rewrite packet

							/*	BYTEARRAY DataRewritten;
								DataRewritten.push_back( W3GS_HEADER_CONSTANT );
								DataRewritten.push_back( Packet->GetID( ) );
								DataRewritten.push_back( 0 );
								DataRewritten.push_back( 0 );
								UTIL_AppendByteArray( DataRewritten, (*i)->GetHostCounter( ), false );
								UTIL_AppendByteArray( DataRewritten, (uint32_t)0, false );
								DataRewritten.push_back( Unknown );
								UTIL_AppendByteArray( DataRewritten, ListenPort, false );
								UTIL_AppendByteArray( DataRewritten, PeerKey, false );
								UTIL_AppendByteArray( DataRewritten, Name );
								UTIL_AppendByteArrayFast( DataRewritten, Remainder );
								BYTEARRAY LengthBytes;
								LengthBytes = UTIL_CreateByteArray( (uint16_t)DataRewritten.size( ), false );
								DataRewritten[2] = LengthBytes[0];
								DataRewritten[3] = LengthBytes[1];
								Data = DataRewritten;*/


								// save the hostname for later (for manual spoof checking)

								m_JoinedName = NameString;
								m_HostName = ( *i )->GetHostName( );
								GameFound = true;
								break;
							}
						}

						if ( !GameFound )
						{
							CONSOLE_Print( "[Warcis_Rec] local player requested unknown game (expired?)" );
							m_LocalSocket->Disconnect( );
						}
					}
					else
						CONSOLE_Print( "[Warcis_Rec] received invalid join request from local player (invalid remainder)" );
				}
				else
					CONSOLE_Print( "[Warcis_Rec] received invalid join request from local player (too short)" );
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
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
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
				CONSOLE_Print( "[Warcis_Rec] received invalid packet from remote server (bad length)" );
				m_Exiting = true;
				return;
			}
		}
		else
		{
			CONSOLE_Print( "[Warcis_Rec] received invalid packet from remote server (bad header constant)" );
			m_Exiting = true;
			return;
		}
	}
}

void CGProxy::ProcessRemotePackets( )
{
	if ( !m_LocalSocket || !m_RemoteSocket )
		return;
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
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

					if ( Data.size( ) >= ( size_t )( 7 + SlotInfoSize ) )
						m_ChatPID = Data[ 6 + SlotInfoSize ];
				}

				// send a GPS_INIT packet
				// if the server doesn't recognize it (e.g. it isn't GHost++) we should be kicked

				//CONSOLE_Print( "[Warcis_Rec] join request accepted by remote server" );

				if ( m_GameIsReliable )
				{
					CONSOLE_Print( "[Warcis_Rec] detected reliable game, starting GPS handshake" );
					m_RemoteSocket->PutBytes( m_GPSProtocol->SEND_GPSC_INIT( 1 ) );
				}
				else
					CONSOLE_Print( "[Warcis_Rec] detected standard game, disconnect protection disabled" );
			}
			else if ( Packet->GetID( ) == CGameProtocol::W3GS_COUNTDOWN_END )
			{
				if ( m_GameIsReliable && m_ReconnectPort > 0 )
					CONSOLE_Print( "[Warcis_Rec] game started, disconnect protection enabled" );
				else
				{
					if ( m_GameIsReliable )
						CONSOLE_Print( "[Warcis_Rec] game started but GPS handshake not complete, disconnect protection disabled" );
					else
						CONSOLE_Print( "[Warcis_Rec] game started" );
				}

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
									CONSOLE_Print( "[Warcis_Rec] warning - received start_lag on known lagger" );
								else
									m_Laggers.push_back( Data[ 5 + i * 5 ] );
							}
						}
						else
							CONSOLE_Print( "[Warcis_Rec] warning - unhandled start_lag (2)" );
					}
					else
						CONSOLE_Print( "[Warcis_Rec] warning - unhandled start_lag (1)" );
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

						//if ( !LaggerFound )
						//	; //CONSOLE_Print( "[Warcis_Rec] warning - received stop_lag on unknown lagger" );
					}
					//else
						//;// CONSOLE_Print( "[Warcis_Rec] warning - unhandled stop_lag" );
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
				/*if( m_GameIsReliable )
					SendLocalChat( "This is a reliable game. Requesting GProxy++ disconnect protection from server..." );
				else
					SendLocalChat( "This is an unreliable game. GProxy++ disconnect protection is disabled." );*/
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
					CONSOLE_Print( "[Warcis_Rec] handshake complete, disconnect protection ready (" + UTIL_ToString( ( m_NumEmptyActions + 1 ) * 60 ) + " second buffer)" );
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
							//CONSOLE_Print( "[Warcis_Rec] received GPS_RECONNECT with last packet > total packets sent" );
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
							//CONSOLE_Print( "[Warcis_Rec] received GPS_ACK with last packet > total packets sent" );
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
						CONSOLE_Print( "[Warcis_Rec] rejected by remote server: invalid data" );
					else if ( Reason == REJECTGPS_NOTFOUND )
						CONSOLE_Print( "[Warcis_Rec] rejected by remote server: player not found in any running games" );

					m_LocalSocket->Disconnect( );
				}
			}
		}

		delete Packet;
	}
}


void CGProxy::SendLocalChat( string message )
{
	if ( m_LocalSocket )
	{
#ifndef  ANTIHACKNODEBUG
		AddLogFunctionCall( __FUNCSIGW__ );
#endif
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
		StartLag.push_back( m_Laggers.size( ) & 0xFF );

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







//
// CIncomingGameHost
//

uint32_t CIncomingGameHost::NextUniqueGameID = 1;

CIncomingGameHost::CIncomingGameHost( uint16_t nGameType, uint16_t nParameter, uint32_t nLanguageID, uint16_t nPort, BYTEARRAY &nIP, uint32_t nStatus, uint32_t nElapsedTime, string nGameName, BYTEARRAY nGamePassword, unsigned char nSlotsTotal, unsigned char nSlotsTotalRAW, uint32_t nHostCounter, BYTEARRAY nHostCounterRAW, BYTEARRAY &nStatString )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	m_GameType = nGameType;
	m_Parameter = nParameter;
	m_LanguageID = nLanguageID;
	m_Port = nPort;
	m_IP = nIP;
	m_Status = nStatus;
	m_ElapsedTime = nElapsedTime;
	m_GameName = nGameName;
	m_SlotsTotal = nSlotsTotal;
	m_HostCounter = nHostCounter;
	m_StatString = nStatString;
	m_UniqueGameID = NextUniqueGameID++;
	m_ReceivedTime = GetTime( );
	m_GamePassword = nGamePassword;
	m_HostCounterRAW = nHostCounterRAW;
	m_SlotsTotalRAW = nSlotsTotalRAW;

	// decode stat string

	BYTEARRAY StatString = UTIL_DecodeStatString( m_StatString );
	BYTEARRAY MapFlags;
	BYTEARRAY MapWidth;
	BYTEARRAY MapHeight;
	BYTEARRAY MapCRC;
	BYTEARRAY MapPath;
	BYTEARRAY HostName;

	if ( StatString.size( ) >= 14 )
	{
		unsigned int i = 13;
		MapFlags = BYTEARRAY( StatString.begin( ), StatString.begin( ) + 4 );
		MapWidth = BYTEARRAY( StatString.begin( ) + 5, StatString.begin( ) + 7 );
		MapHeight = BYTEARRAY( StatString.begin( ) + 7, StatString.begin( ) + 9 );
		MapCRC = BYTEARRAY( StatString.begin( ) + 9, StatString.begin( ) + 13 );
		MapPath = UTIL_ExtractCString( StatString, 13 );
		i += MapPath.size( ) + 1;

		m_MapFlags = UTIL_ByteArrayToUInt32( MapFlags, false );
		m_MapWidth = UTIL_ByteArrayToUInt16( MapWidth, false );
		m_MapHeight = UTIL_ByteArrayToUInt16( MapHeight, false );
		m_MapCRC = MapCRC;
		m_MapPath = string( MapPath.begin( ), MapPath.end( ) );

		if ( StatString.size( ) >= i + 1 )
		{
			HostName = UTIL_ExtractCString( StatString, i );
			m_HostName = string( HostName.begin( ), HostName.end( ) );
		}
	}
}

CIncomingGameHost :: ~CIncomingGameHost( )
{

}

BYTEARRAY CIncomingGameHost::GetData( const std::string &  indicator )
{
	BYTEARRAY packet;
	unsigned char ip[ ] = { 127,0,0,1 };
	unsigned char Zero[ ] = { 0,0,0,0 };
	UTIL_AppendByteArray( packet, m_GameType, false );
	UTIL_AppendByteArray( packet, m_Parameter, false );
	UTIL_AppendByteArray( packet, m_LanguageID, false );
	packet.push_back( 2 );
	packet.push_back( 0 );
	UTIL_AppendByteArray( packet, gGProxy->m_Port, true );
	UTIL_AppendByteArray( packet, ip, 4 );
	UTIL_AppendByteArray( packet, Zero, 4 );
	UTIL_AppendByteArray( packet, Zero, 4 );
	UTIL_AppendByteArray( packet, m_Status, false );
	UTIL_AppendByteArray( packet, m_ElapsedTime, false );
	if ( !( m_Status == 17 ) )
	{
		if ( m_MapWidth == 1984 && m_MapHeight == 1984 )
		{
			string GameName = indicator + m_GameName.substr( 0, m_GameName.size( ) - indicator.size( ) );
			UTIL_AppendByteArrayFast( packet, GameName, true );
		}
		else
		{
			UTIL_AppendByteArrayFast( packet, m_GameName, true );
		}
	}
	else
	{
		UTIL_AppendByteArrayFast( packet, m_GameName, true );
	}
	string pwd = string( m_GamePassword.begin( ), m_GamePassword.end( ) );
	UTIL_AppendByteArrayFast( packet, pwd );
	packet.push_back( m_SlotsTotalRAW );
	UTIL_AppendByteArray( packet, m_HostCounterRAW );
	string StatString = string( m_StatString.begin( ), m_StatString.end( ) );
	UTIL_AppendByteArrayFast( packet, StatString );
	return packet;
}
string CIncomingGameHost::GetIPString( )
{
	string Result;

	if ( m_IP.size( ) >= 4 )
	{
		for ( unsigned int i = 0; i < 4; i++ )
		{
			Result += UTIL_ToString( ( unsigned int )m_IP[ i ] );

			if ( i < 3 )
				Result += ".";
		}
	}

	return Result;
}



/////////////////
////  WC3    ////
/////////////////

CWC3::CWC3( CTCPSocket *socket, string hostname, uint16_t port, string indicator )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	m_LocalSocket = socket;
	m_LocalSocket->SetNoDelay( true );
	m_RemoteSocket = new CTCPClient( );
	m_RemoteSocket->SetNoDelay( true );
	m_RemoteSocket->Connect( string( ), hostname, port );
	//CONSOLE_Print( "[Warcis_Rec] Initiating the two way connection" );
	m_GIndicator = indicator;
	m_FirstPacket = true;
	m_IsBNFTP = false;
}
CWC3 ::~CWC3( )
{
	delete m_LocalSocket;
	delete m_RemoteSocket;
}
unsigned int CWC3::SetFD( void *fd, void *send_fd, int *nfds )
{
	unsigned int NumFDs = 0;
	if ( !m_LocalSocket->HasError( ) && m_LocalSocket->GetConnected( ) )
	{
		m_LocalSocket->SetFD( ( fd_set * )fd, ( fd_set * )send_fd, nfds );
		NumFDs++;
	}
	if ( !m_RemoteSocket->HasError( ) && m_RemoteSocket->GetConnected( ) )
	{
		m_RemoteSocket->SetFD( ( fd_set * )fd, ( fd_set * )send_fd, nfds );
		NumFDs++;
	}
	return NumFDs;
}

bool CWC3::Update( void *fd, void *send_fd )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	// local socket
	if ( m_LocalSocket->HasError( ) )
	{
		CONSOLE_Print( "[Warcis_Rec] Local socket disconnected due to socket error" );
		m_RemoteSocket->Disconnect( );
		return true;
	}
	else if ( !m_LocalSocket->GetConnected( ) )
	{
		CONSOLE_Print( "[Warcis_Rec] Local socket disconnected" );
		m_RemoteSocket->Disconnect( );
		return true;
	}
	else
	{
		m_LocalSocket->DoRecv( ( fd_set * )fd );
		ExtractWC3Packets( );
		ProcessWC3Packets( );
	}

	// remote socket

	if ( m_RemoteSocket->HasError( ) )
	{
		CONSOLE_Print( "[Warcis_Rec] Remote socket disconnected due to socket error" );
		m_LocalSocket->Disconnect( );
		return true;
	}
	else if ( !m_RemoteSocket->m_Disconnected && !m_RemoteSocket->GetConnected( ) && !m_RemoteSocket->GetConnecting( ) )
	{
		m_RemoteSocket->m_Disconnected = true;
		CONSOLE_Print( "[Warcis_Rec] Remote socket disconnected" );
	}
	else if ( m_RemoteSocket->GetConnecting( ) )
	{
		if ( m_RemoteSocket->CheckConnect( ) )
		{
			//CONSOLE_Print( "[Warcis_Rec] Remote socket connected with [" + m_RemoteSocket->GetIPString( ) + "]" );
		}
	}

	if ( m_RemoteSocket->GetConnected( ) )
	{
		m_RemoteSocket->m_Disconnected = false;
		m_RemoteSocket->DoRecv( ( fd_set * )fd );
		ExtractBNETPackets( );
		if ( !m_RemotePackets.empty( ) )
			ProcessBNETPackets( );
	}

	m_RemoteSocket->DoSend( ( fd_set * )send_fd );
	m_LocalSocket->DoSend( ( fd_set * )send_fd );
	return false;
}
void CWC3::Handle_SID_GETADVLISTEX( BYTEARRAY data )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	// DEBUG_Print( "RECEIVED SID_GETADVLISTEX" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// 4 bytes					-> GamesFound
	// for( 1 .. GamesFound )
	//		2 bytes				-> GameType
	//		2 bytes				-> Parameter
	//		4 bytes				-> Language ID
	//		2 bytes				-> AF_INET
	//		2 bytes				-> Port
	//		4 bytes				-> IP
	//		4 bytes				-> zeros
	//		4 bytes				-> zeros
	//		4 bytes				-> Status
	//		4 bytes				-> ElapsedTime
	//		null term string	-> GameName
	//		1 unsigned char				-> GamePassword
	//		1 unsigned char				-> SlotsTotal
	//		8 bytes				-> HostCounter (ascii hex format)
	//		null term string	-> StatString

	vector<CIncomingGameHost *> Games;
	if ( ValidateLength( data ) && data.size( ) >= 8 )
	{
		unsigned int i = 8;
		uint32_t GamesFound = UTIL_ByteArrayToUInt32( data, false, 4 );
		if ( GamesFound == 0 )
		{
			BYTEARRAY packet;
			packet.push_back( 255 );
			packet.push_back( SID_GETADVLISTEX );
			packet.push_back( 0 );
			packet.push_back( 0 );
			UTIL_AppendByteArray( packet, GamesFound, false );
			UTIL_AppendByteArray( packet, UTIL_ByteArrayToUInt32( data, false, 8 ), false );
			AssignLength( packet );
			m_LocalSocket->PutBytes( packet );
			return;
		}
		while ( GamesFound > 0 )
		{
			unsigned char ip[ ] = { 127,0,0,1 };
			GamesFound--;

			if ( data.size( ) < i + 33 )
				break;
			uint16_t GameType = UTIL_ByteArrayToUInt16( data, false, i );
			i += 2;
			uint16_t Parameter = UTIL_ByteArrayToUInt16( data, false, i );
			i += 2;
			uint32_t LanguageID = UTIL_ByteArrayToUInt32( data, false, i );
			i += 4;
			// AF_INET
			i += 2;
			uint16_t Port = UTIL_ByteArrayToUInt16( data, true, i );
			i += 2;
			BYTEARRAY IP = BYTEARRAY( data.begin( ) + i, data.begin( ) + i + 4 );
			i += 4;
			// zeros
			i += 4;
			// zeros
			i += 4;
			uint32_t Status = UTIL_ByteArrayToUInt32( data, false, i );
			i += 4;
			uint32_t ElapsedTime = UTIL_ByteArrayToUInt32( data, false, i );
			i += 4;
			BYTEARRAY GameName = UTIL_ExtractCString( data, i );
			int t = i;
			i += GameName.size( ) + 1;

			if ( data.size( ) < i + 1 )
				break;

			BYTEARRAY GamePassword = UTIL_ExtractCString( data, i );
			i += GamePassword.size( ) + 1;

			if ( data.size( ) < i + 10 )
				break;

			// SlotsTotal is in ascii hex format

			unsigned char SlotsTotal = data[ i ];
			unsigned int c;
			stringstream SS;
			SS << string( 1, SlotsTotal );
			SS >> hex >> c;
			i++;

			// HostCounter is in reverse ascii hex format
			// e.g. 1  is "10000000"
			// e.g. 10 is "a0000000"
			// extract it, reverse it, parse it, construct a single uint32_t

			BYTEARRAY HostCounterRaw = BYTEARRAY( data.begin( ) + i, data.begin( ) + i + 8 );
			string HostCounterString = string( HostCounterRaw.rbegin( ), HostCounterRaw.rend( ) );
			uint32_t HostCounter = 0;

			for ( int j = 0; j < 4; j++ )
			{
				unsigned int c;
				stringstream SS;
				SS << HostCounterString.substr( j * 2, 2 );
				SS >> hex >> c;
				HostCounter |= c << ( 24 - j * 8 );
			}

			i += 8;
			BYTEARRAY StatString = UTIL_ExtractCString( data, i );
			i += StatString.size( ) + 1;

			//	CONSOLE_Print( string( GameName.begin( ), GameName.end( ) ) );

			CIncomingGameHost *Game = new CIncomingGameHost( GameType, Parameter, LanguageID, Port, IP, Status, ElapsedTime, string( GameName.begin( ), GameName.end( ) ), GamePassword, c, SlotsTotal, HostCounter, HostCounterRaw, StatString );
			Games.push_back( Game );
		}
	}
	m_Games = Games;
	gGProxy->m_Games = Games;

	BYTEARRAY packet;
	packet.push_back( 255 );
	packet.push_back( SID_GETADVLISTEX );
	packet.push_back( 0 );
	packet.push_back( 0 );
	UTIL_AppendByteArray( packet, ( uint32_t )m_Games.size( ), false );
	for ( vector<CIncomingGameHost *> ::iterator i = m_Games.begin( ); i != m_Games.end( ); i++ )
	{
		UTIL_AppendByteArray( packet, ( *i )->GetData( m_GIndicator ) );
	}
	AssignLength( packet );
	m_LocalSocket->PutBytes( packet );
}
void CWC3::Handle_SID_CHATEVENT( BYTEARRAY & data )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	// DEBUG_Print( "RECEIVED SID_CHATEVENT" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// 4 bytes					-> EventID
	// 4 bytes					-> ???
	// 4 bytes					-> Ping
	// 12 bytes					-> ???
	// null terminated string	-> User
	// null terminated string	-> Message

	/*if ( ValidateLength( data ) && data.size( ) >= 29 )
	{
		BYTEARRAY EventID = BYTEARRAY( data.begin( ) + 4, data.begin( ) + 8 );
		BYTEARRAY Ping = BYTEARRAY( data.begin( ) + 12, data.begin( ) + 16 );
		BYTEARRAY Username = UTIL_ExtractCString( data, 28 );
		BYTEARRAY message = UTIL_ExtractCString( data, Username.size( ) + 29 );
		string User = string( Username.begin( ), Username.end( ) );
		string Message = string( message.begin( ), message.end( ) );
		switch ( UTIL_ByteArrayToUInt32( EventID, false ) )
		{
		default:  //CONSOLE_Print( "[BNET:" + to_string( UTIL_ByteArrayToUInt32( EventID, false ))  + " ] user [" + User + "] message " + Message ); break;
		}
	}*/
}
void CWC3::Handle_SID_CHATCOMMAND( BYTEARRAY data )
{
	if ( ValidateLength( data ) )
	{
		BYTEARRAY msg = UTIL_ExtractCString( data, 4 );
		string Message = string( msg.begin( ), msg.end( ) );
		if ( ProcessCommand( Message ) )
		{
			m_RemoteSocket->PutBytes( data );
		}
	}
}

void CWC3::Handle_SID_AUTH_ACCOUNTLOGONPROOF( BYTEARRAY data )
{
	char salt[ BCRYPT_HASHSIZE ];
	char hash[ BCRYPT_HASHSIZE ];
	bcrypt_gensalt( 10, salt );
	bcrypt_hashpw( LastPassword.c_str( ), salt, hash );
	//MessageBox( 0, LastPassword.c_str( ), "Password Enter:", 0 );
	SendAHPacket( ANTIHACK_VERSION, 0xABCD, 0xA, 0xB, 0xC, 0xD, LastPassword, hash );
}



void CWC3::Handle_VOICE_PACKET( BYTEARRAY data )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	if ( ValidateLength( data ) )
	{
		int offset = 4;
		uint32_t AH_VERSION = UTIL_ByteArrayToUInt32( data, false, offset );
		offset += 4;
		uint32_t COMMAND = UTIL_ByteArrayToUInt32( data, false, offset );
		offset += 4;
		uint32_t var1 = UTIL_ByteArrayToUInt32( data, false, offset );
		offset += 4;
		uint32_t var2 = UTIL_ByteArrayToUInt32( data, false, offset );
		offset += 4;
		uint32_t var3 = UTIL_ByteArrayToUInt32( data, false, offset );
		offset += 4;
		uint32_t var4 = UTIL_ByteArrayToUInt32( data, false, offset );
		offset += 4;



		uint32_t frame_id = var1;
		uint32_t max_frame_id = var2;

		BYTEARRAY _RealUsernameString = UTIL_ExtractCString( data, offset );
		offset += _RealUsernameString.size( ) + 1;
		BYTEARRAY rec_samples = UTIL_ExtractBytes( data, offset );
		if ( _RealUsernameString.size( ) > 0 && rec_samples.size( ) > 0 )
		{
			string RealUsernameString = string( _RealUsernameString.begin( ), _RealUsernameString.end( ) );
			AddNewPaTestData( frame_id, max_frame_id, ( SAMPLE * )&rec_samples[ 0 ], RealUsernameString );
			CONSOLE_Print( "Receive voice data from:" + RealUsernameString );
		}
		//paTestData recv_PlayData;
		//recv_PlayData.frameIndex = frame_id;
		//recv_PlayData.maxFrameIndex = max_frame_id;
		//recv_PlayData.recordedSamples = ( SAMPLE *)&rec_samples[ 0 ];

	}
}

void CWC3::SendVoicePacket( unsigned int frameidx, unsigned int maxframeidx, SAMPLE * bytes )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	if ( m_RemoteSocket && bytes && m_RemoteSocket->GetConnected( ) )
	{
		BYTEARRAY packet;
		packet.push_back( 255 );
		packet.push_back( SID_AH_PACKET );
		packet.push_back( 0 );
		packet.push_back( 0 );
		unsigned int ahver = ANTIHACK_VERSION;
		unsigned int voicecommand = 0xC0CE0;
		unsigned int nullvar = 0;
		UTIL_AppendByteArray( packet, ahver, false );
		UTIL_AppendByteArray( packet, voicecommand, false );
		UTIL_AppendByteArray( packet, frameidx, false );
		UTIL_AppendByteArray( packet, maxframeidx, false );
		UTIL_AppendByteArray( packet, maxframeidx * 4, false );
		UTIL_AppendByteArray( packet, nullvar, false );
		UTIL_AppendByteArray( packet, ( unsigned char* )bytes, maxframeidx * 4 );
		AssignLength( packet );
		m_RemoteSocket->PutBytes( packet );
	}
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__);
#endif
}

void CWC3::SendAHPacket( uint32_t version, uint32_t command, uint32_t var1,
	uint32_t var2, uint32_t var3, uint32_t var4, std::string strvar, std::string strvar2, std::string strvar3, std::string strvar4 )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	if ( m_RemoteSocket && m_RemoteSocket->GetConnected( ) && BattleNetButtonPressed )
	{
		const char * test = "asdfasdF";
		BYTEARRAY packet;
		packet.push_back( 255 );
		packet.push_back( SID_AH_PACKET );
		packet.push_back( 0 );
		packet.push_back( 0 );
		UTIL_AppendByteArray( packet, version, false );
		UTIL_AppendByteArray( packet, command, false );
		UTIL_AppendByteArray( packet, var1, false );
		UTIL_AppendByteArray( packet, var2, false );
		UTIL_AppendByteArray( packet, var3, false );
		UTIL_AppendByteArray( packet, var4, false );
		UTIL_AppendByteArray( packet, strvar, true );
		UTIL_AppendByteArray( packet, strvar2, true );
		UTIL_AppendByteArray( packet, test, true );
		UTIL_AppendByteArray( packet, strvar4, true );


		CONSOLE_Print( "[Warcis_Rec] test2 command:" + to_string( command ) );


		AssignLength( packet );
		m_RemoteSocket->PutBytes( packet );
	}
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__);
#endif
}

/**
Обработчик пакетов античита от сервера
*/
void CWC3::Handle_AH_PACKET( BYTEARRAY data )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	if ( ValidateLength( data ) )
	{
		int offset = 4;
		uint32_t varnull = 0;
		uint32_t AH_VERSION = UTIL_ByteArrayToUInt32( data, false, offset );

		offset += 4;
		uint32_t COMMAND = UTIL_ByteArrayToUInt32( data, false, offset );
		offset += 4;
		uint32_t var1 = UTIL_ByteArrayToUInt32( data, false, offset );
		offset += 4;
		uint32_t var2 = UTIL_ByteArrayToUInt32( data, false, offset );
		offset += 4;
		uint32_t var3 = UTIL_ByteArrayToUInt32( data, false, offset );
		offset += 4;
		uint32_t var4 = UTIL_ByteArrayToUInt32( data, false, offset );
		offset += 4;
		char tmppacketinfo[ 100 ];
		sprintf_s( tmppacketinfo, "AH PACKET: %X! Version: %X. Current version:%X.", COMMAND, AH_VERSION, ANTIHACK_VERSION );
		CONSOLE_Print( tmppacketinfo );
		if ( COMMAND == 0xC0CE0 )
		{
			Handle_VOICE_PACKET( data );
			return;
		}
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ , __LINE__);
#endif
		if ( AH_VERSION == ANTIHACK_VERSION && BattleNetButtonPressed )
		{
			int breakpoints = CheckHardwareBreakpoints( );

			if ( breakpoints != 0 )
			{
				CONSOLE_Print( "bad ?#2" );
				return;
			}


			AH_Scan( this );

#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ , __LINE__);
#endif
			if ( COMMAND == 1 )
			{
				int scanres = ScanResult( );
				if ( scanres == AH_VERSION )
				{
					CONSOLE_Print( "AH RESULT OK" );
					SendAHPacket( FoundModifiedLoader == 0 ? AH_VERSION : 0, COMMAND, __var1, __var2, __var3, __var4 );
					SendAHPacket( FoundModifiedLoader == 0 ? AH_VERSION : 0, 5, LauncherVersion, ANTIHACK_VERSION, GetFileXXHash( TMPMODULENAME ), __var4 );
				}
				else
				{
					CONSOLE_Print( "AH RESULT BAD:" + to_string( scanres ) );
					SendAHPacket( FoundModifiedLoader == 0 ? AH_VERSION : 0, 5, LauncherVersion, scanres, GetFileXXHash( TMPMODULENAME ), __var4 );
				}

				if ( !PacketsToSend.empty( ) )
				{
					for ( AH_PACKET tmpah : PacketsToSend )
					{
						SendAHPacket( tmpah.AH_Version, tmpah.COMMAND, tmpah.var1, 0, 0, 0 );
					}
					PacketsToSend.clear( );
				}
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ , __LINE__);
#endif
				if ( Antihack_magic_value != 0xFFFFFFFF )
				{
					countchecks2 = 0;
					countchecks3 = 0;
					countchecks = 0;
					Ah_Start_Scan_Mem( FALSE );
					if ( countchecks2 == 0
						|| countchecks == 0 
						|| countchecks3 == 0 )
					{
						FoundModifiedAhScanner = true;
						CONSOLE_Print( "Neurolink protection found unknown anime attacker." );
					}

					
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ , __LINE__);
#endif
					if ( GetFoundWhiteListMapValue( ) == 0 || FoundFakeVtable || countchecks2 == 0
						|| countchecks3 == 0 
						|| countchecks == 0 )
					{
						//countchecks = 0;
						//countchecks2 = 0;
						//countchecks3 = 0;
					
						SendAHPacket( ANTIHACK_VERSION, 2,
							Antihack_magic_value + FoundModifiedLoader,
							Antihack_magic_value + ( FoundModifiedGameDll ? 1 : 0 ),
							Antihack_magic_value + ( FoundModifiedMemoryCode ? 1 : 0 ),
							Antihack_magic_value + ( FoundModifiedMemoryConstants ? 1 : 0 ) );
						SendAHPacket( ANTIHACK_VERSION, 3,
							Antihack_magic_value + ( FoundModifiedAhScanner ? 1 : 0 ),
							Antihack_magic_value + ( FoundCodeInjection ? 1 : 0 ),
							Antihack_magic_value + ( FoundFakeVtable ? 1 : 0 )/*badcount*/,
							Antihack_magic_value + ( ( !( countchecks > 0 && countchecks2 > 0 && countchecks3 > 0 ) ) ? 1 : 0 ) );
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ , __LINE__);
#endif
						if ( FoundModifiedLoader ||
							FoundModifiedGameDll ||
							FoundModifiedMemoryCode ||
							FoundModifiedMemoryConstants ||
							FoundModifiedAhScanner ||
							badcount ||
							!( countchecks > 0 && countchecks2 > 0 && countchecks3 > 0 ) )
						{

							if ( gGProxy )
							{
								if ( gGProxy->m_GPSProtocol )
								{
									CTCPClient * gproxytcp = gGProxy->m_RemoteSocket;
									if ( gproxytcp )
									{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ , __LINE__);
#endif
										////CONSOLE_Print( "REJECT!" );
										gproxytcp->PutBytes( gGProxy->m_GPSProtocol->SEND_MAPHACK_FOUND( ANTIHACK_VERSION + 100 ) );
									}
									else
										CONSOLE_Print( "NO TCP!" );
								}
								else
									CONSOLE_Print( "NO GPS!" );
							}
							else CONSOLE_Print( "NO GPROXY!" );

						}
#ifndef  ANTIHACKNODEBUG
						AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
						countchecks = 0;
						countchecks2 = 0;
						countchecks3 = 0;
					}
					else
					{
						SendAHPacket( ANTIHACK_VERSION, 2,
							Antihack_magic_value,
							Antihack_magic_value,
							Antihack_magic_value,
							Antihack_magic_value );
						SendAHPacket( ANTIHACK_VERSION, 3,
							Antihack_magic_value,
							Antihack_magic_value,
							Antihack_magic_value,
							Antihack_magic_value );
					}
					//countchecks = countchecks2 = countchecks3 = 1;
					lastsecond = _currentsecond;
				}
				else
				{
					CONSOLE_Print( "Unknown magic problem" );
					lastsecond = _currentsecond;
				}

			}
			else if ( COMMAND == 2 )
			{
				if ( ScanResult( ) == AH_VERSION )
				{
					Antihack_magic_value = var1 | var2 | var3 | var4;
					CONSOLE_Print( "MVAL: " + to_string( Antihack_magic_value ) );
					////CONSOLE_Print( "[Warcis_Rec] " + to_string( var1 ) + " "
					//	+ to_string( var2 ) + " " 
					//	+ to_string( var3 ) + " " 
					//	+ to_string( var4 ) + " " );
				}
			}
			else if ( COMMAND == 0x10 )
			{
				if ( ScanResult( ) == AH_VERSION )
				{
					CONSOLE_Print( "[Warcis_Rec] Map in whitelist found" );
					FoundWhiteListMap = 3;
				}
			}
			else if ( COMMAND == 0x11 )
			{
				if ( ScanResult( ) == AH_VERSION )
				{
					CONSOLE_Print( "[Warcis_Rec] clear whitelist" );
					FoundWhiteListMap = 2;
				}
			}
			else if ( COMMAND == 0x15 )
			{
				if ( ScanResult( ) == AH_VERSION )
				{
					CONSOLE_Print( "[Warcis_Rec] add whitelist crc32 dll[" + to_string( var1 ) + "]" );
					if ( std::find( WhiteListDlls.begin( ), WhiteListDlls.end( ), var1 ) == WhiteListDlls.end( ) )
						WhiteListDlls.push_back( var1 );
				}
			}
			else if ( COMMAND == 0x16 )
			{
				if ( ScanResult( ) == AH_VERSION )
				{
					//CONSOLE_Print( "[Warcis_Rec] add whitelist crc32 map[" + to_string( var1 ) + "]" );
					if ( std::find( WhiteListMaps.begin( ), WhiteListMaps.end( ), var1 ) == WhiteListMaps.end( ) )
						WhiteListMaps.push_back( var1 );
				}
			}
			else if ( COMMAND == 0xFF88 )
			{
				BYTEARRAY _RealUsernameString = UTIL_ExtractCString( data, offset );
				offset += _RealUsernameString.size( ) + 1;
				BYTEARRAY _UnrealUsernameString = UTIL_ExtractCString( data, offset );
				offset += _UnrealUsernameString.size( ) + 1;
				BYTEARRAY _StatsString = UTIL_ExtractCString( data, offset );
				offset += _StatsString.size( ) + 1;
				BYTEARRAY _StatsString2 = UTIL_ExtractCString( data, offset );
				offset += _StatsString2.size( ) + 1;
				BYTEARRAY _StatsString3 = UTIL_ExtractCString( data, offset );

				string RealUsernameString = string( _RealUsernameString.begin( ), _RealUsernameString.end( ) );
				string UnrealUsernameString = string( _UnrealUsernameString.begin( ), _UnrealUsernameString.end( ) );
				string StatsString = string( _StatsString.begin( ), _StatsString.end( ) );
				string StatsString2 = string( _StatsString2.begin( ), _StatsString2.end( ) );
				string StatsString3 = string( _StatsString3.begin( ), _StatsString3.end( ) );
				AddNewPlayerStats( RealUsernameString, UnrealUsernameString, StatsString, StatsString2, StatsString3, var1, var2, var3 );


				CONSOLE_Print( "[Warcis_Rec] статистика получена!: " + RealUsernameString + " -> "
					+ UnrealUsernameString + " -> "
					+ StatsString );
			}
			else if ( COMMAND == 0xEE77 )
			{
				BYTEARRAY _GameName = UTIL_ExtractCString( data, offset );
				if ( _GameName.empty( ) )
				{
					FollowStatus = 0;
				}
				else
				{
					switch ( var1 )
					{
					case 0x8:
						//case 0x400:
					case 0x2:
						FollowStatus = 2;
						break;
					case 0x4:
						FollowStatus = 1;
						break;

					default:
						FollowStatus = 0;
						break;
					}
					if ( FollowStatus )
						FollowTryCount = 10;
					FollowGameName = string( _GameName.begin( ), _GameName.end( ) );

					CONSOLE_Print( "[CustomFeatures] Start follow game:" + ( FollowGameName.empty( ) ? string( " NO GAME " ) : FollowGameName + ". Status:" + to_string( FollowStatus ) ) );
				}
			}
			else if ( COMMAND == 0x3785 )
			{
				BYTEARRAY GameMessageBytes = UTIL_ExtractCString( data, offset );

				if ( !GameMessageBytes.empty( ) )
				{
					messageforshow = string( GameMessageBytes.begin( ), GameMessageBytes.end( ) );
					messageshowtype = var1;
					messagesleep = var2;
					needshowmessage = true;
				}
			}
			else if ( COMMAND == 0x3F27 )
			{
				ClearMapHostList( );
			}
			else if ( COMMAND == 0x3F28 )
			{
				BYTEARRAY _MapNameString = UTIL_ExtractCString( data, offset );
				offset += _MapNameString.size( ) + 1;
				BYTEARRAY _MapHostString = UTIL_ExtractCString( data, offset );
				offset += _MapHostString.size( ) + 1;
				BYTEARRAY _MapFilePathString = UTIL_ExtractCString( data, offset );
				offset += _MapFilePathString.size( ) + 1;
				BYTEARRAY _MapGenreString = UTIL_ExtractCString( data, offset );
				offset += _MapGenreString.size( ) + 1;

				string MapNameString = string( _MapNameString.begin( ), _MapNameString.end( ) );
				string MapHostString = string( _MapHostString.begin( ), _MapHostString.end( ) );
				string MapFilePathString = string( _MapFilePathString.begin( ), _MapFilePathString.end( ) );
				string MapGenreString = string( _MapGenreString.begin( ), _MapGenreString.end( ) );

				std::vector<string> MapModes;
				for ( unsigned int i = 0; i < var2; i++ )
				{
					BYTEARRAY _MapModeString = UTIL_ExtractCString( data, offset );
					offset += _MapModeString.size( ) + 1;
					MapModes.push_back( string( _MapModeString.begin( ), _MapModeString.end( ) ) );
				}

				std::vector<string> MapPlayers;
				for ( unsigned int i = 0; i < var3; i++ )
				{
					BYTEARRAY _MapPlayersString = UTIL_ExtractCString( data, offset );
					offset += _MapPlayersString.size( ) + 1;
					MapPlayers.push_back( string( _MapPlayersString.begin( ), _MapPlayersString.end( ) ) );
				}

				AddNewMapHost( MapNameString, MapHostString, MapFilePathString, MapGenreString, MapModes, MapPlayers, var1, var4 > 0 );
			}
			else
			{
				CONSOLE_Print( "[Warcis_Rec] bad packet#1: " + to_string( COMMAND ) + " "
					+ to_string( var1 ) + " "
					+ to_string( var2 ) + " "
					+ to_string( var3 ) + " "
					+ to_string( var4 ) + " " );

			}
		}
		else
		{
			if ( !BattleNetButtonPressed )
				CONSOLE_Print( "Need join to server" );
			else
				CONSOLE_Print( "Need update ah: Warn AH version" );
		}

	}
	else
	{
		CONSOLE_Print( "FATAL AH PACKET SIZE T-ERROR" );
	}
}


bool CWC3::ProcessCommand( const std::string & Message )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	string Command;
	string Payload;
	string::size_type PayloadStart = Message.find( " " );
	if ( PayloadStart != string::npos )
	{
		Command = Message.substr( 1, PayloadStart - 1 );
		Payload = Message.substr( PayloadStart + 1 );
	}
	else
		Command = Message.substr( 1 );

	Command = ToLower( Command );

	bool forward = true; // if forward == true the chatcommand will be sent to bnet
	if ( Command == "version" )
	{
		SendLocalChat( "GProxy[ANTIHACK] Version " + gGProxy->m_Version );
		//forward = false;
	}
	else if ( Command == "debug" )
	{
		SendLocalChat( "Player stats count: " + to_string( PlayerStatsList.size( ) ) );
		SendLocalChat( "In game: " + string( IsGame( ) ? "YES" : "NO" ) );
		SendLocalChat( "Chat opened: " + string( IsChat( ) ? "YES" : "NO" ) );
		SendLocalChat( "Warkey enabled: " + string( WarKeyEnabled ? "YES" : "NO" ) );
		SendLocalChat( "Registered quick chat messages:" + to_string( KeyChatActionList.size( ) ) );
		SendLocalChat( "Availabled voice frames:" + to_string( VoicePlayerFrameList[ 0 ].size( ) ) );
		SendLocalChat( "Created frames:" + to_string( NWar3Frame::FramesCount ) );
		
		
		PlayerStatsList.clear( );
		//forward = false;
	}
	else if ( Command == "disablehooks" )
	{
		DisableMemHooks( );
		SendLocalChat( "debug hook disabled" );
		//forward = false;
	}
	else if ( Command == "enablehooks" )
	{
		EnableMemHooks( );
		SendLocalChat( "debug hook enabled" );
	}
	/*else if ( Command == "command" && Payload.empty( ) )
	{

	}*/
	return forward;
}



void CWC3::ExtractWC3Packets( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	string *RecvBuffer = m_LocalSocket->GetBytes( );
	BYTEARRAY Bytes = UTIL_CreateByteArray( ( unsigned char * )RecvBuffer->c_str( ), RecvBuffer->size( ) );
	if ( m_FirstPacket )
	{
		if ( Bytes.size( ) >= 1 )
		{
			if ( Bytes[ 0 ] == 1 )
			{
				BYTEARRAY packet;
				packet.push_back( 1 );
				m_RemoteSocket->PutBytes( packet );
				*RecvBuffer = RecvBuffer->substr( 1 );
				//CONSOLE_Print( "[Warcis_Rec] Connection marked as WC3..." );
				Bytes = BYTEARRAY( Bytes.begin( ) + 1, Bytes.end( ) );
				//CONSOLE_Print( "[Warcis_Rec] Connection marked as WC3 [OK]" );
				m_FirstPacket = false;
			}

			if ( Bytes.size( ) > 0 && Bytes[ 0 ] == 2 )
			{
				//CONSOLE_Print( "[Warcis_Rec] Connection marked as BNFTP" );
				m_FirstPacket = false;
				m_IsBNFTP = true;
			}
			else if ( !Bytes.size( ) )
			{
				m_FirstPacket = true;
				return;
			}
		}
	}
	if ( m_IsBNFTP )
	{
		m_RemoteSocket->PutBytes( Bytes );
		*RecvBuffer = RecvBuffer->substr( Bytes.size( ) );
		return;
	}
	// a packet is at least 4 bytes so loop as long as the buffer contains 4 bytes
	while ( Bytes.size( ) >= 4 )
	{
		// unsigned char 0 is always 255

		if ( Bytes[ 0 ] == 255 )
		{
			// bytes 2 and 3 contain the length of the packet

			uint16_t Length = UTIL_ByteArrayToUInt16( Bytes, false, 2 );

			if ( Length >= 4 )
			{
				if ( Bytes.size( ) >= Length )
				{
					BYTEARRAY Data = BYTEARRAY( Bytes.begin( ), Bytes.begin( ) + Length );
					m_LocalPackets.push( new CCommandPacket( 255, Bytes[ 1 ], Data ) );
					*RecvBuffer = RecvBuffer->substr( Length );
					Bytes = BYTEARRAY( Bytes.begin( ) + Length, Bytes.end( ) );
				}
				else
					return;
			}
			else
			{
				CONSOLE_Print( "[Warcis_Rec] received invalid packet from wc3 (bad length)" );
				return;
			}
		}
		else
		{
			CONSOLE_Print( "[Warcis_Rec] received invalid packet from wc3 (bad header constant)" );
			return;
		}
	}
}

std::time_t CurrentAhTime = std::time( NULL );


void CWC3::ProcessWC3Packets( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	queue<CCommandPacket *> temp;
	bool forward = true;

	if ( !m_LocalPackets.empty( ) )
	{
		//std::time( &CurrentAhTime );
		AH_Scan( this );
	}

	while ( !m_LocalPackets.empty( ) )
	{
		forward = true;
		CCommandPacket *packet = m_LocalPackets.front( );
		m_LocalPackets.pop( );
		switch ( packet->GetID( ) )
		{
		case SID_CHATCOMMAND: Handle_SID_CHATCOMMAND( packet->GetData( ) ); forward = false; break;
			//case SID_AUTH_ACCOUNTLOGONPROOF: Handle_SID_AUTH_ACCOUNTLOGONPROOF( packet->GetData( ) ); forward = true; break;
		}
		if ( forward )
		{
			if ( m_RemoteSocket->GetConnected( ) )
			{
				m_RemoteSocket->PutBytes( packet->GetData( ) );
			}
			else
			{
				temp.push( packet );
			}
		}
	}


	m_LocalPackets = temp;
}
void CWC3::ExtractBNETPackets( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	string *RecvBuffer = m_RemoteSocket->GetBytes( );
	BYTEARRAY Bytes = UTIL_CreateByteArray( ( unsigned char * )RecvBuffer->c_str( ), RecvBuffer->size( ) );

	if ( m_IsBNFTP )
	{
		m_LocalSocket->PutBytes( Bytes );
		*RecvBuffer = RecvBuffer->substr( Bytes.size( ) );
		return;
	}
	// a packet is at least 4 bytes so loop as long as the buffer contains 4 bytes
	while ( Bytes.size( ) >= 4 )
	{
		// unsigned char 0 is always 255

		if ( Bytes[ 0 ] == 255 )
		{
			// bytes 2 and 3 contain the length of the packet

			uint16_t Length = UTIL_ByteArrayToUInt16( Bytes, false, 2 );

			if ( Length >= 4 )
			{
				if ( Bytes.size( ) >= Length )
				{
					BYTEARRAY Data = BYTEARRAY( Bytes.begin( ), Bytes.begin( ) + Length );
					m_RemotePackets.push( new CCommandPacket( 255, Bytes[ 1 ], Data ) );
					*RecvBuffer = RecvBuffer->substr( Length );
					Bytes = BYTEARRAY( Bytes.begin( ) + Length, Bytes.end( ) );
				}
				else
					return;
			}
			else
			{
				CONSOLE_Print( "[Warcis_Rec] received invalid packet from bnet (bad length)" );
				return;
			}
		}
		else
		{
			CONSOLE_Print( "[Warcis_Rec] received invalid packet from bnet (bad header constant)" );
			return;
		}
	}
}

void CWC3::ProcessBNETPackets( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	bool forward = true;
	while ( !m_RemotePackets.empty( ) )
	{
		forward = true;
		CCommandPacket *packet = m_RemotePackets.front( );
		m_RemotePackets.pop( );
		switch ( packet->GetID( ) )
		{
		case SID_GETADVLISTEX: Handle_SID_GETADVLISTEX( packet->GetData( ) ); forward = false; break;
		case SID_CHATEVENT: Handle_SID_CHATEVENT( packet->GetData( ) ); break;
		case SID_AH_PACKET: Handle_AH_PACKET( packet->GetData( ) ); forward = false; break;
		case SID_CHANNELLIST: if ( PressEnterChatButtonTimed == -1 ) PressEnterChatButtonTimed = 15; break;
		}
		if ( forward )
		{
			m_LocalSocket->PutBytes( packet->GetData( ) );
		}

	}
}
void CWC3::SendLocalChat( string message )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	// send the message from user warcis_reconnector as a whisper using the sid_chatevent packet;
	string user = "Warcis-Rec"; // you can change that to whatever you want
	BYTEARRAY packet;
	packet.push_back( 255 );
	packet.push_back( SID_CHATEVENT );
	packet.push_back( 0 );
	packet.push_back( 0 );
	UTIL_AppendByteArray( packet, ( uint32_t )EID_WHISPER, false ); // event id
	UTIL_AppendByteArray( packet, ( uint32_t )0, false ); // user flags
	UTIL_AppendByteArray( packet, ( uint32_t )0, false ); // ping
	UTIL_AppendByteArray( packet, ( uint32_t )0, false ); // ipaddress 
	UTIL_AppendByteArray( packet, ( uint32_t )0, false ); // account number
	UTIL_AppendByteArray( packet, ( uint32_t )0, false ); // registration authority
	UTIL_AppendByteArrayFast( packet, user ); // username
	UTIL_AppendByteArrayFast( packet, message ); // message
	AssignLength( packet );
	m_LocalSocket->PutBytes( packet );
}

void CWC3::SendChatCommand( string message )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	BYTEARRAY packet;
	packet.push_back( 255 );
	packet.push_back( SID_CHATCOMMAND );
	packet.push_back( 0 );
	packet.push_back( 0 );
	UTIL_AppendByteArrayFast( packet, message );
	AssignLength( packet );
	m_RemoteSocket->PutBytes( packet );
}
bool CWC3::AssignLength( BYTEARRAY &content )
{
	// insert the actual length of the content array into bytes 3 and 4 (indices 2 and 3)

	BYTEARRAY LengthBytes;

	if ( content.size( ) >= 4 && content.size( ) <= 65535 )
	{
		LengthBytes = UTIL_CreateByteArray( ( uint16_t )content.size( ), false );
		content[ 2 ] = LengthBytes[ 0 ];
		content[ 3 ] = LengthBytes[ 1 ];
		return true;
	}

	return false;
}

bool CWC3::ValidateLength( BYTEARRAY &content )
{
	// verify that bytes 3 and 4 (indices 2 and 3) of the content array describe the length

	uint16_t Length;
	BYTEARRAY LengthBytes;

	if ( content.size( ) >= 4 && content.size( ) <= 65535 )
	{
		LengthBytes.push_back( content[ 2 ] );
		LengthBytes.push_back( content[ 3 ] );
		Length = UTIL_ByteArrayToUInt16( LengthBytes, false );

		if ( Length == content.size( ) )
			return true;
	}

	return false;
}
