/*
���Ź���OptAttack��

�ռ��ҷ���λ��ʵ�ʿɿ��ƣ����ų�����Ӧ�Զ��ĵ�λ��������ֿصĵ�λ��
�ռ��з���λ��

ÿ���з���λ�е�λԤ��ʣ��hp����ʼ����ǰhp - Ԥ�����Ա��ֿص�λ���˺���

�ų��������û�е��˵��ҷ���λ���ų����ҷ������û�еĵ��˵�λ��
���ҷ���λ����"������ĵ��˵ľ���"���򡣣����ܹ����Ļ�������=inf��

����ÿ���ҷ���λ��������:

��ѡĿ�굥λ��ʹ��max : ��λ��ֵ / �õ�λɱ��Ŀ�굥λ������� (��Ԥ��ʣ��hp����)
-- ��δ���Χ�˺���
����õ�λ����Ŀ�굥λ, Ŀ�굥λԤ��ʣ��hp -= Ԥ���˺�ֵ�����Ԥ��ʣ��hp < 0���ų����顣
-- ��δ���Χ�˺�����ɵ�Ԥ���˺�

������õ����ɸ����飬ÿ���������е�λ����һ��Ŀ�ꡣ
�Ż�1����������������λ������������ڲ�ͬ���飬����Ŀ������˺������
���ҽ����������ʹ�ò��������٣��򽻻����顣���������й���Ŀ���жϲ������Ƿ���٣�
�Ż�2���ų���ÿ�������е�ǰ����Ŀ���Ѿ�ΪӦ����Ŀ��ĵ�λ��

��������ÿ�����飬����Ŀ�굥λ

���ж��Զ�/�ֶ�����
TODO Ƿȱ��abilityflag�ѱ��ط�����Ӧ����λ����ķ���
- �����������������ı�״̬
- ���ط����¼�����flag concurrent�����ı�״̬
- ���ط����¼�����flag queue���ݲ��ı�״̬

- ���ط����¼�����λA����/P���棬�Զ�
- ���ط����¼�����λ��stop/hold���Զ�
- ���ط����¼����������У��ֶ�
- ѭ���жϵ�ǰָ���ǰ�����Ϊ0 (idle�¼�)���Զ����Զ�A���˵�ʱ��acquireΪĿ�꣬������Ϊ0��
- ѭ���жϵ�ǰָ���ǰ�����ΪA����/P���棬�Զ�
*/


#include "stdafx.h"
#include "DreamWar3Main.h"

namespace OptAttack {

	typedef std::map<Unit*, bool> AutoStateMapType;
	static AutoStateMapType StateMap;

	void debug(Unit *u){
#ifdef _DEBUG
		u->debugPanel->set("auto", "%s", StateMap[u]? "|cff00ff00Y|r":"|cffff0000N|r");
#endif
	}

	void AutoStateSet(Unit *u, bool flag){
		StateMap[u] = flag;
		debug(u);
	}

	void onActionSent(const Event* evt){
		ActionEventData *data = evt->data<ActionEventData>();

		if (data->byProgramm) return;//�����������������ı�״̬
		if (data->flag & Concurrent) return;//���ط����¼�����flag concurrent�����ı�״̬
		if (data->flag & Queued) return;//���ط����¼�����flag queue���ݲ��ı�״̬

		//���A�ذ��P�ذ壬�����п��Ƶ�λΪ�Զ�
		if ((data->id == ACTION_ATTACK || data->id == ACTION_PATROL) && 
			data->target == NULL
		){
			//���Ƶĵ�λ��ͨ�����Ϊȫ����ѡ��������flagΪ��������
			UnitGroup* g = GroupUnitsOfPlayerSelected( PlayerLocal(), (data->flag & Subgroup)!=0 );
			GroupForEachUnit(g, u, AutoStateSet(u, true););
			GroupDestroy(g);
		}
		else if (
			(data->id == ACTION_STOP || data->id == ACTION_HOLD)
		){
			//���Ƶĵ�λ��ͨ�����Ϊȫ����ѡ��������flagΪ��������
			UnitGroup* g = GroupUnitsOfPlayerSelected( PlayerLocal(), (data->flag & Subgroup)!=0 );
			GroupForEachUnit(g, u, AutoStateSet(u, true););
			GroupDestroy(g);
		}
		else {
			UnitGroup* g = GroupUnitsOfPlayerSelected( PlayerLocal(), (data->flag & Subgroup)!=0 );
			GroupForEachUnit(g, u, AutoStateSet(u, false););
			GroupDestroy(g);
		}
	}

	void Init() {
		MainDispatcher()->listen(EVENT_LOCAL_ACTION, onActionSent);
	}

	void Cleanup() {
		StateMap.clear();
	}
}