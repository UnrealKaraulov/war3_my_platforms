/*
 * Copyright (C) 2000  Ross Combs (rocombs@cs.nmsu.edu)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */


/*****/
#ifndef JUST_NEED_TYPES

#ifndef INCLUDED_HANDLE_BNET_PROTOS
#define INCLUDED_HANDLE_BNET_PROTOS

#define JUST_NEED_TYPES
#include "connection.h"
#include "common/packet.h"
#undef JUST_NEED_TYPES
//here
extern std::vector<std::string> HostBotUsers;
extern std::vector<std::string> CrcWhiteList;
extern std::vector<std::string> BlackListUsers;
#include <filesystem>
namespace fs = std::filesystem;
namespace pvpgn
{

	namespace bnetd
	{
		
		struct HostPlayersSettings
		{
			std::string Players;
			bool ForStats;
		};

		struct HostModesSettings
		{
			std::string Mode;
			std::vector<std::string> Players;
			bool ForStats;
		};

		struct MapHostStruct
		{
			std::string MapName;
			std::string MapCode;
			std::string MapLocalPath;


			std::string StatsType;

			std::vector<HostPlayersSettings> PlayersSettings;
			std::string DefaultPlayers;

			std::vector<HostModesSettings> HostModes;
			std::string DefaultMode;
		
			
			
			bool stats;
		
			//std::vector<std::string> MapModes;
			//std::vector<std::string> Players;

			std::string _MapModes;
			//std::string _Players;
			
			std::string Category;
		
			uint32_t crc32;
		};
		extern std::vector<MapHostStruct> MapHostStructList;


		extern int handle_bnet_packet(t_connection * c, t_packet const * const packet);
		//here
		extern void UpdatePlayerStatsTick( );
		extern void UpdatePlayerStatsThread( );
		extern void InitHostBotList( );
		extern bool IsHostBot( const char * name );
		extern void InitCrcWhiteList( );
		extern bool IsInCrcWhiteList( const char * name );
		extern void InitBlackList( );
		extern bool IsInBlackList( const char * name );

		extern bool IsAMHHashOkay( unsigned int hash );
		extern void AddAMHHash( unsigned int hash );

	
	}

}



#endif
#endif
