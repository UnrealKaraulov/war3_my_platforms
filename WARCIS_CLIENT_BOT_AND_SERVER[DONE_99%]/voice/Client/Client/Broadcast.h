// Broadcast.h: interface for the Broadcast class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BROADCAST_H__60F4DC61_5856_11D6_8886_900654C10000__INCLUDED_)
#define AFX_BROADCAST_H__60F4DC61_5856_11D6_8886_900654C10000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include<afxwin.h>

class Broadcast : public CDialog  
{
public:
	int isstarted;
	CDialog *disp;
	CButton *start,*stop;
	
	Broadcast(int n,CDialog *);
	virtual ~Broadcast();
	void OnStart();
	void OnStop();
	void OnClose();
	BOOL OnInitDialog();
	HBRUSH OnCtlColor(CDC *pdc,CWnd *pwnd,UINT ctrl);
	void OnPaint();
DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_BROADCAST_H__60F4DC61_5856_11D6_8886_900654C10000__INCLUDED_)
