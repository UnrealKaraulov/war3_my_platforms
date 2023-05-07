#include "stdafx.h"
#include "SimpleFrame.h"
#include "FunctionTemplate.h"
#include "Tools.h"
#include "Storm.h"

namespace SimpleFrame {
	war3::CSimpleFrame* create(void* parent) {
		war3::CSimpleFrame* t =  Storm::MemAllocStruct<war3::CSimpleFrame>();
		return aero::generic_this_call<war3::CSimpleFrame*>(
			Offset(SIMPLEFRAME_CONSTRUCT),
			t, 
			parent ? parent : GameUIObjectGet()->simpleConsole
		);
	}

	void destory(war3::CSimpleFrame* t) {
		if (GameUIObjectGet()) {
			aero::generic_this_call<void>(
				VTBL(t)[2],
				t,
				1
			);
		}
	}

	war3::SimpleFrameTextureSettings* initTextureSettings(war3::CSimpleFrame* t) {
		war3::SimpleFrameTextureSettings* rv = t->textureSettings =  Storm::MemAllocStruct<war3::SimpleFrameTextureSettings>();
		return aero::generic_fast_call<war3::SimpleFrameTextureSettings*>(
			Offset(SIMPLEFRAME_INITTEXTURESETTINGS),
			rv
		);
	}

	uint32_t applyTextureSetting(war3::CSimpleFrame* t) {
		return aero::generic_this_call<uint32_t>(
			Offset(SIMPLEFRAME_APPLYTEXTURESETTINGS),
			t,
			t->textureSettings
		);
	}

	void show(war3::CSimpleFrame* t) {
		t->visible = 1;
		aero::generic_this_call<void>(vtableFpAtOffset(VTBL(t), 0x68), t);
	}

	void hide(war3::CSimpleFrame* t) {
		t->visible = 0;
		aero::generic_this_call<void>(vtableFpAtOffset(VTBL(t), 0x64), t);
	}
}