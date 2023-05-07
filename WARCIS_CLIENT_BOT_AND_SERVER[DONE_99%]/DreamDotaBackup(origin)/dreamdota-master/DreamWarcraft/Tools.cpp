#include "stdafx.h"
#include <VMP.h>
#include "Game.h"
#include "Tools.h"
#include "Storm.h"
#include "Offsets.h"
#include <cstdarg>
#include "FunctionTemplate.h"
#include "Player.h"
#include "RCString.h"

static void *GameGlobalUI;
static void *GameProfileInt;
static void *GameProfileFloat;
static void *GameUnitArrayGet;
static war3::CGameWar3 **GameGlobalObject;
static void *GameStateGet;
static void *GameObjectFromHash;
static void *GameHandleFromObject;
static void *GameTimeGet;
static void *GameUnitGetObject;
static void *GameItemGetObject;
static void *GameAbilityFromUnit;
static void *GameUIToggle;
static void *GameUnitSelectionPrioGet;
static void *GameSoundPlay;
static void *DefaultUnitTagGet;
static war3::MapInfo *GameMapInfo;
static war3::CGxDevice **CGxDevice;
static void *GlobalGlueMgr;
static void *InputObserverGetCall;
static void *PlayerNameGetCall;
static void *ObjectNameGetCall;
static void *GetIcon;
static void *GetIconObj;
//static void *AbilitySpellCheck;
static void *AgentTimerVTable;
static void *PlayerTechtreeGet;
static void **AgileData;
static void **CCommandButtonVtable;
static void *AbilityDefDataGetByAID;
static void *AbilityUIDefGetByAID;
static void *SelectionSubgroupSetFunc;
static void *SelectionSyncFunc;
static void *NativeFuncGet;
static void *NativeFuncCreate;
static void *JassStringRvToRCString;
static void *UnitDataHashTable;
static void *UnitDataNodeGet;
static void *ItemDataHashTable;
static void *ItemDataNodeGet;

typedef void (__fastcall *Prototype_GameUITextDisplay)(void* pthis, uint32_t dummy, float x, float y, const char* buffer, float duration, int unknown);
Prototype_GameUITextDisplay GameUITextDisplay;
void *GameMissileSpeedGet;

//------------------------------------------------
// implementation

war3::CGameUI* PrepareGameUI()
{
	war3::CGameUI* rv;
	__asm {
		mov ecx, 1;
		mov edx, 0;
		call GameUIToggle;
		mov rv, eax;
	}
	return rv;
}

//�������Ļ
int OutputScreen(float duration, const char *format, ...) {
	if (!IsInGame() || !GameGlobalUI) return 0;

	char buffer[1024];
	int rv;
	va_list args;
	va_start(args, format);
	rv = vsprintf_s(buffer, 1024, format, args);
	va_end(args);

	PrepareGameUI();

	GameUITextDisplay(*((void**)GameGlobalUI), 0, 0.0, 0.0, buffer, duration, -1);

	return rv;
}

void PingMinimapEx(float x, float y, float duration, int red, int green, int blue, bool extraEffect) 
{
	war3::CGameUI *GlobalUI = PrepareGameUI();
	int color = blue | (( green | ((red | 0xFFFFFF00) << 8)) << 8);
	int extra = extraEffect ? 1 : 0;

	void *func = Offset(MINIMAP_PING);
	aero::generic_this_call<void>(func, GlobalUI->minimap, x, y, 0.f, duration, &color, extra);
}

DWORD VersionGet(LPCSTR fileName, unsigned int section) {
	DWORD version = 0;
	unsigned int infoSize = GetFileVersionInfoSizeA(fileName, NULL);
	if (infoSize) {
		char *buffer = (char *) malloc(infoSize + 1);
		if (GetFileVersionInfoA(fileName, NULL, infoSize, buffer)) {
			VS_FIXEDFILEINFO *pFixedInfo;	unsigned int infoLength;
			if (VerQueryValueA(buffer, "\\", reinterpret_cast<LPVOID *>(&pFixedInfo), &infoLength)){
				switch(section){
					case 1: {version = (pFixedInfo->dwFileVersionMS >> 0x10);break;}
					case 2: {version = (pFixedInfo->dwFileVersionMS & 0xFFFF);break;}
					case 3: {version = (pFixedInfo->dwFileVersionLS >> 0x10);break;}
					case 4: {version = (pFixedInfo->dwFileVersionLS & 0xFFFF);break;}
					default: {break;}
				}
			}
		}
		free(buffer);
	}
	return version;
}

DWORD GameCodepage() {
//  English (US)            = 0x00000409
//  English (UK)            = 0x00000809
//  French                  = 0x0000040c
//  German                  = 0x00000407
//  Spanish                 = 0x0000040a
//  Italian                 = 0x00000410
//  Czech                   = 0x00000405
//  Russian                 = 0x00000419
//  Polish                  = 0x00000415
//  Portuguese (Brazilian)  = 0x00000416
//  Portuguese (Portugal)   = 0x00000816
//  Turkish                 = 0x0000041f
//  Japanese                = 0x00000411
//  Korean                  = 0x00000412
//  Chinese (Traditional)   = 0x00000404
//  Chinese (Simplified)    = 0x00000804
//  Thai                    = 0x0000041e
	LANGID langid = Storm::FileGetLocale();
	switch (langid){
	case 0x00000409:
		return 0;
	case 0x00000809:
		return 0;
	case 0x0000040c:
		return 0;
	case 0x00000407:
		return 0;
	case 0x0000040a:
		return 0;
	case 0x00000410:
		return 0;
	case 0x00000405:
		return 0;
	case 0x00000419:
		return 0;
	case 0x00000415:
		return 0;
	case 0x00000416:
		return 0;
	case 0x00000816:
		return 0;
	case 0x0000041f:
		return 0;
	case 0x00000411:
		return 0;
	case 0x00000412:
		return 0;
	case 0x00000404:
		return 0;
	case 0x00000804:
		return 936;
	case 0x0000041e:
		return 0;
	default:
		return 0;
	}
}

//'hfoo' --> "hfoo"
char IntegerIdToCharBuffer[5] = {0};
const char *IntegerIdToChar (uint32_t id){
	uint32_t remainingValue = id;
	for (uint32_t byteno = 0; byteno < 4; byteno++){
		IntegerIdToCharBuffer[3 - byteno] = remainingValue % 256;
		remainingValue /= 256;
	}
	return IntegerIdToCharBuffer;
}

const char *ObjectIdToNameChar(uint32_t typeId) {
	if (!typeId) return "";
	char *rv;
	__asm{
		mov ecx, typeId;
		xor edx, edx;
		call ObjectNameGetCall;
		mov rv, eax;
	}
	return rv;
}

uint32_t TimeGet() {
	uint32_t rv;
	__asm {
		call GameTimeGet;
		mov rv, eax
	}
	return rv;
}

//�������ֵ
float RandomFloat (float lowerBound, float upperBound){
	if (lowerBound >= upperBound) return lowerBound;
	return lowerBound + (upperBound - lowerBound) * (float)rand()/(float)RAND_MAX;
}

//��ȡ��Ϸ�����ı��ļ����� - float
float GameDataProfileGetFloat (LPCSTR appName, LPCSTR keyName, uint32_t index){
	float rv = 0;
	__asm{
		push index;
		push keyName;
		mov edx, appName;
		lea ecx, rv;
		call GameProfileFloat;
	}
	return rv;
}

//��ȡ��Ϸ�����ı��ļ����� - int
int GameDataProfileGetInt (LPCSTR appName, LPCSTR keyName, uint32_t index){
	int rv = 0;
	__asm{
		push index;
		mov edx, keyName;
		mov ecx, appName;
		call GameProfileInt;
		mov rv, eax;
	}
	return rv;
}

//<�ײ�>��ȡ���е�λ
void** UnitGrabAll(uint32_t &count) {
	if (!GameGlobalUI) return NULL;
	void **pArray; 
	uint32_t ucount;
	__asm {		
		mov eax, GameGlobalUI;
		mov eax, [eax];
		mov ecx, [eax + 0x3BC];
		push 1;
		call GameUnitArrayGet;
		mov ecx, [eax+4];
		mov ucount, ecx;
		mov ecx, [eax+8];
		mov pArray, ecx;
	}
	count = ucount;
	return pArray;
}

//<�ײ�>ת��hash group������
void *ObjectFromHashOffset(war3::HashGroup *hash, int offsetInBytes) {
	if (!hash) return NULL;
	int off1 = offsetInBytes;
	uint32_t val1 = hash->hashA;
	uint32_t val2 = hash->hashB;
	if (!val1 || val1 == 0xFFFFFFFF) {return 0;}
	void *obj = 0;
	__asm {
		mov ecx, val1;
		mov edx, val2;
		call GameObjectFromHash;
		test eax, eax;
		je quit;
		add eax, off1;
		mov eax, [eax];
		mov obj, eax;
	quit:
	}
	return obj;
}

void *AgentFromHash(war3::HashGroup *hash) {
	void* obj = ObjectFromHash(hash);
	if (!obj || *(uint32_t*)((uint32_t)obj + 0x20))
		return NULL;
	else
		return *(void**)((uint32_t)obj + 0x54);
}

war3::CUnit *CUnitFromHash(war3::HashGroup *hash) {
	if (!hash) return NULL;
	war3::CAgent *agent = (war3::CAgent *)AgentFromHash(hash);
	return (agent && AgentTypeIdGet(agent) == '+w3u')? (war3::CUnit *)agent : NULL;
}

war3::CItem *CItemFromHash(war3::HashGroup *hash) {
	if (!hash) return NULL;
	war3::CAgent *agent = (war3::CAgent *)AgentFromHash(hash);
	return (agent && AgentTypeIdGet(agent) == 'item')? (war3::CItem *)agent : NULL;
}

void* ObjectFromHash(war3::HashGroup *hash) {
	if (!hash) return NULL;
	uint32_t val1 = hash->hashA;
	uint32_t val2 = hash->hashB;
	void *obj = 0;
	__asm {
		mov ecx, val1;
		mov edx, val2;
		call GameObjectFromHash;
		mov obj, eax
	}
	return obj;
}

//<�ײ�>ת����Ϸ�ڶ���jass handle
uint32_t ObjectToHandle(void *pObj) {
	if (!pObj) return 0;
	uint32_t result;
	__asm {
		mov ecx, GameGlobalObject
		mov ecx, [ecx]
		call GameStateGet
		mov ecx, eax
		push 0
		push pObj
		call GameHandleFromObject
		mov result,eax
	}
	return result;
}

//<�ײ�>ת��jass unit handle������
war3::CUnit *UnitGetObject(uint32_t handleId) {
	return handleId?
	aero::generic_fast_call<war3::CUnit *>(
		GameUnitGetObject,
		handleId
	) : NULL;
}

//<�ײ�>ת��jass item handle������
war3::CItem *ItemGetObject(uint32_t handleId) {
	return handleId?
	aero::generic_fast_call<war3::CItem *>(
		GameItemGetObject,
		handleId
	) : NULL;
}

//<�ײ�>��������ѡ��λ, playerslot 0 ~ 15, syncOnlyΪ���ʾ��ȡͬ��ѡ��, �����ȡ����ѡ��
//��������
uint32_t SelectedUnitGet(int playerSlot, void *unitArray[], bool syncOnly) {
	war3::CSelectionWar3 *selection = (*GameGlobalObject)->players[playerSlot]->selection;
	war3::UnitList *unitList = syncOnly ? &(selection->unitSelectedSync) : &(selection->unitSelectedLocal);
	
	uint32_t count = 0;
	war3::UnitListNode *node = unitList->firstNode;
	while ((count < unitList->nodeCount) && node) {
		unitArray[count++] = node->unit;
		node = node->nextNode;
	}
	return count;
}

//<�ײ�>��������ѡ��Ծ����, playerslot 0 ~ 15
uint32_t ActiveSubgroupGet(int playerSlot, void *unitArray[]) {
	if (!GameGlobalObject || !(*GameGlobalObject)) return 0;
	war3::CSelectionWar3 *selection = (*GameGlobalObject)->players[playerSlot]->selection;
	war3::UnitList *subgroup = selection->activeSubgroup;
	uint32_t count = 0;
	if (subgroup) {
		war3::UnitListNode *node = subgroup->firstNode;
		while ((count < subgroup->nodeCount) && node) {
			unitArray[count++] = node->unit;
			node = node->nextNode;
		}
	}
	return count;
}

//<�ײ�>���������ѡ��Ծ����
void ActiveSubgroupSet(war3::CUnit *targetUnit)
{
	if (!GameGlobalObject || !(*GameGlobalObject) || (!targetUnit)) return;
	war3::CSelectionWar3 *selection = (*GameGlobalObject)->players[PlayerLocal()]->selection;
	if (selection)
	{
		aero::generic_this_call<void>( SelectionSubgroupSetFunc, selection, targetUnit );
		aero::generic_this_call<void>( SelectionSyncFunc, selection, 0 ); //TODO
	}
}

//<�ײ�>��õ�λѡ�����ȼ�
float UnitSelectionPrioGet(war3::CUnit *lpUnit){
	if (!lpUnit) return 0;
	float rv;
	__asm{
		lea eax, rv;
		push eax;
		mov ecx, lpUnit;
		call GameUnitSelectionPrioGet;
	}
	return (float)(rv / 40000.0);
}

//<�ײ�><����>��õ�λ���ܶ���
war3::CAbility *AbilityObjGet (war3::CUnit *lpUnit, uint32_t abilityTypeId){
	if (!lpUnit || !abilityTypeId) { 
		return NULL; 
	}
	war3::CAbility *result;
	__asm {
		mov ecx, lpUnit
		push 1
		push 1
		push 1 //������ĳ�༼��Ϊ0
		push 0
		push abilityTypeId
		call GameAbilityFromUnit
		mov  result, eax
	}
	return result;
}

//<�ײ�>��õ�λ�������ܶ���
war3::CAbilityAttack *AttackAbilityObjGet (uint32_t unitHandleId) {
	if (!unitHandleId) 
		return NULL;
	war3::CAbilityAttack *abilAttack = NULL;
	war3::CUnit *lpUnit = UnitGetObject(unitHandleId);
	if (lpUnit) { 
		abilAttack = (war3::CAbilityAttack *)AbilityObjGet(lpUnit, 'Aatk'); 
	}
	return abilAttack;
}

//<�ײ�>��ü�ʱ��ʣ��ʱ��
float AgentTimerRemainingTimeGet(war3::CAgentTimer* pAgentTimer) {
	if (!pAgentTimer) return 0;
	void** vtable = pAgentTimer->vtable;
	if (!vtable) return 0;
	if (AgentTimerVTable != vtable) return 0;
	void* vfunc_0x18 = vtable[0x18 / 4];
	if (!vfunc_0x18) return 0;
	float rv;
	__asm {
		mov ecx, pAgentTimer;
		lea eax, rv;
		push eax;
		call vfunc_0x18;
	}
	return rv;
}

//<�ײ�>����ӵ��ٶ�
float UnitTypeMissileSpeedGet (uint32_t unitTypeId, int weapon) {
	float rv;
	__asm {
		lea ecx, rv;
		mov edx, unitTypeId;
		push weapon
		call GameMissileSpeedGet;
	}
	return rv;
}

//����ӵ�λ��
bool BulletPositionGet (war3::ProjectilePosition *posInfo, float &x, float &y, float &z){
	bool rv = false;
	if (posInfo) {
		if ((int)(posInfo->posHash.hashA) &&
			(int)(posInfo->posHash.hashB) &&
			(int)(posInfo->posHash.hashA) != -1	&&
			(int)(posInfo->posHash.hashB) != -1 &&
			ObjectFromHash(&(posInfo->posHash)) != 0//FIXME �Ƿ�ͨ������ʹ�ò���Ҫ
			)
		{
			float xyz[3] = {x, y, z};
			void *func = Offset(PROJECTILE_POSITION_GET);
			__asm{
				mov ecx, posInfo;
				xor edx, edx;
				lea eax, xyz;
				push eax;
				call func;
			}
			x = xyz[0]; y = xyz[1]; z = xyz[2];
			rv = true;
		}
	}
	return rv;
}

//<�ײ�>���TextTagManager
war3::CTextTagManager* JassTextTagManagerGet() {
	war3::CTextTagManager* rv;
	__asm {
		mov ecx, GameGlobalObject
		mov ecx, [ecx]
		call GameStateGet
		add eax, 0x2C8
		mov rv, eax
	}
	return rv;
}

war3::CGameWar3* GameObjectGet() {
	return *GameGlobalObject;
}

//<�ײ�>���GameState
war3::CGameState* GameStateObjectGet() {
	war3::CGameState* rv;
	__asm {
		mov ecx, GameGlobalObject
		mov ecx, [ecx]
		call GameStateGet
		mov rv, eax
	}
	return rv;
}

//<�ײ�>��õ�ͼ��Ϣ
war3::MapInfo* MapInfoGet() {
	return GameMapInfo;
}



struct TypeDescriptor {
	DWORD pVFTable;
	DWORD spare;
	DWORD unk0x8;
	const char* name;
};
struct RTTICompleteObjectLocator {
	DWORD signature;
	DWORD offset;
	DWORD cdOffset;
	TypeDescriptor* pTypeDescriptor;
	DWORD pClassHierarchyDescriptor;
};
//<�ײ�>��ȡRTTI����
const char* RTTIClassNameGet(void* object) {
	__try {
		void*** ppp = (void***)object;
		//VTable
		void** pp = *ppp;

		//VTable - 0x4
		pp = (void**)((uint32_t)pp - (uint32_t)0x4);
		void* p = *pp;
		RTTICompleteObjectLocator* p1 = (RTTICompleteObjectLocator*)p;
		TypeDescriptor* p2 = (TypeDescriptor*)p1->pTypeDescriptor;
		return (const char*)((uint32_t)p2 + (uint32_t)0xC);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return NULL;
	}
}

//<�ײ�>����Dump Object
uint32_t TryDumpObject(void* object, uint8_t* buffer, uint32_t size) {
	uint32_t success_size = 0;
	__try {
		for (uint32_t i = 0; i < size; ++i) {
			buffer[i] = ((uint8_t*)object)[i];
			++ success_size;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return success_size;
	}
	return success_size;
}

//������Ч(slk���ַ���)
void SoundPlay(const char *soundlink, war3::CUnit *unit, uint32_t mode) {
	//OutputScreen(10,"soundplay");
	if (!soundlink) return;
	__asm{
		push 0;//?
		push 0;//unit;//?
		push 0;//?
		push 0;//?
		push 0;//?
		push 0;
		//lea ecx, mode; //
		//push ecx;//
		mov edx, soundlink;
		xor ecx, ecx//mov ecx, unit;
		call GameSoundPlay;
	}
}

//���CGxDevice
war3::CGxDevice* GxDeviceGet() {
	return *CGxDevice;
}

//<�ײ�>���GameUI
war3::CGameUI* GameUIObjectGet() {
	return *reinterpret_cast<war3::CGameUI**>(GameGlobalUI);
}

//<�ײ�>���GlueMgr
war3::CGlueMgr* GlueMgrObjectGet() {
	return *reinterpret_cast<war3::CGlueMgr**>(GlobalGlueMgr);
}

//���Ĭ�ϵ�UnitTip
war3::CUnitTip* DefaultUnitTipGet() {
	war3::CUnitTip* rv = NULL;
	__asm {
		call DefaultUnitTagGet;
		mov rv, eax;
	}
	return rv;
}

//��ȡ�߳�����
DWORD* GlobalGameTlsIndex;
void* GameTlsDataGet(uint32_t index) {
	DWORD tlsIndex = *GlobalGameTlsIndex;
	void* tlsValue = TlsGetValue(tlsIndex);
	if (tlsValue)
		return aero::offset_element_get<void*>(tlsValue, index * 4);
	else
		return NULL;
}

DWORD GameTlsIndexGet() {
	return *GlobalGameTlsIndex;
}

//��ȡPreselectUIData
void* GlobalPreselectUIData;
war3::PreselectUIData* PreselectUIDataGet() {
	return reinterpret_cast<war3::PreselectUIData*>(GlobalPreselectUIData);
}

//��ȡ��ǰ����µĶ���
void* ObjectUnderCursor;
void* ObjectUnderCursorGet() {
	return *reinterpret_cast<void**>(ObjectUnderCursor);
}

//��ȡInputObserver
war3::CObserver* InputObserverGet() {
	return aero::generic_std_call<war3::CObserver*>(
		InputObserverGetCall
	);
}

//��ȡGlueMgr�˵�HashTable
war3::TSHashTable_UFRAMENAMECREATEHANDLER* FrameNameHashTableGet() {
	return (war3::TSHashTable_UFRAMENAMECREATEHANDLER*)Offset(GLOBAL_FRAMENAMEHASHTABLE);
}

//��ȡ�Ѿ����ڵĲ˵�HashNode
war3::FrameNameHashNode* FrameNameHashNodeGet(const char* name) {
	return aero::generic_this_call<war3::FrameNameHashNode*>(
		Offset(FRAMENAMEHASHTABLE_GETHASHNODE),
		FrameNameHashTableGet(),
		name
	);
}

//��ȡ¼������ģʽ
uint32_t ReplayState() {
	void* data = aero::offset_element_get<void*>(GameTlsDataGet(0xD), 0x10);
	war3::CNetData* War3NetData = aero::offset_element_get<war3::CNetData*>(data, 0x8);
	return War3NetData->replayState;
}

//������ֻ�ȡ
const char* PlayerNameGet(int playerId, uint32_t arg) {
	return aero::generic_fast_call<const char*>(
		PlayerNameGetCall,
		playerId,
		arg
	);
}

//��ȡAgent���ID
inline uint32_t AgentTypeIdGet( war3::CAgent* agent )
{
	return aero::generic_this_call<uint32_t>(
		VTBL(agent)[0x1C / sizeof(void*)],
		agent
	);
}


//�ж��Ƿ�agent
bool ObjectIsAgent( void *obj )
{
	if (obj)
	{
		DWORD *hashObj = (DWORD *)ObjectFromHash( (war3::HashGroup *)(((DWORD *)obj) + 0x3) ); //hashgroup
		if (hashObj)
		{
			//OutputScreen(10, "obj type = %s", IntegerIdToChar(*(hashObj + 3)) );
			return *(hashObj + 3) == '+agl';
		}
	}
	return false;
}


//FramePoint
uint32_t GetUnitFramePoint(war3::CUnit* unit, FramePoint* point) {
	if (!unit || !point) return 0;//TODO ��֤�Ƿ��bug
	float unk = 0.0;	//TODO ����ʲô
	return aero::generic_fast_call<uint32_t>(Offset(UNIT_FRAMEPOSITION_GET), unit, point, &unk);
}

char *GetIconFromFuncProfile(uint32_t typeId, uint32_t index){
	if (!typeId) return NULL;
	char *rv;
	const char *typeIdStr = IntegerIdToChar(typeId);
	char *param = "Art";
	rv = aero::generic_this_call<char *>(GetIcon, *(DWORD*)GetIconObj, typeIdStr, param, 0, index);
	return rv;
}

char *GetBuffIconFromFuncProfile(uint32_t typeId, uint32_t index){
	if (!typeId) return NULL;
	char *rv;
	const char *typeIdStr = IntegerIdToChar(typeId);
	char *param = "Buffart";
	rv = aero::generic_this_call<char *>(GetIcon, *(DWORD*)GetIconObj, typeIdStr, param, 0, index);
	return rv;
}

void GetIconPos(uint32_t typeId, uint32_t &x, uint32_t &y)//FIXME
{
	if (!typeId) return;
	const char *typeIdStr = IntegerIdToChar(typeId);
	char *param = "Buttonpos";
	char *posXstr = aero::generic_this_call<char *>(GetIcon, *(DWORD*)GetIconObj, typeIdStr, param, 0, 0);
	char *posYstr = aero::generic_this_call<char *>(GetIcon, *(DWORD*)GetIconObj, typeIdStr, param, 0, 1);
	x = posXstr ? atoi(posXstr) : 0;
	y = posYstr ? atoi(posYstr) : 0;
}

uint32_t GetAgentAgileDataHashKey(int agent_id) {
	return aero::generic_fast_call<uint32_t>(Offset(AGILEDATA_HASHKEY_GET), &agent_id);
}

war3::AGILE_TYPE_DATA* GetAgentAgileTypeData(int agent_id) {
	war3::AGILE_TYPE_DATA* rv = NULL;
	void* hash_table = aero::pointer_calc<void*>(*AgileData, 0xC);
	if (hash_table) {
		uint32_t hash_key = GetAgentAgileDataHashKey(agent_id);
		rv = aero::generic_this_call<war3::AGILE_TYPE_DATA*>(Offset(AGILETYPEDATA_NODE_GET), hash_table, hash_key, &agent_id);
	}
	return rv;
}

static std::map<std::pair<int, int>, bool> AgentAncestorData;
bool AgentHasAncestor(int agent_id, int ancestor_id) {
	if (agent_id == ancestor_id)
		return true;

	std::pair<int, int> dataKey(agent_id, ancestor_id);

	if (AgentAncestorData.count(dataKey))
	{
		return AgentAncestorData[dataKey];
	}

	war3::AGILE_TYPE_DATA* data = NULL;
	while (true) {
		data = GetAgentAgileTypeData(agent_id);
		if (data && agent_id != data->parent_id) {
			if (data->parent_id == ancestor_id)
			{
				AgentAncestorData[dataKey] = true;
				return true;
			}
			agent_id = data->parent_id;
		} else
			break;
	}
	AgentAncestorData[dataKey] = false;
	return false;
}

void DumpAgentAncestors(int agent_id) {
#ifndef _VMP
	OutputDebug("================================================================================\n");
	OutputDebug(" Ancestors of %s:\n", IntegerIdToChar(agent_id));
	OutputDebug("--------------------------------------------------------------------------------\n");
	int t = agent_id;
	war3::AGILE_TYPE_DATA* data = NULL;
	while (true) {
		data = GetAgentAgileTypeData(t);
		if (data && t != data->parent_id) {
			OutputDebug(" %s\n", IntegerIdToChar(data->parent_id));
			t = data->parent_id;
		} else
			break;
	}
	OutputDebug("================================================================================\n");
#endif
}

bool IsAbilityBuff (war3::CAbility* ability)
{
	return AgentHasAncestor(AgentTypeIdGet((war3::CAgent*)ability), 'buff');
}

bool IsAbilitySpell(war3::CAbility* ability)//FIXME
{
	return AgentHasAncestor(AgentTypeIdGet((war3::CAgent*)ability), 'AAsp');
}

float AbilityManaCostGet(war3::CAbility* ability) {
	float fv = 0;
	//if (ability->flag2 & 0x80000000) return 0;

	if (IsAbilitySpell(ability)) {
		aero::generic_this_call<float*>(VTBL(ability)[0x374 / 4], ability, &fv);
	}
	return fv;
}

float AbilityCooldownGet(war3::CAbility* ability) {
	float fv = 0;
	//FIXME
	if (ability) {
		aero::generic_this_call<void>(VTBL(ability)[0x2EC / 4], ability, &fv, ability->level);
	}
	return fv;
}

bool IsTechtreeAvailable(int playerId, int techId)
{
	war3::CPlayerWar3* player = PlayerObject(playerId);
	if (player)
	{
		uint32_t *techData;
		techData = aero::generic_this_call<uint32_t *>(PlayerTechtreeGet, player->techtree, techId, 1);
		if (techData)
		{
			return (*(techData + 0x18/4) != 0);
		}
	}
	return false;
}

bool IsCommandButton(war3::CSimpleButton* btn) {
	return VTBL(btn) == CCommandButtonVtable;
}

//�ο� 6F02E640
war3::AbilityDefData* AbilityDefDataGet(int ability_id) {
	return aero::generic_fast_call<war3::AbilityDefData*>(AbilityDefDataGetByAID, ability_id);
}

war3::AbilityDefData* AbilityDefDataGet(war3::CAbility* ability) {
	war3::AbilityDefData* rv = NULL;
	if (ability) {
		if (ability->defData) {
			rv = ability->defData;
		} else {
			rv = AbilityDefDataGet(ability->id);
		}
	}
	return rv;
}


war3::AbilityUIDef* AbilityUIDefGet(int ability_id) {
	return aero::generic_fast_call<war3::AbilityUIDef*>(AbilityUIDefGetByAID, ability_id);
}

war3::AbilityUIDef* AbilityUIDefGet(war3::CAbility* ability) {
	war3::AbilityUIDef* rv = NULL;
	if (ability) {
		war3::AbilityDefData* data = AbilityDefDataGet(ability);
		if (data && data->uiDefAvailable) {
			rv = data->abilityUIDef;
		} else {
			rv = AbilityUIDefGet(ability->id);
		}
	}
	return rv;
}

war3::JassThreadLocal* GetJassThreadLocal() {
	return (war3::JassThreadLocal*)GameTlsDataGet(5);
}

war3::NativeFunc* CreateNativeFuncNode(const char* func) {
	war3::NativeFuncAllocatorHashTable* hashtable = &(GetJassThreadLocal()->hashtableNativeFunc);
	return aero::generic_this_call<war3::NativeFunc*>(NativeFuncCreate, hashtable, Storm::StringGetHash(func), 0, 0);
}

war3::NativeFunc* GetNativeFuncNode(const char* func) {
	war3::NativeFuncAllocatorHashTable* hashtable = &(GetJassThreadLocal()->hashtableNativeFunc);
	return aero::generic_this_call<war3::NativeFunc*>(NativeFuncGet, hashtable, func);
}

const char* GetJassReturedString(uint32_t id) {
	uint32_t offset = (*GameGlobalObject)->jassStringId;
	war3::JassThreadLocal* jtl = GetJassThreadLocal();
	void* data = jtl->stringArr[offset];
	return RCString::getString(aero::generic_this_call<war3::RCString*>(JassStringRvToRCString, data, id));
}

war3::UnitDataNode* GetUnitDataNode(uint32_t type_id) {
	uint32_t agile_hash_key = GetAgentAgileDataHashKey(type_id);
	return aero::generic_this_call<war3::UnitDataNode*>(UnitDataNodeGet, UnitDataHashTable, agile_hash_key, &type_id);
}

war3::ItemDataNode* GetItemDataNode(uint32_t type_id) {
	uint32_t agile_hash_key = GetAgentAgileDataHashKey(type_id);
	return aero::generic_this_call<war3::ItemDataNode*>(ItemDataNodeGet, ItemDataHashTable, agile_hash_key, &type_id);
}

void Tools_Init () {
	VMProtectBeginMutation("Tools Init");

	AgentAncestorData.clear();
	GameGlobalUI = Offset(GLOBAL_UI);
	GameUnitArrayGet = Offset(UNIT_ARRAY_GET);
	GameGlobalObject = (war3::CGameWar3 **)(Offset(GLOBAL_WARCRAFT_GAME));
	GameStateGet = Offset(STATE_GET);
	GameObjectFromHash = Offset(OBJECT_FROM_HASH);
	GameHandleFromObject = Offset(HANDLE_FROM_OBJECT);
	GameTimeGet = Offset(GAME_TIME_ELAPSED_GET);
	GameUnitGetObject = Offset(UNIT_FROM_HANDLE);
	GameItemGetObject = Offset(ITEM_FROM_HANDLE);
	GameAbilityFromUnit = Offset(ABILITY_FROM_UNIT);
	GameUIToggle = Offset(UI_TOGGLE);
	GameUITextDisplay = (Prototype_GameUITextDisplay)Offset(UI_TEXT_DISPLAY);
	GameMissileSpeedGet = Offset(MISSILE_SPEED_GET);
	GameMapInfo = reinterpret_cast<war3::MapInfo *>(Offset(GLOBAL_MAP_INFO));
	GameProfileInt = Offset(PROFILE_GET_INT);
	GameProfileFloat = Offset(PROFILE_GET_FLOAT);
	GameUnitSelectionPrioGet = Offset(SELECTION_PRIO_GET);
	GameSoundPlay = Offset(SOUND_PLAY);
	CGxDevice = reinterpret_cast<war3::CGxDevice**>(Offset(GLOBAL_CGXDEVICE));
	DefaultUnitTagGet = Offset(WAR3_UNITTIP_GET);
	GlobalGlueMgr = Offset(GLOBAL_GLUEMGR);
	InputObserverGetCall = Offset(INPUTOBSERVER_GET);
	GlobalPreselectUIData = Offset(GLOBAL_PRESELECTUIDATA);
	ObjectUnderCursor = Offset(GLOBAL_OBJECT_UNDER_CURSOR);
	PlayerNameGetCall = Offset(PLAYER_NAME_GET);
	ObjectNameGetCall = Offset(OBJECT_NAME_GET);
	GlobalGameTlsIndex = reinterpret_cast<DWORD*>(Offset(GLOBAL_TLS_INDEX));
	GetIcon = Offset(READFUNCPROFILE);
	GetIconObj = Offset(FUNCPROFILEOBJ);
	//AbilitySpellCheck = Offset(ABILITY_SPELL_CHECK);
	AgentTimerVTable = Offset(AGENTTIMER_VTABLE);
	PlayerTechtreeGet = Offset(PLAYER_TECHTREE_GET);
	AgileData = (void**)Offset(GLOBAL_AGILE_DATA);
	CCommandButtonVtable = (void**)Offset(COMMANDBUTTON_VTABLE);
	AbilityDefDataGetByAID = Offset(ABILITYDEFDATA_GET);
	AbilityUIDefGetByAID = Offset(ABILITYUIDEF_GET);
	SelectionSubgroupSetFunc = Offset(SELECTION_SUBGROUP_SET);
	SelectionSyncFunc = Offset(SELECTION_SYNC);
	NativeFuncGet = Offset(NATIVEFUNC_NODE_GET);
	NativeFuncCreate = Offset(NATIVEFUNC_NODE_CREATE);
	JassStringRvToRCString = Offset(JASS_STRING_RV_TO_RCSTRING);
	UnitDataHashTable = Offset(GLOBAL_UNITDATAHASHTABLE);
	UnitDataNodeGet = Offset(UNITDATA_NODE_GET);
	ItemDataHashTable = Offset(GLOBAL_ITEMDATAHASHTABLE);
	ItemDataNodeGet = Offset(ITEMDATA_NODE_GET);

	VMProtectEnd();
}

