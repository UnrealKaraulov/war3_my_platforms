#ifndef STATE_PANEL_H_
#define STATE_PANEL_H_

#include "RefreshManager.h"
#include "Frame.h"
#include "Tools.h"

class Unit;
class Item;
class Texture;
class Label;
class UnitStateIcon;

/*
��ʾ�����(��λ�����ƻ�����Ʒ��)ͷ�ϵ�UI�ؼ�
�������ɼ�ʱ��

���ͣ� ��λ��Ӣ��(��boss)����Ʒ

��λ��ʾͷ��
Ӣ����ʾ�������ͷ�񣬵ȼ���Ѫ����ħ�������ܣ���Ʒ��buff
��Ʒ��ʾͷ�������
*/

class StatePanel : public Frame
{
public:
	void *object;

	bool bShow;
	bool bDeleteme;
	FramePoint point;

	StatePanel(void *obj);
	virtual ~StatePanel();

	virtual bool update();
};

class StatePanelUnit : public StatePanel
{
private:
	Texture *icon;

public:
	uint32_t typeId;

	StatePanelUnit(Unit *bindUnit);
	virtual ~StatePanelUnit();

	virtual bool update();

	static bool enabled;
};

class StatePanelWard : public StatePanelUnit
{
public:
	StatePanelWard(Unit *bindWard);
	virtual ~StatePanelWard();

	virtual bool update();
};

class StatePanelItem : public StatePanel
{
	Texture *icon;
	Label *name;

public:

	StatePanelItem(Item *bindItem);
	virtual ~StatePanelItem();

	virtual bool update();

	static bool enabled;
};

class StatePanelHero : public StatePanel
{
	UnitStateIcon *icon;
	Label *name;

public:
	int level;

	StatePanelHero(Unit *bindHero);
	virtual ~StatePanelHero();

	virtual bool needUpdate();
	virtual bool update();

	static bool enabled;
};

class StatePanelBoss : public StatePanelHero
{
public:
	StatePanelBoss(Unit *boss);
	virtual ~StatePanelBoss();

	virtual bool needUpdate();

	static bool enabled;
};


class StatePanelRefreshObject : public RefreshObject
{
public:
	StatePanelRefreshObject ();

	virtual void refresh ();
	virtual bool isCompleted ();
};

void StatePanelInit();
void StatePanelCleanup();

#endif