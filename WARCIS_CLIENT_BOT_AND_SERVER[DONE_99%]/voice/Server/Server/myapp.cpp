// myapp.cpp: implementation of the myapp class.
//
//////////////////////////////////////////////////////////////////////
#include<afxwin.h>
#include "myapp.h"
#include "Serdlg.h"
#include "resource.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BOOL myapp::InitInstance()
{
	
Serdlg dlg(IDD_DIALOG1);
m_pMainWnd=&dlg;
dlg.DoModal();

return FALSE;
}

myapp a;