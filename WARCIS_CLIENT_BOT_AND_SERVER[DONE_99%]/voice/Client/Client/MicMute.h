// MicMute.h: interface for the MicMute class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MICMUTE_H__7ABBC9A6_528A_11D6_8886_F00753C10001__INCLUDED_)
#define AFX_MICMUTE_H__7ABBC9A6_528A_11D6_8886_F00753C10001__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include<mmsystem.h>

class MicMute  
{
public:


	HMIXER m_mixer;
	MIXERCAPS mixcap;
	MIXERCONTROLDETAILS mxcd;

	DWORD mitems,mindex,mcontrolid,mcontype;
	CString micname,mixname,conname;
	CStdioFile log;

	void SetMicrophone();
	BOOL openMixer();
	BOOL getMicControl();
	BOOL selectMic(int value);
	void closeMixer();

};

#endif // !defined(AFX_MICMUTE_H__7ABBC9A6_528A_11D6_8886_F00753C10001__INCLUDED_)
