#include "Общий заголовок.h"
#include "AntiHack.h"

typedef int(__fastcall* InitFrameDef)(void* a1, int unused, int a2, int a3);
InitFrameDef InitWc3MainMenu_org;
InitFrameDef InitWc3MainMenu_ptr;
InitFrameDef InitLocalGameMenu;

InitFrameDef InitWc3BattleNetMenu_org;
InitFrameDef InitWc3BattleNetMenu_ptr;

InitFrameDef BattleNetChatPanel_org;
InitFrameDef BattleNetChatPanel_ptr;

InitFrameDef BattleNetCustomJoinPanel_org;
InitFrameDef BattleNetCustomJoinPanel_ptr;


BOOL FirstStart = TRUE;

BOOL SkipAllKeys = FALSE;

HANDLE AutoJoinThreadHandle = NULL;
HANDLE DestroyAutoJoinThreadHandle = NULL;


int CurrentOperationID = -1;

DWORD WINAPI DestroyAutoJoinThread(LPVOID)
{
	Sleep(100);
	GiveMeTlsAccess();

	int i = 1000;
	while (i > 0)
	{
		*(int*)((DWORD)GameDll + 0xA9E7A4) = 1;
		SetForegroundWindow(Warcraft3Window);
		SetActiveWindow(Warcraft3Window);
		Sleep(10);
		i--;
	}
	TerminateThread(AutoJoinThreadHandle, 0);
	CloseHandle(AutoJoinThreadHandle);
	CloseHandle(DestroyAutoJoinThreadHandle);
	SkipAllKeys = FALSE;
	FILE* f = NULL;
	fopen_s(&f, "Log_in.me", "wb");
	if (f)
	{
		fclose(f);
	}
	if (CurrentOperationID < 5)
		MessageBoxA(0, "Ошибка автоматического входа", 0, 0);
	else 
		MessageBoxA(0, "Обнаружены потеряшки! Игра не может быть начата :(", 0, 0);
	return 0;
}

void LogLog(std::string s)
{
	ofstream f("AntihackDebugger.txt", fstream::out | fstream::app);
	f << s << std::endl;
}

void LogLogT(DWORD time) 
{
	ofstream f("AntihackDebugger.txt", fstream::out | fstream::app);
	f << "Time:" << time << std::endl;
}

DWORD WINAPI ThreadForAutoJoin(LPVOID)
{
	return 0;
	LogLog("Жапуск потока автоматического входа в игру");
	*(int*)((DWORD)GameDll + 0xA9E7A4) = 1;
	while (!Warcraft3Window)
		Sleep(25);
	*(int*)((DWORD)GameDll + 0xA9E7A4) = 1;
	//EnableWindow( Warcraft3Window, FALSE );

	while (FirstStart)
	{
		Sleep(1);
	}

	Sleep(25);
	LogLog("Получен доступ к TLS");
	GiveMeTlsAccess();
	LogLog("Обнаружено главное меню");
	*(int*)((DWORD)GameDll + 0xA9E7A4) = 1;
	DestroyAutoJoinThreadHandle = CreateThread(0, 0, DestroyAutoJoinThread, 0, 0, 0);

	bool onerestart = true;
	//10580300 6FA8B038  10580300 6FA8B038

	*(int*)((DWORD)GameDll + 0xA9E7A4) = 1;
	ShowWindow(Warcraft3Window, SW_SHOW);
	ShowWindow(Warcraft3Window, SW_RESTORE);
	SkipAllKeys = TRUE;



	Sleep(100);

ONERETRY:

	auto LocalAreaNetworkButton = new CWar3Frame();
	LocalAreaNetworkButton->Load("LocalAreaNetworkButton");
	LocalAreaNetworkButton->SetFrameType(CFrameType::FRAMETYPE_BUTTON);
	LogLog("Нажатие на LocalAreaNetworkButton");
	while (!LocalAreaNetworkButton->CheckIsOk())
	{
		Sleep(1);
	}
	Sleep(50);
	int clicksuccess = LocalAreaNetworkButton->Click();
	LogLog(("Нажатие на LocalAreaNetworkButton прошло успешно ? :" + std::to_string(clicksuccess)).c_str());
	if (clicksuccess != 1)
	{
		LogLog("Нефига, давай еще одну попытку...");

		Sleep(100);
		if (LocalAreaNetworkButton->CheckIsOk())
		{
			LocalAreaNetworkButton->SetFocus();
			LocalAreaNetworkButton->Click();
		}
	}


	delete LocalAreaNetworkButton;

	int maxwait = 300;
	// 1F8 23C signed int __thiscall sub_6F564ED0(_DWORD *this, _DWORD *a2) 0x224 = C , id ? 6FA8B038+C
	// [FrameAddr + 0x190] + 0x1C8 // LISTBOX
	// [[FrameAddr + 0x190] + 0x1C8] + 0x1F0 (count)
	// [[[FrameAddr + 0x190] + 0x1C8] + 0x228] (item)
	auto gamelistcontainerframe = new CWar3Frame();
	gamelistcontainerframe->Load("GameListContainer");
	gamelistcontainerframe->SetFrameType(CFrameType::FRAMETYPE_LISTBOX);
	LogLog("Поиск GameListContainer");
	bool firstfound = true;
	bool firstfound2 = true;
	bool firstfound3 = true;

	while (!gamelistcontainerframe->CheckIsOk())
	{
		Sleep(10);
		maxwait--;
		if (maxwait <= 0)
		{
			if (onerestart)
			{
				onerestart = false;
				goto ONERETRY;
			}
		}
	}
	bool onlyskip = true;
	while (gamelistcontainerframe->CheckIsOk())
	{
		if (firstfound)
		{
			firstfound = false;
			LogLog("Поиск GameListContainer успешно завершен.");
			LogLog("Следующие действия: Найти фрейм ListBox, Игру, ListBoxItem, и нажать на название игры.");
		}


		// LOCAL GAME LAYOUT
		int Offset1 = *(int*)(gamelistcontainerframe->FrameAddr + 0x190);
		if (Offset1)
		{
			if (firstfound2)
			{
				firstfound2 = false;
				LogLog("Найдено: LOCAL GAME LAYOUT");
			}

			// LISTBOX WITH GAMES
			int Offset2 = *(int*)(Offset1 + 0x1C8);
			if (Offset2)
			{
				if (firstfound3)
				{
					firstfound3 = false;
					LogLog("Найдено: LISTBOX");
				}
				// ITEM COUNT
				int Offset3 = *(int*)(Offset2 + 0x1F0);
				if (Offset3 > 0)
				{
					LogLog("Найдено: ИГРА");
					// ITEM LIST
					int Offset4 = *(int*)(Offset2 + 0x228);
					if (Offset4 > 0)
					{
						LogLog("Найдено: LISTBOXITEMLIST");
						// FIRST ITEM
						Offset4 = *(int*)(Offset4);
						if (Offset4 > 0)
						{
							LogLog("Найдено: LISTBOXITEM. Проверка на доступность....");
							int statusOffset4 = *(int*)(Offset4 + 4);
							if (statusOffset4 >= 2)
							{
								LogLog("LISTBOXITEM: ДОСТУПЕН ПРОИЗВОДИТСЯ ВЫДЕЛЕНИЕ ИГРЫ");

								if (onlyskip)
								{
									onlyskip = false;
									FILE* f = NULL;
									fopen_s(&f, "give_me_cookies.me", "wb");
									if (f)
									{
										fclose(f);
									}
									LogLog("ОДИН СТРАННЫЙ ФАЙЛ БЫЛ УСПЕШНО СОЗДАН");
									Sleep(300);
									continue;
								}
								/**(int*)(Offset2 + 0x1F4) = Offset4;
								*(int*)(Offset2 + 0x1F8) = 0;*/

								*(int*)((DWORD)GameDll + 0xA8B038 + 0xC) = Offset4;
								*(int*)((DWORD)GameDll + 0xA8B038 + 0x8) = 0;
								*(int*)((DWORD)GameDll + 0xA8B038 + 0x10) = 1;
								*(int*)((DWORD)GameDll + 0xA8B038 + 0x14) = Offset4;
								*(int*)((DWORD)GameDll + 0xA8B038 + 0x18) = (DWORD)GameDll + 0x95AB80;
								Wc3SimulateClickEventListBox(Offset2, (DWORD)GameDll + 0xA8B038);
								LogLog("LISTBOXITEM: ИГРА УСПЕШНО ВЫДЕЛЕНА. СЛЕДУЮЩЕЕ ДЕЙСТВИЕ НАЖАТЬ JOINBUTTON");
								break;
							}
						}
					}
				}
			}
		}
		Sleep(25);
	}
	delete gamelistcontainerframe;

	auto JoinButton = new CWar3Frame();
	JoinButton->Load("JoinButton");
	JoinButton->SetFrameType(CFrameType::FRAMETYPE_BUTTON);
	while (!JoinButton->CheckIsOk())
	{
		Sleep(1);
	}
	Sleep(100);
	while (!JoinButton->CheckIsOk())
	{
		Sleep(1);
	}
	LogLog("Найдено: JOINBUTTON.");
	JoinButton->Click();
	LogLog("JOINBUTTON. УСПЕШНЫЙ КЛИК, ИГРА ЗАПУЩЕНА?!");

	delete JoinButton;
	FILE* f = NULL;
	fopen_s(&f, "Log_in.me", "wb");
	if (f)
	{
		fclose(f);
	}
	LogLog("ЕЩЕ ОДИН СТРАННЫЙ ФАЙЛ БЫЛ УСПЕШНО СОЗДАН");
	auto LoadingBarText = new NWar3Frame::CWar3Frame();
	LoadingBarText->Load("LoadingBarText");
	while (!LoadingBarText->CheckIsOk())
	{
		Sleep(1);
	}
	Sleep(100);
	LoadingBarText->SetText("|cFF00EF00REBIRTH CLIENT. LOAD MAP!|r");
	delete LoadingBarText;

	TerminateThread(DestroyAutoJoinThreadHandle, 0);
	CloseHandle(AutoJoinThreadHandle);
	CloseHandle(DestroyAutoJoinThreadHandle);

	SkipAllKeys = FALSE;
	return 0;

	//BlockInput( FALSE );

	/*while ( maxwait > 0 )
	{
		int Offset1 = GetFrameItemAddress( "GameListContainer", 0 ) + 180 + 0xDC;
		Offset1 = *( int * )Offset1;
		if ( Offset1 )
		{
			Offset1 = *( int* )( Offset1 + 0x1C8 );
			if ( Offset1 )
			{
				int Offset3 = *( int* )( Offset1 + 0x1F0 );
				int Offset2 = *( int* )( Offset1 + 0x1EC );
				if ( Offset2 > 0 && Offset3 > 0 )
				{
					*( int* )( Offset1 + 0x1F4 ) = Offset2;
					*( int* )( Offset1 + 0x1F8 ) = 0;
					break;
				}
			}
		}
		Sleep( 100 );

		maxwait--;
	}*/

	//int Offset1 = GetFrameItemAddress( "GameListContainer", 0 ) + 180 + 0xDC;
	//Offset1 = *( int * )Offset1;
	//if ( Offset1 )
	//{
	//	Offset1 = *( int* )( Offset1 + 0x1C8 );
	//	if ( Offset1 )
	//	{
	//		int Offset0 = Offset1;
	//		Offset1 = *( int* )( Offset1 + 0x228 );
	//		if ( Offset1 )
	//		{
	//			int Offset2 = *( int* )( Offset1 );
	//			char tete[ 250 ];
	//			sprintf_s( tete, "%X - %X - %X", Offset0, Offset1, Offset2 );
	//			MessageBox( 0, tete, tete, 0 );
	//		}
	//	}
	//}
	//1у
	//EnableWindow(Warcraft3Window, TRUE);
	//Sleep(2500);
	//if ( maxwait > 0 )
	//{
	//	int Offset1 = GetFrameItemAddress( "JoinButton", 0 );
	//	if ( Offset1 )
	//	{
	//		Wc3ControlClickButton_org( Offset1, 1 );
	//	}
	//}
	////EnableWindow(Warcraft3Window, FALSE);
	//Sleep( 2500 );


	//SkipAllKeys = FALSE;
	////EnableWindow( Warcraft3Window, TRUE );

	//Sleep( 2500 );

	//FILE * f = NULL;
	//fopen_s( &f, "Log_in.me", "wb" );
	//if ( f )
	//{
	//	fclose( f );
	//}

	return 0;
}

HWND Warcraft3Window = 0;

WarcraftRealWNDProc WarcraftRealWNDProc_org = NULL;
WarcraftRealWNDProc WarcraftRealWNDProc_ptr;

bool InternalTimerInit = false;

int TimerID = 0;
UINT_PTR timerhndl = NULL;


auto gamelistcontainerframe = new CWar3Frame();

int StopWork = 4;

DWORD StartAutoJoinTime = 0;

LPARAM VKRETURNUP = (LPARAM)(0xC0000001 | (LPARAM)(MapVirtualKey(VK_RETURN, 0) << 16));
LPARAM VKRETURNDOWN = (LPARAM)(0x00000001 | (LPARAM)(MapVirtualKey(VK_RETURN, 0) << 16));

LPARAM VKTABUP = (LPARAM)(0xC0000001 | (LPARAM)(MapVirtualKey(VK_TAB, 0) << 16));
LPARAM VKTABDOWN = (LPARAM)(0x00000001 | (LPARAM)(MapVirtualKey(VK_TAB, 0) << 16));

VOID CALLBACK AutoLoginUpdateEvent(
	HWND hwnd,        // handle to window for timer messages 
	UINT message,     // WM_TIMER message 
	UINT idTimer,     // timer identifier 
	DWORD dwTime)     // current system time 
{

	if (StartAutoJoinTime == 0)
	{
		StartAutoJoinTime = dwTime;
	}

	//LogLogT(dwTime);
	if (!FirstStart)
	{
		if (dwTime - StartAutoJoinTime > 15000)
		{
			RECT rect;

			// get the current window size and position
			GetWindowRect(hwnd, &rect);

			// now change the size, position, and Z order
			// of the window.
			::SetWindowPos(hwnd,       // handle to window
				HWND_NOTOPMOST,  // placement-order handle
				rect.left,     // horizontal position
				rect.top,      // vertical position
				rect.right - rect.left,  // width
				rect.bottom - rect.top, // height
				SWP_SHOWWINDOW); // window-positioning options

			LogLog("Операция: Все кончено :(");
			SkipAllKeys = FALSE;
			FirstStart = TRUE;
			KillTimer(hwnd, idTimer);
			MessageBoxA(hwnd, "Ошибка входа в игру! ЭТО НЕ КРАШ!", 0, 0);
			TerminateProcess(GetCurrentProcess(),0);
			return;
		}
		*(int*)((DWORD)GameDll + 0xA9E7A4) = 1;

		auto War3DialogError = new NWar3Frame::CWar3Frame();
		War3DialogError->Load("DialogText");
		if (War3DialogError->CheckIsOk())
		{
			bool error = false;
			War3DialogError->Load("DialogButtonOK");
			if (War3DialogError->CheckIsOk())
			{
				error = true;
			}
			War3DialogError->Load("DialogButtonYes");
			if (War3DialogError->CheckIsOk())
			{
				error = true;
			}
			War3DialogError->Load("DialogButtonNo");
			if (War3DialogError->CheckIsOk())
			{
				error = true;
			}
			if (error)
			{
				FILE* f = NULL;
				fopen_s(&f, "give_me_cookies2.me", "wb");
				if (f)
				{
					fclose(f);
				}
				LogLog("БЛЯТЬ ОБНАРУЖЕНА КАКАЯ-ТО ОШИБКА ХЗ ЧЕ БУДЕМ ДЕЛАТЬ...");
				WarcraftRealWNDProc_ptr(hwnd, WM_KEYDOWN, VK_RETURN, VKRETURNDOWN);
				WarcraftRealWNDProc_ptr(hwnd, WM_KEYUP, VK_RETURN, VKRETURNUP);
				if (CurrentOperationID > 2)
					CurrentOperationID = 2;
				delete War3DialogError;
				return;
			}
		}
		delete War3DialogError;


		if (CurrentOperationID == -1)
		{
			LogLog("Операция: Инициализация автоматического входа");
			ShowWindow(Warcraft3Window, SW_RESTORE);
			ShowWindow(Warcraft3Window, SW_SHOW);
			RECT rect;

			// get the current window size and position
			GetWindowRect(hwnd, &rect);

			// now change the size, position, and Z order
			// of the window.
			::SetWindowPos(hwnd,       // handle to window
				HWND_TOPMOST,  // placement-order handle
				rect.left,     // horizontal position
				rect.top,      // vertical position
				rect.right - rect.left,  // width
				rect.bottom - rect.top, // height
				SWP_SHOWWINDOW); // window-positioning options
			CurrentOperationID++;
		}
		else if (CurrentOperationID == 0)
		{
			LogLog("Блокировка сообщений окну активирована");
			SkipAllKeys = TRUE;
			CurrentOperationID++;
		}
		else if (CurrentOperationID == 5)
		{
			LogLog("Операция: Ожидание входа всех игроков...");
			auto LoadingBarText = new NWar3Frame::CWar3Frame();
			LoadingBarText->Load("LoadingBarText");
			if (!LoadingBarText->CheckIsOk())
			{
				LogLog("Ожидание остальных игроков....");
				delete LoadingBarText;
				return;
			}
			LoadingBarText->SetText("|cFF00EF00REBIRTH CLIENT. LOAD MAP!|r");
			delete LoadingBarText;
			CurrentOperationID++;
		}
		else if (CurrentOperationID == 6)
		{
			SkipAllKeys = FALSE;
			FirstStart = TRUE;
			CurrentOperationID++;

			RECT rect;

			// get the current window size and position
			GetWindowRect(hwnd, &rect);

			// now change the size, position, and Z order
			// of the window.
			::SetWindowPos(hwnd,       // handle to window
				HWND_NOTOPMOST,  // placement-order handle
				rect.left,     // horizontal position
				rect.top,      // vertical position
				rect.right - rect.left,  // width
				rect.bottom - rect.top, // height
				SWP_SHOWWINDOW); // window-positioning options
			CurrentOperationID++;

			LogLog("Операция: Инициализация автоматического входа успешно завершена");
		}
		else if (CurrentOperationID == 4)
		{
			LogLog("Операция: Нажатие на JoinButton");
			auto JoinButton = new CWar3Frame();
			JoinButton->Load("JoinButton");
			JoinButton->SetFrameType(CFrameType::FRAMETYPE_BUTTON);
			if (!JoinButton->CheckIsOk())
			{
				LogLog("Сбой. Пытаюсь еще раз.");
				delete JoinButton;
				return;
			}
			JoinButton->Click();
			delete JoinButton;
			FILE* f = NULL;
			fopen_s(&f, "Log_in.me", "wb");
			if (f)
			{
				fclose(f);
			}
			LogLog("Операция: Выполнено успешно");
			CurrentOperationID++;
		}
		else if (CurrentOperationID == 2)
		{
			LogLog("Операция: Поиск GameListContainer");
			gamelistcontainerframe->Load("GameListContainer");
			gamelistcontainerframe->SetFrameType(CFrameType::FRAMETYPE_LISTBOX);
			if (!gamelistcontainerframe->CheckIsOk())
			{
				LogLog("Сбой. Пытаюсь еще раз.");
				return;
			}
			LogLog("Операция: Выполнено успешно");
			CurrentOperationID++;
		}
		else if (CurrentOperationID == 3)
		{
			LogLog("Операция: Выделение игры");
			// LOCAL GAME LAYOUT
			int Offset1 = *(int*)(gamelistcontainerframe->FrameAddr + 0x190);

			if (!Offset1)
			{
				LogLog("Сбой. Пытаюсь еще раз.");
				return;
			}

			if (Offset1)
			{
				LogLog("Операция: Выделение игры. Этап 1 выполнен.");
				// LISTBOX WITH GAMES
				int Offset2 = *(int*)(Offset1 + 0x1C8);
				if (!Offset2)
				{
					LogLog("Сбой. Пытаюсь еще раз.");
					return;
				}
				if (Offset2)
				{
					LogLog("Операция: Выделение игры. Этап 2 выполнен.");
					// ITEM COUNT
					int Offset3 = *(int*)(Offset2 + 0x1F0);
					if (!Offset3)
					{
						LogLog("Сбой. Пытаюсь еще раз.");
						return;
					}
					if (Offset3 > 0)
					{
						LogLog("Операция: Выделение игры. Этап 3 выполнен.");
						// ITEM LIST
						int Offset4 = *(int*)(Offset2 + 0x228);
						if (!Offset4)
						{
							LogLog("Сбой. Пытаюсь еще раз.");
							return;
						}
						if (Offset4 > 0)
						{
							LogLog("Операция: Выделение игры. Этап 4 выполнен.");
							// FIRST ITEM
							Offset4 = *(int*)(Offset4);
							if (!Offset4)
							{
								LogLog("Сбой. Пытаюсь еще раз.");
								return;
							}
							if (Offset4 > 0)
							{
								LogLog("Операция: Выделение игры. Этап 5 выполнен.");
								int statusOffset4 = *(int*)(Offset4 + 4);
								if (!(statusOffset4 >= 2))
								{
									LogLog("Сбой. Пытаюсь еще раз.");
									return;
								}
								if (statusOffset4 >= 2)
								{
									FILE* f = NULL;
									fopen_s(&f, "give_me_cookies.me", "wb");
									if (f)
									{
										fclose(f);
									}
									/**(int*)(Offset2 + 0x1F4) = Offset4;
									*(int*)(Offset2 + 0x1F8) = 0;*/

									/**(int*)((DWORD)GameDll + 0xA8B038 + 0xC) = Offset4;
									*(int*)((DWORD)GameDll + 0xA8B038 + 0x8) = 0;
									*(int*)((DWORD)GameDll + 0xA8B038 + 0x10) = 1;
									*(int*)((DWORD)GameDll + 0xA8B038 + 0x14) = Offset4;
									*(int*)((DWORD)GameDll + 0xA8B038 + 0x18) = (DWORD)GameDll + 0x95AB80;
									Wc3SimulateClickEventListBox(Offset2, (DWORD)GameDll + 0xA8B038);*/

									for (int x = 0; x < 10; x++)
									{
										WarcraftRealWNDProc_ptr(hwnd, WM_KEYDOWN, VK_TAB, VKTABDOWN);
										WarcraftRealWNDProc_ptr(hwnd, WM_KEYUP, VK_TAB, VKTABUP);
									}
									
									LogLog("Операция: Выделение игры. Этап 6 и последний выполнен.");
								}
								else
								{
									LogLog("Сильный Сбой. Пытаюсь еще раз.");
									return;
								}
							}
						}
					}
				}
			}

			delete gamelistcontainerframe;
			LogLog("Операция: Выполнено успешно");
			CurrentOperationID++;
		}
		else if (CurrentOperationID == 1)
		{
			LogLog("Операция: Нажатие на LocalAreaNetworkButton");
			auto LocalAreaNetworkButton = new CWar3Frame();
			LocalAreaNetworkButton->Load("LocalAreaNetworkButton");
			LocalAreaNetworkButton->SetFrameType(CFrameType::FRAMETYPE_BUTTON);
			if (!LocalAreaNetworkButton->CheckIsOk())
			{
				LogLog("Сбой. Пытаюсь еще раз.");
				delete LocalAreaNetworkButton;
				return;
			}
			int clicksuccess = LocalAreaNetworkButton->Click();/*
			LogLog(("Нажатие на LocalAreaNetworkButton прошло успешно ? :" + std::to_string(clicksuccess)).c_str());
			if (clicksuccess != 1)
			{
				LogLog("Нефига, давай еще одну попытку...");

				Sleep(100);
				if (LocalAreaNetworkButton->CheckIsOk())
				{
					LocalAreaNetworkButton->SetFocus();
					LocalAreaNetworkButton->Click();
				}
			}*/
			delete LocalAreaNetworkButton;
			LogLog("Операция: Выполнено успешно.");
			CurrentOperationID++;
		}
	}

	if (LoadedMssPlugins != 5)
	{
		StopWork = 5;
		MH_DisableHook(WarcraftRealWNDProc_org);
		MH_EnableHook(WarcraftRealWNDProc_org);
	}
}



LRESULT __fastcall ОбработчикСообщенийОкну(HWND hWnd, unsigned int _Msg, WPARAM _wParam, LPARAM lParam)
{
	unsigned int Msg = _Msg;
	WPARAM wParam = _wParam;

	if (_Msg == WM_TIMER)
	{
		return WarcraftRealWNDProc_ptr(hWnd, Msg, wParam, lParam);
	}

	if (hWnd != NULL)
	{
		if (!InternalTimerInit)
		{
			InternalTimerInit = true;
			srand((unsigned int)time(NULL));
			TimerID = 401 + (rand() % 500);

			timerhndl = SetTimer(hWnd, TimerID, 50, AutoLoginUpdateEvent);
		}
	}

	if (Warcraft3Window == 0)
	{
		Warcraft3Window = hWnd;
	}

	if (SkipAllKeys)
		return 0;
	if (StopWork == 5)
	{
		return 0;
	}
	return WarcraftRealWNDProc_ptr(hWnd, Msg, wParam, lParam);
}

int __fastcall InitWc3MainMenu_my(void* a1, int unused, int a2, int a3)
{
	int retvalue = InitWc3MainMenu_ptr(a1, unused, a2, a3);

	FirstStart = FALSE;
	LASTINITIALIZED2 = GetTickCount();
	//char data[256];
	//sprintf_s(data, "%X %X %X %X", a1, unused, a2, a3);
	//MessageBox(0, data, 0, 0);
	//retvalue = InitLocalGameMenu(a1, (int)(void *)&InitLocalGameMenu, a2, 8);
	return retvalue;
}
//
//-------------------------- -
//Ошибка
//-------------------------- -
//106100B0 6F58BD20 0 2
//-------------------------- -
//ОК
//-------------------------- -
////
//
//-------------------------- -
//Ошибка
//-------------------------- -
//111D00C0 6F57CBC0 0 8
//-------------------------- -
//ОК
//-------------------------- -


void ЗапускАвтоматическогоВхода()
{
	InitWc3MainMenu_org = (InitFrameDef)((DWORD)GameDll + 0x58BD20);//1.27a 0x2BE270
	InitLocalGameMenu = (InitFrameDef)((DWORD)GameDll + 0x57CBC0);

	MH_CreateHook(InitWc3MainMenu_org, &InitWc3MainMenu_my, reinterpret_cast<void**>(&InitWc3MainMenu_ptr));
	MH_EnableHook(InitWc3MainMenu_org);

	WarcraftRealWNDProc_org = (WarcraftRealWNDProc)((DWORD)GameDll + 0x6C6AA0);
	MH_CreateHook(WarcraftRealWNDProc_org, &ОбработчикСообщенийОкну, reinterpret_cast<void**>(&WarcraftRealWNDProc_ptr));
	MH_EnableHook(WarcraftRealWNDProc_org);



	//AutoJoinThreadHandle = CreateThread(0, 0, НовыйПотокДляАвтоматическогоВхода, 0, 0, 0);
}

