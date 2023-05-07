#include "stdafx.h"
#include "Profile.h"
#include "PreselectUIBind.h"
#include "Tools.h"
#include "UI.h"
#include "Unit.h"
#include "Game.h"
#include "Event.h"
#include "Timer.h"
#include "ObjectPool.h"
#include "UISimpleTexture.h"
#include "UnitStateTag.h"
#include "OptionMenu.h"
#include "../DreamAuth2/StringManager.h"
#include <VMP.h>

namespace PreselectUIBind {
	ObjectPool<war3::CStatBar>* StatBarPool = NULL;
	ObjectPool<UnitStateTag>* UnitStateTagPool = NULL;

	PreselectUIBindRefreshObject::PreselectUIBindRefreshObject() {
		
	}

	PreselectUIBindRefreshObject::~PreselectUIBindRefreshObject() {
		
	}

	static FramePoint UnitFramePoint;
	static war3::PreselectUIData* preselectUIData = NULL;
	static uint32_t Marker = 0;

	//����
	static bool ManaBarEnabled;
	static bool StateTagEnabled;
	static bool StateTagShowForHero;
	static bool StateTagShowForNonHero;

	#define MANABARCOLOR 0xFF2060FF	//0xFF0040FF

	inline bool UnitNeedBindManabar(uint32_t unitHandle) {
		return (
				ManaBarEnabled
			&&	Jass::GetUnitState(unitHandle, Jass::UNIT_STATE_MAX_MANA) > 0	//TODO �Ż�
			);
	}

	inline bool UnitNeedBindStateTag(uint32_t unitHandle) {
		bool rv = StateTagEnabled;
		if (rv){
			if (Jass::IsUnitType(unitHandle, Jass::UNIT_TYPE_HERO)) rv &= StateTagShowForHero;
			else rv &= StateTagShowForNonHero;
		}
		return rv;
	}

	void PreselectUIBindRefreshObject::refresh() {
		war3::CPreselectUI** puiArray = preselectUIData->visiblePreselectUIArray.objectArray;
		war3::CPreselectUI* pui;
		war3::CUnit* unit;
		PreselectUIBindInfo* info;
		war3::CStatBar* bar;
		war3::CStatBar* hpBar;
		UnitStateTag* stateTag;
		uint32_t unitHandle;
		std::map<war3::CUnit*, PreselectUIBindInfo>::iterator iter;

		//TODO �Ż�scale
		float scale = (1.2217305f / Jass::GetCameraField(Jass::CAMERA_FIELD_FIELD_OF_VIEW))
			* (1650.f / Jass::GetCameraField(Jass::CAMERA_FIELD_TARGET_DISTANCE));

		//��Ǵ������а�����ĵ�λ
		for (uint32_t i = 0; i < preselectUIData->visiblePreselectUIArray.currentCount; ++i) {
			pui = puiArray[i];
			if (pui->unit && pui->statBarHP && pui->statBarHP->baseSimpleStatusBar.baseSimpleFrame.visible) { //���hpBar�ǿɼ���
				unit = pui->unit;
				unitHandle = ObjectToHandle(unit);
				//��������Ѫ��
				if ((hpBar = pui->statBarHP) != NULL)
				{
					hpBar->baseSimpleStatusBar.baseSimpleFrame.baseLayoutFrame.scale = scale;
				}

				bool infoExists = this->unitBindMap_.count(unit) > 0;
				bool bindManabar = UnitNeedBindManabar(unitHandle);
				bool bindStateTag = UnitNeedBindStateTag(unitHandle);

				if (bindManabar || bindStateTag) {
					info = &(this->unitBindMap_[unit]);
					info->marker = Marker;

					//�������״̬
					if (bindManabar)
						info->marker_manaBar = Marker;
					if (bindStateTag)
						info->marker_stateTag = Marker;

					if (!infoExists) { //֮ǰ�����ڣ���Ӷ��� 
						if (bindManabar) {
							info->manaBar = bar = StatBarPool->get();
							StatBar::setOwner(bar, unit);
							UISimpleRegion(bar->baseSimpleStatusBar.texture).setColor(MANABARCOLOR);
							//OutputDebug("[IMPORTANT]Manabar Added");
						} else { //�������Ҫ����ΪNULL
							info->manaBar = NULL;
						}

						if (bindStateTag) {
							info->stateTag = stateTag = UnitStateTagPool->get();
							stateTag->setOwner(unitHandle);
							//OutputDebug("[IMPORTANT]StateTag Added");
						} else { //�������Ҫ����ΪNULL
							info->stateTag = NULL;
						}
						//OutputDebug("[IMPORTANT]Info Added, total = %u", this->unitBindMap_.size());
					}
				}
			}
		}
		//����δ��ǵĵ�λ��ˢ�±�ǵ�
		for (std::map<war3::CUnit*, PreselectUIBindInfo>::iterator iter = this->unitBindMap_.begin();
			iter != this->unitBindMap_.end();) 
		{
			unit = iter->first;
			unitHandle = ObjectToHandle(unit);
			info = &iter->second;

			bar = info->manaBar;
			stateTag = info->stateTag;

			//���ʲô����û�У�ֱ������
			if (info->marker != Marker) {
				if (bar) {
					StatBar::setOwner(bar, NULL);
					SimpleFrame::hide((war3::CSimpleFrame*)(bar));
					StatBarPool->ret(bar);
					//OutputDebug("[IMPORTANT]Manabar Removed");
				}

				if(stateTag) {
					stateTag->setOwner(NULL);
					stateTag->hide();
					UnitStateTagPool->ret(stateTag);
					//OutputDebug("[IMPORTANT]StateTag Removed");
				}
				iter = this->unitBindMap_.erase(iter);
				//OutputDebug("[IMPORTANT]Info Removed. Total = %u", this->unitBindMap_.size());
				continue;
			}

			
			if (info->marker_manaBar == Marker) { //ˢ��Manabar
				if (!bar) { //���ر仯
					info->manaBar = bar = StatBarPool->get();
					StatBar::setOwner(bar, unit);
					UISimpleRegion(bar->baseSimpleStatusBar.texture).setColor(
						MANABARCOLOR
					);
					//OutputDebug("[IMPORTANT]Manabar Added");
				}
				hpBar = unit->preSelectUI->statBarHP;
				LayoutFrame::setWidth(bar, hpBar->baseSimpleStatusBar.baseSimpleFrame.baseLayoutFrame.width);
				LayoutFrame::setHeight(bar, hpBar->baseSimpleStatusBar.baseSimpleFrame.baseLayoutFrame.height);

				//����ΪѪ����һ���
				bar->baseSimpleStatusBar.baseSimpleFrame.baseLayoutFrame.height = hpBar->baseSimpleStatusBar.baseSimpleFrame.baseLayoutFrame.height/2;

				bar->baseSimpleStatusBar.baseSimpleFrame.baseLayoutFrame.scale = scale;//MUSTDO �����
				LayoutFrame::setRelativePosition(
					bar, LayoutFrame::Position::TOP_CENTER,
					hpBar, LayoutFrame::Position::BOTTOM_CENTER,
					0.0f,
					-0.001f
				);

				SimpleFrame::show((war3::CSimpleFrame*)bar);
			} else if (bar) { //������ҪManabar�ҵ�ǰ��Manabar����
				StatBar::setOwner(bar, NULL);
				SimpleFrame::hide((war3::CSimpleFrame*)(bar));
				StatBarPool->ret(bar);
				info->manaBar = NULL;
				//OutputDebug("[IMPORTANT]Manabar Removed"); 
			}
			
			if (info->marker_stateTag == Marker) { //ˢ��StateTag
				if (!stateTag) {//���ر仯
					info->stateTag = stateTag = UnitStateTagPool->get();
					stateTag->setOwner(unitHandle);
					//OutputDebug("[IMPORTANT]StateTag Added");
				}
				stateTag->update();
			} else if (stateTag) {
				stateTag->setOwner(NULL);
				stateTag->hide();
				UnitStateTagPool->ret(stateTag);
				info->stateTag = NULL;
				//OutputDebug("[IMPORTANT]StateTag Removed");				
			}

			++iter;
		}
		++ Marker;
	}

	bool PreselectUIBindRefreshObject::isCompleted() {
		return false;
	}

	void StatBarInitFunc(war3::CStatBar* obj) {
		StatBar::init(obj);
		SimpleFrame::hide((war3::CSimpleFrame*)obj);
	}

	void StatBarDestructFunc(war3::CStatBar* obj) {
		StatBar::destroy(obj);
	}

	static CheckBox*		CbManaBarEnabled;
	static Label*			LbManaBarEnabled;

	//TODO
	static CheckBox*		CbStateTagEnabled;
	static Label*			LbStateTagEnabled;

	static CheckBox*		CbStateTagHero;
	static Label*			LbStateTagHero;

	static CheckBox*		CbStateTagNonHero;
	static Label*			LbStateTagNonHero;

	void Dependency(CheckBox* cb, bool checked){
		CbStateTagHero->activate(checked);
		LbStateTagHero->activate(checked);
		CbStateTagNonHero->activate(checked);
		LbStateTagNonHero->activate(checked);
	}

	void CreateMenuContent(){
		VMProtectBeginVirtualization("PreselectUIBind_MenuContent");

		UISimpleFrame* Panel = DefaultOptionMenuGet()->category(StringManager::GetString(STR::UIENHANCEMENT_CATEGORYNAME), NULL);
		CbManaBarEnabled = new CheckBox(
			Panel, 
			0.024f, 
			NULL,
			&ManaBarEnabled, 
			"ManaBar", 
			"Enable", 
			true);
		CbManaBarEnabled->setRelativePosition(
			POS_UL,
			Panel,
			POS_UL,
			0.03f, -0.03f);
		LbManaBarEnabled = new Label(Panel, StringManager::GetString(STR::MANABAR_ENABLE), 0.013f);
		LbManaBarEnabled->setRelativePosition(
			POS_L, 
			CbManaBarEnabled, 
			POS_R,
			0.01f, 0);

		//state tag enable
		CbStateTagEnabled = new CheckBox(
			Panel, 
			0.024f, 
			Dependency,
			&StateTagEnabled, 
			"StateTag", 
			"Enable", 
			true );
		CbStateTagEnabled->setRelativePosition(
			POS_UL,
			Panel,
			POS_UL,
			0.03f, -0.057f);
		LbStateTagEnabled = new Label(Panel, StringManager::GetString(STR::STATETAG_ENABLE), 0.013f);
		LbStateTagEnabled->setRelativePosition(
			POS_L, 
			CbStateTagEnabled, 
			POS_R,
			0.01f, 0);

		//state tag enable hero
		CbStateTagHero = new CheckBox(
			Panel, 0.024f, NULL, &StateTagShowForHero, "StateTag", "EnableForHero", true );
		CbStateTagHero->setRelativePosition(
			POS_UL,	Panel,	POS_UL,	0.03f + Panel->width()*0.3f, -0.057f);
		LbStateTagHero = new Label(Panel, StringManager::GetString(STR::STATETAG_HERO), 0.013f);
		LbStateTagHero->setRelativePosition(
			POS_L, CbStateTagHero, POS_R, 0.01f, 0);

		//state tag enable non hero
		CbStateTagNonHero = new CheckBox(
			Panel, 0.024f, NULL, &StateTagShowForNonHero, "StateTag", "EnableForNonHero", false );
		CbStateTagNonHero->setRelativePosition(
			POS_UL,	Panel, POS_UL, 0.03f + Panel->width()*0.6f, -0.057f);
		LbStateTagNonHero = new Label(Panel, StringManager::GetString(STR::STATETAG_NONHERO), 0.013f);
		LbStateTagNonHero->setRelativePosition(
			POS_L, CbStateTagNonHero, POS_R, 0.01f, 0);

		VMProtectEnd();
	}

	void Init() {
		VMProtectBeginVirtualization("PreSelectUIInit");

		//UI
		CreateMenuContent();

		preselectUIData = PreselectUIDataGet();
		StatBarPool = new ObjectPool<war3::CStatBar>(24, StatBarInitFunc, StatBarDestructFunc);
		UnitStateTagPool = new ObjectPool<UnitStateTag>(12);
		RefreshManager_AddObject(new PreselectUIBindRefreshObject());

		VMProtectEnd();
	}

	void Cleanup() {
		delete UnitStateTagPool;
		delete StatBarPool;
	}
}