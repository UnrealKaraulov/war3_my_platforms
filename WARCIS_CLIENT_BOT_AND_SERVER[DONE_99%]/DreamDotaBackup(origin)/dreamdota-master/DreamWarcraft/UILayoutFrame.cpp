#include "stdafx.h"
#include "UILayoutFrame.h"

#include "Offsets.h"
void UILayoutFrame::setAbsolutePosition(uint32_t originPos, float absoluteX, float absoluteY) {
	 aero::generic_this_call<war3::CFramePointAbsolute*>(
		Offset(LAYOUTFRAME_SETABSPOS), 
		this->base<war3::CLayoutFrame*>(this->baseLayoutFrameOffset_), 
		originPos, 
		absoluteX, 
		absoluteY, 
		1			//TODO ����������ʲô
	);
}

void UILayoutFrame::setRelativePosition(uint32_t originPos, UILayoutFrame* target, uint32_t toPos, float relativeX, float relativeY) {
	float x = relativeX / War3WindowWidthRatio();
	aero::generic_this_call<war3::FRAMENODE*>(
		Offset(LAYOUTFRAME_SETRELPOS), 
		this->base<war3::CLayoutFrame*>(this->baseLayoutFrameOffset_), 
		originPos,
		target->baseLayoutFrame(),
		toPos,
		x,//relativeX,
		relativeY,
		1			//TODO ����������ʲô
	);
}

void UILayoutFrame::setRelativeObject(UILayoutFrame* target) {
	aero::generic_this_call<war3::FRAMENODE*>(
		Offset(LAYOUTFRAME_SETRELFRAME),
		this->base<war3::CLayoutFrame*>(this->baseLayoutFrameOffset_),
		target->baseLayoutFrame(),
		1			//TODO ����������ʲô
	);
}

void UILayoutFrame::applyPosition() {
	aero::generic_this_call<uint32_t>(
		Offset(LAYOUTFRAME_UPDATEPOSITION), 
		this->base<war3::CLayoutFrame*>(this->baseLayoutFrameOffset_),
		1			//TODO ����������ʲô
	);
}