#include "stdafx.h"
#include "UISimpleTexture.h"
#include "UISimpleFrame.h"
#include "Offsets.h"
#include "Tools.h"
#include "Storm.h"
#include "ObjectPool.h"

ObjectPool<UISimpleTexture> SimpleTexturePool(10);

UISimpleTexture* UISimpleTexture::Create(UISimpleFrame* parent) {
	UISimpleTexture* rv = SimpleTexturePool.get();
	war3::CSimpleTexture* baseObj = Storm::MemAllocStruct<war3::CSimpleTexture>();
	aero::generic_this_call<void>(
		Offset(SIMPLETEXTURE_CONSTRUCT), 
		baseObj,
		parent ? parent->base(): NULL,
		2,				//TODO ����ʲô
		1				//TODO ����ʲô
	);
	rv->setBase(baseObj);
	return rv;
}

void UISimpleTexture::Destroy(UISimpleTexture* obj) {
	if (!obj) return;
	war3::CSimpleTexture* baseObj = obj->base<war3::CSimpleTexture*>();
	if (baseObj) {
		if (GameUIObjectGet()) {
			aero::generic_this_call<void>(
				VTBL(baseObj)[2],
				baseObj,
				1
			);
		}
	}
	SimpleTexturePool.ret(obj);
}

uint32_t UISimpleTexture::fillBitmap(const char* path) {
	uint32_t rv = 0;
	war3::CSimpleTexture* baseObj = this->base<war3::CSimpleTexture*>();
	if (baseObj) {
		rv = aero::generic_this_call<uint32_t>(
			Offset(SIMPLETEXTURE_FILLBITMAP),
			baseObj,
			path,
			0				//����ʲô��
		);
	}
	return rv;
}

uint32_t UISimpleTexture::fillColor(uint32_t color) {
	uint32_t rv = 0;
	war3::CSimpleTexture* baseObj = this->base<war3::CSimpleTexture*>();
	if (baseObj) {
		rv = aero::generic_this_call<uint32_t>(
			Offset(SIMPLETEXTURE_FILLCOLOR),
			baseObj,
			&color
		);
	}
	return rv;
}