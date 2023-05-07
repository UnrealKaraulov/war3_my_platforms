// Display.h: interface for the Display class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DISPLAY_H__33C94244_415D_11D6_8886_200654C10000__INCLUDED_)
#define AFX_DISPLAY_H__33C94244_415D_11D6_8886_200654C10000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "mysocket.h"
#include "RecordSound.h"
#include "PlaySound1.h"
#include "WriteSound.h"
#include<afxcmn.h>

#define MAXBUFFER 12


class Display : public CDialog  
{
public:
	CStdioFile log;
	mysocket sockclt;
	CComboBox *cbox;
	RecordSound *record;
	PlaySound1 *play;
	WriteSound *write;
	CString curuser,writeuser;
	char *playmesg;
	//CListBox *from;
	CButton *radio1,*radio2,*start,*stop;
	CAnimateCtrl *anicon,*talk;
	
	LPWAVEHDR playhead[MAXBUFFER];
	int curhead;

	int selectflag;
	int isconnected;
	int isstart;
	int doit;
	
	int sendcount;
	int reccount;
	BOOL success;
	BOOL isSave;

	Display(int n);
	virtual ~Display();
	
	void PreCreateHeader();
	int OnInitDialog();
	void Onconnect();
	void Receive();
	void updateList(CString mesg);
	void OnAbout();
	void OnSave();
	void OnPlay();
	void OnChange(int id);
	void OnStart();
	void OnStop();
	void OnVolume();
	void OnPaint();
	void OnStartWrite(char *);
	void OnStopWrite();
	int OnEraseBkgnd(CDC *pdc);
	HBRUSH OnCtlColor(CDC *pdc,CWnd *pwnd,UINT ctrl);
	void OnCancel();
	void updateState(BOOL ,BOOL);
	void startRecording();
	void stopRecording();
	void startPlaying();
	void stopPlaying();

	void sendMessage(char *mesg,int length);
	void showFlash();	
DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_DISPLAY_H__33C94244_415D_11D6_8886_200654C10000__INCLUDED_)
