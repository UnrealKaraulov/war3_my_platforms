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

#include "ggb.h"
#include "util.h"
#include "config.h"
#include "socket.h"
#include "commandpacket.h"
#include "gameprotocol.h"
#include "gpsprotocol.h"

#include <signal.h>
#include <stdlib.h>

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




string gLogFile;
CGHostGamesBroadcaster *gGGB = NULL;
uint32_t NextUniqueID = 0;

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
	if( info.denom == 0 )
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

int main( int argc, char **argv )
{
	//string CFGFile = "GGB.cfg";

	//if( argc > 1 && argv[1] )
	//	CFGFile = argv[1];

	//// read config file

	//CConfig CFG;
	//CFG.Read( CFGFile );
	CONSOLE_Print( "[GGB] starting up" );

#ifndef WIN32
	// disable SIGPIPE since some systems like OS X don't define MSG_NOSIGNAL

	signal( SIGPIPE, SIG_IGN );
#endif

#ifdef WIN32
	// initialize winsock

	CONSOLE_Print( "[GGB] starting winsock" );
	WSADATA wsadata;

	if( WSAStartup( MAKEWORD( 2, 2 ), &wsadata ) != 0 )
	{
		CONSOLE_Print( "[GGB] error starting winsock" );
		return 1;
	}

	// increase process priority

	CONSOLE_Print( "[GGB] setting process priority to \"above normal\"" );
	SetPriorityClass( GetCurrentProcess( ), ABOVE_NORMAL_PRIORITY_CLASS );
#endif
	string Hostname;
	cout << "[GGB] Input the remote ghost's server ip/hostname" << endl;
	getline( cin , Hostname );

	string Port;
	cout << "[GGB] Input the remote ghost's server ggb listening port" << endl;
	getline( cin ,Port );

	// initialize GGB

	gGGB = new CGHostGamesBroadcaster( Hostname , UTIL_ToUInt16(Port) );
	while( 1 )
	{
		if( gGGB->Update( 40000 ) )
			break;
	}

	// shutdown GGB

	CONSOLE_Print( "[GGB] shutting down" );
	delete gGGB;
	gGGB = NULL;

#ifdef WIN32
	// shutdown winsock

	CONSOLE_Print( "[GGB] shutting down winsock" );
	WSACleanup( );
#endif

	return 0;
}

//
// CGHostGamesBroadcaster
//

void CONSOLE_Print( string message )
{
	cout << message << endl;

	// logging

    if( !gLogFile.empty( ) )
	{
		ofstream Log;
		Log.open( gLogFile.c_str( ), ios :: app );

		if( !Log.fail( ) )
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

CGHostGamesBroadcaster :: CGHostGamesBroadcaster( string nHostname, uint16_t nPort )
{
	m_Version = "1.0";
	m_Exiting = false;
	m_Hostname = nHostname;
	m_Port = nPort;
	m_ListeningPort = 6120;
	m_Socket = new CTCPClient( );
	m_GameServer = new CTCPServer( );
	m_UDPSocket = new CUDPSocket( );
	m_Socket->SetSuppressError( true );
	m_GameServer->Listen( string( ) ,m_ListeningPort );
	CONSOLE_Print("[GGB] Listening for game connections on port " + UTIL_ToString( m_ListeningPort ) );
	m_Socket->Connect(string( ),m_Hostname, m_Port );
	m_LastConnectionAttemptTime = GetTime( );
	CONSOLE_Print("[GGB] Connecting to the ghost server" );
	CONSOLE_Print( "[GGB] GHost Games Broadcaster  Version " + m_Version );
	
}

CGHostGamesBroadcaster :: ~CGHostGamesBroadcaster( )
{
	for( vector<CGame *> ::iterator i = m_Games.begin( ); i != m_Games.end( );i++)
		delete *i;
	for( vector<CGPG *> ::iterator i = m_Cons.begin( ); i != m_Cons.end( );i++)
		delete *i;
	while ( !m_Packets.empty( ) )
	{
		delete m_Packets.front( );
		m_Packets.pop( );
	}
	delete m_GameServer;
	delete m_Socket;

}

bool CGHostGamesBroadcaster :: Update( long usecBlock )
{
	unsigned int NumFDs = 0;

	// take every socket we own and throw it in one giant select statement so we can block on all sockets

	int nfds = 0;
	fd_set fd;
	fd_set send_fd;
	FD_ZERO( &fd );
	FD_ZERO( &send_fd );
	
	// 1. The game server
	m_GameServer->SetFD( &fd,&send_fd,&nfds );
	NumFDs++;
	
	// 2. The main socket
	
	if ( m_Socket->GetConnected( ) && !m_Socket->HasError( ) )
	{
		m_Socket->SetFD( &fd,&send_fd, &nfds );
		NumFDs++;
	}

	// 3. the games

	for ( vector<CGPG *>::iterator i = m_Cons.begin( ); i != m_Cons.end( ) ; i++ )
	{
		NumFDs += (*i)->SetFD( &fd,&send_fd, &nfds );
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

	if( NumFDs == 0 )
		MILLISLEEP( 50 );

	if ( m_Socket->HasError( ) )
	{
		CONSOLE_Print("[GGB] Main socket disconnected due to socket error : " + m_Socket->GetErrorString( ) );
		m_Exiting = true;
	}
	else if ( !m_Socket->GetConnected( ) && !m_Socket->GetConnecting( ) )
	{
		CONSOLE_Print("[GGB] Main socket disconnected" );
		m_Exiting = true;
	}
	else if ( m_Socket->GetConnecting( ) )
	{
		if ( m_Socket->CheckConnect( ) )
		{
			CONSOLE_Print("[GGB] Main socket connected to [" + m_Socket->GetIPString( ) +"]" );
		}
		else if( GetTime( ) - m_LastConnectionAttemptTime >= 10 )
		{
			CONSOLE_Print("[GGB] Connection to server timed out");
			m_Exiting = true;
		}

	}
	else if ( m_Socket->GetConnected( ) )
	{
		m_Socket->DoRecv( &fd );
		ExtractPackets( );
		ProcessPackets( );
		m_Socket->DoSend( &send_fd );
	}

	CTCPSocket *NewSocket = m_GameServer->Accept( &fd );
	if ( NewSocket )
	{
		m_Cons.push_back( new CGPG( NewSocket ) );
		CONSOLE_Print("[GGB] Game connection request received from [" + NewSocket->GetIPString( ) +"]");
	}

	for ( vector<CGPG *>::iterator i = m_Cons.begin( ); i!= m_Cons.end( ); )
	{
		if ( (*i)->Update( &fd, &send_fd ) )
		{
			delete *i;
			i = m_Cons.erase( i );
		}
		else
			i++;
	}
	
	return m_Exiting;
}
void CGHostGamesBroadcaster::ExtractPackets()
{
	string *RecvBuffer = m_Socket->GetBytes( );
	BYTEARRAY Bytes = UTIL_CreateByteArray( (unsigned char *)RecvBuffer->c_str( ), RecvBuffer->size( ) );

	// a packet is at least 4 bytes so loop as long as the buffer contains 4 bytes

	while( Bytes.size( ) >= 4 )
	{
		if( Bytes[0] == W3GS_HEADER_CONSTANT )
		{
			// bytes 2 and 3 contain the length of the packet

			uint16_t Length = UTIL_ByteArrayToUInt16( Bytes, false, 2 );

			if( Length >= 4 )
			{
				if( Bytes.size( ) >= Length )
				{
					m_Packets.push( new CCommandPacket( Bytes[0], Bytes[1], BYTEARRAY( Bytes.begin( ), Bytes.begin( ) + Length ) ) );
					*RecvBuffer = RecvBuffer->substr( Length );
					Bytes = BYTEARRAY( Bytes.begin( ) + Length, Bytes.end( ) );
				}
				else
					return;
			}
			else
			{
				CONSOLE_Print( "[GGB] received invalid packet from remote server (bad length)" );
				m_Exiting = true;
				return;
			}
		}
		else
		{
			CONSOLE_Print( "[GGB] received invalid packet from remote server (bad header constant)" );
			m_Exiting = true;
			return;
		}
	}
}

void CGHostGamesBroadcaster::ProcessPackets()
{
	while ( !m_Packets.empty( ) )
	{
		CCommandPacket * packet = m_Packets.front( );
		m_Packets.pop( );
		switch( packet->GetID( ) )
		{
		case CGameProtocol::W3GS_GAMEINFO : Event_GameInfo( packet->GetData( ) ); break;
		}
	}
}
void CGHostGamesBroadcaster::Event_GameInfo(BYTEARRAY data)
{
	 //dword product id
	//dword version
	//dword hostcounter
	//dword entrykey
	//string gamename
	//byte terminator
	//string statstring
	//dword mapgametype
	//dword unknown
	//dword slotsopen
	//dword upTime
	//word port

	unsigned char Version = data[8];
	BYTEARRAY HostCounter = BYTEARRAY( data.begin( ) + 12 , data.begin( ) + 16 );
	BYTEARRAY EntryKey = BYTEARRAY( data.begin( ) + 16, data.begin( ) +20 );
	BYTEARRAY Gamename = UTIL_ExtractCString( data,  20 );
	BYTEARRAY nStatString = UTIL_ExtractCString( data ,22 + Gamename.size( ) );
	int i = 23 + Gamename.size( )  + nStatString.size( );
	BYTEARRAY SlotsTotal = BYTEARRAY( data.begin( ) + i, data.begin( ) + i + 4 );
	BYTEARRAY GameType = BYTEARRAY( data.begin( ) + i + 4 , data.begin( ) + i + 8 );
	BYTEARRAY Unknown = BYTEARRAY( data.begin( ) + i + 8 , data.begin( ) + i + 12 );
	BYTEARRAY SlotsOpen = BYTEARRAY ( data.begin( ) + i + 12, data.begin( ) + i+ 16 );
	BYTEARRAY UpTime = BYTEARRAY ( data.begin( ) + i + 16, data.begin( ) + i + 20 );
	BYTEARRAY Port = BYTEARRAY( data.begin( ) + i + 20 ,data.begin( ) + i + 22 );
	
	// decode stat string
	
	BYTEARRAY StatString = UTIL_DecodeStatString( nStatString );
	BYTEARRAY MapFlags;
	BYTEARRAY MapWidth;
	BYTEARRAY MapHeight;
	BYTEARRAY MapCRC;
	BYTEARRAY MapPath;
	BYTEARRAY HostName;

	if( StatString.size( ) >= 14 )
	{
		unsigned int i = 13;
		MapFlags = BYTEARRAY( StatString.begin( ), StatString.begin( ) + 4 );
		MapWidth = BYTEARRAY( StatString.begin( ) + 5, StatString.begin( ) + 7 );
		MapHeight = BYTEARRAY( StatString.begin( ) + 7, StatString.begin( ) + 9 );
		MapCRC = BYTEARRAY( StatString.begin( ) + 9, StatString.begin( ) + 13 );
		MapPath = UTIL_ExtractCString( StatString, 13 );
		i += MapPath.size( ) + 1;

		if( StatString.size( ) >= i + 1 )
		{
			HostName = UTIL_ExtractCString( StatString, i );
			//m_HostName = string( HostName.begin( ), HostName.end( ) );
		}
	}
	string GameName = string( Gamename.begin( ), Gamename.end( ) );
	if ( UTIL_ByteArrayToUInt16(MapWidth,false) == 1984 && UTIL_ByteArrayToUInt16(MapHeight,false) == 1984 ) 
	{
		// gproxy game
		
		GameName = "|cFF4080C0" + GameName;

		if ( GameName.size( ) > 31 )
			GameName = GameName.substr( 0, 31 );
	}

	CGame *Game = NULL;
	for(vector<CGame *> ::iterator i = m_Games.begin( ); i != m_Games.end( ); i++ )
	{
		if ((*i)->GetGameName( ) == string( Gamename.begin( ), Gamename.end( ) ) )
		{
			Game = *i;
		}
	}

	if ( !Game )
	{
		m_Games.push_back( new CGame(string (Gamename.begin( ), Gamename.end( ) ) , NextUniqueID , UTIL_ByteArrayToUInt16(Port,false ), ( UTIL_ByteArrayToUInt16(MapWidth,false) == 1984 && UTIL_ByteArrayToUInt16(MapHeight,false) == 1984 )) );

		CONSOLE_Print("[GGB] Received new game " + string( Gamename.begin( ) ,Gamename.end( ) ) );
		m_UDPSocket->Broadcast(6112, m_Protocol->SEND_W3GS_GAMEINFO(true,Version,GameType,MapFlags,MapWidth,MapHeight,GameName,string(HostName.begin( ),HostName.end( ) ),UTIL_ByteArrayToUInt32(UpTime,false),string( MapPath.begin( ) ,MapPath.end( ) ),MapCRC,UTIL_ByteArrayToUInt32(SlotsTotal,false),UTIL_ByteArrayToUInt32( SlotsOpen,false),m_ListeningPort,UTIL_ByteArrayToUInt32(HostCounter,false),NextUniqueID));
		NextUniqueID++;
		//m_UDPSocket->Broadcast(6112,data);
	}
	else
	{
	 	m_UDPSocket->Broadcast( 6112, m_Protocol->SEND_W3GS_GAMEINFO( true,Version,GameType,MapFlags,MapWidth,MapHeight,GameName,string(HostName.begin( ),HostName.end( ) ),UTIL_ByteArrayToUInt32(UpTime,false),string( MapPath.begin( ) ,MapPath.end( ) ),MapCRC,UTIL_ByteArrayToUInt32(SlotsTotal,false),UTIL_ByteArrayToUInt32( SlotsOpen,false),m_ListeningPort,UTIL_ByteArrayToUInt32(HostCounter,false),Game->GetUniqueID( )));
		//m_UDPSocket->Broadcast(6112,data );
	}

}
////////////////////////
/// GProxy Game (GPG)/// 
////////////////////////
CGPG ::CGPG(CTCPSocket *socket)
{
	m_LocalSocket = socket;
	m_RemoteSocket = new CTCPClient( );
	m_RemoteSocket->SetNoDelay( true );
	m_GameProtocol = new CGameProtocol( gGGB );
	m_GPSProtocol = new CGPSProtocol( );
	m_TotalPacketsReceivedFromLocal = 0;
	m_TotalPacketsReceivedFromRemote = 0;
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
	m_GameName = "";
}

CGPG ::~CGPG( )
{
	delete m_LocalSocket;
	delete m_RemoteSocket;
	delete m_GameProtocol;
	delete m_GPSProtocol;

	while( !m_LocalPackets.empty( ) )
	{
		delete m_LocalPackets.front( );
		m_LocalPackets.pop( );
	}

	while( !m_RemotePackets.empty( ) )
	{
		delete m_RemotePackets.front( );
		m_RemotePackets.pop( );
	}

	while( !m_PacketBuffer.empty( ) )
	{
		delete m_PacketBuffer.front( );
		m_PacketBuffer.pop( );
	}
}
unsigned int CGPG ::SetFD( void *fd, void *send_fd, int *nfds )
{
	unsigned int NumFDs = 0;
	if ( !m_LocalSocket->HasError( ) && m_LocalSocket->GetConnected( ) )
	{ 
		m_LocalSocket->SetFD( (fd_set *)fd, (fd_set *)send_fd, nfds );
		NumFDs++;
	}
	if ( !m_RemoteSocket->HasError( ) && m_RemoteSocket->GetConnected( ) )
	{
		m_RemoteSocket->SetFD( (fd_set * )fd, (fd_set *)send_fd, nfds );
		NumFDs++;
	}
	return NumFDs;
}
bool CGPG ::Update( void *fd, void *send_fd )
{

	if( m_LocalSocket->HasError( ) || !m_LocalSocket->GetConnected( ) )
	{
		CONSOLE_Print( "[GPROXY] local player disconnected" );
	
		// ensure a leavegame message was sent, otherwise the server may wait for our reconnection which will never happen
		// if one hasn't been sent it's because Warcraft III exited abnormally

		if( m_GameIsReliable && !m_LeaveGameSent )
		{
			// note: we're not actually 100% ensuring the leavegame message is sent, we'd need to check that DoSend worked, etc...

			BYTEARRAY LeaveGame;
			LeaveGame.push_back( 0xF7 );
			LeaveGame.push_back( 0x21 );
			LeaveGame.push_back( 0x08 );
			LeaveGame.push_back( 0x00 );
			UTIL_AppendByteArray( LeaveGame, (uint32_t)PLAYERLEAVE_GPROXY, false );
			m_RemoteSocket->PutBytes( LeaveGame );
			m_RemoteSocket->DoSend( (fd_set *)send_fd );
		}
		return true;
	}
	else
	{
		m_LocalSocket->DoRecv( (fd_set *)fd );
		ExtractLocalPackets( );
		ProcessLocalPackets( );

		if( !m_RemoteServerIP.empty( ) )
		{
			if( m_GameIsReliable && m_ActionReceived && GetTime( ) - m_LastActionTime >= 60 )
			{
				if( m_NumEmptyActionsUsed < m_NumEmptyActions )
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

			if( m_RemoteSocket->HasError( ) )
			{
				CONSOLE_Print( "[GPROXY] disconnected from remote server due to socket error" );

				if( m_GameIsReliable && m_ActionReceived && m_ReconnectPort > 0 )
				{
					SendLocalChat( "You have been disconnected from the server due to a socket error." );
					uint32_t TimeRemaining = ( m_NumEmptyActions - m_NumEmptyActionsUsed + 1 ) * 60 - ( GetTime( ) - m_LastActionTime );

					if( GetTime( ) - m_LastActionTime > ( m_NumEmptyActions - m_NumEmptyActionsUsed + 1 ) * 60 )
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
					m_LocalSocket->Disconnect( );
					return true;
				}
			}

			if( !m_RemoteSocket->GetConnecting( ) && !m_RemoteSocket->GetConnected( ) )
			{
				CONSOLE_Print( "[GPROXY] disconnected from remote server" );

				if( m_GameIsReliable && m_ActionReceived && m_ReconnectPort > 0 )
				{
					SendLocalChat( "You have been disconnected from the server." );
					uint32_t TimeRemaining = ( m_NumEmptyActions - m_NumEmptyActionsUsed + 1 ) * 60 - ( GetTime( ) - m_LastActionTime );

					if( GetTime( ) - m_LastActionTime > ( m_NumEmptyActions - m_NumEmptyActionsUsed + 1 ) * 60 )
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
					m_LocalSocket->Disconnect( );
					return true;
				}
			}

			if( m_RemoteSocket->GetConnected( ) )
			{
				if( m_GameIsReliable && m_ActionReceived && m_ReconnectPort > 0 && GetTime( ) - m_RemoteSocket->GetLastRecv( ) >= 20 )
				{
					CONSOLE_Print( "[GPROXY] disconnected from remote server due to 20 second timeout" );
					SendLocalChat( "You have been timed out from the server." );
					uint32_t TimeRemaining = ( m_NumEmptyActions - m_NumEmptyActionsUsed + 1 ) * 60 - ( GetTime( ) - m_LastActionTime );

					if( GetTime( ) - m_LastActionTime > ( m_NumEmptyActions - m_NumEmptyActionsUsed + 1 ) * 60 )
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
					m_RemoteSocket->DoRecv( (fd_set *)fd );
					ExtractRemotePackets( );
					ProcessRemotePackets( );

					if( m_GameIsReliable && m_ActionReceived && m_ReconnectPort > 0 && GetTime( ) - m_LastAckTime >= 10 )
					{
						m_RemoteSocket->PutBytes( m_GPSProtocol->SEND_GPSC_ACK( m_TotalPacketsReceivedFromRemote ) );
						m_LastAckTime = GetTime( );
					}

					m_RemoteSocket->DoSend( (fd_set *)send_fd );
				}
			}

			if( m_RemoteSocket->GetConnecting( ) )
			{
				// we are currently attempting to connect

				if( m_RemoteSocket->CheckConnect( ) )
				{
					// the connection attempt completed

					if( m_GameIsReliable && m_ActionReceived )
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
				else if( GetTime( ) - m_LastConnectionAttemptTime >= 10 )
				{
					// the connection attempt timed out (10 seconds)

					CONSOLE_Print( "[GPROXY] connect to remote server timed out" );

					if( m_GameIsReliable && m_ActionReceived && m_ReconnectPort > 0 )
					{
						uint32_t TimeRemaining = ( m_NumEmptyActions - m_NumEmptyActionsUsed + 1 ) * 60 - ( GetTime( ) - m_LastActionTime );

						if( GetTime( ) - m_LastActionTime > ( m_NumEmptyActions - m_NumEmptyActionsUsed + 1 ) * 60 )
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
						m_LocalSocket->Disconnect( );
						return true;
					}
				}
			}
		}
		m_LocalSocket->DoSend( (fd_set *)send_fd );
	}
	return false;
}
void CGPG :: ExtractLocalPackets( )
{
	if( !m_LocalSocket )
		return;

	string *RecvBuffer = m_LocalSocket->GetBytes( );
	BYTEARRAY Bytes = UTIL_CreateByteArray( (unsigned char *)RecvBuffer->c_str( ), RecvBuffer->size( ) );

	// a packet is at least 4 bytes so loop as long as the buffer contains 4 bytes

	while( Bytes.size( ) >= 4 )
	{
		// byte 0 is always 247

		if( Bytes[0] == W3GS_HEADER_CONSTANT )
		{
			// bytes 2 and 3 contain the length of the packet

			uint16_t Length = UTIL_ByteArrayToUInt16( Bytes, false, 2 );

			if( Length >= 4 )
			{
				if( Bytes.size( ) >= Length )
				{
					BYTEARRAY Data = BYTEARRAY( Bytes.begin( ), Bytes.begin( ) + Length );

						m_LocalPackets.push( new CCommandPacket( W3GS_HEADER_CONSTANT, Bytes[1], Data ) );
						m_PacketBuffer.push( new CCommandPacket( W3GS_HEADER_CONSTANT, Bytes[1], Data ) );
						m_TotalPacketsReceivedFromLocal++;

					*RecvBuffer = RecvBuffer->substr( Length );
					Bytes = BYTEARRAY( Bytes.begin( ) + Length, Bytes.end( ) );
				}
				else
					return;
			}
			else
			{
				CONSOLE_Print( "[GGB] received invalid packet from local player (bad length)" );
				return;
			}
		}
		else
		{
			CONSOLE_Print( "[GGB] received invalid packet from local player (bad header constant)" );
			return;
		}
	}
}

void CGPG :: ProcessLocalPackets( )
{
	if( !m_LocalSocket )
		return;

	while( !m_LocalPackets.empty( ) )
	{
		CCommandPacket *Packet = m_LocalPackets.front( );
		m_LocalPackets.pop( );
		BYTEARRAY Data = Packet->GetData( );

		if( Packet->GetPacketType( ) == W3GS_HEADER_CONSTANT )
		{
			if( Packet->GetID( ) == CGameProtocol :: W3GS_REQJOIN )
			{
				if( Data.size( ) >= 20 )
				{
					// parse

					uint32_t HostCounter = UTIL_ByteArrayToUInt32( Data, false, 4 );
					uint32_t EntryKey = UTIL_ByteArrayToUInt32( Data, false, 8 );
					unsigned char Unknown = Data[12];
					uint16_t ListenPort = UTIL_ByteArrayToUInt16( Data, false, 13 );
					uint32_t PeerKey = UTIL_ByteArrayToUInt32( Data, false, 15 );
					BYTEARRAY Name = UTIL_ExtractCString( Data, 19 );
					string NameString = string( Name.begin( ), Name.end( ) );
					BYTEARRAY Remainder = BYTEARRAY( Data.begin( ) + Name.size( ) + 20, Data.end( ) );
					if( Remainder.size( ) == 18 )
					{
						// lookup the game in the main list

						bool GameFound = false;

						for( vector<CGame *> :: iterator i = gGGB->m_Games.begin( ); i != gGGB->m_Games.end( ); i++ )
						{
							if( (*i)->GetUniqueID( ) == EntryKey )
							{
								CONSOLE_Print( "[GGB] local player requested game name [" + (*i)->GetGameName( ) + "]" );

								CONSOLE_Print( "[GGB] connecting to remote server [" + gGGB->m_Socket->GetIPString( ) + "] on port " + UTIL_ToString( (*i)->GetPort( ) ) );
								m_RemoteServerIP = gGGB->m_Hostname;
								m_RemoteServerPort = (*i)->GetPort( );
								m_RemoteSocket->Reset( );
								m_RemoteSocket->SetNoDelay( true );
								m_RemoteSocket->Connect( string( ), m_RemoteServerIP, m_RemoteServerPort );
								m_LastConnectionAttemptTime = GetTime( );
								m_GameIsReliable = (*i)->GetGProxySupport( );
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
								m_HostName = "Exm : todo remove this";
								GameFound = true;
								break;
							}
						}

						if( !GameFound )
						{
							CONSOLE_Print( "[GGB] local player requested unknown game (expired?)" );
							m_LocalSocket->Disconnect( );
						}
					}
					else
						CONSOLE_Print( "[GGB] received invalid join request from local player (invalid remainder)" );
				}
				else
					CONSOLE_Print( "[GGB] received invalid join request from local player (too short)" );
			}
		}
		else if( Packet->GetID( ) == CGameProtocol :: W3GS_LEAVEGAME )
		{
			m_LeaveGameSent = true;
			m_LocalSocket->Disconnect( );
		}
		else if( Packet->GetID( ) == CGameProtocol :: W3GS_CHAT_TO_HOST )
		{
			// handled in ExtractLocalPackets (yes, it's ugly)
		}

		// warning: do not forward any data if we are not synchronized (e.g. we are reconnecting and resynchronizing)
		// any data not forwarded here will be cached in the packet buffer and sent later so all is well

		if( m_RemoteSocket && m_Synchronized )
			m_RemoteSocket->PutBytes( Data );

		delete Packet;
	}
}

void CGPG :: ExtractRemotePackets( )
{
	string *RecvBuffer = m_RemoteSocket->GetBytes( );
	BYTEARRAY Bytes = UTIL_CreateByteArray( (unsigned char *)RecvBuffer->c_str( ), RecvBuffer->size( ) );

	// a packet is at least 4 bytes so loop as long as the buffer contains 4 bytes

	while( Bytes.size( ) >= 4 )
	{
		if( Bytes[0] == W3GS_HEADER_CONSTANT || Bytes[0] == GPS_HEADER_CONSTANT )
		{
			// bytes 2 and 3 contain the length of the packet

			uint16_t Length = UTIL_ByteArrayToUInt16( Bytes, false, 2 );

			if( Length >= 4 )
			{
				if( Bytes.size( ) >= Length )
				{
					m_RemotePackets.push( new CCommandPacket( Bytes[0], Bytes[1], BYTEARRAY( Bytes.begin( ), Bytes.begin( ) + Length ) ) );

					if( Bytes[0] == W3GS_HEADER_CONSTANT )
						m_TotalPacketsReceivedFromRemote++;

					*RecvBuffer = RecvBuffer->substr( Length );
					Bytes = BYTEARRAY( Bytes.begin( ) + Length, Bytes.end( ) );
				}
				else
					return;
			}
			else
			{
				CONSOLE_Print( "[GGB] received invalid packet from remote server (bad length)" );
				return;
			}
		}
		else
		{
			CONSOLE_Print( "[GGB] received invalid packet from remote server (bad header constant)" );
			return;
		}
	}
}

void CGPG :: ProcessRemotePackets( )
{
	if( !m_LocalSocket || !m_RemoteSocket )
		return;

	while( !m_RemotePackets.empty( ) )
	{
		CCommandPacket *Packet = m_RemotePackets.front( );
		m_RemotePackets.pop( );

		if( Packet->GetPacketType( ) == W3GS_HEADER_CONSTANT )
		{
			if( Packet->GetID( ) == CGameProtocol :: W3GS_SLOTINFOJOIN )
			{
				BYTEARRAY Data = Packet->GetData( );

				if( Data.size( ) >= 6 )
				{
					uint16_t SlotInfoSize = UTIL_ByteArrayToUInt16( Data, false, 4 );

					if( Data.size( ) >= 7 + SlotInfoSize )
						m_ChatPID = Data[6 + SlotInfoSize];
				}

				// send a GPS_INIT packet
				// if the server doesn't recognize it (e.g. it isn't GHost++) we should be kicked

				CONSOLE_Print( "[GGB] join request accepted by remote server" );

				if( m_GameIsReliable )
				{
					CONSOLE_Print( "[GGB] detected reliable game, starting GPS handshake" );
					m_RemoteSocket->PutBytes( m_GPSProtocol->SEND_GPSC_INIT( 1 ) );
				}
				else
					CONSOLE_Print( "[GGB] detected standard game, disconnect protection disabled" );
			}
			else if( Packet->GetID( ) == CGameProtocol :: W3GS_COUNTDOWN_END )
			{
				if( m_GameIsReliable && m_ReconnectPort > 0 )
					CONSOLE_Print( "[GGB] game started, disconnect protection enabled" );
				else
				{
					if( m_GameIsReliable )
						CONSOLE_Print( "[GGB] game started but GPS handshake not complete, disconnect protection disabled" );
					else
						CONSOLE_Print( "[GGB] game started" );
				}

				m_GameStarted = true;
			}
			else if( Packet->GetID( ) == CGameProtocol :: W3GS_INCOMING_ACTION )
			{
				if( m_GameIsReliable )
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

					for( unsigned char i = m_NumEmptyActionsUsed; i < m_NumEmptyActions; i++ )
						m_LocalSocket->PutBytes( EmptyAction );

					m_NumEmptyActionsUsed = 0;
				}

				m_ActionReceived = true;
				m_LastActionTime = GetTime( );
			}
			else if( Packet->GetID( ) == CGameProtocol :: W3GS_START_LAG )
			{
				if( m_GameIsReliable )
				{
					BYTEARRAY Data = Packet->GetData( );

					if( Data.size( ) >= 5 )
					{
						unsigned char NumLaggers = Data[4];

						if( Data.size( ) == 5 + NumLaggers * 5 )
						{
							for( unsigned char i = 0; i < NumLaggers; i++ )
							{
								bool LaggerFound = false;

								for( vector<unsigned char> :: iterator j = m_Laggers.begin( ); j != m_Laggers.end( ); j++ )
								{
									if( *j == Data[5 + i * 5] )
										LaggerFound = true;
								}

								if( LaggerFound )
									CONSOLE_Print( "[GGB] warning - received start_lag on known lagger" );
								else
									m_Laggers.push_back( Data[5 + i * 5] );
							}
						}
						else
							CONSOLE_Print( "[GGB] warning - unhandled start_lag (2)" );
					}
					else
						CONSOLE_Print( "[GGB] warning - unhandled start_lag (1)" );
				}
			}
			else if( Packet->GetID( ) == CGameProtocol :: W3GS_STOP_LAG )
			{
				if( m_GameIsReliable )
				{
					BYTEARRAY Data = Packet->GetData( );

					if( Data.size( ) == 9 )
					{
						bool LaggerFound = false;

						for( vector<unsigned char> :: iterator i = m_Laggers.begin( ); i != m_Laggers.end( ); )
						{
							if( *i == Data[4] )
							{
								i = m_Laggers.erase( i );
								LaggerFound = true;
							}
							else
								i++;
						}

						if( !LaggerFound )
							CONSOLE_Print( "[GGB] warning - received stop_lag on unknown lagger" );
					}
					else
						CONSOLE_Print( "[GGB] warning - unhandled stop_lag" );
				}
			}
			else if( Packet->GetID( ) == CGameProtocol :: W3GS_INCOMING_ACTION2 )
			{
				if( m_GameIsReliable )
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

					for( unsigned char i = m_NumEmptyActionsUsed; i < m_NumEmptyActions; i++ )
						m_LocalSocket->PutBytes( EmptyAction );

					m_NumEmptyActionsUsed = m_NumEmptyActions;
				}
			}

			// forward the data

			m_LocalSocket->PutBytes( Packet->GetData( ) );

			// we have to wait until now to send the status message since otherwise the slotinfojoin itself wouldn't have been forwarded

			if( Packet->GetID( ) == CGameProtocol :: W3GS_SLOTINFOJOIN )
			{
				if( m_GameIsReliable )
					SendLocalChat( "This is a reliable game. Requesting GGB++ disconnect protection from server..." );
				else
					SendLocalChat( "This is an unreliable game. GGB++ disconnect protection is disabled." );
			}
		}
		else if( Packet->GetPacketType( ) == GPS_HEADER_CONSTANT )
		{
			if( m_GameIsReliable )
			{
				BYTEARRAY Data = Packet->GetData( );

				if( Packet->GetID( ) == CGPSProtocol :: GPS_INIT && Data.size( ) == 12 )
				{
					m_ReconnectPort = UTIL_ByteArrayToUInt16( Data, false, 4 );
					m_PID = Data[6];
					m_ReconnectKey = UTIL_ByteArrayToUInt32( Data, false, 7 );
					m_NumEmptyActions = Data[11];
					SendLocalChat( "GGB++ disconnect protection is ready (" + UTIL_ToString( ( m_NumEmptyActions + 1 ) * 60 ) + " second buffer)." );
					CONSOLE_Print( "[GGB] handshake complete, disconnect protection ready (" + UTIL_ToString( ( m_NumEmptyActions + 1 ) * 60 ) + " second buffer)" );
				}
				else if( Packet->GetID( ) == CGPSProtocol :: GPS_RECONNECT && Data.size( ) == 8 )
				{
					uint32_t LastPacket = UTIL_ByteArrayToUInt32( Data, false, 4 );
					uint32_t PacketsAlreadyUnqueued = m_TotalPacketsReceivedFromLocal - m_PacketBuffer.size( );

					if( LastPacket > PacketsAlreadyUnqueued )
					{
						uint32_t PacketsToUnqueue = LastPacket - PacketsAlreadyUnqueued;

						if( PacketsToUnqueue > m_PacketBuffer.size( ) )
						{
							CONSOLE_Print( "[GGB] received GPS_RECONNECT with last packet > total packets sent" );
							PacketsToUnqueue = m_PacketBuffer.size( );
						}

						while( PacketsToUnqueue > 0 )
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

					while( !m_PacketBuffer.empty( ) )
					{
						if( m_PacketBuffer.size( ) > m_LocalPackets.size( ) )
							m_RemoteSocket->PutBytes( m_PacketBuffer.front( )->GetData( ) );

						TempBuffer.push( m_PacketBuffer.front( ) );
						m_PacketBuffer.pop( );
					}

					m_PacketBuffer = TempBuffer;

					// we can resume forwarding local packets again
					// doing so prior to this point could result in an out-of-order stream which would probably cause a desync

					m_Synchronized = true;
				}
				else if( Packet->GetID( ) == CGPSProtocol :: GPS_ACK && Data.size( ) == 8 )
				{
					uint32_t LastPacket = UTIL_ByteArrayToUInt32( Data, false, 4 );
					uint32_t PacketsAlreadyUnqueued = m_TotalPacketsReceivedFromLocal - m_PacketBuffer.size( );

					if( LastPacket > PacketsAlreadyUnqueued )
					{
						uint32_t PacketsToUnqueue = LastPacket - PacketsAlreadyUnqueued;

						if( PacketsToUnqueue > m_PacketBuffer.size( ) )
						{
							CONSOLE_Print( "[GGB] received GPS_ACK with last packet > total packets sent" );
							PacketsToUnqueue = m_PacketBuffer.size( );
						}

						while( PacketsToUnqueue > 0 )
						{
							delete m_PacketBuffer.front( );
							m_PacketBuffer.pop( );
							PacketsToUnqueue--;
						}
					}
				}
				else if( Packet->GetID( ) == CGPSProtocol :: GPS_REJECT && Data.size( ) == 8 )
				{
					uint32_t Reason = UTIL_ByteArrayToUInt32( Data, false, 4 );

					if( Reason == REJECTGPS_INVALID )
						CONSOLE_Print( "[GGB] rejected by remote server: invalid data" );
					else if( Reason == REJECTGPS_NOTFOUND )
						CONSOLE_Print( "[GGB] rejected by remote server: player not found in any running games" );

					m_LocalSocket->Disconnect( );
				}
			}
		}

		delete Packet;
	}
}


void CGPG :: SendLocalChat( string message )
{
    if( m_LocalSocket )
	{
		if( m_GameStarted )
		{
			if( message.size( ) > 127 )
				message = message.substr( 0, 127 );

			m_LocalSocket->PutBytes( m_GameProtocol->SEND_W3GS_CHAT_FROM_HOST( m_ChatPID, UTIL_CreateByteArray( m_ChatPID ), 32, UTIL_CreateByteArray( (uint32_t)0, false ), message ) );
		}
		else
		{
			if( message.size( ) > 254 )
				message = message.substr( 0, 254 );

			m_LocalSocket->PutBytes( m_GameProtocol->SEND_W3GS_CHAT_FROM_HOST( m_ChatPID, UTIL_CreateByteArray( m_ChatPID ), 16, BYTEARRAY( ), message ) );
		}
	}
}

void CGPG :: SendEmptyAction( )
{
	// we can't send any empty actions while the lag screen is up
	// so we keep track of who the lag screen is currently showing (if anyone) and we tear it down, send the empty action, and put it back up

	for( vector<unsigned char> :: iterator i = m_Laggers.begin( ); i != m_Laggers.end( ); i++ )
	{
		BYTEARRAY StopLag;
		StopLag.push_back( 0xF7 );
		StopLag.push_back( 0x11 );
		StopLag.push_back( 0x09 );
		StopLag.push_back( 0 );
		StopLag.push_back( *i );
		UTIL_AppendByteArray( StopLag, (uint32_t)60000, false );
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

	if( !m_Laggers.empty( ) )
	{
		BYTEARRAY StartLag;
		StartLag.push_back( 0xF7 );
		StartLag.push_back( 0x10 );
		StartLag.push_back( 0 );
		StartLag.push_back( 0 );
		StartLag.push_back( m_Laggers.size( ) );

		for( vector<unsigned char> :: iterator i = m_Laggers.begin( ); i != m_Laggers.end( ); i++ )
		{
			// using a lag time of 60000 ms means the counter will start at zero
			// hopefully warcraft 3 doesn't care about wild variations in the lag time in subsequent packets

			StartLag.push_back( *i );
			UTIL_AppendByteArray( StartLag, (uint32_t)60000, false );
		}

		BYTEARRAY LengthBytes;
		LengthBytes = UTIL_CreateByteArray( (uint16_t)StartLag.size( ), false );
		StartLag[2] = LengthBytes[0];
		StartLag[3] = LengthBytes[1];
		m_LocalSocket->PutBytes( StartLag );
	}
}










CGame::CGame(std::string nGameName, uint32_t nUniqueID,uint16_t nPort,bool nGProxyGame)
{
	m_GameName = nGameName;
	m_UniqueID = nUniqueID;
	m_Port = nPort;
	m_SupportsGProxy = nGProxyGame;
}
CGame::~CGame()
{
}
