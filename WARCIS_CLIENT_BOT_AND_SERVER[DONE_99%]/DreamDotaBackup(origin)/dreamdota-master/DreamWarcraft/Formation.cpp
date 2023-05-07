/*
- ��ѡ��λ����½����ͬ���Ͳ����ƶ��ٶ�һ�£��ƶ�ʱ������ + Ⱥ��
- �����ǰѡ�����пվ�������û��Ӣ�������½������ȡ��Ⱥ��ȡ������

- ��������alt + �Ҽ���������
- ���������������ͣ��ۼ����ͣ���ɢ���ͣ�
*/

#include "stdafx.h"
#include "DreamWar3Main.h"

namespace Formation {

	void onLocalActionSent (const Event *evt) {
		ActionEventData *data = evt->data<ActionEventData>();
		war3::PacketActionPoint *packet = (war3::PacketActionPoint *)(data->packetSender->packet);
		UnitGroup *group = GroupUnitsOfPlayerSelected(PlayerLocal(), false);

		//½������
		if (	!data->byProgramm			//�ǳ�����������
			&&	data->id == ACTION_SMART		//����Ϊ�Ҽ�
			&&	data->target == NULL		)	//û�е�λĿ��
		{
			if (	group->size() > 1
				&&	group->countUnitTypeId() == 1
				&&	group->testFlag(UnitFilter::GROUND)	)	//���ȫ�����ǵ��浥λ���ҵ�λ������ͬ
			{
				packet->flag &= (~IgnoreFormation);		//ǿ��ʹ�������ƶ�
				packet->flag |= (GroupPathing);			//ǿ��ʹ��Ⱥ���ƶ�
			}
		}

		//�վ�����
		if (!data->byProgramm)	//�ǳ�����������
		{
			if (	group->size() > 1
				&&	group->filterExistsMatch(	//���ڿ��е�λ
						UnitFilter(
							UnitFilter::FLYING,
							NULL))
				&&	!group->filterExistsMatch(	//�����ڷ�Ӣ�۵��浥λ
						UnitFilter(
							UnitFilter::GROUND,
							UnitFilter::HERO))	)
			{
				packet->flag |= (IgnoreFormation);	//ǿ�Ʋ�ʹ�������ƶ�
				packet->flag &= (~GroupPathing);	//ǿ�Ʋ�ʹ��Ⱥ���ƶ�
			}
		}

		GroupDestroy(group);
	}

	void Init() {
		MainDispatcher()->listen(EVENT_LOCAL_ACTION, onLocalActionSent);
	}
}