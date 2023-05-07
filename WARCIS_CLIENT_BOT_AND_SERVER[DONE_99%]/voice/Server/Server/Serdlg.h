// Serdlg.h: interface for the Serdlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SERDLG_H__73FA8624_4122_11D6_8886_801654C10000__INCLUDED_)
#define AFX_SERDLG_H__73FA8624_4122_11D6_8886_801654C10000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include"mysocket.h"

class Serdlg : public CDialog  
{
public:
	mysocket sersock;
	CString userlist,buser;
	CPtrList clientlist;
	CStdioFile log;
	CListBox *displist;
	int reccount,broadcast;
	
	Serdlg(int n);
	virtual ~Serdlg();

	int OnInitDialog();
	void OnStart();
	void OnAbout();
	void OnPaint();
	HBRUSH OnCtlColor(CDC *pdc,CWnd *pwnd,UINT ctrl);
	void OnCloseDlg();
	void Accept();
	void Receive(mysocket *client);
	void SendToAll(CString client,char *data,int size,int flag);
	void SendToClient(CString user,char *data,int size);
	void UpdateList(CString name);
	void OnCloseClient(mysocket *client);

DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_SERDLG_H__73FA8624_4122_11D6_8886_801654C10000__INCLUDED_)
