// mysocket.h: interface for the mysocket class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MYSOCKET_H__33C94245_415D_11D6_8886_200654C10000__INCLUDED_)
#define AFX_MYSOCKET_H__33C94245_415D_11D6_8886_200654C10000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include<afxsock.h>


class mysocket : public CSocket  
{
public:
	CDialog *dlg;
	CString name;
	int closeflag;
	
	mysocket();
	setparent(CDialog *dlg);
	//void OnConnect(int errcode);
	void OnReceive(int errcode);
	void OnClose(int errcode);
	

};

#endif // !defined(AFX_MYSOCKET_H__33C94245_415D_11D6_8886_200654C10000__INCLUDED_)
