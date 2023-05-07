///////////////////////////////////////////////////////////////////
// mysocket.cpp: implementation of the mysocket class.
// Handles communication related details....
//////////////////////////////////////////////////////////////////////


#include<afxwin.h>
#include<afxsock.h>
#include "mysocket.h"
#include"Display.h"



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

mysocket::mysocket()
{
	closeflag=0;
	name="";
}



mysocket::setparent(CDialog *dialog)
{
dlg=dialog;
closeflag=0;
}


//
// This function will be invoked when the client receives message
// from remote socket

void mysocket::OnReceive(int errcode)
{
	//call the receive function of Display class.
	if(errcode==0)  
	((Display*)dlg)->Receive();  

	CSocket::OnReceive(errcode);
}

//
// Called when Server has shutdown....
//
//

void mysocket::OnClose(int errcode)
{
	closeflag=1;
	((Display*)dlg)->OnCancel();

}