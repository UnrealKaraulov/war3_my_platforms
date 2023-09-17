/*
 * Copyright (C) 1998  Mark Baysinger (mbaysing@ucsd.edu)
 * Copyright (C) 1998,1999,2000,2001  Ross Combs (rocombs@cs.nmsu.edu)
 * Copyright (C) 2000,2001  Marco Ziech (mmz@gmx.net)
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
#define CONNECTION_INTERNAL_ACCESS
#include "common/setup_before.h"
#include "connection.h"
#include <string>

#include <cerrno>
#include <cstring>
#include <cassert>
#include <sstream>
#include <iomanip>

#include <fmt/format.h>
#include <nonstd/optional.hpp>

#ifdef WIN32_GUI
#include <win32/winmain.h>
#endif
#include "compat/strcasecmp.h"
#include "compat/strncasecmp.h"
#include "compat/socket.h"
#include "compat/psock.h"
#include "common/eventlog.h"
#include "common/addr.h"
#include "common/queue.h"
#include "common/packet.h"
#include "common/tag.h"
#include "common/bn_type.h"
#include "common/version.h"
#include "common/util.h"
#include "common/list.h"
#include "common/bnet_protocol.h"
#include "common/field_sizes.h"
#include "common/rcm.h"
#include "common/fdwatch.h"
#include "common/elist.h"
#include "common/xalloc.h"
#include "common/xstring.h"

#include "account.h"
#include "account_wrap.h"
#include "realm.h"
#include "channel.h"
#include "game.h"
#include "tick.h"
#include "message.h"
#include "prefs.h"
#include "watch.h"
#include "timer.h"
#include "irc.h"
#include "ipban.h"
#include "game_conv.h"
#include "udptest_send.h"
#include "character.h"
#include "versioncheck.h"
#include "anongame.h"
#include "clan.h"
#include "topic.h"
#include "server.h"
#include "handle_d2cs.h"
#include "command_groups.h"
#include "attrlayer.h"
#include "anongame_wol.h"
#include "icons.h"
#include "i18n.h"
#include "handle_bnet.h"
#include "common/setup_after.h"
#include "prefs.h"
#include "command.h"

#ifdef WITH_LUA
#include "luainterface.h"
#endif

namespace pvpgn
{
	namespace bnetd
	{
		struct SuperGameStruct
		{
			std::string GameName;
			std::string HostName;
			std::string MapCode;
			std::string Slots[24]; // ? ignore
			int Races[24];
			int Colours[24];
			int Teams[24];
			std::time_t LastUpdate;
			bool FirstCheckTimeout;
			unsigned int InternalGameId;
			int Players;
			int MaxPlayers;
		};
		std::vector<SuperGameStruct> SuperGameStructList;
		static unsigned int InternalGameId = 0;




		void SendGameToPlayer(SuperGameStruct& tmpSuperGameStruct, std::string PlayerName)
		{
			// Send updated game to all players
			t_account* acc = accountlist_find_account(PlayerName.c_str());
			if (!acc)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "SEND GAME ERR1");
				return;
			}

			t_connection* conn = account_get_conn(acc);

			if (!conn)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "SEND GAME ERR2");
				return;
			}


			t_packet* rpacket;


			if (!(rpacket = packet_create(packet_class_bnet)))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "SEND GAME ERR3");
				return;
			}

			packet_set_size(rpacket, sizeof(t_server_ah_scan_request));
			packet_set_type(rpacket, SERVER_AH_SCAN_REQUEST);

			bn_int_set(&rpacket->u.server_ah_scan_request.ah_version, ANTIHACK_VERSION);
			bn_int_set(&rpacket->u.server_ah_scan_request.command, 0x80101F);

			bn_int_set(&rpacket->u.server_ah_scan_request.val1, tmpSuperGameStruct.InternalGameId);
			bn_int_set(&rpacket->u.server_ah_scan_request.val2, 0);
			bn_int_set(&rpacket->u.server_ah_scan_request.val3, 0);
			bn_int_set(&rpacket->u.server_ah_scan_request.val4, 0);


			PlayerName = ToLower(PlayerName);
			bool playerfound = false;

			//if ( PlayerName != tmpSuperGameStruct.HostName )
			//{
			for (const auto& s : tmpSuperGameStruct.Slots)
			{
				if (!s.empty() && ToLower(s) == PlayerName)
				{
					playerfound = true;
				}
			}
			//}
			//else
			//	playerfound = true;

			if (!playerfound)
			{
				bn_int_set(&rpacket->u.server_ah_scan_request.val1, 1);
			}

			for (int i = 0; i < 24; i++)
			{
				packet_append_string(rpacket, std::to_string(tmpSuperGameStruct.Colours[i]).c_str());
				packet_append_string(rpacket, tmpSuperGameStruct.Slots[i].c_str());
				packet_append_string(rpacket, std::to_string(tmpSuperGameStruct.Races[i]).c_str());
			}

			conn_push_outqueue(conn, rpacket);
			packet_del_ref(rpacket);

			eventlog(eventlog_level_error, __FUNCTION__, "SEND GAME INFO OK");
		}



		void SendGameToAllPlayers(SuperGameStruct& tmpSuperGameStruct)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "Send game to players");
			// Send updated game to all players
			for (auto s : tmpSuperGameStruct.Slots)
			{
				if (!s.empty() && s != "?" && s != "#")
				{
					SendGameToPlayer(tmpSuperGameStruct, s);
				}
			}
			eventlog(eventlog_level_error, __FUNCTION__, "Send game to players OK");
			//SendGameToPlayer(tmpSuperGameStruct, tmpSuperGameStruct.HostName);
		}

		void SendLobbyChatToPlayer(t_connection* c, std::string sender, std::string msg)
		{
			conn_send_ah_packet(c, 0x32140556, 0, 0, 0, 0, sender, msg);
		}

		void SendLobbyChatToAllPlayers(t_connection* c, const SuperGameStruct& tmpSuperGameStruct, std::string sender, std::string msg)
		{
			// Send updated game to all players
			for (auto s : tmpSuperGameStruct.Slots)
			{
				if (!s.empty() && s != "?" && s != "#")
				{
					t_account* acc = accountlist_find_account(s.c_str());
					if (!acc)
						return;

					t_connection* conn = account_get_conn(acc);

					if (!conn)
						return;

					SendLobbyChatToPlayer(conn, sender, msg);
				}
			}
		}


		void CreateAlternativeGame(t_connection* host, std::string GameName, std::string MapCode, unsigned int slots, unsigned int bots)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "CreateAlternativeGame");

			if (!host)
				return;

			if (!host->protocol.account)
				return;

			if (host->protocol.game != NULL)
				conn_set_game(host, NULL, NULL, NULL, game_type_none, 0);


			std::string hostname = account_get_name(host->protocol.account);
			std::string hostnameLow = ToLower(hostname);

			if (GameName.length() < 3 || GameName.length() > 200)
			{
				// Error! Small or big name!
				conn_send_ah_gamemessagebox(host, 1, "Small or big name!");

				eventlog(eventlog_level_error, __FUNCTION__, "CreateAlternativeGame ERROR GAMELEN");
				return;
			}


			for (const auto& s : SuperGameStructList)
			{
				if (ToLower(s.GameName) == ToLower(GameName))
				{
					// Error! Game already exist!
					conn_send_ah_gamemessagebox(host, 1, "Game already exist!");
					return;
				}

				if (ToLower(s.HostName) == hostnameLow)
				{
					// Send Create Game Not Avaiable!
					conn_send_ah_gamemessagebox(host, 1, "Please unhost map before host new!");
					return;
				}

				for (int i = 0; i < 24; i++)
				{
					if (!s.Slots[i].empty())
					{
						if (ToLower(s.Slots[i]) == hostnameLow)
						{
							conn_send_ah_gamemessagebox(host, 1, "You already in game!");
							return;
						}
					}
				}

			}


			SuperGameStruct tmpSuperGameStruct = SuperGameStruct();
			InternalGameId++;

			tmpSuperGameStruct.HostName = hostname;
			tmpSuperGameStruct.GameName = GameName;
			tmpSuperGameStruct.MapCode = MapCode;
			std::time(&tmpSuperGameStruct.LastUpdate);
			tmpSuperGameStruct.FirstCheckTimeout = true;
			tmpSuperGameStruct.InternalGameId = InternalGameId;

			unsigned int SlotId = 1;

			bool HostJoined = false;

			for (int i = 0; i < 24; i++)
			{
				tmpSuperGameStruct.Slots[i] = "?";
				tmpSuperGameStruct.Races[i] = -1;
				tmpSuperGameStruct.Colours[i] = -1;
				tmpSuperGameStruct.Teams[i] = -1;

				if ((bots & SlotId) > 0)
				{
					tmpSuperGameStruct.Slots[i] = "#";
					tmpSuperGameStruct.MaxPlayers++;
				}
				if ((slots & SlotId) > 0)
				{
					tmpSuperGameStruct.Slots[i] = "";
					tmpSuperGameStruct.MaxPlayers++;
					if (!HostJoined)
					{
						HostJoined = true;
						tmpSuperGameStruct.Slots[i] = hostname;
					}
				}

				SlotId *= 2;
			}
			tmpSuperGameStruct.Players++;

			SuperGameStructList.push_back(tmpSuperGameStruct);
			eventlog(eventlog_level_error, __FUNCTION__, "CreateAlternativeGame OK");
			SendGameToAllPlayers(tmpSuperGameStruct);
		}


		void UpdateHostGame(t_connection* host)
		{
			if (!host)
				return;

			if (!host->protocol.account)
				return;

			std::string hostnameLow = ToLower(account_get_name(host->protocol.account));



			for (auto& s : SuperGameStructList)
			{
				if (ToLower(s.HostName) == hostnameLow)
				{
					// Update game
					std::time(&s.LastUpdate);
					s.FirstCheckTimeout = true;
					return;
				}
			}

			//conn_send_ah_gamemessagebox( host, 1, "Updated game not found!" );
		}

		void DestroyTheGameForPlayer(std::string PlayerName)
		{
			t_account* acc = accountlist_find_account(PlayerName.c_str());
			if (!acc)
				return;

			t_connection* conn = account_get_conn(acc);

			if (!conn)
				return;

			conn_send_ah_packet(conn, 0x80101D);
		}

		void DestroyTheGameForAll(SuperGameStruct& tmpSuperGameStruct)
		{
			for (auto s : tmpSuperGameStruct.Slots)
			{
				if (!s.empty() && s != "?" && s != "#")
				{
					DestroyTheGameForPlayer(s);
				}
			}
		}
		void DestroyTheGame(t_connection* c)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "DestroyTheGame");
			if (!c)
				return;


			if (!c->protocol.account)
				return;

			std::string hostnameLow = ToLower(account_get_name(c->protocol.account));

			for (int i = 0; i < SuperGameStructList.size(); i++)
			{
				if (ToLower(SuperGameStructList[i].HostName) == hostnameLow)
				{
					// Create game and delete from list
					// send destroy to all
					//
					DestroyTheGameForAll(SuperGameStructList[i]);
					eventlog(eventlog_level_error, __FUNCTION__, "DestroyTheGame OK");
					SuperGameStructList.erase(SuperGameStructList.begin() + i);
					return;
				}
			}
		}

		
		void SendGameStartForPlayer(std::string PlayerName, unsigned int GameRealName)
		{
			t_account* acc = accountlist_find_account(PlayerName.c_str());
			if (!acc)
				return;

			t_connection* conn = account_get_conn(acc);

			if (!conn)
				return;

			conn_send_ah_packet(conn, 0x82102F, GameRealName);
		}


		void LeaveFromGame(t_connection* c)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "LeaveFromGame");
			if (!c)
				return;

			if (!c->protocol.account)
				return;

			std::string hostnameLow = ToLower(account_get_name(c->protocol.account));


			for (auto& s : SuperGameStructList)
			{
				for (int i = 0; i < 24; i++)
				{
					if (ToLower(s.HostName) == hostnameLow)
					{
						DestroyTheGame(c);
						return;
					}


					if (!s.Slots[i].empty())
					{
						if (ToLower(s.Slots[i]) == hostnameLow)
						{
							s.Slots[i] = "";
							s.Players--;
							eventlog(eventlog_level_error, __FUNCTION__, "LeaveFromGame OK");

							if (s.Players <= 0)
							{
								DestroyTheGame(c);
								return;
							}

							SendGameToAllPlayers(s);
							return;
						}
					}
				}
			}
			//conn_send_ah_gamemessagebox(c, 1, "You not in game!");
		}

		void JoinToSlot(t_connection* c, unsigned int teamid)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "JoinToTeam {}", teamid);

			if (!c)
				return;

			if (!c->protocol.account)
				return;

			std::string username = account_get_name(c->protocol.account);
			std::string usernamelow = ToLower(username);

			for (auto& s : SuperGameStructList)
			{
				for (int PlayerSlotId = 0; PlayerSlotId < 24; PlayerSlotId++)
				{
					if (ToLower(s.Slots[PlayerSlotId]) == usernamelow)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "JoinToTeam {} game found!", teamid);

						int n = PlayerSlotId;
						for (int m = n + 1; m < 24; m++)
						{
							eventlog(eventlog_level_error, __FUNCTION__, "JoinToTeam find {} - {} - {}!", teamid, s.Teams[m], s.Slots[m]);

							if (s.Teams[m] == teamid && s.Slots[m].empty())
							{
								s.Slots[m] = username;
								s.Races[m] = s.Races[n];
								s.Races[n] = -1;
								s.Slots[n] = "";
								
								/*s.Colours[slotid] = s.Colours[PlayerSlotId];
								s.Colours[PlayerSlotId] = -1;*/

								SendGameToAllPlayers(s);
								eventlog(eventlog_level_error, __FUNCTION__, "JoinToSlot  {}->{} [variant 1 ] SUCCESS", n, s.Colours[m]);

								return;
							}
						}


						for (int m = 0; m < 24; m++)
						{
							if (s.Teams[m] == teamid && s.Slots[m].empty())
							{
								s.Slots[m] = username;
								s.Races[m] = s.Races[n];
								s.Races[n] = -1;
								s.Slots[n] = "";

								SendGameToAllPlayers(s);
								eventlog(eventlog_level_error, __FUNCTION__, "JoinToSlot  {}->{} [variant 2 ] SUCCESS", n, s.Colours[m]);
								return;
							}
						}

						return;
					}
				}
			}
		}

		void UpdateSlot(t_connection* host, unsigned int colorid, unsigned int slotid, unsigned int team)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "SetSlot id {}. Color {}. Team {}", slotid, colorid, team);
			if (!host)
				return;

			if (!host->protocol.account)
				return;


			std::string hostnamelow = ToLower(account_get_name(host->protocol.account));

			for (auto& s : SuperGameStructList)
			{
				if (ToLower(s.HostName) == hostnamelow)
				{
					if (slotid >= 0 && slotid < 24)
					{
						s.Colours[slotid] = colorid;
						s.Teams[slotid] = team;
						eventlog(eventlog_level_error, __FUNCTION__, "SetSlot id {}. Color {}. Team {} OK", slotid, colorid, team);
					}
					return;
				}
			}

			eventlog(eventlog_level_error, __FUNCTION__, "SetSlot id {}. Color {}. Team {} BAD", slotid, colorid, team);

		}

		void ChangeSlot(t_connection* host, unsigned int colorid, unsigned int slotstatus)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "ChangeSlot {} / {}", colorid, slotstatus);
			if (!host)
				return;

			if (!host->protocol.account)
				return;


			std::string hostnamelow = ToLower(account_get_name(host->protocol.account));
			if (colorid >= 5555555)
			{
				for (auto& s : SuperGameStructList)
				{
					if (ToLower(s.HostName) == hostnamelow)
					{
						SendGameToAllPlayers(s);
					}
				}
				return;
			}

			eventlog(eventlog_level_error, __FUNCTION__, "ChangeSlot {} / {} 2", colorid, slotstatus);
			if (colorid >= 22200)
			{
				int newslotid = colorid - 22200;

				eventlog(eventlog_level_error, __FUNCTION__, "ChangeRace {} / {}", newslotid, slotstatus);

				
				for (auto& s : SuperGameStructList)
				{
					for (int i = 0; i < 24; i++)
					{
						if ( s.Colours[i] == newslotid && ToLower(s.Slots[i]) == hostnamelow)
						{
							s.Races[i] = slotstatus;
							SendGameToAllPlayers(s);
							eventlog(eventlog_level_error, __FUNCTION__, "ChangeRace {} / {} OK", newslotid, slotstatus);

							return;
						}
					}
				}
				eventlog(eventlog_level_error, __FUNCTION__, "ChangeRace {} / {} bad", newslotid, slotstatus);
			}
			else
			{
				eventlog(eventlog_level_error, __FUNCTION__, "ChangeSlot {} / {} 5", colorid, slotstatus);

				for (auto& s : SuperGameStructList)
				{
					if (ToLower(s.HostName) == hostnamelow)
					{
						for (int i = 0; i < 24; i++)
						{
							if (s.Colours[i] == colorid)
							{
								if (ToLower(s.Slots[i]) != hostnamelow)
								{
									std::string result = "";
									if (slotstatus == 1)
									{
										result = "?";
									}
									else if (slotstatus == 2)
									{
										result = "#";
									}
									if (!s.Slots[i].empty() && s.Slots[i] != "?" && s.Slots[i] != "#")
									{
										DestroyTheGameForPlayer(s.Slots[i]);
									}
									s.Slots[i] = result;
								}

								SendGameToAllPlayers(s);
								break;
							}
						}

						eventlog(eventlog_level_error, __FUNCTION__, "ChangeSlot OK");

						return;
					}
				}
			}
			eventlog(eventlog_level_error, __FUNCTION__, "ChangeSlot FAIL");
			conn_send_ah_gamemessagebox(host, 1, "ANAL ERROR");
		}

		void SendLobbyChat(t_connection* c, std::string message)
		{
			if (!c)
				return;

			if (!c->protocol.account)
				return;
			std::string hostname = account_get_name(c->protocol.account);
			std::string hostnamelow = ToLower(hostname);

			for (const auto& s : SuperGameStructList)
			{
				for (int i = 0; i < 24; i++)
				{
					if (!s.Slots[i].empty())
					{
						if (ToLower(s.Slots[i]) == hostnamelow)
						{
							SendLobbyChatToAllPlayers(c, s, hostname, message);
							return;
						}
					}
				}
			}

			conn_send_ah_gamemessagebox(c, 1, "No hosted game found");
		}

		void JoinToGame(t_connection* c, std::string GameName)
		{
			if (!c)
				return;

			if (!c->protocol.account)
				return;

			LeaveFromGame(c);
			if (c->protocol.game != NULL)
				conn_set_game(c, NULL, NULL, NULL, game_type_none, 0);

			eventlog(eventlog_level_error, __FUNCTION__, "JoinToGame");

			std::string username = account_get_name(c->protocol.account);

			for (auto& s : SuperGameStructList)
			{
				if (ToLower(s.GameName) == ToLower(GameName))
				{

					eventlog(eventlog_level_error, __FUNCTION__, "JoinToGame OK");

					if (s.Players >= s.MaxPlayers)
					{
						conn_send_ah_gamemessagebox(c, 1, "Game full!");
						DestroyTheGameForPlayer(username);
						return;
					}
					bool Join = false;

					for (int i = 0; i < 24; i++)
					{
						if (s.Slots[i].empty())
						{
							s.Slots[i] = username;
							s.Players++;
							Join = true;
							break;
						}
					}

					if (Join)
					{
						SendGameToAllPlayers(s);
					}
					else
					{
						conn_send_ah_gamemessagebox(c, 1, "Game full or slot not available!");
					}

					return;
				}
			}

			// not found game!
		}

		std::string MakeGameName(unsigned int num)
		{
			std::ostringstream ss;
			ss << std::setw(10) << std::setfill('0') << num;
			std::string result = ss.str();
			if (result.length() > 10)
			{
				result.erase(0, result.length() - 10);
			}
			return "GAMEID" + result;
		}


		void StartAlternativeGame(t_connection* host)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "StartAlternativeGame");


			if (!host || !host->protocol.account)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "NO HOST");
				return;
			}

			std::string hostnamelow = ToLower(account_get_name(host->protocol.account));

			for (auto& s : SuperGameStructList)
			{
				if (ToLower(s.HostName) == hostnamelow)
				{
					// Create game and delete from list

					// host strvar=dota strvar2=5x5 strvar3=aptb strvar4 = name
					handle_command(host, ("/chost " + s.MapCode + " " + MakeGameName(s.InternalGameId)).c_str());
					eventlog(eventlog_level_error, __FUNCTION__, "StartAlternativeGame CMD: [{}:{}] ", s.MapCode, s.InternalGameId);

					/*	for ( auto & slot : s.Slots )
						{
							if ( !slot.empty( ) && slot != "?" && slot != "#" )
							{
								SendGameStartForPlayer( slot, s.InternalGameId );
							}
						}
	*/
					return;
				}
			}

			conn_send_ah_gamemessagebox(host, 1, "No created games!");
		}



		void InitializeAlternativeGameData(t_connection* host)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "InitializeAlternativeGameData preparing");
			if (!host)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "InitializeAlternativeGameData fail 1");

				return;
			}

			if (!host->protocol.account)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "InitializeAlternativeGameData fail 2");

				return;
			}
			std::string hostnamelow = ToLower(account_get_name(host->protocol.account));

			for (auto& s : SuperGameStructList)
			{
				if (ToLower(s.HostName) == hostnamelow)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "InitializeAlternativeGameData Started: {} : {} ", hostnamelow, s.InternalGameId);

					std::string SlotIds = "";

					for (int i = 0; i < 12; i++)
					{
						if (!s.Slots[i].empty() && s.Slots[i] != "?" && s.Slots[i] != "#")
						{
							SlotIds += " " + s.Slots[i];
						}
					}

					if (!SlotIds.empty())
						handle_command(host, ("/hold" + SlotIds).c_str());
					SlotIds = "";


					for (int i = 12; i < 24; i++)
					{
						if (!s.Slots[i].empty() && s.Slots[i] != "?" && s.Slots[i] != "#")
						{
							SlotIds += " " + s.Slots[i];
						}
					}

					if (!SlotIds.empty())
						handle_command(host, ("/hold" + SlotIds).c_str());
					SlotIds = "";


					for (int i = 0; i < 24; i++)
					{
						if (!s.Slots[i].empty() && s.Slots[i] == "#")
						{
							SlotIds += " " + std::to_string(i + 1) + " 2";
						}
					}
					if (!SlotIds.empty())
						handle_command(host, ("/comp" + SlotIds).c_str());

					SlotIds = "";

					for (int i = 0; i < 24; i++)
					{
						if (!s.Slots[i].empty() && s.Slots[i] != "?" && s.Slots[i] != "#")
						{
							if (s.Colours[i] >= 0)
							{
								SlotIds += " " + std::to_string(s.Colours[i]);
							}
							else
								SlotIds += " " + std::to_string(i + 201);
						}

					}


					if (!SlotIds.empty())
					{
						handle_command(host, ("/holdslots" + SlotIds).c_str());



						for (int i = 0; i < 24; i++)
						{
							if (s.Races[i] > -1)
							{
								SlotIds += " " + std::to_string(s.Colours[i]) + " " + std::to_string(s.Races[i]);
							}
						}

						if (!SlotIds.empty())
							handle_command(host, ("/corace" + SlotIds).c_str());

						SlotIds = "";


					}
					else
					{
						handle_command(host, "/unhost");
						conn_send_ah_gamemessagebox(host, 1, "Error no players!!!");
						eventlog(eventlog_level_error, __FUNCTION__, "InitializeAlternativeGameData fail 55");

						DestroyTheGameForAll(s);
						DestroyTheGame(host);

						eventlog(eventlog_level_error, __FUNCTION__, "InitializeAlternativeGameData error!");

						return;
					}


					for (auto& slot : s.Slots)
					{
						if (!slot.empty() && slot != "?" && slot != "#")
						{
							SendGameStartForPlayer(slot, s.InternalGameId);
						}
					}


					DestroyTheGameForAll(s);
					DestroyTheGame(host);

					eventlog(eventlog_level_error, __FUNCTION__, "InitializeAlternativeGameData ok!");

					return;
				}

			}
			eventlog(eventlog_level_error, __FUNCTION__, "InitializeAlternativeGameData fail 3");

			conn_send_ah_gamemessagebox(host, 1, "No created games!");
		}

		/*
			Create game Name & MapCode & slotstate

			Join to game [check avaiabled slot]

			Leave from game (clear slot)

			Join to slot (if avaiabled)

			Start game
		*/

		extern int conn_send_gamelistitem(t_connection* c, int gameid)
		{

			t_packet* rpacket;


			if (!(rpacket = packet_create(packet_class_bnet)))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "SEND GAME ERR3");
				return -1;
			}

			packet_set_size(rpacket, sizeof(t_server_ah_scan_request));
			packet_set_type(rpacket, SERVER_AH_SCAN_REQUEST);

			bn_int_set(&rpacket->u.server_ah_scan_request.ah_version, ANTIHACK_VERSION);
			bn_int_set(&rpacket->u.server_ah_scan_request.command, 0x80101C);

			bn_int_set(&rpacket->u.server_ah_scan_request.val1, 0);
			bn_int_set(&rpacket->u.server_ah_scan_request.val2, 0);
			bn_int_set(&rpacket->u.server_ah_scan_request.val3, 0);
			bn_int_set(&rpacket->u.server_ah_scan_request.val4, 0);

			packet_append_string(rpacket, SuperGameStructList[gameid].GameName.c_str());
			packet_append_string(rpacket, SuperGameStructList[gameid].MapCode.c_str());
			packet_append_string(rpacket, SuperGameStructList[gameid].HostName.c_str());
			packet_append_string(rpacket, std::to_string(SuperGameStructList[gameid].Players).c_str());
			packet_append_string(rpacket, std::to_string(SuperGameStructList[gameid].MaxPlayers).c_str());
			packet_append_string(rpacket, MakeGameName(SuperGameStructList[gameid].InternalGameId).c_str());

			conn_push_outqueue(c, rpacket);
			packet_del_ref(rpacket);
			return 1;
		}

		extern int conn_send_newgamelist(t_connection* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "SEND GAME ERR0");
				return 0;
			}

			conn_send_ah_packet(c, 0x80101B);

			//eventlog( eventlog_level_error, __FUNCTION__, "-------" );

			t_account* acc = c->protocol.account;

			if (acc)
			{
				if (c->protocol.IamBOT /*|| account_get_auth_botlogin(acc)*/)
				{
					//eventlog( eventlog_level_error, __FUNCTION__, "SEND GAME ERR1" );
					return 0;
				}
			}
			/*	else
			return 0;*/
			
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "SEND GAME ERR2");
				return -1;
			}

			for (int i = 0; i < SuperGameStructList.size(); i++)
			{
				conn_send_gamelistitem(c, i);
			}

			//eventlog( eventlog_level_error, __FUNCTION__, "SEND GAME MAP" );// {} -> {} -> {} -> {}", map->MapName.c_str( ), map->MapCode.c_str( ) , map->MapLocalPath.c_str( ), map->Category.c_str( ) );

			return 0;
		}



		//void SendGameListToConn( )
		//{
		//	//clear gamelist
		//	t_elem const * curr;
		//	t_connection * conn;
		//	LIST_TRAVERSE_CONST( connlist( ), curr )
		//	{
		//		conn = ( t_connection* )elem_get_data( curr );
		//		if ( conn && conn->protocol.IsNewClient)
		//		{
		//			conn_send_newgamelist( conn );
		//		}
		//	}
		//	//send each game
		//}


		void UpdateAlternativeGames()
		{
			std::time_t curtime;
			std::time(&curtime);

			for (unsigned int i = 0; i < SuperGameStructList.size(); i++)
			{
				if (curtime - SuperGameStructList[i].LastUpdate > 5)
				{
					if (SuperGameStructList[i].FirstCheckTimeout)
					{
						SuperGameStructList[i].FirstCheckTimeout = false;
						eventlog(eventlog_level_error, __FUNCTION__, "DestroyTheGame Wait 5 second...");
					}
					else
					{
						eventlog(eventlog_level_error, __FUNCTION__, "DestroyTheGame OK");
						SuperGameStructList.erase(SuperGameStructList.begin() + i);
						DestroyTheGameForAll(SuperGameStructList[i]);
						UpdateAlternativeGames();
					}
				}
			}
		}



		bool NeedReloadMapList = false;

		/* types and data structures used for the connlist array */
		typedef struct {
			t_connection* c;
			t_elist freelist;
		} t_conn_entry;

		t_conn_entry* connarray = NULL;
		t_elist arrayflist;

		static int      totalcount = 0;
		static t_list* conn_head = NULL;
		static t_list* conn_dead = NULL;

		static void conn_send_welcome(t_connection* c);
		static void conn_send_issue(t_connection* c);

		static int connarray_create(void);
		static void connarray_destroy(void);
		static t_connection* connarray_get_conn(unsigned index);
		static unsigned connarray_add_conn(t_connection* c);
		static void connarray_del_conn(unsigned index);

		static void conn_send_welcome(t_connection* c)
		{
			char const* filename;
			std::FILE* fp;

			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return;
			}

			if (c->protocol.cflags & conn_flags_welcomed)
				return;
			if ((conn_get_class(c) == conn_class_irc) || (conn_get_class(c) == conn_class_wol))
			{
				c->protocol.cflags |= conn_flags_welcomed;
				return;
			}
			if (filename = prefs_get_motdfile())
			{
				std::string lang_filename = i18n_filename(filename, conn_get_gamelang_localized(c));
				if (fp = std::fopen(lang_filename.c_str(), "r"))
				{
					message_send_file(c, fp);
					if (std::fclose(fp) < 0)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "could not close MOTD file \"{}\" after reading (std::fopen: {})", lang_filename, std::strerror(errno));
					}
				}
				else
				{
					eventlog(eventlog_level_error, __FUNCTION__, "could not open MOTD file \"{}\" for reading (std::fopen: {})", filename, std::strerror(errno));
				}
			}
			c->protocol.cflags |= conn_flags_welcomed;
		}


		static void conn_send_issue(t_connection* c)
		{
			char const* filename;
			std::FILE* fp;

			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return;
			}

			if ((filename = prefs_get_issuefile()))
				if ((fp = std::fopen(filename, "r")))
				{
					message_send_file(c, fp);
					if (std::fclose(fp) < 0)
						eventlog(eventlog_level_error, __FUNCTION__, "could not close issue file \"{}\" after reading (std::fopen: {})", filename, std::strerror(errno));
				}
				else
					eventlog(eventlog_level_error, __FUNCTION__, "could not open issue file \"{}\" for reading (std::fopen: {})", filename, std::strerror(errno));
			else
				eventlog(eventlog_level_debug, __FUNCTION__, "no issue file");
		}

		// [zap-zero] 20020629
		extern void conn_shutdown(t_connection* c, std::time_t now, t_timer_data foo)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return;
			}

			if (now == (std::time_t)0) /* zero means user logged out before expiration */
			{
				eventlog(eventlog_level_trace, __FUNCTION__, "[{}] connection already closed", conn_get_socket(c));
				return;
			}

			eventlog(eventlog_level_trace, __FUNCTION__, "[{}] closing connection", conn_get_socket(c));

			conn_set_state(c, conn_state_destroy);

		}

		extern void conn_test_latency(t_connection* c, std::time_t now, t_timer_data delta)
		{
			t_packet* packet;

			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return;
			}


			if (now == (std::time_t)0) /* zero means user logged out before expiration */
				return;

			if (conn_get_state(c) == conn_state_destroy)	// [zap-zero] 20020910
				return;					// state_destroy: do nothing


			if ((conn_get_class(c) == conn_class_irc) || (conn_get_class(c) == conn_class_wol)) {
				/* We should start pinging the client after we received the first line ... */
				/* NOTE: RFC2812 only suggests that PINGs are being sent
				 * if no other activity is detected. However it explicitly
				 * allows PINGs to be sent if there is activity on this
				 * connection. In other words we just don't care :)
				 */
				if (conn_get_ircping(c) != 0) {
					eventlog(eventlog_level_warn, __FUNCTION__, "[{}] ping timeout (closing connection)", conn_get_socket(c));
					conn_set_latency(c, 0);
					conn_set_state(c, conn_state_destroy);
				}
				irc_send_ping(c);
			}
			else if (conn_get_class(c) == conn_class_w3route) {
				if (!(packet = packet_create(packet_class_w3route))) {
					eventlog(eventlog_level_error, __FUNCTION__, "[{}] packet_create failed", conn_get_socket(c));
				}
				else {
					packet_set_size(packet, sizeof(t_server_w3route_echoreq));
					packet_set_type(packet, SERVER_W3ROUTE_ECHOREQ);
					bn_int_set(&packet->u.server_w3route_echoreq.ticks, get_ticks());
					conn_push_outqueue(c, packet);
					packet_del_ref(packet);
				}
			}
			else {

				/* FIXME: I think real Battle.net sends these even before login */
				if (!conn_get_game(c))
				{
					if ((packet = packet_create(packet_class_bnet)))
					{
						packet_set_size(packet, sizeof(t_server_echoreq));
						packet_set_type(packet, SERVER_ECHOREQ);
						bn_int_set(&packet->u.server_echoreq.ticks, get_ticks());
						conn_push_outqueue(c, packet);
						packet_del_ref(packet);
					}
					else
					{
						eventlog(eventlog_level_error, __FUNCTION__, "could not create packet");
					}
				}
			}

			if (timerlist_add_timer(c, now + (std::time_t)delta.n, conn_test_latency, delta) < 0)
				eventlog(eventlog_level_error, __FUNCTION__, "could not add timer");
		}


		static void conn_send_nullmsg(t_connection* c, std::time_t now, t_timer_data delta)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return;
			}

			if (now == (std::time_t)0) /* zero means user logged out before expiration */
				return;

			message_send_text(c, message_type_null, c, NULL);

			if (timerlist_add_timer(c, now + (std::time_t)delta.n, conn_send_nullmsg, delta) < 0)
				eventlog(eventlog_level_error, __FUNCTION__, "could not add timer");
		}


		extern char const* conn_class_get_str(t_conn_class cclass)
		{
			switch (cclass)
			{
			case conn_class_init:
				return "init";
			case conn_class_bnet:
				return "bnet";
			case conn_class_file:
				return "file";
			case conn_class_bot:
				return "bot";
			case conn_class_d2cs_bnetd:
				return "d2cs_bnetd";
			case conn_class_telnet:
				return "telnet";
			case conn_class_irc:
				return "irc";
			case conn_class_wol:
				return "wol";
			case conn_class_wserv:
				return "wserv";
			case conn_class_apireg:
				return "apireg";
			case conn_class_wgameres:
				return "wgameres";
			case conn_class_wladder:
				return "wladder";
			case conn_class_none:
				return "none";
			case conn_class_w3route:
				return "w3route";
			default:
				return "UNKNOWN";
			}
		}


		extern char const* conn_state_get_str(t_conn_state state)
		{
			switch (state)
			{
			case conn_state_empty:
				return "empty";
			case conn_state_initial:
				return "initial";
			case conn_state_connected:
				return "connected";
			case conn_state_bot_username:
				return "bot_username";
			case conn_state_bot_password:
				return "bot_password";
			case conn_state_loggedin:
				return "loggedin";
			case conn_state_destroy:
				return "destroy";
			case conn_state_untrusted:
				return "untrusted";
			case conn_state_pending_raw:
				return "pending_raw";
			default:
				return "UNKNOWN";
			}
		}

		extern int conn_set_realm_cb(void* data, void* newref);

		extern t_connection* conn_create(int tsock, int usock, unsigned int real_local_addr, unsigned short real_local_port, unsigned int local_addr, unsigned short local_port, unsigned int addr, unsigned short port)
		{
			t_connection* temp;


			if (tsock < 0)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad TCP socket {}", tsock);
				return NULL;
			}
			if (usock < -1) /* -1 is allowed for some connection classes like bot, irc, and telnet */
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad UDP socket {}", usock);
				return NULL;
			}

			temp = (t_connection*)xmalloc(sizeof(t_connection));
			temp->socket.tcp_sock = tsock;
			temp->socket.tcp_addr = addr;
			temp->socket.tcp_port = port;
			temp->socket.udp_sock = usock;
			temp->socket.udp_addr = addr; /* same for now but client can request it to be different */
			temp->socket.local_addr = local_addr;
			temp->socket.local_port = local_port;
			temp->socket.real_local_addr = real_local_addr;
			temp->socket.real_local_port = real_local_port;
			temp->socket.udp_port = port;
			temp->socket.fdw_idx = -1;
			temp->protocol.cclass = conn_class_init;
			temp->protocol.state = conn_state_initial;
			temp->protocol.sessionkey = ((unsigned int)std::rand()) ^ ((unsigned int)now + (unsigned int)real_local_port);
			temp->protocol.sessionnum = connarray_add_conn(temp);
			temp->protocol.secret = ((unsigned int)std::rand()) ^ (totalcount + ((unsigned int)now));
			temp->protocol.flags = MF_PLUG;
			temp->protocol.latency = 0;
			temp->protocol.chat.dnd = NULL;
			temp->protocol.chat.away = NULL;
			temp->protocol.chat.ignore_list = NULL;
			temp->protocol.chat.ignore_count = 0;
			temp->protocol.chat.quota.totcount = 0;
			temp->protocol.chat.quota.list = list_create();
			temp->protocol.client.versionid = 0;
			temp->protocol.client.gameversion = 0;
			temp->protocol.client.checksum = 0;
			temp->protocol.client.archtag = 0;
			temp->protocol.client.clienttag = 0;
			temp->protocol.client.clientver = NULL;
			temp->protocol.client.gamelang = 0;
			temp->protocol.client.country = NULL;
			temp->protocol.client.tzbias = 0;
			temp->protocol.client.host = NULL;
			temp->protocol.client.user = NULL;
			temp->protocol.client.clientexe = NULL;
			temp->protocol.client.owner = NULL;
			temp->protocol.client.cdkey = NULL;
			temp->protocol.account = NULL;
			temp->protocol.chat.channel = NULL;
			temp->protocol.chat.last_message = now;
			temp->protocol.chat.lastsender = NULL;
			temp->protocol.chat.irc.ircline = NULL;
			temp->protocol.chat.irc.ircping = 0;
			temp->protocol.chat.irc.ircpass = NULL;
			temp->protocol.chat.tmpOP_channel = NULL;
			temp->protocol.chat.tmpVOICE_channel = NULL;
			temp->protocol.game = NULL;
			temp->protocol.queues.outqueue = NULL;
			temp->protocol.queues.outsize = 0;
			temp->protocol.queues.outsizep = 0;
			temp->protocol.queues.inqueue = NULL;
			temp->protocol.queues.insize = 0;
			temp->protocol.loggeduser = NULL;
			temp->protocol.d2.realm = NULL;
			rcm_regref_init(&temp->protocol.d2.realm_regref, &conn_set_realm_cb, temp);
			temp->protocol.d2.character = NULL;
			temp->protocol.d2.realminfo = NULL;
			temp->protocol.d2.charname = NULL;
			temp->protocol.w3.routeconn = NULL;
			temp->protocol.w3.anongame = NULL;
			temp->protocol.w3.anongame_search_starttime = 0;
			temp->protocol.w3.client_proof = NULL;
			temp->protocol.w3.server_proof = NULL;
			temp->protocol.bound = NULL;
			elist_init(&temp->protocol.timers);

			temp->protocol.wol.ingame = 0;

			temp->protocol.wol.codepage = 0;
			temp->protocol.wol.pageme = true;
			temp->protocol.wol.findme = true;

			temp->protocol.wol.apgar = NULL;
			temp->protocol.wol.anongame_player = NULL;


			temp->protocol.cr_time = now;
			temp->protocol.passfail_count = 0;


			temp->protocol.cflags = 0;

			//here
			temp->protocol.ah_status = 0;
			temp->protocol.ah_version = 0;
			temp->protocol.found_cheat = 0;
			temp->protocol.found_cheat2 = 0;
			temp->protocol.lastahtime = 0;
			temp->protocol.lastahtime2 = 0;
			temp->protocol.lastahtime3 = 0;
			temp->protocol.lastahtime4 = 0;
			temp->protocol.lastbadgametime = 0;
			memset(temp->protocol.hardwareid, 0, 4 * 4);

			temp->protocol.ah_magic_value = rand() % 22767;
			temp->protocol.all_infos_okay = false;
			temp->protocol.IsNewClient = false;
			temp->protocol.followaccount = NULL;
			temp->protocol.botconnection = NULL;
			temp->protocol.IamBOT = false;
			temp->protocol.FoundWhiteMep = false;

			temp->protocol.hostedgame = NULL;


			temp->protocol.bot_locate = 0;


			temp->protocol.availabledBot = true;
			temp->protocol.HostTimeOut = 0;
			temp->protocol.HostStatsType = new char[256];
			temp->protocol.ForStats = false;
			temp->protocol.LobbyNicknameColor = 0xFFFFFFFF;
			temp->protocol.ChatNicknameColor = 0xFFFFFFFF;
			temp->protocol.ChatTextColor = 0xFFFFFFFF;

			list_prepend_data(conn_head, temp);

			memset(temp->protocol.hash, 0, 256);
			memset(temp->protocol.plainpassword, 0, 256);
			eventlog(eventlog_level_info, __FUNCTION__, "[{}][{}] sessionkey=0x{:08} sessionnum=0x{:08}", temp->socket.tcp_sock, temp->socket.udp_sock, temp->protocol.sessionkey, temp->protocol.sessionnum);

			return temp;
		}


		extern t_anongame* conn_create_anongame(t_connection* c)
		{
			t_anongame* temp;
			int i;

			if (c->protocol.w3.anongame) {
				eventlog(eventlog_level_error, __FUNCTION__, "anongame already allocated");
				return c->protocol.w3.anongame;
			}

			temp = (t_anongame*)xmalloc(sizeof(t_anongame));
			temp->count = 0;
			temp->id = 0;
			temp->tid = 0;

			for (i = 0; i < ANONGAME_MAX_GAMECOUNT / 2; i++)
				temp->tc[i] = NULL;

			temp->race = 0;
			temp->playernum = 0;
			temp->handle = 0;
			temp->addr = 0;
			temp->loaded = 0;
			temp->joined = 0;
			temp->map_prefs = 0xffffffff;
			temp->type = 0;
			temp->gametype = 0;
			temp->queue = 0;
			temp->info = NULL;

			c->protocol.w3.anongame = temp;

			return temp;
		}

		extern t_anongame* conn_get_anongame(t_connection* c)
		{
			if (!c) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return NULL;
			}
			return c->protocol.w3.anongame;
		}

		extern void conn_destroy_anongame(t_connection* c)
		{
			t_anongame* a;

			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return;
			}

			if (!(a = c->protocol.w3.anongame))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "NULL anongame");
				return;
			}

			// delete reference to this connection
			if (a->info) {
				a->info->player[a->playernum - 1] = NULL;
				if (--(a->info->currentplayers) == 0)
					anongameinfo_destroy(a->info);
			}

			// [quetzal] 20020824
			// unqueue from anongame search list,
			// if we got AT game, unqueue entire team.
			if (anongame_arranged(a->queue)) {
				anongame_unqueue(a->tc[0], a->queue);
			}
			else {
				anongame_unqueue(c, a->queue);
			}
			xfree(c->protocol.w3.anongame);
			c->protocol.w3.anongame = NULL;
		}

		extern void conn_destroy(t_connection* c, t_elem** elem, int conn_or_dead_list)
		{
			char const* classstr;
			t_elem* curr;


			if (c == NULL) {
				eventlog(eventlog_level_error, "conn_destroy", "got NULL connection");
				return;
			}


#ifdef			WITH_LUA
if (c->protocol.account)
{
lua_handle_user(c, NULL, NULL,luaevent_user_disconnect);
}
#endif


			classstr = conn_class_get_str(c->protocol.cclass);

			//here
			RemoveBotConnectionFromConnections(c);
			if (list_remove_data(conn_head, c, (conn_or_dead_list) ? &curr : elem) < 0)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could not remove item from list");
				return;
			}

			// «авершить игру если хост выходит с сервера
			if (c->protocol.botconnection && c->protocol.hostedgame &&
				(game_get_status(c->protocol.hostedgame) == game_status_open ||
					game_get_status(c->protocol.hostedgame) == game_status_full
					))
			{
				eventlog(eventlog_level_debug, __FUNCTION__, "!send unhost command");
				message_send_text(c->protocol.botconnection, message_type_whisper, c->protocol.botconnection, "!unhost");
			}


			if (c->protocol.cclass == conn_class_d2cs_bnetd)
			{
				t_realm* realm;

				realm = conn_get_realm(c);
				if (realm)
					realm_deactive(realm);
				else
				{
					eventlog(eventlog_level_error, __FUNCTION__, "could not find realm for d2cs connection");
				}
			}
			else if (c->protocol.cclass == conn_class_w3route && c->protocol.w3.routeconn && c->protocol.w3.routeconn->protocol.w3.anongame)
			{
				anongame_stats(c);
				conn_destroy_anongame(c->protocol.w3.routeconn);  // [zap-zero] destroy anongame too when game connection is invalid
			}

			eventlog(eventlog_level_error, __FUNCTION__, "2");
			if (c->protocol.d2.realm) {
				realm_add_player_number(c->protocol.d2.realm, -1);
				realm_put(c->protocol.d2.realm, &c->protocol.d2.realm_regref);
			}


			/* free the memory with user quota */
			{
				t_qline* qline;

				LIST_TRAVERSE(c->protocol.chat.quota.list, curr)
				{
					qline = (t_qline*)elem_get_data(curr);
					xfree(qline);
					list_remove_elem(c->protocol.chat.quota.list, &curr);
				}
				list_destroy(c->protocol.chat.quota.list);
			}

			/* if this user in a channel, notify everyone that the user has left */
			if (c->protocol.chat.channel)
				channel_del_connection(c->protocol.chat.channel, c, message_type_quit, NULL);

			if ((c->protocol.game) && (c->protocol.account))
			{
				if (game_get_status(c->protocol.game) == game_status_started)
				{
					game_set_self_report(c->protocol.game, c->protocol.account, game_result_disconnect);
					game_set_report(c->protocol.game, c->protocol.account, "disconnect", "disconnect");
				}
			}

			conn_set_game(c, NULL, NULL, NULL, game_type_none, 0);
			c->protocol.state = conn_state_empty;

			watchlist->del(c);
			timerlist_del_all_timers(c);

			clanmember_set_offline(c);

			if (c->protocol.account)
				watchlist->dispatch(c->protocol.account, NULL, c->protocol.client.clienttag, Watch::ET_logout);

			if (c->protocol.chat.lastsender)
				xfree((void*)c->protocol.chat.lastsender); /* avoid warning */

			if (c->protocol.chat.away)
				xfree((void*)c->protocol.chat.away); /* avoid warning */
			if (c->protocol.chat.dnd)
				xfree((void*)c->protocol.chat.dnd); /* avoid warning */
			if (c->protocol.chat.tmpOP_channel)
				xfree((void*)c->protocol.chat.tmpOP_channel); /* avoid warning */
			if (c->protocol.chat.tmpVOICE_channel)
				xfree((void*)c->protocol.chat.tmpVOICE_channel); /* avoid warning */

			if (c->protocol.client.clientver)
				xfree((void*)c->protocol.client.clientver); /* avoid warning */
			if (c->protocol.client.country)
				xfree((void*)c->protocol.client.country); /* avoid warning */
			if (c->protocol.client.host)
				xfree((void*)c->protocol.client.host); /* avoid warning */
			if (c->protocol.client.user)
				xfree((void*)c->protocol.client.user); /* avoid warning */
			if (c->protocol.client.clientexe)
				xfree((void*)c->protocol.client.clientexe); /* avoid warning */
			if (c->protocol.client.owner)
				xfree((void*)c->protocol.client.owner); /* avoid warning */
			if (c->protocol.client.cdkey)
				xfree((void*)c->protocol.client.cdkey); /* avoid warning */
			if (c->protocol.d2.realminfo)
				xfree((void*)c->protocol.d2.realminfo); /* avoid warning */
			if (c->protocol.d2.charname)
				xfree((void*)c->protocol.d2.charname); /* avoid warning */
			if (c->protocol.chat.irc.ircline)
				xfree((void*)c->protocol.chat.irc.ircline); /* avoid warning */
			if (c->protocol.chat.irc.ircpass)
				xfree((void*)c->protocol.chat.irc.ircpass); /* avoid warning */

			if (c->protocol.wol.apgar)
				xfree((void*)c->protocol.wol.apgar); /* avoid warning */

			if (c->protocol.wol.anongame_player)
				anongame_wol_destroy(c);
 
			if (c->protocol.w3.client_proof)
				xfree((void*)c->protocol.w3.client_proof); /* avoid warning */

			if (c->protocol.w3.server_proof)
				xfree((void*)c->protocol.w3.server_proof); /* avoid warning */

			if (c->protocol.bound)
				c->protocol.bound->protocol.bound = NULL;

			if (c->protocol.chat.ignore_count > 0)
			{
				if (!c->protocol.chat.ignore_list)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "found NULL ignore_list with ignore_count={}", c->protocol.chat.ignore_count);
				}
				else
				{
					xfree(c->protocol.chat.ignore_list);
				}
			}

			if (c->protocol.account)
			{
				eventlog(eventlog_level_info, __FUNCTION__, "[{}] \"{}\" logged out", c->socket.tcp_sock, conn_get_loggeduser(c));
				//amadeo
#ifdef WIN32_GUI
				guiOnUpdateUserList();
#endif
				if (prefs_get_sync_on_logoff()) {
					if (account_save(conn_get_account(c), FS_FORCE) < 0)
						eventlog(eventlog_level_error, __FUNCTION__, "cannot sync account (sync_on_logoff)");
				}

				if (account_get_conn(c->protocol.account) == c)  /* make sure you don't set this when already on new conn (relogin with same account) */
					account_set_conn(c->protocol.account, NULL);
				c->protocol.account = NULL; /* the account code will free the memory later */
			}


			/* logged user is no longer only for logged in users */
			if (c->protocol.loggeduser) xfree((void*)c->protocol.loggeduser);

			/* make sure the connection is closed */
			if (c->socket.tcp_sock != -1) { /* -1 means that the socket was already closed by conn_close() */
				fdwatch_del_fd(c->socket.fdw_idx);
				psock_shutdown(c->socket.tcp_sock, PSOCK_SHUT_RDWR);
				psock_close(c->socket.tcp_sock);
			}
			/* clear out the packet queues */
			if (c->protocol.queues.inqueue) packet_del_ref(c->protocol.queues.inqueue);
			queue_clear(&c->protocol.queues.outqueue);

			// [zap-zero] 20020601
			if (c->protocol.w3.routeconn) {
				c->protocol.w3.routeconn->protocol.w3.routeconn = NULL;
				if (c->protocol.w3.routeconn->protocol.cclass == conn_class_w3route)
					conn_set_state(c->protocol.w3.routeconn, conn_state_destroy);
			}

			if (c->protocol.w3.anongame)
				conn_destroy_anongame(c);

			c->protocol.followaccount = NULL;
			delete[] c->protocol.HostStatsType;
			c->protocol.HostStatsType = NULL;
			c->protocol.ForStats = false;
			memset(c->protocol.hash, 0, 256);
			memset(c->protocol.plainpassword, 0, 256);

			/* delete the conn from the dead list if its there, we dont check for error
			 * because connections may be destroyed without first setting state to destroy */
			if (conn_dead) list_remove_data(conn_dead, c, (conn_or_dead_list) ? elem : &curr);
			connarray_del_conn(c->protocol.sessionnum);

			eventlog(eventlog_level_info, __FUNCTION__, "[{}] closed {} connection", c->socket.tcp_sock, classstr);

			xfree(c);
		}


		extern int conn_match(t_connection const* c, char const* username)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}
			if (!username)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL username");
				return -1;
			}

			if (!c->protocol.account)
				return 0;

			return account_match(c->protocol.account, username);
		}


		extern t_conn_class conn_get_class(t_connection const* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return conn_class_none;
			}

			return c->protocol.cclass;
		}


		extern void conn_set_class(t_connection* c, t_conn_class cclass)
		{
			t_timer_data  data;
			unsigned long delta;
			t_conn_class oldclass;

			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return;
			}

			if (c->protocol.cclass == cclass)
				return;

			oldclass = c->protocol.cclass;
			c->protocol.cclass = cclass;

			switch (cclass) {
			case conn_class_bnet:
				if (prefs_get_udptest_port() != 0)
					conn_set_game_port(c, (unsigned short)prefs_get_udptest_port());
				udptest_send(c);

				/* remove any init timers */
				if (oldclass == conn_class_init) timerlist_del_all_timers(c);
				delta = prefs_get_latency();
				data.n = delta;
				if (timerlist_add_timer(c, now + (std::time_t)delta, conn_test_latency, data) < 0)
					eventlog(eventlog_level_error, __FUNCTION__, "could not add timer");

				eventlog(eventlog_level_debug, __FUNCTION__, "added latency check timer");
				break;

			case conn_class_w3route:
				delta = prefs_get_latency();
				data.n = delta;
				if (timerlist_add_timer(c, now + (std::time_t)delta, conn_test_latency, data) < 0)
					eventlog(eventlog_level_error, __FUNCTION__, "could not add timer");
				break;

			case conn_class_bot:
			case conn_class_telnet:
			{
				t_packet* rpacket;
				if (cclass == conn_class_bot) {
					if ((delta = prefs_get_nullmsg()) > 0) {
						data.n = delta;
						if (timerlist_add_timer(c, now + (std::time_t)delta, conn_send_nullmsg, data) < 0)
							eventlog(eventlog_level_error, __FUNCTION__, "could not add timer");
					}
				}

				/* remove any init timers */
				if (oldclass == conn_class_init) timerlist_del_all_timers(c);
				conn_send_issue(c);

				if (!(rpacket = packet_create(packet_class_raw)))
					eventlog(eventlog_level_error, __FUNCTION__, "could not create rpacket");
				else {
					packet_append_ntstring(rpacket, "Username: ");
					conn_push_outqueue(c, rpacket);
					packet_del_ref(rpacket);
				}

				break;
			}

			default:
				/* remove any init timers */
				if (oldclass == conn_class_init)
					timerlist_del_all_timers(c);
				break;
			}

		}


		extern t_conn_state conn_get_state(t_connection const* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return conn_state_empty;
			}

			return c->protocol.state;
		}


		extern void conn_set_state(t_connection* c, t_conn_state state)
		{
			t_elem* elem;

			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return;
			}

			/* special case for destroying connections, add them to conn_dead list */
			if (state == conn_state_destroy && c->protocol.state != conn_state_destroy) {
				if (!conn_dead)
					conn_dead = list_create();
				list_append_data(conn_dead, c);
			}
			else if (state != conn_state_destroy && c->protocol.state == conn_state_destroy)
				if (list_remove_data(conn_dead, c, &elem)) {
					eventlog(eventlog_level_error, __FUNCTION__, "could not remove dead connection");
					return;
				}

			c->protocol.state = state;
		}

		extern unsigned int conn_get_sessionkey(t_connection const* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return 0;
			}

			return c->protocol.sessionkey;
		}


		extern unsigned int conn_get_sessionnum(t_connection const* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return 0;
			}

			return c->protocol.sessionnum;
		}


		extern unsigned int conn_get_secret(t_connection const* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return 0;
			}

			return c->protocol.secret;
		}


		extern unsigned int conn_get_addr(t_connection const* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return 0;
			}

			return c->socket.tcp_addr;
		}


		extern unsigned short conn_get_port(t_connection const* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return 0;
			}

			return c->socket.tcp_port;
		}


		extern unsigned int conn_get_local_addr(t_connection const* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return 0;
			}

			return c->socket.local_addr;
		}


		extern unsigned short conn_get_local_port(t_connection const* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return 0;
			}

			return c->socket.local_port;
		}


		extern unsigned int conn_get_real_local_addr(t_connection const* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return 0;
			}

			return c->socket.real_local_addr;
		}


		extern unsigned short conn_get_real_local_port(t_connection const* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return 0;
			}

			return c->socket.real_local_port;
		}


		extern unsigned int conn_get_game_addr(t_connection const* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return 0;
			}

			return c->socket.udp_addr;
		}


		extern int conn_set_game_addr(t_connection* c, unsigned int game_addr)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}

			c->socket.udp_addr = game_addr;
			return 0;
		}


		extern unsigned short conn_get_game_port(t_connection const* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return 0;
			}

			return c->socket.udp_port;
		}


		extern int conn_set_game_port(t_connection* c, unsigned short game_port)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}

			c->socket.udp_port = game_port;
			return 0;
		}


		extern void conn_set_host(t_connection* c, char const* host)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return;
			}
			if (!host)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL host");
				return;
			}

			if (c->protocol.client.host)
				xfree((void*)c->protocol.client.host); /* avoid warning */
			c->protocol.client.host = xstrdup(host);
		}


		extern void conn_set_user(t_connection* c, char const* user)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return;
			}
			if (!user)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL user");
				return;
			}

			if (c->protocol.client.user)
				xfree((void*)c->protocol.client.user); /* avoid warning */
			c->protocol.client.user = xstrdup(user);
		}


		extern void conn_set_owner(t_connection* c, char const* owner)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return;
			}
			if (!owner)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL owner");
				return;
			}

			if (c->protocol.client.owner)
				xfree((void*)c->protocol.client.owner); /* avoid warning */
			c->protocol.client.owner = xstrdup(owner);
		}

		extern const char* conn_get_user(t_connection const* c)
		{
			if (!c) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return NULL;
			}
			return c->protocol.client.user;
		}

		extern const char* conn_get_owner(t_connection const* c)
		{
			if (!c) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return NULL;
			}
			return c->protocol.client.owner;
		}

		extern void conn_set_cdkey(t_connection* c, char const* cdkey)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return;
			}
			if (!cdkey)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL cdkey");
				return;
			}

			if (c->protocol.client.cdkey)
				xfree((void*)c->protocol.client.cdkey); /* avoid warning */
			c->protocol.client.cdkey = xstrdup(cdkey);
		}


		extern char const* conn_get_clientexe(t_connection const* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return NULL;
			}

			if (!c->protocol.client.clientexe)
				return "";
			return c->protocol.client.clientexe;
		}


		extern void conn_set_clientexe(t_connection* c, char const* clientexe)
		{
			char const* temp;

			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return;
			}
			if (!clientexe)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL clientexe");
				return;
			}

			temp = xstrdup(clientexe);
			if (c->protocol.client.clientexe)
				xfree((void*)c->protocol.client.clientexe); /* avoid warning */
			c->protocol.client.clientexe = temp;
		}


		extern char const* conn_get_clientver(t_connection const* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return NULL;
			}

			if (!c->protocol.client.clientver)
				return "";
			return c->protocol.client.clientver;
		}


		extern void conn_set_clientver(t_connection* c, char const* clientver)
		{
			char const* temp;

			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return;
			}
			if (!clientver)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL clientver");
				return;
			}

			temp = xstrdup(clientver);
			if (c->protocol.client.clientver)
				xfree((void*)c->protocol.client.clientver); /* avoid warning */
			c->protocol.client.clientver = temp;
		}


		extern t_tag conn_get_archtag(t_connection const* c)
		{
			if (!c) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return 0; /* unknown */
			}

			return c->protocol.client.archtag;
		}


		extern void conn_set_archtag(t_connection* c, t_tag archtag)
		{
			if (!c) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return;
			}
			if (!tag_check_arch(archtag)) {
				eventlog(eventlog_level_error, __FUNCTION__, "got UNKNOWN archtag");
				return;
			}
			if (c->protocol.client.archtag != archtag)
			c->protocol.client.archtag = archtag;
		}


		extern t_tag conn_get_gamelang(t_connection const* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return 0;
			}

			return c->protocol.client.gamelang;
		}


		extern void conn_set_gamelang(t_connection* c, t_tag gamelang)
		{
			char gamelang_str[5];

			if (!c) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return;
			}
			if (!gamelang)
				return; /* only war3 & w3xp have gamelang */

			bool found;
			language_find_by_tag(gamelang, found);
			if (!found)
			{
				eventlog(eventlog_level_warn, __FUNCTION__, "got UNKNOWN gamelang");
				return;
			}
			if (c->protocol.client.gamelang != gamelang)
				eventlog(eventlog_level_info, __FUNCTION__, "[{}] setting client gamelang to \"{}\"", conn_get_socket(c), tag_uint_to_str(gamelang_str, gamelang));

			c->protocol.client.gamelang = gamelang;
		}


		extern t_clienttag conn_get_clienttag(t_connection const* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return CLIENTTAG_UNKNOWN_UINT;
			}

			if (!c->protocol.client.clienttag)
				return CLIENTTAG_UNKNOWN_UINT;
			return c->protocol.client.clienttag;
		}


		extern t_clienttag conn_get_fake_clienttag(t_connection const* c)
		{
			char const* clienttag;
			t_account* account;

			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return 0; /* unknown */
			}

			account = conn_get_account(c);
			if (account) /* BITS remote connections don't need to have an account */
				if ((clienttag = account_get_strattr(account, "BNET\\fakeclienttag")))
					return tag_str_to_uint(clienttag);
			return c->protocol.client.clienttag;
		}


		extern void conn_set_clienttag(t_connection* c, t_clienttag clienttag)
		{
			char clienttag_str[5];

			if (!c) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return;
			}
			if (!tag_check_client(clienttag)) {
				eventlog(eventlog_level_error, __FUNCTION__, "got UNKNOWN clienttag \"{}\"", tag_uint_to_str(clienttag_str, clienttag));
				return;
			}
			if (c->protocol.client.clienttag != clienttag)
			{
				c->protocol.client.clienttag = clienttag;
				if (c->protocol.chat.channel)
					channel_update_userflags(c);
			}

		}


		extern unsigned long conn_get_gameversion(t_connection const* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return 0;
			}

			return c->protocol.client.gameversion;
		}


		extern int conn_set_gameversion(t_connection* c, unsigned long gameversion)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}

			c->protocol.client.gameversion = gameversion;
			return 0;
		}


		extern unsigned long conn_get_checksum(t_connection const* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return 0;
			}

			return c->protocol.client.checksum;
		}


		extern int conn_set_checksum(t_connection* c, unsigned long checksum)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}

			c->protocol.client.checksum = checksum;
			return 0;
		}


		extern unsigned long conn_get_versionid(t_connection const* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return 0;
			}

			return c->protocol.client.versionid;
		}


		extern int conn_set_versionid(t_connection* c, unsigned long versionid)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}

			c->protocol.client.versionid = versionid;
			return 0;
		}


		extern int conn_get_tzbias(t_connection const* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return 0;
			}

			return c->protocol.client.tzbias;
		}


		extern void conn_set_tzbias(t_connection* c, int tzbias)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return;
			}

			c->protocol.client.tzbias = tzbias;
		}


		static void conn_set_account(t_connection* c, t_account* account)
		{
			t_connection* other;
			char const* tname;

			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return;
			}
			if (!account)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return;
			}

			eventlog(eventlog_level_debug, __FUNCTION__, "conn_set_account");

			//here
			if (IsHostBot(account_get_name(account)))
			{
				c->protocol.IamBOT = true;
			}

			if (!c->protocol.IamBOT /*&& !account_get_auth_botlogin(account) */ && (other = connlist_find_connection_by_accountname((tname = account_get_name(account)))))
			{
				eventlog(eventlog_level_info, __FUNCTION__, "[{}] forcing logout of previous login for \"{}\"", conn_get_socket(c), tname);
				conn_set_state(other, conn_state_destroy);
			}

			c->protocol.account = account;


			c->protocol.state = conn_state_loggedin;
			account_set_conn(account, c);
			{
				char const* flagstr;

				if ((flagstr = account_get_strattr(account, "BNET\\flags\\initial")))
					conn_add_flags(c, std::strtoul(flagstr, NULL, 0));
			}
			unsigned int bdcolor = account_get_numattr(account, "BNET\\acct\\LobbyNicknameColor");
			if (c->protocol.LobbyNicknameColor == 0xFFFFFFFF && bdcolor != 0)
				c->protocol.LobbyNicknameColor = bdcolor;
			bdcolor = account_get_numattr(account, "BNET\\acct\\ChatNicknameColor");
			if (c->protocol.ChatNicknameColor == 0xFFFFFFFF && bdcolor != 0)
				c->protocol.ChatNicknameColor = bdcolor;
			bdcolor = account_get_numattr(account, "BNET\\acct\\ChatTextColor");
			if (c->protocol.ChatTextColor == 0xFFFFFFFF && bdcolor != 0)
				c->protocol.ChatTextColor = bdcolor;


			account_set_ll_time(c->protocol.account, (unsigned int)now);
			account_set_ll_owner(c->protocol.account, c->protocol.client.owner);
			account_set_ll_clienttag(c->protocol.account, c->protocol.client.clienttag);
			account_set_ll_ip(c->protocol.account, addr_num_to_ip_str(c->socket.tcp_addr));

			if (c->protocol.client.host)
			{
				xfree((void*)c->protocol.client.host); /* avoid warning */
				c->protocol.client.host = NULL;
			}
			if (c->protocol.client.user)
			{
				xfree((void*)c->protocol.client.user); /* avoid warning */
				c->protocol.client.user = NULL;
			}
			if (c->protocol.client.clientexe)
			{
				xfree((void*)c->protocol.client.clientexe); /* avoid warning */
				c->protocol.client.clientexe = NULL;
			}
			if (c->protocol.client.owner)
			{
				xfree((void*)c->protocol.client.owner); /* avoid warning */
				c->protocol.client.owner = NULL;
			}
			if (c->protocol.client.cdkey)
			{
				xfree((void*)c->protocol.client.cdkey); /* avoid warning */
				c->protocol.client.cdkey = NULL;
			}

			clanmember_set_online(c);

			totalcount++;

			watchlist->dispatch(c->protocol.account, NULL, c->protocol.client.clienttag, Watch::ET_login);

			return;
		}


		extern void conn_login(t_connection* c, t_account* a, const char* loggeduser)
		{
			assert(c != NULL);
			assert(a != NULL);
			assert(loggeduser != NULL);

			conn_set_account(c, a);
			if (std::strcmp(conn_get_loggeduser(c), loggeduser))
				conn_set_loggeduser(c, loggeduser);
		}


		extern t_account* conn_get_account(t_connection const* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return NULL;
			}

			return c->protocol.account;
		}


		extern int conn_set_loggeduser(t_connection* c, char const* username)
		{
			const char* temp;

			assert(c != NULL);
			assert(username != NULL);

			if (username[0] != '#')
				temp = xstrdup(username);
			else {
				unsigned int userid = 0;
				str_to_uint(&username[1], &userid);
				if (userid != 0)
				{
					if (prefs_get_account_force_username())
					{
						t_account* account = accountlist_find_account_by_uid(userid);
						temp = xstrdup(account_get_name(account));
					}
					else
						temp = xstrdup(std::string("#" + userid).c_str());
				}
				else
				{  //theoretically this should never happen...
					eventlog(eventlog_level_error, __FUNCTION__, "got invalid numeric uid \"{}\"", username);
					// set value that would have been set prior to this bugfix...
					temp = xstrdup(username);
				}
			}
			if (c->protocol.loggeduser) xfree((void*)c->protocol.loggeduser);

			c->protocol.loggeduser = temp;

			return 0;
		}


		extern char const* conn_get_loggeduser(t_connection const* c)
		{
			assert(c != NULL);

			if (!c->protocol.loggeduser && c->protocol.account)
				return account_get_name(c->protocol.account);
			return c->protocol.loggeduser;
		}


		extern unsigned int conn_get_flags(t_connection const* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return 0;
			}

			return c->protocol.flags;
		}


		extern int conn_set_flags(t_connection* c, unsigned int flags)
		{
			if (!c) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}

			if (flags != c->protocol.flags) {
				c->protocol.flags = flags;
				if (c->protocol.chat.channel) channel_update_userflags(c);
			}

			return 0;
		}


		extern void conn_add_flags(t_connection* c, unsigned int flags)
		{
			unsigned int oldflags;

			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return;
			}
			oldflags = c->protocol.flags;
			c->protocol.flags |= flags;

			if (oldflags != c->protocol.flags && c->protocol.chat.channel)
				channel_update_userflags(c);
		}


		extern void conn_del_flags(t_connection* c, unsigned int flags)
		{
			unsigned int oldflags;

			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return;
			}
			oldflags = c->protocol.flags;
			c->protocol.flags &= ~flags;

			if (oldflags != c->protocol.flags && c->protocol.chat.channel)
				channel_update_userflags(c);
		}


		extern unsigned int conn_get_latency(t_connection const* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return 0;
			}

			return c->protocol.latency;
		}


		extern void conn_set_latency(t_connection* c, unsigned int ms)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return;
			}


			if (c->protocol.latency != ms)
			{
				c->protocol.latency = ms;

				if (c->protocol.chat.channel)
					channel_update_latency(c);
			}
		}


		extern char const* conn_get_awaystr(t_connection const* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return NULL;
			}

			return c->protocol.chat.away;
		}


		extern int conn_set_awaystr(t_connection* c, char const* away)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}

			if (c->protocol.chat.away)
				xfree((void*)c->protocol.chat.away); /* avoid warning */
			if (!away)
				c->protocol.chat.away = NULL;
			else
				c->protocol.chat.away = xstrdup(away);

			return 0;
		}


		extern char const* conn_get_dndstr(t_connection const* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return NULL;
			}

			return c->protocol.chat.dnd;
		}


		extern int conn_set_dndstr(t_connection* c, char const* dnd)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}

			if (c->protocol.chat.dnd)
				xfree((void*)c->protocol.chat.dnd); /* avoid warning */
			if (!dnd)
				c->protocol.chat.dnd = NULL;
			else
				c->protocol.chat.dnd = xstrdup(dnd);

			return 0;
		}


		extern int conn_add_ignore(t_connection* c, t_account* account)
		{
			t_account** newlist;
			t_connection* dest_c;

			if (!c) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}

			if (!account) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return -1;
			}

			newlist = (t_account**)xrealloc(c->protocol.chat.ignore_list, sizeof(t_account const*) * (c->protocol.chat.ignore_count + 1));
			newlist[c->protocol.chat.ignore_count++] = account;
			c->protocol.chat.ignore_list = newlist;

			dest_c = account_get_conn(account);
			if (dest_c) {
				t_message* message;

				message = message_create(message_type_userflags, dest_c, NULL);
				if (!message) return 0;
				message_send(message, c);
				message_destroy(message);
			}

			return 0;
		}


		extern int conn_del_ignore(t_connection* c, t_account const* account)
		{
			t_account** newlist;
			t_account* temp;
			unsigned int  i;

			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}
			if (!account)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return -1;
			}

			for (i = 0; i < c->protocol.chat.ignore_count; i++)
				if (c->protocol.chat.ignore_list[i] == account)
					break;
			if (i == c->protocol.chat.ignore_count)
				return -1; /* not in list */

			/* swap entry to be deleted with last entry */
			temp = c->protocol.chat.ignore_list[c->protocol.chat.ignore_count - 1];
			c->protocol.chat.ignore_list[c->protocol.chat.ignore_count - 1] = c->protocol.chat.ignore_list[i];
			c->protocol.chat.ignore_list[i] = temp;

			if (c->protocol.chat.ignore_count == 1) /* some realloc()s are buggy */
			{
				xfree(c->protocol.chat.ignore_list);
				newlist = NULL;
			}
			else
				newlist = (t_account**)xrealloc(c->protocol.chat.ignore_list, sizeof(t_account const*) * (c->protocol.chat.ignore_count - 1));

			c->protocol.chat.ignore_count--;
			c->protocol.chat.ignore_list = newlist;

			return 0;
		}

		// here
		extern int conn_add_watch(t_connection* c, t_account* account, t_clienttag clienttag, unsigned int events)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}

			watchlist->add(c, account, clienttag, events);
			return 0;
		}


		extern int conn_del_watch(t_connection* c, t_account* account, t_clienttag clienttag)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}

			if (watchlist->del(c, account, clienttag, Watch::ET_login | Watch::ET_logout | Watch::ET_joingame | Watch::ET_leavegame) < 0)
				return -1;
			return 0;
		}

		//here
		extern int conn_add_follow(t_connection* c, t_account* account, t_clienttag clienttag)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}

			watchlist->add(c, account, clienttag, Watch::ET_login | Watch::ET_logout | Watch::ET_joingame | Watch::ET_leavegame | Watch::ET_FOLLOW);
			return 0;
		}

		extern int conn_del_follow(t_connection* c, t_account* account, t_clienttag clienttag)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}

			if (watchlist->del(c, account, clienttag, Watch::ET_login | Watch::ET_logout | Watch::ET_joingame | Watch::ET_leavegame | Watch::ET_FOLLOW) < 0)
				return -1;
			return 0;
		}


		extern int conn_check_ignoring(t_connection const* c, char const* me)
		{
			unsigned int i;
			t_account* temp;

			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}

			if (!me || !(temp = accountlist_find_account(me)))
				return -1;

			if (c->protocol.chat.ignore_list)
				for (i = 0; i < c->protocol.chat.ignore_count; i++)
					if (c->protocol.chat.ignore_list[i] == temp)
						return 1;

			return 0;
		}


		extern t_channel* conn_get_channel(t_connection const* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return NULL;
			}

			return c->protocol.chat.channel;
		}


		extern int conn_set_channel_var(t_connection* c, t_channel* channel)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}
			c->protocol.chat.channel = channel;
			return 0;
		}


		extern int conn_set_channel(t_connection* c, char const* channelname)
		{
			t_channel* channel;
			t_channel* oldchannel;
			t_account* acc;
			t_elem* curr;
			int clantag = 0;
			t_clan* clan = NULL;
			t_clanmember* member = NULL;
			unsigned int created;

			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}


			acc = c->protocol.account;

			if (!acc)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return -1;
			}

			if (!channelname)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL channelname");
				return -1;
			}

			eventlog(eventlog_level_error, __FUNCTION__, " -> {} -> {}", account_get_name(acc), channelname);

			if (conn_get_game(c))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "Game info {} {}",
					game_get_count(conn_get_game(c)), game_get_status(conn_get_game(c)));
			}

			//if ( conn_get_game( c ) != NULL )
			//{
			//	handle_command( c, "/unhost" );
			//	conn_set_game( c, NULL, NULL, NULL, game_type_none, 0 );
			//}

			oldchannel = c->protocol.chat.channel;

			channel = channellist_find_channel_by_name(channelname, conn_get_country(c), realm_get_name(conn_get_realm(c)));

			if (channel && (channel == oldchannel))
				return 0;

			if ((strncasecmp(channelname, "clan ", 5) == 0) && (std::strlen(channelname) < 10))
				clantag = str_to_clantag(&channelname[5]);

			if ((clantag) && !((account_get_auth_admin(acc, channelname) == 1) || (account_get_auth_admin(acc, NULL) == 1))) {
				/* PELISH: Admins should be able to join any channel */
				clan = account_get_clan(acc);
				if ((!clan) || (clan_get_clantag(clan) != clantag)) {
					if (!channel)
					{
						message_send_text(c, message_type_error, c, std::string("Unable to join channel " + std::string(channelname) + ", there is no member of that clan in the channel!").c_str());

						if (conn_get_game(c) || c->protocol.chat.channel == NULL)
						{
							// FIXME: This is not tested to be according to battle.net!!
							// This is fix for empty clan channels with preventing to join CHANNEL_NAME_BANNED when is used _handle_join_command
							message_send_text(c, message_type_error, c, std::string("You have been redirected to " + std::string(CHANNEL_NAME_BANNED)).c_str());
							channel = channellist_find_channel_by_name(CHANNEL_NAME_BANNED, conn_get_country(c), realm_get_name(conn_get_realm(c)));
						}
						else
							return 0;
					}
					else
					{
						t_clan* ch_clan;
						if ((ch_clan = clanlist_find_clan_by_clantag(clantag)) && (clan_get_channel_type(ch_clan) == 1))
						{
							message_send_text(c, message_type_error, c, localize(c, "This is a private clan channel, unable to join!"));
							return 0;
						}
					}
				}
				else {
					if ((clan) && (clan_get_clantag(clan) == clantag) && (member = account_get_clanmember(acc))) {
						if (clanmember_get_status(member) >= CLAN_SHAMAN)
							/* PELISH: Giving tmpOP to SHAMAN and CHIEFTAIN on clanchannel */
							conn_set_tmpOP_channel(c, channelname);
					}
				}
			}

			if (c->protocol.chat.channel)
				conn_part_channel(c);

			if (channel)
			{
				if (channel_check_banning(channel, c))
				{
					message_send_text(c, message_type_error, c, localize(c, "You are banned from that channel."));
					return -1;
				}

				if ((account_get_auth_admin(acc, NULL) != 1) && (account_get_auth_admin(acc, channelname) != 1) &&
					(account_get_auth_operator(acc, NULL) != 1) && (account_get_auth_operator(acc, channelname) != 1) &&
					(channel_get_max(channel) == 0))
				{
					message_send_text(c, message_type_error, c, localize(c, "That channel is for Admins/Operators only."));
					return -1;
				}

				if ((account_get_auth_admin(acc, NULL) != 1) && (account_get_auth_admin(acc, channelname) != 1) &&
					(account_get_auth_operator(acc, NULL) != 1) && (account_get_auth_operator(acc, channelname) != 1) &&
					(channel_get_max(channel) != -1) && (channel_get_curr(channel) >= channel_get_max(channel)))
				{
					message_send_text(c, message_type_error, c, localize(c, "The channel is currently full."));
					return -1;
				}
			}

			if (conn_set_joingamewhisper_ack(c, 0) < 0)
				eventlog(eventlog_level_error, __FUNCTION__, "Unable to reset conn_set_joingamewhisper_ack flag");

			if (conn_set_leavegamewhisper_ack(c, 0) < 0)
				eventlog(eventlog_level_error, __FUNCTION__, "Unable to reset conn_set_leavegamewhisper_ack flag");

			/* if you're entering a channel, make sure they didn't exit a game without telling us */
			if (c->protocol.game)
			{
				if (!c->protocol.IamBOT)
				{
					if (c->protocol.game != NULL && c->protocol.game == c->protocol.hostedgame)
					{
						if (c->protocol.botconnection)
						{
							eventlog(eventlog_level_debug, __FUNCTION__, "!send unhost command");
							message_send_text(c->protocol.botconnection, message_type_whisper, c->protocol.botconnection, "!unhost");
						}
					}
				}

				//game_del_player(c->protocol.game, c);
				conn_send_ah_packet(c, 0x11); //here
				conn_set_game(c, 0, 0, 0,t_game_type::game_type_none, 0);
				if (c->protocol.IamBOT)
				{
					set_bot_availabled(c, true);
				}
			}

			created = 0;

			if (!channel)
			{
				if (clantag)
					channel = channel_create(channelname, channelname, 0, 0, 1, 1, prefs_get_chanlog(), NULL, NULL, (prefs_get_maxusers_per_channel() > 0) ? prefs_get_maxusers_per_channel() : -1, 0, 1, 0);
				else
					channel = channel_create(channelname, channelname, 0, 0, 1, 1, prefs_get_chanlog(), NULL, NULL, (prefs_get_maxusers_per_channel() > 0) ? prefs_get_maxusers_per_channel() : -1, 0, 0, 0);
				if (!channel)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "[{}] could not create channel on join \"{}\"", conn_get_socket(c), channelname);
					return -1;
				}
				created = 1;
			}

			c->protocol.chat.channel = channel;

			if (channel_add_connection(channel, c) < 0)
			{
				if (created)
					channel_destroy(channel, &curr);
				c->protocol.chat.channel = NULL;
				return -1;
			}

			eventlog(eventlog_level_info, __FUNCTION__, "[{}] joined channel \"{}\"", conn_get_socket(c), channel_get_name(c->protocol.chat.channel));
			conn_send_welcome(c);

			if (c->protocol.chat.channel && (channel_get_flags(c->protocol.chat.channel) & channel_flags_thevoid))
				message_send_text(c, message_type_info, c, localize(c, "This channel does not have chat privileges."));
			if (clantag && clan && (clan_get_clantag(clan) == clantag))
			{
				char msgtemp[MAX_MESSAGE_LEN];
				std::sprintf(msgtemp, "%s", clan_get_motd(clan));
				message_send_text(c, message_type_info, c, msgtemp);
			}


			if (conn_is_irc_variant(c) == 0)
			{
				channel_display_topic(c->protocol.chat.channel, c);
			}

			if (c->protocol.chat.channel && (channel_get_flags(c->protocol.chat.channel) & channel_flags_moderated))
				message_send_text(c, message_type_error, c, localize(c, "This channel is moderated."));

			if (c->protocol.chat.channel != oldchannel)
				clanmember_on_change_status_by_connection(c);


			if (channel)
			{
				t_elem const* _curr;
				t_connection* _conn;
				LIST_TRAVERSE_CONST(connlist(), _curr)
				{
					_conn = (t_connection*)elem_get_data(_curr);
					if (_conn && _conn->protocol.followaccount == acc)
					{
						if (!_conn->protocol.game)
						{
							message_send_text(_conn, message_type_info, _conn, localize(c, "You follow {} user to new channel.", account_get_name(acc)));
							conn_set_channel(_conn, channelname);
						}
					}
				}

			}


			return 0;
		}

		extern int conn_part_channel(t_connection* c)
		{
			if (!c) {
				ERROR0("got NULL connection");
				return -1;
			}

			if (!c->protocol.chat.channel) {
				ERROR0("client want to PART channel but is not in channel! c->protocol.chat.channel == NULL");
				return -1;
			}

			channel_del_connection(c->protocol.chat.channel, c, message_type_part, NULL);
			c->protocol.chat.channel = NULL;

			return 0;
		}

		extern int conn_kick_channel(t_connection* c, char const* text)
		{
			/* According to RFC2812 in *text is reason, wol sends by text op nickname */

			if (!c) {
				ERROR0("got NULL connection");
				return -1;
			}

			if (!c->protocol.chat.channel) {
				ERROR0("client want to KICK channel but is not in channel! c->protocol.chat.channel == NULL");
				return -1;
			}

			channel_del_connection(c->protocol.chat.channel, c, message_type_kick, text);
			c->protocol.chat.channel = NULL;

			return 0;
		}

		extern int conn_quit_channel(t_connection* c, char const* text)
		{
			/* According to RFC2812 in *text is user difinable QUIT message i.e.: Gone to have lunch */

			if (!c) {
				ERROR0("got NULL connection");
				return -1;
			}

			if (!c->protocol.chat.channel) {
				ERROR0("client want to QUIT channel but is not in channel! c->protocol.chat.channel == NULL");
				return -1;
			}

			channel_del_connection(c->protocol.chat.channel, c, message_type_quit, text);
			c->protocol.chat.channel = NULL;

			return 0;
		}

		extern t_game* conn_get_game(t_connection const* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return NULL;
			}

			//eventlog(eventlog_level_error, __FUNCTION__, "info: {} {}", game_get_count(c->protocol.game), game_get_status(c->protocol.game));

			return c->protocol.game;
		}


		extern int conn_set_game(t_connection* c, char const* gamename, char const* gamepass, char const* gameinfo, t_game_type type, int version)
			/*
			 * If game not exists (create) version != 0 (called in handle_bnet.c, function _client_startgameX())
			 * If game exists (join) version == 0 always (called in handle_bnet.c, function _client_joingame())
			 * If game exists (join) gameinfo == "" (called in handle_bnet.c, function _client_joingame())
			 * [KWS]
			 */
		{
			if (!c) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return 0;
			}


			if (!c->protocol.account) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return 0;
			}

			if (c->protocol.lastbadgametime + 3 > now)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got badgame ");
				return 0;
			}

			int timeoutreason = 0;
			if (conn_ah_timeout(c, &timeoutreason) && conn_get_account(c))
			{
				account_add_newevent(conn_get_account(c), 8);
				eventlog(eventlog_level_error, __FUNCTION__, "[ANTIHACK] ban user [{}]. Reason: Timeout.[EXTRA][{}]", conn_get_socket(c), timeoutreason);
				/*	account_set_auth_lock( c->protocol.account, 1 );
				account_set_auth_lockreason( c->protocol.account, "Antihack" );*/
				conn_send_ah_gamemessagebox(c, 1, ("|c0000FF40Test Protection:|r|n |c00FF8000Ban with reason:|r |c00FF0000Timeout.[EXTRA]"));
				if (c->protocol.game)
				{
					game_del_player(c->protocol.game, c);
				}
				conn_set_account(c, NULL);
				conn_set_state(c, conn_state_destroy);
				return 0;
			}


			t_account* useracc = c->protocol.account;
			if (useracc && !c->protocol.game && !c->protocol.IamBOT)
			{

				eventlog(eventlog_level_error, __FUNCTION__, " acc {}  #2", account_get_name(useracc));



				// Ќужно исправить
				/*bool OkayCreateBadGame = false;

				if ( t_connection * ownerbadgame = game_get_owner( c->protocol.game ) )
				{
					t_account * accbadowner = ownerbadgame->protocol.account;
					if ( accbadowner && IsHostBot( account_get_name( useracc ) ) )
					{
						OkayCreateBadGame = true;
					}
				}

				if ( !OkayCreateBadGame && c->protocol.lastbadgametime + 3 > now )
				{
					eventlog( eventlog_level_error, __FUNCTION__, "[ANTIHACK] user [{}] want to create bad game!", conn_get_socket( c ) );
					return 0;
				}*/
			}

			bool GameFound = true;

			if (c->protocol.game) {
				if (gamename) {
					if (strcasecmp(gamename, game_get_name(c->protocol.game)))
						eventlog(eventlog_level_error, __FUNCTION__, "[{}] tried to join a new game \"{}\" while already in a game \"{}\"!", conn_get_socket(c), gamename, game_get_name(c->protocol.game));
					else return 0;
				}
				game_del_player(conn_get_game(c), c);
				if (!c->protocol.IamBOT)
					conn_send_ah_packet(c, 0x11);//here
				else
					set_bot_availabled(c, true);
				
				c->protocol.game = NULL;
			}
			else GameFound = false;
			//}


			//if (c->protocol.game) {
			//	if (gamename && gamename[0]!='\0')
			//		eventlog(eventlog_level_error, __FUNCTION__, "[{}] tried to join a new game \"{}\"!", conn_get_socket(c), gamename);
			//	else
			//	{
			//		conn_send_ah_packet(c, 0xC9108E3);
			//		eventlog(eventlog_level_error, __FUNCTION__, "[{}] tried to close game!", conn_get_socket(c), gamename);
			//	}

			//	game_del_player(c->protocol.game, c);

			//	if (!c->protocol.IamBOT)
			//		conn_send_ah_packet(c, 0x11);//here


			//	if (gamename) {
			//		if (strcasecmp(gamename, game_get_name(c->protocol.game)))
			//			eventlog(eventlog_level_error, __FUNCTION__, "[{}] tried to join a new game \"{}\" while already in a game \"{}\"!", conn_get_socket(c), gamename, game_get_name(c->protocol.game));
			//		else return 0;
			//	}
			//	if (c->protocol.IamBOT)
			//	{
			//		set_bot_availabled(c, true);
			//	}
			//	c->protocol.game = NULL;
			//}
			//

			if (gamename)
			{
				if (!(c->protocol.game = gamelist_find_game_available(gamename, c->protocol.client.clienttag, type))
					&& !gamelist_find_game_available(gamename, c->protocol.client.clienttag, game_type_all)) {
					/* do not allow creation of games with same name of same clienttag when game is not started or done */
					// create game with initial values
					c->protocol.game = game_create(gamename, gamepass, gameinfo, type, version, c->protocol.client.clienttag, conn_get_gameversion(c));

					if (c->protocol.game && conn_get_realm(c) && conn_get_charname(c)) {
						game_set_realmname(c->protocol.game, realm_get_name(conn_get_realm(c)));
						realm_add_game_number(conn_get_realm(c), 1);
						send_d2cs_gameinforeq(c);
					}
					if (!c->protocol.game)
					{
						conn_send_ah_gamemessagebox(c, 1, "Please enter correct game name.");
					}
				}
				else
				{
					if (game_get_forstats(c->protocol.game) && game_get_statstype(c->protocol.game) &&
						game_get_statstype(c->protocol.game)[0] != '\0'
						)
					{
						std::time_t bantime = account_is_banned_stats(c->protocol.account);
						if (bantime > 0) {

							conn_send_ah_gamemessagebox(c, 1, localize(c, "You is banned.|nTry to join after {}:{} seconds.", bantime / 60, bantime % 60).c_str(), 250);
							return 0;
						}
					}
				}
				if (c->protocol.game)
				{
					// add new player to the game
					if (game_add_player(c->protocol.game, gamepass, version, c) < 0) {
						if (!c->protocol.IamBOT)
							conn_send_ah_packet(c, 0x11);//here
						c->protocol.game = NULL;
						if (c->protocol.IamBOT)
						{
							set_bot_availabled(c, true);
						}
						return -1;
					}

#ifdef WITH_LUA
					// handle game create when it's owner joins the game
					if (c == game_get_owner(c->protocol.game))
						lua_handle_game(c->protocol.game, NULL, luaevent_game_create);
#endif

					if (game_is_ladder(c->protocol.game)) {
						if (c == game_get_owner(c->protocol.game))
							message_send_text(c, message_type_info, c, localize(c, "Created ladder game"));
						else
							message_send_text(c, message_type_info, c, localize(c, "Joined ladder game"));
					}
				}
			}
			else
			{
				c->protocol.game = NULL;
			}

			//here
			/*if ( !GameFound && c->protocol.game )
			{
				const char * _gamename2 = game_get_name( c->protocol.game );
				if ( _gamename2 && _gamename2[ 0 ] != '\0' )
				{
					if ( strstr( _gamename2, "[WL]" ) == _gamename2 )
						conn_send_ah_packet( c, 0x10 );
				}
				else if ( gamename && gamename[ 0 ] != '\0' )
				{
					if ( strstr( gamename, "[WL]" ) == gamename )
						conn_send_ah_packet( c, 0x10 );
				}

			}
*/

			if (!c->protocol.game)
			{
				if (!c->protocol.IamBOT)
				{
					conn_send_ah_packet(c, 0x11);//here
					c->protocol.FoundWhiteMep = false;
				}
				if (c->protocol.IamBOT)
				{
					set_bot_availabled(c, true);
				}
			}
			else
			{
				if (!c->protocol.IamBOT)
				{
					if (c->protocol.FoundWhiteMep)
					{
						c->protocol.FoundWhiteMep = false;
						conn_send_ah_packet(c, 0x10);//here
						eventlog(eventlog_level_error, __FUNCTION__, " acc {}  create whitelist map", account_get_name(useracc));
					}
				}
			}

			if (useracc)
			{
				eventlog(eventlog_level_error, __FUNCTION__, " acc {}  #2", account_get_name(useracc));
			}



			return 0;
		}

		extern unsigned int conn_get_tcpaddr(t_connection* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return 0;
			}

			return c->socket.tcp_addr;
		}


		extern t_packet* conn_get_in_queue(t_connection* c)
		{
			assert(c);

			return c->protocol.queues.inqueue;
		}


		extern void conn_put_in_queue(t_connection* c, t_packet* packet)
		{
			assert(c);

			c->protocol.queues.inqueue = packet;
		}


		extern unsigned int conn_get_in_size(t_connection const* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return 0;
			}

			return c->protocol.queues.insize;
		}


		extern void conn_set_in_size(t_connection* c, unsigned int size)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return;
			}

			c->protocol.queues.insize = size;
		}


		extern unsigned int conn_get_out_size(t_connection const* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return 0;
			}
			return c->protocol.queues.outsize;
		}


		extern void conn_set_out_size(t_connection* c, unsigned int size)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return;
			}

			c->protocol.queues.outsize = size;
		}

		extern int conn_push_outqueue(t_connection* c, t_packet* packet)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}

			if (!packet)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL packet");
				return -1;
			}

			// Protection from hack attempt
			// Limit out queue packets due to it may cause memory leak with not enough memory program crash on a server machine
			t_queue** q = &c->protocol.queues.outqueue;
			if (prefs_get_packet_limit() && queue_get_length((t_queue const * const *)q) > prefs_get_packet_limit())
			{
				queue_clear(q);
				conn_set_state(c, conn_state_destroy);
				eventlog(eventlog_level_error, __FUNCTION__, "outqueue reached limit of {} packets (hack attempt?)", prefs_get_packet_limit());
				return 0;
			}

			queue_push_packet((t_queue**)&c->protocol.queues.outqueue, packet);

			if (!c->protocol.queues.outsizep++) fdwatch_update_fd(c->socket.fdw_idx, fdwatch_type_read | fdwatch_type_write);

			return 0;
		}

		extern int conn_clear_outqueue(t_connection* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}

			queue_clear(&c->protocol.queues.outqueue);
			return 0;
		}

		extern t_packet* conn_peek_outqueue(t_connection* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return NULL;
			}

			if (c->protocol.queues.outqueue)
				return queue_peek_packet((t_queue const* const*)&c->protocol.queues.outqueue);
			else return 0;
		}

		extern t_packet* conn_pull_outqueue(t_connection* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return NULL;
			}

			if (c->protocol.queues.outsizep) {
				if (!(--c->protocol.queues.outsizep)) fdwatch_update_fd(c->socket.fdw_idx, fdwatch_type_read);
				return queue_pull_packet((t_queue**)&c->protocol.queues.outqueue);
			}

			return NULL;
		}

		extern char const* conn_get_username_real(t_connection const* c, char const* fn, unsigned int ln)
		{
			char const* result;

			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection (from {}:{})", fn, ln);
				return NULL;
			}

			if (!c->protocol.account)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account (from {}:{})", fn, ln);
				return NULL;
			}
			result = account_get_name(c->protocol.account);
			if (result == NULL)
				eventlog(eventlog_level_error, __FUNCTION__, "returned previous error after being called by {}:{}", fn, ln);

			return result;
		}


		extern char const* conn_get_chatname(t_connection const* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return NULL;
			}

			if ((c->protocol.cclass == conn_class_bnet) && c->protocol.bound)
			{
				if (c->protocol.d2.character)
					return character_get_name(c->protocol.d2.character);
				if (c->protocol.bound->protocol.d2.character)
					return character_get_name(c->protocol.bound->protocol.d2.character);
				eventlog(eventlog_level_error, __FUNCTION__, "[{}] got connection class {} bound to class {} without a character", conn_get_socket(c), conn_class_get_str(c->protocol.cclass), c->protocol.bound->protocol.cclass);
			}
			if (!c->protocol.account)
				return NULL; /* no name yet */
			return conn_get_loggeduser(c);
		}


		extern int conn_unget_chatname(t_connection const* c, char const* name)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}

			if ((c->protocol.cclass == conn_class_bnet) && c->protocol.bound)
				return 0;
			return 0;
		}



		extern char const* conn_get_chatcharname(t_connection const* c, t_connection const* dst)
		{
			char const* accname;
			char* chatcharname;

			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return NULL;
			}

			if (!c->protocol.account)
				return NULL; /* no name yet */

			/* for D2 Users */
			accname = conn_get_loggeduser(c);
			if (!accname)
				return NULL;

			if (dst && dst->protocol.d2.charname)
			{
				const char* mychar;

				if (c->protocol.d2.charname) mychar = c->protocol.d2.charname;
				else mychar = "";
				chatcharname = (char*)xmalloc(std::strlen(accname) + 2 + std::strlen(mychar));
				std::sprintf(chatcharname, "%s*%s", mychar, accname);
			}
			else chatcharname = xstrdup(accname);

			return chatcharname;
		}


		extern int conn_unget_chatcharname(t_connection const* c, char const* name)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}
			if (!name)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL name");
				return -1;
			}

			xfree((void*)name); /* avoid warning */
			return 0;
		}


		extern t_message_class conn_get_message_class(t_connection const* c, t_connection const* dst)
		{
			if (dst && dst->protocol.d2.charname) /* message to D2 user must be char*account */
				return message_class_charjoin;

			return message_class_normal;
		}


		extern unsigned int conn_get_userid(t_connection const* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return 0;
			}

			if (!c->protocol.account)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return 0;
			}

			return account_get_uid(c->protocol.account);
		}


		extern int conn_get_socket(t_connection const* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}

			return c->socket.tcp_sock;
		}


		extern int conn_get_game_socket(t_connection const* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}

			return c->socket.udp_sock;
		}


		extern int conn_set_game_socket(t_connection* c, int usock)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}

			c->socket.udp_sock = usock;
			return 0;
		}

		/* Player icon that displayed in a channel in all games */
		extern nonstd::optional<std::string> conn_get_playerinfo(t_connection const * c)
		{
			if (c == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return nonstd::nullopt;
			}

			t_account* account = conn_get_account(c);
			if (account == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "connection has no account");
				return nonstd::nullopt;
			}

			t_clienttag client_tag = conn_get_fake_clienttag(c);
			if (client_tag == 0)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "connection has NULL fakeclienttag");
				return nonstd::nullopt;
			}

			try
			{
				char reversed_client_tag[5] = {};
				tag_uint_to_revstr(reversed_client_tag, client_tag);

				std::string icon;
				{
					// Icon Precedance
					// 1. Lua (handle_user.lua:handle_user_icon())
					// 2. user-selected icon: account_get_strattr(account, "Record\\" + tag_uint_to_str2(clienttag) + "\\userselected_icon")
					// 3. custom icons (icons.conf)
					// 4. default

					t_icon_info* custom_icon = customicons_get_icon_by_account(account, client_tag);

			// allow set icon to a user directly from the database (override default tag always if not null)
					const char* usericon = account_get_user_icon(account, client_tag);
					if (usericon)
					{
						icon = usericon;
					}
					else if (prefs_get_custom_icons() == 1 && customicons_allowed_by_client(client_tag) && (custom_icon != nullptr) && (custom_icon->icon_code != nullptr))
			{
						// if custom_icons is enabled then set a custom client tag by player rating
						icon = custom_icon->icon_code;
			}
					else
			{
						// default icon code for WAR3 and W3XP is different than all other clients, which simply use their reversed client tag
						if (client_tag == CLIENTTAG_WARCRAFT3_UINT || client_tag == CLIENTTAG_WAR3XP_UINT)
				{
							char raceicon = '\0'; // appeared in 1.03
							unsigned int raceiconnumber = 0;
							unsigned int wins = 0;
							account_get_raceicon(account, &raceicon, &raceiconnumber, &wins, client_tag);

							icon = fmt::format("{}{}{}", raceiconnumber, raceicon, "3W");
				}
				else
				{
							icon = reversed_client_tag;
						}
					}

#ifdef WITH_LUA
					// change icon info from Lua
					if (const char* lua_icon = lua_handle_user_icon((t_connection*)c, icon.c_str()))
					{
						icon = lua_icon;
			}
#endif
				}

				// AKA chat statstring
				// https://bnetdocs.org/document/18/chat-statstrings
				std::string playerinfo;

				if (client_tag == CLIENTTAG_BNCHATBOT_UINT)
				{
					playerinfo = fmt::format("{}", reversed_client_tag); // FIXME: what to return here?
				}
				else if ((client_tag == CLIENTTAG_STARCRAFT_UINT) || (client_tag == CLIENTTAG_BROODWARS_UINT))
				{
					if (conn_get_versionid(c) <= 0x000000c7)
			{
						playerinfo = fmt::format("{} {} {} {} {} {}",
							reversed_client_tag,
							account_get_ladder_rating(account, client_tag, ladder_id_normal),
							account_get_ladder_rank(account, client_tag, ladder_id_normal),
							account_get_normal_wins(account, client_tag),
							0,  // unknown
							0); // unknown
					}
					else
					{
						playerinfo = fmt::format("{} {} {} {} {} {} {} {} {} {}",
							reversed_client_tag,
							account_get_ladder_rating(account, client_tag, ladder_id_normal),
							account_get_ladder_rank(account, client_tag, ladder_id_normal),
							account_get_normal_wins(account, client_tag),
							0, // 0 = not spawn, 1 = spawn
							0, // League ID
							account_get_ladder_high_rating(account, client_tag, ladder_id_normal),
							0, // IronMan Ladder Rating (not applicable to StarCraft)
							0, // IronMan Ladder Rank   (not applicable to StarCraft)
							icon);
					}
				}
				else if (client_tag == CLIENTTAG_SHAREWARE_UINT)
				{
					// same format as STAR/SEXP version id <= 0xC7

					playerinfo = fmt::format("{} {} {} {} {} {}",
						reversed_client_tag,
						account_get_ladder_rating(account, client_tag, ladder_id_normal),
						account_get_ladder_rank(account, client_tag, ladder_id_normal),
						account_get_normal_wins(account, client_tag),
						0,  // unknown
						0); // unknown
				}
				else if (client_tag == CLIENTTAG_WARCIIBNE_UINT)
				{
					unsigned int normal_ladder_rating = account_get_ladder_rating(account, client_tag, ladder_id_normal);
					unsigned int ironman_ladder_rating = account_get_ladder_rating(account, client_tag, ladder_id_ironman);
					unsigned int highest_ladder_rating = normal_ladder_rating > ironman_ladder_rating ? normal_ladder_rating : ironman_ladder_rating;

					playerinfo = fmt::format("{} {} {} {} {} {} {} {} {} {}",
						reversed_client_tag,
						normal_ladder_rating,
						account_get_ladder_rank(account, client_tag, ladder_id_normal),
						account_get_normal_wins(account, client_tag),
						0, // 0 = not spawn, 1 = spawn
						0, // League ID
						highest_ladder_rating,
						ironman_ladder_rating,
						account_get_ladder_rank(account, client_tag, ladder_id_ironman),
						icon);
				}
				else if ((client_tag == CLIENTTAG_DIABLORTL_UINT) || (client_tag == CLIENTTAG_DIABLOSHR_UINT))
				{
					playerinfo = fmt::format("{} {} {} {} {} {} {} {} {} {}",
						reversed_client_tag,
						account_get_normal_level(account, client_tag),
						account_get_normal_class(account, client_tag),
						account_get_normal_diablo_kills(account, client_tag),
						account_get_normal_strength(account, client_tag),
						account_get_normal_magic(account, client_tag),
						account_get_normal_dexterity(account, client_tag),
						account_get_normal_vitality(account, client_tag),
						account_get_normal_gold(account, client_tag),
						0); // 0 = not spawn, 1 = spawn
			}
				else if (client_tag == CLIENTTAG_DIABLO2DV_UINT || client_tag == CLIENTTAG_DIABLO2XP_UINT)
			{
				/* This sets portrait of character */
				if (!conn_get_realm(c) || !conn_get_realminfo(c))
				{
						// "Open Battle.net"

						playerinfo = fmt::format("{}",
							reversed_client_tag);
				}
				else
				{
						// "Closed Battle.net"

						// std::sprintf(realminfo, "%4s%s,%s,%s", revtag, realmname, charname, portrait);
						playerinfo = fmt::format("{}",
							conn_get_realminfo(c));
					}
				}
				else if (client_tag == CLIENTTAG_WARCRAFT3_UINT || client_tag == CLIENTTAG_WAR3XP_UINT)
				{
					t_clantag clantag = 0;
					t_clan* clan = account_get_clan(account);
					if (clan != nullptr)
					{
						clantag = clan_get_clantag(clan);
					}

					if (clantag == 0)
					{
						// account is not in a clan
						playerinfo = fmt::format("{} {} {}",
							reversed_client_tag,
							icon,
							account_get_highestladderlevel(account, client_tag));
			}
			else
					{
						// account is in a clan
						char reversed_clan_tag[5] = {};
						tag_uint_to_revstr(reversed_clan_tag, clantag);

						playerinfo = fmt::format("{} {} {} {}",
							reversed_client_tag,
							icon,
							account_get_highestladderlevel(account, client_tag),
							reversed_clan_tag);
					}
				}
				else
			{
					char client_tag_str[5];
					tag_uint_to_str(client_tag_str, client_tag);
					eventlog(eventlog_level_error, __FUNCTION__, "unknown client tag \"{}\"", client_tag_str);

					playerinfo = fmt::format("{}",
						reversed_client_tag);
			}

			return playerinfo;
		}
			catch (const std::exception& e)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could not return playerinfo ({})", e.what());
				return nonstd::nullopt;
			}
		}


		extern int conn_set_playerinfo(t_connection const* c, char const* playerinfo)
		{
			t_clienttag clienttag;
			char	clienttag_str[5];

			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}
			if (!playerinfo)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL playerinfo");
				return -1;
			}
			clienttag = c->protocol.client.clienttag;

			if (clienttag == CLIENTTAG_DIABLORTL_UINT)
			{
				unsigned int level;
				unsigned int chclass;
				unsigned int diablo_kills;
				unsigned int strength;
				unsigned int magic;
				unsigned int dexterity;
				unsigned int vitality;
				unsigned int gold;

				if (std::sscanf(playerinfo, "LTRD %u %u %u %u %u %u %u %u %*u",
					&level,
					&chclass,
					&diablo_kills,
					&strength,
					&magic,
					&dexterity,
					&vitality,
					&gold) != 8)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got bad playerinfo");
					return -1;
				}

				account_set_normal_level(conn_get_account(c), clienttag, level);
				account_set_normal_class(conn_get_account(c), clienttag, chclass);
				account_set_normal_diablo_kills(conn_get_account(c), clienttag, diablo_kills);
				account_set_normal_strength(conn_get_account(c), clienttag, strength);
				account_set_normal_magic(conn_get_account(c), clienttag, magic);
				account_set_normal_dexterity(conn_get_account(c), clienttag, dexterity);
				account_set_normal_vitality(conn_get_account(c), clienttag, vitality);
				account_set_normal_gold(conn_get_account(c), clienttag, gold);
			}
			else if (clienttag == CLIENTTAG_DIABLOSHR_UINT)
			{
				unsigned int level;
				unsigned int chclass;
				unsigned int diablo_kills;
				unsigned int strength;
				unsigned int magic;
				unsigned int dexterity;
				unsigned int vitality;
				unsigned int gold;

				if (std::sscanf(playerinfo, "RHSD %u %u %u %u %u %u %u %u %*u",
					&level,
					&chclass,
					&diablo_kills,
					&strength,
					&magic,
					&dexterity,
					&vitality,
					&gold) != 8)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got bad playerinfo");
					return -1;
				}

				account_set_normal_level(conn_get_account(c), clienttag, level);
				account_set_normal_class(conn_get_account(c), clienttag, chclass);
				account_set_normal_diablo_kills(conn_get_account(c), clienttag, diablo_kills);
				account_set_normal_strength(conn_get_account(c), clienttag, strength);
				account_set_normal_magic(conn_get_account(c), clienttag, magic);
				account_set_normal_dexterity(conn_get_account(c), clienttag, dexterity);
				account_set_normal_vitality(conn_get_account(c), clienttag, vitality);
				account_set_normal_gold(conn_get_account(c), clienttag, gold);
			}
			else if (clienttag == CLIENTTAG_DIABLO2DV_UINT)
			{
				/* not much to do */ /* FIXME: get char name here? */
				eventlog(eventlog_level_trace, __FUNCTION__, "[{}] playerinfo request for client \"{}\" playerinfo=\"{}\"", conn_get_socket(c), tag_uint_to_str(clienttag_str, clienttag), playerinfo);
			}
			else if (clienttag == CLIENTTAG_DIABLO2XP_UINT)
			{
				/* in playerinfo we get strings of the form "Realmname,charname" */
				eventlog(eventlog_level_trace, __FUNCTION__, "[{}] playerinfo request for client \"{}\" playerinfo=\"{}\"", conn_get_socket(c), tag_uint_to_str(clienttag_str, clienttag), playerinfo);
			}
			else
			{
				eventlog(eventlog_level_warn, __FUNCTION__, "setting playerinfo for client \"{}\" not supported (playerinfo=\"{}\")", tag_uint_to_str(clienttag_str, clienttag), playerinfo);
				return -1;
			}

			return 0;
		}


		extern char const* conn_get_realminfo(t_connection const* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return NULL;
			}
			return c->protocol.d2.realminfo;
		}


		extern int conn_set_realminfo(t_connection* c, char const* realminfo)
		{
			char const* temp;

			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}

			if (realminfo)
				temp = xstrdup(realminfo);
			else
				temp = NULL;

			if (c->protocol.d2.realminfo) /* if it was set before, free it now */
				xfree((void*)c->protocol.d2.realminfo); /* avoid warning */
			c->protocol.d2.realminfo = temp;
			return 0;
		}


		extern char const* conn_get_charname(t_connection const* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return NULL;
			}
			return c->protocol.d2.charname;
		}


		extern int conn_set_charname(t_connection* c, char const* charname)
		{
			char const* temp;

			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}

			if (charname)
				temp = xstrdup(charname);
			else
				temp = charname;

			if (c->protocol.d2.charname) /* free it, if it was previously set */
				xfree((void*)c->protocol.d2.charname); /* avoid warning */
			c->protocol.d2.charname = temp;
			return 0;
		}


		extern int conn_set_idletime(t_connection* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}

			c->protocol.chat.last_message = now;
			return 0;
		}


		extern unsigned int conn_get_idletime(t_connection const* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return 0;
			}

			return (unsigned int)std::difftime(now, c->protocol.chat.last_message);
		}


		extern t_realm* conn_get_realm(t_connection const* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return NULL;
			}

			return c->protocol.d2.realm;
		}


		extern int conn_set_realm(t_connection* c, t_realm* realm)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}

			if (c->protocol.d2.realm)
				realm_put(c->protocol.d2.realm, &c->protocol.d2.realm_regref);

			if (!realm)
				c->protocol.d2.realm = NULL;
			else
			{
				c->protocol.d2.realm = realm_get(realm, &c->protocol.d2.realm_regref);
				eventlog(eventlog_level_debug, __FUNCTION__, "[{}] set to \"{}\"", conn_get_socket(c), realm_get_name(realm));
			}

			return 0;
		}

		extern int conn_set_realm_cb(void* data, void* newref)
		{
			t_connection* c = (t_connection*)data;
			t_realm* newrealm = (t_realm*)newref;

			assert(c->protocol.d2.realm);	/* this should never be NULL here */

			/* we are removing a reference */
			realm_put(c->protocol.d2.realm, &c->protocol.d2.realm_regref);

			if (newrealm)
				c->protocol.d2.realm = realm_get(newrealm, &c->protocol.d2.realm_regref);
			else {
				/* close the connection for players on unconfigured realms */
				conn_set_state(c, conn_state_destroy);
				c->protocol.d2.realm = NULL;
			}

			return 0;
		}


		extern int conn_set_character(t_connection* c, t_character* character)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}
			if (!character)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL character");
				return -1;
			}

			c->protocol.d2.character = character;

			return 0;
		}


		extern void conn_set_country(t_connection* c, char const* country)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return;
			}
			if (!country)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL country");
				return;
			}

			if (c->protocol.client.country)
				xfree((void*)c->protocol.client.country); /* avoid warning */
			c->protocol.client.country = xstrdup(country);
		}


		extern char const* conn_get_country(t_connection const* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return NULL;
			}

			return c->protocol.client.country;
		}


		extern int conn_bind(t_connection* c1, t_connection* c2)
		{
			if (!c1)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}
			if (!c2)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}

			c1->protocol.bound = c2;
			c2->protocol.bound = c1;

			return 0;
		}


		extern int conn_set_ircline(t_connection* c, char const* line)
		{
			if (!c) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}
			if (!line) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL line");
				return -1;
			}
			if (c->protocol.chat.irc.ircline)
				xfree((void*)c->protocol.chat.irc.ircline); /* avoid warning */
			c->protocol.chat.irc.ircline = xstrdup(line);
			return 0;
		}


		extern char const* conn_get_ircline(t_connection const* c)
		{
			if (!c) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return NULL;
			}
			return c->protocol.chat.irc.ircline;
		}


		extern int conn_set_ircpass(t_connection* c, char const* pass)
		{
			if (!c) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}
			if (c->protocol.chat.irc.ircpass)
				xfree((void*)c->protocol.chat.irc.ircpass); /* avoid warning */
			if (!pass)
				c->protocol.chat.irc.ircpass = NULL;
			else
				c->protocol.chat.irc.ircpass = xstrdup(pass);

			return 0;
		}


		extern char const* conn_get_ircpass(t_connection const* c)
		{
			if (!c) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return NULL;
			}
			return c->protocol.chat.irc.ircpass;
		}


		extern int conn_set_ircping(t_connection* c, unsigned int ping)
		{
			if (!c) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}
			c->protocol.chat.irc.ircping = ping;
			return 0;
		}


		extern unsigned int conn_get_ircping(t_connection const* c)
		{
			if (!c) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return 0;
			}
			return c->protocol.chat.irc.ircping;
		}

		// NonReal
		extern int conn_get_welcomed(t_connection const* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return 0;
			}

			return (c->protocol.cflags & conn_flags_welcomed);
		}

		// NonReal
		extern void conn_set_welcomed(t_connection* c, int welcomed)
		{

			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return;
			}
			c->protocol.cflags |= conn_flags_welcomed;
		}

		extern int conn_quota_exceeded(t_connection* con, char const* text)
		{
			t_qline* qline;
			t_elem* curr;
			t_account* con_acc = con->protocol.account;

			//here
			if (!prefs_get_quota() ||
				!conn_get_account(con) || con->protocol.IsNewClient || con->protocol.IamBOT || account_get_auth_botlogin(con_acc)
				// FIXME: (HarpyWar) do not allow flood for admins due to possible abuse with quick command sending that high load a server processor
				//                   If we really need to ignore flood protection, it can be allowed in Lua config for special users
				/* || (account_get_command_groups(conn_get_account(con)) & command_get_group("/admin-con"))*/
				)
				return 0;

			if (std::strlen(text) > prefs_get_quota_maxline() && !account_get_auth_botlogin(con_acc))
			{
				message_send_text(con, message_type_error, con, localize(con, "Your line length quota has been exceeded!"));
				return 1;
			}

			LIST_TRAVERSE(con->protocol.chat.quota.list, curr)
			{
				qline = (t_qline*)elem_get_data(curr);
				if (now >= qline->inf + (std::time_t)prefs_get_quota_time())
				{
					/* these lines are at least quota_time old */
					list_remove_elem(con->protocol.chat.quota.list, &curr);
					if (qline->count > con->protocol.chat.quota.totcount)
						eventlog(eventlog_level_error, __FUNCTION__, "qline->count={} but con->protocol.chat.quota.totcount={}", qline->count, con->protocol.chat.quota.totcount);
					con->protocol.chat.quota.totcount -= qline->count;
					xfree(qline);
				}
				else
					break; /* old items are first, so we know nothing else will match */
			}

			qline = (t_qline*)xmalloc(sizeof(t_qline));
			qline->inf = now; /* set the moment */
			if (std::strlen(text) > prefs_get_quota_wrapline()) /* round up on the divide */
				qline->count = (std::strlen(text) + prefs_get_quota_wrapline() - 1) / prefs_get_quota_wrapline();
			else
				qline->count = 1;

			list_append_data(con->protocol.chat.quota.list, qline);

			con->protocol.chat.quota.totcount += qline->count;



			if (con->protocol.chat.quota.totcount >= prefs_get_quota_lines())
			{
				if (con_acc && !con->protocol.IamBOT && !account_get_auth_botlogin(con_acc))
				{
					message_send_text(con, message_type_error, con, localize(con, "Your message quota has been exceeded!"));
					if (con->protocol.chat.quota.totcount >= prefs_get_quota_dobae())
					{
						/* kick out the dobae user for violation of the quota rule */
						conn_set_state(con, conn_state_destroy);
						if (con->protocol.chat.channel)
							channel_message_log(con->protocol.chat.channel, con, 0, "DISCONNECTED FOR DOBAE ABUSE");
						return 2;
					}
					return 1;
				}
			}

			return 0;
		}


		extern int conn_set_lastsender(t_connection* c, char const* sender)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}
			if (c->protocol.chat.lastsender)
				xfree((void*)c->protocol.chat.lastsender); /* avoid warning */
			if (!sender)
			{
				c->protocol.chat.lastsender = NULL;
				return 0;
			}
			c->protocol.chat.lastsender = xstrdup(sender);

			return 0;
		}


		extern char const* conn_get_lastsender(t_connection const* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return NULL;
			}
			return c->protocol.chat.lastsender;
		}

		extern int conn_get_echoback(t_connection* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return 0;
			}

			return (c->protocol.cflags & conn_flags_echoback);
		}

		extern void conn_set_echoback(t_connection* c, int echoback)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return;
			}
			if (echoback)
				c->protocol.cflags |= conn_flags_echoback;
			else
				c->protocol.cflags &= ~conn_flags_echoback;
		}

		extern int conn_set_udpok(t_connection* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}

			if (!(c->protocol.cflags & conn_flags_udpok))
			{
				c->protocol.cflags |= conn_flags_udpok;
				c->protocol.flags &= ~MF_PLUG;
			}

			return 0;
		}


		extern t_connection* conn_get_routeconn(t_connection const* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return NULL;
			}

			return c->protocol.w3.routeconn;
		}


		extern int conn_set_routeconn(t_connection* c, t_connection* rc)
		{
			if (!c) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}
			c->protocol.w3.routeconn = rc;

			return 0;
		}

		extern int conn_get_crtime(t_connection* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, "conn_get_crtime", "got NULL connection");
				return -1;
			}
			return c->protocol.cr_time;
		}

		extern int conn_set_joingamewhisper_ack(t_connection* c, unsigned int value)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}
			if (value)
				c->protocol.cflags |= conn_flags_joingamewhisper;
			else
				c->protocol.cflags &= ~conn_flags_joingamewhisper;
			return 0;
		}
		extern int conn_get_joingamewhisper_ack(t_connection* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}
			return (c->protocol.cflags & conn_flags_joingamewhisper);
		}

		extern int conn_set_leavegamewhisper_ack(t_connection* c, unsigned int value)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}
			if (value)
				c->protocol.cflags |= conn_flags_leavegamewhisper;
			else
				c->protocol.cflags &= ~conn_flags_leavegamewhisper;
			return 0;
		}
		extern int conn_get_leavegamewhisper_ack(t_connection* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}
			return (c->protocol.cflags & conn_flags_leavegamewhisper);
		}

		extern int conn_set_anongame_search_starttime(t_connection* c, std::time_t t)
		{
			if (c == NULL) {
				eventlog(eventlog_level_error, "conn_set_anongame_search_starttime", "got NULL connection");
				return -1;
			}
			c->protocol.w3.anongame_search_starttime = t;
			return 0;
		}

		extern std::time_t conn_get_anongame_search_starttime(t_connection* c)
		{
			if (c == NULL) {
				eventlog(eventlog_level_error, "conn_set_anongame_search_starttime", "got NULL connection");
				return ((std::time_t) 0);
			}
			return c->protocol.w3.anongame_search_starttime;
		}


		extern t_elist* conn_get_timer(t_connection* c)
		{
			if (!c) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return NULL;
			}

			return &c->protocol.timers;
		}


		extern int conn_add_fdwatch(t_connection* c, fdwatch_handler handle)
		{
			assert(c);
			c->socket.fdw_idx = fdwatch_add_fd(c->socket.tcp_sock, fdwatch_type_read, handle, c);
			return c->socket.fdw_idx;
		}


		extern void conn_close_read(t_connection* c)
		{
			assert(c);
			conn_set_state(c, conn_state_destroy);

			/* only if we still got output packets remove the read availability
			 * from fdwatch, we are NOT allowed to remove all availability or
			 * remove it completely from fdwatch while handling read, also
			 * if the connection has no output packets is ok to leave it
			 * in read availability check cause it will be closed immediately
			 * in connlist_reap() anyway
			 */
			if (conn_peek_outqueue(c))
				fdwatch_update_fd(c->socket.fdw_idx, fdwatch_type_write);
		}


		extern int conn_get_user_count_by_clienttag(t_clienttag ct)
		{
			t_connection* conn;
			t_elem const* curr;
			int clienttagusers = 0;

			/* Get Number of Users for client tag specific */
			LIST_TRAVERSE_CONST(connlist(), curr)
			{
				conn = (t_connection*)elem_get_data(curr);
				if ((ct == conn->protocol.client.clienttag)
					&& (conn->protocol.state == conn_state_loggedin)) clienttagusers++;
			}

			return clienttagusers;
		}

		extern int connlist_create(void)
		{
			conn_head = list_create();
			connarray_create();
			return 0;
		}

		extern int connlist_destroy(void)
		{
			if (conn_dead) list_destroy(conn_dead);
			conn_dead = NULL;
			connarray_destroy();
			/* FIXME: if called with active connection, connection are not freed */
			if (list_destroy(conn_head) < 0)
				return -1;
			conn_head = NULL;
			return 0;
		}

		extern void connlist_reap(void)
		{
			t_elem* curr;
			t_connection* c;

			if (!conn_dead || !conn_head) return;

			LIST_TRAVERSE(conn_dead, curr)
			{
				c = (t_connection*)elem_get_data(curr);

				if (!c)
					eventlog(eventlog_level_error, __FUNCTION__, "found NULL entry in conn_dead list");
				else if (!conn_peek_outqueue(c)) {
					conn_destroy(c, &curr, DESTROY_FROM_DEADLIST); /* also removes from conn_dead list and fdwatch */
				}
			}
		}

		extern t_list* connlist(void)
		{
			return conn_head;
		}


		extern t_connection* connlist_find_connection_by_accountname(char const* accountname)
		{
			t_account* temp;

			if (!accountname)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL accountname");
				return NULL;
			}

			if (!(temp = accountlist_find_account(accountname)))
				return NULL;

			return account_get_conn(temp);
		}

		extern t_connection* connlist_find_connection_by_account(t_account* account)
		{
			if (!account) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return NULL;
			}
			return account_get_conn(account);
		}


		extern t_connection* connlist_find_connection_by_sessionkey(unsigned int sessionkey)
		{
			t_connection* c;
			t_elem const* curr;

			LIST_TRAVERSE_CONST(conn_head, curr)
			{
				c = (t_connection*)elem_get_data(curr);
				if (c->protocol.sessionkey == sessionkey)
					return c;
			}

			return NULL;
		}


		extern t_connection* connlist_find_connection_by_sessionnum(unsigned int sessionnum)
		{
			return connarray_get_conn(sessionnum);
		}


		extern t_connection* connlist_find_connection_by_socket(int socket)
		{
			t_connection* c;
			t_elem const* curr;

			LIST_TRAVERSE_CONST(conn_head, curr)
			{
				c = (t_connection*)elem_get_data(curr);
				if (c->socket.tcp_sock == socket)
					return c;
			}

			return NULL;
		}


		extern t_connection* connlist_find_connection_by_name(char const* name, t_realm* realm)
		{
			char         charname[MAX_CHARNAME_LEN];
			char const* temp;

			if (!name)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL name");
				return NULL;
			}
			if (name[0] == '\0')
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got empty name");
				return NULL;
			}

			/* format: *username */
			if (name[0] == '*')
			{
				name++;
				return connlist_find_connection_by_accountname(name);
			}

			/* If is charname@otherrealm or ch@rname@realm */
			if ((temp = std::strrchr(name, '@'))) /* search from the right */
			{
				unsigned int n;

				n = temp - name;
				if (n >= MAX_CHARNAME_LEN)
				{
					eventlog(eventlog_level_info, __FUNCTION__, "character name too long in \"{}\" (charname@otherrealm format)", name);
					return NULL;
				}
				std::strncpy(charname, name, n);
				charname[n] = '\0';
				return connlist_find_connection_by_charname(name, temp + 1);
			}

			/* format: charname*username */
			if ((temp = std::strchr(name, '*')))
			{
				unsigned int n;

				n = temp - name;
				if (n >= MAX_CHARNAME_LEN)
				{
					eventlog(eventlog_level_info, __FUNCTION__, "character name too long in \"{}\" (charname*username format)", name);
					return NULL;
				}
				name = temp + 1;
				return connlist_find_connection_by_accountname(name);
			}

			/* format: charname (realm must be not NULL) */
			if (realm)
				return connlist_find_connection_by_charname(name, realm_get_name(realm));

			/* format: Simple username, clients with no realm, like starcraft or d2 open,
			 * the format is the same of charname but is matched if realmname is NULL */
			return connlist_find_connection_by_accountname(name);
		}


		extern t_connection* connlist_find_connection_by_charname(char const* charname, char const* realmname)
		{
			t_connection* c;
			t_elem const* curr;

			if (!realmname) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL realmname");
				return NULL;
			}
			LIST_TRAVERSE_CONST(conn_head, curr)
			{
				c = (t_connection*)elem_get_data(curr);
				if (!c)
					continue;
				if (!c->protocol.d2.charname)
					continue;
				if (!c->protocol.d2.realm)
					continue;
				if ((strcasecmp(c->protocol.d2.charname, charname) == 0) && (strcasecmp(realm_get_name(c->protocol.d2.realm), realmname) == 0))
					return c;
			}
			return NULL;
		}


		extern t_connection* connlist_find_connection_by_uid(unsigned int uid)
		{
			t_account* temp;

			if (!(temp = accountlist_find_account_by_uid(uid)))
			{
				return NULL;
			}
			return account_get_conn(temp);
		}

		extern int connlist_get_length(void)
		{
			return list_get_length(conn_head);
		}


		extern unsigned int connlist_login_get_length(void)
		{
			t_connection const* c;
			unsigned int         count;
			t_elem const* curr;

			count = 0;
			LIST_TRAVERSE_CONST(conn_head, curr)
			{
				c = (const t_connection*)elem_get_data(curr);
				if ((c->protocol.state == conn_state_loggedin) &&
					((c->protocol.cclass == conn_class_bnet) || (c->protocol.cclass == conn_class_bot) || (c->protocol.cclass == conn_class_telnet)
						|| (c->protocol.cclass == conn_class_irc) || (c->protocol.cclass == conn_class_wol)))
					count++;
			}

			return count;
		}


		extern int connlist_total_logins(void)
		{
			return totalcount;
		}


		extern unsigned int connlist_count_connections(unsigned int addr)
		{
			t_connection* c;
			t_elem const* curr;
			unsigned int count;

			count = 0;

			LIST_TRAVERSE_CONST(conn_head, curr)
			{
				c = (t_connection*)elem_get_data(curr);
				if (c->socket.tcp_addr == addr)
					count++;
			}

			return count;
		}

		extern int conn_get_passfail_count(t_connection* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, "conn_get_passfail_count", "got NULL connection");
				return -1;
			}
			return c->protocol.passfail_count;
		}


		extern int conn_set_passfail_count(t_connection* c, unsigned int n)
		{
			if (c == NULL)
			{
				eventlog(eventlog_level_error, "conn_set_passfail_count", "got NULL connection");
				return -1;
			}
			c->protocol.passfail_count = n;
			return 0;
		}


		extern int conn_increment_passfail_count(t_connection* c)
		{
			unsigned int count;

			if (prefs_get_passfail_count() > 0)
			{
				count = conn_get_passfail_count(c) + 1;
				if (count == prefs_get_passfail_count())
				{
					ipbanlist_add(NULL, addr_num_to_ip_str(conn_get_addr(c)), now + (std::time_t)prefs_get_passfail_bantime());
					eventlog(eventlog_level_info, __FUNCTION__, "[{}] failed password tries: {} (banned ip)", conn_get_socket(c), count);
					conn_set_state(c, conn_state_destroy);
					return -1;
				}
				else conn_set_passfail_count(c, count);
			}
			return 0;
		}


		extern char const* conn_get_client_proof(t_connection* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL conn");
				return NULL;
			}

			return c->protocol.w3.client_proof;
		}

		extern int conn_set_client_proof(t_connection* c, char const* client_proof)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL conn");
				return -1;
			}

			if (c->protocol.w3.client_proof) {
				xfree((void*)c->protocol.w3.client_proof);
				c->protocol.w3.client_proof = NULL;
			}

			if (client_proof != NULL) {
				char* proof = (char*)xmalloc(20);
				std::memcpy(proof, client_proof, 20);
				c->protocol.w3.client_proof = proof;
			}
			return 0;
		}


		extern char const* conn_get_server_proof(t_connection* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL conn");
				return NULL;
			}

			return c->protocol.w3.server_proof;
		}

		extern int conn_set_server_proof(t_connection* c, char const* server_proof)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL conn");
				return -1;
			}

			if (c->protocol.w3.server_proof) {
				xfree((void*)c->protocol.w3.server_proof);
				c->protocol.w3.server_proof = NULL;
			}

			if (server_proof != NULL) {
				char* proof = (char*)xmalloc(20);
				std::memcpy(proof, server_proof, 20);
				c->protocol.w3.server_proof = proof;
			}
			return 0;
		}


		extern int conn_set_tmpOP_channel(t_connection* c, char const* tmpOP_channel)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL conn");
				return -1;
			}

			if (c->protocol.chat.tmpOP_channel)
			{
				xfree((void*)c->protocol.chat.tmpOP_channel);
				c->protocol.chat.tmpOP_channel = NULL;
			}

			if (tmpOP_channel)
				c->protocol.chat.tmpOP_channel = xstrdup(tmpOP_channel);

			return 0;
		}

		extern char const* conn_get_tmpOP_channel(t_connection* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL conn");
				return NULL;
			}

			return c->protocol.chat.tmpOP_channel;
		}

		extern int conn_set_tmpVOICE_channel(t_connection* c, char const* tmpVOICE_channel)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL conn");
				return -1;
			}

			if (c->protocol.chat.tmpVOICE_channel)
			{
				xfree((void*)c->protocol.chat.tmpVOICE_channel);
				c->protocol.chat.tmpVOICE_channel = NULL;
			}

			if (tmpVOICE_channel)
				c->protocol.chat.tmpVOICE_channel = xstrdup(tmpVOICE_channel);

			return 0;
		}

		extern char const* conn_get_tmpVOICE_channel(t_connection* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL conn");
				return NULL;
			}

			return c->protocol.chat.tmpVOICE_channel;
		}

		static int connarray_create(void)
		{
			unsigned i;
			t_conn_entry* curr;

			if (connarray) connarray_destroy();
			connarray = (t_conn_entry*)xmalloc(sizeof(t_conn_entry) * fdw_maxcons);

			elist_init(&arrayflist);
			/* put all elements as free */
			for (i = 0, curr = connarray; i < fdw_maxcons; i++, curr++) {
				elist_add_tail(&arrayflist, &curr->freelist);
				curr->c = NULL;
			}

			return 0;
		}


		static void connarray_destroy(void)
		{
			if (connarray) xfree((void*)connarray);
			connarray = NULL;
		}

		static t_connection* connarray_get_conn(unsigned index)
		{
			if (index >= fdw_maxcons) return NULL;
			return connarray[index].c;
		}

		static unsigned connarray_add_conn(t_connection* c)
		{
			t_conn_entry* curr;

			assert(c);
			assert(!elist_empty(&arrayflist));

			curr = elist_entry(elist_next(&arrayflist), t_conn_entry, freelist);
			assert(curr->c == NULL);	/* it should never be free and != NULL */
			curr->c = c;
			elist_del(&curr->freelist);
			return (curr - connarray);	/* return the array index */
		}

		static void connarray_del_conn(unsigned index)
		{
			t_conn_entry* curr;

			if (index >= fdw_maxcons) return;
			curr = connarray + index;
			curr->c = NULL;
			elist_add_tail(&arrayflist, &curr->freelist);
		}

		extern int conn_is_irc_variant(t_connection* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "get NULL conn");
				return -1;
			}

			if ((c->protocol.cclass == conn_class_irc) ||
				(c->protocol.cclass == conn_class_wol) ||
				(c->protocol.cclass == conn_class_wserv) ||
				(c->protocol.cclass == conn_class_wladder)
				)
				return 1;

			return 0;
		}

		/**
		*  Westwood Online Extensions
		*/
		extern int conn_get_wol(t_connection* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "get NULL conn");
				return -1;
			}

			if (c->protocol.cclass == conn_class_wol)
				return 1;

			return 0;
		}

		extern void conn_wol_set_apgar(t_connection* c, char const* apgar)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return;
			}
			if (!apgar)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL WOL apgar");
				return;
			}

			if (c->protocol.wol.apgar)
				xfree((void*)c->protocol.wol.apgar); /* avoid warning */
			c->protocol.wol.apgar = xstrdup(apgar);
		}

		extern char const* conn_wol_get_apgar(t_connection* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL conn");
				return NULL;
			}

			return c->protocol.wol.apgar;
		}

		extern void conn_wol_set_codepage(t_connection* c, int codepage)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL conn");
				return;
			}

			c->protocol.wol.codepage = codepage;
		}

		extern int conn_wol_get_codepage(t_connection* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL conn");
				return -1;
			}

			return c->protocol.wol.codepage;
		}

		extern void conn_wol_set_findme(t_connection* c, bool findme)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL conn");
				return;
			}

			c->protocol.wol.findme = findme;
		}

		extern bool conn_wol_get_findme(t_connection* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL conn");
				return false;
			}

			bool bRet;
			if (c->protocol.wol.findme == 0)
				bRet = false;
			else
				bRet = true;

			return bRet;
		}

		extern void conn_wol_set_pageme(t_connection* c, bool pageme)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL conn");
				return;
			}

			c->protocol.wol.pageme = pageme;
		}

		extern bool conn_wol_get_pageme(t_connection* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL conn");
				return false;
			}

			bool bRet;
			if (c->protocol.wol.pageme == 0)
				bRet = false;
			else
				bRet = true;

			return bRet;
		}

		extern void conn_wol_set_anongame_player(t_connection* c, t_anongame_wol_player* anongame_player)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL conn");
				return;
			}

			if (!anongame_player)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL anongame_player");
				return;
			}

			c->protocol.wol.anongame_player = anongame_player;
		}

		extern t_anongame_wol_player* conn_wol_get_anongame_player(t_connection* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL conn");
				return NULL;
			}

			return c->protocol.wol.anongame_player;
		}



		extern int conn_client_readmemory(t_connection* c, unsigned int request_id, unsigned int offset, unsigned int length)
		{
			t_packet* rpacket;
			t_clienttag clienttag;

			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL conn");
				return -1;
			}
			clienttag = conn_get_clienttag(c);

			// disallow clients that doesn't support SID_READMEMORY
			if (clienttag != CLIENTTAG_STARCRAFT_UINT && clienttag != CLIENTTAG_BROODWARS_UINT && clienttag != CLIENTTAG_STARJAPAN_UINT && clienttag != CLIENTTAG_SHAREWARE_UINT &&
				clienttag != CLIENTTAG_DIABLORTL_UINT && clienttag != CLIENTTAG_DIABLOSHR_UINT && clienttag != CLIENTTAG_WARCIIBNE_UINT)
			{
				return -1;
			}
			if (!(rpacket = packet_create(packet_class_bnet)))
				return -1;

			packet_set_size(rpacket, sizeof(t_server_readmemory));
			packet_set_type(rpacket, SERVER_READMEMORY);

			bn_int_set(&rpacket->u.server_readmemory.request_id, request_id);
			bn_int_set(&rpacket->u.server_readmemory.address, offset);
			bn_int_set(&rpacket->u.server_readmemory.length, length);

			conn_push_outqueue(c, rpacket);
			packet_del_ref(rpacket);

			return 0;
		}

		extern int conn_client_requiredwork(t_connection* c, const char* filename)
		{
			t_packet* rpacket;

			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL conn");
				return -1;
			}
			if (!(rpacket = packet_create(packet_class_bnet)))
				return -1;

			packet_set_size(rpacket, sizeof(t_server_requiredwork));
			packet_set_type(rpacket, SERVER_REQUIREDWORK);
			packet_append_string(rpacket, filename); // filename should be "IX86ExtraWork.mpq"

			conn_push_outqueue(c, rpacket);
			packet_del_ref(rpacket);

			return 0;
		}


		//start
		extern int conn_send_ah_packet(t_connection* c, uint32_t command, uint32_t var1, uint32_t var2, uint32_t var3, uint32_t var4, std::string strvar, std::string strvar2, std::string strvar3, std::string strvar4)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL conn");
				return 0;
			}
			//eventlog( eventlog_level_error, __FUNCTION__, "." );

			t_account* acc = c->protocol.account;

			if (acc)
			{
				if (c->protocol.IamBOT /*|| account_get_auth_botlogin(acc)*/)
				{
					return 0;
				}

				eventlog(eventlog_level_trace, __FUNCTION__, " acc {} -> {} ", account_get_name(acc), command);

			}


			if (acc && !c->protocol.found_cheat && !c->protocol.found_cheat2)
			{
				int timeoutreason = 0;
				if (conn_ah_timeout(c, &timeoutreason))
				{
					account_add_newevent(conn_get_account(c), 8);
					eventlog(eventlog_level_error, __FUNCTION__, "[ANTIHACK] baban user [{}]. Reason: Timeout.[STANDART]:[{}]", account_get_name(acc), timeoutreason);
					//account_set_auth_lock( c->protocol.account, 1 );
					//account_set_auth_lockreason( c->protocol.account, "Antihack" );
					conn_send_ah_gamemessagebox(c, 1, "|c0000FF40Test Protection:|r|n |c00FF8000Ban with reason:|r |c00FF0000Timeout.[STANDART]|r");
					conn_send_bot_command(std::string("!superkick ") + account_get_name(acc) + std::string(" locked by AH. Reason: timeout"));
					/*if (c->protocol.game)
					{
						conn_set_game(c, 0, 0, 0, t_game_type::game_type_none, 0);

					}*/
					if (c->protocol.found_cheat == 0)
						c->protocol.found_cheat = 1;
					if (c->protocol.found_cheat2 == 0)
						c->protocol.found_cheat2 = 1;

					//account_ban
					c->protocol.account = NULL;
					return -1;
				}
			}

			/*	else
					return 0;
	*/

			t_packet* rpacket;

			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL conn");
				return -1;
			}
			if (!(rpacket = packet_create(packet_class_bnet)))
				return -1;

			packet_set_size(rpacket, sizeof(t_server_ah_scan_request));
			packet_set_type(rpacket, SERVER_AH_SCAN_REQUEST);

			bn_int_set(&rpacket->u.server_ah_scan_request.ah_version, ANTIHACK_VERSION);
			bn_int_set(&rpacket->u.server_ah_scan_request.command, command);

			switch (command)
			{
			case 2:
				bn_int_set(&rpacket->u.server_ah_scan_request.val1, var1 & 0xFF000000);
				bn_int_set(&rpacket->u.server_ah_scan_request.val2, var2 & 0x00FF0000);
				bn_int_set(&rpacket->u.server_ah_scan_request.val3, var3 & 0x0000FF00);
				bn_int_set(&rpacket->u.server_ah_scan_request.val4, var4 & 0x000000FF);
				break;
			default:
				bn_int_set(&rpacket->u.server_ah_scan_request.val1, var1);
				bn_int_set(&rpacket->u.server_ah_scan_request.val2, var2);
				bn_int_set(&rpacket->u.server_ah_scan_request.val3, var3);
				bn_int_set(&rpacket->u.server_ah_scan_request.val4, var4);
				break;
			}

			packet_append_string(rpacket, strvar.c_str());
			packet_append_string(rpacket, strvar2.c_str());
			packet_append_string(rpacket, strvar3.c_str());
			packet_append_string(rpacket, strvar4.c_str());

			conn_push_outqueue(c, rpacket);
			packet_del_ref(rpacket);

			return 0;
		}


		extern int conn_send_ah_gamestate(t_connection* c, const char* realusername, const char* newusername, const char* statsstring1
			, const char* statsstring2, const char* statsstring3, unsigned int var1, unsigned int var2, unsigned int var3, unsigned int var4)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL conn");
				return 0;
			}

			eventlog(eventlog_level_error, __FUNCTION__, ".");

			t_account* acc = c->protocol.account;

			if (acc)
			{
				eventlog(eventlog_level_error, __FUNCTION__, " acc {} ", account_get_name(acc));

				if (c->protocol.IamBOT /*|| account_get_auth_botlogin(acc)*/)
					return 0;
			}
			else
				return 0;
			t_packet* rpacket;

			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL conn");
				return -1;
			}

			if (!(rpacket = packet_create(packet_class_bnet)))
				return -1;

			packet_set_size(rpacket, sizeof(t_server_ah_scan_request));
			packet_set_type(rpacket, SERVER_AH_SCAN_REQUEST);

			bn_int_set(&rpacket->u.server_ah_scan_request.ah_version, ANTIHACK_VERSION);
			bn_int_set(&rpacket->u.server_ah_scan_request.command, 0xFF88);

			bn_int_set(&rpacket->u.server_ah_scan_request.val1, var1);
			bn_int_set(&rpacket->u.server_ah_scan_request.val2, var2);
			bn_int_set(&rpacket->u.server_ah_scan_request.val3, var3);
			bn_int_set(&rpacket->u.server_ah_scan_request.val4, var4);

			packet_append_string(rpacket, realusername);
			packet_append_string(rpacket, newusername);
			packet_append_string(rpacket, statsstring1);
			packet_append_string(rpacket, statsstring2);
			packet_append_string(rpacket, statsstring3);

			conn_push_outqueue(c, rpacket);
			packet_del_ref(rpacket);

			return 0;
		}


		extern int conn_send_ah_newmap(t_connection* c, MapHostStruct* map)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "SEND GAME ERR0");
				return 0;
			}

			//eventlog( eventlog_level_error, __FUNCTION__, "-------" );

			t_account* acc = c->protocol.account;

			if (acc)
			{

				if (c->protocol.IamBOT/* || account_get_auth_botlogin(acc)*/)
				{
					//eventlog( eventlog_level_error, __FUNCTION__, "SEND GAME ERR1" );
					return 0;
				}
			}
			/*	else
					return 0;*/
			t_packet* rpacket;

			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "SEND GAME ERR2");
				return -1;
			}

			if (!(rpacket = packet_create(packet_class_bnet)))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "SEND GAME ERR3");
				return -1;
			}

			packet_set_size(rpacket, sizeof(t_server_ah_scan_request));
			packet_set_type(rpacket, SERVER_AH_SCAN_REQUEST);

			bn_int_set(&rpacket->u.server_ah_scan_request.ah_version, ANTIHACK_VERSION);
			bn_int_set(&rpacket->u.server_ah_scan_request.command, 0x3F28);

			bn_int_set(&rpacket->u.server_ah_scan_request.val1, map->crc32);
			bn_int_set(&rpacket->u.server_ah_scan_request.val2, map->HostModes.size());
			bn_int_set(&rpacket->u.server_ah_scan_request.val3, map->PlayersSettings.size());
			bn_int_set(&rpacket->u.server_ah_scan_request.val4, map->stats ? 1 : 0);

			packet_append_string(rpacket, map->MapName.c_str());
			packet_append_string(rpacket, map->MapCode.c_str());
			packet_append_string(rpacket, map->MapLocalPath.c_str());
			packet_append_string(rpacket, map->Category.c_str());

			for (auto s : map->HostModes)
			{
				packet_append_string(rpacket, s.Mode.c_str());
			}

			for (auto s : map->PlayersSettings)
			{
				packet_append_string(rpacket, s.Players.c_str());
			}

			conn_push_outqueue(c, rpacket);
			packet_del_ref(rpacket);

			eventlog( eventlog_level_error, __FUNCTION__, "SEND GAME MAP  {} -> {} -> {} -> {}", map->MapName.c_str( ), map->MapCode.c_str( ) , map->MapLocalPath.c_str( ), map->Category.c_str( ) );

			return 0;
		}


		extern int conn_send_ah_followstate(t_connection* c, int followstate, const char* gamename)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL conn");
				return 0;
			}
			eventlog(eventlog_level_error, __FUNCTION__, ".");

			t_account* acc = c->protocol.account;
			if (acc)
			{
				eventlog(eventlog_level_error, __FUNCTION__, " acc {} ", account_get_name(acc));

				if (c->protocol.IamBOT/* || account_get_auth_botlogin(acc)*/)
				{
					eventlog(eventlog_level_error, __FUNCTION__, " in white list ", account_get_name(acc));
					return 0;
				}
				else
				{
					eventlog(eventlog_level_error, __FUNCTION__, " ok debug", account_get_name(acc));
				}
			}
			else
				return 0;

			t_packet* rpacket;

			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL conn");
				return -1;
			}

			if (!(rpacket = packet_create(packet_class_bnet)))
				return -1;

			packet_set_size(rpacket, sizeof(t_server_ah_scan_request));
			packet_set_type(rpacket, SERVER_AH_SCAN_REQUEST);

			bn_int_set(&rpacket->u.server_ah_scan_request.ah_version, ANTIHACK_VERSION);
			bn_int_set(&rpacket->u.server_ah_scan_request.command, 0xEE77);

			bn_int_set(&rpacket->u.server_ah_scan_request.val1, followstate);
			bn_int_set(&rpacket->u.server_ah_scan_request.val2, 0);
			bn_int_set(&rpacket->u.server_ah_scan_request.val3, 0);
			bn_int_set(&rpacket->u.server_ah_scan_request.val4, 0);

			packet_append_string(rpacket, (gamename && gamename[0] != '\0') ? gamename : "/");

			conn_push_outqueue(c, rpacket);
			packet_del_ref(rpacket);
			eventlog(eventlog_level_error, __FUNCTION__, " acc {} ok flw", account_get_name(acc));
			return 0;
		}


		extern int conn_send_ah_gamemessagebox(t_connection* c, int messagetype, const char* message, int sleeptime)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL conn");
				return 0;
			}
			eventlog(eventlog_level_trace, __FUNCTION__, ".");

			t_account* acc = c->protocol.account;
			if (acc)
			{
				eventlog(eventlog_level_trace, __FUNCTION__, " acc {} ", account_get_name(acc));

				if (c->protocol.IamBOT/* || account_get_auth_botlogin(acc)*/)
				{
					eventlog(eventlog_level_trace, __FUNCTION__, " in white list ", account_get_name(acc));
					return 0;
				}
				else
				{
					eventlog(eventlog_level_trace, __FUNCTION__, " ok debug", account_get_name(acc));
				}
			}
			/*	else
					return 0;*/

			t_packet* rpacket;

			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL conn");
				return -1;
			}

			if (!(rpacket = packet_create(packet_class_bnet)))
				return -1;

			packet_set_size(rpacket, sizeof(t_server_ah_scan_request));
			packet_set_type(rpacket, SERVER_AH_SCAN_REQUEST);

			bn_int_set(&rpacket->u.server_ah_scan_request.ah_version, ANTIHACK_VERSION);
			bn_int_set(&rpacket->u.server_ah_scan_request.command, 0x3785);

			bn_int_set(&rpacket->u.server_ah_scan_request.val1, messagetype);
			bn_int_set(&rpacket->u.server_ah_scan_request.val2, sleeptime);
			bn_int_set(&rpacket->u.server_ah_scan_request.val3, 0);
			bn_int_set(&rpacket->u.server_ah_scan_request.val4, 0);

			packet_append_string(rpacket, message);

			conn_push_outqueue(c, rpacket);
			packet_del_ref(rpacket);
			//eventlog( eventlog_level_error, __FUNCTION__, " acc {} ok messagebox {}", account_get_name( acc ), message );
			return 0;
		}



		extern bool conn_ah_timeout(t_connection* c, int* reason)
		{
			bool retval = false;


			std::time_t now;

			if (reason)
				*reason = 0;

			//if (!c->protocol.account)
			//	return retval;
			if (c->protocol.state == conn_state_destroy)
				return retval;
			if (c->protocol.IamBOT /*|| account_get_auth_botlogin(c->protocol.account)*/)
				return retval;


			std::time(&now);

			if (c->protocol.lastahtime == 0)
				c->protocol.lastahtime = now;
			if (c->protocol.lastahtime2 == 0)
				c->protocol.lastahtime2 = now;
			if (c->protocol.lastahtime3 == 0)
				c->protocol.lastahtime3 = now;
			if (c->protocol.lastahtime4 == 0)
				c->protocol.lastahtime4 = now;

			/*if ( c->protocol.lastahtime + 32 < now )
			{
				retval = true;
				if ( reason )
					*reason += 2;
			}*/
			if (c->protocol.lastahtime2 + 60 < now)
			{
				retval = true;
				if (reason)
					*reason += 4;
			}
			if (c->protocol.lastahtime3 + 60 < now)
			{
				retval = true;
				if (reason)
					*reason += 8;
			}
			
			/*if ( c->protocol.lastahtime4 + 32 < now )
			{
				retval = true;
				if ( reason )
					*reason += 16;
			}*/
			//if (retval)
			//	eventlog(eventlog_level_error, __FUNCTION__, " acc {} need timeout! conn state:{}", account_get_name(c->protocol.account), c->protocol.state);
			return retval;
		}

		bool IsInMapWhiteListCrc32(uint32_t crc32)
		{
			for (auto& maphoststr : MapHostStructList)
			{
				if (crc32 == maphoststr.crc32)
					return true;
			}
			return false;
		}


		extern int conn_send_voicepacket(t_connection* sender, t_connection* dst, uint32_t frameidx, uint32_t maxframeidx, uint32_t len, const void* voicedata)
		{
			if (!dst)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL conn");
				return 0;
			}

			if (!sender)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL conn");
				return 0;
			}
			//eventlog( eventlog_level_error, __FUNCTION__, "." );

			t_account* acc = sender->protocol.account;
			const char* playername = "";
			if (acc)
			{
				playername = account_get_name(acc);
				eventlog(eventlog_level_error, __FUNCTION__, " acc {} ", playername);
				/*
				if ( IsHostBot( playername ) )
				{
					eventlog( eventlog_level_error, __FUNCTION__, " in white list ", account_get_name( acc ) );
					return 0;
				}
				else
				{
					eventlog( eventlog_level_error, __FUNCTION__, " ok debug", account_get_name( acc ) );
				}*/
			}
			else
				return 0;


			t_packet* rpacket;
			if (!(rpacket = packet_create(packet_class_bnet)))
				return -1;

			packet_set_size(rpacket, sizeof(t_server_ah_scan_request));
			packet_set_type(rpacket, SERVER_AH_SCAN_REQUEST);

			bn_int_set(&rpacket->u.server_ah_scan_request.ah_version, ANTIHACK_VERSION);
			bn_int_set(&rpacket->u.server_ah_scan_request.command, 0xC0CE0);

			bn_int_set(&rpacket->u.server_ah_scan_request.val1, frameidx);
			bn_int_set(&rpacket->u.server_ah_scan_request.val2, maxframeidx);
			bn_int_set(&rpacket->u.server_ah_scan_request.val3, len);
			bn_int_set(&rpacket->u.server_ah_scan_request.val4, 0);

			packet_append_string(rpacket, playername);
			packet_append_data(rpacket, voicedata, len);

			conn_push_outqueue(dst, rpacket);
			packet_del_ref(rpacket);
			//eventlog( eventlog_level_error, __FUNCTION__, " acc {} ", playername );
			return 0;
		}

		extern int conn_client_voice_response(t_connection* c, uint32_t frameidx, uint32_t maxframeidx, uint32_t len, const void* voicedata)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL conn");
				return 0;
			}





			t_account* acc = c->protocol.account;

			if (!acc)
				return 0;

			/*	eventlog( eventlog_level_error, __FUNCTION__, "Sender : {}", account_get_name(acc) );
	*/
			t_game* game = c->protocol.game;
			if (game)
			{
				unsigned int count = game_get_count(game);

				for (int i = 0; i < count; i++)
				{
					if (t_connection* user = game_get_player_conn(game, i))
					{
						/*if ( user->protocol.account )
						{
							eventlog( eventlog_level_error, __FUNCTION__, "Receiver : {}", account_get_name( user->protocol.account ) );
						}*/
						if (!user->protocol.IamBOT && user != c)
						{
							/*if ( user->protocol.account )
							{
								eventlog( eventlog_level_error, __FUNCTION__, "Send!" );
							}*/
							conn_send_voicepacket(c, user, frameidx, maxframeidx, len, voicedata);
						}
					}
				}
			}

			return 0;
		}




		extern int conn_client_ah_scan_response(t_connection* c, uint32_t command, uint32_t ahversion, uint32_t var1, uint32_t var2, uint32_t var3, uint32_t var4, std::string strvar, std::string strvar2, std::string strvar3, std::string strvar4)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "[{}] AH PACKET RECIEVED: {} ", conn_get_socket(c), command);

			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL conn");
				return 0;
			}

			t_account* checkifwhite = c->protocol.account;
			if (checkifwhite && c->protocol.IamBOT)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "bot");
				return 0;
			}

			/*	if (checkifwhite && account_get_auth_botlogin(checkifwhite))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "bot2");
					return 0;
				}*/

			
			int MagicValue = c->protocol.ah_magic_value;

			std::string CheatNames = "Cheats:";
			std::string CheatNamesReason = "";



			if (command == 0x101505)
			{
				c->protocol.IsNewClient = true;
			}

			if (command == 0x101506)
			{
				if (c->protocol.IsNewClient)
					conn_send_newgamelist(c);
			}

			if (command == 0x101507)
			{
				if (c->protocol.IsNewClient)
					JoinToGame(c, strvar);
			}

			if (command == 0x101508)
			{
				if (c->protocol.IsNewClient)
					LeaveFromGame(c);
			}

			if (command == 0x101509)
			{
				if (c->protocol.IsNewClient)
					DestroyTheGame(c);
			}

			if (command == 0x101510)
			{
				if (c->protocol.IsNewClient)
					StartAlternativeGame(c);
			}

			if (command == 0x101511)
			{
				if (c->protocol.IsNewClient)
					UpdateHostGame(c);
			}

			if (command == 0x101512)
			{
				if (c->protocol.IsNewClient)
					JoinToSlot(c, var1);
			}

			if (command == 0x101513)
			{
				if (c->protocol.IsNewClient)
				{
					ChangeSlot(c, var1, var2);
				}
			}

			if (command == 0x251512)
			{
				if (c->protocol.IsNewClient)
					UpdateSlot(c, var1, var2, var3);
			}

			if (command == 0x32140556)
			{
				if (c->protocol.IsNewClient)
					SendLobbyChat(c, strvar);
			}



			eventlog(eventlog_level_trace, __FUNCTION__, "[{}] AH PACKET PROCESSED", conn_get_socket(c));

			if (command == 0xABCD && var1 == 0xA && var2 == 0xB)
			{
				eventlog(eventlog_level_trace, __FUNCTION__, "Set hash! Hash:{} ", conn_get_socket(c), strvar2.c_str());

				conn_set_plainpassword(c, strvar.c_str());
				conn_set_hash(c, strvar2.c_str());
			}

			if (command == 0xEEBCD1 && strvar3.size() > 3)
			{
				eventlog(eventlog_level_trace, __FUNCTION__, "Create Account {} ! Hash:{} ", strvar.c_str(), strvar2.c_str());

				conn_set_plainpassword(c, strvar3.c_str());
				conn_set_hash(c, strvar2.c_str());

				t_account* account = accountlist_create_account(strvar.c_str(), strvar2.c_str());
				if (!account || conn_get_account(c))
				{
					conn_send_ah_packet(c, 0xEEBCD2, 1);
					if (c->protocol.game)
					{
						game_del_player(c->protocol.game, c);
					}
					conn_set_state(c, conn_state_destroy);
					conn_set_account(c, NULL);
				}
				else
				{
					conn_send_ah_packet(c, 0xEEBCD3);
				}
			}
			else if (command == 0xEEBCD1 && strvar3.size() <= 3)
			{
				conn_send_ah_packet(c, 0xEEBCD2, 2);
			}

			if (command == 0xBCDE)
			{
				c->protocol.LobbyNicknameColor = var1;
				c->protocol.ChatNicknameColor = var2;
				c->protocol.ChatTextColor = var3;
			}

			if (command == 0x1934)
			{
				UpdatePlayerInQueue(c, ToLower(strvar), ToLower(strvar2), account_map_get_stats(c->protocol.account, strvar, "mmr"));
			}

			if (command == 0x1935)
			{
				RemovePlayerFromQueue(c);
			}

			if (command == 0x1020)
			{
				// fixme
				if (c->protocol.account)
				{
					// fixme autosearch magic value!!! (SWORD?!)
					if (var1 == 6666)
					{
						t_game* autosearchgame = game_search_for_player(account_map_get_stats(c->protocol.account, strvar, "mmr"), strvar);

						if (autosearchgame)
							conn_send_ah_followstate(c, Watch::ET_joingame, game_get_name(autosearchgame));
					}
					else
					{
						std::string hostcmd = "/host";
						if (var1 != 666)
						{
							if (var1 == 0)
								hostcmd = "/host ";
							else if (var1 == 1)
								hostcmd = "/chost ";
							else
								hostcmd = "/phost ";

							// host strvar=dota strvar2=5x5 strvar3=aptb strvar4 = name
							handle_command(c, (hostcmd + strvar + " " + strvar2 + " " + strvar3 + " " + strvar4).c_str());
							eventlog(eventlog_level_error, __FUNCTION__, "HOST CMD: [{}:{}:{}:{}:{}] ", hostcmd, strvar, strvar2, strvar3, strvar4);
						}
						else
						{
							c->protocol.IsNewClient = true;
							LeaveFromGame(c);
							DestroyTheGame(c);
							CreateAlternativeGame(c, strvar4, strvar, var2, var3);
						}
					}
				}
				else
				{
					eventlog(eventlog_level_error, __FUNCTION__, "NO ACCOUNT ");

				}
			}


			if (command == 1)
			{
				if (ahversion == ANTIHACK_VERSION)
				{
					c->protocol.ah_status = 1;
				}
				else
				{
					c->protocol.ah_version = ahversion;
				}

				c->protocol.hardwareid[0] = var1;
				c->protocol.hardwareid[1] = var2;
				c->protocol.hardwareid[2] = var3;
				c->protocol.hardwareid[3] = var4;

				if (var1 == var2 && var2 == var3 && var3 == var4)
				{
					c->protocol.ah_status = 2;
				}

				std::time(&c->protocol.lastahtime);
			}

			if (command == 4)
			{
				if (ahversion != ANTIHACK_VERSION)
				{
					c->protocol.ah_status = 0;
				}

				if (!IsInCrcWhiteList(std::to_string(var1).c_str()))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "[{}] AH BAD FILE, CRC32: [{}]. You can add to whitelist if need. Type:{}", conn_get_socket(c), var1, var1);

					//conn_send_ah_packet( c, 0x15, var1 );
					//0x15
				}
			}

			if (command == 2)
			{
				if (ahversion != ANTIHACK_VERSION)
				{
					c->protocol.ah_status = 0;
				}

				unsigned int hackcode = 0;

				if (var1 != MagicValue)
				{
					hackcode = hackcode | 0x1;
					CheatNames += "AntihackModule-SelfProtect,";
					if (CheatNamesReason.empty())
						CheatNamesReason = CheatNames;
				}
				if (var2 != MagicValue)
				{
					hackcode = hackcode | 0x2;
					CheatNames += "GameDll-Modification,";
					if (CheatNamesReason.empty())
						CheatNamesReason = CheatNames;
				}
				if (var3 != MagicValue)
				{
					hackcode = hackcode | 0x4;
					CheatNames += "Patched-GameDll-Code,";
					if (CheatNamesReason.empty())
						CheatNamesReason = CheatNames;
				}
				if (var4 != MagicValue)
				{
					hackcode = hackcode | 0x8;
					CheatNames += "Patched-GameDll-Constants,";
					if (CheatNamesReason.empty())
						CheatNamesReason = CheatNames;
				}

				std::time(&c->protocol.lastahtime2);

				c->protocol.found_cheat = hackcode;
			}
			if (command == 3)
			{
				if (ahversion != ANTIHACK_VERSION)
				{
					c->protocol.ah_status = 0;
				}

				unsigned int hackcode = 0;

				if (var1 != MagicValue)
				{
					hackcode = hackcode | 0x1;
					CheatNames += "AHScanner-SelfProtect,";
					if (CheatNamesReason.empty())
						CheatNamesReason = CheatNames;
				}
				//UNTESTED:
				//if ( var2 != 0 )
				//{
				//	hackcode = hackcode | 0x2;
				//}
				if (var3 != MagicValue)
				{
					//eventlog( eventlog_level_error, __FUNCTION__, "[{}] BAD COUNT: [{}]", conn_get_socket( c ), var3 );
					hackcode = hackcode | 0x4;
					CheatNames += "Detected-FakeVTable,";
					if (CheatNamesReason.empty())
						CheatNamesReason = CheatNames;
				}
				if (var4 != MagicValue)
				{
					hackcode = hackcode | 0x8;
					CheatNames += "MemoryScanner-SelfProtect,";
					if (CheatNamesReason.empty())
						CheatNamesReason = CheatNames;
				}
				std::time(&c->protocol.lastahtime3);
				c->protocol.found_cheat2 = hackcode;
			}



			if (command == 5)
			{
				if (ahversion != ANTIHACK_VERSION)
				{
					c->protocol.ah_status = 0;
				}
				if (var1 != LAUNCHER_VERSION)
				{
					c->protocol.ah_status = 3;
				}
				if (var2 != ANTIHACK_VERSION)
				{
					c->protocol.ah_status = 2;
				}

				if (!IsAMHHashOkay(var3))
				{
					c->protocol.ah_status = 4;
				}
				std::time(&c->protocol.lastahtime4);
				if (!IsAMHHashOkay(var3))
					eventlog(eventlog_level_error, __FUNCTION__, "[{}] AHVERSION RETURN CODE[{}] LAUNCHER VERSION [{}] AMH CRC32[{}]", conn_get_socket(c), var2 - ANTIHACK_VERSION, var1, var3);
			}

			if (command == 0x33)
			{
				if (ahversion != ANTIHACK_VERSION)
				{
					c->protocol.ah_status = 0;
				}

				t_game* badgamecheck = c->protocol.game;
				/*	if ( badgamecheck )
					{


						t_connection * gameowner = game_get_owner( badgamecheck );
						if ( gameowner )
						{
							t_account * account = conn_get_account( gameowner );
							if ( account && !IsHostBot( account_get_name( account ) ) )
							{
								conn_set_state( gameowner, conn_state_destroy );
							}
						}eventlog( eventlog_level_error, __FUNCTION__, "Found bad map crc32:{}", var1 );
					}*/



				bool FoundMapInWhiteList = false;

				for (auto& s : MapHostStructList)
				{
					if (var1 == s.crc32)
					{

						FoundMapInWhiteList = true;
						eventlog(eventlog_level_error, __FUNCTION__, "Bad map in white list!");

						if (!badgamecheck)
						{
							c->protocol.FoundWhiteMep = true;
						}
						else
						{
							c->protocol.FoundWhiteMep = false;
							//conn_send_ah_packet( c, 0x11 );
						}

						break;
					}
				}


				if (!FoundMapInWhiteList)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "Found bad map crc32:{}", var1);

					if (badgamecheck)
						game_del_player(badgamecheck, c);
					//conn_set_game( c, NULL, NULL, NULL, game_type_none, 0 );
					conn_set_lastbadgametime(c);
					conn_send_ah_packet(c, 0x11);
				}



			}

			if (c->protocol.found_cheat || c->protocol.found_cheat2)
			{
				if (c->protocol.account)
				{
					if (CheatNamesReason.empty())
					{
						CheatNames += "(" + std::to_string(c->protocol.found_cheat) + "," + std::to_string(c->protocol.found_cheat2) + ")";
					}
					account_add_newevent(conn_get_account(c), 8);
					eventlog(eventlog_level_error, __FUNCTION__, "[ANTIHACK] ban user [{}]. Reason: {}", conn_get_socket(c), CheatNames.c_str());
					/*account_set_auth_lock( c->protocol.account, 1 );
					account_set_auth_lockreason( c->protocol.account, CheatNamesReason.c_str( ) );*/
					conn_send_ah_gamemessagebox(c, 1, ("|c0000FF40Test Protection:|r|n |c00FF8000Ban with reason:|r |c00FF0000" + CheatNames).c_str());
					if (c->protocol.game)
					{
						game_del_player(c->protocol.game, c);
					}
					conn_set_state(c, conn_state_destroy);
					conn_set_account(c, NULL);
				}
				else
				{
					eventlog(eventlog_level_error, __FUNCTION__, "[ANTIHACK] user [{}] use cheats. Code1 {}. Code2 [ 0x{:08} --  0x{:08} --  0x{:08} --  0x{:08} ].", conn_get_socket(c), CheatNames.c_str(), var1, var2, var3, var4);
				}
			}

			eventlog(eventlog_level_trace, __FUNCTION__, "[{}] AH PACKET PROCESSED", conn_get_socket(c));


			return 0;
		}

		extern int conn_get_ah_status(t_connection* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL conn");
				return 0;
			}
			return c->protocol.ah_status;
		}

		extern int conn_get_cheats(t_connection* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL conn");
				return 0;
			}
			return c->protocol.found_cheat + c->protocol.found_cheat2 * 0x10000;
		}

		extern int conn_get_ah_version(t_connection* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL conn");
				return 0;
			}
			return c->protocol.ah_version;
		}

		extern unsigned int* conn_get_hardwareid(t_connection* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL conn");
				return NULL;
			}
			return c->protocol.hardwareid;
		}

		extern unsigned int conn_get_lobbynickcolor(t_connection* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL conn");
				return NULL;
			}
			return c->protocol.LobbyNicknameColor;
		}

		extern unsigned int conn_get_chatnickcolor(t_connection* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL conn");
				return NULL;
			}
			return c->protocol.ChatNicknameColor;
		}

		extern unsigned int conn_get_chattextcolor(t_connection* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL conn");
				return NULL;
			}
			return c->protocol.ChatTextColor;
		}

		extern unsigned int conn_get_antihack_magic_value(t_connection* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL conn");
				return 0;
			}
			return c->protocol.ah_magic_value;
		}

		extern void conn_send_gamestats_to_player(t_connection* source, t_connection* dest, std::string gametype)
		{

			if (!source || !dest)
			{
				return;
			}

			eventlog(eventlog_level_error, __FUNCTION__, ".");



			t_account* srcacc = source->protocol.account;
			t_account* dstacc = dest->protocol.account;

			if (srcacc && dstacc)
			{
				if (!dest->protocol.IamBOT/* && !account_get_auth_botlogin(dstacc)*/)
				{
					eventlog(eventlog_level_trace, __FUNCTION__, " send player state : \"{}\"", gametype);

					char sendusername[100];
					if (conn_get_lobbynickcolor(source) != 0xFFFFFFFF)
						sprintf_s(sendusername, "|c%08X%s", conn_get_lobbynickcolor(source), account_get_name(srcacc));
					else
						sprintf_s(sendusername, "%s", account_get_name(srcacc));


					if (gametype.empty() || gametype.size() <= 1)
					{
						conn_send_ah_gamestate(dest, account_get_name(srcacc), sendusername, "no_stats", "no_stats", "no_stats", conn_get_lobbynickcolor(source), conn_get_chatnickcolor(source), conn_get_chattextcolor(source));
						return;
					}

					std::string lgametype = ToLower(gametype);



					if (lgametype.find("dota") == 0)
					{

						// MMR {1}
						// WIN {2}
						// LOSE {3}
						// WINRATE {4}
						// LEAVES {5}
						// STREAK {6}
						// MAXSTREAK {7}
						// MINSTREAK {8}
						// SKYPE {9}

						std::string BuildStatsString =
							"{" + std::to_string(account_map_get_stats(srcacc, lgametype, "mmr")) + "}" +
							"{" + std::to_string(account_map_get_stats(srcacc, lgametype, "wins")) + "}" +
							"{" + std::to_string(account_map_get_stats(srcacc, lgametype, "loses")) + "}" +
							"{" + std::to_string(account_map_get_stats(srcacc, lgametype, "leaves")) + "}" +
							"{" + std::to_string(account_map_get_stats(srcacc, lgametype, "streak")) + "}" +
							"{" + std::to_string(account_map_get_stats(srcacc, lgametype, "minstreak")) + "}" +
							"{" + std::to_string(account_map_get_stats(srcacc, lgametype, "maxstreak")) + "}" +
							"{" + std::to_string(account_map_get_stats(srcacc, lgametype, "kills")) + "}" +
							"{" + std::to_string(account_map_get_stats(srcacc, lgametype, "deaths")) + "}" +
							"{" + std::to_string(account_map_get_stats(srcacc, lgametype, "assists")) + "}";


						//unsigned int randomcolor = 0x110101 + rand( ) % 0xEEFEFD;
						//std::string BuildNewName = fmt::format( "|cFF{0:X}", randomcolor );
						//BuildNewName += account_get_name( srcacc );
						//BuildNewName += "|r";



						conn_send_ah_gamestate(dest, account_get_name(srcacc), sendusername, BuildStatsString.c_str(), "no_stats", "no_stats", conn_get_lobbynickcolor(source), conn_get_chatnickcolor(source), conn_get_chattextcolor(source));
					}
					else if (lgametype == "default")
					{
						std::string BuildNewName = sendusername + fmt::format("|r ( {} \\ {} ) ", account_map_get_stats(srcacc, lgametype, "games"), account_map_get_stats(srcacc, lgametype, "leaves"));
						conn_send_ah_gamestate(dest, account_get_name(srcacc), BuildNewName.c_str(), "no_stats", "no_stats", "no_stats", conn_get_lobbynickcolor(source), conn_get_chatnickcolor(source), conn_get_chattextcolor(source));
					}
					else
					{
						conn_send_ah_gamestate(dest, account_get_name(srcacc), sendusername, "no_stats", "no_stats", "no_stats", conn_get_lobbynickcolor(source), conn_get_chatnickcolor(source), conn_get_chattextcolor(source));
					}

				}
			}
		}

		extern void conn_set_follow_account(t_connection* c, t_account* dstacc)
		{
			if (!c)
			{
				return;
			}

			if (dstacc)
				ERROR0("Set follow account");
			else
				ERROR0("Remove follow account");
			c->protocol.followaccount = dstacc;
		}



		extern t_account* conn_get_follow_account(t_connection* c)
		{
			if (!c)
			{
				return NULL;
			}


			return c->protocol.followaccount;
		}

		extern t_connection* conn_search_followed(t_account* dst)
		{
			t_elem const* curr;
			t_connection* conn;
			LIST_TRAVERSE_CONST(connlist(), curr)
			{
				conn = (t_connection*)elem_get_data(curr);
				if (conn && conn->protocol.followaccount == dst)
					return conn;
			}
			return NULL;
		}


		extern bool conn_availabled(t_connection* c)
		{
			if (!c)
				return NULL;
			t_elem const* curr;
			t_connection* conn;
			LIST_TRAVERSE_CONST(connlist(), curr)
			{
				conn = (t_connection*)elem_get_data(curr);
				if (conn = c)
					return true;
			}
			return false;
		}


		// c - бот
		extern t_connection* conn_get_botconnection(t_connection* c)
		{
			if (!c)
				return NULL;
			return c->protocol.botconnection;
		}
		extern void conn_set_botconnection(t_connection* c, t_connection* bot)
		{
			if (!c)
				return;
			c->protocol.botconnection = bot;
		}

		extern bool get_bot_availabled(t_connection* c)
		{
			if (!c)
				return NULL;

			return c->protocol.game != NULL
				&& c->protocol.availabledBot;
			//return c->protocol.availabledBot;
		}

		extern void set_bot_availabled(t_connection* c, bool available)
		{
			if (!c)
				return;
			c->protocol.availabledBot = available;
		}

		extern t_game* conn_get_hosted_game(t_connection* c)
		{
			if (!c)
				return NULL;
			return c->protocol.hostedgame;
		}

		// ¬ызвать перед удалением имени игры
		extern void RemoveHostedGameFromConnections(t_game* game, bool showmsg)
		{
			if (!game)
				return;
			t_elem const* curr;
			t_connection* conn;
			LIST_TRAVERSE_CONST(connlist(), curr)
			{
				conn = (t_connection*)elem_get_data(curr);
				if (conn->protocol.hostedgame == game)
				{
					if (showmsg)
					{
						if (conn->protocol.botconnection)
							message_send_text(conn, message_type_info, conn, localize(conn, "Your hosted game {} is end!",
								game_get_name(game) ? game_get_name(game) : ""));
						else
							message_send_text(conn, message_type_info, conn, localize(conn, "Your unknown hosted gameis end!"));
					}
					conn->protocol.hostedgame = NULL;
				}
			}
		}

		// ¬ызвать дл€ удалени€ бота, провер€ть совсем не об€зательно, т.к действие происходит только при удалении соединени€
		extern void RemoveBotConnectionFromConnections(t_connection* connection)
		{
			if (!connection)
				return;
			t_elem const* curr;
			t_connection* conn;
			LIST_TRAVERSE_CONST(connlist(), curr)
			{
				conn = (t_connection*)elem_get_data(curr);
				if (conn->protocol.botconnection == connection && connection != NULL)
				{
					message_send_text(conn, message_type_info, conn, localize(conn, "Your selected bot - leave server."));
					if (conn_get_game(conn) && (conn->protocol.hostedgame == conn_get_game(conn->protocol.botconnection) && conn->protocol.hostedgame == conn_get_game(conn) && conn_get_game(conn) != NULL) || (conn_get_game(conn) == conn_get_game(conn->protocol.botconnection) && conn_get_game(conn) != NULL))
					{
						game_del_player(conn_get_game(conn), conn);
					}
					if (conn_get_hosted_game(conn))
					{
						conn->protocol.hostedgame = NULL;
					}
					conn->protocol.botconnection = NULL;
				}
			}
		}

		std::time_t lastahscan = std::time(NULL);
		std::time_t lastingametimeupdate = std::time(NULL);

		extern void UpdatePlayersTick()//no bot!
		{
			UpdateAlternativeGames();

			bool NeedSetScanPacket = false;
			bool NeedUpdateGameTime = false;
			std::time_t now;
			std::time(&now);
			if (lastahscan + 10 < now)
			{
				lastahscan = now;
				NeedSetScanPacket = true;
			}

			if (lastingametimeupdate + 60 < now)
			{
				lastingametimeupdate = now;
				NeedUpdateGameTime = true;
			}

			if (NeedReloadMapList)
			{
				NeedReloadMapList = false;

				// IP PORT USERNAME PASSWORD
				ReloadMapHostList(0);
				eventlog(eventlog_level_error, __FUNCTION__, "Ok. Start printmaplist.");
				PrintMapList(0);
				eventlog(eventlog_level_error, __FUNCTION__, "Ok.");

			}

			t_elem const* curr;
			t_connection* conn;
			LIST_TRAVERSE_CONST(connlist(), curr)
			{
				conn = (t_connection*)elem_get_data(curr);
				if (conn->protocol.IamBOT)
				{
					continue;
				}

				if (conn_get_state(conn) == conn_state_destroy)
				{
					continue;
				}

				if (NeedSetScanPacket)
				{
					conn_send_ah_packet(conn, 1);
				}


				t_account* connacc = conn->protocol.account;

				if (!connacc)
				{
					continue;
				}


				if (NeedUpdateGameTime)
				{
					t_game* conngame = NULL;
					if ((conngame = conn_get_game(conn)))
					{
						account_add_newevent(connacc, 7);
						account_set_playmap(conn->protocol.account, game_get_statstype(conngame));
					}
					else
					{
						account_set_playmap(conn->protocol.account, "");
						account_add_newevent(connacc, 5);
					}
				}

				if (conn->protocol.HostTimeOut > 0)
					conn->protocol.HostTimeOut--;
				if (conn->protocol.botconnection)
				{
					if (conn->protocol.HostTimeOut > 1)
					{
						// ≈сли бот создал игру установить ее в структуру и оповестить игрока
						t_game* createdgame = conn->protocol.botconnection->protocol.game;
						if (createdgame)
						{

							const char* gamename = game_get_name(createdgame);
							conn->protocol.hostedgame = createdgame;
							conn->protocol.HostTimeOut = 0;
							message_send_text(conn, message_type_info, conn, localize(conn, "Success! Game {} created!",
								gamename));

							account_add_newevent(connacc, 6);

							conn_send_ah_followstate(conn, Watch::ET_joingame, gamename);

							game_set_statstype(createdgame, conn->protocol.HostStatsType);
							game_set_forstats(createdgame, conn->protocol.ForStats);
							game_set_maxplayers(createdgame, conn->protocol.HostPlayerCount);

							if (conn->protocol.IsNewClient)
							{
								InitializeAlternativeGameData(conn);
							}

							// 
							if (!conn->protocol.IsNewClient && conn->protocol.ForStats && conn->protocol.HostStatsType && conn->protocol.HostStatsType[0] != '\0')
							{
								int upts = account_map_get_stats(connacc, conn->protocol.HostStatsType, "mmr");
								if (upts > 800)
								{
									// 1500 / 100 = 15 * 30 = 450 -> Ћимит = 1050 - 1950
									float uptspercent = upts / 100.0f * 30.f;
									float minlimit = upts - uptspercent;
									if (minlimit < 800.0f)
										minlimit = 800.0f;
									float maxlimit = upts + uptspercent;
									game_set_dotaptslimit(createdgame, (int)minlimit, (int)maxlimit);
								}
								else
								{
									game_set_dotaptslimit(createdgame, -100, 799);
								}
							}
						}
					}
					else if (conn->protocol.HostTimeOut == 1)
					{
						if (!conn->protocol.hostedgame)
						{
							conn_set_lastbadgametime(conn->protocol.botconnection);
							message_send_text(conn, message_type_error, conn, localize(conn, "ERROR! BOT CAN'T CREATE GAME!"));
							conn->protocol.botconnection->protocol.availabledBot = true;
							message_send_text(conn->protocol.botconnection, message_type_whisper, conn->protocol.botconnection, "!unhost");
						}
					}
				}
				else if (conn->protocol.HostTimeOut > 0) {
					message_send_text(conn, message_type_error, conn, localize(conn, "ERROR! SELECTED BOT LOST!!"));
					conn->protocol.HostTimeOut = 0;
				}
			}
		}

		extern void conn_set_host_timeout(t_connection* c, int timeout)
		{
			if (!c)
				return;
			c->protocol.HostTimeOut = timeout;
		}

		extern int conn_get_host_timeout(t_connection* c)
		{
			if (!c)
				return 10;
			return c->protocol.HostTimeOut;
		}

		extern void conn_set_locate(t_connection* c, const char* locate)
		{
			if (!c)
				return;
			int locateval = 0;
			std::string locatelower = ToLower(locate);
			if (locatelower == "ru")
				locateval = 0;
			else if (locatelower == "ua")
				locateval = 1;
			else if (locatelower == "eu")
				locateval = 2;
			else if (locatelower == "usa")
				locateval = 3;
			else if (locatelower == "trololo")
				locateval = 4;


			c->protocol.bot_locate = locateval;
		}

		extern void conn_send_bot_command(std::string msg)
		{
			t_elem const* curr;
			t_connection* conn;
			LIST_TRAVERSE_CONST(connlist(), curr)
			{
				conn = (t_connection*)elem_get_data(curr);
				if (conn->protocol.IamBOT)
				{
					message_send_text(conn, message_type_whisper, conn, msg);
				}
			}

		}

		extern t_connection* conn_search_availablebot(t_connection* c)
		{
			if (!c)
				return NULL;
			eventlog(eventlog_level_error, __FUNCTION__, ".");

			t_elem const* curr;
			t_account* botacc;
			t_connection* conn;
			LIST_TRAVERSE_CONST(connlist(), curr)
			{
				conn = (t_connection*)elem_get_data(curr);
				if (conn->protocol.IamBOT)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "FOUND BOT {} #1", account_get_name(conn->protocol.account));

					botacc = conn->protocol.account;

					if (botacc && conn->protocol.availabledBot && conn->protocol.bot_locate == c->protocol.bot_locate)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "OK1");

						std::time_t tnow;
						std::time(&tnow);

						if (conn->protocol.lastbadgametime > 0)
						{
							if (tnow < conn->protocol.lastbadgametime + 10)
							{
								eventlog(eventlog_level_error, __FUNCTION__, "BAD1");
								continue;
							}
							else conn->protocol.lastbadgametime = 0;
						}
						eventlog(eventlog_level_error, __FUNCTION__, "OK2");
						return conn;
					}
					else
						eventlog(eventlog_level_error, __FUNCTION__, "BAD2");
				}
			}

			curr = NULL;
			conn = NULL;
			LIST_TRAVERSE_CONST(connlist(), curr)
			{
				conn = (t_connection*)elem_get_data(curr);
				if (conn->protocol.IamBOT)
				{
					botacc = conn->protocol.account;
					eventlog(eventlog_level_error, __FUNCTION__, "FOUND BOT {} #2", account_get_name(conn->protocol.account));

					if (botacc && conn->protocol.availabledBot)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "OK1");
						std::time_t tnow;
						std::time(&tnow);

						if (conn->protocol.lastbadgametime > 0)
						{
							if (tnow < conn->protocol.lastbadgametime + 10)
							{
								eventlog(eventlog_level_error, __FUNCTION__, "BAD1");
								continue;
							}
							else conn->protocol.lastbadgametime = 0;
						}

						eventlog(eventlog_level_error, __FUNCTION__, "OK2");
						return conn;
					}
					else
						eventlog(eventlog_level_error, __FUNCTION__, "BAD2");
				}
			}

			return NULL;
		}


		extern void conn_set_lastbadgametime(t_connection* c)
		{
			if (!c)
				return;
			std::time(&c->protocol.lastbadgametime);
		}

		extern void conn_set_host_statstype(t_connection* c, const char* statstype)
		{
			if (!c)
				return;
			eventlog(eventlog_level_error, __FUNCTION__, ".");

			if (!statstype || statstype[0] == '\0')
			{
				c->protocol.HostStatsType[0] = '\0';
				return;
			}
			std::snprintf(c->protocol.HostStatsType, 256, "%s", statstype);
		}

		extern const char* conn_get_host_statstype(t_connection* c)
		{
			if (!c)
				return NULL;

			t_account* acc = c->protocol.account;
			if (acc)
			{
				eventlog(eventlog_level_error, __FUNCTION__, " acc {} ", account_get_name(acc));
			}
			else
				return "";
			return c->protocol.HostStatsType;
		}


		extern void conn_set_host_withstats(t_connection* c, bool stats)
		{
			if (!c)
				return;
			eventlog(eventlog_level_error, __FUNCTION__, ".");
			c->protocol.ForStats = stats;
		}

		extern bool conn_get_host_withstats(t_connection* c)
		{
			if (!c)
				return NULL;
			return c->protocol.ForStats;
		}



		extern void conn_set_host_players(t_connection* c, int players)
		{
			if (!c)
				return;

			c->protocol.HostPlayerCount = players;
		}

		extern int conn_get_host_players(t_connection* c)
		{
			if (!c)
				return NULL;
			return c->protocol.HostPlayerCount;
		}

		extern void conn_set_hash(t_connection* c, const char* hash)
		{
			memset(c->protocol.hash, 0, 256);

			if (hash)
			{
				size_t hashlen = strlen(hash);
				if (hashlen > 30 && hashlen < 256)
				{
					memcpy(c->protocol.hash, hash, hashlen);
				}
			}
		}

		extern const char* conn_get_hash(t_connection* c)
		{
			if (!c)
				return NULL;
			return c->protocol.hash;
		}

		extern void conn_set_plainpassword(t_connection* c, const char* hash)
		{
			memset(c->protocol.plainpassword, 0, 256);

			if (hash)
			{
				size_t hashlen = strlen(hash);
				if (hashlen > 1 && hashlen < 256)
				{
					memcpy(c->protocol.plainpassword, hash, hashlen);
				}
			}
		}

		extern const char* conn_get_plainpassword(t_connection* c)
		{
			if (!c)
				return NULL;
			return c->protocol.plainpassword;
		}

		extern void conn_send_all_infos(t_connection* c)
		{
			if (!c)
				return;
			unsigned int magicvalue = conn_get_antihack_magic_value(c);


			conn_send_ah_packet(c, 2, magicvalue, magicvalue, magicvalue, magicvalue);
			conn_send_ah_packet(c, 1);

			for (std::string s : CrcWhiteList)
			{
				conn_send_ah_packet(c, 0x15, string_to_uint32(s));
			}

			for (auto& s : MapHostStructList)
			{
				if (s.crc32 != 0)
					conn_send_ah_packet(c, 0x16, s.crc32);
			}

			conn_send_ah_packet(c, 0x3F27);

			for (auto& s : MapHostStructList)
			{
				conn_send_ah_newmap(c, &s);
			}
			c->protocol.all_infos_okay = true;
		}

		extern bool conn_is_infos_okay(t_connection* c)
		{
			if (!c)
				return false;
			return c->protocol.all_infos_okay;
		}

		extern bool conn_is_new_client(t_connection* c)
		{
			if (c)
				return c->protocol.IsNewClient;
			return false;
		}

		std::vector<AutoPlayPlayerQueue> AutoPlayPlayerList;
		std::vector<AutoPlayPlayerQueueInfo> AutoPlayPlayerInfoList;


		// Ќужно следить и удал€ть игроков которые не отправили данные в течении 4х секунд
		extern void UpdatePlayerInQueue(t_connection* c, std::string MapCode, std::string Players, int rank)
		{
			if (!c || !c->protocol.account || MapCode.length() == 0 || Players.length() == 0)
				return;

			bool FoundInQueue = false;

			for (auto& p : AutoPlayPlayerList)
			{
				if (p.playeracc == c->protocol.account)
				{
					FoundInQueue = true;
					std::time(&p.lastupdatetime);
					p.MapCode = MapCode;
					p.Players = Players;
					p.rank = rank;
				}
			}

			if (!FoundInQueue)
			{
				AutoPlayPlayerQueue tmpAutoPlayPlayerQueue = AutoPlayPlayerQueue();
				tmpAutoPlayPlayerQueue.playeracc = c->protocol.account;
				tmpAutoPlayPlayerQueue.MapCode = MapCode;
				tmpAutoPlayPlayerQueue.Players = Players;
				tmpAutoPlayPlayerQueue.rank = rank;
				std::time(&tmpAutoPlayPlayerQueue.lastupdatetime);
				AutoPlayPlayerList.push_back(tmpAutoPlayPlayerQueue);
			}


		}


		extern void RemovePlayerFromQueue(t_connection* c)
		{
			if (!c)
				return;

			for (int i = 0; i < AutoPlayPlayerList.size(); )
			{
				if (c->protocol.account == AutoPlayPlayerList[i].playeracc)
				{
					AutoPlayPlayerList.erase(AutoPlayPlayerList.begin() + i);
					return;
				}

			}
		}

		extern void SendPlayerQueue(t_connection* c)
		{


		}

		extern void SendInfoToPlayersInQueue()
		{

			// Build Queue
			AutoPlayPlayerInfoList.clear();


			for (const auto& p : AutoPlayPlayerList)
			{
				bool foundinlist = false;
				for (auto& pinfo : AutoPlayPlayerInfoList)
				{
					if (p.rank == pinfo.rank && p.MapCode == pinfo.MapCode && p.Players == pinfo.Players)
					{
						foundinlist = true;
						pinfo.count++;
					}
				}

				if (!foundinlist)
				{
					AutoPlayPlayerQueueInfo pinfotmp = AutoPlayPlayerQueueInfo();
					pinfotmp.MapCode = p.MapCode;
					pinfotmp.Players = p.Players;
					pinfotmp.count++;
					pinfotmp.rank = p.rank;
					AutoPlayPlayerInfoList.push_back(pinfotmp);
				}
			}

			std::time_t now;
			std::time(&now);
			for (int i = 0; i < AutoPlayPlayerList.size(); )
			{
				if (now - AutoPlayPlayerList[i].lastupdatetime > 5)
				{
					AutoPlayPlayerList.erase(AutoPlayPlayerList.begin() + i);
					continue;
				}


				t_connection* pconn = account_get_conn(AutoPlayPlayerList[i].playeracc);
				if (pconn)
				{
					// Send new player queue indicator
					SendPlayerQueue(pconn);
					// and new player queue
				}


				i++;
			}
		}

	}

}
