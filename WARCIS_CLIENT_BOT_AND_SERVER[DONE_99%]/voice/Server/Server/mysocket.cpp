// mysocket.cpp: implementation of the mysocket class.
//
//////////////////////////////////////////////////////////////////////
#include<afxwin.h>
#include<afxsock.h>
#include "mysocket.h"
#include "Serdlg.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

mysocket::mysocket()
{
name="";
}

mysocket::~mysocket()
{

}

void mysocket::setparent(CDialog *dialog)
{
dlg=dialog;
}


//
// This function will be called when the client connects to
// the Server.
//

void mysocket::OnAccept(int errcode)
{
	if(errcode==0)
	((Serdlg*)dlg)->Accept();

	CSocket::OnAccept(errcode);
}

//
// This function is gets invoked when the socket recieves the
// data from the remote socket
//

void mysocket::OnReceive(int errcode)
{
	
	if(errcode==0)
	((Serdlg*)dlg)->Receive(this);

	CSocket::OnAccept(errcode);
}

void mysocket::OnClose(int errcode)
{
	if(errcode==0)
	((Serdlg*)dlg)->OnCloseClient(this);

}