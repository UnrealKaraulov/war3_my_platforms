#include "stdafx.h"
#include "UISimpleFontString.h"
#include "UILayoutFrame.h"
#include "ObjectPool.h"
#include "Storm.h"
#include "Tools.h"

ObjectPool<UISimpleFontString> SimpleFontStringPool(32);

UISimpleFontString* UISimpleFontString::Create(UILayoutFrame* parent) {
	UISimpleFontString* rv = SimpleFontStringPool.get();
	war3::CSimpleFontString* baseObj = Storm::MemAllocStruct<war3::CSimpleFontString>();
	aero::generic_this_call<void>(
		Offset(SIMPLEFONTSTRING_CONSTRUCT), 
		baseObj,
		parent ? parent->base() : NULL,
		2,				//TODO ����ʲô
		1				//TODO ����ʲô
	);
	rv->setBase(baseObj);
	return rv;
}

UISimpleFontString* UISimpleFontString::Create(
	UILayoutFrame* parent,
	const char* text,
	float size
){
	UISimpleFontString* sfs = UISimpleFontString::Create(parent);
	sfs->initFont(Skin::getPathByName("MasterFont"), size, 0);
	sfs->setText(text);
	sfs->setColorFloat(1, 0.8f, 0, 1);
	return sfs;
}

void UISimpleFontString::Destroy(UISimpleFontString* obj) {
	war3::CSimpleFontString* baseObj = obj->base<war3::CSimpleFontString*>();
	if (baseObj) {
		if (GameUIObjectGet()) {
			aero::generic_this_call<void>(
				VTBL(baseObj)[2],
				baseObj,
				1
			);
		}
	}
	SimpleFontStringPool.ret(obj);
}

void UISimpleFontString::setText(const char* text) {
	war3::CSimpleFontString* baseObj = this->base<war3::CSimpleFontString*>();
	if (baseObj) {
		aero::generic_this_call<uint32_t>(
			Offset(SIMPLEFONTSTRING_SETTEXT), 
			baseObj,
			text
		);
	}
}

void UISimpleFontString::initFont(const char* path, float size, uint32_t useShadow) {
	war3::CSimpleFontString* baseObj = this->base<war3::CSimpleFontString*>();
	if (baseObj) {
		aero::generic_this_call<uint32_t>(
			Offset(SIMPLEFONTSTRING_INITFONT), 
			baseObj,
			path,
			size,
			useShadow
		);

		aero::generic_this_call<uint32_t>(
			Offset(SIMPLEFONTSTRING_INITTEXT), 
			baseObj,
			1		//TODO ����ʲô
		);
	}
}

float UISimpleFontString::getTextWidth() {
	float rv = -1.0f;
	war3::CSimpleFontString* baseObj = this->base<war3::CSimpleFontString*>();
	if (baseObj) {
		rv = aero::generic_this_call<float>(
			Offset(SIMPLEFONTSTRING_GETTEXTWIDTH), 
			baseObj
		);
	}
	return rv;
}