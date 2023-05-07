#include "stdafx.h"
#include "Game.h"
#include "Offsets.h"
#include "Packet.h"
#include "Event.h"
#include "Action.h"
#include "Timer.h"
#include "GameTime.h"
#include "Latency.h"
#include "APM.h"


const float JITSelectBackChanceInit = 0.01f;
const float JITSelectBackChanceFactor = 1.5f;
static float JITSelectBackChanceCurrent = 0.0f;
static Timer* JITSelectBackTimer = NULL;

static bool ActionSentByScript = false;
static bool ActionProcessing = false;

#define ACTION_PROGRAMM_BEGIN ActionSentByScript = true;
#define ACTION_PROGRAMM_END   ActionSentByScript = false;
#define ACTION_PROGRAMM (ActionSentByScript!=false)

#define ACTION_PROCESS_BEGIN ActionProcessing = true;
#define ACTION_PROCESS_END   ActionProcessing = false;
#define ACTION_PROCESS (ActionProcessing!=false)

void PacketSend (void *pPacket, uint32_t size, bool sendAsProgramm) {
	void *funcPacketSend = Offset(PACKET_SEND);
	war3::PacketSender sender;
	sender.vtable = (void **)Offset(DATASTORECACHE1460_VTABLE);
	sender.packet = pPacket;
	sender.unk_8 = 0;
	sender.unk_C = 0x5B4;
	sender.sizePacket = size;
	sender.unk_14 = -1;
	if(sendAsProgramm)ACTION_PROGRAMM_BEGIN	//���涯����Ϊ����������
	__asm {
		lea ecx, sender;
		xor edx, edx;
		call funcPacketSend;
	}
	if(sendAsProgramm)ACTION_PROGRAMM_END	//����
}

void PacketSenderDestroy (void *pPacketSender) {
	if ( pPacketSender ) {
		((war3::PacketSender *)pPacketSender)->sizePacket = 0;
		//war3::PacketGeneral *packet = (war3::PacketGeneral *)((war3::PacketSender *)pPacketSender)->packet;
		//packet->id = 0xFF;
	}
}

void PacketSelectionSend (std::set<war3::CUnit *> *set, bool select) {
	war3::CUnit *lpUnit = NULL;
	if (set->size() > 0) {
		war3::PacketSelection packet;
		packet.id = 0x16;
		packet.mode = select ? 1 : 2;
		packet.count = set->size();
		std::set<war3::CUnit *>::iterator iter; uint32_t i = 0;
		for (iter = set->begin(); iter != set->end(); ++iter) {
			lpUnit = *iter;
			if (lpUnit) { 
				packet.objects[i] = lpUnit->hash; 
				++i;
			}
		}
		PacketSend(&packet, (4 + i * 8), true);//TODO ��֤��iȡ��set->size�Ƿ��bug
	}
}

bool UnitSetCheckIdent (std::set<war3::CUnit *> *set1, std::set<war3::CUnit *> *set2) {
	std::set<war3::CUnit *>::iterator iter1, iter2;
	if (set1->size() == set2->size()) {
		iter1 = set1->begin(); iter2 = set2->begin();
		while( iter1 != set1->end()) {
			if (*iter1 != *iter2) {
				//OutputScreen(0,0,10,"check ident: element inequal 0x%X = *iter1 != *iter2 = 0x%X", *iter1, *iter2);
				return false;
			}
			++iter1; ++iter2;
		}
		return true;
	}
	//OutputScreen(0,0,10,"check ident: size inequal %d != %d", set1->size(), set2->size());
	return false;
}

void UnitSetCopyFromArr (std::set<war3::CUnit *> *set, war3::CUnit *arr[], uint32_t & count){
	for (uint32_t i = 0; i < count; ++i) set->insert(arr[i]);
}

void UnitSetCopyToArr (std::set<war3::CUnit *> *set, war3::CUnit *arr[], uint32_t & count){
	std::set<war3::CUnit *>::iterator iter; uint32_t i = 0;
	for (iter = set->begin(); iter != set->end(); ++iter){
		arr[i] = *iter;	i++;
	}
	count = i;
}

//current selections used in JIT selection
static std::set<war3::CUnit *> JustInTimeSelection;

void PacketJustInTimeActionSelectBack ();

void PacketJustInTimeActionSend (
	bool useJIT,
	std::set<war3::CUnit *>	*unitSetToSelect,
	void					*pPacketAction,
	uint32_t				sizePacket,
	bool					sendAsProgramm
)	
{
	JITSelectBackChanceCurrent = JITSelectBackChanceInit;

//	1. ��� !processing, 
//	current <- ����ѡ��(UI); processing <- true;
	if (!ACTION_PROCESS) {
		ACTION_PROCESS_BEGIN
		//OutputScreen(0,0,10,"step 1");
		war3::CUnit *arr[12] = {0};
		uint32_t arr_count = SelectedUnitGet(PlayerLocal(), (void **)arr, false);
		if (arr_count)
			UnitSetCopyFromArr(&JustInTimeSelection, arr, arr_count);
	}

//	[ ���� current != toSelect
//		2. �Ƴ�current
//		3. ���toSelect; current <- toSelect
//		   ˢ��(0x1A); �л�����toSelect[0] (0x19)
//	]
	if (!UnitSetCheckIdent(&JustInTimeSelection, unitSetToSelect)){
		PacketSelectionSend(&JustInTimeSelection, false);
		PacketSelectionSend(unitSetToSelect, true);

		war3::CUnit *unitSetToSelectBegin;;
		if (unitSetToSelect->size() && (unitSetToSelectBegin = *(unitSetToSelect->begin()))){
			war3::PacketPreSubSelection preSubSel; preSubSel.id = 0x1A;
			PacketSend(&preSubSel, 1, true);
		
			uint32_t typeId = unitSetToSelectBegin->typeId;
			war3::PacketSubgroup subg; subg.id = 0x19;
			subg.typeId = typeId;
			subg.object = unitSetToSelectBegin->hash;
			PacketSend(&subg, 13, true);
		}

		JustInTimeSelection = *unitSetToSelect;
		
		//OutputScreen(0,0,10,"step 2, 3");
	}

//	4. action
	PacketSend(pPacketAction, sizePacket, sendAsProgramm);
	//OutputScreen(0,0,10,"step 4");

	if (!useJIT){
		PacketJustInTimeActionSelectBack();
	}
}

//	7. current <- ��; processing <- false
void PacketJustInTimeActionClear () {
	JITSelectBackChanceCurrent = JITSelectBackChanceInit;
	JustInTimeSelection.clear();
	ACTION_PROCESS_END
	//OutputScreen(0,0,10,"step 7");
}

//	[ ���� current != ����ѡ��(UI)
//		5. �Ƴ�current
//		6. ��� ����ѡ��(UI)
//         ˢ��(0x1A); �л����� ���ظ�������(0x19)
//	]
void PacketJustInTimeActionSelectBack () {
	war3::CUnit *arr[12] = {0};	war3::CUnit *arrSubgroup[12] = {0};
	uint32_t arr_count = SelectedUnitGet(PlayerLocal(), (void **)arr, false);
	uint32_t arrSubgroup_count = ActiveSubgroupGet(PlayerLocal(), (void **)arrSubgroup);

	std::set<war3::CUnit *> setCurrentSelection, setCurrentSubgroup;
	if (arr_count)
		UnitSetCopyFromArr(&setCurrentSelection, arr, arr_count);
	if (arrSubgroup_count)
		UnitSetCopyFromArr(&setCurrentSubgroup, arrSubgroup, arrSubgroup_count);

	if (!UnitSetCheckIdent(&JustInTimeSelection, &setCurrentSelection)){
		//OutputScreen(0,0,10,"step 5, 6");
		PacketSelectionSend(&JustInTimeSelection, false);//BUG��������Ϸ��Ҫ����
		PacketSelectionSend(&setCurrentSelection, true);
		if (setCurrentSubgroup.size()){
			war3::CUnit *setCurrentSubgroupBegin = *(setCurrentSubgroup.begin());
			if (setCurrentSubgroupBegin){
				war3::PacketPreSubSelection preSubSel; preSubSel.id = 0x1A;
				PacketSend(&preSubSel, 1, true);
				uint32_t typeId = setCurrentSubgroupBegin->typeId;
				war3::PacketSubgroup subg; subg.id = 0x19;
				subg.typeId = typeId; subg.object = setCurrentSubgroupBegin->hash;
				PacketSend(&subg, 13, true);
			}
		}
	}
	PacketJustInTimeActionClear();
}

void JITSelectBackTimerFunc (Timer *tm) {
	if (ACTION_PROCESS){
		tm->setTimeout(RandomFloat(0.06f, 0.14f));
		JITSelectBackChanceCurrent *= JITSelectBackChanceFactor;
		if (RandomFloat(0.0f, 1.0f) <= JITSelectBackChanceCurrent){
			PacketJustInTimeActionSelectBack();
		}
	}
}


static Event PacketEventObject;
void PacketEventDispatch (war3::PacketSender *pPacketSender, void *pPacket, uint32_t packetId) {
	if (!IsInGame())
		return;

	//OutputScreen(10, "PacketEventDispatch, packetId = 0x%X", packetId);

	PacketEventObject.setId(EVENT_LOCAL_ACTION);
	ActionEventData data;

	data.packetSender = pPacketSender;
	data.type = TargetNone;
	data.x = 0;
	data.y = 0;
	data.target = NULL;

	//ͨ�ò���
	data.byProgramm = ACTION_PROGRAMM;
	data.id		= ((war3::PacketAction *)pPacket)->actionId;
	data.flag	= ((war3::PacketAction *)pPacket)->flag;
	data.usingItem = ObjectToHandle(CItemFromHash(	&(((war3::PacketAction *)pPacket)->hashUsedItem)	));

	//point
	if (packetId == 0x11){
		data.type = TargetPoint;
		data.x = ((war3::PacketActionPoint *)pPacket)->targetX;
		data.y = ((war3::PacketActionPoint *)pPacket)->targetY;
	}

	//target
	if (packetId == 0x12){
		data.type = Target;
		data.x = ((war3::PacketActionTarget *)pPacket)->targetX;
		data.y = ((war3::PacketActionTarget *)pPacket)->targetY;
		data.target = ObjectToHandle(AgentFromHash(	&(((war3::PacketActionTarget *)pPacket)->hashWidgetTarget)	));
	}

	//drop item
	if (packetId == 0x13){
		data.type = DropItem;
		data.x = ((war3::PacketActionDropItem *)pPacket)->targetX;
		data.y = ((war3::PacketActionDropItem *)pPacket)->targetY;
		data.target = ObjectToHandle(AgentFromHash(	&(((war3::PacketActionDropItem *)pPacket)->hashTarget)	));
		data.transferItem = ObjectToHandle(CItemFromHash(	&(((war3::PacketActionDropItem *)pPacket)->hashItem)	));
	}
	
	//to add

	//send
	PacketEventObject.setData<ActionEventData> (&data);
	MainDispatcher()->dispatch(&PacketEventObject);
}

static Event ControlGroupEventObj;
void ControlGroupAssignEventDispatch ( war3::PacketControlGroupAssign *pPacket ) {
	ControlGroupEventData data;
	ControlGroupEventObj.setId(EVENT_CONTROL_GROUP_ASSIGN);
	data.reason = ControlGroupEventData::REASON_ASSIGN;
	data.numGroup = pPacket->numGroup;
	ControlGroupEventObj.setData<ControlGroupEventData>(&data);
	MainDispatcher()->dispatch(&ControlGroupEventObj);
}

struct PacketHistoryInfo {
	float timeGame;
	uint32_t id;
	PacketHistoryInfo (float _time, uint32_t _id) : timeGame(_time), id(_id) { }
};
static std::list<PacketHistoryInfo> LocalPacketQueue, LocalActionPacketQueue;
void PacketQueueRefresh(std::list<PacketHistoryInfo> *queue){
	if (queue->size() > 0x100) {
		OutputScreen(10, "Local packet queue overrun: id = 0x%X", queue->rbegin()->id);
		while(queue->size() > 0x100){
			queue->pop_front();//��ֹ����
		}
	}
}

static Event packetEvtObj;
void PacketAnalyze (void *pPacketSender) {
	war3::PacketSender *sender = (war3::PacketSender *)pPacketSender;
	uint8_t packetId = *(uint8_t *)(sender->packet);

//	��processing״̬ʱ����⡾�Ǳ����������ġ������������Ҳ�����war3�Զ������Ĳ�����
//	ֻ�������з�� ����(0x10 ~ 0x14)��ѡ��λ(0x16)��ѡ����(0x18)���л�����(0x19)��ˢ��(0x1A)
//	5.a. �������Ϊ ����(0x10 ~ 0x14, 0x1D, 0x1E)���Ӳ�5, 6, 7�����
//	5.b. �������Ϊ ѡ����(0x18)���Ӳ�7�����
//	5.c. �������Ϊ ѡ��λ(0x16)��ˢ��(0x1A)���л�����(0x19)��ѡ����Ʒ(0x1C)�����ط��ʹ֮��Ч��
	if (ACTION_PROCESS && !ACTION_PROGRAMM){
		switch (packetId) {
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x1D:
		case 0x1E:
		case 0x61: //����ESC
		case 0x66: //����ѡ�����Ӳ˵�
		case 0x67: //���뽨���Ӳ˵�
			PacketJustInTimeActionSelectBack(); break;
		case 0x18:
			PacketJustInTimeActionClear(); break;
		case 0x16:
		case 0x1A:
		case 0x19:
		case 0x1C:
			//OutputScreen(0,0,10,"destroy packet!");
			PacketSenderDestroy(sender); return;//break;
		}
	}

//	any packet sent event
	if (IsInGame()){//MUSTDO ��Ȼ�����
		packetEvtObj.setId(EVENT_PACKET);
		MainDispatcher()->dispatch(&packetEvtObj);
	}


//	local action sent event
//	0x10 ~ 0x14
	switch (packetId) {
	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
	case 0x14:
		PacketEventDispatch(sender, sender->packet, packetId); break;
	}

//	local control group assign event
//	0x17
	switch (packetId) {
	case 0x17:
		ControlGroupAssignEventDispatch((war3::PacketControlGroupAssign *)sender->packet);
	}

//	����ϵͳ
	//LocalPacketQueue.push_back(PacketHistoryInfo(Time(), packetId));
	//PacketQueueRefresh(&LocalPacketQueue);
	if (packetId >= 0x10 && packetId <= 0x14){
		LocalActionPacketQueue.push_back(PacketHistoryInfo(Time(), packetId));
		PacketQueueRefresh(&LocalActionPacketQueue);
	}
}

void PacketNetEventAnalyze (war3::CEvent *evt) {
	if (!evt) return;
	uint32_t id = evt->id;
	//if (id < 0x40000000) OutputScreen(10, "event id = 0x%X", id);
	if (id >= 0xA0000 && id < 0x40000000) //��Χ��
	{
		uint8_t playerId = ((uint8_t *)evt)[0x15];

		//������ң�����APM
		APMUpdateAction(playerId, (id-0xA0000));

		//������ң��������Ȿ��netevent�¼�
		if (playerId == PlayerLocal())
		{	
			if (IsInGame()){
				Event localNetEvt;
				localNetEvt.setId(EVENT_LOCAL_NETEVENT);
				MainDispatcher()->dispatch(&localNetEvt);
			}

			id -= 0xA0000;
			std::list<PacketHistoryInfo>::iterator iter;
			bool found = false;
			/*
			for (iter = LocalPacketQueue.begin(); iter != LocalPacketQueue.end(); ++iter){
				if (iter->id == id) { found = true; break; }
			}
			if (found) {
				while (LocalPacketQueue.front().id != id) {
					LocalPacketQueue.pop_front();
				}
				LocalPacketQueue.pop_front();
			}
			*/

			if (id >= 0x10 && id <= 0x14) {
				found = false;
				for (iter = LocalActionPacketQueue.begin(); iter != LocalActionPacketQueue.end(); ++iter){
					if (iter->id == id) { found = true; break; }
				}
				if (found) {
					while (LocalActionPacketQueue.front().id != id) {
						LocalActionPacketQueue.pop_front();
					}
					float lag = Time() - LocalActionPacketQueue.front().timeGame;
					if (lag > 0 && lag < 2.5f){//������ֵ��ֹbug
						Latency_Update(lag);
					}
					while (LocalActionPacketQueue.size() && LocalActionPacketQueue.front().id == id) {//��������������ͬ�ļ�¼������Ϊ����ͬ������һ������
						LocalActionPacketQueue.pop_front();
					}
				}
			}

		}
	}
}

/*
�ƻ�
Just-in-time selection

�õ�����
SelectedUnitGet		��ȡ����ѡ��(UI)
ActiveSubgroupGet	��ȡ���ظ�������(UI)

�Ե�λ����toSelect��������action��

1. ��� !processing, 
   current <- ����ѡ��(UI); processing <- true;

[ ���� current != toSelect
  2. �Ƴ�current
  3. ���toSelect; current <- toSelect
     ˢ��(0x1A); �л�����toSelect[0] (0x19)
]

4. action

[ 5, 6, 7��ִ�м�����

  [ ���� current != ����ѡ��(UI)
    5. �Ƴ�current
    6. ��� ����ѡ��(UI)
       ˢ��(0x1A); �л����� ���ظ�������(0x19)
  ]
  7. current <- ��; processing <- false
]

��processing״̬ʱ����⡾�Ǳ����������ġ������������Ҳ�����war3�Զ������Ĳ�����
ֻ�������з�� ����(0x10 ~ 0x14)��ѡ��λ(0x16)��ѡ����(0x18)���л�����(0x19)��ˢ��(0x1A)

5.a. �������Ϊ ����(0x10 ~ 0x14, 0x1D, 0x1E)���Ӳ�5, 6, 7�����
5.b. �������Ϊ ѡ����(0x18)���Ӳ�7�����
5.c. �������Ϊ ѡ��λ(0x16)��ˢ��(0x1A)���л�����(0x19)��ѡ����Ʒ(0x1C)�����ط��ʹ֮��Ч��

һ������ĵ���ʱ����������ʱ������ָ����ʣ�����0.5�����x%���ʻָ�ѡ��1�����2x%���ʣ�1.5����4x%���ʡ�����

ԭ������ѡ��(UI)�뱾�ظ�������(UI)���Ƕ�Ӧ����ʾ�ĵ�λѡ��״̬�������ȫ�����������
ѡ�����¼���ѡ��λ������Ӱ�쵽����ѡ��(UI)���л������Ӱ�쵽���ظ�������(UI)
����ѡ�����¼���������������Ӹ���ԭ����ѡ����˿���ֱ������
ѡ��λ���л�������������ط�������ܵ���ͬ��ѡ��bug����Ϊ�����ķ������Ӧ������������ѡ��
�ڱ���ѡ��(UI)����ʵ����ѡ���첽�Ĺ����У���λ����current��¼����ʵ����ѡ��
��һ����Ҷ�������ʱ�����ز��Ӳ�ѡ�����ʹ�ñ���ѡ��ָ�UI��ѡ��

*/

void Packet_Cleanup() {
	APM_Cleanup();
	ACTION_PROGRAMM_END
	PacketJustInTimeActionClear();
	JITSelectBackTimer->destroy();
	LocalPacketQueue.clear();
	LocalActionPacketQueue.clear();
}

void Packet_Init() {
	JITSelectBackTimer = GetTimer(0.1, JITSelectBackTimerFunc, true);
	JITSelectBackTimer->start();
	APM_Init();
}