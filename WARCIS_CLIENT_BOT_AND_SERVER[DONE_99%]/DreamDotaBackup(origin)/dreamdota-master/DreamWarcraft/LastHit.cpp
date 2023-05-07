/*
����

TODO

�о�����ж�����ʹ��0����1
���safedelay����Ĺ�����ʽ

����Ԥ�ⵥλ����ʱ���У����ӿ��ǲ����߱���ǰ����CD����������ƶ�������������ҵ��ƶ����������µĹ���cd���Ŷ�����

��������acquire�¼�����Ӧ����ͬʱҪ���Ƕ���ʱ��ĵ���ȡ��������risk����μ��㣡����
������������ϵͳ�������Ż�����Ŀ���ѡ�񣬰��������Ƿ�ֻ�����ȣ���
��������ʾ���ߵ�debugϵͳ������
��miss�¼�ʱ��Ӧ������update cooldownǿ�Ƹ��£���

*/





#include "stdafx.h"
#include <VMP.h>
#include "DreamDotaMain.h"
#include <process.h>
#include "Benchmark.h" 

#include "DebugPanel.h"

namespace LastHit {

	void LastHitUnitInitProc(Unit *u);
	void LastHitUnitEndProc(Unit *u);

	uint32_t GetHotkey(){
		return ProfileFetchInt("LastHit", "Hotkey", KEYCODE::KEY_TILDE);
	}
	

	
	typedef std::map<float, DnDAtkDist, DistributionLess>	damagemaptype;
	typedef std::map<float, float, DistributionLess>		distributiontype;
	typedef std::map<Unit*, float>		unitvaluetype;

	static UnitGroup				LastHitUnits;
	static unitvaluetype			UpdateCooldownMap;
	static float					ReactionDelay;			//ƽ���Գ��������Ӧ�ӳ�
	static bool						EnableHit;
	static bool						EnableDeny;
	
	static UnitGroup* UnitAttackers;

	//����
	static const float	PREDICT_TIME_MAX = 6.f;
	static const float	PREDICT_EXACT_TIME_MAX = 4.f;
	static const int	PREDICT_EXACT_COUNT_MAX = 15;
	static const int	PREDICT_COUNT_MAX = 30;
	static const int	PREDICT_COUNT_MAX_SINGLE = 8;
	static const float	PREDICT_COOLDOWN = 0.15f;	//�Ƿ�Բ�ͬ�Ĺ����߽����жϾ��� ?
	static const float	PREDICT_MIN_DISTANCE = 1500.f;//��ʾ��λ���پ���һ�������߶�Զ�Ż�Ԥ��
	
	static const int STRATEGY_ERROR = -1;
	static const int STRATEGY_DONTMIND		= 0;
	static const int STRATEGY_ENEMYONLY		= 1;
	static const int STRATEGY_ALLYONLY		= 2;

	struct Action	//TODO ��׼��
	{
		uint32_t	actionId;
		ActionType	actionType;
		uint32_t	actionFlag;
		Item		*usingItem;
		Point		targetPoint;
		Unit		*targetUnit;
	};

	static std::map<Unit*, Action> UnitLastActionMap;


	int LastHitStrategy() {
		if (EnableHit && EnableDeny) return STRATEGY_DONTMIND;
		else if (EnableHit && !EnableDeny) return STRATEGY_ENEMYONLY;
		else if (!EnableHit && EnableDeny) return STRATEGY_ALLYONLY;
		else return STRATEGY_ERROR;
	}

	void NormalAttack(Unit *attacker, Unit *target) {
		if (!(attacker && target))
			return;

		attacker->sendAction(
			ACTION_ATTACK,
			Target,
			None,
			NULL,
			target->position(),
			target,
			true
		);
	}

	void SafeDelayAttack_waited(Timer *tm){
		std::pair<Unit *, Unit *> *pData = tm->data<std::pair<Unit *, Unit *>>();
		Unit *attacker = pData->first;
		Unit *target = pData->second;

		if (!(attacker && target))
			return;

		NormalAttack(attacker, target);
	}

	void SafeDelayAttack(Unit *attacker, Unit *target){
		if (!(attacker && target))
			return;

		attacker->sendAction(
			ACTION_SMART,
			Target,
			None,
			NULL,
			attacker->position(),
			NULL,
			true
		);
		Timer *tm = GetTimer(0.03, SafeDelayAttack_waited);
		std::pair<Unit *, Unit *> data (attacker, target);
		tm->setData<std::pair<Unit *, Unit *>>(&data);
		tm->start();
	}
	
	void StandStillAndCool(Unit *attacker) {
		if (!attacker)
			return;

		attacker->sendAction(
			ACTION_ATTACKGROUND,
			TargetPoint,
			None,
			NULL,
			attacker->position().offsetPolar(50.f, attacker->direction()),
			NULL,
			true
		);
	}

	bool UnitIsStandStillAndCool (Unit *attacker) {//TODO ʹ�ñ���ϵͳ
		if (!attacker)
			return false;
		return (attacker->currentOrder() == ACTION_ATTACKGROUND);
	}

	bool UnitIsValid(Unit *u) {
		if (u) {
			if (u->handleId()){
				if (u->life() > 0){
					return true;
				}
			}
		}
		return false;
	}

	bool UnitNearAnyLastHitter (Unit *u, float minimumDistance){
		int lastHitUnitCount = LastHitUnits.size();//����������
		if (lastHitUnitCount <= 0) return false;//�������������Ϊ0����λ�϶������κβ����߸���
		if (!UnitIsValid(u)) return false;//�����λ������Ч���϶������κβ����߸���
		GroupForEachUnit(&LastHitUnits, lasthitter,
			if (UnitIsValid(lasthitter) && u->distanceToUnit(lasthitter) <= minimumDistance){
				return true;
			}
		);
		return false;
	}

	UnitGroup *UnitGetAttackers (Unit *u, float range){
		if (UnitAttackers) {
			GroupDestroy(UnitAttackers); 
			UnitAttackers = NULL;
		}

		if (!u)
			return UnitAttackers = new UnitGroup();

		UnitGroup *tmpGroup = GroupUnits(
			NULL,
			NULL,
			PlayerGroupAll(),
			&RegionCircle(u->position(), range),//ֻ���ǵ�λ�����Ĺ����� TODO Ӧ�ø�Ϊsight ?
			UnitFilter(
				NULL,
				(UnitFilter::DEAD | UnitFilter::HIDDEN)
			)
		);

		UnitAttackers = new UnitGroup();
		GroupForEachUnit(tmpGroup, attacker,
			if (UnitIsValid(attacker) && attacker->acquiredTarget() == u){
				UnitAttackers->add(attacker);
			}
		);
		GroupDestroy(tmpGroup);
		return UnitAttackers;
	}

	//����debug distribution����
	void DistributionMapContentPrint(distributiontype *pDistMap, const char *identStr, bool clearScreenBefore = false){
#ifndef _VMP
		if (pDistMap->empty()) {
			OutputScreen(10,"pDistMap of |cffffcc00%s|r : Empty", identStr);
			return;
		}
		if (clearScreenBefore) Jass::ClearTextMessages();
		OutputScreen(10,"pDistMap of |cffffcc00%s|r :", identStr);
		for (distributiontype::const_iterator it = pDistMap->begin();
			it != pDistMap->end(); ++it)	
		{
			OutputScreen(10, "  [%.3f] = %.3f", it->first, it->second);
		}
#endif
	}

	//����debug damagemap����
	void DamageMapContentPrint(damagemaptype *pDamageMap, const char *identStr, bool clearScreenBefore = false){
#ifndef _VMP
		if (pDamageMap->empty()) {
			OutputScreen(10,"DamageMap of |cffffcc00%s|r : Empty", identStr);
			return;
		}
		if (clearScreenBefore) Jass::ClearTextMessages();
		OutputScreen(10,"DamageMap of |cffffcc00%s|r :", identStr);
		for (damagemaptype::const_iterator it = pDamageMap->begin();
			it != pDamageMap->end(); ++it)	
		{
			OutputScreen(10, "  [%.3f] = %s", it->first, it->second.toString().c_str());
		}
#endif
	}

	//��damagemapд������ָ��Ŀ�굥λ���ӵ��˺�
	void DamageMapAddAllMissile(damagemaptype *pDamageMap, Unit *targetUnit){
		if (!targetUnit)
			return;
		const MissileInfo* info; float timeNeed;
		while(NULL != (info = MissileManager_TargetUnitMissileFetch(targetUnit))) {
			timeNeed = info->timeNeeded();//Ϊ�ӵ��������Ŀ�������ʱ��
			if (timeNeed > -1.f){//˵����Ч
				(pDamageMap->operator[](Time() + timeNeed)) 
					&= DnDAtkDist( //�����ӵ�ӵ��ȷ���˺�ֵ���������һ��ֻ�л���ֵ���������ӵĹ����ֲ�
						info->attackTotal() //������ͨ�˺� + �����˺�
						* targetUnit->damageFactorAsTarget(info->attackType()) //Ϊ�ӵ������������Ͷ�Ŀ�굥λ���˺�����
					);
			}
		}
	}

	//��damagemapд��Ŀ�굥λ���й����ߵĹ����˺�
	void DamageMapAddAllDamages(damagemaptype *pDamageMap, Unit *targetUnit, bool countLastHitter) {
		if (!targetUnit)
			return;
		UnitGroup *groupAttackers = UnitGetAttackers(targetUnit, 1400.f);
		if (!countLastHitter) groupAttackers->remove(&LastHitUnits);
		float time, dmgptRemain, coolRemain, cd;
		float timePredictMaximum = Time() + PREDICT_TIME_MAX;
		int SinglePredictCount;
		for (UnitGroup::iterator iter = groupAttackers->begin(); iter != groupAttackers->end(); ++iter){
			Unit *attacker = *iter;//TODO �жϵ�λ��̷�Χ��״̬
			DnDAtkDist u_dmg = attacker->damage(attacker->weaponTo(targetUnit), targetUnit);
			time = -1.f;
			if ((dmgptRemain = attacker->damagePointRemain()) > 0) {
				time = Time() + dmgptRemain + attacker->attackTime(attacker->weaponTo(targetUnit), targetUnit, false, false);
			}
			else if ((coolRemain = attacker->cooldownRemain()) > 0) {
				time = Time() + coolRemain + attacker->attackTime(attacker->weaponTo(targetUnit), targetUnit, true, false);
			}
			else {//TODO ����Ҫ����acquire����δ���������
				continue;
			}
			
			cd = attacker->cooldown(attacker->weaponTo(targetUnit));
			if (time <= 0 || cd <= 0) continue;
			SinglePredictCount = 0;
			while (time < timePredictMaximum && SinglePredictCount < PREDICT_COUNT_MAX_SINGLE){
				(pDamageMap->operator[](time)) &= u_dmg;
				time += cd; ++SinglePredictCount;
			}
		}
	}

	static uint32_t FunctionResultMax;//debug
	static damagemaptype damageMap;	//���ڴ������а�ʱ�����������˺��ֲ�
	void UpdateUnitTiming (Unit *targetUnit, bool forceIgnoreCooldown = false) {
		if (!targetUnit) return;

		damageMap.clear();

		//���ͼ�⣺�������µ�Ŀ�굥λ(targetUnit�����¼��Ŀ��)��Ӣ�ۣ�������
		if (targetUnit->testFlag(UnitFilter::HERO)) return;

		//��Χ����������ڲ����ߣ���Ŀ�겻���κβ����߸�����PREDICT_MIN_DISTANCE�����ڣ�ֱ�ӽ���
		if (!UnitNearAnyLastHitter(targetUnit, PREDICT_MIN_DISTANCE)) return;

		//Ƶ�ʣ������Ƶ���ĸ��£����Ŀ������ϴθ���ʱ�䲻��PREDICT_COOLDOWN��ֱ�ӽ��������ָ������forceIgnoreCooldownǿ�Ƹ���������
		if (!forceIgnoreCooldown && (Time() < UpdateCooldownMap[targetUnit] + PREDICT_COOLDOWN)) return;

		//ͳ����Ԥ��Ŀ��δ�������˺��������ӵ�Ϊȷ��ʱ�����ȷ���˺�ֵ��������������ΪԤ��ʱ������˺��ֲ�
		DamageMapAddAllMissile(&damageMap, targetUnit);

		//��������
		DamageMapAddAllDamages(&damageMap, targetUnit, false);//������������

		//����
		//DamageMapContentPrint(&damageMap, targetUnit->debugName(), false);

		//���������ʱ�䳬��exact���Ͱ��˺���Ϊƽ��ֵ���Լ��ټ�����
		int TotalPredictCount = 0;
		float timePredictExactMaximum = Time() + PREDICT_EXACT_TIME_MAX;
		damagemaptype damageMapTemp;
		for (damagemaptype::const_iterator iterOuter = damageMap.begin(); 
			iterOuter != damageMap.end(); 
			++iterOuter, ++TotalPredictCount) 
		{
			float time = iterOuter->first;
			if ( 
					TotalPredictCount >= PREDICT_EXACT_COUNT_MAX
				||	time > timePredictExactMaximum
			){
				damageMapTemp[time]	= DnDAtkDist(iterOuter->second.expectValue());
			}
			else{
				damageMapTemp[time] = iterOuter->second;
			}
		}
		damageMap.swap(damageMapTemp);

		std::map<Unit *, distributiontype> lasthitMap;
		float deathRate = 0;
		float deathCDF = 0;
		float currentHP;
		damagemaptype::const_iterator iterOuter = damageMap.begin(); 
		int iterOuterLoopCount = 0;
		DnDAtkDist deathRateDist;
		while(
			(iterOuter != damageMap.end()) && 
			(iterOuterLoopCount < PREDICT_COUNT_MAX)
		){
			//OutputScreen(10, "iterOuterLoopCount = %d, iterOuter->first = %.3f", iterOuterLoopCount, iterOuter->first);
			deathRateDist = DnDAtkDist();
			//������1..i���˺�
			damagemaptype::const_iterator 
				iterInner = iterOuter;
			while(true){
				//OutputScreen(10, "iterInner");
				DnDAtkDist d = iterInner->second;
				deathRateDist &= d;
				if (iterInner == damageMap.begin()) break;
				--iterInner;
			}
			
			currentHP = targetUnit->life();
			currentHP -= 0.405f;
			currentHP += max(0, ((iterOuter->first)-Time())*(targetUnit->regenSpeedLife()));
			deathRate = deathRateDist.chanceGreaterEqual(currentHP, false);
			deathRate = max(deathRate, 0);
			deathRate = min(deathRate, 1);

			//OutputScreen(10, "[%s] deathRate at %.3f is %.2f", targetUnit->debugName(), iterOuter->first, deathRate);

			deathCDF += (1 - deathCDF) * deathRate;//

			//OutputScreen(10, "[%s] deathCDF at %.3f is %.2f", targetUnit->debugName(), iterOuter->first, deathCDF);

			DnDAtkDist lastHitRateDist;
			float lastHitRate;
			for (uint32_t i = 0; i < LastHitUnits.size(); i++){ //�����в����߼���ôι������ӵ�λ�������Ľ��
				Unit *u = LastHitUnits.getUnit(i);
				lastHitRateDist = deathRateDist;
				lastHitRateDist &= u->damage(0, targetUnit);

				lastHitRate = lastHitRateDist.chanceGreaterEqual(currentHP, false);
				lastHitRate *= (1 - deathCDF);//(1 - deathRate);
				lastHitRate = max(lastHitRate, 0);
				lastHitRate = min(lastHitRate, 1);

				//OutputScreen(10, "[%s] lastHitRate at %.3f is %.2f", targetUnit->debugName(), iterOuter->first, lastHitRate);

				lasthitMap[u][iterOuter->first] = lastHitRate;//iterOuter->first������
			}

			//if (deathRate >= 1.0f){
			if (deathCDF >= 1.0f){
				break;
			}

			++iterOuter; ++iterOuterLoopCount;
		}
		
		//�ѽ��ת��Ϊ��������д��Unit
		std::pair<float, float> chancePair;
		GroupForEachUnit(&LastHitUnits, u,
			//OutputScreen(10, "Before transform:");
			//DistributionMapContentPrint(&(lasthitMap[u]), targetUnit->debugName());
			u->lasthitRate[targetUnit].setValueFromDiscrete(&(lasthitMap[u]), 0);//TODO
			//OutputScreen(10, "Before conv:");
			//u->lasthitRate[targetUnit].print();
			u->lasthitRate[targetUnit].convolution(LatencyGetNegDist());
			//OutputScreen(10, "AFTER conv:");
			//u->lasthitRate[targetUnit].print();
			//LatencyDistPrint();//debug

				//chancePair = u->lasthitRate[targetUnit].maxima();
				//OutputScreen(10, "maxima of %s is (%.3f, %.3f)", targetUnit->debugName(), chancePair.first, chancePair.second);

			//FunctionResultMax = max(FunctionResultMax, u->lasthitRate[targetUnit].size());
			//DefaultDebugPanel->set("functionSize","%d (max %d)",
			//	u->lasthitRate[targetUnit].size(), 
			//	FunctionResultMax
			//);

			//OutputScreen(10, "transfer to function");
			//u->lasthitRate[targetUnit].print();
		);

		//������ɣ����¡��ϴθ���ʱ�䡱
		UpdateCooldownMap[targetUnit] = Time();
	}

	void onTimerExpire (Timer *tm){
		if (STRATEGY_ERROR == LastHitStrategy()) return;

		//DefaultDebugPanel->set("time", "|cffffcc00%.3f|r", Time());

		Unit *lasthitter;
		float timeNeedTotal = 0;
		std::pair<float, float> chancePair;
		float chance;
		float lasthitterCurrentCooldown;

		bool lasthitterIsRanged = false;
		bool missionFound = false;
		Function lasthitFunctionCopy;

		for (UnitGroup::iterator iter = LastHitUnits.begin(); iter != LastHitUnits.end(); ++iter){
			lasthitter = *iter;

			//���������û�б��������ѡ�У�ֱ����������
			if (!lasthitter->isSelectedLocal()){
				LastHitUnits.remove(lasthitter);
				LastHitUnitEndProc(lasthitter);
				return;
			}

			if (lasthitter->lasthitCurrentTarget){
				if (UnitIsValid(lasthitter->lasthitCurrentTarget)) continue;//�����ǰ��������Թ�
				else lasthitter->lasthitCurrentTarget = NULL;
			}
			missionFound = false;
			lasthitterIsRanged = lasthitter->testFlag(UnitFilter::RANGED_ATTACKER);

			std::map<Unit*, Function> * pLasthitRateMap = &(lasthitter->lasthitRate);
			//�õ���λ��Χ�ĵ�λ
			UnitGroup *unitsNearby = 
				GroupUnits(
				NULL,
				NULL,
				PlayerGroupAll(),
				&RegionCircle(lasthitter->position(), lasthitter->range(0)+
					(lasthitterIsRanged ? 100.f : 150.f)
				),//buffer
				UnitFilter(
					NULL,
					UnitFilter::HIDDEN | UnitFilter::DEAD | UnitFilter::INVULNERABLE
				)
			);
			//DefaultDebugPanel->set("Nearby Unit Count", "%d", unitsNearby->size());

			lasthitterCurrentCooldown = lasthitter->cooldownRemain();

			//��ÿ��Ŀ��Ĳ���function��ȥ����ʱ��
			if (unitsNearby->size()){
				GroupForEachUnit(unitsNearby, target,
					if (LastHitStrategy() == STRATEGY_ENEMYONLY  &&  !target->isEnemyToLocalPlayer() ) continue;
					if (LastHitStrategy() == STRATEGY_ALLYONLY  &&  !target->isAllyToLocalPlayer() ) continue;
					if (pLasthitRateMap->count(target)){
						lasthitFunctionCopy = pLasthitRateMap->operator[](target);
						timeNeedTotal = lasthitterIsRanged ?
							max(lasthitterCurrentCooldown, 0.5f)
							+ lasthitter->attackTime(lasthitter->weaponTo(target), target, true, false)//Զ����safe delay �Ӳ�������������ʱ�� = �ӳ� + 0.5 + ǰҡ + �ӵ�ʱ��
							:
							max(lasthitterCurrentCooldown, lasthitter->turnTimeToUnit(target))//TODO �뷴Ӧ�ӳٵļ��㣿
							+
							lasthitter->attackTime(lasthitter->weaponTo(target), target, true, false);//��ս����ͨ������Ҫת��

						//lasthitFunctionCopy.smooth(uint32_t((0.03f/RASTER_UNIT-1.f)/2.f));//ģ��0.03�������ȶ���

						if (!lasthitterIsRanged){
							uint32_t smoothRadius = uint32_t((ReactionDelay/RASTER_UNIT-1.f)/2.f);
							lasthitFunctionCopy.smooth(smoothRadius);
							lasthitFunctionCopy.shift((-ReactionDelay/2.f));
						}

						chancePair = lasthitFunctionCopy.maxima();//TODO ��ֵ(����)��5%������?
						chance = chancePair.second;
						if (chance > 0.05f){//TODO ��С������ʣ�������/���û�
							if (
								//Time() < chancePair.first &&//TODO �Ƿ������壿
								Time() + timeNeedTotal >= chancePair.first //��ǰʱ���Ѿ���������ʱ��
							){
								//����
								//Jass::ClearTextMessages();
								//OutputScreen(10, "[%.3f] lasthit unit %s(%d) chance = |cffffcc00%.2f|r", Time(), target->name(), target->handleId(), chance);
								//lasthitFunctionCopy.print();

								lasthitter->lasthitCurrentTarget = target;
								if (lasthitterIsRanged){
									SafeDelayAttack(lasthitter, target);
								}
								else{
									NormalAttack(lasthitter, target);
								}
								missionFound = true;
							}
						}
					}
					if (!missionFound) {//ֱ���ж�Ѫ��
						if (//TODO ���ֵ���
							target != lasthitter
							&&	lasthitter->damage(lasthitter->weaponTo(target), target).minValue() >= target->life()	)		//ֱ��ɱ��Ŀ��
						{
							//����
							//OutputScreen(10, "lasthit low HP unit %s(%d)", target->name(), target->handleId() );
							lasthitter->lasthitCurrentTarget = target;
							NormalAttack(lasthitter, target);
							missionFound = true;
						}
					}
					if (missionFound) break;//TODO ���ȼ���ϲ��
				);
				if (!missionFound) {
					if (!UnitIsStandStillAndCool(lasthitter)){
						StandStillAndCool(lasthitter);
					}
				}
			}

			GroupDestroy(unitsNearby);
		}

		//debug
		//DefaultDebugPanel->set("Last Hit | LastHitter count", "%d", LastHitUnits.size());		
	}

	//��λ���벹��ģʽʱ����Ϊ
	void LastHitUnitInitProc(Unit *u){
		if (!u)	return;

		//��¼��ǰ����
		//ֻ��¼smart
		if (u->currentOrder() == 0xD0003)
		{
			Unit *targetUnit = u->currentTargetUnit();

			Action action;
			action.actionId = ACTION_SMART;
			action.actionType = Target;//targetUnit ? Target : TargetPoint;
			action.actionFlag = None;
			action.targetPoint = u->currentTargetPoint();
			action.targetUnit = targetUnit;
			UnitLastActionMap[u] = action;
		}

		UnitGroup *unitsNearby = //�õ���λ��Χ�ĵ�λ
			GroupUnits(
			NULL,
			NULL,
			PlayerGroupAll(),
			&RegionCircle(u->position(), PREDICT_MIN_DISTANCE),
			UnitFilter(
				NULL,
				UnitFilter::HIDDEN | UnitFilter::DEAD | UnitFilter::INVULNERABLE
			)
		);
		GroupForEachUnit(unitsNearby, target,
			//OutputScreen(10, "init update for unit %s (%d)", target->name(), target->handleId() );
			UpdateUnitTiming(target, true);
		);
		GroupDestroy(unitsNearby);
	}

	//��λ�˳�����ģʽʱ����Ϊ
	void LastHitUnitEndProc(Unit *u){
		if (!u) return;
		//TODO ��Ϊ�ռ�����ģʽʱ����Ҳ�����������Ҫ�Ҽ��ƶ����������ͷŲ���
		//����Ϊ�˳�����ʱ��stop
		/*u->sendAction(
			ACTION_STOP, 
			TargetNone, 
			None, 
			NULL, 
			POINT_NONE, 
			NULL, 
			true
		);*/

		if (UnitLastActionMap.count(u))
		{
			Action action = UnitLastActionMap[u];
			u->sendAction(
				action.actionId, 
				action.actionType,
				action.actionFlag, 
				NULL,//action.usingItem,
				action.targetPoint,
				action.targetUnit, 
				false
			);
			UnitLastActionMap.erase(u);
		}
		else
		{
			u->sendAction(
				ACTION_STOP, 
				TargetNone, 
				None, 
				NULL, 
				POINT_NONE, 
				NULL, 
				true
			);
		}
		
	}

	void onAttackReleased_Waited (Timer *tm) {
		Unit *target = *(tm->data<Unit *>());
		if (target)
			UpdateUnitTiming(target);
	}

	void onAttackReleased(const Event *evt) {
		UnitAttackReleasedEventData* data = evt->data<UnitAttackReleasedEventData>();	
		Unit *attacker = GetUnit(data->attacker);
		if (attacker) {
			Unit *target = attacker->acquiredTarget();
			if (target) {
				if ( true
				//&&	!LastHitUnits.has(attacker)	//��λ���ǲ�����
				){				
					Timer *tm = GetTimer(0, onAttackReleased_Waited, false);
					tm->setData<Unit *>(&target);
					tm->start();
				}

				//������Ŀ��
				if (attacker->lasthitCurrentTarget){//TODO ������ǵ�1�����Ϲ������ܲ���ȷ
					attacker->lasthitCurrentTarget = NULL;
					//OutputScreen(10, "last hit target cleanup");
				}
			}
		}
		
	}

	void onAttackMissed_Waited(Timer *tm) {
		Unit *target = *(tm->data<Unit *>());
		//OutputScreen(10, "[%.3f] missed , update!", Time());
		if (target)
			UpdateUnitTiming(target, true);//ǿ�Ƹ���
	}

	void onAttackMissed(const Event *evt) {
		UnitAttackMissedEventData* data = evt->data<UnitAttackMissedEventData>();
		Unit *attacker = GetUnit(data->attacker);
		if (attacker) {
			Unit *target = attacker->acquiredTarget();
			if (target) {
				if ( !LastHitUnits.has(attacker)	//��λ���ǲ�����
				){
					Timer *tm = GetTimer(0, onAttackReleased_Waited, false);
					tm->setData<Unit *>(&target);
					tm->start();
				}
			}
		}
	}

	void onUnitDamaged (const Event *evt) {
		UnitDamagedEventData *data = evt->data<UnitDamagedEventData>();
		Unit *targetUnit = GetUnit(data->target);
		Unit *sourceUnit = GetUnit(data->source);

		if (!(targetUnit && sourceUnit))
			return;

		float damage = data->damage;
		/*
		if (sourceUnit->testFlag(UnitFilter::HERO)){
			OutputScreen(10, "|cffffcc00[%.3f]|r Damaged : %.3f damage dealt", Time(), damage);
		}
		else{
			OutputScreen(10, "[%.3f] Damaged : %.3f damage dealt", Time(), damage);
		}
		*/
	}

	void onUnitAcquireStart (const Event *evt){
		UnitAcquireEventData *data = evt->data<UnitAcquireEventData>();
		//OutputScreen(10, "|cffffcc00[%.3f]|r %d starts to acquire %d", Time(), data->eventUnit, data->target);
		Unit *eventUnit = GetUnit(data->eventUnit);
		Unit *targetUnit = GetUnit(data->target);

		if (!(eventUnit && targetUnit))
			return;
		//TODO
	}

	void onUnitAcquireReady (const Event *evt){
		UnitAcquireEventData *data = evt->data<UnitAcquireEventData>();
		//OutputScreen(10, "|cffffcc00[%.3f]|r %d is ready acquire %d", Time(), data->eventUnit, data->target);
		Unit *eventUnit = GetUnit(data->eventUnit);
		Unit *targetUnit = GetUnit(data->target);

		if (!(eventUnit && targetUnit))
			return;
		//TODO
	}

	void onUnitAcquireStop (const Event *evt){
		UnitAcquireEventData *data = evt->data<UnitAcquireEventData>();
		//OutputScreen(10, "|cffffcc00[%.3f]|r %d stops acquire %d", Time(), data->eventUnit, data->target);
		Unit *eventUnit = GetUnit(data->eventUnit);
		Unit *targetUnit = GetUnit(data->target);
		
		if (!(eventUnit && targetUnit))
			return;
		
		UpdateUnitTiming(targetUnit, true);//TODO !!!
	}

	void onUnitDeath (const Event *evt) {
		Unit *killer = GetUnit(Jass::GetKillingUnit());
		Unit *deadUnit = GetUnit(Jass::GetDyingUnit());

		if (!(killer && deadUnit))
			return;

		Unit *acquiredUnit = deadUnit->acquiredTarget();
		if (acquiredUnit){
			UpdateUnitTiming(acquiredUnit, true);
		}
		GroupForEachUnit(&LastHitUnits, lasthitter,
			if (deadUnit == lasthitter->lasthitCurrentTarget) lasthitter->lasthitCurrentTarget = NULL;
		);
	}


	static bool				Enabled;
	void SetLastHitActivate (const Event *evt) {
		
		VMProtectBeginVirtualization("LastHitActivate");

		if (!Enabled) return;

		KeyboardEventData *data = evt->data<KeyboardEventData>();
		if ( data->code == GetHotkey() ) {//�жϰ����ǲ�����
			data->discard();	 DiscardCurrentEvent();
			UnitGroup *g_currentSelected = GroupUnitsOfPlayerSelected(PlayerLocal(), false);//������ҵ�ǰ��ѡ��λ
			bool active = (evt->id() == EVENT_KEY_DOWN);//����Ϊ���벹��������Ϊ�˳�����
			uint32_t countChanged = 0, countValid = 0;
			GroupForEachUnit(g_currentSelected, enumUnit, 
				if(	enumUnit->testFlag(UnitFilter::CONTROLLABLE, PlayerLocal())	) {//��λ����Ϊ������ҿɿ���
					if (enumUnit->lasthitActive != active){//���ò�������
						enumUnit->lasthitActive = active; ++countChanged;
						active ? LastHitUnits.add(enumUnit) : LastHitUnits.remove(enumUnit);
						if (active) LastHitUnitInitProc(enumUnit);
						else		LastHitUnitEndProc(enumUnit);
					} ++countValid;
				}
			);
			if (countChanged)				SoundPlay("InterfaceClick", NULL, 0);
			else if (!countValid && active)	SoundPlay("InterfaceError", NULL, 0);
			GroupDestroy(g_currentSelected);
		}
		
		VMProtectEnd();
	}

	static CheckBox*		CbEnabled;
	static Label*			LbEnabled;

	static CheckBox*		CbEnableHit;
	static Label*			LbEnableHit;

	static CheckBox*		CbEnableDeny;
	static Label*			LbEnableDeny;

	static Button*			BtnHotkey;

	void Dependency(CheckBox* cb, bool flag) {
		CbEnableHit->activate(flag);
		LbEnableHit->activate(flag);
		CbEnableDeny->activate(flag);
		LbEnableDeny->activate(flag);
		BtnHotkey->enable(flag);
	}

	void UpdateBtnText(Button *btn) {
		std::string str;
		str+=StringManager::GetString(STR::LASTHIT_BTNSETHOTKEY);
		str+=": |cffffcc00";
		str+=KEYCODE::
			//ToString(GetHotkey());
			getStr(GetHotkey());
		str+="|r";
		btn->setText(str.c_str());
	}

	static bool EditingHotkey;
	static Button* EditingHotkeyBtn;
	void BtnCallback(Button *btn) {
		btn->setText(StringManager::GetString(STR::LASTHIT_BTNSETHOTKEY_NOTE));
		EditingHotkey = true;
		EditingHotkeyBtn = btn;
	}

	void DetectHotkey(const Event* evt){
		KeyboardEventData *data = evt->data<KeyboardEventData>();
		if (EditingHotkey){
			ProfileSetInt("LastHit", "Hotkey", data->code);
			UpdateBtnText(EditingHotkeyBtn);
			EditingHotkey = false;
			EditingHotkeyBtn = NULL;
			data->discard();	 DiscardCurrentEvent();
			SoundPlay("GlueScreenClick", NULL, 0);
		}
	}

	void onActionSent(const Event *evt)
	{
		ActionEventData *data = evt->data<ActionEventData>();
		if ( !data->byProgramm )		//��Ҳ���
		{
			//��õ�ǰ������λ
			UnitGroup *g_currentSelected = GroupUnitsOfPlayerSelected(PlayerLocal(), false);//������ҵ�ǰ��ѡ��λ
			Unit *u;
			for (UnitGroup::iterator iter = g_currentSelected->begin(); 
				iter != g_currentSelected->end(); ++iter)
			{
				u = *iter;
				if (u)
				{
					if (LastHitUnits.has(u))
					{
						Action action;
						action.actionId = data->id;
						action.actionType = data->type;
						action.actionFlag = data->flag;
						action.usingItem = GetItem(data->usingItem);
						action.targetPoint = Point(data->x, data->y);
						action.targetUnit = GetUnit(data->target);
						UnitLastActionMap[u] = action;
						data->discard(); DiscardCurrentEvent();
					}
				}
			}


			GroupDestroy(g_currentSelected);
		}
	}

	void CreateMenuContent(){

		UISimpleFrame* Panel = DefaultOptionMenuGet()->category(StringManager::GetString(STR::LASTHIT_CATEGORYNAME), NULL); //TODO fix tooltip

		CbEnabled = new CheckBox(Panel);
		CbEnabled->bindProfile("LastHit", "Enabled", true);//Ĭ�Ͽ���
		CbEnabled->bindVariable(&Enabled);
		CbEnabled->bindCallback(Dependency);
		CbEnabled->setRelativePosition( POS_UL,	Panel, POS_UL, 0.03f, -0.03f );
		LbEnabled = new Label( Panel, StringManager::GetString(STR::LASTHIT_ENABLE), 0.013f );
		LbEnabled->setRelativePosition(	POS_L, CbEnabled, POS_R, 0.01f, 0 );

		CbEnableHit = new CheckBox( Panel );
		CbEnableHit->bindProfile( "LastHit", "EnableHit", true );
		CbEnableHit->bindVariable( &EnableHit );
		CbEnableHit->setRelativePosition( POS_UL, Panel, POS_UL, 0.03f, -0.057f );
		LbEnableHit = new Label( Panel, StringManager::GetString(STR::LASTHIT_ENABLEHIT), 0.013f );
		LbEnableHit->setRelativePosition( POS_L, CbEnableHit, POS_R, 0.01f, 0 );

		CbEnableDeny = new CheckBox(Panel);
		CbEnableDeny->bindProfile("LastHit", "EnableDeny", true);
		CbEnableDeny->bindVariable(&EnableDeny);
		CbEnableDeny->setRelativePosition(POS_UL,Panel,POS_UL,0.03f, -0.084f);
		LbEnableDeny = new Label(Panel, StringManager::GetString(STR::LASTHIT_ENABLEDENY), 0.013f);
		LbEnableDeny->setRelativePosition(POS_L, CbEnableDeny, POS_R, 0.01f, 0);

		BtnHotkey = new Button(
			Panel,
			0.135f,
			0.035f,
			UISimpleButton::MOUSEBUTTON_LEFT,
			UISimpleButton::STATE_ENABLED,
			BtnCallback
		);
		UpdateBtnText(BtnHotkey);
		BtnHotkey->setRelativePosition(
			POS_UL, 
			Panel, 
			POS_UL,
			0.03f + Panel->width()/2, -0.03f);

		MainDispatcher()->listen(EVENT_KEY_DOWN, DetectHotkey);
		EditingHotkeyBtn = NULL;
		EditingHotkey  = false;

		Dependency(CbEnabled, CbEnabled->isChecked());
	}

	void Init() {

		VMProtectBeginVirtualization("LastHitInit");

		//UI
		CreateMenuContent();

		//LastHitStrategy = ProfileFetchInt("LastHit", "Strategy", STRATEGY_DONTMIND);
		ReactionDelay = GameDataProfileGetFloat("Misc", "ReactionDelay", 0);
		

		MainDispatcher()->listen(EVENT_UNIT_RECEIVE_DAMAGE, onUnitDamaged);//����debug
		MainDispatcher()->listen(EVENT_UNIT_ATTACK_RELEASED, onAttackReleased);//���湥������������Ŀ�굥λ����
		MainDispatcher()->listen(EVENT_UNIT_ATTACK_MISS, onAttackMissed);//���湥������miss

		//��ʼע�⵽һ��Ŀ�� --> ��Ӧ�ӳ� + [ת�� + �ƶ�] + �����¼���ʼ��������ʱ�䣬����Ŀ�굥λ����
		//���ע�⵽һ��Ŀ�� --> �����¼���ʼ��������ʱ�䣬����Ŀ�굥λ���� TODO �Ƿ���ȷ��
		//ֹͣע�⵽һ��Ŀ�� --> ������ȥ���κζ�ԭĿ��Ĺ���������ԭĿ�굥λ����
		MainDispatcher()->listen(EVENT_UNIT_ACQUIRE_START, onUnitAcquireStart);
		MainDispatcher()->listen(EVENT_UNIT_ACQUIRE_READY, onUnitAcquireReady);
		MainDispatcher()->listen(EVENT_UNIT_ACQUIRE_STOP,  onUnitAcquireStop);

		//TODO ��λ�����¼������ڷ���bug������
		//TODO ʵ����Ӧ��Ϊ����λ��Ϊ���ܱ������߹������¼�
		MainDispatcher()->listen(Jass::EVENT_PLAYER_UNIT_DEATH, onUnitDeath);

		//����ѭ�������Ƿ񲹵�
		GetTimer(0.03, onTimerExpire, true)->start();

		//����/�˳�����ģʽ���жϰ���״̬
		MainDispatcher()->listen(EVENT_KEY_DOWN, SetLastHitActivate);
		MainDispatcher()->listen(EVENT_KEY_UP, SetLastHitActivate);

		//�����Ҳ�������¼���������һ�β���
		MainDispatcher()->listen(EVENT_LOCAL_ACTION, onActionSent);

		VMProtectEnd();
	}

	void Cleanup() {
		VMProtectBeginVirtualization("LastHitClean");

		LastHitUnits.clear();
		UpdateCooldownMap.clear();
		if (UnitAttackers) GroupDestroy(UnitAttackers); UnitAttackers = NULL;

		UnitLastActionMap.clear();

		VMProtectEnd();
	}
}