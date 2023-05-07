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
//#else
// #include <stdint.h>
//#endif

// STL
#include "ms_stdint.h"
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


// new
// написал для удобного чтения из куска памяти :)
class SmallBinReader
{
public:
	std::vector<unsigned char> internaldata;
	bool ErrorFound = false;
	int Operations = 0; // for detect error

	SmallBinReader( const unsigned char * data, int size )
	{
		if ( Operations )
		{
			internaldata.clear( );
			Operations = 0;
			ErrorFound = false;
		}
		Operations++;

		if ( size )
			internaldata = std::vector<unsigned char>( data, data + size );
		else
			internaldata = std::vector<unsigned char >( );
	}

	void WriteString( const std::string & str )
	{
		unsigned char * memforwrite = new unsigned char[ str.length( ) + 1 ];
		memcpy_s( memforwrite, str.length( ) + 1, str.data( ), str.length( ) );

		for ( int i = 0; i < str.length( ); i++ )
		{
			internaldata.push_back( memforwrite[ i ] );
		}
		internaldata.push_back( 0 );
		Operations++;
		delete[ ] memforwrite;
	}

	void WriteInt( int val )
	{
		unsigned char * memforwrite = new unsigned char[ 4 ];
		memcpy_s( memforwrite, 4, ( unsigned char * )&val, 4 );
		for ( int i = 0; i < 4; i++ )
		{
			internaldata.push_back( memforwrite[ i ] );
		}
		Operations++;
		delete[ ] memforwrite;
	}

	void WriteUInt( unsigned int val )
	{
		unsigned char * memforwrite = new unsigned char[ 4 ];
		memcpy_s( memforwrite, 4, ( unsigned char * )&val, 4 );
		for ( int i = 0; i < 4; i++ )
		{
			internaldata.push_back( memforwrite[ i ] );
		}
		Operations++;
		delete[ ] memforwrite;
	}

	void WriteShort( short val )
	{
		unsigned char * memforwrite = new unsigned char[ 4 ];
		memcpy_s( memforwrite, 4, ( unsigned char * )&val, 4 );
		for ( int i = 0; i < 4; i++ )
		{
			internaldata.push_back( memforwrite[ i ] );
		}
		Operations++;
		delete[ ] memforwrite;
	}

	void WriteFloat( float val )
	{
		unsigned char * memforwrite = new unsigned char[ 4 ];
		memcpy_s( memforwrite, 4, ( unsigned char * )&val, 4 );
		for ( int i = 0; i < 4; i++ )
		{
			internaldata.push_back( memforwrite[ i ] );
		}
		Operations++;
		delete[ ] memforwrite;
	}

	void WriteUShort( unsigned short val )
	{
		unsigned char * memforwrite = new unsigned char[ 4 ];
		memcpy_s( memforwrite, 4, ( unsigned char * )&val, 4 );
		for ( int i = 0; i < 4; i++ )
		{
			internaldata.push_back( memforwrite[ i ] );
		}
		Operations++;
		delete[ ] memforwrite;
	}

	void WriteByte( unsigned char val )
	{
		internaldata.push_back( val );
		Operations++;
	}

	void WriteChar( char val )
	{
		unsigned char * memforwrite = new unsigned char[ 1 ];
		memcpy_s( memforwrite, 1, ( unsigned char * )&val, 1 );
		internaldata.push_back( memforwrite[ 0 ] );
		Operations++;
		delete[ ] memforwrite;
	}

	std::string ReadStr( )
	{
		if ( internaldata.empty( ) )
		{
			ErrorFound = true;
			return "";
		}
		Operations++;
		std::vector<unsigned char> outstr;

		while ( !internaldata.empty( ) && internaldata[ 0 ] != '\0' )
		{
			outstr.push_back( internaldata[ 0 ] );
			internaldata.erase( internaldata.begin( ) );
		}
		if ( !internaldata.empty( ) )
			internaldata.erase( internaldata.begin( ) );

		return std::string( &outstr[ 0 ], &outstr[ 0 ] + outstr.size( ) );
	}

	std::vector<unsigned char> ReadData( )
	{
		Operations++;
		return internaldata;
	}

	unsigned char ReadByte( )
	{
		if ( internaldata.empty( ) )
		{
			ErrorFound = true;
			return 0;
		}
		Operations++;
		unsigned char result = internaldata[ 0 ];
		internaldata.erase( internaldata.begin( ) );
		return result;
	}

	char ReadChar( )
	{
		if ( internaldata.empty( ) )
		{
			ErrorFound = true;
			return 0;
		}
		Operations++;

		char result = *( char* )&internaldata[ 0 ];
		internaldata.erase( internaldata.begin( ) );
		return result;
	}

	short ReadShort( )
	{
		if ( internaldata.empty( ) || internaldata.size( ) < 2 )
		{
			ErrorFound = true;
			return 0;
		}
		Operations++;

		short result = *( short* )&internaldata[ 0 ];
		internaldata.erase( internaldata.begin( ) );
		return result;
	}

	unsigned short ReadUShort( )
	{
		if ( internaldata.empty( ) || internaldata.size( ) < 2 )
		{
			ErrorFound = true;
			return 0;
		}
		Operations++;

		unsigned short result = *( unsigned short* )&internaldata[ 0 ];
		internaldata.erase( internaldata.begin( ) );
		return result;
	}

	int ReadInt( )
	{
		if ( internaldata.empty( ) || internaldata.size( ) < 4 )
		{
			ErrorFound = true;
			return 0;
		}
		Operations++;

		int result = *( int* )&internaldata[ 0 ];
		internaldata.erase( internaldata.begin( ) );
		internaldata.erase( internaldata.begin( ) );
		internaldata.erase( internaldata.begin( ) );
		internaldata.erase( internaldata.begin( ) );
		return result;
	}

	int ReadUInt( )
	{
		if ( internaldata.empty( ) || internaldata.size( ) < 4 )
		{
			ErrorFound = true;
			return 0;
		}
		Operations++;

		unsigned int result = *( unsigned int* )&internaldata[ 0 ];
		internaldata.erase( internaldata.begin( ) );
		internaldata.erase( internaldata.begin( ) );
		internaldata.erase( internaldata.begin( ) );
		internaldata.erase( internaldata.begin( ) );
		return result;
	}

};

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
void CONSOLE_Print( string message, bool toMainBuffer = true );
void CONSOLE_Print(BYTEARRAY message);
void CONSOLE_PrintNoCRLF( string message );
void CONSOLE_ChangeChannel( string channel );
void CONSOLE_AddChannelUser( string name, int flag );
void CONSOLE_UpdateChannelUser ( string name, int flag );
void CONSOLE_RemoveChannelUser( string name );
void CONSOLE_RemoveChannelUsers( );
void CONSOLE_Draw( );
void CONSOLE_Resize( );
extern int LoginStatus;

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

class CGProxy
{
public:
    string c_BotName;
	string m_VersionNum;
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
	bool m_SendInfo;


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


typedef void( __stdcall * pProcessCommandsCallback )( const char* text, const char* arg1,
	const char* arg2, const char* arg3, const char* arg4, const char* arg5, const char* arg6,
	const char* arg7, const char* arg8, const char* arg9, const char* arg10 );

extern pProcessCommandsCallback CommandsCallback;
bool __stdcall ProcessCommandsCallback( const char* text = "", const char* arg1 = "",
	const char* arg2 = "", const char* arg3 = "", const char* arg4 = "", const char* arg5 = "", const char* arg6 = "", const char* arg7 = "", const char* arg8 = "", const char* arg9 = "", const char* arg10 = "" );

#endif
