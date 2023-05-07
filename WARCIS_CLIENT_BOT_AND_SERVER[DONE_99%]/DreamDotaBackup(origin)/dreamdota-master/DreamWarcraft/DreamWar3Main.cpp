#include "stdafx.h"
#include "DreamWar3Main.h"
#include "PreselectUIBind.h"

void DreamWar3_Init ( ) {

#ifdef _DREAMWAR3
	PreselectUIBind::Init();
	KeyRepeat::Init();//�����ظ�
	DynamicHotkey::Init();//��̬��Ʒ�ȼ�
	SlotHotkey::Init();//��λ�ļ�
	QuickDropItem::Init();//��ݴ�����Ʒ
	CameraToEnemyHero::Init();//�л���ͷ������Ӣ��
	ControlGroup::Init();//����Ż�
	Surrounding::Init();//Χɱ
	ImprovedAutoCast::Init();//ctrl + �����Զ�ʩ��������ǿ�ƿ��ز�����
	EnemyAlert::Init();//���˵�λ������Ұ����
	Formation::Init();//����
	FastPullToBase::Init();//��������λ�ػ���
	FastDropUnitFromTransport::Init();//���ؾߵ�λ����ж�ص�λ
	OptAttack::Init();//�Զ��Ż�����
	InformationPanel::Init();
	ShowLatency::Init();
	ReplayMode::Init();
	ScorePanel::Init();
	Blocking::Init();//��λ
#endif

#ifdef _DREAMDOTA
	PreselectUIBind::Init();
	KeyRepeat::Init();//�����ظ�
	DynamicHotkey::Init();//��̬��Ʒ�ȼ�
	SlotHotkey::Init();//��λ�ļ�
	QuickDropItem::Init();//��ݴ�����Ʒ
	InformationPanel::Init();
	ShowLatency::Init();
	//ReplayMode::Init();
	ScorePanel::Init();
	CameraToEnemyHero::Init();//�л���ͷ������Ӣ��
#endif

#ifdef _FREEPLUGIN
	PreselectUIBind::Init();
	KeyRepeat::Init();//�����ظ�
	DynamicHotkey::Init();//��̬��Ʒ�ȼ�
	SlotHotkey::Init();//��λ�ļ�
	InformationPanel::Init();
	ShowLatency::Init();
	ReplayMode::Init();
	ScorePanel::Init();
#endif

#ifdef _GLEAGUE
	GLeagueDota::Init();
#endif
	
}

void DreamWar3_Cleanup() {

#ifdef _DREAMWAR3
	Blocking::Cleanup();//��λ
	ScorePanel::Cleanup();
	ReplayMode::Cleanup();
	ShowLatency::Cleanup();
	InformationPanel::Cleanup();
	OptAttack::Cleanup();
	FastDropUnitFromTransport::Cleanup();
	FastPullToBase::Cleanup();
	SlotHotkey::Cleanup();
	DynamicHotkey::Cleanup();
	PreselectUIBind::Cleanup();
#endif

#ifdef _DREAMDOTA	//DreamDota ������
	CameraToEnemyHero::Cleanup();
	ScorePanel::Cleanup();
	ReplayMode::Cleanup();
	ShowLatency::Cleanup();
	InformationPanel::Cleanup();
	SlotHotkey::Cleanup();
	DynamicHotkey::Cleanup();
	PreselectUIBind::Cleanup();
#endif

#ifdef _FREEPLUGIN
	ScorePanel::Cleanup();
	ReplayMode::Cleanup();
	ShowLatency::Cleanup();
	InformationPanel::Cleanup();
	SlotHotkey::Cleanup();
	DynamicHotkey::Cleanup();
	PreselectUIBind::Cleanup();
#endif

#ifdef _GLEAGUE
	GLeagueDota::Cleanup();
#endif

	

}