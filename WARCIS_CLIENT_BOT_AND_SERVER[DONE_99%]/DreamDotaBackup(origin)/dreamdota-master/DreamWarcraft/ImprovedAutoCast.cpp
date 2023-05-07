//ctrl + ���ؼ����Զ� = ��������ͬ���͵�λ���Լ�δ����λ�Զ�����

#include "stdafx.h"
#include "DreamWar3Main.h"


namespace ImprovedAutoCast{

	void onActionSent(const Event* evt) {
		ActionEventData *data = evt->data<ActionEventData>();
		if (	!data->byProgramm && 
				(data->flag & AutoCast) &&
				KeyIsDown(KEYCODE::KEY_CONTROL) &&
				!KeyIsDown(KEYCODE::KEY_SHIFT) &&
				!KeyIsDown(KEYCODE::KEY_ALT)
		){
			uint32_t actionId = data->id;
			UnitGroup *group = GroupUnitsOfPlayerSelected(PlayerLocal(), true);
			if (group->size() > 0){
				ProfileSetInt("AutoCastForceState", group->getUnit(0)->typeIdChar(), actionId);
			}
			GroupDestroy(group);
		}
	}

	void onUnitCreated(const Event* evt) {
		UnitCreationEventData *data = evt->data<UnitCreationEventData>();
		Unit *u = GetUnit(data->createdUnit);
		uint32_t actionId = ProfileGetInt("AutoCastForceState", u->typeIdChar(), 0);
		if (actionId){
			//TODO: Ӧ����Ϊ������������ɺ���/�ر��Զ�������
			u->sendAction(
				actionId,
				TargetNone,
				Concurrent | AutoCast,
				NULL,
				POINT_NONE,
				NULL,
				false
			);
		}
	}

	void Init () {
		//���action
		MainDispatcher()->listen(EVENT_LOCAL_ACTION, onActionSent);
	
		//��ⵥλ������
		MainDispatcher()->listen(EVENT_UNIT_CREATED, onUnitCreated);
	}

}