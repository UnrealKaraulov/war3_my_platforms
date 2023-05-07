#ifndef SLOT_HOTKEY_BUTTON_H_
#define SLOT_HOTKEY_BUTTON_H_
#include "HotkeyButton.h"

class SlotHotkeyButton : public HotkeyButton {
	UISimpleButton *btnClose;	//���ϽǵĹرհ�ť
	Observer *obsBtnClose;		//�رհ�ť��observer
public:
	char *stateProfileApp;
	char *stateProfileKey;
	bool isHidden;

	SlotHotkeyButton(
		UISimpleFrame*			parent,
		float					width,
		float					height,
		int*					hotkeyVar,
		int						defaultHotkey,
		char*					profileApp = NULL,
		char*					profileKey = NULL,
		char*					enabledProfileKey = NULL
	);

	~SlotHotkeyButton();

	UISimpleButton *getCloseButton() {return btnClose;}

	void enable(bool flag = true);
};

#endif