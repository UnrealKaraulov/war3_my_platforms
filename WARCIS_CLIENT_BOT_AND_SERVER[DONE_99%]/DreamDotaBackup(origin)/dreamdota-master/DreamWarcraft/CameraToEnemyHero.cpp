#include "stdafx.h"
#include "DreamWar3Main.h"

namespace CameraToEnemyHero 
{
	void onKeyEvent (const Event *evt)
	{
		KeyboardEventData *data = evt->data<KeyboardEventData>();
		if (	data->code >= KEYCODE::KEY_1 && 
				data->code <= KEYCODE::KEY_9 && 
				data->alt && 
				!data->ctrl && 
				!data->shift		)
		{
			//����ԭ��Ϣ
			data->discard();
			//Ѱ�ҵ����������Ӣ��, ����=�Ա�����Ҳ��Ѻû���û�й�����Ұ
			UnitGroup *groupAllHeroes = 
				GroupUnits(
					NULL,
					NULL,
					PlayerGroup(
						NULL,
						PlayerFilter::ALLIANCE_PASSIVE | PlayerFilter::ALLIANCE_SHARED_VISION,
						PlayerLocal()
					),
					NULL,
					UnitFilter(
						UnitFilter::HERO,
						NULL
					)
				);
			//UnitGroupĬ�Ͼ��ǰ�ѡ�����ȼ�����

			int index = (data->code - KEYCODE::KEY_1);
			Unit *hero = groupAllHeroes->getUnit(index);
			if (	hero != NULL &&
					hero->life() > 0		)
			{
				Jass::SetCameraPosition(hero->x(), hero->y());
				//TODO: IsUnitInTransport / IsUnitLoaded
			}
			//���alt 1û��Ӣ�ۣ��л������˻���
			if (index==0 && hero == NULL) 
			{
				UnitGroup *groupBase = 
					GroupUnits(
						NULL,
						NULL,
						PlayerGroupAll(),
						NULL,
						UnitFilter(
							UnitFilter::TOWNHALL | UnitFilter::ENEMY,
							NULL,
							PlayerLocal()
						), 1
					);

				if (groupBase->size()) 
				{
					Unit *base = groupBase->getUnit(0);
					Jass::SetCameraPosition(base->x(), base->y());
				}
				GroupDestroy(groupBase);
			}


			GroupDestroy(groupAllHeroes);
		}
	}


	void Init() 
	{
		//alt + ����
		MainDispatcher()->listen(EVENT_KEY_DOWN, onKeyEvent);
	}

	void Cleanup()
	{

	}

}