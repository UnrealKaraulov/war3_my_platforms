// Serdlg.cpp: implementation of the Serdlg class.
//
//////////////////////////////////////////////////////////////////////

#include <afxwin.h>
#include "Serdlg.h"
#include "about.h"
#include "mysocket.h"
#include "resource.h"




//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


BEGIN_MESSAGE_MAP(Serdlg,CDialog)
ON_COMMAND(IDC_BUTTON1,OnStart)
ON_COMMAND(IDC_BUTTON2,OnAbout)
ON_COMMAND(IDCANCEL,OnCloseDlg)
ON_WM_PAINT()
ON_WM_CTLCOLOR()
ON_WM_SYSCOMMAND()
END_MESSAGE_MAP()

//
//Construction
//
Serdlg::Serdlg(int n):CDialog(n)
{
	log.Open("serlog.txt",CFile::modeCreate |CFile::modeWrite);
	reccount=0;
	buser.Empty();
	broadcast=0;
}


//
//Destuction
//
Serdlg::~Serdlg()
{
	log.Close();
}



/*                                                           */
/* This function does initial operations                     */
/*                                                           */ 

int Serdlg::OnInitDialog()
{
SetDlgItemInt(IDC_EDIT1,1051);
displist=(CListBox *)GetDlgItem(IDC_LIST2);

	CDialog::OnInitDialog();
	
	//Set the Icon for the application
	HICON m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON2);
	SetIcon(m_hIcon,TRUE);

	
	//Initialize the socket options
	if(!AfxSocketInit())
	{
	log.WriteString("Unable to initialize the socket");
	AfxMessageBox("Unable to initialize socket");
	CDialog::OnCancel();
	}

	userlist.Empty();
	broadcast=0;

	log.WriteString("\nIn the InitDialog of Dialog");
	
	return TRUE;
}


/*                                                           */
/* OnPaint() will fill the client area with                   */
/* the selected gradient color.                              */
/*                                                           */ 

void Serdlg::OnPaint()
{
CRect r;
CPen p[64];
int i,factor;
CBrush mybrush;

CPaintDC pdc(this);

log.WriteString("\nIn the erase background box");

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

HBRUSH Serdlg::OnCtlColor(CDC *pdc,CWnd *pwnd,UINT ctrl)
{
int id=pwnd->GetDlgCtrlID();

	pdc->SetTextColor(RGB(0,0,255));

	switch(id)
	{

	case 1093:case 1094:
	pdc->SetBkMode(TRANSPARENT);
	HBRUSH hbr=CreateSolidBrush(RGB(255,250,220));
	return hbr;

	}

return NULL;
}



/*                                                           */
/* When the user presses Start button Server                 */
/* will be started.                                          */
/*                                                           */ 

void Serdlg::OnStart()
{
int port=GetDlgItemInt(IDC_EDIT1);

	log.WriteString("\nIn the OnStart method of Dialog");
	CButton *start=(CButton*)this->GetDlgItem(IDC_BUTTON1);

	if(port<1024)
	{
	AfxMessageBox("Please user the port numbers greater than 1023");
	return;
	}	

	if(sersock.Create(port))
	{
		if(sersock.Listen())
		log.WriteString("\nServer started successfully");
		sersock.setparent(this);
	}
	else
	log.WriteString("\nUnable to start the server");

	if(start!=NULL)      //disable the start button
	start->EnableWindow(FALSE);

}


/*                                                           */
/* Displays the About dialog box                             */
/*                                                           */ 


void Serdlg::OnAbout()
{
about ab(IDD_DIALOG2);
ab.DoModal();
	
}



/*                                                           */
/* Closes the Server dialog box on pressing close button     */
/* It will notify the clients about shutdown.                */ 
/*                                                           */ 

void Serdlg::OnCloseDlg()
{
	log.WriteString("\nClosing the dialog box");

	sersock.Close();
	CDialog::OnCancel();
}




/*                                                           */
/* Accepts the client connections..stores them in the list   */
/* (Called from "mysocket OnAccept method)                   */ 
/*                                                           */ 

void Serdlg::Accept()
{
mysocket *client=new mysocket();
	
log.WriteString("\n In the Accept button");
	
	if(sersock.Accept(*client))
	{
	clientlist.AddTail(client);
	client->setparent(this);
	log.WriteString("\nConnected to client :");
	}
	else
	log.WriteString("\nUnable to connect to client");

}






/*            **** MESSAGE FORMAT ****
	There are 2 type of message formats
   
	header : mesg
    header : length : mesg
	
	* first type of mesg used for sending userlist,shutdown mesg  
	header = USER,DOWN,NEW;	
	USER = mesg contains the new userlist	    ; server->client
	DOWN = server is going to shutdown mesg=NULL; server->client		
	NEW  = mesg contains the name of new client ; client->server		  
	
	* second type of format used for sending voice data
	header = <client name>,ALL;
	<client>= name of target client and the voice data ; client->server
	 ALL    = send the voice data to all clients       ; client->server

*/


/*                                                    */
/* Receives the message from client and redirect      */
/* mesg to other client...                            */
/*  (Called from "mysocket OnReceive method)          */ 
/*                                                    */ 

void Serdlg::Receive(mysocket *client)
{

char buff[2021];
char str[100];
int size=2020,count,index,i;
CString mesg,header,test,bmesg;



	
	//Retrieve the message and store it in buff
	count=client->Receive(buff,size);

	if(count==SOCKET_ERROR)
	{
	log.WriteString("\nError occured during receive");
	return;
	}

	if(count>2020 || count<0)
	{
	sprintf(str,"message length= %d is greater than buffer ",count);
	log.WriteString(str);
	return;
	buff[2020]=NULL;
	}
	else
	{
	buff[count]=NULL;
	}
	mesg=buff;

	sprintf(str,"\nReceived mesg with len = %d ",count);
	log.WriteString(str);
	

	index=mesg.Find(':');
	
	if(index==-1)
	{
	   log.WriteString("\n Wrong Message Format-- mesg is \n");

		for(i=0;i<2020;i++)
		log.Write(&buff[i],1);
		
	return;
	}

	header=mesg.Left(index);

	mesg=mesg.Right(mesg.GetLength()-index-1);


	if(header.CompareNoCase("NEW")==0)  //User name of new client
	{
		test=mesg;
		test.MakeUpper();
		displist->AddString(test);
		userlist+=test+";";
		client->name=test;
		test="USER:"+userlist;
		SendToAll(client->name,(char*)(LPCTSTR)test,test.GetLength(),0);
	
		return;
	}
	

	if(broadcast==1 && buser.CompareNoCase(client->name)!=0)
	return;

	if( broadcast==1 && header.CompareNoCase("OVER")==0)
	{
		broadcast=0;
		buser.Empty();
		bmesg="RESUME:";
		SendToAll(client->name,(char*)(LPCTSTR)bmesg,bmesg.GetLength(),1);
		
		return;
	}

		for(i=0;i<client->name.GetLength();i++)
		buff[i]=client->name[i];
		buff[i]=':';
		
			
		if(header.CompareNoCase("ALL")==0)
		{
			if(broadcast==0)
			{
			buser=client->name;
			bmesg="WAIT:"+buser;
			broadcast=1;
			SendToAll(client->name,(char*)(LPCTSTR)bmesg,bmesg.GetLength(),1);
			Sleep(200);
			}	
			
			SendToAll(client->name,buff,count,1);
		}
		else
		SendToClient(header,buff,count);
	
		
		//User for debugging
		reccount++;
		sprintf(str,"\n Total Message received = %d ",reccount);
		log.WriteString(str);
	


}



/*                                                    */
/* Sends message to all clients                       */
/*                                                    */ 

void Serdlg::SendToAll(CString name,char *data,int size,int flag)
{
mysocket *temp;	
POSITION pos=clientlist.GetHeadPosition();

	for(int i=0;i<clientlist.GetCount();i++)
	{
	temp=(mysocket *)clientlist.GetNext(pos);
		
		if(flag && (name.CompareNoCase(temp->name)==0))
		continue;
	
	temp->Send(data,size);
	}

}


/*                                                    */
/* Sends message to perticular client                 */
/*                                                    */ 

void Serdlg::SendToClient(CString user,char *data,int size)
{
mysocket *temp;	
POSITION pos=clientlist.GetHeadPosition();
	
	for(int i=0;i<clientlist.GetCount();i++)
	{
	temp=(mysocket *)clientlist.GetNext(pos);
		if(user.CompareNoCase(temp->name)==0)
		{
		temp->Send(data,size);
		break;
		}	
	}

}


/*                                                      */
/* This function closes the socket of perticular client */
/* when the client logged out.                          */
/* ( Called from mysocket's  OnClose method             */
/*                                                      */
 

void Serdlg::OnCloseClient(mysocket *client)
{
mysocket *temp;	
POSITION delpos,pos=clientlist.GetHeadPosition();

	
log.WriteString("\nClosing the client " +client->name);

	for(int i=0;i<clientlist.GetCount();i++)
	{
	delpos=pos;
		
	temp=(mysocket *)clientlist.GetNext(pos);
		if(temp->name==client->name)
		{
		clientlist.RemoveAt(delpos);
		break;
		}

	}

	log.WriteString("\nDeleted the client from the ptr list" );

	int index=userlist.Find(client->name);
	if(index!=-1)
	userlist.Delete(index,client->name.GetLength()+1);

	log.WriteString("\n user name is deleted from userlist");
	
	CString test="USER:"+userlist;

	//Send the new user list to all clients
	SendToAll(client->name,(char*)(LPCTSTR)test,test.GetLength(),0);

	UpdateList(client->name);
	
	client->Close();
	
	
}




/*                                                    */
/* Removes the client name from the list box          */
/* (Called from the OnCloseClient method              */ 
/*                                                    */ 


void Serdlg::UpdateList(CString name)
{
int index=displist->FindString(0,name);	

log.WriteString("\nupdating the listbox");

if(index!=LB_ERR)
displist->DeleteString(index);


}