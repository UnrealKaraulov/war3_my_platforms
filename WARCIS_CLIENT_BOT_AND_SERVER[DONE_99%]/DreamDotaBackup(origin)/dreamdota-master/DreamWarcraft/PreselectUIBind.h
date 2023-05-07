#include "stdafx.h"
#ifndef PRESELECTUIBIND_H_INCLUDED
#define PRESELECTUIBIND_H_INCLUDED

#include "RefreshManager.h"
#include "GameStructs.h"
#include "UIStructs.h"
class Unit;
class UnitStateTag;

namespace PreselectUIBind {
	class PreselectUIBindRefreshObject : public RefreshObject {
	public:
		PreselectUIBindRefreshObject();

		~PreselectUIBindRefreshObject();

		void refresh();

		bool isCompleted();
	private:
		struct PreselectUIBindInfo {
			uint32_t marker;			//��ǵ�λ�Ƿ���preSelectUI

			war3::CStatBar* manaBar;
			uint32_t marker_manaBar;	//��ǵ�λ�Ƿ�Ҫ��ʾ����

			UnitStateTag* stateTag;
			uint32_t marker_stateTag;	//��ǵ�λ�Ƿ���ʾ״̬��Ϣ
		};

		std::map<war3::CUnit*, PreselectUIBindInfo> unitBindMap_;
	};
	void Init();
	void Cleanup();
}

#endif