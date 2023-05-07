#include "CustomFeatures.h"
#include "Antihack.h"


std::vector<int> GetUnitsFromGroup( int grouphandle )
{
	std::vector<int> localvector;

	// Get group address
	int GroupAddr = ConvertHandle( grouphandle );

	if ( GroupAddr )
	{
	
		// Set group offset
		GroupAddr += 0x24;
		// Set group data
		int GroupData = *( int* )( GroupAddr + 0xC );
		while ( GroupData > 0 )
		{
	
			// Get unit 
			int UnitAddr = *( int* )( GroupData + 0x8 );


			// Get next data
			GroupData = *( int* )( GroupData + 0x4 );
			// save unit to list
			localvector.push_back( UnitAddr );
		}
	}
	return localvector;
}

// Является ли юнит героем
BOOL __stdcall IsHero( int unitaddr )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__, __LINE__ );//;
#endif
	if ( unitaddr > 0 )
	{
		unsigned int ishero = *( unsigned int* )( unitaddr + 48 );
		ishero = ishero >> 24;
		ishero = ishero - 64;
		return ishero < 0x19;
	}
	return FALSE;
}


BOOL __stdcall IsUnitIllusion( int unitaddr )
{
	return ( *( unsigned int* )( unitaddr + 0x5C ) & 0x40000000 );
}

// Проверяет юнит или не юнит
BOOL __stdcall IsNotBadUnit( int unitaddr, BOOL onlymem )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__, __LINE__ );//;
#endif
	if ( unitaddr > 0 )
	{
		int xaddraddr = ( int )&UnitVtable;

		if ( *( BYTE* )xaddraddr != *( BYTE* )unitaddr )
			return FALSE;
		else if ( *( BYTE* )( xaddraddr + 1 ) != *( BYTE* )( unitaddr + 1 ) )
			return FALSE;
		else if ( *( BYTE* )( xaddraddr + 2 ) != *( BYTE* )( unitaddr + 2 ) )
			return FALSE;
		else if ( *( BYTE* )( xaddraddr + 3 ) != *( BYTE* )( unitaddr + 3 ) )
			return FALSE;

		unsigned int x1 = *( unsigned int* )( unitaddr + 0xC );
		unsigned int y1 = *( unsigned int* )( unitaddr + 0x10 );

		int udata = *( int* )( unitaddr + 0x28 );


		if ( x1 == 0xFFFFFFFF || y1 == 0xFFFFFFFF || udata == 0 )
		{
			return FALSE;
		}

		if ( onlymem )
			return TRUE;

		unsigned int unitflag = *( unsigned int* )( unitaddr + 0x20 );
		unsigned int unitflag2 = *( unsigned int* )( unitaddr + 0x5C );

		if ( unitflag & 1u )
		{
			return FALSE;
		}

		if ( !( unitflag & 2u ) )
		{
			return FALSE;
		}

		if ( unitflag2 & 0x100u )
		{
			return FALSE;
		}

	/*	if ( unitflag2 == 0x1001u )
		{
			if ( SetInfoObjDebugVal )
			{
				PrintText( "Flag 4 bad" );
			}
			return FALSE;
		}
*/
		return TRUE;
	}
	return FALSE;
}


// Проверяет враг юнит локальному игроку или нет
BOOL __stdcall IsEnemy( int UnitAddr )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__, __LINE__ );//;
#endif
	if ( UnitAddr > 0 && IsNotBadUnit( UnitAddr ) )
	{
		int unitownerslot = GetUnitOwnerSlot( ( int )UnitAddr );

		if ( GetLocalPlayerId( ) == unitownerslot )
		{
#ifdef DOTA_HELPER_LOG
			AddNewLineToDotaHelperLog( __func__, __LINE__ );//;
#endif
			return FALSE;
		}

		if ( unitownerslot <= 15 && unitownerslot >= 0 && GetLocalPlayerId( ) <= 15 && GetLocalPlayerId( ) >= 0 )
		{
			int Player1 = Player( unitownerslot );
			int Player2 = Player( GetLocalPlayerId( ) );

			if ( Player1 == Player2 )
			{
#ifdef DOTA_HELPER_LOG
				AddNewLineToDotaHelperLog( __func__, __LINE__ );//;
#endif
				return FALSE;
			}
			if ( Player1 == 0 || Player2 == 0 )
			{
#ifdef DOTA_HELPER_LOG
				AddNewLineToDotaHelperLog( __func__, __LINE__ );//;
#endif
				return FALSE;
			}

			BOOL retval = IsPlayerEnemy( Player1, Player2 );
#ifdef DOTA_HELPER_LOG
			AddNewLineToDotaHelperLog( __func__, __LINE__ );//;
#endif
			return retval;
		}
	}
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__, __LINE__ );//;
#endif
	return FALSE;
}


// Проверяет предмет или не предмет
BOOL __stdcall IsNotBadItem( int itemaddr, BOOL extracheck )
{
	if ( itemaddr > 0 )
	{
		int xaddraddr = ( int )&ItemVtable;

		if ( *( BYTE* )xaddraddr != *( BYTE* )itemaddr )
			return FALSE;
		else if ( *( BYTE* )( xaddraddr + 1 ) != *( BYTE* )( itemaddr + 1 ) )
			return FALSE;
		else if ( *( BYTE* )( xaddraddr + 2 ) != *( BYTE* )( itemaddr + 2 ) )
			return FALSE;
		else if ( *( BYTE* )( xaddraddr + 3 ) != *( BYTE* )( itemaddr + 3 ) )
			return FALSE;

		if ( extracheck )
		{
			float hitpoint = *( float * )( itemaddr + 0x58 );

			return hitpoint != 0.0f;
		}

		return TRUE;
	}

	return FALSE;
}






int ReadObjectAddrFromGlobalMat( unsigned int a1, unsigned int a2 )
{
	BOOL found1;
	int result;
	int AddrType1;
	int v5;

	if ( !( a1 >> 31 ) )
	{
		if ( a1 < *( unsigned int * )( *( int* )pGameClass1 + 28 ) )
		{
			found1 = *( int * )( *( int * )( *( int* )pGameClass1 + 12 ) + 8 * a1 ) == -2;
			if ( !found1 )
				return 0;
			if ( a1 >> 31 )
			{
				AddrType1 = *( int * )( *( int * )( *( int* )pGameClass1 + 44 ) + 8 * a1 + 4 );
				result = *( unsigned int * )( AddrType1 + 24 ) != a2 ? 0 : AddrType1;
			}
			else
			{
				v5 = *( int * )( *( int * )( *( int* )pGameClass1 + 12 ) + 8 * a1 + 4 );
				result = *( unsigned int * )( v5 + 24 ) != a2 ? 0 : v5;
			}
			return result;
		}
		return 0;
	}
	if ( ( a1 & 0x7FFFFFFF ) >= *( unsigned int * )( *( int* )pGameClass1 + 60 ) )
		return 0;
	found1 = *( int * )( *( int * )( *( int* )pGameClass1 + 44 ) + 8 * a1 ) == -2;
	if ( !found1 )
		return 0;
	if ( a1 >> 31 )
	{
		AddrType1 = *( int * )( *( int * )( *( int* )pGameClass1 + 44 ) + 8 * a1 + 4 );
		result = *( unsigned int * )( AddrType1 + 24 ) != a2 ? 0 : AddrType1;
	}
	else
	{
		v5 = *( int * )( *( int * )( *( int* )pGameClass1 + 12 ) + 8 * a1 + 4 );
		result = *( unsigned int * )( v5 + 24 ) != a2 ? 0 : v5;
	}
	return result;
}


int GetObjectDataAddr( int addr )
{
	int mataddr;
	int result; // eax@3

	mataddr = ReadObjectAddrFromGlobalMat( *( unsigned int * )addr, *( unsigned int * )( addr + 4 ) );

	if ( !mataddr || *( int * )( mataddr + 32 ) )
		result = 0;
	else
		result = *( int * )( mataddr + 84 );
	return result;
}


vector<int> ReturnAbils;

int * FindUnitAbils( int unitaddr, unsigned int * count, int abilcode, int abilbasecode )
{
	if ( !ReturnAbils.empty( ) )
		ReturnAbils.clear( );
	*count = 0;
	if ( unitaddr > 0 )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );//;
#endif
		int pAddr1 = unitaddr + 0x1DC;
		int pAddr2 = unitaddr + 0x1E0;

		if ( ( int )( *( unsigned int * )( pAddr1 ) & *( unsigned int * )( pAddr2 ) ) != -1 )
		{
			//PrintText( "Found abils ... 1" );
			int pData = GetObjectDataAddr( pAddr1 );

			while ( pData > 0 )
			{
				//	PrintText( "Found abils ... 2" );
				int pData2 = *( int* )( pData + 0x54 );
				if ( pData2 > 0 )
				{
					/*	char foundabil3[ 100 ];
						sprintf_s( foundabil3, "%s:%X:%X", "Found new abil:", *( int* )( pData2 + 0x30 ), *( int* )( pData2 + 0x34 ) );

						PrintText(foundabil3 );*/

					if ( abilcode != 0 && *( int* )( pData2 + 0x34 ) == abilcode )
					{
						if ( abilbasecode != 0 && *( int* )( pData2 + 0x30 ) == abilbasecode )
						{
							ReturnAbils.push_back( pData );
						}
						else if ( abilbasecode == 0 )
						{
							ReturnAbils.push_back( pData );
						}
					}
					else if ( abilcode == 0 )
					{
						if ( abilbasecode != 0 && *( int* )( pData2 + 0x30 ) == abilbasecode )
						{
							ReturnAbils.push_back( pData );
						}
						else if ( abilbasecode == 0 )
						{
							ReturnAbils.push_back( pData );
						}
					}
				}
				pData = GetObjectDataAddr( pData + 0x24 );
			}

			*count = ReturnAbils.size( );
		}
	}

	return &ReturnAbils[ 0 ];
}





int * GetItemCountAndItemArray( int ** itemarray )
{
	int GlobalClassOffset = *( int* )( pW3XGlobalClass );
	if ( GlobalClassOffset )
	{
		int ItemsOffset1 = *( int* )( GlobalClassOffset + 0x3BC ) + 0x10;
		if ( ItemsOffset1 )
		{
			int * ItemsCount = ( int * )( ItemsOffset1 + 0x604 );
			if ( *ItemsCount > 0 )
			{
				*itemarray = ( int * ) *( int* )( ItemsOffset1 + 0x608 );
				return ItemsCount;
			}
		}
	}

	*itemarray = 0;
	return 0;
}



int * GetUnitCountAndUnitArray( int ** unitarray )
{
	int GlobalClassOffset = *( int* )( pW3XGlobalClass );
	if ( GlobalClassOffset )
	{
		int UnitsOffset1 = *( int* )( GlobalClassOffset + 0x3BC );
		if ( UnitsOffset1 )
		{
			int * UnitsCount = ( int * )( UnitsOffset1 + 0x604 );
			if ( *UnitsCount > 0 )
			{
				*unitarray = ( int * ) *( int* )( UnitsOffset1 + 0x608 );
				return UnitsCount;
			}
		}
	}

	*unitarray = 0;
	return 0;
}



void GetUnitLocation2D( int unitaddr, float * x, float * y )
{
	if ( unitaddr && *( int* )( unitaddr + 0x284 ) != 0 && *( float* )( unitaddr + 0x288 ) != 0 )
	{
		*x = *( float* )( unitaddr + 0x284 );
		*y = *( float* )( unitaddr + 0x288 );
	}
	else
	{
		*x = 0.0;
		*y = 0.0;
	}
}


void GetItemLocation2D( int itemaddr, float * x, float * y )
{
	if ( itemaddr )
	{
		int iteminfo = *( int * )( itemaddr + 0x28 );
		if ( iteminfo )
		{
			*x = *( float* )( iteminfo + 0x88 );
			*y = *( float* )( iteminfo + 0x8C );
		}
		else
		{
			*x = 0.0;
			*y = 0.0;
		}
	}
	else
	{
		*x = 0.0;
		*y = 0.0;
	}
}




int GetUnitAddressFloatsRelated( int unitaddr, int step )
{
	if ( unitaddr > 0 )
	{
		int offset1 = unitaddr + step;
		int offset2 = *( int* )pGameClass1;

		if ( *( int* )offset1 &&  offset2 )
		{
			offset1 = *( int* )offset1;
			offset2 = *( int* )( offset2 + 0xC );
			if ( offset2 )
			{
				return *( int* )( ( offset1 * 8 ) + offset2 + 4 );
			}
		}
	}
	return 0;
}


float GetUnitHPregen( int unitaddr )
{
	float result = 0.0f;
	if ( unitaddr > 0 )
	{
		int offset1 = GetUnitAddressFloatsRelated( unitaddr, 0xA0 );
		if ( offset1 )
		{

			result = *( float* )( offset1 + 0x7C );
		}
	}
	return result;
}

float GetUnitHP( int unitaddr )
{
	float result = 0.0f;
	if ( unitaddr > 0 )
	{
		int offset1 = GetUnitAddressFloatsRelated( unitaddr, 0xA0 );
		if ( offset1 )
		{

			result = *( float* )( offset1 + 0x80 );
		}
	}
	return result;
}


float GetUnitMP( int unitaddr )
{
	float result = 0.0f;
	if ( unitaddr > 0 )
	{
		int offset1 = GetUnitAddressFloatsRelated( unitaddr, 0xC0 );
		if ( offset1 )
		{/*
			char test[ 100 ];
			sprintf_s( test, 100, "Unit MP struct: %X", offset1 );
			PrintText( test );*/
			result = *( float* )( offset1 + 0x80 );
		}
	}
	return result;
}

float GetUnitMPregen( int unitaddr )
{
	float result = 0.0f;
	if ( unitaddr > 0 )
	{
		int offset1 = GetUnitAddressFloatsRelated( unitaddr, 0xC0 );
		if ( offset1 )
		{
			result = *( float* )( offset1 + 0x7C );
		}
	}
	return result;
}




float GetUnitX_real( int unitaddr )
{
	float result = 0.0f;
	if ( unitaddr > 0 )
	{
		int offset1 = GetUnitAddressFloatsRelated( unitaddr, 0xA0 );
		if ( offset1 > 0 )
		{
			unsigned int x1 = *( unsigned int* )( offset1 + 0x14 );
			unsigned int y1 = *( unsigned int* )( offset1 + 0x18 );
			if ( x1 != 0xFFFFFFFF && y1 != 0xFFFFFFFF )
			{
				offset1 = *( int* )( offset1 + 0x28 );
				if ( offset1 > 0 )
				{
					result = *( float* )( offset1 + 0x54 );
				}
			}
		}
	}
	return result;
}

float GetUnitY_real( int unitaddr )
{
	float result = 0.0f;
	if ( unitaddr > 0 )
	{
		int offset1 = GetUnitAddressFloatsRelated( unitaddr, 0xA0 );
		if ( offset1 > 0 )
		{
			unsigned int x1 = *( unsigned int* )( offset1 + 0x14 );
			unsigned int y1 = *( unsigned int* )( offset1 + 0x18 );
			if ( x1 != 0xFFFFFFFF && y1 != 0xFFFFFFFF )
			{
				offset1 = *( int* )( offset1 + 0x28 );
				if ( offset1 > 0 )
				{
					result = *( float* )( offset1 + 0x58 );
				}
			}
		}
	}
	return result;
}


void (__thiscall * SelectUnitReal)( int pPlayerSelectData, int pUnit, int id, int unk1, int unk2, int unk3 ) = NULL;
void( __thiscall * UpdatePlayerSelection )( int pPlayerSelectData, int unk ) = NULL;
int( __cdecl * ClearSelection)( void ) = NULL;

int( __thiscall * sub_6F332700 )( void *a1 );

void SelectUnit( int unit )
{
	if ( SelectUnitReal && UpdatePlayerSelection && IsNotBadUnit( unit ) )
	{
		sub_6F332700 = ( int( __thiscall * )( void *a1 ) )( GameDll + 0x332700 );
		int playerslot = GetLocalPlayerId( );
		int LocalPlayer = GetPlayerByNumber( playerslot );
		int playerseldata = *( int * )( LocalPlayer + 0x34 );
		SelectUnitReal( playerseldata, unit, playerslot, 0, 1, 1 );
		UpdatePlayerSelection( ( int )playerseldata, 0 );
		sub_6F332700( 0 );
	}
}