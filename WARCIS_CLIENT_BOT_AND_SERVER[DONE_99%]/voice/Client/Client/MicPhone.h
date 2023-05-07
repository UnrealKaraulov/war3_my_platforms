// MicPhone.h: interface for the MicPhone class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MICPHONE_H__12303AA1_4AF2_11D6_8886_200654C10000__INCLUDED_)
#define AFX_MICPHONE_H__12303AA1_4AF2_11D6_8886_200654C10000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include<mmsystem.h>

class MicPhone  
{
public:


	HMIXER m_mixer;
	MIXERCAPS mixcap;
	
	DWORD mitems,mindex,mcontrolid,mcontype;
	CString micname,mixname,conname;
	CStdioFile log;

	void SetMicrophone();
	BOOL openMixer();
	BOOL getMicControl();
	BOOL selectMic(int value);
	void closeMixer();

};

#endif // !defined(AFX_MICPHONE_H__12303AA1_4AF2_11D6_8886_200654C10000__INCLUDED_)
