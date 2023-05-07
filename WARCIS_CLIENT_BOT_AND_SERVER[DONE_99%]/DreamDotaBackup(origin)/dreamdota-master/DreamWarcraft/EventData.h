#ifndef EVENT_DATA_H_INCLUDED_
#define EVENT_DATA_H_INCLUDED_

#include "GameStructs.h"
#include "Jass.h"
#include "UnitGroup.h"
#include "GameStructs.h"
#include "RCString.h"



//Custom Events
const uint32_t EVENT_GAME_START					= 1000;
const uint32_t EVENT_GAME_END					= 1001;
const uint32_t EVENT_KEY_UP						= 1002;
const uint32_t EVENT_KEY_DOWN					= 1003;
const uint32_t EVENT_MOUSE_MOVE					= 1004;
const uint32_t EVENT_MOUSE_UP					= 1005;
const uint32_t EVENT_MOUSE_DOWN					= 1006;
const uint32_t EVENT_MOUSE_SCROLL				= 1007;
const uint32_t EVENT_ITEM_CLICK					= 1008;
const uint32_t EVENT_PACKET						= 1009;
const uint32_t EVENT_CONTROL_GROUP_ASSIGN		= 1010;
const uint32_t EVENT_CONTROL_GROUP_SELECT		= 1011;
const uint32_t EVENT_UNIT_ATTACK_MISS			= 1012;
const uint32_t EVENT_UNIT_RECEIVE_DAMAGE		= 1013;
const uint32_t EVENT_UNIT_CREATED				= 1014;
const uint32_t EVENT_LOCAL_ACTION				= 1015;
const uint32_t EVENT_LOCAL_CHAT					= 1016;
const uint32_t EVENT_ITEM_CREATED				= 1017;
const uint32_t EVENT_UNIT_ATTACK_RELEASED		= 1018;
const uint32_t EVENT_UNIT_ACQUIRE_START			= 1019;
const uint32_t EVENT_UNIT_ACQUIRE_READY			= 1020;
const uint32_t EVENT_UNIT_ACQUIRE_STOP			= 1021;
const uint32_t EVENT_FOCUS						= 1022;
const uint32_t EVENT_LOCAL_NETEVENT				= 1023;//���Ȿ��netevent

//LastHit
//const uint32_t EVENT_LASTHIT_DATA_READY = 2000;
//const uint32_t EVENT_LASTHIT_UNIT_IDLE = 2001;

//UI
const uint32_t EVENT_CLICK = 3000;
const uint32_t EVENT_MOUSE_OVER = 3001;
const uint32_t EVENT_MOUSE_OUT = 3002;

//define event data structs
struct KeyboardEventData {
	int code;
	int vkcode;
	char chars[4];//����
	bool shift;
	bool ctrl;
	bool alt;
	bool byProgramm;
	war3::CKeyEvent* evtObj;

	void discard() { if(evtObj) evtObj->baseEvent.id = 0; }
	void resetCode(int newCode) {code = newCode; evtObj->keyCode = newCode;}
};

//���=1 �м�=2 �Ҽ�=4 ��߿���Ĳ��=8 ��߿�ǰ�Ĳ��=0x10
namespace MOUSECODE {
	const uint32_t MOUSE_LEFT		= (1 << 0);
	const uint32_t MOUSE_MIDDLE		= (1 << 1);
	const uint32_t MOUSE_RIGHT		= (1 << 2);
}

struct MouseEventData {
	int mouseCode;
	float x;
	float y;
	war3::CSimpleButton* buttonPushed;
};

struct MouseEventScrollData {
	bool up;
	war3::CControlWheelEvent* evtObj;

	void discard() {if(evtObj) evtObj->baseMouseEvent.baseEvent.id = 0; }
};

struct ItemClickData {
	int mouseCode;
	item clickedItem;
	bool byProgramm;
	war3::CSimpleButtonClickEvent* evtObj;
	void discard() {if(evtObj) evtObj->baseEvent.id = 0; }
};

struct ActionEventData {
	war3::PacketSender *packetSender;
	bool byProgramm;
	uint32_t id;
	ActionType type;
	uint32_t flag;
	item usingItem;
	float x;
	float y;
	widget target;
	item transferItem;
	void discard() {PacketSenderDestroy(packetSender);}
};

struct ControlGroupEventData {
	uint32_t numGroup;
	int reason;
	//TODO: ��λ��
	static const int REASON_ASSIGN = 0;
	static const int REASON_SELECT = 1;
	//...
};

struct UnitCreationEventData {
	unit createdUnit;
};

struct ItemCreationEventData {
	item createdItem;
};

struct UnitDamagedEventData {
	unit target;
	unit source;
	bool isSpell;
	float damage;
	float damageRaw;
};

struct LocalChatEventData {
	char* content;
};

struct UnitAttackReleasedEventData {
	unit attacker;
};

struct UnitAttackMissedEventData {
	unit attacker;
};

struct UnitAcquireEventData {
	unit eventUnit;
	unit target;
};

#endif