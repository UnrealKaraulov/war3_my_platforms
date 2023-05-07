#include "stdafx.h"

#include "DreamWar3Main.h"
#include "Latency.h"

namespace ShowLatency
{
	static Label *LbPing;

	void onTimer ( Timer *tm )
	{
		if (LbPing && LbPing->isShown())
		{
			float lag = LatencyGetAverage();
			if (lag > 0)
			{
				std::string str;
				if (lag < 0.150f)
				{
					str = "|CFF00FF00";	//��ɫ
				}
				else if (lag < 0.250f)
				{
					str = "|CFF80FF00";	//����ɫ
				}
				else if (lag < 0.300f)
				{
					str = "|CFFFFFF00"; //��ɫ
				}
				else if (lag < 0.350f)
				{
					str = "|CFFFF8000";	//�ٻ�ɫ
				}
				else if (lag < 0.400f)
				{
					str = "|CFFFF4000";	//�ٺ�ɫ
				}
				else
				{
					str = "|CFFFF0000";	//��ɫ
				}

				LbPing->setText( "%s%d|r ms", str.c_str(), (int)(lag*1000)	);
			}
			else
				LbPing->setText( "Lag: N/A" );
		}
	}

	void onLocalChat (const Event *evt) 
	{
		LocalChatEventData* data = evt->data<LocalChatEventData>(); //TODO ��Сд
		if (
				strcmp(data->content, "/ping") == 0
			||	strcmp(data->content, "/latency") == 0
			||	strcmp(data->content, "/lag") == 0
			||	strcmp(data->content, "/delay") == 0
			)
		{
			data->content[0] = '\0';
			DiscardCurrentEvent();
			LbPing->show(!LbPing->isShown());//TODO �ҵ���������ķ�ʽ
		}
	}

	void Init()
	{
		LbPing = new Label(UI_NULL, "", 0.011f);
		LbPing->setAbsolutePosition(POS_T, 0.4f, 0.547f);
		LbPing->show(false);
		if (	ReplayState() != REPLAY_STATE_STREAMINGOUT
			&& !Jass::IsPlayerObserver(Jass::Player(PlayerLocal())) )
		{
			MainDispatcher()->listen(EVENT_LOCAL_CHAT, onLocalChat);
			GetTimer(0.5, onTimer, true)->start();
		}
	}

	void Cleanup()
	{
		delete LbPing;
		LbPing = NULL;
	}
}