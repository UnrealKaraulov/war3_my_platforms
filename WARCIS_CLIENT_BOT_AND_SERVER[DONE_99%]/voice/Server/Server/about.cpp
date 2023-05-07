// about.cpp: implementation of the about class.
//
//////////////////////////////////////////////////////////////////////
#include <afxwin.h>
#include "about.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(about,CDialog)
ON_WM_PAINT()
ON_WM_CTLCOLOR()
END_MESSAGE_MAP()
about::about(int n):CDialog(n)
{

}

about::~about()
{

}
/*                                                           */
/* OnPaint() will fill the client area with                   */
/* the selected gradient color.                              */
/*                                                           */ 

void about::OnPaint()
{
CRect r;
CPen p[64];
int i,factor;
CBrush mybrush;

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

HBRUSH about::OnCtlColor(CDC *pdc,CWnd *pwnd,UINT ctrl)
{
int id=pwnd->GetDlgCtrlID();

	pdc->SetTextColor(RGB(0,0,255));

	switch(id)
	{

	case 2091:case 2092:
	pdc->SetBkMode(TRANSPARENT);
	HBRUSH hbr=CreateSolidBrush(RGB(255,250,220));
	return hbr;

	}

return NULL;
}

