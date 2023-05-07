/*
������һ����λ�Ҽ�Ŀ��Ϊһ�����˵�λ�ԣ�TODO �������ƣ���

����Ŀ�굥λ�������Ʋ�Ŀ�굥λ���ƶ�����
�ӵ�λ�ĵ�ǰ���� p �Ʋ��һ���µ�ƫ�����꣺
��ǰ���꣬��ǰ�ƶ�����ƫ��54 * (�ƶ��ٶ�/270)���룬�����Ϊp' TODO �����ӳ�

����Ҽ�Ŀ�������p'Ϊ���ģ�2*dΪ�߳��ķ��η�Χ�ڣ���clip�������⡣

���� d ����Ŀ�굥λ����ײ���������С�����16~31����d = 118
�����������32~ ����d = 128
*/

#include "stdafx.h"
#include "DreamWar3Main.h"

namespace Blocking
{
	void onActionSent(const Event* evt) 
	{
		ActionEventData *data = evt->data<ActionEventData>();
		if (	!data->byProgramm && 
			data->id == ACTION_SMART&& 
			data->target == NULL &&
			!(data->flag & Queued)
		){
			Point pointClicked = Point(data->x, data->y);

			UnitGroup *group;
			group = GroupUnitsOfPlayerSelected(PlayerLocal(), false);

			//�ж���ѡ��λ��һ����λ TODO ����Ϊ�ɿ��Ƶ�λ!
			if (group->size() == 1)
			{
				Unit *blocker = group->getUnit(0);
				//��ȡ��Χ�ڷ�����λ
				UnitGroup *g_targets;
				g_targets = GroupUnitsInRange(data->x, data->y, 300);
				g_targets->remove(blocker);
				//TODO �ж��ǵ��ˣ���ʬ�壬�ǿվ���������

				if (g_targets->size() > 0) //TODO ��δ�������λ
				{
					Unit *target = g_targets->nearestUnit(blocker->position());
					Point p = target->position();
					float angle = target->direction();

					if (target->currentOrder() == ACTION_SMART) //TODO ���������ж�
					{
						angle = p.angleTo(target->currentTargetPoint());

						//TODO �����ӳ�
						p = p.offsetPolar( 
							0.25f * target->moveSpeed(),
							angle
						);
					}

					float dist = 96.f + max(0, (target->moveSpeed() - blocker->moveSpeed())*0.25f);

					if ( pointClicked.maxXYDistanceTo(p) < dist )
					{
						pointClicked = p.offsetPolarMaxXY( dist, angle );

						//��д����
						PacketSenderDestroy (data->packetSender);
						blocker->sendAction(	
							ACTION_SMART,
							Target,
							None,
							NULL,
							pointClicked,
							NULL,
							true
						);

						OutputScreen(1, "action! %s", pointClicked.toString().c_str());
					}

				}

				GroupDestroy(g_targets);
			}

			GroupDestroy(group);
		}

		

	}

	void Init()
	{
		//��Ȿ�ز�����smart / move��һ�������
		//MainDispatcher()->listen(EVENT_LOCAL_ACTION, onActionSent);
	}

	void Cleanup()
	{

	}
}