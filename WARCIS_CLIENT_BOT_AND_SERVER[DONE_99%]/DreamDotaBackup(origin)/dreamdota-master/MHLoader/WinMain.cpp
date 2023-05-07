#include "stdafx.h"
#include <CommCtrl.h>
#pragma comment (lib, "comctl32.lib")
#pragma comment (linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include "Exception.h"

//Modules
#include "Utils.h"
#include "NTDLL.h"
#include "Injection.h"

#include "MHMain.h"

int DreamMain() {
	VMProtectBeginVirtualization("DreamMain");

	//����ģ���ʼ�� 
	Utils::Init();

	int rv;
	bool module_init_success = true;
	srand((DWORD)time(NULL));
	InitCommonControls();

	//��ʼ��ģ��
	int e = GLOBAL_SETJMP();
	if (e == 0) {
		NTDLL::Init();
	} else {
		module_init_success = false;
		Abort(e);
	}

	if (module_init_success)
		rv = Main();

	//ģ������
	NTDLL::Cleanup();

	//����ģ������
	Utils::Cleanup();

	VMProtectEnd();
	return rv;
}

int CALLBACK WinMain(
  __in  HINSTANCE hInstance,
  __in  HINSTANCE hPrevInstance,
  __in  LPSTR lpCmdLine,
  __in  int nCmdShow
) 
{
	return DreamMain();
}