#ifndef ABILITY_H_
#define ABILITY_H_
class Unit;

class Ability {
public:
	Ability* nextNode ( ) const;			//��һ�����ܣ�û��ΪNULL
	uint32_t		typeId( ) const;			//����ID
	const char*		typeIdChar ( ) const;
	const char*		name ( ) const;				//����
	uint32_t		baseTypeId( ) const;		//ģ�弼��ID
	const char*		className( ) const;			//��������, TODO�����о�6f028f40

	uint32_t		level( ) const;				//�ȼ� ��0��ʼ
	float			cooldown( ) const;			//��ȴʱ��
	float			cooldownRemain( ) const;	//ʣ����ȴʱ��
	bool			isAvailable( ) const;		//�Ƿ����
	const Unit*		owner( ) const;				//������
	const char*		iconPath( ) const;			//ͼ��
	uint32_t		flag ( ) const;
	bool			isSpell () const;
	float			mana () const;				//��ħ
	int				iconPositionValue( ) const;	//ͼ���������ȼ�, TODO
	bool			isSpellBook ( ) const;
	int getSpellBookSpells( std::set<uint32_t> *pSpellIdSet );
	uint32_t		order ( ) const;			//����

	const char*		tooltip(int level = -1) const;
	const char*		description(int level = -1) const;
	uint32_t		hotkey(int level = -1) const;


};

inline Ability* GetAbility(void* obj) {
	return (Ability*)obj;
}

/*
	0x1
	0x2
	0x4
	0x8
	0x10		ability
	0x20		item
	0x40
	0x80		buff?
	0x100		unuseable?
	0x200		in cd
	0x400		using?
	0x800
	0x1000
	0x2000		active???
	0x4000
	0x8000
	0x10000
	0x20000
	0x40000
	0x80000
	0x100000
	0x200000
	0x400000
	0x800000
	0x1000000
	0x2000000
	0x4000000
	0x8000000
	0x10000000
	0x20000000
	0x40000000
	0x80000000	passive ?
*/

#endif