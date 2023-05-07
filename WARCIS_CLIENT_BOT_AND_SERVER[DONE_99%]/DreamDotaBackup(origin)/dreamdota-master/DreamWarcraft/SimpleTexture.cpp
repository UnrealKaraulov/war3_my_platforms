#include "stdafx.h"
#include "SimpleTexture.h"

#include "FunctionTemplate.h"
#include "Offsets.h"
#include "Tools.h"
#include "Storm.h"

namespace SimpleTexture {
	war3::CSimpleTexture* create(void* parent, uint32_t arg, uint32_t flag) {
		war3::CSimpleTexture* t = Storm::MemAllocStruct<war3::CSimpleTexture>();
		aero::generic_this_call<void>(
			Offset(SIMPLETEXTURE_CONSTRUCT), 
			t,
			parent,
			arg,
			flag
		);
		return t;
	}

	void destory(war3::CSimpleTexture* t) {
		if (GameUIObjectGet()) {
			aero::generic_this_call<void>(
				VTBL(t)[2],
				t,
				1
			);
		}
	}

	uint32_t setTexturePath(war3::CSimpleTexture* t, const char* path, uint32_t flag) {
		return aero::generic_this_call<uint32_t>(
			Offset(SIMPLETEXTURE_FILLBITMAP), 
			t,
			path,
			flag
		);
	}
}