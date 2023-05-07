// mysocket.h: interface for the mysocket class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MYSOCKET_H__73FA8625_4122_11D6_8886_801654C10000__INCLUDED_)
#define AFX_MYSOCKET_H__73FA8625_4122_11D6_8886_801654C10000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include<afxsock.h>


class mysocket : public CSocket  
{
public:
	
	CDialog *dlg;
	CString name;

	mysocket();
	virtual ~mysocket();
	void setparent(CDialog *dlg);
	virtual void OnAccept(int errcode);
	virtual void OnReceive(int errcode);
	virtual void OnClose(int errcode);
};

#endif // !defined(AFX_MYSOCKET_H__73FA8625_4122_11D6_8886_801654C10000__INCLUDED_)
