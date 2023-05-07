// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com



#include "Antihack.h"
#include "CustomFeatures.h"


bool PacketInitialized = false;

struct CDataStore {
	void**			vtable;				//0x0	Offset(DATASTORECACHE1460_VTABLE)
	void*			packet;				//0x4	pointer to real packet
	uint32_t		unk_8;				//0x8	default 0
	uint32_t		unk_C;				//0xC	default 0x5B4
	uint32_t		sizePacket;			//0x10  sizeof(packet)
	int				unk_14;				//0x14	default -1
};


void Handle_Jass_Packet( unsigned char * packetraw, size_t packetsize );

/*  I just use CNetCommandTeamChangeAlliance for send and recv packets  */
typedef int( __fastcall * RecvNetCommand )( int a1, int a2, CDataStore * packetdata, char pid, int a5, int a6, int a7, int a8, int a9 );
RecvNetCommand CNetCommandTeamChangeAlliance_org;
RecvNetCommand CNetCommandTeamChangeAlliance_ptr;

int __fastcall CNetCommandTeamChangeAlliance_my( int a1, int a2, CDataStore * packetdata, char pid, int a5, int a6, int a7, int a8, int a9 )
{
	if ( packetdata && packetdata->packet && packetdata->sizePacket > 0 )
	{
		unsigned char * packetraw = ( unsigned char * )packetdata->packet;

		if ( packetraw[ 1 ] == 0xFF )
		{
			if ( PacketInitialized )
			{
				Handle_Jass_Packet( packetraw, packetdata->sizePacket );
			}

			return 0;
		}
	}

	return CNetCommandTeamChangeAlliance_ptr(a1,a2,packetdata,pid,a5,a6,a7,a8,a9);
}

std::vector<unsigned char> BytesToSend;

void PacketSend( void *pPacket, uint32_t size ) {
	void *( __fastcall * SendGamePacket ) ( CDataStore* packet, DWORD zero );
	CDataStore sender = CDataStore( );
	sender.vtable = ( void ** )( 0x932D2C + GameDll );
	sender.packet = pPacket;
	sender.unk_8 = 0;
	sender.unk_C = 0x5B4;
	sender.sizePacket = size;
	sender.unk_14 = -1;
	SendGamePacket = ( void *( __fastcall * ) ( CDataStore* packet, DWORD zero ) )( GameDll + 0x54D970 );
	SendGamePacket( &sender, 0 );
}

void Packet_Clear( )
{
	BytesToSend.clear( );
}


void Packet_Initialize( )
{
	BytesToSend.clear( );
}


void Packet_PushInteger( unsigned int IntegerData )
{
	unsigned char * data = ( unsigned char* )&IntegerData;
	BytesToSend.push_back( data[ 3 ] );
	BytesToSend.push_back( data[ 2 ] );
	BytesToSend.push_back( data[ 1 ] );
	BytesToSend.push_back( data[ 0 ] );
}

int Packet_PopInteger( )
{
	if ( BytesToSend.size( ) >= 4 )
	{
		int retval = *( int* )&BytesToSend[ 0 ];
		BytesToSend.erase( BytesToSend.begin( ), BytesToSend.begin( ) + 4 );
		return retval;
	}
	return 0;
}

void Packet_PushReal( unsigned int RealData )
{


}

float Packet_PopReal( )
{
	if ( BytesToSend.size( ) >= 4 )
	{
		float retval = *( float* )&BytesToSend[ 0 ];
		BytesToSend.erase( BytesToSend.begin( ), BytesToSend.begin( ) + 4 );
		return retval;
	}
	return 0;
}

void Handle_Jass_Packet( unsigned char * packetraw, size_t packetsize )
{

}


void InitNativesList( )
{


}



void NewJassNativesInitialize( )
{


}