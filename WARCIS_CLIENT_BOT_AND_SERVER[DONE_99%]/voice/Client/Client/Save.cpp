// Save.cpp: implementation of the Save class.
//
//////////////////////////////////////////////////////////////////////

#include<afxwin.h>
#include "Save.h"
#include "resource.h"
#include "Display.h"
#include "WriteSound.h"

BEGIN_MESSAGE_MAP(Save,CDialog)
ON_WM_PAINT()
ON_WM_CTLCOLOR()
ON_COMMAND(IDC_BUTTON1,OnStart)
ON_COMMAND(IDC_BUTTON2,OnStop)
ON_COMMAND(IDCANCEL,OnCancel)

END_MESSAGE_MAP()

Save::Save(int n,CDialog *dlg):CDialog(n)
{
disp=dlg;
}

Save::~Save()
{

}


int Save::OnInitDialog()
{

isstarted=FALSE;

start=(CButton*)GetDlgItem(IDC_BUTTON1);
stop =(CButton*)GetDlgItem(IDC_BUTTON2);

stop->EnableWindow(FALSE);

return CDialog::OnInitDialog();
}



/*                                                           */
/* OnPaint() will fill the client area with                  */
/* the selected gradient color.                              */
/*                                                           */ 

void Save::OnPaint()
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

HBRUSH Save::OnCtlColor(CDC *pdc,CWnd *pwnd,UINT ctrl)
{

return NULL;
}



void Save::OnStart()
{
char *fname=new char[100];

GetDlgItemText(IDC_EDIT1,fname,100);
	
	if(strlen(fname)<1)
	{
	MessageBox("Please specify the filename for saving");
	return;
	}	

	
	isstarted=TRUE;

	((Display*)disp)->OnStartWrite(fname);

	//don't access fname //deleted by thread
	start->EnableWindow(FALSE);
	stop->EnableWindow(TRUE);


}


void Save::OnStop()
{

	if(isstarted==FALSE) 
	return;
	
	isstarted=FALSE;

	((Display*)disp)->OnStopWrite();
	
	start->EnableWindow(TRUE);
	stop->EnableWindow(FALSE);


}


void Save::OnCancel()
{
	
	if(isstarted)
	OnStop();

	CDialog::OnCancel();
}


