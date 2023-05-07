//�������Ͱ�ť����������һ���ȼ�����ť��ʾ����Ϊ�ȼ����ı��������ť�󰴼���������

#ifndef HOTKEY_BUTTON_H_
#define HOTKEY_BUTTON_H_

#include "Button.h"

class HotkeyButton : public Button {
	int		_hotkey;
	int*	_pVarHotkey; //�󶨵��ȼ�����(������ֵ)
	char*	_profileApp; //�󶨵�profile������
	char*	_profileKey; //�󶨵�profileС����
	bool	_enteringHotkey;	//��ʾ�Ƿ���������ȼ�״̬
public:
	ButtonCallback	hotkeyButtonCallback;

	HotkeyButton() : _hotkey(NULL), _pVarHotkey(NULL), _profileApp(NULL), _profileKey(NULL), _enteringHotkey(false) {}

	HotkeyButton(
		UISimpleFrame*			parent,
		float					width,
		float					height,
		int*					hotkeyVar,
		int						defaultHotkey,
		char*					profileApp = NULL,
		char*					profileKey = NULL,
		ButtonCallback			callback = NULL,
		bool					noSurface = false
	);

	~HotkeyButton ();

	void setKey(int keyCode);
	int getKey() {return _hotkey;}
	void toggleEnteringHotkey() {_enteringHotkey = !_enteringHotkey;}
	bool isEnteringHotkey() {return _enteringHotkey;}

	static void Init();
	static void Cleanup();
};

#endif