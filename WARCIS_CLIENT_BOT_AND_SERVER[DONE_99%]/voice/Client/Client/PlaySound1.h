/// PlaySound1.h: interface for the PlaySound1 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PlaySound1_H__EA26EF42_4169_11D6_8886_F00753C10001__INCLUDED_)
#define AFX_PlaySound1_H__EA26EF42_4169_11D6_8886_F00753C10001__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#define WM_PLAYSOUND_STARTPLAYING WM_USER+600
#define WM_PLAYSOUND_STOPPLAYING WM_USER+601
#define WM_PLAYSOUND_PLAYBLOCK WM_USER+602
#define WM_PLAYSOUND_ENDTHREAD  WM_USER+603

#define SOUNDSAMPLES 1000
#define PLAYBUFFER   2000
#define SAMPLEPSEC   8000


#include<afxmt.h>
#include<mmsystem.h>


class PlaySound1 : public CWinThread  
{
DECLARE_DYNCREATE(PlaySound1)

public:
	WAVEFORMATEX m_WaveFormatEx;
	BOOL Playing;
	HWAVEOUT m_hPlay;
	CStdioFile log;
	CDialog *dlg;


	PlaySound1();
	PlaySound1(CDialog *dlg);
	virtual ~PlaySound1();
	BOOL InitInstance();
	int ExitInstance();
	void displayError(int code,char []);
	void displayHeader(LPWAVEHDR lphdr);
	LPWAVEHDR CreateWaveHeader(CString mesg);
	void ProcessSoundData(short int *sound, DWORD dwSamples);
	void GetDevProperty();
	
	afx_msg LRESULT OnStartPlaying(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnStopPlaying(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEndPlaySound1Data(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWriteSoundData(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEndThread(WPARAM wParam, LPARAM lParam);
	
	DECLARE_MESSAGE_MAP()

};

#endif // !defined(AFX_PlaySound1_H__EA26EF42_4169_11D6_8886_F00753C10001__INCLUDED_)
