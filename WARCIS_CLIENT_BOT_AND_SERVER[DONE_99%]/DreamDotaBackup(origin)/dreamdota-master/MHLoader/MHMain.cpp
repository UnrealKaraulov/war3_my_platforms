#include "stdafx.h"
#include "MHMain.h"
#include "NTDLL.h"
#include "DialogMain.h"
#include "ThreadSearch.h"
#include "Exception.h"
#include "Injection.h"

static ThreadSearch::ResultType Result;
static DialogMain* Dialog;

static void onWar3Found() {
	bool success;
	Dialog->t_status()->SetWindowText(L"��⵽��Ϸ��ע����...");
	VMProtectBeginVirtualization("Inject");
	success = Inject(Result.win, Result.process_handle, Result.invoker, L"MHModule.dll");
	VMProtectEnd();
	if (success)
		Dialog->t_status()->SetWindowText(L"ע��ɹ�");
	else {
		MessageBox(Dialog->handle(), L"ע��ʧ�ܣ��볢�Թر�ɱ�������", NULL, MB_ICONERROR);
		Dialog->t_status()->SetWindowText(L"ע��ʧ��");
	}
}

static void onWar3Closed() {
	Dialog->t_status()->SetWindowText(L"�ȴ���Ϸ����");
}

int Main() {

	BOOLEAN enabled;
	VMProtectBeginVirtualization("AdjustPrivilege");
	if (!NT_SUCCESS(NTDLL::RtlAdjustPrivilege(NTDLL::SE_DEBUG_PRIVILEGE, 1, 0, &enabled))) {
		Abort(EXCEPTION_ADJUST_PRIVILEGE);
	}
	VMProtectEnd();

	ThreadSearch ts;
	ts.onWar3WindowFound += onWar3Found;
	ts.onWar3WindowClosed += onWar3Closed;
	ts.Run((DWORD)&Result);

	Dialog = new DialogMain();
	Dialog->Show(true);
	Dialog->WaitClose();
	delete Dialog;
	
	ts.Term();
	return 0;
}