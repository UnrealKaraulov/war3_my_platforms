
// PlaySound1.cpp: implementation of the CPlaySound1 class.
//
//////////////////////////////////////////////////////////////////////
#include<afxwin.h>
#include<mmsystem.h>
#include<mmreg.h>
#include "PlaySound1.h"
#include "Display.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(PlaySound1, CWinThread)

BEGIN_MESSAGE_MAP(PlaySound1, CWinThread)
	ON_THREAD_MESSAGE(WM_PLAYSOUND_STARTPLAYING, OnStartPlaying)
	ON_THREAD_MESSAGE(WM_PLAYSOUND_STOPPLAYING, OnStopPlaying)
	ON_THREAD_MESSAGE(WM_PLAYSOUND_PLAYBLOCK, OnWriteSoundData)
	ON_THREAD_MESSAGE(MM_WOM_DONE, OnEndPlaySound1Data)
	ON_THREAD_MESSAGE(WM_PLAYSOUND_ENDTHREAD,OnEndThread)
END_MESSAGE_MAP()




PlaySound1::PlaySound1()
{


}

PlaySound1::PlaySound1(CDialog *dialog)
{
	
	log.Open("playfile.txt",CFile::modeCreate | CFile::modeWrite);
	
	dlg=dialog;
	
	GetDevProperty();

	memset(&m_WaveFormatEx,0x00,sizeof(m_WaveFormatEx));
	m_WaveFormatEx.wFormatTag = WAVE_FORMAT_PCM;
	m_WaveFormatEx.nChannels = 1;
	m_WaveFormatEx.wBitsPerSample = 8;
	m_WaveFormatEx.cbSize = 0;
	m_WaveFormatEx.nSamplesPerSec = SAMPLEPSEC;
	m_WaveFormatEx.nAvgBytesPerSec = SAMPLEPSEC ;
	m_WaveFormatEx.nBlockAlign = 1;
	
	//m_pSemaphore = new CSemaphore(2,2);

	Playing = FALSE;
	
	log.WriteString("\n In the constructor of Play sound");
}


PlaySound1::~PlaySound1()
{
log.Close();
}


void PlaySound1::GetDevProperty()
{
CString format;
WAVEOUTCAPS wavecap;
int propno[]= { 
				WAVECAPS_LRVOLUME ,
				WAVECAPS_PITCH ,
				WAVECAPS_PLAYBACKRATE,
				WAVECAPS_SYNC ,
				WAVECAPS_VOLUME,
				WAVECAPS_SAMPLEACCURATE ,
				};

CString propstr[]={
				"WAVECAPS_LRVOLUME ",
				"WAVECAPS_PITCH ",
				"WAVECAPS_PLAYBACKRATE",
				"WAVECAPS_SYNC" ,
				"WAVECAPS_VOLUME",
				"WAVECAPS_SAMPLEACCURATE" ,
				};

	//Special property
	format.Empty();
	format="\nSpecial properties... \n";
		for(int j=0;j<6;j++)
		{
			if( (wavecap.dwSupport & (unsigned)propno[j]) ==(unsigned) propno[j])
			{
			format+=propstr[j]+"\n";	
			}
		}
	
	log.WriteString(format);


}




BOOL PlaySound1::InitInstance()
{

return TRUE;
}


int PlaySound1::ExitInstance()
{
	return CWinThread::ExitInstance();
}


//Called from the Display class for start playing....

LRESULT PlaySound1::OnStartPlaying(WPARAM wParam, LPARAM lParam)
{
	MMRESULT mmReturn = 0;

	if(Playing==TRUE)
		return FALSE;

	log.WriteString("\n Starting playing");
	
		// open wavein device
		 mmReturn = ::waveOutOpen( &m_hPlay, WAVE_MAPPER,
			&m_WaveFormatEx, ::GetCurrentThreadId(), 0, CALLBACK_THREAD);
		
		if(mmReturn )
			displayError(mmReturn,"PlayStart");	
		else
		{	
			Playing = TRUE;
			DWORD volume=0xffffffff;
			char str[100];
			
			if(!waveOutSetVolume(m_hPlay,volume))
			{
				volume=0;
				if(!waveOutGetVolume(m_hPlay,&volume))
				{
					wsprintf(str,"\n Volume is  %lx",volume);
					log.WriteString(str);
				}
								
			}	

		}			

		
return TRUE;
}



void PlaySound1::displayError(int code,char mesg[])
{

char errorbuffer[MAX_PATH];
char errorbuffer1[MAX_PATH];
waveOutGetErrorText( code,errorbuffer,MAX_PATH);
sprintf(errorbuffer1,"PLAY : %s :%x:%s",mesg,code,errorbuffer);
AfxMessageBox(errorbuffer1);  

}


//Called from the Display class for stopping playing....by sending
// PostThreadMessage ....

LRESULT PlaySound1::OnStopPlaying(WPARAM wParam, LPARAM lParam)
{
	
	MMRESULT mmReturn = 0;

		if(Playing==FALSE)
		return FALSE;

        log.WriteString("\n Stop playing");
		
		
		mmReturn = ::waveOutReset(m_hPlay);
		
		if(!mmReturn)
		{
			Playing = FALSE;
			Sleep(500);
			mmReturn = ::waveOutClose(m_hPlay);
		}

		return mmReturn;
}



LRESULT PlaySound1::OnEndPlaySound1Data(WPARAM wParam, LPARAM lParam)
{
	LPWAVEHDR lpHdr = (LPWAVEHDR) lParam;
	
	if(lpHdr)
	{
		::waveOutUnprepareHeader(m_hPlay, lpHdr, sizeof(WAVEHDR));
		
	
//		m_pSemaphore->Unlock();
	}

	return ERROR_SUCCESS;
}


//
// Called from Display class for playing received data....using
// PostThreadMessage....
//

LRESULT PlaySound1::OnWriteSoundData(WPARAM wParam, LPARAM lParam)
{
	MMRESULT mmResult = 0;
	
	LPWAVEHDR lpHdr=(LPWAVEHDR)lParam;

	
	if(lpHdr==NULL)
	return ERROR_SUCCESS;
		
		if(Playing)
		{
			
			mmResult = ::waveOutPrepareHeader(m_hPlay, lpHdr, sizeof(WAVEHDR));
					
			if(mmResult)
			{
				log.WriteString("\nError while preparing header");
				return ERROR_SUCCESS;
			}
		
			mmResult = ::waveOutWrite(m_hPlay, lpHdr, sizeof(WAVEHDR));
			
			if(mmResult)
			{
				log.WriteString("\nError while writing to device");
				return ERROR_SUCCESS;
				
			}
		}



	return ERROR_SUCCESS;
}





LRESULT PlaySound1::OnEndThread(WPARAM wParam, LPARAM lParam)
{
	
	if(Playing==TRUE)
	OnStopPlaying(0,0);
	
	log.WriteString("\nEnding the play device");

	::PostQuitMessage(0);
	return TRUE;
}
