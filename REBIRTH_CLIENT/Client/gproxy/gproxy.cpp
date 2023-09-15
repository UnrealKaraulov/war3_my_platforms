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
#include "config.h"
#include "socket.h"
#include "commandpacket.h"
#include "bnetprotocol.h"
#include "bnet.h"
#include "gameprotocol.h"
#include "gpsprotocol.h"
#include <mutex>
#include <signal.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>           
#ifdef WIN32
#include <windows.h>
#include <winsock.h>
#endif

#include <time.h>

#ifndef WIN32
#include <sys/time.h>
#endif



#include <BlpConv.h>







pProcessCommandsCallback CommandsCallback = NULL;


bool __stdcall ProcessCommandsCallback(const char* text, const char* arg1,
	const char* arg2, const char* arg3, const char* arg4, const char* arg5, const char* arg6, const char* arg7, const char* arg8, const char* arg9, const char* arg10, const char* arg11)
{
	if (CommandsCallback != NULL)
	{
		CommandsCallback(text, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11);
		return true;
	}

	return false;
}



bool gCurses = false;
vector<string> gMainBuffer;
string gInputBuffer;
string gChannelName;
vector<string> gChannelUsers;

string gLogFile = "w3proxy.txt";
CGProxy* gGProxy = NULL;

uint32_t GetTime()
{
	return GetTicks() / 1000;
}

uint32_t GetTicks()
{
#ifdef WIN32
	return timeGetTime();
#elif __APPLE__
	uint64_t current = mach_absolute_time();
	static mach_timebase_info_data_t info = { 0, 0 };
	// get timebase info
	if (info.denom == 0)
		mach_timebase_info(&info);
	uint64_t elapsednano = current * (info.numer / info.denom);
	// convert ns to ms
	return elapsednano / 1e6;
#else
	uint32_t ticks;
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	ticks = t.tv_sec * 1000;
	ticks += t.tv_nsec / 1000000;
	return ticks;
#endif
}

void LOG_Print(string message)
{
	if (!gLogFile.empty())
	{
		ofstream Log;
		Log.open(gLogFile.c_str(), ios::app);

		if (!Log.fail())
		{
			time_t Now = time(NULL);
			string Time = asctime(localtime(&Now));

			// erase the newline

			Time.erase(Time.size() - 1);
			Log << "[" << Time << "] " << message << endl;
			Log.close();
		}
	}
}
std::mutex CONSOLE_Print_mutex;

void CONSOLE_Print(string message, bool log)
{
	CONSOLE_Print_mutex.lock();
	CONSOLE_PrintNoCRLF(message, log);
	LOG_Print(message);
	CONSOLE_Print_mutex.unlock();
	//if ( !gCurses )
	//	cout << endl;
}

void CONSOLE_PrintNoCRLF(string message, bool log)
{
	gMainBuffer.push_back(message);

	if (gMainBuffer.size() > 512)
		gMainBuffer.erase(gMainBuffer.begin());

}


void __stdcall LogMessage(const char* message)
{
	if (message && message[0] != '\0')
		CONSOLE_Print(message, true);
}

void CONSOLE_ChangeChannel(string channel)
{
	ProcessCommandsCallback("/channelname", channel.c_str());
	gChannelName = channel;
}

void CONSOLE_AddChannelUser(string name)
{
	for (vector<string> ::iterator i = gChannelUsers.begin(); i != gChannelUsers.end(); i++)
	{
		if (*i == name)
			return;
	}

	gChannelUsers.push_back(name);


	ProcessCommandsCallback("/addchatuser", name.c_str());
}

void CONSOLE_RemoveChannelUser(string name)
{
	for (vector<string> ::iterator i = gChannelUsers.begin(); i != gChannelUsers.end(); )
	{
		if (*i == name)
			i = gChannelUsers.erase(i);
		else
			i++;
	}

	ProcessCommandsCallback("/removechatuser", name.c_str());

}

void CONSOLE_RemoveChannelUsers()
{
	gChannelUsers.clear();
	ProcessCommandsCallback("/clearchatusers", "");
}

//
// main
//	

BOOLEAN WINAPI DllMain(IN HINSTANCE hDllHandle,
	IN DWORD     nReason,
	IN LPVOID    Reserved)
{

	switch (nReason)
	{
	case DLL_PROCESS_ATTACH:

		//  For optimization.
		gMainBuffer.push_back("Warcraft 3 proxy client loaded.");
		gMainBuffer.push_back("Ghost++ proxy client loaded.");
		DisableThreadLibraryCalls(hDllHandle);

		break;

	case DLL_PROCESS_DETACH:

		break;
	}


	return TRUE;
}


PvPGN_Login_Status m_login_status = Login_Status_NONE;

PvPGN_Login_Status __stdcall LoginStatus()
{
	return m_login_status;
}


void __stdcall ResetLoginStatus()
{
	m_login_status = Login_Status_NONE;
}



void __stdcall GetLogData(TextCallback handler)
{
	for (const auto& s : gMainBuffer)
	{
		handler(s.c_str());
	}
}


void __stdcall SetCommandsCallback(pProcessCommandsCallback handler)
{
	CommandsCallback = handler;

	for (const auto& s : MapHostStructList)
	{
		ProcessCommandsCallback("/addnewmap", s.MapName.c_str(), s.MapHost.c_str(), s.MapFileName.c_str(),
			s.MapCategory.c_str(), /*MapModes.c_str( ), MapPlayers.c_str( ),*/ s.crc32.c_str(), s.ForStats ? "true" : "false", "", "", "", "", s.MapFileName.c_str());;
	}
	MapHostStructList.clear();
	//ProcessCommandsCallback( "/addtext", " TEST :) !!! :D " );
}




void __stdcall SendClientPacket(uint32_t command, uint32_t var1,
	uint32_t var2, uint32_t var3, uint32_t var4, const char* strvar, const char* strvar2, const char* strvar3, const char* strvar4)
{
	//MessageBoxA( 0, "Send", "Send", 0 );
	if (gGProxy && gGProxy->m_BNET)
	{
		gGProxy->m_BNET->SendAHPacket(ANTIHACK_VERSION, command, var1, var2, var3, var4,
			strvar ? strvar : "", strvar2 ? strvar2 : "", strvar3 ? strvar3 : "", strvar4 ? strvar4 : "");
	}
	//	MessageBoxA( 0, "SendOK", "SendOK", 0 );
}


void SendAHPackets()
{
	if (gGProxy && gGProxy->m_BNET && gGProxy->m_BNET->m_Protocol)
		gGProxy->m_BNET->m_Protocol->RECEIVE_SID_AH_PACKET(BYTEARRAY());
}


void __stdcall Renew()
{
	SendAHPackets();
}

std::string GlobalUsername;
std::string GlobalPassword;
bool IsRegisterAccount = false;;
HANDLE hmutex;
bool AllreadyInitialized = false;

int __stdcall InitializeClientProxy(BOOL istft, const char* War3Path,
	const char* Server,
	const char* Username,
	const char* Password,
	unsigned int War3Version,
	unsigned short Port,
	const char* _EXEVersion,
	const char* _EXEVersionHash,
	bool IsRegister
	)
{
	ShutDownDOOOOWN = false;
	IsRegisterAccount = IsRegister;

	if (!AllreadyInitialized)
	{
		hmutex = CreateMutex(NULL, TRUE, L"{1F1F0EF4-3BA2-62fe-AC3F-24F0526BDE64}");
		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			MessageBox(0, L"only one instance at a time", L"Error", 0);
			return 0;
		}
	}

	if (AllreadyInitialized)
	{
		if (gGProxy)
		{
			ShutDownDOOOOWN = true;
			MessageBoxA(0, "Try again!", "Try again", 0);
		}
	}

	AllreadyInitialized = true;

	m_login_status = Login_Status_NONE;

	GlobalUsername = Username ? Username : "";
	GlobalPassword = Password ? Password : "";



	CONSOLE_Print("[Client] starting up with username:" + GlobalUsername);

	if (!UTIL_FileExists(War3Path + string("WAR.bin")))
	{
		CONSOLE_Print("[Client] Error, war3 not found at " + std::string(War3Path) );
		MessageBoxA(0, (std::string("No WAR.bin in war3 folder! Path:") + War3Path + string("WAR.bin")).c_str(), "Tatal error!", 0);
		return -5;
	}

#ifndef WIN32
	// disable SIGPIPE since some systems like OS X don't define MSG_NOSIGNAL

	signal(SIGPIPE, SIG_IGN);
#endif

#ifdef WIN32
	// initialize winsock

	CONSOLE_Print("[Client] starting winsock");
	WSADATA wsadata;

	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)
	{
		CONSOLE_Print("[Client] error starting winsock");
		return 1;
	}

	// increase process priority

	CONSOLE_Print("[Client] setting critical process priority");

	SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
#endif

	BYTEARRAY EXEVersion;
	BYTEARRAY EXEVersionHash;
	EXEVersion = UTIL_ExtractNumbers(_EXEVersion, 4);
	EXEVersionHash = UTIL_ExtractNumbers(_EXEVersionHash, 4);
	gCurses = true;

	CONSOLE_Print("  Type /help at any time for help.", false);
	CONSOLE_Print("  Press any key to continue.", false);
	CONSOLE_Print("", false);
	// initialize gproxy

	gGProxy = new CGProxy(istft, War3Path, "FFFFFFFFFFFFFFFFFFFFFFFFF0", "FFFFFFFFFFFFFFFFFFFFFFFFF1", Server, Username, Password, "chat", War3Version, Port, EXEVersion, EXEVersionHash, "pvpgn");

	while (1)
	{
		if (gGProxy->Update(40000))
			break;
	}

	// shutdown gproxy

	CONSOLE_Print("[Client] shutting down");
	if (gGProxy)
		delete gGProxy;
	gGProxy = NULL;

#ifdef WIN32
	// shutdown winsock

	CONSOLE_Print("[Client] shutting down winsock");
	WSACleanup();
#endif
	AllreadyInitialized = false;

	ReleaseMutex(hmutex);
	CloseHandle(hmutex);
	hmutex = NULL;
	return 0;
}
void __stdcall TerminateProxy()
{
	ShutDownDOOOOWN = true;
}

bool NeedAnnounce = true;
void __stdcall AnnounceGame()
{
	CONSOLE_Print("AnnounceGame");
	if (gGProxy && gGProxy->m_GameProtocol)
	{
		for (vector<CIncomingGameHost*> ::iterator i = gGProxy->m_Games.begin(); i != gGProxy->m_Games.end(); i++)
			gGProxy->m_UDPSocket->Broadcast(6113, gGProxy->m_GameProtocol->SEND_W3GS_DECREATEGAME((*i)->GetUniqueGameID()));
	}
	if (gGProxy)
	{
		for (vector<CIncomingGameHost*> ::iterator i = gGProxy->m_Games.begin(); i != gGProxy->m_Games.end(); i++)
			delete* i;
		gGProxy->m_Games.clear();
	}
	CIncomingGameHost::NextUniqueGameID = 1;
	NeedAnnounce = true;
	CONSOLE_Print("AnnounceGame OK");
}
void __stdcall StopAnnounceGame()
{
	m_PublicGameFilter.clear();
	NeedAnnounce = false;
}

void __stdcall ProcessCommand(const char* inputcmd)
{
	if (!gGProxy || !gGProxy->m_BNET || !inputcmd || inputcmd[0] == '\0')
		return;

	string Command = inputcmd;
	transform(Command.begin(), Command.end(), Command.begin(), (int(*)(int))tolower);

	CONSOLE_Print("Command: \"" + Command + "\"");

	/*if ( Command == "/commands" )
	{
		CONSOLE_Print( ">>> /commands" );
		CONSOLE_Print( "", false );
		CONSOLE_Print( "  In the War3Lan console:", false );
		CONSOLE_Print( "   /commands           : show command list", false );
		CONSOLE_Print( "   /exit or /quit      : close War3Lan", false );
		CONSOLE_Print( "   /filter <f>         : start filtering public game names for <f>", false );
		CONSOLE_Print( "   /filteroff          : stop filtering public game names", false );
		CONSOLE_Print( "   /game <gamename>    : look for a specific game named <gamename>", false );
		CONSOLE_Print( "   /help               : show help text", false );
		CONSOLE_Print( "   /public             : enable listing of public games", false );
		CONSOLE_Print( "   /publicoff          : disable listing of public games", false );
		CONSOLE_Print( "   /r <message>        : reply to the last received whisper", false );
#ifdef WIN32
		CONSOLE_Print( "   4              : start warcraft 3", false );
#endif
		CONSOLE_Print( "   /version            : show version text", false );
		CONSOLE_Print( "", false );
		CONSOLE_Print( "  In game:", false );
		CONSOLE_Print( "   /re <message>       : reply to the last received whisper", false );
		CONSOLE_Print( "   /sc                 : whispers \"spoofcheck\" to the game host", false );
		CONSOLE_Print( "   /status             : show status information", false );
		CONSOLE_Print( "   /w <user> <message> : whispers <message> to <user>", false );
		CONSOLE_Print( "", false );
	}
	else*/ /*if ( Command.size( ) >= 9 && Command.substr( 0, 8 ) == "/filter " )
	{
		string Filter = gInputBuffer.substr( 8 );

		if ( !Filter.empty( ) && Filter.size( ) <= 31 )
		{
			gGProxy->m_BNET->SetPublicGameFilter( Filter );
			CONSOLE_Print( "[SERVER] started filtering public game names for \"" + Filter + "\"" );
		}
	}*/
	//else if ( Command == "/filteroff" )
	//{
	//	gGProxy->m_BNET->SetPublicGameFilter( string( ) );
	//	CONSOLE_Print( "[SERVER] stopped filtering public game names" );
	//}
	//else if ( Command.size( ) >= 7 && Command.substr( 0, 6 ) == "/game " )
	//{
	//	string GameName = gInputBuffer.substr( 6 );

	//	if ( !GameName.empty( ) && GameName.size( ) <= 31 )
	//	{
	//		gGProxy->m_BNET->SetSearchGameName( GameName );
	//		CONSOLE_Print( "[SERVER] looking for a game named \"" + GameName + "\" for up to two minutes" );
	//	}
	//}else
	if (Command == "/help")
	{
		CONSOLE_Print(">>> /help");
		CONSOLE_Print("", false);
		CONSOLE_Print("  War3Lan connects to battle.net and looks for games for you to join.", false);
		CONSOLE_Print("  If War3Lan finds any they will be listed on the Warcraft III LAN screen.", false);
		CONSOLE_Print("  To join a game, simply open Warcraft III and wait at the LAN screen.", false);
		CONSOLE_Print("  Standard games will be white and War3Lan enabled games will be blue.", false);
		CONSOLE_Print("  Note: You must type \"/public\" to enable listing of public games.", false);
		CONSOLE_Print("", false);
		CONSOLE_Print("  If you want to join a specific game, type \"/game <gamename>\".", false);
		CONSOLE_Print("  War3Lan will look for that game for up to two minutes before giving up.", false);
		CONSOLE_Print("  This is useful for private games.", false);
		CONSOLE_Print("", false);
		CONSOLE_Print("  Please note:", false);
		CONSOLE_Print("  War3Lan will join the game using your battle.net name, not your LAN name.", false);
		CONSOLE_Print("  Other players will see your battle.net name even if you choose another name.", false);
		CONSOLE_Print("  This is done so that you can be automatically spoof checked.", false);
		CONSOLE_Print("", false);
		CONSOLE_Print("  Type \"/commands\" for a full command list.", false);
		CONSOLE_Print("", false);
	}/*
	else if ( Command == "/public" || Command == "/publicon" || Command == "/public on" || Command == "/list" || Command == "/liston" || Command == "/list on" )
	{
		gGProxy->m_BNET->SetListPublicGames( true );
		CONSOLE_Print( "[SERVER] listing of public games enabled" );
	}
	else if ( Command == "/publicoff" || Command == "/public off" || Command == "/listoff" || Command == "/list off" )
	{
		gGProxy->m_BNET->SetListPublicGames( false );
		CONSOLE_Print( "[SERVER] listing of public games disabled" );
	}*/
	else if (Command.size() >= 4 && Command.substr(0, 3) == "/r ")
	{
		if (!gGProxy->m_BNET->GetReplyTarget().empty())
			gGProxy->m_BNET->QueueChatCommand(gInputBuffer.substr(3), gGProxy->m_BNET->GetReplyTarget(), true);
		else
			CONSOLE_Print("[SERVER] nobody has whispered you yet");
	}
	else if (Command == "/version")
		CONSOLE_Print("[Client] War3Lan Version " + gGProxy->m_Version);
	else if (Command == "/announcegame")
	{
		AnnounceGame();
	}
	else if (Command == "/stopannouncegame")
	{
		StopAnnounceGame();
	}
	else
		gGProxy->m_BNET->QueueChatCommand(inputcmd);

	gInputBuffer.clear();
}

//
// CGProxy
//

CGProxy::CGProxy(bool nTFT, string nWar3Path, string nCDKeyROC, string nCDKeyTFT, string nServer, string nUsername, string nPassword, string nChannel, uint32_t nWar3Version, uint16_t nPort, BYTEARRAY nEXEVersion, BYTEARRAY nEXEVersionHash, string nPasswordHashType)
{
	m_Version = "Karaulov. Custom build. 2018";
	m_LocalServer = new CTCPServer();
	m_LocalSocket = NULL;
	m_RemoteSocket = new CTCPClient();
	m_RemoteSocket->SetNoDelay(true);
	m_UDPSocket = new CUDPSocket();
	m_UDPSocket->SetBroadcastTarget("127.0.0.1");
	m_GameProtocol = new CGameProtocol(this);
	m_GPSProtocol = new CGPSProtocol();
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
	m_BNET = new CBNET(this, m_Server, string(), 0, 0, m_CDKeyROC, m_CDKeyTFT, "RUS", "BAD", m_Username, m_Password, m_Channel, m_War3Version, nEXEVersion, nEXEVersionHash, nPasswordHashType, 200);
	m_LocalServer->Listen(string(), m_Port);
	CONSOLE_Print("[Client] War3Lan Version " + m_Version);
}

void __stdcall JoinChannelCommand(const char* channelname)
{
	if (gGProxy->m_BNET)
	{
		gGProxy->m_BNET->SendJoinChannel(channelname);
	}
}


CGProxy :: ~CGProxy()
{
	for (vector<CIncomingGameHost*> ::iterator i = m_Games.begin(); i != m_Games.end(); i++)
		m_UDPSocket->Broadcast(6113, m_GameProtocol->SEND_W3GS_DECREATEGAME((*i)->GetUniqueGameID()));

	delete m_LocalServer;
	delete m_LocalSocket;
	delete m_RemoteSocket;
	delete m_UDPSocket;
	ShutDownDOOOOWN = true;
	//delete m_BNET;

	for (vector<CIncomingGameHost*> ::iterator i = m_Games.begin(); i != m_Games.end(); i++)
		delete* i;

	m_Games.clear();

	delete m_GameProtocol;
	delete m_GPSProtocol;

	while (!m_LocalPackets.empty())
	{
		delete m_LocalPackets.front();
		m_LocalPackets.pop();
	}

	while (!m_RemotePackets.empty())
	{
		delete m_RemotePackets.front();
		m_RemotePackets.pop();
	}

	while (!m_PacketBuffer.empty())
	{
		delete m_PacketBuffer.front();
		m_PacketBuffer.pop();
	}
}

bool ShutDownDOOOOWN = false;
bool CGProxy::Update(long usecBlock)
{
	unsigned int NumFDs = 0;

	// take every socket we own and throw it in one giant select statement so we can block on all sockets

	int nfds = 0;
	fd_set fd;
	fd_set send_fd;
	FD_ZERO(&fd);
	FD_ZERO(&send_fd);

	// 1. the battle.net socket

	NumFDs += m_BNET->SetFD(&fd, &send_fd, &nfds);

	// 2. the local server

	m_LocalServer->SetFD(&fd, &send_fd, &nfds);
	NumFDs++;

	// 3. the local socket

	if (m_LocalSocket)
	{
		m_LocalSocket->SetFD(&fd, &send_fd, &nfds);
		NumFDs++;
	}

	// 4. the remote socket

	if (!m_RemoteSocket->HasError() && m_RemoteSocket->GetConnected())
	{
		m_RemoteSocket->SetFD(&fd, &send_fd, &nfds);
		NumFDs++;
	}

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = usecBlock;

	struct timeval send_tv;
	send_tv.tv_sec = 0;
	send_tv.tv_usec = 0;

#ifdef WIN32
	select(1, &fd, NULL, NULL, &tv);
	select(1, NULL, &send_fd, NULL, &send_tv);
#else
	select(nfds + 1, &fd, NULL, NULL, &tv);
	select(nfds + 1, NULL, &send_fd, NULL, &send_tv);
#endif

	if (NumFDs == 0)
		MILLISLEEP(50);

	if (m_BNET->Update(&fd, &send_fd))
		return true;

	//
	// accept new connections
	//

	CTCPSocket* NewSocket = m_LocalServer->Accept(&fd);

	if (NewSocket)
	{
		if (m_LocalSocket)
		{
			delete NewSocket;
		}
		else
		{
			CONSOLE_Print("[Client] local player connected");
			m_LocalSocket = NewSocket;
			m_LocalSocket->SetNoDelay(true);
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
			m_JoinedName.clear();
			m_HostName.clear();

			while (!m_PacketBuffer.empty())
			{
				delete m_PacketBuffer.front();
				m_PacketBuffer.pop();
			}
		}
	}

	if (m_LocalSocket)
	{
		//
		// handle proxying (reconnecting, etc...)
		//

		if (m_LocalSocket->HasError() || !m_LocalSocket->GetConnected())
		{
			CONSOLE_Print("[Client] local player disconnected");

			if (m_BNET->GetInGame())
				m_BNET->QueueEnterChat();

			delete m_LocalSocket;
			m_LocalSocket = NULL;

			// ensure a leavegame message was sent, otherwise the server may wait for our reconnection which will never happen
			// if one hasn't been sent it's because Warcraft III exited abnormally

			if (m_GameIsReliable && !m_LeaveGameSent)
			{
				// note: we're not actually 100% ensuring the leavegame message is sent, we'd need to check that DoSend worked, etc...

				BYTEARRAY LeaveGame;
				LeaveGame.push_back(0xF7);
				LeaveGame.push_back(0x21);
				LeaveGame.push_back(0x08);
				LeaveGame.push_back(0x00);
				UTIL_AppendByteArray(LeaveGame, (uint32_t)PLAYERLEAVE_GPROXY, false);
				m_RemoteSocket->PutBytes(LeaveGame);
				m_RemoteSocket->DoSend(&send_fd);
			}

			m_RemoteSocket->Reset();
			m_RemoteSocket->SetNoDelay(true);
			m_RemoteServerIP.clear();
			m_RemoteServerPort = 0;

			ProcessCommandsCallback("/closewar3", "1");
		}
		else
		{
			m_LocalSocket->DoRecv(&fd);
			ExtractLocalPackets();
			ProcessLocalPackets();

			if (!m_RemoteServerIP.empty())
			{
				if (m_GameIsReliable && m_ActionReceived && GetTime() - m_LastActionTime >= 60)
				{
					if (m_NumEmptyActionsUsed < m_NumEmptyActions)
					{
						SendEmptyAction();
						m_NumEmptyActionsUsed++;
					}
					else
					{
						SendLocalChat("War3Lan ran out of time to reconnect, Warcraft III will disconnect soon.");
						CONSOLE_Print("[Client] ran out of time to reconnect");
					}

					m_LastActionTime = GetTime();
				}

				if (m_RemoteSocket->HasError())
				{
					CONSOLE_Print("[Client] disconnected from remote server due to socket error");

					if (m_GameIsReliable && m_ActionReceived && m_ReconnectPort > 0)
					{
						SendLocalChat("You have been disconnected from the server due to a socket error.");
						uint32_t TimeRemaining = (m_NumEmptyActions - m_NumEmptyActionsUsed + 1) * 60 - (GetTime() - m_LastActionTime);

						if (GetTime() - m_LastActionTime > (m_NumEmptyActions - m_NumEmptyActionsUsed + 1) * 60)
							TimeRemaining = 0;

						SendLocalChat("War3Lan is attempting to reconnect... (" + UTIL_ToString(TimeRemaining) + " seconds remain)");
						CONSOLE_Print("[Client] attempting to reconnect");

						m_RemoteSocket->Reset();
						m_RemoteSocket->SetNoDelay(true);
						m_RemoteSocket->Connect(string(), m_RemoteServerIP, m_ReconnectPort);
						m_LastConnectionAttemptTime = GetTime();
					}
					else
					{
						if (m_BNET->GetInGame())
							m_BNET->QueueEnterChat();

						m_LocalSocket->Disconnect();
						delete m_LocalSocket;
						m_LocalSocket = NULL;
						m_RemoteSocket->Reset();
						m_RemoteSocket->SetNoDelay(true);
						m_RemoteServerIP.clear();
						m_RemoteServerPort = 0;
						return false;
					}
				}

				if (!m_RemoteSocket->GetConnecting() && !m_RemoteSocket->GetConnected())
				{
					CONSOLE_Print("[Client] disconnected from remote server");

					if (m_GameIsReliable && m_ActionReceived && m_ReconnectPort > 0)
					{
						SendLocalChat("You have been disconnected from the server.");
						uint32_t TimeRemaining = (m_NumEmptyActions - m_NumEmptyActionsUsed + 1) * 60 - (GetTime() - m_LastActionTime);

						if (GetTime() - m_LastActionTime > (m_NumEmptyActions - m_NumEmptyActionsUsed + 1) * 60)
							TimeRemaining = 0;

						SendLocalChat("War3Lan is attempting to reconnect... (" + UTIL_ToString(TimeRemaining) + " seconds remain)");
						CONSOLE_Print("[Client] attempting to reconnect");
						m_RemoteSocket->Reset();
						m_RemoteSocket->SetNoDelay(true);
						m_RemoteSocket->Connect(string(), m_RemoteServerIP, m_ReconnectPort);
						m_LastConnectionAttemptTime = GetTime();
					}
					else
					{
						if (m_BNET->GetInGame())
							m_BNET->QueueEnterChat();

						m_LocalSocket->Disconnect();
						delete m_LocalSocket;
						m_LocalSocket = NULL;
						m_RemoteSocket->Reset();
						m_RemoteSocket->SetNoDelay(true);
						m_RemoteServerIP.clear();
						m_RemoteServerPort = 0;
						return false;
					}
				}

				if (m_RemoteSocket->GetConnected())
				{
					if (m_GameIsReliable && m_ActionReceived && m_ReconnectPort > 0 && GetTime() - m_RemoteSocket->GetLastRecv() >= 20)
					{
						CONSOLE_Print("[Client] disconnected from remote server due to 20 second timeout");
						SendLocalChat("You have been timed out from the server.");
						uint32_t TimeRemaining = (m_NumEmptyActions - m_NumEmptyActionsUsed + 1) * 60 - (GetTime() - m_LastActionTime);

						if (GetTime() - m_LastActionTime > (m_NumEmptyActions - m_NumEmptyActionsUsed + 1) * 60)
							TimeRemaining = 0;

						SendLocalChat("War3Lan is attempting to reconnect... (" + UTIL_ToString(TimeRemaining) + " seconds remain)");
						CONSOLE_Print("[Client] attempting to reconnect");
						m_RemoteSocket->Reset();
						m_RemoteSocket->SetNoDelay(true);
						m_RemoteSocket->Connect(string(), m_RemoteServerIP, m_ReconnectPort);
						m_LastConnectionAttemptTime = GetTime();
					}
					else
					{
						m_RemoteSocket->DoRecv(&fd);
						ExtractRemotePackets();
						ProcessRemotePackets();

						if (m_GameIsReliable && m_ActionReceived && m_ReconnectPort > 0 && GetTime() - m_LastAckTime >= 10)
						{
							m_RemoteSocket->PutBytes(m_GPSProtocol->SEND_GPSC_ACK(m_TotalPacketsReceivedFromRemote));
							m_LastAckTime = GetTime();
						}

						m_RemoteSocket->DoSend(&send_fd);
					}
				}

				if (m_RemoteSocket->GetConnecting())
				{
					// we are currently attempting to connect

					if (m_RemoteSocket->CheckConnect())
					{
						// the connection attempt completed

						if (m_GameIsReliable && m_ActionReceived)
						{
							// this is a reconnection, not a new connection
							// if the server accepts the reconnect request it will send a GPS_RECONNECT back requesting a certain number of packets

							SendLocalChat("War3Lan reconnected to the server!");
							SendLocalChat("==================================================");
							CONSOLE_Print("[Client] reconnected to remote server");

							// note: even though we reset the socket when we were disconnected, we haven't been careful to ensure we never queued any data in the meantime
							// therefore it's possible the socket could have data in the send buffer
							// this is bad because the server will expect us to send a GPS_RECONNECT message first
							// so we must clear the send buffer before we continue
							// note: we aren't losing data here, any important messages that need to be sent have been put in the packet buffer
							// they will be requested by the server if required

							m_RemoteSocket->ClearSendBuffer();
							m_RemoteSocket->PutBytes(m_GPSProtocol->SEND_GPSC_RECONNECT(m_PID, m_ReconnectKey, m_TotalPacketsReceivedFromRemote));

							// we cannot permit any forwarding of local packets until the game is synchronized again
							// this will disable forwarding and will be reset when the synchronization is complete

							m_Synchronized = false;
						}
						else
							CONSOLE_Print("[Client] connected to remote server");
					}
					else if (GetTime() - m_LastConnectionAttemptTime >= 10)
					{
						// the connection attempt timed out (10 seconds)

						CONSOLE_Print("[Client] connect to remote server timed out");

						if (m_GameIsReliable && m_ActionReceived && m_ReconnectPort > 0)
						{
							uint32_t TimeRemaining = (m_NumEmptyActions - m_NumEmptyActionsUsed + 1) * 60 - (GetTime() - m_LastActionTime);

							if (GetTime() - m_LastActionTime > (m_NumEmptyActions - m_NumEmptyActionsUsed + 1) * 60)
								TimeRemaining = 0;

							SendLocalChat("War3Lan is attempting to reconnect... (" + UTIL_ToString(TimeRemaining) + " seconds remain)");
							CONSOLE_Print("[Client] attempting to reconnect");
							m_RemoteSocket->Reset();
							m_RemoteSocket->SetNoDelay(true);
							m_RemoteSocket->Connect(string(), m_RemoteServerIP, m_ReconnectPort);
							m_LastConnectionAttemptTime = GetTime();
						}
						else
						{
							if (m_BNET->GetInGame())
								m_BNET->QueueEnterChat();

							m_LocalSocket->Disconnect();
							delete m_LocalSocket;
							m_LocalSocket = NULL;
							m_RemoteSocket->Reset();
							m_RemoteSocket->SetNoDelay(true);
							m_RemoteServerIP.clear();
							m_RemoteServerPort = 0;
							return false;
						}
					}
				}
			}

			m_LocalSocket->DoSend(&send_fd);
		}
	}
	else
	{
		//
		// handle game listing
		//

		if (GetTickCount() - m_LastRefreshTime > 250)
		{
			bool hostedgame = false;
			for (int n = 0; n < m_Games.size(); n++)
			{
				auto i = &m_Games[n];

				CONSOLE_Print("Broadcast game:" + (*i)->GetGameName());

				BYTEARRAY MapGameType;
				UTIL_AppendByteArray(MapGameType, (*i)->GetGameType(), false);
				UTIL_AppendByteArray(MapGameType, (*i)->GetParameter(), false);
				BYTEARRAY MapFlags = UTIL_CreateByteArray((*i)->GetMapFlags(), false);
				BYTEARRAY MapWidth = UTIL_CreateByteArray((*i)->GetMapWidth(), false);
				BYTEARRAY MapHeight = UTIL_CreateByteArray((*i)->GetMapHeight(), false);
				string GameName = (*i)->GetGameName();

				// colour reliable game names so they're easier to pick out of the list

				if (GameName.size() > 31)
					GameName = GameName.substr(0, 31);

				BYTEARRAY retval = BYTEARRAY();
				/*if (m_PublicGameFilter.length() > 0)
					retval = m_GameProtocol->SEND_W3GS_GAMEINFO(m_TFT, m_War3Version, MapGameType, MapFlags, MapWidth, MapHeight, m_PublicGameFilter, (*i)->GetHostName(), (*i)->GetElapsedTime(), (*i)->GetMapPath(), (*i)->GetMapCRC(), 12, 12, m_Port, (*i)->GetUniqueGameID(), (*i)->GetUniqueGameID());
				else*/
					retval = m_GameProtocol->SEND_W3GS_GAMEINFO(m_TFT, m_War3Version, MapGameType, MapFlags, MapWidth, MapHeight, (*i)->GetGameName(), (*i)->GetHostName(), (*i)->GetElapsedTime(), (*i)->GetMapPath(), (*i)->GetMapCRC(), 12, 12, m_Port, (*i)->GetUniqueGameID(), (*i)->GetUniqueGameID());

				if (retval.size() > 0)
				{
					hostedgame = true;
					m_UDPSocket->Broadcast(6113, retval);
					break;
				}
				else
				{

				}
			}
			m_LastRefreshTime = GetTickCount();
		}
	}

	return m_Exiting;
}

void CGProxy::ExtractLocalPackets()
{
	if (!m_LocalSocket)
		return;

	string* RecvBuffer = m_LocalSocket->GetBytes();
	BYTEARRAY Bytes = UTIL_CreateByteArray((unsigned char*)RecvBuffer->c_str(), RecvBuffer->size());

	// a packet is at least 4 bytes so loop as long as the buffer contains 4 bytes

	while (Bytes.size() >= 4)
	{
		// byte 0 is always 247

		if (Bytes[0] == W3GS_HEADER_CONSTANT)
		{
			// bytes 2 and 3 contain the length of the packet

			uint16_t Length = UTIL_ByteArrayToUInt16(Bytes, false, 2);

			if (Length >= 4)
			{
				if (Bytes.size() >= Length)
				{
					// we have to do a little bit of packet processing here
					// this is because we don't want to forward any chat messages that start with a "/" as these may be forwarded to battle.net instead
					// in fact we want to pretend they were never even received from the proxy's perspective

					bool Forward = true;
					BYTEARRAY Data = BYTEARRAY(Bytes.begin(), Bytes.begin() + Length);

					if (Bytes[1] == CGameProtocol::W3GS_CHAT_TO_HOST)
					{
						if (Data.size() >= 5)
						{
							unsigned int i = 5;
							unsigned char Total = Data[4];

							if (Total > 0 && Data.size() >= i + Total)
							{
								i += Total;
								unsigned char Flag = Data[i + 1];
								i += 2;

								string MessageString;

								if (Flag == 16 && Data.size() >= i + 1)
								{
									BYTEARRAY Message = UTIL_ExtractCString(Data, i);
									MessageString = string(Message.begin(), Message.end());
								}
								else if (Flag == 32 && Data.size() >= i + 5)
								{
									BYTEARRAY Message = UTIL_ExtractCString(Data, i + 4);
									MessageString = string(Message.begin(), Message.end());
								}

								string Command = MessageString;
								transform(Command.begin(), Command.end(), Command.begin(), (int(*)(int))tolower);

								if (Command.size() >= 1 && Command.substr(0, 1) == "/")
								{
									Forward = false;

									if (Command.size() >= 5 && Command.substr(0, 4) == "/re ")
									{
										if (m_BNET->GetLoggedIn())
										{
											if (!m_BNET->GetReplyTarget().empty())
											{
												m_BNET->QueueChatCommand(MessageString.substr(4), m_BNET->GetReplyTarget(), true);
												SendLocalChat("Whispered to " + m_BNET->GetReplyTarget() + ": " + MessageString.substr(4));
											}
											else
												SendLocalChat("Nobody has whispered you yet.");
										}
										else
											SendLocalChat("You are not connected to battle.net.");
									}
									else if (Command == "/sc" || Command == "/spoof" || Command == "/spoofcheck" || Command == "/spoof check")
									{
										if (m_BNET->GetLoggedIn())
										{
											if (!m_GameStarted)
											{
												m_BNET->QueueChatCommand("spoofcheck", m_HostName, true);
												SendLocalChat("Whispered to " + m_HostName + ": spoofcheck");
											}
											else
												SendLocalChat("The game has already started.");
										}
										else
											SendLocalChat("You are not connected to battle.net.");
									}
									else if (Command == "/status")
									{
										if (m_LocalSocket)
										{
											if (m_GameIsReliable && m_ReconnectPort > 0)
												SendLocalChat("War3Lan disconnect protection: Enabled");
											else
												SendLocalChat("War3Lan disconnect protection: Disabled");

											if (m_BNET->GetLoggedIn())
												SendLocalChat("battle.net: Connected");
											else
												SendLocalChat("battle.net: Disconnected");
										}
									}
									else if (Command.size() >= 4 && Command.substr(0, 3) == "/w ")
									{
										if (m_BNET->GetLoggedIn())
										{
											// todotodo: fix me

											m_BNET->QueueChatCommand(MessageString);
											// SendLocalChat( "Whispered to ???: ???" );
										}
										else
											SendLocalChat("You are not connected to battle.net.");
									}
								}
							}
						}
					}

					if (Forward)
					{
						m_LocalPackets.push(new CCommandPacket(W3GS_HEADER_CONSTANT, Bytes[1], Data));
						m_PacketBuffer.push(new CCommandPacket(W3GS_HEADER_CONSTANT, Bytes[1], Data));
						m_TotalPacketsReceivedFromLocal++;
					}

					*RecvBuffer = RecvBuffer->substr(Length);
					Bytes = BYTEARRAY(Bytes.begin() + Length, Bytes.end());
				}
				else
					return;
			}
			else
			{
				CONSOLE_Print("[Client] received invalid packet from local player (bad length)");
				m_Exiting = true;
				return;
			}
		}
		else
		{
			CONSOLE_Print("[Client] received invalid packet from local player (bad header constant)");
			m_Exiting = true;
			return;
		}
	}
}

void CGProxy::ProcessLocalPackets()
{
	if (!m_LocalSocket)
		return;

	while (!m_LocalPackets.empty())
	{
		CCommandPacket* Packet = m_LocalPackets.front();
		m_LocalPackets.pop();
		BYTEARRAY Data = Packet->GetData();

		if (Packet->GetPacketType() == W3GS_HEADER_CONSTANT)
		{
			if (Packet->GetID() == CGameProtocol::W3GS_REQJOIN)
			{
				if (Data.size() >= 20)
				{
					// parse
					uint32_t HostCounter = UTIL_ByteArrayToUInt32(Data, false, 4);
					uint32_t EntryKey = UTIL_ByteArrayToUInt32(Data, false, 8);
					unsigned char Unknown = Data[12];
					uint16_t ListenPort = UTIL_ByteArrayToUInt16(Data, false, 13);
					uint32_t PeerKey = UTIL_ByteArrayToUInt32(Data, false, 15);
					BYTEARRAY Name = UTIL_ExtractCString(Data, 19);
					string NameString = string(Name.begin(), Name.end());
					BYTEARRAY Remainder = BYTEARRAY(Data.begin() + Name.size() + 20, Data.end());

					if (Remainder.size() == 18)
					{
						// lookup the game in the main list

						bool GameFound = false;

						for (vector<CIncomingGameHost*> ::iterator i = m_Games.begin(); i != m_Games.end(); i++)
						{
							if ((*i)->GetUniqueGameID() == EntryKey)
							{
								CONSOLE_Print("[Client] local player requested game name [" + (*i)->GetGameName() + "]");

								if (NameString != m_Username)
									CONSOLE_Print("[Client] using battle.net name [" + m_Username + "] instead of requested name [" + NameString + "]");

								CONSOLE_Print("[Client] connecting to remote server [" + (*i)->GetIPString() + "] on port " + UTIL_ToString((*i)->GetPort()));
								m_RemoteServerIP = (*i)->GetIPString();
								m_RemoteServerPort = (*i)->GetPort();
								m_RemoteSocket->Reset();
								m_RemoteSocket->SetNoDelay(true);
								m_RemoteSocket->Connect(string(), m_RemoteServerIP, m_RemoteServerPort);
								m_LastConnectionAttemptTime = GetTime();
								m_GameIsReliable = ((*i)->GetMapWidth() == 1984 && (*i)->GetMapHeight() == 1984);
								m_GameStarted = false;

								// rewrite packet

								BYTEARRAY DataRewritten;
								DataRewritten.push_back(W3GS_HEADER_CONSTANT);
								DataRewritten.push_back(Packet->GetID());
								DataRewritten.push_back(0);
								DataRewritten.push_back(0);
								UTIL_AppendByteArray(DataRewritten, (*i)->GetHostCounter(), false);
								UTIL_AppendByteArray(DataRewritten, (uint32_t)0, false);
								DataRewritten.push_back(Unknown);
								UTIL_AppendByteArray(DataRewritten, ListenPort, false);
								UTIL_AppendByteArray(DataRewritten, PeerKey, false);
								UTIL_AppendByteArray(DataRewritten, m_Username);
								UTIL_AppendByteArrayFast(DataRewritten, Remainder);
								BYTEARRAY LengthBytes;
								LengthBytes = UTIL_CreateByteArray((uint16_t)DataRewritten.size(), false);
								DataRewritten[2] = LengthBytes[0];
								DataRewritten[3] = LengthBytes[1];
								Data = DataRewritten;

								// tell battle.net we're joining a game (for automatic spoof checking)

								m_BNET->QueueJoinGame((*i)->GetGameName());

								// save the hostname for later (for manual spoof checking)

								m_JoinedName = NameString;
								m_HostName = (*i)->GetHostName();
								GameFound = true;
								break;
							}
						}

						if (!GameFound)
						{
							CONSOLE_Print("[Client] local player requested unknown game (expired?)");
							m_LocalSocket->Disconnect();
						}
					}
					else
						CONSOLE_Print("[Client] received invalid join request from local player (invalid remainder)");
				}
				else
					CONSOLE_Print("[Client] received invalid join request from local player (too short)");
			}
			else if (Packet->GetID() == CGameProtocol::W3GS_LEAVEGAME)
			{
				m_LeaveGameSent = true;
				m_LocalSocket->Disconnect();
			}
			else if (Packet->GetID() == CGameProtocol::W3GS_CHAT_TO_HOST)
			{
				// handled in ExtractLocalPackets (yes, it's ugly)
			}
		}

		// warning: do not forward any data if we are not synchronized (e.g. we are reconnecting and resynchronizing)
		// any data not forwarded here will be cached in the packet buffer and sent later so all is well

		if (m_RemoteSocket && m_Synchronized)
			m_RemoteSocket->PutBytes(Data);

		delete Packet;
	}
}

void CGProxy::ExtractRemotePackets()
{
	string* RecvBuffer = m_RemoteSocket->GetBytes();
	BYTEARRAY Bytes = UTIL_CreateByteArray((unsigned char*)RecvBuffer->c_str(), RecvBuffer->size());

	// a packet is at least 4 bytes so loop as long as the buffer contains 4 bytes

	while (Bytes.size() >= 4)
	{
		if (Bytes[0] == W3GS_HEADER_CONSTANT || Bytes[0] == GPS_HEADER_CONSTANT)
		{
			// bytes 2 and 3 contain the length of the packet

			uint16_t Length = UTIL_ByteArrayToUInt16(Bytes, false, 2);

			if (Length >= 4)
			{
				if (Bytes.size() >= Length)
				{
					m_RemotePackets.push(new CCommandPacket(Bytes[0], Bytes[1], BYTEARRAY(Bytes.begin(), Bytes.begin() + Length)));

					if (Bytes[0] == W3GS_HEADER_CONSTANT)
						m_TotalPacketsReceivedFromRemote++;

					*RecvBuffer = RecvBuffer->substr(Length);
					Bytes = BYTEARRAY(Bytes.begin() + Length, Bytes.end());
				}
				else
					return;
			}
			else
			{
				CONSOLE_Print("[Client] received invalid packet from remote server (bad length)");
				m_Exiting = true;
				return;
			}
		}
		else
		{
			CONSOLE_Print("[Client] received invalid packet from remote server (bad header constant)");
			m_Exiting = true;
			return;
		}
	}
}

void CGProxy::ProcessRemotePackets()
{
	if (!m_LocalSocket || !m_RemoteSocket)
		return;

	while (!m_RemotePackets.empty())
	{
		CCommandPacket* Packet = m_RemotePackets.front();
		m_RemotePackets.pop();

		if (Packet->GetPacketType() == W3GS_HEADER_CONSTANT)
		{
			if (Packet->GetID() == CGameProtocol::W3GS_SLOTINFOJOIN)
			{
				BYTEARRAY Data = Packet->GetData();

				if (Data.size() >= 6)
				{
					uint16_t SlotInfoSize = UTIL_ByteArrayToUInt16(Data, false, 4);

					if (Data.size() >= 7 + SlotInfoSize)
						m_ChatPID = Data[6 + SlotInfoSize];
				}

				// send a GPS_INIT packet
				// if the server doesn't recognize it (e.g. it isn't GHost++) we should be kicked

				CONSOLE_Print("[Client] join request accepted by remote server");

				if (m_GameIsReliable)
				{
					CONSOLE_Print("[Client] detected reliable game, starting GPS handshake");
					m_RemoteSocket->PutBytes(m_GPSProtocol->SEND_GPSC_INIT(1));
				}
				else
					CONSOLE_Print("[Client] detected standard game, disconnect protection disabled");
			}
			else if (Packet->GetID() == CGameProtocol::W3GS_COUNTDOWN_END)
			{
				if (m_GameIsReliable && m_ReconnectPort > 0)
					CONSOLE_Print("[Client] game started, disconnect protection enabled");
				else
				{
					if (m_GameIsReliable)
						CONSOLE_Print("[Client] game started but GPS handshake not complete, disconnect protection disabled");
					else
						CONSOLE_Print("[Client] game started");
				}

				m_GameStarted = true;
			}
			else if (Packet->GetID() == CGameProtocol::W3GS_INCOMING_ACTION)
			{
				if (m_GameIsReliable)
				{
					// we received a game update which means we can reset the number of empty actions we have to work with
					// we also must send any remaining empty actions now
					// note: the lag screen can't be up right now otherwise the server made a big mistake, so we don't need to check for it

					BYTEARRAY EmptyAction;
					EmptyAction.push_back(0xF7);
					EmptyAction.push_back(0x0C);
					EmptyAction.push_back(0x06);
					EmptyAction.push_back(0x00);
					EmptyAction.push_back(0x00);
					EmptyAction.push_back(0x00);

					for (unsigned char i = m_NumEmptyActionsUsed; i < m_NumEmptyActions; i++)
						m_LocalSocket->PutBytes(EmptyAction);

					m_NumEmptyActionsUsed = 0;
				}

				m_ActionReceived = true;
				m_LastActionTime = GetTime();
			}
			else if (Packet->GetID() == CGameProtocol::W3GS_START_LAG)
			{
				if (m_GameIsReliable)
				{
					BYTEARRAY Data = Packet->GetData();

					if (Data.size() >= 5)
					{
						unsigned char NumLaggers = Data[4];

						if (Data.size() == 5 + NumLaggers * 5)
						{
							for (unsigned char i = 0; i < NumLaggers; i++)
							{
								bool LaggerFound = false;

								for (vector<unsigned char> ::iterator j = m_Laggers.begin(); j != m_Laggers.end(); j++)
								{
									if (*j == Data[5 + i * 5])
										LaggerFound = true;
								}

								if (LaggerFound)
									CONSOLE_Print("[Client] warning - received start_lag on known lagger");
								else
									m_Laggers.push_back(Data[5 + i * 5]);
							}
						}
						else
							CONSOLE_Print("[Client] warning - unhandled start_lag (2)");
					}
					else
						CONSOLE_Print("[Client] warning - unhandled start_lag (1)");
				}
			}
			else if (Packet->GetID() == CGameProtocol::W3GS_STOP_LAG)
			{
				if (m_GameIsReliable)
				{
					BYTEARRAY Data = Packet->GetData();

					if (Data.size() == 9)
					{
						bool LaggerFound = false;

						for (vector<unsigned char> ::iterator i = m_Laggers.begin(); i != m_Laggers.end(); )
						{
							if (*i == Data[4])
							{
								i = m_Laggers.erase(i);
								LaggerFound = true;
							}
							else
								i++;
						}

						if (!LaggerFound)
							CONSOLE_Print("[Client] warning - received stop_lag on unknown lagger");
					}
					else
						CONSOLE_Print("[Client] warning - unhandled stop_lag");
				}
			}
			else if (Packet->GetID() == CGameProtocol::W3GS_INCOMING_ACTION2)
			{
				if (m_GameIsReliable)
				{
					// we received a fractured game update which means we cannot use any empty actions until we receive the subsequent game update
					// we also must send any remaining empty actions now
					// note: this means if we get disconnected right now we can't use any of our buffer time, which would be very unlucky
					// it still gives us 60 seconds total to reconnect though
					// note: the lag screen can't be up right now otherwise the server made a big mistake, so we don't need to check for it

					BYTEARRAY EmptyAction;
					EmptyAction.push_back(0xF7);
					EmptyAction.push_back(0x0C);
					EmptyAction.push_back(0x06);
					EmptyAction.push_back(0x00);
					EmptyAction.push_back(0x00);
					EmptyAction.push_back(0x00);

					for (unsigned char i = m_NumEmptyActionsUsed; i < m_NumEmptyActions; i++)
						m_LocalSocket->PutBytes(EmptyAction);

					m_NumEmptyActionsUsed = m_NumEmptyActions;
				}
			}

			// forward the data

			m_LocalSocket->PutBytes(Packet->GetData());

			// we have to wait until now to send the status message since otherwise the slotinfojoin itself wouldn't have been forwarded

			if (Packet->GetID() == CGameProtocol::W3GS_SLOTINFOJOIN)
			{
				if (m_JoinedName != m_Username)
					SendLocalChat("Using battle.net name \"" + m_Username + "\" instead of LAN name \"" + m_JoinedName + "\".");

				if (m_GameIsReliable)
					SendLocalChat("This is a reliable game. Requesting War3Lan disconnect protection from server...");
				else
					SendLocalChat("This is an unreliable game. War3Lan disconnect protection is disabled.");
			}
		}
		else if (Packet->GetPacketType() == GPS_HEADER_CONSTANT)
		{
			if (m_GameIsReliable)
			{
				BYTEARRAY Data = Packet->GetData();

				if (Packet->GetID() == CGPSProtocol::GPS_INIT && Data.size() == 12)
				{
					m_ReconnectPort = UTIL_ByteArrayToUInt16(Data, false, 4);
					m_PID = Data[6];
					m_ReconnectKey = UTIL_ByteArrayToUInt32(Data, false, 7);
					m_NumEmptyActions = Data[11];
					SendLocalChat("War3Lan disconnect protection is ready (" + UTIL_ToString((m_NumEmptyActions + 1) * 60) + " second buffer).");
					CONSOLE_Print("[Client] handshake complete, disconnect protection ready (" + UTIL_ToString((m_NumEmptyActions + 1) * 60) + " second buffer)");
				}
				else if (Packet->GetID() == CGPSProtocol::GPS_RECONNECT && Data.size() == 8)
				{
					uint32_t LastPacket = UTIL_ByteArrayToUInt32(Data, false, 4);
					uint32_t PacketsAlreadyUnqueued = m_TotalPacketsReceivedFromLocal - m_PacketBuffer.size();

					if (LastPacket > PacketsAlreadyUnqueued)
					{
						uint32_t PacketsToUnqueue = LastPacket - PacketsAlreadyUnqueued;

						if (PacketsToUnqueue > m_PacketBuffer.size())
						{
							CONSOLE_Print("[Client] received GPS_RECONNECT with last packet > total packets sent");
							PacketsToUnqueue = m_PacketBuffer.size();
						}

						while (PacketsToUnqueue > 0)
						{
							delete m_PacketBuffer.front();
							m_PacketBuffer.pop();
							PacketsToUnqueue--;
						}
					}

					// send remaining packets from buffer, preserve buffer
					// note: any packets in m_LocalPackets are still sitting at the end of this buffer because they haven't been processed yet
					// therefore we must check for duplicates otherwise we might (will) cause a desync

					queue<CCommandPacket*> TempBuffer;

					while (!m_PacketBuffer.empty())
					{
						if (m_PacketBuffer.size() > m_LocalPackets.size())
							m_RemoteSocket->PutBytes(m_PacketBuffer.front()->GetData());

						TempBuffer.push(m_PacketBuffer.front());
						m_PacketBuffer.pop();
					}

					m_PacketBuffer = TempBuffer;

					// we can resume forwarding local packets again
					// doing so prior to this point could result in an out-of-order stream which would probably cause a desync

					m_Synchronized = true;
				}
				else if (Packet->GetID() == CGPSProtocol::GPS_ACK && Data.size() == 8)
				{
					uint32_t LastPacket = UTIL_ByteArrayToUInt32(Data, false, 4);
					uint32_t PacketsAlreadyUnqueued = m_TotalPacketsReceivedFromLocal - m_PacketBuffer.size();

					if (LastPacket > PacketsAlreadyUnqueued)
					{
						uint32_t PacketsToUnqueue = LastPacket - PacketsAlreadyUnqueued;

						if (PacketsToUnqueue > m_PacketBuffer.size())
						{
							CONSOLE_Print("[Client] received GPS_ACK with last packet > total packets sent");
							PacketsToUnqueue = m_PacketBuffer.size();
						}

						while (PacketsToUnqueue > 0)
						{
							delete m_PacketBuffer.front();
							m_PacketBuffer.pop();
							PacketsToUnqueue--;
						}
					}
				}
				else if (Packet->GetID() == CGPSProtocol::GPS_REJECT && Data.size() == 8)
				{
					uint32_t Reason = UTIL_ByteArrayToUInt32(Data, false, 4);

					if (Reason == REJECTGPS_INVALID)
						CONSOLE_Print("[Client] rejected by remote server: invalid data");
					else if (Reason == REJECTGPS_NOTFOUND)
						CONSOLE_Print("[Client] rejected by remote server: player not found in any running games");

					m_LocalSocket->Disconnect();
				}
			}
		}

		delete Packet;
	}
}


bool CGProxy::AddGame(CIncomingGameHost* game)
{
	// check for duplicates and rehosted games
	if (m_Games.size() )
	{
		bool skiponegame = false;
		for (vector<CIncomingGameHost*> ::iterator i = m_Games.begin(); i != m_Games.end(); i++)
		{
			m_UDPSocket->Broadcast(6113, m_GameProtocol->SEND_W3GS_DECREATEGAME(((CIncomingGameHost*)(*i))->GetUniqueGameID()));
			delete* i;
		}
		m_Games.clear();
	}
	//for (vector<CIncomingGameHost*> ::iterator i = m_Games.begin(); i != m_Games.end(); i++)
	//{
	//	if (game->GetIP() == ((CIncomingGameHost*)(*i))->GetIP() && game->GetPort() == ((CIncomingGameHost*)(*i))->GetPort())
	//	{
	//		// duplicate or rehosted game, delete the old one and add the new one
	//		// don't forget to remove the old one from the LAN list first

	//		m_UDPSocket->Broadcast(6112, m_GameProtocol->SEND_W3GS_DECREATEGAME(((CIncomingGameHost*)(*i))->GetUniqueGameID()));
	//		delete* i;
	//		*i = game;
	//		DuplicateFound = true;
	//		break;
	//	}

	//}

	
		m_Games.push_back(game);

	// the game list cannot hold more than 20 games (warcraft 3 doesn't handle it properly and ignores any further games)
	// if this game puts us over the limit, remove the oldest game
	// don't remove the "search game" since that's probably a pretty important game
	// note: it'll get removed automatically by the 60 second timeout in the main loop when appropriate


	return true;
}


void CGProxy::SendLocalChat(string message)
{
	if (m_LocalSocket)
	{
		if (m_GameStarted)
		{
			if (message.size() > 127)
				message = message.substr(0, 127);

			m_LocalSocket->PutBytes(m_GameProtocol->SEND_W3GS_CHAT_FROM_HOST(m_ChatPID, UTIL_CreateByteArray(m_ChatPID), 32, UTIL_CreateByteArray((uint32_t)0, false), message));
		}
		else
		{
			if (message.size() > 254)
				message = message.substr(0, 254);

			m_LocalSocket->PutBytes(m_GameProtocol->SEND_W3GS_CHAT_FROM_HOST(m_ChatPID, UTIL_CreateByteArray(m_ChatPID), 16, BYTEARRAY(), message));
		}
	}
}

void CGProxy::SendEmptyAction()
{
	// we can't send any empty actions while the lag screen is up
	// so we keep track of who the lag screen is currently showing (if anyone) and we tear it down, send the empty action, and put it back up

	for (vector<unsigned char> ::iterator i = m_Laggers.begin(); i != m_Laggers.end(); i++)
	{
		BYTEARRAY StopLag;
		StopLag.push_back(0xF7);
		StopLag.push_back(0x11);
		StopLag.push_back(0x09);
		StopLag.push_back(0);
		StopLag.push_back(*i);
		UTIL_AppendByteArray(StopLag, (uint32_t)60000, false);
		m_LocalSocket->PutBytes(StopLag);
	}

	BYTEARRAY EmptyAction;
	EmptyAction.push_back(0xF7);
	EmptyAction.push_back(0x0C);
	EmptyAction.push_back(0x06);
	EmptyAction.push_back(0x00);
	EmptyAction.push_back(0x00);
	EmptyAction.push_back(0x00);
	m_LocalSocket->PutBytes(EmptyAction);

	if (!m_Laggers.empty())
	{
		BYTEARRAY StartLag;
		StartLag.push_back(0xF7);
		StartLag.push_back(0x10);
		StartLag.push_back(0);
		StartLag.push_back(0);
		StartLag.push_back((unsigned char)m_Laggers.size());

		for (vector<unsigned char> ::iterator i = m_Laggers.begin(); i != m_Laggers.end(); i++)
		{
			// using a lag time of 60000 ms means the counter will start at zero
			// hopefully warcraft 3 doesn't care about wild variations in the lag time in subsequent packets

			StartLag.push_back(*i);
			UTIL_AppendByteArray(StartLag, (uint32_t)60000, false);
		}

		BYTEARRAY LengthBytes;
		LengthBytes = UTIL_CreateByteArray((uint16_t)StartLag.size(), false);
		StartLag[2] = LengthBytes[0];
		StartLag[3] = LengthBytes[1];
		m_LocalSocket->PutBytes(StartLag);
	}
}
