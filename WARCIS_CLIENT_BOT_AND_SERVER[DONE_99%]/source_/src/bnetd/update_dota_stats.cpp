/*
* This is an independent project of an individual developer. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/
#include "common/setup_before.h"
#include "config.h"


#include <cstdlib>
#include "common/eventlog.h"
#include "common/util.h"
#include "bnetd/handle_bnet.h"
#include "bnetd/account.h"
#include "bnetd/account_wrap.h"
#include "bnetd/message.h"
#include "bnetd/connection.h"


#include <iostream>
#include <chrono>
#include <thread>
#include <iomanip>
#include <iostream>
#include <map>
#include <queue>
#include <sstream>
#include <string>
#include <vector>
#include <concurrent_vector.h>
#define safevector concurrency::concurrent_vector

#include <stdint.h>
#include <fstream>
#include <regex>

#ifdef WIN32
#include <winsock.h>
#endif

#include <mysql.h>
#include "i18n.h"

#include "common/setup_after.h"

using namespace std;


namespace pvpgn
{
	namespace bnetd
	{
		void CONSOLE_Print( string message )
		{
			cout << message << endl;
		}

		string MySQLEscapeString( MYSQL *conn, string str )
		{
			char *to = new char[ str.size( ) * 2 + 1 ];
			unsigned long size = mysql_real_escape_string( conn, to, str.c_str( ), str.size( ) );
			string result( to, size );
			delete[ ] to;
			return result;
		}

		vector<string> MySQLFetchRow( MYSQL_RES *res )
		{
			vector<string> Result;

			MYSQL_ROW Row = mysql_fetch_row( res );

			if ( Row )
			{
				unsigned long *Lengths;
				Lengths = mysql_fetch_lengths( res );

				for ( unsigned int i = 0; i < mysql_num_fields( res ); i++ )
				{
					if ( Row[ i ] )
						Result.push_back( string( Row[ i ], Lengths[ i ] ) );
					else
						Result.push_back( string( ) );
				}
			}

			return Result;
		}

		string UTIL_ToString( uint32_t i )
		{
			string result;
			stringstream SS;
			SS << i;
			SS >> result;
			return result;
		}

		string UTIL_ToString( float f, int digits )
		{
			string result;
			stringstream SS;
			SS << std::fixed << std::setprecision( digits ) << f;
			SS >> result;
			return result;
		}

		uint32_t UTIL_ToUInt32( string &s )
		{
			uint32_t result;
			stringstream SS;
			SS << s;
			SS >> result;
			return result;
		}

		float UTIL_ToFloat( string &s )
		{
			float result;
			stringstream SS;
			SS << s;
			SS >> result;
			return result;
		}

		struct StatsUpdateDBList
		{
			string username;
			string password;
			string server;
			string database;
			int port;
			MYSQL * connection;
		};

		struct MessageList
		{
			string username;
			string message;
			MessageList( string User, string Message )
			{
				username = User;
				message = Message;
			}
		};

		safevector<MessageList> messages;

		void AddNewMessage( string user, string msg )
		{
			messages.push_back( MessageList( user, msg ) );
		}

		void ProcessAllMessages( )
		{
			safevector<MessageList> newmessages;

			for ( auto msg : messages )
			{
				t_account * acc = accountlist_find_account( msg.username.c_str( ) );
				if ( acc )
				{
					t_connection * conn = account_get_conn( acc );
					if ( conn )
					{
						if ( conn_get_channel( conn ) )
							message_send_text( conn, message_type_info, conn, msg.message.c_str( ) );
						else
							newmessages.push_back( msg );
					}
				}
			}

			messages.clear( );
			messages = newmessages;
			newmessages.clear( );
		}

		extern void UpdatePlayerStatsThread( )
		{
			// коофициент победы
			float CustomWinCF = 1.0;
			// коофициент поражения
			float CustomLoseCF = 1.0;
			// базовое значение mmr
			float base_mmr_value = 25.0;


			// глобальные статические переменные для среднего количества игр
			static bool LoadDuration = false;
			// сумма продолжительности игр
			static float GameDurations[ 10 ];
			// количество игр
			static int GameCounts[ 10 ];



			vector<StatsUpdateDBList> statsdblist;
			// username;password;server;db;prefix;port
			std::ifstream infile( "dblist.txt" );
			if ( !infile.is_open( ) )
			{
				cout << "Error in load dblist.txt file!" << endl;
				return;
			}
			std::smatch matchDB;

			std::regex GetDBconfig( "(.+?);(.+?);(.+?);(.+?);(.+?)" );
			std::string str;
			while ( std::getline( infile, str ) )
			{
				if ( std::regex_match( str, matchDB, GetDBconfig ) )
				{
					StatsUpdateDBList tmpDB = StatsUpdateDBList( );
					tmpDB.username = matchDB[ 1 ].str( );
					tmpDB.password = matchDB[ 2 ].str( );
					tmpDB.server = matchDB[ 3 ].str( );
					tmpDB.database = matchDB[ 4 ].str( );
					tmpDB.port = stoi( matchDB[ 5 ].str( ) );
					statsdblist.push_back( tmpDB );
					cout << "Added new server:" << tmpDB.server << " with login:" << tmpDB.username << " and port : " << tmpDB.port << endl;
				}
			}

			for ( StatsUpdateDBList & currentdb : statsdblist )
			{
				cout << "connecting to database server" << endl;
				MYSQL *Connection = NULL;

				if ( !( Connection = mysql_init( NULL ) ) )
				{
					cout << "error: " << mysql_error( Connection ) << endl;
					goto cleanup;
				}

				my_bool Reconnect = true;
				mysql_options( Connection, MYSQL_OPT_RECONNECT, &Reconnect );

				if ( !( mysql_real_connect( Connection, currentdb.server.c_str( ), currentdb.username.c_str( ), currentdb.password.c_str( ), currentdb.database.c_str( ), currentdb.port, NULL, 0 ) ) )
				{
					cout << "error: " << mysql_error( Connection ) << endl;
					goto cleanup;
				}

				cout << "connected" << endl;
				currentdb.connection = Connection;
			}

			while ( true )
			{
				std::this_thread::sleep_for( std::chrono::seconds( 2 ) );
				// Сюда надо будет вставить переделанный код udpate dota elo +
				// Можно сделать подключение к списку адресов, и внесение статистики в список, затем  +
				// в UpdatePlayerStatsTick отправить всем игрокам статистику
				for ( StatsUpdateDBList & currentdb : statsdblist )
				{

					MYSQL *Connection = currentdb.connection;

					//cout << "beginning transaction" << endl;

					string QBegin = "BEGIN";

					if ( mysql_real_query( Connection, QBegin.c_str( ), QBegin.size( ) ) != 0 )
					{
						cout << "error: " << mysql_error( Connection ) << endl;
						goto cleanup;
					}

					//cout << "creating tables" << endl;

					string QCreate1 = "CREATE TABLE IF NOT EXISTS dota_elo_scores ( id INT NOT NULL AUTO_INCREMENT PRIMARY KEY, name VARCHAR(15) NOT NULL, server VARCHAR(100) NOT NULL, score REAL NOT NULL )";
					string QCreate2 = "CREATE TABLE IF NOT EXISTS dota_elo_games_scored ( id INT NOT NULL AUTO_INCREMENT PRIMARY KEY, gameid INT NOT NULL, duration INT NOT NULL, gamerank INT NOT NULL )";

					if ( mysql_real_query( Connection, QCreate1.c_str( ), QCreate1.size( ) ) != 0 )
					{
						cout << "error: " << mysql_error( Connection ) << endl;
						goto cleanup;
					}

					if ( mysql_real_query( Connection, QCreate2.c_str( ), QCreate2.size( ) ) != 0 )
					{
						cout << "error: " << mysql_error( Connection ) << endl;
						goto cleanup;
					}

					//	cout << "getting unscored games" << endl;
					::queue<uint32_t> UnscoredGames;

					string QSelectUnscored = "SELECT id FROM games WHERE id NOT IN ( SELECT gameid FROM dota_elo_games_scored ) ORDER BY id";
					string QSelectScoredGamesDuration = "SELECT duration, gamerank FROM `dota_elo_games_scored` WHERE duration > 0";

					if ( mysql_real_query( Connection, QSelectUnscored.c_str( ), QSelectUnscored.size( ) ) != 0 )
					{
						cout << "error: " << mysql_error( Connection ) << endl;
						goto cleanup;
					}
					else
					{
						MYSQL_RES *Result = mysql_store_result( Connection );

						if ( Result )
						{
							vector<string> Row = MySQLFetchRow( Result );

							while ( !Row.empty( ) )
							{
								UnscoredGames.push( UTIL_ToUInt32( Row[ 0 ] ) );
								Row = MySQLFetchRow( Result );
							}

							mysql_free_result( Result );
						}
						else
						{
							cout << "error: " << mysql_error( Connection ) << endl;
							goto cleanup;
						}
					}

					if ( !LoadDuration )
					{
						LoadDuration = true;

						cout << "Load Games Duration Stats.." << endl;

						for ( int i = 0; i < 10; i++ )
						{
							GameDurations[ i ] = 30.0f * 60;
							GameCounts[ i ] = 1;
						}


						if ( mysql_real_query( Connection, QSelectScoredGamesDuration.c_str( ), QSelectScoredGamesDuration.size( ) ) != 0 )
						{
							cout << "error: " << mysql_error( Connection ) << endl;
							goto cleanup;
						}
						else
						{
							MYSQL_RES *Result = mysql_store_result( Connection );

							if ( Result )
							{

								vector<string> Row = MySQLFetchRow( Result );

								cout << "game duration found! row size:" << Row.size( ) << endl;


								while ( Row.size( ) == 2 )
								{
									uint32_t rankid = UTIL_ToUInt32( Row[ 1 ] );
									uint32_t duration = UTIL_ToUInt32( Row[ 0 ] );

									if ( rankid > 0 && rankid < 10 )
									{
										GameCounts[ rankid ]++;
										GameDurations[ rankid ] += ( float )duration;
									}
									cout << "Added game with rank:" << rankid << ". Duration:" << duration << endl;
									Row = MySQLFetchRow( Result );
								}

								mysql_free_result( Result );
							}
							else
							{
								cout << "error: " << mysql_error( Connection ) << endl;
								goto cleanup;
							}
						}

					}
					//	cout << "found " << UnscoredGames.size( ) << " unscored games" << endl;

					while ( !UnscoredGames.empty( ) )
					{
						uint32_t GameID = UnscoredGames.front( );
						UnscoredGames.pop( );

						string QSelectPlayers = "SELECT dota_elo_scores.id, gameplayers.name, gameplayers.left, spoofedrealm, newcolour, winner, min, sec, statstype, firstblood, score, dotaplayers.kills, dotaplayers.deaths, dotaplayers.assists, dotaplayers.creepkills, dotaplayers.creepdenies, dotaplayers.neutralkills, dotaplayers.towerkills, dotaplayers.raxkills, dotaplayers.courierkills, games.duration, games.gamename FROM dotaplayers LEFT JOIN dotagames ON dotagames.gameid=dotaplayers.gameid LEFT JOIN  gameplayers ON gameplayers.gameid=dotaplayers.gameid AND gameplayers.colour=dotaplayers.colour LEFT JOIN dota_elo_scores ON dota_elo_scores.name=gameplayers.name LEFT JOIN games on games.id=dotagames.gameid WHERE dotaplayers.gameid=" + UTIL_ToString( GameID );

						if ( mysql_real_query( Connection, QSelectPlayers.c_str( ), QSelectPlayers.size( ) ) != 0 )
						{
							cout << "error: " << mysql_error( Connection ) << endl;
							goto cleanup;
						}
						else
						{
							MYSQL_RES *Result = mysql_store_result( Connection );

							if ( Result )
							{

								cout << "gameid " << UTIL_ToString( GameID ) << " found" << endl;


								bool Leaver[ 10 ];

								// Инициализация ливеров в false
								for ( int i = 0; i < 10; i++ )
								{
									Leaver[ i ] = false;
								}


								uint32_t lefttime[ 10 ];

								// Инициализация время выхода из игры в 0
								for ( int i = 0; i < 10; i++ )
								{
									lefttime[ i ] = 0;
								}

								// Если была победа то все ливеры за 5 минут не считаются ливерами

								bool CreepStart = false;
								bool FirstBloodFound = false;

								/*
									 Первый игрок ливает = лив
									 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

									 Если не было фб или не пошли крипы то:
									 1. Разрешить всем ливнуть в течении 5 минут.
									 (^ в этом случае игра пропускается)

									 2. Все кто остался ливать не могут и должны доиграть до конца.

									 3. Если все же кто-то ливнул и различие между командами  более одного человека
									 abs(Team1-Team2) > 1 то снова разрешить всем ливнуть в течении 5 минут.
									 (^ в этом случае игра тоже пропускается)

									 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

									 Если был фб и пошли крипы то:
									 1. Ливать никому нельзя.

									 2. Если все же кто-то ливнул и различие между командами  более одного человека
									 abs(Team1-Team2) > 1 то снова разрешить всем ливнуть в течении 5 минут.
									 (^ в этом случае игра пропускается)

									 3. Все кто остался ливать не могут и должны доиграть до конца
								*/

								string ignore = "";
								t_account * accs[ 10 ];
								uint32_t rowids[ 10 ];
								string names[ 10 ];
								string servers[ 10 ];

								uint32_t colours[ 10 ];


								uint32_t Winner = 0;

								string gamename = "unknown";

								string statstype = "";

								uint32_t firstblood = 0;

								double team_min_mmr[ 2 ];
								double team_max_mmr[ 2 ];

								team_min_mmr[ 0 ] =
									team_min_mmr[ 1 ] =
									team_max_mmr[ 0 ] =
									team_max_mmr[ 1 ] = 0.0f;


								double player_ratings[ 10 ];
								double player_global_ratings[ 10 ];

								uint32_t kills[ 10 ];
								uint32_t deaths[ 10 ];
								uint32_t assists[ 10 ];
								uint32_t creeps[ 10 ];
								uint32_t denies[ 10 ];
								uint32_t neutrals[ 10 ];
								uint32_t towers[ 10 ];
								uint32_t rax[ 10 ];
								uint32_t couriers[ 10 ];

								uint32_t rank[ 10 ];
								//Индивидуальный корректировочный коэффициент
								float individ_c[ 10 ];
								//Ранг игры
								int GameRank = 1;
								//Продолжительность игры:
								uint32_t gameduration = 0;
								//Продолжительность со старта крипов:
								uint32_t Min = 0;// Не учитывается (добавлено к Sec)
								uint32_t Sec = 0;
								//Время старта крипов:
								uint32_t CreepStartTime = 0;

								bool exists[ 10 ];
								int num_players = 0;

								int player_teams[ 10 ];
								int num_teams = 2;
								double team_ratings[ 2 ];
								double team_ratings_player[ 2 ];
								float team_winners[ 2 ];
								int team_numplayers[ 2 ];
								int team_numleavers[ 2 ];
								team_ratings[ 0 ] = 0.0;
								team_ratings[ 1 ] = 0.0;
								team_numplayers[ 0 ] = 0;
								team_numplayers[ 1 ] = 0;
								team_numleavers[ 0 ] = 0;
								team_numleavers[ 1 ] = 0;


								vector<string> Row = MySQLFetchRow( Result );


								cout << "Player list:" << endl;

								while ( Row.size( ) == 22 )
								{

									// Дота поддерживает максимум 10 игроков	
									if ( num_players >= 10 )
									{
										cout << "gameid " << UTIL_ToString( GameID ) << " has more than 10 players, ignoring" << endl;
										ignore += "(Big player count)";
										break;
									}

									// Установить ID игрока
									rowids[ num_players ] = !Row[ 0 ].empty( ) ? UTIL_ToUInt32( Row[ 0 ] ) : 0;



									if ( Row[ 1 ].empty( ) )
									{
										Row = MySQLFetchRow( Result );
										continue;
									}

									//int idd = 0;
									//for ( auto s : Row )
									//{
									//	cout << "(" << idd++ << "):" << Row[ idd ];
								//	}

									// Сохранить имя игрока
									names[ num_players ] = Row[ 1 ];

									cout << "Player:" << Row[ 1 ];

									// Сохранить время выхода из игры
									lefttime[ num_players ] = !Row[ 2 ].empty( ) ? UTIL_ToUInt32( Row[ 2 ] ) : 0;

									cout << ", left:" << Row[ 2 ];

									// Сохранить имя сервера
									servers[ num_players ] = Row[ 3 ];


									uint32_t Colour = !Row[ 4 ].empty( ) ? UTIL_ToUInt32( Row[ 4 ] ) : 0;
									// Сохранить цвет игрока
									colours[ num_players ] = Colour;


									// Записать победителя
									if ( Winner > 0 && !Row[ 5 ].empty( ) )
									{
										if ( Winner != UTIL_ToUInt32( Row[ 5 ] ) )
										{
											ignore += "(Winner info has been hacked?!)";
										}
									}
									else
										Winner = !Row[ 5 ].empty( ) ? UTIL_ToUInt32( Row[ 5 ] ) : 0;

									// Продолжительность игры
									gameduration = !Row[ 20 ].empty( ) ? UTIL_ToUInt32( Row[ 20 ] ) : 0;



									// Установить продолжительность игры x2
									if ( Min == 0 && Sec == 0 )
									{
										Min = !Row[ 6 ].empty( ) ? UTIL_ToUInt32( Row[ 6 ] ) : 0;
										Sec = !Row[ 7 ].empty( ) ? UTIL_ToUInt32( Row[ 7 ] ) : 0;
										Sec = Sec + Min * 60;

										if ( Min == 0 && Sec == 0 )
										{
											// Исправить если не было победителя

											if ( gameduration > 2 * 60 )
											{
												Min = ( gameduration - ( 2 * 60 ) ) / 60;
												Sec = gameduration - ( 2 * 60 );
											}

										}
									}

									// Время старта крипов
									CreepStartTime = gameduration - Sec;


									if ( Sec == 0 )
									{
										ignore += "(skip creep no start!)";
									}

									// Установить код карты(тип статы)
									if ( statstype.empty( ) )
										statstype = Row[ 8 ];
									if ( statstype.empty( ) )
										ignore += "(no stats)";


									// Установить флаг FirstBlood
									if ( firstblood == 0 )
										firstblood = !Row[ 9 ].empty( ) ? UTIL_ToUInt32( Row[ 9 ] ) : 0;
									if ( firstblood != 0 )
										FirstBloodFound = true;

									// Рейтинг игрока, 0 значит новый игрок
									player_ratings[ num_players ] = 1000.0;

									// Получить аккаунт игрока и установить в переменную
									t_account * newplayeracc = accountlist_find_account( names[ num_players ].c_str( ) );
									accs[ num_players ] = newplayeracc;
									if ( !newplayeracc )
									{
										cout << "ERROR! NO PLAYER FOUND AT THIS SERVER!" << endl;
										ignore += "(Found unknown player)";
									}
									else
									{
										// Установить рейтинг игрока
										player_ratings[ num_players ] = ( double )account_map_get_stats( newplayeracc, statstype, "mmr" );

										if ( account_map_is_firstgame( newplayeracc, statstype ) /*|| player_ratings[ num_players ] < 1.0*/ )
										{
											account_map_set_firstgame( newplayeracc, statstype, 0 );

											player_ratings[ num_players ] = 1000.0;
											account_map_set_stats( newplayeracc, statstype, "mmr", 1000 );
											account_map_set_stats( newplayeracc, statstype, "wins", 0 );
											account_map_set_stats( newplayeracc, statstype, "loses", 0 );
											account_map_set_stats( newplayeracc, statstype, "leaves", 0 );
											account_map_set_stats( newplayeracc, statstype, "streak", 0 );
											account_map_set_stats( newplayeracc, statstype, "minstreak", 0 );
											account_map_set_stats( newplayeracc, statstype, "maxstreak", 0 );
											account_map_set_stats( newplayeracc, statstype, "kills", 0 );
											account_map_set_stats( newplayeracc, statstype, "deaths", 0 );
											account_map_set_stats( newplayeracc, statstype, "assists", 0 );

											account_map_set_stats( newplayeracc, "global", "leavesstreak", 0 );
											account_map_set_stats( newplayeracc, "global", "bantime", 0 );
										}

									}

									if ( player_ratings[ num_players ] > 6700.0 )
									{
										//Индивидуальный корректировочный коэффициент
										individ_c[ num_players ] = 3.5f;
										//Номер ранга
										rank[ num_players ] = 6;
									}
									else if ( player_ratings[ num_players ] > 5400.0 )
									{
										individ_c[ num_players ] = 2.5f;
										rank[ num_players ] = 5;
									}
									else if ( player_ratings[ num_players ] > 4100.0 )
									{
										individ_c[ num_players ] = 1.65f;
										rank[ num_players ] = 4;
									}
									else if ( player_ratings[ num_players ] > 2800.0 )
									{
										individ_c[ num_players ] = 1.2f;
										rank[ num_players ] = 3;
									}
									else if ( player_ratings[ num_players ] > 1500.0 )
									{
										individ_c[ num_players ] = 1.0f;
										rank[ num_players ] = 2;
									}
									else if ( player_ratings[ num_players ] > 800.0 )
									{
										individ_c[ num_players ] = 0.83f;
										rank[ num_players ] = 1;
									}
									else
									{
										individ_c[ num_players ] = 1.2f;
										rank[ num_players ] = 0;
									}


									if ( !Row[ 10 ].empty( ) )
									{
										exists[ num_players ] = true;
										player_global_ratings[ num_players ] = UTIL_ToFloat( Row[ 10 ] );
									}
									else
									{
										cout << "(new player)";
										exists[ num_players ] = false;
										player_global_ratings[ num_players ] = 1000.0f;
									}

									// Установить статистику игрока 
									kills[ num_players ] = !Row[ 11 ].empty( ) ? UTIL_ToUInt32( Row[ 11 ] ) : 0;
									deaths[ num_players ] = !Row[ 12 ].empty( ) ? UTIL_ToUInt32( Row[ 12 ] ) : 0;
									assists[ num_players ] = !Row[ 13 ].empty( ) ? UTIL_ToUInt32( Row[ 13 ] ) : 0;
									creeps[ num_players ] = !Row[ 14 ].empty( ) ? UTIL_ToUInt32( Row[ 14 ] ) : 0;
									denies[ num_players ] = !Row[ 15 ].empty( ) ? UTIL_ToUInt32( Row[ 15 ] ) : 0;
									neutrals[ num_players ] = !Row[ 16 ].empty( ) ? UTIL_ToUInt32( Row[ 16 ] ) : 0;
									towers[ num_players ] = !Row[ 17 ].empty( ) ? UTIL_ToUInt32( Row[ 17 ] ) : 0;
									rax[ num_players ] = !Row[ 18 ].empty( ) ? UTIL_ToUInt32( Row[ 18 ] ) : 0;
									couriers[ num_players ] = !Row[ 19 ].empty( ) ? UTIL_ToUInt32( Row[ 19 ] ) : 0;

									// Установить имя игры
									gamename = !Row[ 21 ].empty( ) ? Row[ 21 ] : "unknown";


									//// Поиск главного ливера
									//if ( FirstLeaveTime > lefttime[ num_players ] )
									//{
									//	Leaver = num_players;
									//	FirstLeaveTime = lefttime[ num_players ];
									//}




									// Ограничение на минимальный рейтинг
									if ( player_ratings[ num_players ] < 1.0 )
									{
										player_ratings[ num_players ] = 1.0;
									}
									// Ограничение на максимальный рейтинг
									else if ( player_ratings[ num_players ] > 100000.0 )
									{
										player_ratings[ num_players ] = 100000.0;
									}


									cout << ", rating:" << ( int )player_ratings[ num_players ] << endl;

									// Поиск и установка минимального и максимального ммр (статистики)
									if ( Colour >= 1 && Colour <= 5 )
									{
										if ( team_min_mmr[ 0 ] == 0.0 || team_min_mmr[ 0 ] > player_ratings[ num_players ] )
											team_min_mmr[ 0 ] = player_ratings[ num_players ];

										if ( team_max_mmr[ 0 ] == 0.0 || team_max_mmr[ 0 ] < player_ratings[ num_players ] )
											team_max_mmr[ 0 ] = player_ratings[ num_players ];

										player_teams[ num_players ] = 0;
										team_ratings[ 0 ] += player_ratings[ num_players ];
										team_numplayers[ 0 ]++;

										cout << "(Sentinel)";
									}
									else if ( Colour >= 7 && Colour <= 11 )
									{
										if ( team_min_mmr[ 1 ] == 0.0 || team_min_mmr[ 1 ] > player_ratings[ num_players ] )
											team_min_mmr[ 1 ] = player_ratings[ num_players ];

										if ( team_max_mmr[ 1 ] == 0.0 || team_max_mmr[ 1 ] < player_ratings[ num_players ] )
											team_max_mmr[ 1 ] = player_ratings[ num_players ];


										player_teams[ num_players ] = 1;

										team_ratings[ 1 ] += player_ratings[ num_players ];
										team_numplayers[ 1 ]++;
										cout << "(Scourge)";
									}
									else
									{
										cout << "gameid " << UTIL_ToString( GameID ) << " has a player with an invalid newcolour, ignoring" << endl;
										ignore += "(Unknown player color found)";
									}

									num_players++;
									Row = MySQLFetchRow( Result );


									cout << endl;
								}


								cout << " Game time (full) :" << gameduration;
								cout << " ---   Game time (ingame) :" << Sec;
								cout << endl;


								// Ливеры
								// 1 ливер это первый ливнувший

								// Последний ливер
								uint32_t LastLeaveTime = 0;
								int LastLeaver = 0;

								// Предпоследний ливер
								uint32_t PreLastLeaveTime = 0;
								int PreLastLeaver = 0;

								// Первый ливер
								uint32_t FirstLeaveTime = 0;
								int FirstLeaver = 0;

								// Можно ливать если прошло больше чем:
								uint32_t TimeForLeave = 0;

								bool SkipGameIfOneTeamLeave = false;

								// Поиск самого самого первого ливера!
								for ( int i = 0; i < num_players; i++ )
								{
									if ( lefttime[ i ] < FirstLeaveTime )
									{
										FirstLeaveTime = lefttime[ i ];
										FirstLeaver = i;
									}
								}


								// Поиск самого самого последнего ливера!
								for ( int i = 0; i < num_players; i++ )
								{
									if ( lefttime[ i ] > LastLeaveTime )
									{
										LastLeaveTime = lefttime[ i ];
										LastLeaver = i;
									}
								}

								// Поиск самого самого предпоследнего ливера!
								for ( int i = 0; i < num_players; i++ )
								{
									if ( LastLeaver != i &&
										FirstLeaver != i &&
										lefttime[ i ] > FirstLeaveTime
										&& lefttime[ i ] < LastLeaveTime )
									{
										PreLastLeaveTime = lefttime[ i ];
										PreLastLeaver = i;
									}
								}

								// Разрешить ливать за 2-4 минуты до конца игры
								if ( gameduration - PreLastLeaveTime > 120 )
									PreLastLeaveTime = gameduration - 120;
								TimeForLeave = PreLastLeaveTime - 120;

								if ( Winner == 1 || Winner == 2 )
								{
									// Если есть победитель и первый ливер ливнул в разрешенное время, ниче не делать
									if ( FirstLeaveTime > TimeForLeave )
									{
										FirstLeaveTime = 0;
										PreLastLeaveTime = 0;
										LastLeaveTime = 0;

										FirstLeaver = 0;
										LastLeaver = 0;
										PreLastLeaver = 0;
									}
									else
									{
										//иначе установить лив если ливнули через 3 мин после первого ливера
										//или ливнули раньше чем за 2-4 минуты до конца  игры
										for ( int i = 0; i < num_players; i++ )
										{
											if ( player_teams[ i ] == 0 )
											{
												if ( lefttime[ i ] < TimeForLeave &&
													lefttime[ i ] > FirstLeaveTime + 3 * 60 )
												{
													Leaver[ i ] = true;
												}
											}
										}

										for ( int i = 0; i < num_players; i++ )
										{
											if ( player_teams[ i ] == 1 )
											{
												if ( lefttime[ i ] < TimeForLeave &&
													lefttime[ i ] > FirstLeaveTime + 3 * 60 )
												{
													Leaver[ i ] = true;
												}
											}
										}

									}
								}
								bool allscourgeleave = false;
								bool allsentinelleave = false;






								// Определить кому можно ливать
								if ( Winner != 1 && Winner != 2 )
								{
									if ( FirstLeaveTime > 0 )
									{
										Leaver[ FirstLeaver ] = true;
										if ( FirstLeaveTime < 5 * 60 )
										{
											SkipGameIfOneTeamLeave = true;
											allsentinelleave = true;

											for ( int i = 0; i < num_players; i++ )
											{
												if ( player_teams[ i ] == 0 )
												{
													if ( lefttime[ i ] > FirstLeaveTime + 3 * 60 )
													{
														allsentinelleave = false;
													}
												}
											}

											allscourgeleave = true;

											for ( int i = 0; i < num_players; i++ )
											{
												if ( player_teams[ i ] == 1 )
												{
													if ( lefttime[ i ] > FirstLeaveTime + 3 * 60 )
													{
														allscourgeleave = false;
													}
												}
											}

											// Ливнула одна команда в течении 5 минут после первого ливера
											if ( allscourgeleave || allsentinelleave )
												SkipGameIfOneTeamLeave = true;

										}
									}
								}

								allscourgeleave = false;
								allsentinelleave = false;

								bool nowinner = false;

								// Если нет победителя найти его!
								if ( Winner != 1 && Winner != 2 )
								{
									nowinner = true;
									Winner = 1;

									uint32_t _lastleaver = 0;
									for ( int i = 0; i < num_players; i++ )
									{
										if ( player_teams[ i ] == 0 )
										{
											if ( lefttime[ i ] > _lastleaver )
											{
												_lastleaver = lefttime[ i ];
											}
										}
									}
									// Если последний игрок покинувший игру из Scourge значит победили Scourge
									for ( int i = 0; i < num_players; i++ )
									{
										if ( player_teams[ i ] == 1 )
										{
											if ( lefttime[ i ] > _lastleaver )
											{
												_lastleaver = lefttime[ i ];
												Winner = 2;
											}
										}
									}
									//cout << " Unknown ";
								}


								if ( Winner == 1 )
								{
									team_winners[ 0 ] = 1.0;
									team_winners[ 1 ] = 0.0;
								}
								else if ( Winner == 2 )
								{
									team_winners[ 0 ] = 0.0;
									team_winners[ 1 ] = 1.0;
								}


								mysql_free_result( Result );



								if ( num_players == 0 )
								{
									cout << "gameid " << UTIL_ToString( GameID ) << " has no players, ignoring" << endl;
									ignore += "(No players)";
								}
								else if ( team_numplayers[ 0 ] == 0 )
								{
									cout << "gameid " << UTIL_ToString( GameID ) << " has no Sentinel players, ignoring" << endl;
									ignore += "(No Sentinel players)";
								}
								else if ( team_numplayers[ 1 ] == 0 )
								{
									cout << "gameid " << UTIL_ToString( GameID ) << " has no Scourge players, ignoring" << endl;
									ignore += "(No Scourge players)";
								}

								if ( ignore.empty( ) )
								{
									cout << "gameid " << UTIL_ToString( GameID ) << " is calculating" << endl;

									if ( FirstLeaveTime > 0 )
									{
										// тут добавить стрик ливы
										account_map_set_stats( accs[ FirstLeaver ], "global", "bantime", std::time( NULL ) + 60 * 5 );
										account_map_add_stats( accs[ FirstLeaver ], statstype, "leaves", 1 );
										player_ratings[ num_players ] -= 150;
										if ( player_ratings[ num_players ] < 1.0 )
											player_ratings[ num_players ] = 1.0;
										account_map_set_stats( accs[ FirstLeaver ], statstype, "mmr", ( unsigned int )player_ratings[ num_players ] );
									}

									if ( SkipGameIfOneTeamLeave )
									{
										;
									}
									else if ( Winner == 1 || Winner == 2 )
									{
										float ResultMMR[ 10 ];
										for ( int i = 0; i < 10; i++ )
										{
											ResultMMR[ i ] = base_mmr_value;
										}


										//Победитель 
										bool SentinelWinner = Winner == 1;

										cout << "Winner:" << ( SentinelWinner ? "Sentinel" : "Scourge" ) << endl;
										cout << "MMR. Sentinel:" << ( int )team_ratings[ 0 ]
											<< ". Scourge:" << ( int )team_ratings[ 1 ] << ". Mean:";

										double old_player_ratings[ 10 ];
										memcpy( old_player_ratings, player_ratings, sizeof( double ) * 10 );

										/*team_ratings[ 0 ] /= team_numplayers[ 0 ];
										team_ratings[ 1 ] /= team_numplayers[ 1 ];
										*/
										team_ratings_player[ 0 ] = team_ratings[ 0 ] / team_numplayers[ 0 ];
										team_ratings_player[ 1 ] = team_ratings[ 1 ] / team_numplayers[ 1 ];

										//У кого больше MMR
										bool SentinelMMRbigger = team_ratings[ 0 ] > team_ratings[ 1 ];

										//Средний MMR игры округленный в меньшую сторону
										double GameMMR = ( team_ratings[ 0 ] + team_ratings[ 1 ] ) / num_players;

										cout << ( int )GameMMR;

										//Степень разброса
										double GameSCp = abs( team_ratings[ 0 ] / team_ratings[ 1 ] - 1 );

										cout << ", GameSCp:" << GameSCp << endl;


										//Значение A
										float GameCategory = 1.0f;

										if ( GameMMR > 6700.0 )
										{
											GameCategory = 6.0f;
											GameRank = 6;
										}
										else if ( GameMMR > 5400.0 )
										{
											GameCategory = 4.0f;
											GameRank = 5;
										}
										else if ( GameMMR > 4100.0 )
										{
											GameCategory = 3.0f;
											GameRank = 4;
										}
										else if ( GameMMR > 2800.0 )
										{
											GameCategory = 2.0f;
											GameRank = 3;
										}
										else if ( GameMMR > 1500.0 )
										{
											GameCategory = 1.5f;
											GameRank = 2;
										}
										else if ( GameMMR < 800.0 )
										{
											GameRank = 0;
											GameCategory = 5.0f;
										}

										if ( gameduration > 0 )
										{
											GameDurations[ GameRank ] += ( float )gameduration;
											GameCounts[ GameRank ]++;
										}
										// Среднее время игры
										double GameSumDuration = GameDurations[ GameRank ] / GameCounts[ GameRank ];

										cout << ". Category:" << GameCategory << ". Rank:" << GameRank << endl;
										cout << "Game duration:" << gameduration << "(mean~:" << GameSumDuration << ")" << endl;

										//Командный параметр разброса ( Team ) –первая очередь
										double CommandArg = GameSCp / GameCategory;

										cout << "Command arg:" << CommandArg << endl;

										if ( CommandArg > 1.0 )
											CommandArg = 1.0;
										else if ( CommandArg < 0.0 )
											CommandArg = 0.0;
										cout << "Command arg after fix:" << CommandArg << endl;

										cout << "MMR default:";


										for ( int i = 0; i < num_players; i++ )
										{
											cout << ( int )ResultMMR[ i ] << ", ";
										}

										cout << endl;

										cout << "Start first pass.";

										cout << "Base mmr value:" << base_mmr_value;
										cout << ", team Sentinel mmr:" << team_min_mmr[ 0 ];
										cout << ", team Scourge mmr:" << team_min_mmr[ 1 ];
										cout << endl;



										for ( int i = 0; i < num_players; i++ )
										{
											if ( SentinelWinner )
											{
												// Победили Sentinel и игрок Sentinel
												if ( player_teams[ i ] == 0 )
												{
													cout << base_mmr_value << "/" << player_ratings[ i ] << "/" << team_min_mmr[ 0 ] << " .... ";
													ResultMMR[ i ] = base_mmr_value / ( player_ratings[ i ] / team_min_mmr[ 0 ] );
												}
												// Победили Sentinel и игрок Scourge
												else
												{
													cout << base_mmr_value << "*" << player_ratings[ i ] << "/" << team_max_mmr[ 1 ] << " .... ";
													ResultMMR[ i ] = base_mmr_value * ( player_ratings[ i ] / team_max_mmr[ 1 ] );
												}
											}
											else
											{
												// Победили Scourge и игрок Scourge
												if ( player_teams[ i ] == 1 )
												{
													cout << base_mmr_value << "/" << player_ratings[ i ] << "/" << team_min_mmr[ 1 ] << " .... ";
													ResultMMR[ i ] = base_mmr_value / ( player_ratings[ i ] / team_min_mmr[ 1 ] );
												}
												// Победили Scourge и игрок Sentinel
												else
												{
													cout << base_mmr_value << "*" << player_ratings[ i ] << "/" << team_max_mmr[ 0 ] << " .... ";
													ResultMMR[ i ] = base_mmr_value * ( player_ratings[ i ] / team_max_mmr[ 0 ] );
												}
											}
										}

										cout << "MMR after first pass:";


										for ( int i = 0; i < num_players; i++ )
										{
											cout << ( int )ResultMMR[ i ] << ", ";
										}


										cout << endl;


										for ( int i = 0; i < num_players; i++ )
										{
											if ( SentinelWinner )
											{
												// Если победил игрок Sentinel и разброс в пользу Scourge
												// добавить mmr 
												if ( player_teams[ i ] == 0 && !SentinelMMRbigger )
												{
													ResultMMR[ i ] += ResultMMR[ i ] * CommandArg;
												}
												// Если победил игрок Sentinel и разброс в пользу Sentinel
												// снять mmr 
												else if ( player_teams[ i ] == 0 && SentinelMMRbigger )
												{
													ResultMMR[ i ] -= ResultMMR[ i ] * CommandArg;
												}
												// Если проиграл игрок Scourge и разброс в пользу Sentinel
												// снять меньше mmr
												else if ( player_teams[ i ] == 1 && !SentinelMMRbigger )
												{
													ResultMMR[ i ] -= ResultMMR[ i ] * CommandArg;
												}
												// Если проиграл игрок Scourge и разброс в пользу Scourge
												// снять больше mmr
												else if ( player_teams[ i ] == 1 && SentinelMMRbigger )
												{
													ResultMMR[ i ] += ResultMMR[ i ] * CommandArg;
												}
											}
											else
											{
												// Если проиграл игрок Sentinel и разброс в пользу Scourge
												// снять меньше mmr
												if ( player_teams[ i ] == 0 && !SentinelMMRbigger )
												{
													ResultMMR[ i ] -= ResultMMR[ i ] * CommandArg;
												}
												// Если проиграл игрок Sentinel и разброс в пользу Sentinel
												// снять больше mmr
												else if ( player_teams[ i ] == 0 && SentinelMMRbigger )
												{
													ResultMMR[ i ] += ResultMMR[ i ] * CommandArg;
												}
												// Если победил игрок Scourge и разброс в пользу Sentinel
												// даль больше mmr
												else if ( player_teams[ i ] == 1 && !SentinelMMRbigger )
												{
													ResultMMR[ i ] += ResultMMR[ i ] * CommandArg;
												}
												// Если победил игрок Scourge и разброс в пользу Scourge
												// дать меньше mmr
												else if ( player_teams[ i ] == 1 && SentinelMMRbigger )
												{
													ResultMMR[ i ] -= ResultMMR[ i ] * CommandArg;
												}
											}
										}


										cout << ".MMR after pass 1.2:";


										for ( int i = 0; i < num_players; i++ )
										{
											cout << ( int )ResultMMR[ i ] << ", ";
										}


										cout << endl;



										//СКО 
										double SKOval = 0.0;
										for ( int i = 0; i < num_players; i++ )
										{
											SKOval += pow( player_ratings[ i ] - GameMMR, 2 );
										}
										SKOval /= num_players;
										SKOval = sqrt( SKOval );

										//СКО Вариация
										double SKOvariation = SKOval / GameMMR * 100.0;

										// Корректировочный коэффициент, зависящий от статуса игры
										float correctgamestatus = 1;
										int badplayers = abs( ( team_numplayers[ 0 ] - team_numleavers[ 0 ] )
											- ( team_numplayers[ 1 ] - team_numleavers[ 1 ] ) );



										switch ( badplayers )
										{
										case 1:
											correctgamestatus = 1.25f;
											break;
										case 2:
											correctgamestatus = 1.5f;
											break;
										case 3:
											correctgamestatus = 1.75f;
											break;
										case 4:
											correctgamestatus = 2.f;
											break;
										case 5:
											ignore = "(game 5x0)";
											correctgamestatus = 0.0f;
											break;
										default:
											break;
										}


										if ( correctgamestatus != 0.0f )
										{
											// Sentinel имеет больше игроков
											bool SentinelPlayersBigger = team_numplayers[ 0 ] > team_numplayers[ 1 ];


											if ( team_numplayers[ 0 ] != team_numplayers[ 1 ] )
											{
												for ( int i = 0; i < num_players; i++ )
												{
													if ( SentinelWinner )
													{
														// Количество Sentinel игроков больше и игрок Sentinel
														// Победили Sentinel
														// Дать меньше mmr
														if ( SentinelPlayersBigger && player_teams[ i ] == 0 )
														{
															ResultMMR[ i ] /= correctgamestatus;
														}
														// Количество Sentinel игроков меньше и игрок Sentinel
														// Победили Sentinel
														// Дать больше mmr
														else if ( !SentinelPlayersBigger && player_teams[ i ] == 0 )
														{
															ResultMMR[ i ] *= correctgamestatus;
														}
														// Количество Scourge игроков больше и игрок Scourge
														// Победили Sentinel
														// Снять больше mmr
														else if ( !SentinelPlayersBigger && player_teams[ i ] == 1 )
														{
															ResultMMR[ i ] *= correctgamestatus;
														}
														// Количество Scourge игроков меньше и игрок Scourge
														// Победили Sentinel
														// Снять меньше mmr
														else if ( SentinelPlayersBigger && player_teams[ i ] == 1 )
														{
															ResultMMR[ i ] /= correctgamestatus;
														}
													}
													else
													{
														// Количество Sentinel игроков больше и игрок Sentinel
														// Победили Scourge
														// Снять больше mmr
														if ( SentinelPlayersBigger && player_teams[ i ] == 0 )
														{
															ResultMMR[ i ] *= correctgamestatus;
														}
														// Количество Sentinel игроков меньше и игрок Sentinel
														// Победили Scourge
														// Снять меньше mmr
														else if ( !SentinelPlayersBigger && player_teams[ i ] == 0 )
														{
															ResultMMR[ i ] /= correctgamestatus;
														}
														// Количество Scourge игроков больше и игрок Scourge
														// Победили Scourge
														// Дать меньше mmr
														else if ( !SentinelPlayersBigger && player_teams[ i ] == 1 )
														{
															ResultMMR[ i ] /= correctgamestatus;
														}
														// Количество Scourge игроков меньше и игрок Scourge
														// Победили Scourge
														// Дать больше mmr
														else if ( SentinelPlayersBigger && player_teams[ i ] == 1 )
														{
															ResultMMR[ i ] *= correctgamestatus;
														}
													}
												}


												cout << "MMR after pass 2:";



												for ( int i = 0; i < num_players; i++ )
												{
													cout << ( int )ResultMMR[ i ] << ", ";
												}


												cout << endl;
											}
											else
											{
												cout << "Skip 2 pass!" << endl;
											}


											for ( int i = 0; i < num_players; i++ )
											{
												// Если победили Sentinel и игрок Sentinel 
												if ( SentinelWinner && player_teams[ i ] == 0 )
												{
													ResultMMR[ i ] *= correctgamestatus / individ_c[ i ];
												}
												// Если победили Scourge и игрок Scourge 
												else if ( !SentinelWinner && player_teams[ i ] == 1 )
												{
													ResultMMR[ i ] *= correctgamestatus / individ_c[ i ];
												}
												// Если победили Scourge и игрок Sentinel
												else if ( !SentinelWinner && player_teams[ i ] == 0 )
												{
													ResultMMR[ i ] *= correctgamestatus;
												}
												// Если победили Sentinel и игрок Scourge
												else if ( SentinelWinner && player_teams[ i ] == 1 )
												{
													ResultMMR[ i ] *= correctgamestatus;
												}
											}




											cout << "MMR after pass 3:";



											for ( int i = 0; i < num_players; i++ )
											{
												cout << ( int )ResultMMR[ i ] << ", ";
											}


											cout << endl;


											if ( gameduration > GameSumDuration / 2 )
											{
												float gamedurationcfval = 1.0f;

												if ( gameduration + 300 > GameSumDuration )
												{
													gamedurationcfval = 1.04f;
												}
												else if ( gameduration + 600 > GameSumDuration )
												{
													gamedurationcfval = 1.08f;
												}
												else if ( gameduration + 1200 > GameSumDuration )
												{
													gamedurationcfval = 1.16f;
												}


												for ( int i = 0; i < num_players; i++ )
												{
													if ( SentinelWinner )
													{
														// Sentinel победили и игрок Sentinel
														// Добавить mmr
														if ( player_teams[ i ] == 0 )
														{
															ResultMMR[ i ] *= gamedurationcfval;
														}
														// Sentinel победили и игрок Scourge
														// Снять меньше mmr
														else if ( player_teams[ i ] == 1 )
														{
															ResultMMR[ i ] /= gamedurationcfval;
														}
													}
													else
													{
														// Scourge победили и игрок Sentinel
														// Снять меньше mmr
														if ( player_teams[ i ] == 0 )
														{
															ResultMMR[ i ] /= gamedurationcfval;
														}
														// Scourge победили и игрок Scourge
														// добавить mmr
														else if ( player_teams[ i ] == 1 )
														{
															ResultMMR[ i ] *= gamedurationcfval;
														}
													}
												}



												cout << "MMR after pass 4:";



												for ( int i = 0; i < num_players; i++ )
												{
													cout << ( int )ResultMMR[ i ] << ", ";
												}


												cout << endl;

											}
											else
											{
												cout << "Skip 4 pass!" << endl;
											}



											for ( int i = 0; i < num_players; i++ )
											{
												// Дать больше mmr
												if ( SentinelWinner && player_teams[ i ] == 0 )
												{
													ResultMMR[ i ] *= CustomWinCF;
												}
												else if ( SentinelWinner && player_teams[ i ] == 1 )
												{
													ResultMMR[ i ] *= CustomLoseCF;
												}
												else if ( !SentinelWinner && player_teams[ i ] == 0 )
												{
													ResultMMR[ i ] *= CustomLoseCF;
												}
												else if ( !SentinelWinner && player_teams[ i ] == 1 )
												{
													ResultMMR[ i ] *= CustomWinCF;
												}
											}




											cout << "MMR after pass 4:";



											for ( int i = 0; i < num_players; i++ )
											{
												cout << ( int )ResultMMR[ i ] << ", ";
											}


											cout << endl;


											for ( int i = 0; i < num_players; i++ )
											{
												bool iswinner = false;

												// Если игрок Sentinel и победил Sentinel
												if ( player_teams[ i ] == 0 && SentinelWinner )
												{
													player_ratings[ i ] += ResultMMR[ i ];
													player_global_ratings[ i ] += ResultMMR[ i ];
													iswinner = true;
												}
												// Если игрок Scourge и победил Scourge
												else if ( player_teams[ i ] == 1 && !SentinelWinner )
												{
													player_ratings[ i ] += ResultMMR[ i ];
													player_global_ratings[ i ] += ResultMMR[ i ];
													iswinner = true;
												}
												// Если игрок Sentinel и победил Scourge
												else if ( player_teams[ i ] == 0 && !SentinelWinner )
												{
													player_ratings[ i ] -= ResultMMR[ i ];
													player_global_ratings[ i ] -= ResultMMR[ i ];
												}
												// Если игрок Scourge и победил Sentinel
												else if ( player_teams[ i ] == 1 && SentinelWinner )
												{
													player_ratings[ i ] -= ResultMMR[ i ];
													player_global_ratings[ i ] -= ResultMMR[ i ];
												}


												if ( accs[ i ] )
												{
													t_account * acc = accs[ i ];


													if ( Leaver[ i ] )
													{
														account_map_add_stats( acc, statstype, "leaves", 1 );
														player_ratings[ i ] -= 100;
														AddNewMessage( names[ i ], "You leave from game:" + gamename );
													}


													if ( player_ratings[ i ] <= 0 )
														player_ratings[ i ] = 1;
													if ( player_ratings[ i ] >= 50000 )
														player_ratings[ i ] = 50000;


													cout << "player [" << names[ i ] << "] " << ( iswinner ? "WIN!" : "LOSE." ) << " rating " << UTIL_ToString( ( uint32_t )old_player_ratings[ i ] ) << " -> " << UTIL_ToString( ( uint32_t )player_ratings[ i ] ) << endl;


													account_map_set_stats( acc, statstype, "mmr", player_ratings[ i ] );





													string gmresultstr = string( "Game \"" ) + gamename + "\" end!";
													gmresultstr += iswinner ? "You win!" : "You lose";
													gmresultstr += ( ResultMMR[ i ] < 0.0f ? "You lost:" : "Added:" ) + to_string( ( int )ResultMMR[ i ] ) + " points.";


													AddNewMessage( names[ i ], gmresultstr );

													if ( iswinner )
														account_map_add_stats( acc, statstype, "wins", 1 );
													else
														account_map_add_stats( acc, statstype, "loses", 1 );


													int curstreak = account_map_get_stats( acc, statstype, "streak" );
													int curminstreak = account_map_get_stats( acc, statstype, "minstreak" );
													int curmaxstreak = account_map_get_stats( acc, statstype, "maxstreak" );

													if ( iswinner )
													{
														if ( curstreak < 1 )
														{
															curstreak = 1;
															account_map_set_stats( acc, statstype, "streak", 1 );
														}
														else
														{
															curstreak++;
															account_map_add_stats( acc, statstype, "streak", 1 );
														}

														if ( curstreak > curmaxstreak )
															account_map_set_stats( acc, statstype, "maxstreak", curstreak );
													}
													else
													{
														if ( curstreak > -1 )
														{
															curstreak = -1;
															account_map_set_stats( acc, statstype, "streak", -1 );
														}
														else
														{
															curstreak--;
															account_map_add_stats( acc, statstype, "streak", -1 );
														}

														if ( curstreak < curminstreak )
															account_map_set_stats( acc, statstype, "minstreak", curstreak );
													}


													account_map_add_stats( acc, statstype, "kills", kills[ i ] );
													account_map_add_stats( acc, statstype, "deaths", deaths[ i ] );
													account_map_add_stats( acc, statstype, "assists", assists[ i ] );
												}

												if ( exists[ i ] )
												{
													string QUpdateScore = "UPDATE dota_elo_scores SET score=" + UTIL_ToString( player_global_ratings[ i ], 2 ) + " WHERE id=" + UTIL_ToString( rowids[ i ] );

													if ( mysql_real_query( Connection, QUpdateScore.c_str( ), QUpdateScore.size( ) ) != 0 )
													{
														cout << "error: " << mysql_error( Connection ) << endl;
														goto cleanup;
													}
												}
												else
												{
													string EscName = MySQLEscapeString( Connection, names[ i ] );
													string EscServer = MySQLEscapeString( Connection, servers[ i ] );
													string QInsertScore = "INSERT INTO dota_elo_scores ( name, server, score ) VALUES ( '" + EscName + "', '" + EscServer + "', " + UTIL_ToString( player_global_ratings[ i ], 2 ) + " )";

													if ( mysql_real_query( Connection, QInsertScore.c_str( ), QInsertScore.size( ) ) != 0 )
													{
														cout << "error: " << mysql_error( Connection ) << endl;
														goto cleanup;
													}
												}
											}
										}
										else
										{
											// Дать ливерам лив и снять очки
										}



									}
									else
										ignore = "(Bad Winner!)";

								}

							skipgame:

								if ( !ignore.empty( ) )
								{
									for ( int i = 0; i < 10; i++ )
									{
										string playername = names[ i ];
										if ( !playername.empty( ) )
										{
											AddNewMessage( playername, string( "[ERROR] Game \"" ) + gamename + "\" error:" + ignore );
											cout << "[ERROR] Game \"" << gamename << "\" error:" << ignore << endl;
										}
									}
								}


								string QInsertScored = "INSERT INTO dota_elo_games_scored ( gameid, duration, gamerank ) VALUES ( " + UTIL_ToString( GameID ) + ", " + UTIL_ToString( ( int )gameduration ) + ", " + UTIL_ToString( GameRank ) + " )";

								if ( mysql_real_query( Connection, QInsertScored.c_str( ), QInsertScored.size( ) ) != 0 )
								{
									cout << "error: " << mysql_error( Connection ) << endl;
									goto cleanup;
								}
							}
							else
							{
								cout << "error: " << mysql_error( Connection ) << endl;
								goto cleanup;
							}

						}
					}

					//	cout << "copying dota elo scores to scores table" << endl;

					string QCopyScores1 = "DELETE FROM scores WHERE category='dota_elo'";
					string QCopyScores2 = "INSERT INTO scores ( category, name, server, score ) SELECT 'dota_elo', name, server, score FROM dota_elo_scores";

					if ( mysql_real_query( Connection, QCopyScores1.c_str( ), QCopyScores1.size( ) ) != 0 )
					{
						cout << "error: " << mysql_error( Connection ) << endl;
						goto cleanup;
					}

					if ( mysql_real_query( Connection, QCopyScores2.c_str( ), QCopyScores2.size( ) ) != 0 )
					{
						cout << "error: " << mysql_error( Connection ) << endl;
						goto cleanup;
					}

					//cout << "committing transaction" << endl;

					string QCommit = "COMMIT";

					if ( mysql_real_query( Connection, QCommit.c_str( ), QCommit.size( ) ) != 0 )
					{
						cout << "error: " << mysql_error( Connection ) << endl;
						goto cleanup;
					}

					//cout << "done" << endl;
				}

			}



		cleanup:
			for ( StatsUpdateDBList & currentdb : statsdblist )
			{
				if ( currentdb.connection != NULL )
				{
					mysql_close( currentdb.connection );
				}
			}

			statsdblist.clear( );

			std::this_thread::sleep_for( std::chrono::seconds( 5 ) );
			UpdatePlayerStatsThread( );

		}


		static bool update_dota_elo_connected = false;

		extern void UpdatePlayerStatsTick( )
		{
			ProcessAllMessages( );
			// Тут буду отправлять подготовленную статистику всем игрокам.
			// 
		}


		static double ztable[ 301 ] = {
			0.0013498974275996112,
			0.0013948866316060848,
			0.0014412413183508832,
			0.0014889981559605414,
			0.001538194637014656,
			0.001588869092024936,
			0.0016410607029800905,
			0.0016948095169527777,
			0.001750156459762453,
			0.0018071433496901768,
			0.0018658129112387178,
			0.0019262087889329593,
			0.0019883755611550535,
			0.002052358754007555,
			0.0021182048551989241,
			0.0021859613279451295,
			0.0022556766248800209,
			0.002327400201968588,
			0.0024011825324152247,
			0.0024770751205606678,
			0.0025551305157595627,
			0.0026354023262312176,
			0.0027179452328760512,
			0.0028028150030493526,
			0.0028900685042839713,
			0.0029797637179544423,
			0.0030719597528726106,
			0.0031667168588071504,
			0.0032640964399167638,
			0.0033641610680892331,
			0.0034669744961752791,
			0.0035726016711090103,
			0.0036811087469041937,
			0.003792563097516688,
			0.0039070333295626036,
			0.0040245892948822526,
			0.0041453021029385084,
			0.0042692441330391961,
			0.0043964890463726869,
			0.0045271117978451514,
			0.0046611886477079811,
			0.0047987971729647194,
			0.0049400162785444568,
			0.0050849262082311442,
			0.005233608555335667,
			0.0053861462730993015,
			0.0055426236848157839,
			0.0057031264936595605,
			0.0058677417922075592,
			0.0060365580716419398,
			0.0062096652306207201,
			0.0063871545838029564,
			0.0065691188700164327,
			0.00675565226005298,
			0.0069468503640799373,
			0.0071428102386527081,
			0.0073436303933163138,
			0.0075494107967811197,
			0.0077602528826604122,
			0.0079762595547552273,
			0.0081975351918731065,
			0.0084241856521673486,
			0.0086563182769826552,
			0.0088940418941937938,
			0.0091374668210232324,
			0.0093867048663250907,
			0.0096418693323204185,
			0.0099030750157714231,
			0.010170438208581323,
			0.01044407669780506,
			0.010724109765059997,
			0.011010658185321376,
			0.01130384422509112,
			0.011603791639926586,
			0.011910625671316843,
			0.012224473042894923,
			0.012545461955972614,
			0.012873722084387529,
			0.01320938456864984,
			0.0135525820093787,
			0.013903448460015899,
			0.01426211941880845,
			0.014628731820047924,
			0.015003424024558676,
			0.015386335809424667,
			0.015777608356947082,
			0.016177384242823556,
			0.016585807423542764,
			0.017003023222986247,
			0.017429178318231431,
			0.017864420724550745,
			0.018308899779600374,
			0.018762766126796393,
			0.019226171697872219,
			0.019699269694617383,
			0.0201822145697933,
			0.020675162007226688,
			0.021178268901080211,
			0.021691693334300266,
			0.022215594556245999,
			0.022750132959500013,
			0.023295467792401992,
			0.023851764485976978,
			0.024419185453399439,
			0.024997895303140116,
			0.025588059631423632,
			0.026189844992999645,
			0.026803418870745443,
			0.027428949644120293,
			0.028066606556483631,
			0.028716559681294385,
			0.029378979887203005,
			0.030054038802049154,
			0.03074190877577504,
			0.031442762842267435,
			0.032156774680134892,
			0.032884118572433452,
			0.033624969365345592,
			0.034379502425825104,
			0.03514789359821513,
			0.035930319159845292,
			0.036726955775621406,
			0.03753798045161244,
			0.038363570487644771,
			0.03920390342891289,
			0.040059157016615887,
			0.040929509137628106,
			0.041815137773216382,
			0.042716220946811212,
			0.043632936670844613,
			0.044565462892666607,
			0.045513977439549824,
			0.046478657962796821,
			0.047459681880963556,
			0.048457226322209723,
			0.049471468065793944,
			0.050502583482726904,
			0.05155074847559854,
			0.052616138417595104,
			0.053698928090725517,
			0.054799291623271407,
			0.055917402426483021,
			0.057053433130537734,
			0.058207555519782628,
			0.059379940467282288,
			0.06057075786869176,
			0.061780176575479129,
			0.063008364327519828,
			0.064255487685086599,
			0.065521711960259965,
			0.066807201147784379,
			0.06811211785539717,
			0.069436623233655514,
			0.070780876905290446,
			0.072145036894116166,
			0.073529259553522797,
			0.074933699494582784,
			0.076358509513802353,
			0.077803840520547174,
			0.079269841464176316,
			0.080756659260914632,
			0.082264438720498823,
			0.083793322472628884,
			0.085343450893260786,
			0.086914962030774756,
			0.088507991532053798,
			0.090122672568510254,
			0.091759135762094657,
			0.093417509111326291,
			0.095097917917381369,
			0.096800484710277135,
			0.098525329175190968,
			0.10027256807895296,
			0.10204231519675117,
			0.10383468123908968,
			0.10564977377903884,
			0.10748769717981899,
			0.10934855252275771,
			0.11123243753566175,
			0.11313944652164465,
			0.11506967028845272,
			0.11702319607832912,
			0.11900010749846002,
			0.12100048445204298,
			0.1230244030700216,
			0.12507193564352681,
			0.12714315055706898,
			0.12923811222252168,
			0.13135688101394061,
			0.13349951320325965,
			0.13566606089690608,
			0.13785657197337792,
			0.14007109002182488,
			0.1423096542816758,
			0.14457229958335321,
			0.14685905629011764,
			0.14916995024108309,
			0.15150500269544415,
			0.15386423027795648,
			0.15624764492571003,
			0.15865525383623708,
			0.1610870594169927,
			0.1635430592362484,
			0.16602324597543822,
			0.16852760738299372,
			0.17105612622970845,
			0.1736087802656674,
			0.17618554217877852,
			0.17878637955494342,
			0.18141125483990161,
			0.18406012530278404,
			0.18673294300140969,
			0.18942965474935874,
			0.192150202084855,
			0.19489452124148976,
			0.19766254312081705,
			0.2004541932668526,
			0.2032693918425032,
			0.20610805360795725,
			0.20897008790106242,
			0.21185539861971758,
			0.21476388420630449,
			0.21769543763418348,
			0.22064994639627722,
			0.22362729249576385,
			0.22662735243890197,
			0.22964999723000723,
			0.23269509236859903,
			0.23576249784873582,
			0.23885206816055621,
			0.24196365229403949,
			0.24509709374500249,
			0.24825223052334316,
			0.25142889516354494,
			0.25462691473745036,
			0.25784611086931519,
			0.26108629975314901,
			0.26434729217234998,
			0.2676288935216391,
			0.27093090383129681,
			0.27425311779370587,
			0.27759532479220023,
			0.28095730893222093,
			0.28433884907477647,
			0.28773971887220517,
			0.29115968680623477,
			0.29459851622833394,
			0.29805596540234741,
			0.30153178754940763,
			0.30502573089511137,
			0.30853753871895068,
			0.31206694940598489,
			0.31561369650073978,
			0.31917750876331719,
			0.32275811022769929,
			0.3263552202622283,
			0.32996855363224042,
			0.33359782056483578,
			0.33724272681575818,
			0.34090297373836298,
			0.34457825835464639,
			0.34826827342831013,
			0.35197270753983279,
			0.35569124516351835,
			0.35942356674649178,
			0.36316934878960927,
			0.36692826393024869,
			0.37069998102694834,
			0.37448416524585493,
			0.37828047814894639,
			0.38208857778398969,
			0.38590811877619485,
			0.38973875242152484,
			0.39358012678161919,
			0.39743188678028851,
			0.40129367430153673,
			0.40516512828906587,
			0.40904588484721671,
			0.41293557734330005,
			0.41683383651126915,
			0.4207402905566856,
			0.42465456526292905,
			0.42857628409859905,
			0.43250506832605989,
			0.43644053711107383,
			0.44038230763347214,
			0.44432999519880867,
			0.44828321335094301,
			0.45224157398549714,
			0.45620468746413118,
			0.46017216272958139,
			0.46414360742140437,
			0.46811862799236997,
			0.47209682982544549,
			0.47607781735131321,
			0.4800611941663635,
			0.4840465631511035,
			0.48803352658892385,
			0.49202168628516352,
			0.49601064368641357,
			0.5
		};

		/* Looks like ELO uses a standard deviation of 200. */
#define STD_DEV (double)200.0
#define INCR (double)(1.0 / 100.0)

		static double get_normal_cdf( double z )
		{
			/* This may not be worth the trouble... */
			if ( z < 0 )
				return 1 - get_normal_cdf( -z );

			/* outside of our range */
			if ( z > 3 )
				return 1.0;

			/* There are 100 entries per std-dev, and they're in reverse order,
			AND they're for the lower end of the CDF. */
			return 1 - ztable[ 300 - ( int )( z * 100.0 + 0.5 ) ];
		}


		static double elo_integrate_part( double val, double incr,
			int num, int i, double *ratings )
		{
			double prob, myrating = ratings[ i ];
			int j;

			prob = get_normal_cdf( val + incr ) - get_normal_cdf( val );

			for ( j = 0; j < num; j++ ) {
				double prob2;
				double ratingdiff = myrating - ratings[ j ];

				if ( j == i )
					continue;

				prob2 = get_normal_cdf( val + ratingdiff );

				prob *= prob2;
			}

			return prob;
		}

		/* Returns ELO-style probability of winning determined by integration num is
		number of players i is player being considered ratings is ratings of all
		players */
		static double elo_integrate_all( int num, int i, double *ratings )
		{
			double prob = 0, p;

			for ( p = -3; p < 3; p += INCR ) {
				prob += elo_integrate_part( p, INCR, num, i, ratings );
			}

			return prob;
		}

		static void elo_compute_expectations( int num, float *ratings, float *probs )
		{
			double *myratings = new double[ num ];
			int i;
			double sum = 0;

			/* "normalize" the ratings */
			for ( i = 0; i < num; i++ ) {
				myratings[ i ] = ratings[ i ];
				myratings[ i ] /= STD_DEV;
			}

			for ( i = 0; i < num; i++ ) {
				probs[ i ] = elo_integrate_all( num, i, myratings );
				sum += probs[ i ];
			}

			/* dbg_msg(GGZ_DBG_STATS, "Probabilities sum to %f; normalizing.", sum); */
			for ( i = 0; i < num; i++ )
				probs[ i ] /= sum;

			delete[ ] myratings;
		}

		void elo_recalculate_ratings( int num_players, float *player_ratings,
			int *player_teams, int num_teams,
			float *team_ratings, float *team_winners )
		{
			float *team_probs = new float[ num_teams ];
			int i;

			/* Calculate the probability for each player to win, ELO-style. */
			elo_compute_expectations( num_teams, team_ratings, team_probs );

			/* Debugging data */
			for ( i = 0; i < num_players; i++ ) {
				int team = num_teams > 0 ? player_teams[ i ] : i;
				/* dbg_msg(GGZ_DBG_STATS,
				"Player %d has rating %f, expectation %f.", i,
				team_ratings[team], team_probs[team]); */
			}

			/* Calculate new ratings for all players. */
			for ( i = 0; i < num_players; i++ ) {
				int team = num_teams > 0 ? player_teams[ i ] : i;
				float K, diff;

				/* FIXME: this is the chess distribution; games should be
				able to set their own. */
				if ( player_ratings[ i ] < 2000 )
					K = 30.0;
				else if ( player_ratings[ i ] > 2400 )
					K = 10.0;
				else
					K = 130.0 - player_ratings[ i ] / 20.0;

				diff = K * ( team_winners[ team ] - team_probs[ team ] );
				player_ratings[ i ] += diff;
				/* dbg_msg(GGZ_DBG_STATS,
				"Player %d has new rating %f (slope %f).", i,
				player_ratings[i], K); */
			}

			delete[ ] team_probs;
		}


	}
}