///////////////////////////////////////////////////////////////////
// Broadcast.cpp: implementation of the Broadcast class.
// Used to support broadcasting feature on demand...
//
//////////////////////////////////////////////////////////////////////
#include<afxwin.h>
#include "Broadcast.h"
#include "resource.h"
#include "Display.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(Broadcast,CDialog)
ON_COMMAND(IDC_BUTTON1,OnStart)
ON_COMMAND(IDC_BUTTON2,OnStop)
ON_WM_PAINT()
ON_COMMAND(IDCANCEL,OnClose)

END_MESSAGE_MAP()

Broadcast::Broadcast(int n,CDialog *dlg):CDialog(n)
{
isstarted=0;
disp=dlg;
}

Broadcast::~Broadcast()
{

}


BOOL Broadcast::OnInitDialog()
{
start=(CButton *)GetDlgItem(IDC_BUTTON1);
stop=(CButton *)GetDlgItem(IDC_BUTTON2);
	
start->EnableWindow(TRUE);
stop->EnableWindow(FALSE);

return CDialog::OnInitDialog();

}

void Broadcast::OnStart()
{
isstarted=1;

start->EnableWindow(FALSE);
stop->EnableWindow(TRUE);
((Display*)disp)->selectflag=1;
((Display*)disp)->startRecording();
((Display*)disp)->startPlaying();

}


void Broadcast::OnStop()
{

	if(isstarted)
	{
	((Display*)disp)->selectflag=0;
	((Display*)disp)->stopRecording();
	((Display*)disp)->stopPlaying();

	}

	CDialog::OnCancel();
}

void Broadcast::OnClose()
{

	if(isstarted)
		OnStop();
	
	
	CDialog::OnCancel();

}


/*                                                           */
/* OnPaint() will fill the client area with                  */
/* the selected gradient color.                              */
/*                                                           */ 

void Broadcast::OnPaint()
{
CRect r;
CPen p[64];
int i,factor;

CPaintDC pdc(this);


	GetClientRect(&r);
	factor=r.bottom/63;

	for(i=0;i<64;i++)
	p[i].CreatePen(PS_SOLID,1,RGB(250,120+i*2,0));
	
	for(i=0;i<r.bottom;i++)
	{
	pdc.SelectObject(&p[i/factor]);
	pdc.MoveTo(0,i);
	pdc.LineTo(r.right,i);
	}

}


/*                                                           */
/* This function will paint the bkgnd of individual          */
/* components of dialog box                                  */
/*                                                           */ 

HBRUSH Broadcast::OnCtlColor(CDC *pdc,CWnd *pwnd,UINT ctrl)
{
	int id=pwnd->GetDlgCtrlID();

pdc->SetTextColor(RGB(0,0,255));

	switch(id)
	{

	case 2091:case 2094:
	pdc->SetBkMode(TRANSPARENT);
		
	HBRUSH hbr=CreateSolidBrush(RGB(255,250,220));
	return hbr;

	}

return NULL;
}