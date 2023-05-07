/*
����Ŀ�굥λ�������Ʋ�Ŀ�굥λ���ƶ�����
�ӵ�λ�ĵ�ǰ���� p �Ʋ��һ���µ�ƫ�����꣺
��ǰ���꣬��ǰ�ƶ�����ƫ��54 * (�ƶ��ٶ�/270)���룬�����Ϊp'

ָ��һ�����򣨶������������ƶ��ҷ���λ��p'�ĸ÷����d����

���� d ����Ŀ�굥λ����ײ���������С�����16~31����d = 118
�����������32~ ����d = 128

Ϊ�˷�������4����λ��Ŀ�굥λ�Ķ�����������Ҫ���λ�÷����㷨��
����λ�÷��������С�������е�λ����λ��������ʱ�䡱��
��Ȼ��Χɱ�ĳɹ�ȡ����ȫԱ��ϵ�ʱ���ܷ���̡�

�Զ�������4�����꣬�����������ҷ���λ���룬�õ��ҷ�����ÿ����������ĵ�λ���Լ����롣
�ҳ�����ÿ�����������λ�У�ʵ�ʾ�����Զ��һ�����������������꣬��Ǹ�����λ��Ϊ����ɡ�
�Դ������������������λ��
*/


#include "stdafx.h"
#include "DreamWar3Main.h"
#define _USE_MATH_DEFINES
#include <math.h>

namespace Surrounding {

	void PointShiftMinXY(Point pointToGo, Point &rv, Unit *inUnit, Unit *inUnitTarget) {
		float dx = (pointToGo.x - inUnitTarget->x());
		float dy = (pointToGo.y - inUnitTarget->y());
		float dxUnit = (inUnit->x() - inUnitTarget->x());
		float dyUnit = (inUnit->y() - inUnitTarget->y());
		if((abs(dxUnit) > abs(dyUnit)) && (abs(dx) > abs(dy)) && (dxUnit * dx > 0)) {//modify dx
			float dxUnitAbsShrink = max(abs(dxUnit)-25, 64);
			rv = Point( inUnitTarget->x() + (min(abs(dx), dxUnitAbsShrink) * (dx/abs(dx))), pointToGo.y);
			return;
		}
		else if ((abs(dyUnit) > abs(dxUnit)) && (abs(dy) > abs(dx)) && (dyUnit * dy > 0)){//modify dy
			float dyUnitAbsShrink = max(abs(dyUnit)-25, 64);
			rv = Point(pointToGo.x, inUnitTarget->y()+ (min(abs(dy), dyUnitAbsShrink) * (dy/abs(dy))) );
			return;
		}
		rv = Point(pointToGo.x, pointToGo.y);
	}


	//describes 4-direction relationship of units
	struct SurrounderGroup
	{
		Point destination;
		UnitGroup units;
	};

	void SurroundArrange (UnitGroup *inGroupSurrounder, Unit *inUnitTarget) 
	{
		//Point pointsToMove[4];
		float surroundDist = 128.0f;

		Point p = inUnitTarget->position();

		Point pDestination = inUnitTarget->currentTargetPoint();

		p = p.offsetPolar(54.f * inUnitTarget->moveSpeed() / 270.f, p.angleTo(pDestination));


		SurrounderGroup Surrounders[4];
		Surrounders[0].destination = p.offset(-surroundDist, 0);//west
		Surrounders[1].destination = p.offset(0, -surroundDist);//south
		Surrounders[2].destination = p.offset(surroundDist, 0);//east
		Surrounders[3].destination = p.offset(0, surroundDist);//north

		for( uint32_t i = 0; i < inGroupSurrounder->size(); i++ )
		{
			Unit *u = inGroupSurrounder->getUnit(i);
			//sort unit into surrounder group by direction
			float uDirection = p.angleTo(u->position());
			float minDiffDirection = float(M_PI);
			int minDiffDirectionIdx = -1;
			for ( int surrounderIdx = 0; surrounderIdx < 4; surrounderIdx++ )
			{
				float surrounderDirection = p.angleTo(Surrounders[surrounderIdx].destination);

				float diffDirection = fmod(float(abs(uDirection - surrounderDirection)), float(M_PI * 2.f));
				if ( diffDirection > M_PI )
					diffDirection -= float(M_PI * 2);//-PI ~ PI
				diffDirection = abs(diffDirection);//0 ~ PI

				if ( diffDirection < minDiffDirection)
				{
					minDiffDirection = diffDirection;
					minDiffDirectionIdx = surrounderIdx;
				}
			}
			if ( minDiffDirectionIdx > -1 )
			{
				Surrounders[minDiffDirectionIdx].units.add(u);
			}
		}

		//re-arrange units if not all sides are covered. starting from enemy flee side.
		

		/*
		DreamWar3 algorithm:
		split all surround units by their direction to the target point ( pDestination )
		start arranging points from the direction(N,W,E,S) that the target unit flees.

		if there is no surround unit on that direction, find the unit that forms smallest angle between unit-pDestination-direction
		and shift that unit to the direction
		repeat for all directions until there exist unit on each direction

		*/



		//bool pointTaskArranged[4] = {0}; uint32_t pointTaskSolvedCount = 0;

		//uint32_t i; float dist, maxDist; Unit *u, *maxDistUnit; int maxDistPointIndex;

		//Jass::ClearTextMessages();

		
		////we set a final command for these units: "move" t the target ( same as player sent action which was destroyed )
		//inGroupSurrounder->sendAction(
		//	ACTION_MOVE,
		//	Target,
		//	None,
		//	NULL,
		//	inUnitTarget->position(),
		//	inUnitTarget,
		//	NULL,
		//	true);

		//while ( pointTaskSolvedCount < 4) 
		//{
		//	maxDist = 0; maxDistUnit = NULL;
		//	for (i = 0; i < 4; i++)
		//	{
		//		if (!pointTaskArranged[i])
		//		{
		//			u = inGroupSurrounder->nearestUnit(pointsToMove[i], &dist);
		//			if (maxDist < dist) 
		//			{
		//				maxDist = dist; maxDistUnit = u; maxDistPointIndex = i;
		//			}
		//		}
		//	}
		//	if (maxDistUnit)
		//	{
		//		pointTaskArranged[maxDistPointIndex] = true;
		//		inGroupSurrounder->remove(maxDistUnit);//remove unit from group
		//		Point p;
		//		PointShiftMinXY(
		//			pointsToMove[maxDistPointIndex],
		//			p,
		//			maxDistUnit,
		//			inUnitTarget
		//		);
		//		maxDistUnit->sendAction(	
		//			ACTION_SMART,
		//			Target,
		//			Preemt,
		//			NULL,
		//			p,
		//			NULL,
		//			true
		//		);
		//		//OutputScreen(10, "point : %s", pointsToMove[maxDistPointIndex].toString().c_str());
		//	}
		//
		//	pointTaskSolvedCount++;
		//}
	}


	/*
	�¼�����ҷ����ƶ���һ���з���λ����ѡ����4����λ
	*/
	void onActionSent(const Event* evt) 
	{
		int enabled = ProfileFetchInt("Surround", "Enable", 1);
		if (!enabled) return;
		ActionEventData *data = evt->data<ActionEventData>();
		UnitGroup *group;
		if (	!data->byProgramm && 
				data->id == ACTION_MOVE && 
				data->target != NULL &&
				!(data->flag & Queued)
		)
		{
			Unit *u = GetUnit(data->target);//Ҫ�ж���unit����
			group = GroupUnitsOfPlayerSelected(PlayerLocal(), false);
			if (
				//u->isEnemyToLocalPlayer() && 
				group->size() >= 4)
			{
				PacketSenderDestroy (data->packetSender);
				SurroundArrange(group, u);
			}
			GroupDestroy(group);
		}
	}

	void onAltSmartSent(const Event* evt) {
		int enabled = ProfileFetchInt("Surround", "Enable", 1);
		int altRightClickMoveEnabled = ProfileFetchInt("Surround", "AllowAltRightClickMove", 0);
		if (!enabled) return;
		if (!altRightClickMoveEnabled) return;

		ActionEventData *data = evt->data<ActionEventData>();
		if (	!data->byProgramm
			&&	data->id == ACTION_SMART
			&&	!(data->flag & Queued)
			&&	KeyIsDown(KEYCODE::KEY_ALT)
			&&	!KeyIsDown(KEYCODE::KEY_CONTROL)
			&&	!KeyIsDown(KEYCODE::KEY_SHIFT)
		){
			//�޸�ΪMOVE
			PacketSenderDestroy(data->packetSender);
			UnitGroup *group = GroupUnitsOfPlayerSelected(PlayerLocal(), false);
			Unit *targetUnit = NULL;
			if (data->target){
				if (Jass::GetUnitTypeId(data->target))	{	//����ж��ǵ�λ?
					targetUnit = GetUnit(data->target);
				}
			}
			group->sendAction(
				ACTION_MOVE,
				Target,
				GroupPathing | IgnoreFormation ,
				NULL,
				Point(data->x, data->y),
				targetUnit,
				NULL,
				true,
				false);//α�����Ҳ���
			GroupDestroy(group);
		}

	}

	void Init () {
		//MainDispatcher()->listen(EVENT_LOCAL_ACTION, onAltSmartSent);
		MainDispatcher()->listen(EVENT_LOCAL_ACTION, onActionSent);
	}

}