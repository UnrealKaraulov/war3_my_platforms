// about.h: interface for the about class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ABOUT_H__33737ACC_43C6_11D6_8886_F00753C10001__INCLUDED_)
#define AFX_ABOUT_H__33737ACC_43C6_11D6_8886_F00753C10001__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class about : public CDialog  
{
public:
	about(int n);
	virtual ~about();
	HBRUSH OnCtlColor(CDC *pdc,CWnd *pwnd,UINT ctrl);
	void OnPaint();


	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_ABOUT_H__33737ACC_43C6_11D6_8886_F00753C10001__INCLUDED_)
