// Save.h: interface for the Save class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SAVE_H__642728A1_5579_11D6_8886_F00753C10001__INCLUDED_)
#define AFX_SAVE_H__642728A1_5579_11D6_8886_F00753C10001__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include<afxwin.h>


class Save : public CDialog  
{
public:
	
	BOOL isstarted;
	CButton *start,*stop;
	CDialog *disp;	


	Save(int,CDialog *dlg);
	virtual ~Save();
	int OnInitDialog();
	void OnStart();
	void OnStop();
	void OnCancel();

	HBRUSH OnCtlColor(CDC *pdc,CWnd *pwnd,UINT ctrl);
	void OnPaint();
DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_SAVE_H__642728A1_5579_11D6_8886_F00753C10001__INCLUDED_)
