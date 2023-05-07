#ifndef UNIT_H_
#define UNIT_H_

#include "Player.h"
#include "Point.h"
#include "Item.h"
#include "Ability.h"
#include "Stochastic.h"
#include "Jass.h"
#include "Action.h"


struct VisibleState;
struct AttackData;
struct DeathRateData;
struct FramePoint;
class UnitFilter;
class Unit;
class UnitGroup;
class DebugPanel;

class UnitFilter {
public:
	//�������� - ��λ����
	static const uint64_t GIANT			= (uint64_t)1 << 0;
	static const uint64_t UNDEAD		= (uint64_t)1 << 1;
	static const uint64_t SUMMONED		= (uint64_t)1 << 2;
	static const uint64_t MECHANICAL	= (uint64_t)1 << 3;
	static const uint64_t PEON			= (uint64_t)1 << 4;
	static const uint64_t SAPPER		= (uint64_t)1 << 5;
	static const uint64_t TOWNHALL		= (uint64_t)1 << 6;
	static const uint64_t ANCIENT		= (uint64_t)1 << 7;
	static const uint64_t NEUTRAL		= (uint64_t)1 << 8;
	static const uint64_t WARD			= (uint64_t)1 << 9;
	static const uint64_t STANDON		= (uint64_t)1 << 10;
	static const uint64_t TAUREN		= (uint64_t)1 << 11;

	//�������� - ��λ
	static const uint64_t HERO				= (uint64_t)1 << 12;
	static const uint64_t DEAD				= (uint64_t)1 << 13;
	static const uint64_t STRUCTURE			= (uint64_t)1 << 14;
	static const uint64_t FLYING			= (uint64_t)1 << 15;
	static const uint64_t GROUND			= (uint64_t)1 << 16;
	static const uint64_t ATTACKS_FLYING	= (uint64_t)1 << 17;
	static const uint64_t ATTACKS_GROUND	= (uint64_t)1 << 18;
	static const uint64_t MELEE_ATTACKER	= (uint64_t)1 << 19;
	static const uint64_t RANGED_ATTACKER	= (uint64_t)1 << 20;

	//״̬����
	static const uint64_t STUNNED		= (uint64_t)1 << 21;
	static const uint64_t PLAGUED		= (uint64_t)1 << 22;
	static const uint64_t SNARED		= (uint64_t)1 << 23;
	static const uint64_t POISONED		= (uint64_t)1 << 24;
	static const uint64_t POLYMORPHED	= (uint64_t)1 << 25;
	static const uint64_t SLEEPING		= (uint64_t)1 << 26;
	static const uint64_t RESISTANT		= (uint64_t)1 << 27;
	static const uint64_t ETHEREAL		= (uint64_t)1 << 28;
	static const uint64_t MAGIC_IMMUNE	= (uint64_t)1 << 29;
	static const uint64_t ILLUSION		= (uint64_t)1 << 30;
	static const uint64_t PAUSED		= (uint64_t)1 << 31;
	static const uint64_t HIDDEN		= (uint64_t)1 << 32;
	static const uint64_t LOADED		= (uint64_t)1 << 33;
	static const uint64_t UNDER_CONSTRUCTION = (uint64_t)1 << 34;
	static const uint64_t UNDER_QUEUE	= (uint64_t)1 << 35;
	static const uint64_t UNDER_UPGRADE	= (uint64_t)1 << 36;
	static const uint64_t INVULNERABLE	= (uint64_t)1 << 37;

	//״̬���� - ������
	static const uint64_t DETECTED		= (uint64_t)1 << 38;
	static const uint64_t FOGGED		= (uint64_t)1 << 39;
	static const uint64_t MASKED		= (uint64_t)1 << 40;
	static const uint64_t INVISIBLE		= (uint64_t)1 << 41;
	static const uint64_t VISIBLE		= (uint64_t)1 << 42;
	static const uint64_t ALLY			= (uint64_t)1 << 43;
	static const uint64_t ENEMY			= (uint64_t)1 << 44;
	static const uint64_t SELECTED		= (uint64_t)1 << 45;
	static const uint64_t CONTROLLABLE	= (uint64_t)1 << 46;
	static const uint64_t FULLCONTROLLABLE	= (uint64_t)1 << 47;

	//TODO ���������������� HAS_MANA, IDLE, CAN_SLEEP, MOVABLE, CONTROLGROUPED, CHANNELING

	UnitFilter (
		uint64_t requireFields, //require1 | require2 | ... | requireN
		uint64_t excludeFields, //exclude1 | exclude2 | ... | excludeN
		int toWhichPlayerId = PLAYER_ANY
	);

	UnitFilter ();
	uint64_t requireFieldsGet() { return require; }
	uint64_t excludeFieldsGet() { return exclude; }
	int toPlayerIdGet() { return player; }
	bool operator==(int rhs);

private:
	uint64_t require;
	uint64_t exclude;
	int player;
};


struct VisibleState {
	bool lastVisible;
	Point lastVisiblePoint;
	VisibleState();
};


class Unit {
	uint32_t handleId_;
	DISALLOW_COPY_AND_ASSIGN(Unit);
public:
	Unit (uint32_t handleId);
	~Unit ();

	uint32_t	handleId() const;//��ȡJass handle id
	const char *name() const;
	const char *debugName(bool showHandleId = true, bool showCUnit = false) const;

	float		acquireRange () const;
	float		acquireRangeDefault () const;
	float		turnSpeed () const;
	float		turnSpeedDefault () const;
	float		propWindow () const;
	float		propWindowDefault () const;
	float		flyHeight () const;
	float		flyHeightDefault () const;
	float		x () const;
	float		y () const;
	Point		position () const;		//����
	float		facing () const;		//ģ�ͳ���, ע���Ⲣ���ǵ�λ��������. ���ؽǶ�
	float		moveSpeed () const;
	float		moveSpeedDefault () const;
	float		life () const;
	float		lifeMax () const;
	float		mana () const;
	float		manaMax () const;
	int			level ( bool hero ) const;
	bool		isInvulnerable () const;
	uint32_t	currentOrder() const;
	Unit*		currentTargetUnit() const;
	Point		currentTargetPoint() const;

	int			owner () const;			//����������, ��0��ʼ
	uint32_t	typeId () const;		//��ȡ��λ����ID, ����'hfoo'
	const char* typeIdChar () const;	//��ȡ��λ����ID, ����"hfoo"
	uint32_t	race () const;			//�ο�Jass::RACE_<X>
	int			foodUsed () const;
	int			foodMade () const;

	int			costGold () const;
	int			costLumber () const;
	int			timeBuild () const;

	bool		isEnemy (int playerSlot) const;
	bool		isEnemyToLocalPlayer () const;
	bool		isAlly (int playerSlot) const;
	bool		isAllyToLocalPlayer () const;
	int			abilityLevel (uint32_t id) const;
	bool		hasItemType(uint32_t id) const;
	bool		isSelectedLocal(bool subgroupOnly = false) const;

	void		cargoUnitGet(UnitGroup *group) const;

	bool		framepointGet(FramePoint* fp) const;

	//��Ӧjass IsUnitType����common.j UNIT_TYPE_<*>����
	bool typeMatch (
		uint32_t reqField,
		uint32_t excludeField
	) const;

	//����λ��ÿ���������Զ����ϣ����Ҷ�ÿ���ų����Զ�������ʱ�����档
	bool filterMatch (
		UnitFilter	filter
	) const;

	//���Ե��������Ƿ����
	bool testFlag (
		uint64_t	inFilterFlag, 
		int			inPlayerId = PLAYER_ANY
	) const;

	uint32_t typeFlag() const;

	float		distanceToUnit	(Unit *otherUnit) const; //����һ����λ��ƽ�����
	float		angleToUnit		(Unit *otherUnit) const; //����һ����λ�ķ��򻡶�
	float		turnTime			(float angleRadian) const;		//ת�����򻡶������ʱ��
	float		turnTimeToDirection	(float directionRadian) const;	//�ӵ�ǰ����ת���򻡶������ʱ��
	float		turnTimeToUnit		(Unit *otherUnit) const;		//�ӵ�ǰ����ת��λ�����ʱ��

	enum DefenseTypeEnum {
		DEFENSE_TYPE_SMALL,			//����
		DEFENSE_TYPE_MEDIUM,		//����
		DEFENSE_TYPE_LARGE,			//����
		DEFENSE_TYPE_FORTIFIED,		//�Ƿ�
		DEFENSE_TYPE_NORMAL,//����
		DEFENSE_TYPE_HERO,			//Ӣ��
		DEFENSE_TYPE_DIVINE,		//��ʥ
		DEFENSE_TYPE_UNARMORED		//�޻���
	};

	float		radius			( ) const; //�뾶, ������̼���, ��Ӧ�༭��collision
	float		direction		( ) const; //��λ��ʵ���򻡶�
	float		priority		( ) const; //ѡ������Ȩ, Ӱ��ѡ��������Ӣ��˳��
	float		regenSpeedLife	( ) const;
	float		defense			( ) const;
	uint32_t	defenseType		( ) const;

	enum AttackTypeEnum {
		ATTACK_TYPE_UNKNOWN,		//����
		ATTACK_TYPE_NORMAL,			//��ͨ
		ATTACK_TYPE_PIERCE,			//����
		ATTACK_TYPE_SIEGE,			//����
		ATTACK_TYPE_MAGIC,			//ħ��
		ATTACK_TYPE_CHAOS,			//����
		ATTACK_TYPE_HERO			//Ӣ��
	};
	enum WeaponTypeEnum {
		WEAPON_TYPE_NONE,			//����
		WEAPON_TYPE_NORMAL,			//��ͨ
		WEAPON_TYPE_MISSILE,		//��ʸ
		WEAPON_TYPE_ARTILLERY,		//����
		WEAPON_TYPE_INSTANT,		//����
		WEAPON_TYPE_MISSILE_SPLASH,	//��ʸ - ����
		WEAPON_TYPE_MISSILE_BOUNCE,	//��ʸ - ����
		WEAPON_TYPE_MISSILE_LINE,	//��ʸ - ֱ��
		WEAPON_TYPE_ARTILLERY_LINE	//���� - ֱ��
	};

	int			weaponTo			(Unit *target) const {return 0;}//TODO

	DnDAtkDist	attack				(int weapon) const;
	DnDAtkDist	attackExpected		(int weapon) const;
	uint32_t	attackType			(int weapon) const;
	uint32_t	weaponType			(int weapon) const;
	Unit*		acquiredTarget		( ) const; //��ǰ��Ҫ������Ŀ�굥λ, �������ж��Զ�����������missʱ������Ŀ��

	float		launchX				( ) const;
	float		launchY				( ) const;

	float		range				(int weapon) const;
	float		missileSpeed		(int weapon) const;
	float		cooldown			(int weapon) const;
	float		cooldownRemain		( ) const;
	float		damagePoint			(int weapon) const;
	float		damagePointRemain	( ) const;
	float		backSwing			(int weapon) const;
	float		backSwingRemain		( ) const;
	float		damageBonusFactor	(int weapon, Unit *target) const;	//���������˺����ʣ���������״̬�罨���С����ޡ�ħ�⣬�������޵�
	//TODO ���㼼��Ӱ�죬��������ף��
	float		damageFactor		(int weapon, Unit *target) const;	//���㹥�����͡�����ֵ���˺����ʣ�����������罨����
	float		damageFactorAsTarget(uint32_t attacktype) const;
	DnDAtkDist	damage				(int weapon, Unit *target) const;
	DnDAtkDist	damageExpected		(int weapon, Unit *target) const;
	float		attackTime			(int weapon, Unit *target, bool countDmgPt, bool faceTarget) const;	//�ӹ��������������ǰҡ + ����ʱ��

	float		constructionPercent	() const;
	float		upgradePercent() const;
	uint32_t	upgradeType () const;
	float		queuePercent () const;
	uint32_t	queueType () const;

	Ability *AbilityListHead ( ) const;
	Ability *AbilityByHotkey ( uint32_t key, bool includeItem = false ) const;
	Ability *AbilityById(uint32_t typeId) const;


	float	abilityCooldownRemain (uint32_t abilityId) const;

	void sendAction (
		uint32_t	actionId,
		ActionType	actionType,
		uint32_t	actionFlag,
		Item		*usingItem,
		Point		targetPoint,
		Unit		*targetUnit,
		bool		flagReduceAPM,
		bool		sendAsProgramm = true
	);

	void sendActionDropItem (
		uint32_t	actionFlag,
		Point		targetPoint,
		Unit		*targetUnit,
		item		dropItemHandle,
		bool		flagReduceAPM,
		bool		sendAsProgramm = true
	);

	void *heroButton ();
#ifdef _DEBUG
	DebugPanel*		debugPanel;
#endif

	VisibleState	visibleState;//alert��¼

	bool lasthitActive;
	Unit *lasthitCurrentTarget;
	std::map<Unit*, Function> lasthitRate; //��Ŀ�굥λ�Ĳ���������
};

int UnitTypeFoodUsed (uint32_t unitTypeId);
int UnitTypeFoodMade (uint32_t unitTypeId);



Unit* GetUnit(uint32_t handle);

void UnitManager_Init();
void UnitManager_Cleanup();

#endif