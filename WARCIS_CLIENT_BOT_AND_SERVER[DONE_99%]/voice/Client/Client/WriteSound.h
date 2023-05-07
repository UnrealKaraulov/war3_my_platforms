// WriteSound.h: interface for the WriteSound class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WRITESOUND_H__34AE8561_55E6_11D6_8886_801253C10001__INCLUDED_)
#define AFX_WRITESOUND_H__34AE8561_55E6_11D6_8886_801253C10001__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#define WM_WRITESOUND_CREATEFILE    WM_USER+701
#define WM_WRITESOUND_WRITEDATA		WM_USER+702
#define WM_WRITESOUND_CLOSEFILE		WM_USER+703
#define WM_WRITESOUND_ENDTHREAD		WM_USER+704

#define SAMPLEWSEC 8000

class WriteSound : public CWinThread  
{

DECLARE_DYNCREATE(WriteSound);
public:
	MMCKINFO riffblock,fmtblock,datablock;
	HMMIO m_hwrite;		
	WAVEFORMATEX waveformat;
	CStdioFile log;


	WriteSound();
	virtual ~WriteSound();
	BOOL InitInstance();
	int ExitInstance();
	afx_msg LRESULT OnCreateFile(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWriteData(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnCloseFile(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEndThread(WPARAM wParam, LPARAM lParam);

	
DECLARE_MESSAGE_MAP()
};


#endif // !defined(AFX_WRITESOUND_H__34AE8561_55E6_11D6_8886_801253C10001__INCLUDED_)
