#include "stdafx.h"
#ifndef BUTTON_H_INCLUDED
#define BUTTON_H_INCLUDED
#include "UIStructs.h"
#include "UISimpleButton.h"
#include "UISimpleFrame.h"
#include "UISimpleTexture.h"
#include "UISimpleFontString.h"
#include "EventDispatcher.h"
#include "Observer.h"
#include "Event.h"
#include "IUIObject.h"

class ToolTip;
class Button;
class Label;

typedef void (*ButtonCallback)(Button* button);

//��һ��UISimpleFrame������/���һ��UISimpleButton���ؼ���һ��UISimpleFontString����ʾ�������
class Button {
public:
	Button();

	Button (
		UISimpleFrame*			parent,
		float					width,
		float					height,
		uint32_t				mouseClickFlag,
		UISimpleButton::State	initClickState,
		ButtonCallback			callback = NULL,
		const char*				tooltip = NULL,
		bool					dontStore = false,
		bool					noSurface = false,
		float					fontHeight = 0.013f
	);

	~Button();

	void			setAbsolutePosition (UISimpleButton::Position originPos, float x, float y);
	void			setRelativePosition (uint32_t originPos, UILayoutFrame* target, uint32_t toPos, float relativeX, float relativeY);
	void			setText (const char *text);
	const char*		getText () const;
	void			applyPosition();
	void			push ();
	void			showToolTip();
	void			hideToolTip();//TODO �ϲ�����һ��showToolTip
	void			setWidth ( float width );
	void			setHeight ( float height );

	void			showText(bool flag);
	void			setTextAlign(TextAlignmentHorizontal alignHoriz, TextAlignmentVertical alignVert);
	bool			isTextShown();

	void			bindData(int key, void *dataObj);
	void*			getData(int key);

	UILayoutFrame*	getFrame();
	UISimpleButton* getButton();
	Label*			getCaption();

	void enable(bool flag = true);
	

	Observer*				eventObserver;
	UISimpleFrame*			controlCoverNormal;
	UISimpleFrame*			controlCoverPushed;
	UISimpleFrame*			controlCoverDisabled;

	static void Init();
	static void Cleanup();
private:
	static uint32_t			indexTotal;

	uint32_t				index;
	UISimpleFrame*			_backdrop;
	UISimpleButton*			_control;
	ButtonCallback			_controlCallback;
	Label*					_caption;
	float					_fontHeight;
	ToolTip*				_tooltip;

	bool				_isDisabled;
	bool				_isTextShown;

	std::map<int, void*>	boundData;
};

#endif