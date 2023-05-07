////////////////////////////////////////////////////////////////////
// Recording and playing Thread program concept is based on the 
// code by ***  Paul Chauffers..***...from www.codeguru.com
//                                         
// I am greatful to him  for helping me to turn my dream project into reality.
//
// Display.cpp: implementation of the Display class.
//
//////////////////////////////////////////////////////////////////////


#include<afxwin.h>
#include<afxcmn.h>
#include<afxdlgs.h>
#include "about.h"
#include "Display.h"
#include "PlaySound1.h"
#include "resource.h"
#include "Volume.h"
#include "Save.h"
#include "WriteSound.h"
#include "Broadcast.h"

//////////////////////////
//	MESSAGE MAPPING     //
//////////////////////////

BEGIN_MESSAGE_MAP(Display,CDialog)
ON_COMMAND(IDC_BUTTON1,Onconnect)
ON_COMMAND(IDC_BUTTON2,OnSave)
ON_COMMAND(IDC_BUTTON3,OnVolume)
ON_COMMAND(IDC_BUTTON4,OnPlay)
ON_COMMAND(IDC_BUTTON5,OnStart)
ON_COMMAND(IDC_BUTTON6,OnStop)
ON_COMMAND_RANGE(IDC_RADIO1,IDC_RADIO2,OnChange)
ON_WM_DESTROY()
ON_WM_PAINT()
//ON_WM_ERASEBKGND()
ON_WM_CTLCOLOR()
END_MESSAGE_MAP()



//
//Constructor
//
Display::Display(int n):CDialog(n)
{

isconnected=FALSE;
sendcount=0;
reccount=0;
//create the buffers for playing.....buffers are reused..
PreCreateHeader();
log.Open("log.txt",CFile::modeCreate | CFile::modeWrite);

}



//
// Destructor
//
Display::~Display()
{
log.WriteString("\n Deallocating the memory");
log.Close();

//if(anicon && success)
//anicon->Close();
	
	//Release the allocated memory
	for(int i=0;i<MAXBUFFER;i++)
	{
	if(playhead[i]->lpData)
	delete playhead[i]->lpData;
	if(playhead[i])
	delete playhead[i];
	}
}



/*                                                           */
/* This function precreates the waveblocks for storing       */
/* voice data before playing.This will allow the REUSE of    */
/* waveblocks.                                               */
/*                                                           */ 

void Display::PreCreateHeader()
{
	//Initial waveblock
	curhead=0;
	
	isstart=1;

	for(int i=0;i<MAXBUFFER;i++)
	{
	playhead[i] = new WAVEHDR;
	ZeroMemory(playhead[i], sizeof(WAVEHDR));  //Initialize it to zero
	char *temp=new char[PLAYBUFFER+50];	
	
	playhead[i]->lpData =temp;
	playhead[i]->dwBufferLength = PLAYBUFFER;
	playhead[i]->dwFlags=0;
	}

}


int Display::OnEraseBkgnd(CDC *pdc)
{
	log.WriteString("\nIn the Onerase function");

	return CDialog::OnEraseBkgnd(pdc);

}


/*                                                           */
/* This function does initial operations such                */
/* creating Record and Play threads...                       */
/*                                                           */ 

int Display::OnInitDialog()
{
CFont myfont;	
	

	isSave=FALSE;

	myfont.CreateFont(20,12,0,0,0,0,0,0,0,0,0,0,0,"System");	

	SetDlgItemText(IDC_EDIT1,"localhost");
	SetDlgItemInt(IDC_EDIT2,1051);
	SetDlgItemText(IDC_EDIT3,"logname");

	cbox=(CComboBox*)GetDlgItem(IDC_COMBO1);

	anicon=(CAnimateCtrl*)GetDlgItem(IDC_ANIMATE1);
	talk=(CAnimateCtrl*)GetDlgItem(IDC_ANIMATE2);
	

	if(!anicon->Open("conn.avi"))
	{
	success=FALSE;
	log.WriteString("\nUnable to open the avi file ****");
	}
	else
	success=TRUE;
	
	if(!talk->Open("talk.avi"))
	{
	doit=FALSE;
	log.WriteString("\n Unable to open the talk.avi file");
	}
	else
	doit=TRUE;

//	from=(CListBox *)GetDlgItem(IDC_LIST1);
//	from->SetFont(&myfont);

	curuser.Empty();


	//Set the Icon for the  application
	HICON m_hicon=AfxGetApp()->LoadIcon(IDI_ICON1);
	SetIcon(m_hicon,TRUE);
	SetIcon(m_hicon,FALSE);

	
	//Initializes  socket 
	if(!::AfxSocketInit())	
	{
	log.WriteString("\nSocket initialization failed");
	return 0;
	}

	//Create Client socket object
	sockclt.setparent(this);
	sockclt.Create();

	//Create and Start Recorder Thread
	record=new RecordSound(this);
	record->CreateThread();
	
	//Create and Start Player Thread
	play=new PlaySound1(this);
	play->CreateThread();
	
	//Create the Writer Thread to save the sound
	write=new WriteSound();
	write->CreateThread();

	selectflag=0;
	start=stop=NULL;

return CDialog::OnInitDialog();
}



/*                                                           */
/* OnPaint() will fill the client area with                  */
/* the selected gradient color.                              */
/*                                                           */ 

void Display::OnPaint()
{
CRect r;
CPen p[64];
int i,factor;

//log.WriteString("\n In the OnPaint fun");
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

HBRUSH Display::OnCtlColor(CDC *pdc,CWnd *pwnd,UINT ctrl)
{
int id=pwnd->GetDlgCtrlID();
//log.WriteString("\n In the on ctrl color fun");

pdc->SetTextColor(RGB(0,0,255));

	switch(id)
	{

	case 1098:case 6051:case 1099:case 1086:case 1075:case 1071:
    case IDC_RADIO1:case IDC_RADIO2:
	pdc->SetBkMode(TRANSPARENT);
	HBRUSH hbr=CreateSolidBrush(RGB(255,250,220));
	return hbr;

	}

return NULL;
}



/*                                                           */
/* When user presses the Connect Button this                 */
/* method will be called                                     */
/*                                                           */ 

void Display::Onconnect()
{
int port;
CString sername,username,mesg;	

	
	//Write to log file
	log.WriteString("\n Connecting to Server");

	//Get data from dialog box
	GetDlgItemText(IDC_EDIT1,sername);
	GetDlgItemText(IDC_EDIT3,username);
	port=GetDlgItemInt(IDC_EDIT2);
	
	if(username=="" || sername=="" )
	{
		AfxMessageBox("Please Enter the data properly");
		return;
	}
	
	if(success)
	{
		anicon->Play(0,-1,-1);
		log.WriteString("\n Playing data");
	}
	
	//Connect the client to server
	username.MakeUpper();
	sockclt.name=username;
	
	SetDlgItemText(IDC_BUTTON1,"Connecting");

	if(sockclt.Connect(sername,port))
		log.WriteString("\n Successfully Connected to Server");
	else
	{
		SetDlgItemText(IDC_BUTTON1,"&Connect");
		if(success)
		anicon->Stop();
		
		MessageBox("Unable to Connect to Server\nVerify if Server name and Port are correct");
		log.WriteString("\n Unable to connect to Server");
		
		
		return;
	}
	
	isconnected=TRUE;
	

	if(success)
	anicon->Stop();
	
	
	//Send the login message to server 
	mesg="NEW:"+username;

	if(sockclt.Send(mesg,mesg.GetLength()))
	log.WriteString("\n login Mesg sent to Server");
	else
	log.WriteString("\n Unable to send mesg to Server");
	
	updateState(FALSE,TRUE);

}




/*                                                           */
/* This function updates the state of dialog box             */
/* when the client is successfully connected to server.       */
/*                                                           */ 
void Display::updateState(BOOL pstate,BOOL nstate)
{
CEdit *eser,*eport,*euser;
CStatic *sser,*sport,*suser,*sgroup1,*sgroup2;
CButton *con;

		//Destroy server related items	
		con=(CButton*)GetDlgItem(IDC_BUTTON1);
		con->ShowWindow(pstate); 
		eser=(CEdit*)GetDlgItem(IDC_EDIT1);
		eser->ShowWindow(pstate); 
		
		
		eport=(CEdit*)GetDlgItem(IDC_EDIT2);
		eport->ShowWindow(pstate); 	
		
		euser=(CEdit*)GetDlgItem(IDC_EDIT3);
		euser->ShowWindow(pstate); 
		
		sser=(CStatic*)GetDlgItem(1098);
		sser->ShowWindow(pstate); 
		
		suser=(CStatic*)GetDlgItem(1086);
		suser->ShowWindow(pstate);

		
		sport=(CStatic*)GetDlgItem(1099);
		sport->ShowWindow(pstate); 

		sgroup1=(CStatic*)GetDlgItem(1097);
		sgroup1->ShowWindow(pstate); 



		//Enable Send to items
		sgroup2=(CStatic*)GetDlgItem(1075);
		sgroup2->ShowWindow(nstate);

		radio1=(CButton*)GetDlgItem(IDC_RADIO1);
		radio2=(CButton*)GetDlgItem(IDC_RADIO2);
		start=(CButton*)GetDlgItem(IDC_BUTTON5);
		stop=(CButton*)GetDlgItem(IDC_BUTTON6);
		
		
		radio1->ShowWindow(nstate);
		radio2->ShowWindow(nstate);

		radio1->SetCheck(1);
		radio2->SetCheck(0);

		start->ShowWindow(nstate);
		cbox->ShowWindow(nstate);
		if(doit==TRUE)
		talk->ShowWindow(TRUE);
		anicon->ShowWindow(FALSE);
	
		
}



/*											                  */	
/*  This function will be caled when the client               */
/*  receives message from server                              */ 
/*	(called form "mysocket" OnReceive function )              */	
/*											                  */	

void Display::Receive()
{
	char buff[PLAYBUFFER+25],str[PLAYBUFFER+60];
	int size=PLAYBUFFER+20,rcount,index,i,j,length;
	CString mesg,header,disp;



	rcount=sockclt.Receive(buff,size);

	if(rcount==SOCKET_ERROR)
	{
		log.WriteString("\nError in Receiving the data");
		return;
	}

	if(rcount>(PLAYBUFFER+20))
	{
		log.WriteString("\nMesg size exceeded buffer capacity");
		return;
	}
	

	buff[rcount]=NULL;
	
	mesg=buff;
	
//	sprintf(str,"Received data len= %d , %s ",rcount,&buff[20]);
//	log.WriteString(str);

	
	
	index=mesg.Find(':');
	
	
	//Invalid message Format --must have atleast one tag
	if(index==-1)
	return;
	
				
	header=mesg.Left(index);
				
	//Check if server has sent the list of clients
	if(header=="USER")
	{
		mesg=mesg.Right(mesg.GetLength()-index-1);
		updateList(mesg);
		return;
	}
	
	if(header=="WAIT")
	{
		if(isstart==0)  //stop the chat
		OnStop();
		start->EnableWindow(FALSE);
		mesg=mesg.Right(mesg.GetLength()-index-1);
		showFlash();
		//disp=" Please wait....\n "+mesg+" is broadcasting";
		//SetDlgItemText(6051,(char*)(LPCTSTR)disp);
		MessageBox("Please wait....\n "+mesg+" is broadcasting");
		return;				
	}
	
	if(header=="RESUME")
	{
		start->EnableWindow(TRUE);
		showFlash();
		disp=" Broadcasting is over \n Now You can continue...";
		SetDlgItemText(6051,(char*)(LPCTSTR)disp);
	//	MessageBox(" Broadcasting is over \n Now You can continue...");
		log.WriteString("\n****Received the resume message****");
		return;
	}
	
	
	else           //play the voice data
	{
		//No of messages received
		
		//***  Part of this code has to be removed later  ***\\
		
		
		//Get the legth of buffer
		length=0;
		sscanf(&buff[15],"%d",&length);
		
		if(length<1 || length>PLAYBUFFER)
		return;

		//If audio is not playing, start playing	
	
		
		if(play->Playing==FALSE)
			play->PostThreadMessage(WM_PLAYSOUND_STARTPLAYING,0,0);
		

		LPWAVEHDR lpHdr=playhead[curhead];
		curhead=(curhead+1)%MAXBUFFER;
	
		//	playmesg=new char[2020];
		for(i=20,j=0;j<length;i++,j++)
			lpHdr->lpData[j]=buff[i];

			lpHdr->dwBufferLength=length;
			lpHdr->dwFlags=0;

						
		play->PostThreadMessage(WM_PLAYSOUND_PLAYBLOCK,0,(LPARAM)lpHdr);
		
		//If Save button is pressed....
		//Save the Voice data from perticular user
		if(isSave==TRUE && writeuser.CompareNoCase(header)==0)
		{
			write->PostThreadMessage(WM_WRITESOUND_WRITEDATA,0,(LPARAM)lpHdr);
			log.WriteString("\n Writing to store");			
		}



//				log.WriteString("\nPlaying......");
	
		disp="  "+header+" is talking...";
		SetDlgItemText(6051,(char*)(LPCTSTR)disp);
/*
		if(from->FindStringExact(-1,header)==LB_ERR)
		{
			from->ResetContent();
			from->AddString(header);
		}
*/		
	//	reccount++;
	//	sprintf(str,"\n No of Messages received = %d ",reccount);
	//	log.WriteString(str);
	}
}
	



/*                                                             */
/*  This function will flash the window whenever               */
/*  important message has been received...shutdown,logout..etc */
/*                                                             */ 
 
void Display::showFlash()
{
	//flash for 3 times
	FlashWindow(TRUE);
	Sleep(200);
	FlashWindow(TRUE);
	

}

//
//  After pressing the Save button ...
//
//

void Display::OnStartWrite(char *name)
{

isSave=TRUE;

log.WriteString("\n Calling create fun of thread");
write->PostThreadMessage(WM_WRITESOUND_CREATEFILE,0,(LPARAM)name);
}



//
// After pressing the Save button ...later stop...
//

void Display::OnStopWrite()
{
isSave=FALSE;
write->PostThreadMessage(WM_WRITESOUND_CLOSEFILE,0,0);
}

/*															 */	
/*  This function will be called whenever user               */
/*  selects one of the radio button (*all or *client)        */
/*                                                           */ 

void Display::OnChange(int id)
{
CString bmesg;

	if(cbox->GetCount()<=0)
	return;
	
	if(id==IDC_RADIO1 && radio1->GetCheck())    //Send to client
	{
		cbox->EnableWindow(TRUE);
		
		if(cbox->GetCount()>=1)
		{
		curuser.Empty();
		cbox->GetLBText(cbox->GetCurSel(),curuser);
		log.WriteString("\nConnected to user "+curuser);
		selectflag=1;
		startRecording();
		}
		else
		{
		curuser.Empty();
		selectflag=0;
		stopRecording();
		 //stopPlaying();         //****** REMOVED ******//
		}	
	}
	
	if(id==IDC_RADIO2 && radio2->GetCheck())				//Send to All 
	{
	
	curuser="ALL";

	if(isstart==0)    //Stop the record and play 
	OnStop();
	
	log.WriteString("\n Broadcasting message");
	
	Broadcast bd(IDD_DIALOG5,this);
	bd.DoModal();
	bmesg="OVER:";
	selectflag=1;
	
	
	if(sockclt.Send(bmesg,bmesg.GetLength()))
	log.WriteString("\n Over Mesg sent to Server");
	else
	log.WriteString("\n Unable to send over mesg to Server");
	
	curuser.Empty();
	radio1->SetCheck(1);
	radio2->SetCheck(0);
	
		

	}

}

/*											                  */	
/*  This function will be called whenever client              */
/*  receives new userlist from the  server   and it will      */
/*  update the combo box                                      */ 
/*											                  */	

void Display::updateList(CString mesg)
{
int index,num,prevcount;
CString name;
static int first=0;
CString disp;
		
	//Clear the combo box contents
	prevcount=cbox->GetCount();
	
	cbox->ResetContent();
	
	do
	{
		index=mesg.Find(';');
	
		if(index==-1)
		break;
	
		name=mesg.Left(index);
		
		if(name.CompareNoCase(sockclt.name)!=0)
		cbox->AddString(name);
		
		mesg=mesg.Right(mesg.GetLength()-index-1);
		
	}
	while(TRUE);

	
	
	if(cbox->GetCount()==1)
	{
		cbox->SetCurSel(0);
		cbox->GetLBText(0,curuser);
		selectflag=1;
		if(prevcount==0 && first==1)
		{
		showFlash();
		disp="New user " + curuser + " logged in";
		SetDlgItemText(6051,(char*)(LPCTSTR)disp);
		}
	}
	
	first=1;

	if(cbox->GetCount()>1)
	{
		if(curuser.IsEmpty()==FALSE)
		{
			if((num=cbox->FindStringExact(-1,curuser))>=0)
			{
			selectflag=1;
			cbox->SetCurSel(num);					
			return;
			}
		}
			selectflag=0;
			OnStop();
			//		stopRecording();
			cbox->SetCurSel(0);
			
			if(curuser.IsEmpty()==FALSE && curuser!="ALL" )
			{
				showFlash();
				disp=curuser+" logged out  \n Please select the user";
				SetDlgItemText(6051,(char*)(LPCTSTR)disp);
	//			MessageBox(curuser+" logged out  ......Please select the user");
			}
			else if(curuser!="ALL")
			{
				
				showFlash();
				disp="New user logged in";
				SetDlgItemText(6051,(char*)(LPCTSTR)disp);

				//MessageBox("New user logged in....");
			
			}
	
	}
	
		
	if(cbox->GetCount()==0)
	{
		OnStop();
		//stopRecording();
		//stopPlaying();        //******* REMOVED *******//
		if(curuser.IsEmpty()==FALSE && curuser!="ALL")
		{
			showFlash();
			disp=curuser+" logged out";
			SetDlgItemText(6051,(char*)(LPCTSTR)disp);
		//	MessageBox(curuser+" logged out");
		}
		//SetDlgItemText(6051,"");
		//from->ResetContent();   //Clear the From Listbox
		curuser.Empty();
	}

}


/*															 */	
/*  This function will be called whenever user               */
/*  presses the start/stop  button                           */
/*  It updates the button status and starts recording and playing.                                                         */ 
/*															*/
void Display::OnStart()
{
	if(cbox->GetCount()<1)
	{
	MessageBox("No users are present");
	return;
	}
	
	if(!isstart)
	return;

	if(doit==TRUE)	
	talk->Play(0,-1,-1);

	cbox->GetLBText(cbox->GetCurSel(),curuser);
	selectflag=1;
	
	//Start recording and playing...
	startRecording();
	startPlaying();
	
	log.WriteString("\n New Target user is "+ curuser);
	//change state
	isstart=0;
	start->ShowWindow(FALSE);
	stop->ShowWindow(TRUE);

}



/*															 */	
/*  This function will be called whenever user               */
/*  presses the Stop  button                                 */
/*                                                           */ 
void Display::OnStop()
{
	
	if(isstart)
	return;
	
	if(doit==TRUE)
	talk->Stop();

	stopRecording();
	stopPlaying();
	selectflag=0;


	//change state
	isstart=1;
	start->ShowWindow(TRUE);
	stop->ShowWindow(FALSE);
}




//
//*** Part of this code needs to be removed later***
//
void Display::startRecording()
{
		if(record->recording==FALSE)
		{
//			if(sockclt.name.CompareNoCase("logname")==0)
			record->PostThreadMessage(WM_RECORDSOUND_STARTRECORDING,0,0);
		}

}


/*											                  */	
/*  This function will stop the recording by sending           */
/*  stop message to recording thread                          */
/*                                                            */ 

void Display::stopRecording()
{
		if(record->recording==TRUE)
		record->PostThreadMessage(WM_RECORDSOUND_STOPRECORDING,0,0);	 
}

/*											                  */	
/*  This function will stop the playing  by sending            */
/*  stop message to player thread.                            */
/*                                                            */ 
void Display::startPlaying()
{

	if(play->Playing==FALSE)
	play->PostThreadMessage(WM_PLAYSOUND_STARTPLAYING,0,0);

}


/*											                  */	
/*  This function will stop the playing  by sending            */
/*  stop message to player thread.                            */
/*                                                            */ 
void Display::stopPlaying()
{

	if(play->Playing==TRUE)
	play->PostThreadMessage(WM_PLAYSOUND_STOPPLAYING,0,0);

}


/*											                  */	
/*  This function will send the voice data to the server      */
/*  (called from the Recorder Thread)                         */
/*                                                            */ 
void Display::sendMessage(char *mesg,int length)
{
	char buflen[15];		
	char str[PLAYBUFFER+50];
	int i,j;		
		
		
		//If user not selected or no users are present return;
		if(selectflag==0 )
		return;

		if(mesg==NULL)
		{
		log.WriteString("\nEmpty buffer received from recordsound");
		return;
		}
		
		if(length<1 || length>PLAYBUFFER)
		{
		log.WriteString("\n Send : Length is more than the 2000 or less than 1");
		return;
		}	
		
		mesg=mesg-20;	

	 
		//First 15 bits  contains the destination information
		//Next   5 bits contains the length of message
		//Remaining part contains the data

		if(curuser.IsEmpty())
		{
		log.WriteString("\n Curuser is Empty- No user selected");
		return;
		}

		for(i=0;i < curuser.GetLength();i++)
		mesg[i]=curuser[i];
		mesg[i]=':';
		
		
		sprintf(buflen,"%d",length);

		for(j=0,i=15;j<5;j++,i++)
		mesg[i]=buflen[j];


		
	//Write to log file
	sprintf(str,"\n Sending message datalen = %d , mesg = %s",length,&mesg[20]);
	log.WriteString(str);
	
//	sendcount++;
//	sprintf(str,"\n No of Messages Sent = %d ",sendcount);
//	log.WriteString(str);
	
	if(sockclt.Send(mesg,length+20)!=SOCKET_ERROR )
	log.WriteString("\n Data sent successfully to "+curuser);
	else
	log.WriteString("\n Data sent Error ");
	
}




/*											                  */	
/*  Display the About Dialog Box                              */
/*                                                            */ 


void Display::OnAbout()
{
	about my(IDD_DIALOG2);
	my.DoModal();
}


/*											                  */	
/*  Display the About Dialog Box                              */
/*                                                            */ 
void Display::OnSave()
{

	if(curuser.IsEmpty())
	{
	MessageBox("\n First connect to one of the user...");
	return;
	}	

writeuser=curuser;
	
Save sa(IDD_DIALOG4,this);
sa.DoModal();
}



/*											                  */	
/*  This function will destroy the client window and          */
/*  closes all threads and socket connecion                   */ 
/*											                  */	

void Display::OnCancel()
{
	
	log.WriteString("\n Closing the dialog box");

	//Stop the recording and playing threads
	
	if(record->recording==TRUE)
	record->PostThreadMessage(WM_RECORDSOUND_STOPRECORDING,0,0);
	
	record->PostThreadMessage(WM_RECORDSOUND_ENDTHREAD,0,0);

	if(play->Playing==TRUE)
	play->PostThreadMessage(WM_PLAYSOUND_STOPPLAYING,0,0);
	
	play->PostThreadMessage(WM_PLAYSOUND_ENDTHREAD,0,0);

		  
	write->PostThreadMessage(WM_WRITESOUND_ENDTHREAD,0,0);

	if(sockclt.closeflag==1)
	{
		showFlash();
		MessageBox("Server has shutdown");
	}
	if(isconnected==TRUE)	
	sockclt.Close();	

	CDialog::OnCancel();
}


/*											                  */	
/*  This function will display the volume setting             */
/*  dialog box.                                               */ 
/*											                  */	

void Display::OnVolume()
{
Volume vol(IDD_DIALOG3);
vol.DoModal();

}


/*											                  */	
/*  This function will play the stored sound                  */
/*                                                            */ 
/*											                  */	

void Display::OnPlay()
{
	
CFileDialog fd(1,0,0,0,"Sound Files(*.wav)|*.wav|All Files|*.*||" );
fd.m_ofn.lpstrTitle="Browse";

	if(isstart==0)
	{
		MessageBox("Please stop chatting...");
		return;
	}
	if(fd.DoModal()==IDOK)
	{
	sndPlaySound(fd.GetPathName(),SND_ASYNC);
	}

}