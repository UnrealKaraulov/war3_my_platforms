//////////////////////////////////////////////////////////////////
//Controls the volume for recording and playing volume....
// Volume.cpp: implementation of the Volume class.
//
//////////////////////////////////////////////////////////////////////


#include<afxwin.h>
#include<mmsystem.h>
#include<afxcmn.h>
#include"Mixer.h"
#include "Volume.h"
#include "Resource.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(Volume,CDialog)
ON_WM_HSCROLL()
ON_WM_PAINT()
ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


Volume::Volume(int n):CDialog(n)
{

}

BOOL Volume::OnInitDialog()
{
int min,max;

//Volume control for recording 
mixrec=new CMixer(MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE, CMixer::Record,min,max);

srec=(CSliderCtrl*)GetDlgItem(IDC_SLIDER1);

 srec->SetRange(min,max);
 int pos=mixrec->GetVolume();
 srec->SetPos(pos);

//Volume control for playing 
mixplay=new CMixer(MIXERLINE_COMPONENTTYPE_DST_SPEAKERS ,CMixer::Play,min,max);	

splay=(CSliderCtrl*)GetDlgItem(IDC_SLIDER2);

 splay->SetRange(min,max);
 pos=mixplay->GetVolume();
 splay->SetPos(pos);




return TRUE;
}


void Volume::OnHScroll(UINT code,UINT pos,CScrollBar *scroll)
{
int vol;
	if(srec==(CSliderCtrl*)scroll)
	{
	vol=srec->GetPos();
	mixrec->SetVolume(vol);
	}

	if(splay==(CSliderCtrl*)scroll)
	{
	vol=splay->GetPos();
	mixplay->SetVolume(vol);
	}

}

/*                                                           */
/* OnPaint() will fill the client area with                  */
/* the selected gradient color.                              */
/*                                                           */ 

void Volume::OnPaint()
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

HBRUSH Volume::OnCtlColor(CDC *pdc,CWnd *pwnd,UINT ctrl)
{
int id=pwnd->GetDlgCtrlID();
HBRUSH hbr;
pdc->SetTextColor(RGB(0,0,255));

	switch(id)
	{

	case 4441:case 4442:
	pdc->SetBkMode(TRANSPARENT);
		
	hbr=CreateSolidBrush(RGB(255,250,220));
	return hbr;

//	case IDC_SLIDER1:case IDC_SLIDER2:
//		return NULL;

	}

//return NULL;
}

