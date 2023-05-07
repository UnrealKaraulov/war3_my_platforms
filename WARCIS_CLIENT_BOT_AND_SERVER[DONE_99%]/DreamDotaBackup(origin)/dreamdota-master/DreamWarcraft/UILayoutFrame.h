#include "stdafx.h"
#ifndef UI_LAYOUTFRAME_H_INCLUDED
#define UI_LAYOUTFRAME_H_INCLUDED

#include "UI.h"
#include "UIObject.h"
#include "War3Window.h"

class UILayoutFrame : public UIObject {
public:
	UILayoutFrame() {
		this->baseLayoutFrameOffset_ = 0;
		this->setBase(NULL);		
	}

	UILayoutFrame(void* base, uint32_t offset = 0) {
		this->baseLayoutFrameOffset_ = offset;
		this->setBase(base);
	}

	//--------------------------------------------------------------------------------
	// LayoutFrame
	//--------------------------------------------------------------------------------
	enum Position {
		POSITION_TOP_LEFT		= 0,
		POSITION_TOP_CENTER		= 1,
		POSITION_TOP_RIGHT		= 2,
		POSITION_LEFT			= 3,
		POSITION_CENTER			= 4,
		POSITION_RIGHT			= 5,
		POSITION_BOTTOM_LEFT	= 6,
		POSITION_BOTTOM_CENTER	= 7,
		POSITION_BOTTOM_RIGHT	= 8
	};

	war3::CLayoutFrame* baseLayoutFrame() {return base<war3::CLayoutFrame*>(this->baseLayoutFrameOffset_);}

	//TODO ����ֱ�Ӳ�����ı��С��λ�õ�API

	void setAbsolutePosition(uint32_t originPos, float absoluteX, float absoluteY);
	void setRelativePosition(uint32_t originPos, UILayoutFrame* target, uint32_t toPos, float relativeX, float relativeY);
	void setRelativeObject(UILayoutFrame* target);
	float width() {return this->base<war3::CLayoutFrame*>(this->baseLayoutFrameOffset_)->width
		* War3WindowWidthRatio();}
	float height() {return this->base<war3::CLayoutFrame*>(this->baseLayoutFrameOffset_)->height;}
	void setWidth(float width) {
		this->base<war3::CLayoutFrame*>(this->baseLayoutFrameOffset_)->width = 
			( width / War3WindowWidthRatio() );
		applyPosition();
	}
	void setHeight(float height) {
		this->base<war3::CLayoutFrame*>(this->baseLayoutFrameOffset_)->height = height;
		applyPosition();
	}
	float scale() {return this->base<war3::CLayoutFrame*>(this->baseLayoutFrameOffset_)->scale;}
	void setScale(float scale) {
		this->base<war3::CLayoutFrame*>(this->baseLayoutFrameOffset_)->scale = scale;
		applyPosition();
	}
	bool containsXY(float x, float y) {
		war3::CLayoutFrame* base = this->base<war3::CLayoutFrame*>(this->baseLayoutFrameOffset_);
		return (
			y > base->borderB &&
			y < base->borderU &&
			x > base->borderL &&
			x < base->borderR
		);
	}
	float getCenterX() {
		war3::CLayoutFrame* base = this->base<war3::CLayoutFrame*>(this->baseLayoutFrameOffset_);
		return base ? (base->borderL + base->borderR)/2.f : 0;
	}
	float getCenterY() {
		war3::CLayoutFrame* base = this->base<war3::CLayoutFrame*>(this->baseLayoutFrameOffset_);
		return base ? (base->borderU + base->borderB)/2.f : 0;
	}
	void applyPosition();
private:
	uint32_t baseLayoutFrameOffset_;
};

#endif