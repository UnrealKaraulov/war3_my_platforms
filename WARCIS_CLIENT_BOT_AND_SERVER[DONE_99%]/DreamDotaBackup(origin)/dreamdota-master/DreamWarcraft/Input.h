#include "stdafx.h"
#ifndef INPUT_H_INCLUDED_
#define INPUT_H_INCLUDED_

#include "GameStructs.h"

struct MousePosition {
	float x;
	float y;
	MousePosition(): x(0), y(0) {}
	MousePosition(float _x, float _y) : x(_x), y(_y) {}
};

bool isChatBoxOn();

//�����ť
void GameUIButtonClick (void *button, int mouseCode, bool sendAsProgramm);
//�������
void GameUIKeyPress (int keyCode, bool down, bool sendAsProgramm);

void Input_Init();
void Input_Update(war3::CEvent* evt);
void ItemClick_Report(war3::CEvent *evt);

bool KeyIsDown(const uint32_t keyCode);

MousePosition* GetMousePosition();

void* PositionGetButton(float x, float y, bool ignoreTooLarge = true);
void* PositionGetButton(MousePosition *pos, bool ignoreTooLarge = true);
war3::CCommandButton* HotkeyGetButton(uint32_t key);
bool IsCancelPanelOn();

#endif