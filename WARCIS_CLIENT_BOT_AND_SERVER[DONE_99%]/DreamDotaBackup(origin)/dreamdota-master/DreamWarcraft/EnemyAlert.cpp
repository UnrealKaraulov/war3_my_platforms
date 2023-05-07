/*
���з���λ�ɲ��ɼ���Ϊ�ɼ�ʱ�������档���ɿ��Ǻ��ʵ��ӳ� + ���жϻ���ʱ�䣩

�����棺��λ�ɲ��ɼ���Ϊ����⣨���ҷ�����������
�����棺����ĵ�λ���ڵ��ҽ�ս��Χ�ڣ���ս�еб���Ԯ��

*/
#include "stdafx.h"
#include "DreamWar3Main.h"

namespace EnemyAlert {

	const char *underAttackSoundLink(bool useTownAlert, bool useAllyAlert) {
		switch(PlayerRace(PlayerLocal())) {
		case Jass::RACE_HUMAN:
			if (!useTownAlert && !useAllyAlert)	return "UnderAttackHuman";
			if (!useTownAlert && useAllyAlert)	return "AllyUnderAttackHuman";
			if (useTownAlert && !useAllyAlert)	return "TownAttackHuman";
			if (useTownAlert && useAllyAlert)	return "AllyTownUnderAttackHuman";
			break;
		case Jass::RACE_ORC:
			if (!useTownAlert && !useAllyAlert)	return "UnderAttackOrc";
			if (!useTownAlert && useAllyAlert)	return "AllyUnderAttackOrc";
			if (useTownAlert && !useAllyAlert)	return "TownAttackOrc";
			if (useTownAlert && useAllyAlert)	return "AllyTownUnderAttackOrc";
			break;
		case Jass::RACE_UNDEAD:
			if (!useTownAlert && !useAllyAlert)	return "UnderAttackUndead";
			if (!useTownAlert && useAllyAlert)	return "AllyUnderAttackUndead";
			if (useTownAlert && !useAllyAlert)	return "TownAttackUndead";
			if (useTownAlert && useAllyAlert)	return "AllyTownUnderAttackUndead";
			break;
		case Jass::RACE_NIGHTELF:
			if (!useTownAlert && !useAllyAlert)	return "UnderAttackNightElf";
			if (!useTownAlert && useAllyAlert)	return "AllyUnderAttackNightElf";
			if (useTownAlert && !useAllyAlert)	return "TownAttackNightElf";
			if (useTownAlert && useAllyAlert)	return "AllyTownUnderAttackNightElf";
			break;
		}
		return NULL;
	}

	static std::set<Point> LastAlertPoints;

	static float NotifyInitDelay;
	static float NotifyCycleTime;
	static float NotifyDelay;
	static float NotifyRange;
	static Timer* NotifyCycleTimer;

	void AlertPointClear(Timer *eventTimer) {
		Point *p = eventTimer->data<Point>();
		LastAlertPoints.erase(*p);
		//OutputScreen(10, "Point %s erased.", p->toString().c_str());
	}

	void AlertPoint(Point p, bool useTownAlert, bool useAllyAlert) {
		std::set<Point>::iterator iter;
		for (iter = LastAlertPoints.begin(); iter != LastAlertPoints.end(); ++iter) {
			if ((*iter).distanceTo(p) < NotifyRange) return;
		}

		//OutputScreen(10, "Alert : %s", p.toString().c_str());
		PingMinimapEx(p.x, p.y, 3, 255, 0, 0, false);
		Jass::SetCameraQuickPosition(p.x, p.y);
		SoundPlay(underAttackSoundLink(useTownAlert, useAllyAlert), NULL, 0);

		//�ص����ʣ�һ������֮��������Ļ��Χ�����ü����x�룩���ڼ������֮ǰ�������Χ�ڷ����ľ��汻����
		LastAlertPoints.insert(p);
		Timer *tm = GetTimer(NotifyDelay, AlertPointClear, false);
		tm->setData<Point>(&p);
		tm->start();
		
	}

	void alertPerform(Unit *u) {
		//�����棺������ϴ�λ��û�б仯�ĵ�λ�����粻���ƶ��Ľ�����
		if (u->position() == u->visibleState.lastVisiblePoint) { return; }

		//�����棺�ڵ�ǰ��Ļ
		if (u->position().distanceTo(Point(Jass::GetCameraTargetPositionX(), Jass::GetCameraTargetPositionY())) < NotifyRange){
			return;
		}

		//��ȡ���뵥λ����ĵ�λ
		UnitGroup *g = GroupUnits(
			NULL,
			NULL,
			PlayerGroup(
				PlayerFilter::ALLIANCE_PASSIVE | PlayerFilter::ALLIANCE_SHARED_CONTROL,
				NULL,
				PlayerLocal()
			),
			&RegionCircle(
				u->position(),
				2000.f
			),
			UnitFilter()
		);
		//
		Unit *alertUnit = g->nearestUnit(u->position());
		if (alertUnit) {
			AlertPoint(
				u->position().offsetPolar(
					min( 500, u->distanceToUnit(alertUnit) ), 
					u->angleToUnit(alertUnit)
				),
				alertUnit->testFlag(UnitFilter::STRUCTURE),
				alertUnit->owner() != PlayerLocal()
			);
		}

		GroupDestroy(g);
	}

	void onTimerAction (Timer *eventTimer){
		
		//��λ�����ޣ�������ұ�������Ϸ״̬��������, ��Ұ�֣���λ����Ϊ������ҵ��ˣ����ٻ������������ǻ���
		UnitGroup *g = GroupUnits(
			NULL,
			NULL,
			PlayerGroup(
				PlayerFilter::STATE_PLAYING,
				PlayerFilter::CONTROL_NEUTRAL |	PlayerFilter::CONTROL_CREEP
			),
			NULL,
			UnitFilter(
				UnitFilter::ENEMY,
				UnitFilter::SUMMONED | UnitFilter::WARD | UnitFilter::ILLUSION,
				PlayerLocal()
			)
		);
		
		//��ÿ����λ: ��������visible״̬�Լ�����ʱ��, ����ɲ��ɼ����ɼ�������
		GroupForEachUnit(g, enumUnit,
			bool visible = enumUnit->testFlag(UnitFilter::VISIBLE, PlayerLocal());
			if (!enumUnit->visibleState.lastVisible && visible) {
				if (Time() > NotifyInitDelay) {//�ж���Ϸʱ��, ǰ15�벻����
					alertPerform(enumUnit);
				}
			}
			enumUnit->visibleState.lastVisible = visible;
			enumUnit->visibleState.lastVisiblePoint = enumUnit->position();
		);

		GroupDestroy(g);
	}

	void Init () {
		NotifyInitDelay = 15.0f;
		NotifyCycleTime = 0.5f;
		NotifyDelay = GameDataProfileGetFloat("Misc", "AttackNotifyDelay", 0);
		NotifyRange = GameDataProfileGetFloat("Misc", "AttackNotifyRange", 0);

		NotifyCycleTimer = GetTimer(NotifyCycleTime, onTimerAction, true);
		NotifyCycleTimer->start();
	}
}