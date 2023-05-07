#include "stdafx.h"
#include "TextFrame.h"
#include "FunctionTemplate.h"
#include "Offsets.h"
#include "Storm.h"
#include "Tools.h"

namespace TextFrame {
	war3::CTextFrame* create(void* parent, uint32_t a2, uint32_t a3) {
		war3::CTextFrame* rv = Storm::MemAllocStruct<war3::CTextFrame>();
		return aero::generic_this_call<war3::CTextFrame*>(
			Offset(TEXTFRAME_CONSTRUCT),
			rv,
			parent,
			a2,
			a3
		);
	}

	void initFont(war3::CTextFrame* t, const char* fontPath, float fontHeight, uint32_t arg3) {
		aero::generic_this_call<void>(
			Offset(TEXTFRAME_INITFONT),
			t,
			fontPath,
			fontHeight,
			arg3
		);
	}

	void setFont(war3::CTextFrame* t, uint32_t arg1, float a2, float a3) {
		war3::TextFrameFontSettings settings;
		settings.unk_0 = a2;
		settings.unk_4 = a3;
		aero::generic_this_call<void>(
			Offset(TEXTFRAME_SETFONT),
			t,
			arg1,
			&settings
		);
	}

	void setShadow(war3::CTextFrame* t, uint32_t arg1) {
		aero::generic_this_call<void>(
			Offset(TEXTFRAME_SETSHADOW),
			t,
			arg1
		);
	}

	void setColor(war3::CTextFrame* t, uint32_t color) {
		aero::generic_this_call<void>(
			Offset(TEXTFRAME_SETCOLOR),
			t,
			&color
		);
	}

	void setText(war3::CTextFrame* t, const char* text) {
		aero::generic_this_call<void>(
			Offset(TEXTFRAME_SETTEXT),
			t,
			text
		);
	}

	void destroy(war3::CTextFrame* t) {
		if (GlueMgrObjectGet()) {
			aero::generic_this_call<void>(
				VTBL(t)[1],
				t,
				1
			);
		}
	}
}