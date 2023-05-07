//
// RecordSound.cpp: implementation of the RecordSound class.
//
//////////////////////////////////////////////////////////////////////

#include<afxwin.h>
#include<mmsystem.h>
#include<mmreg.h>
#include "RecordSound.h"
#include "Display.h"
#include "MicPhone.h"
#include "MicMute.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(RecordSound, CWinThread)


BEGIN_MESSAGE_MAP(RecordSound,CWinThread)
ON_THREAD_MESSAGE(MM_WIM_DATA, OnSoundData)
ON_THREAD_MESSAGE(WM_RECORDSOUND_STARTRECORDING,OnStartRecording)
ON_THREAD_MESSAGE(WM_RECORDSOUND_STOPRECORDING,OnStopRecording)
ON_THREAD_MESSAGE(WM_RECORDSOUND_ENDTHREAD,OnEndThread)
END_MESSAGE_MAP()


RecordSound::RecordSound()
{

}


RecordSound::RecordSound(CDialog *dialog)
{
	dlg=dialog;
	
	log.Open("recfile.txt",CFile::modeCreate | CFile::modeWrite);
	log.WriteString("In the Recordsound Constructor\n");
	
	
	//Select the microphone
	MicPhone *mic=new MicPhone;
	mic->SetMicrophone();

	MicMute *mute=new MicMute;
	mute->SetMicrophone();


	recording=FALSE;
	isallocated=0;      //memory is not allocated to wavebuffer

	GetDevProperty();

	//Create Headers for buffering
	PreCreateHeader();

	//Setting WAVEFORMATEX  structure for the audio input
	memset(&m_WaveFormatEx,0x00,sizeof(m_WaveFormatEx));
	
	m_WaveFormatEx.wFormatTag=WAVE_FORMAT_PCM;
	m_WaveFormatEx.nChannels=1;
	m_WaveFormatEx.wBitsPerSample=8;
	m_WaveFormatEx.cbSize=0;
	m_WaveFormatEx.nSamplesPerSec=SAMPLERSEC;  //22.05 KHz
	
	m_WaveFormatEx.nBlockAlign=1; //(m_WaveFormatEx.wBitsPerSample/8)*m_WaveFormatEx.nChannels;
	
	m_WaveFormatEx.nAvgBytesPerSec=SAMPLERSEC ;  //m_WaveFormatEx.nBlockAlign;


}


RecordSound::~RecordSound()
{
log.Close();
	if(!isallocated)
	return;

	for(int i=0;i<MAXRECBUFFER;i++)
	{
		if(rechead[i])
		delete rechead[i];
	}
}


BOOL RecordSound::InitInstance()
{
return TRUE;
}

int RecordSound::ExitInstance()
{
	return CWinThread::ExitInstance();
}


void RecordSound::PreCreateHeader()
{

for(int i=0;i<MAXRECBUFFER;i++)
rechead[i]=CreateWaveHeader();

isallocated=1;
}




void RecordSound::GetDevProperty()
{
WAVEINCAPS wavecap;
int i,j,n=waveInGetNumDevs();
char str[100];
CString format;	
int form[]={WAVE_FORMAT_1M08, 
			WAVE_FORMAT_1M16,
			WAVE_FORMAT_1S08,
			WAVE_FORMAT_1S16,
			WAVE_FORMAT_2M08,
			WAVE_FORMAT_2M16,
			WAVE_FORMAT_2S08,
			WAVE_FORMAT_2S16,
			WAVE_FORMAT_4M08,
			WAVE_FORMAT_4M16,
			WAVE_FORMAT_4S08,
			WAVE_FORMAT_4S16,
			};	
	
CString fstr[]={
		    "WAVE_FORMAT_1M08", 
			"WAVE_FORMAT_1M16",
			"WAVE_FORMAT_1S08",
			"WAVE_FORMAT_1S16",
			"WAVE_FORMAT_2M08",
			"WAVE_FORMAT_2M16",
			"WAVE_FORMAT_2S08",
			"WAVE_FORMAT_2S16",
			"WAVE_FORMAT_4M08",
			"WAVE_FORMAT_4M16",
			"WAVE_FORMAT_4S08",
			"WAVE_FORMAT_4S16",
			};


sprintf(str,"\n Total no of devices = %d ",n);
log.WriteString(str);
	
	for(i=0;i<n;i++)
	{
	waveInGetDevCaps(i,&wavecap,sizeof(wavecap));
	sprintf(str,"\n Product Name = %s ",wavecap.szPname);
	log.WriteString(str);
	sprintf(str,"\n No of channels %d ",wavecap.wChannels);

	format.Empty();
	format="\nIt supports  \n";
		for(j=0;j<12;j++)
		{
			if( (wavecap.dwFormats & (unsigned)form[j]) ==(unsigned) form[j])
			{
			format+=fstr[j]+"\n";	
			}
		}
	
		log.WriteString(format);
	

	
		

	}

}



LRESULT RecordSound::OnStartRecording(WPARAM wp,LPARAM lp)
{

	MMRESULT mmReturn = ::waveInOpen( &m_hRecord, WAVE_MAPPER,
			&m_WaveFormatEx, ::GetCurrentThreadId(), 0, CALLBACK_THREAD);
	
	log.WriteString("In OnStartrecording\n");
	//Error has occured while opening device
		
	if(mmReturn!=MMSYSERR_NOERROR )
	{
		displayError(mmReturn,"Open");
		return FALSE;
	}		
			
		if(mmReturn==MMSYSERR_NOERROR )
		{
			
			for(int i=0; i < MAXRECBUFFER ; i++)
			{
				mmReturn = ::waveInPrepareHeader(m_hRecord,rechead[i], sizeof(WAVEHDR));
				mmReturn = ::waveInAddBuffer(m_hRecord, rechead[i], sizeof(WAVEHDR));
			}
				
			
					
			mmReturn = ::waveInStart(m_hRecord);
			
					
					if(mmReturn!=MMSYSERR_NOERROR )
					displayError(mmReturn,"Start");
					else
					recording=TRUE;
					
		}

return TRUE;
}

	
void RecordSound::displayError(int mmReturn,char errmsg[])
{
char errorbuffer[MAX_PATH];
char errorbuffer1[MAX_PATH];

	waveInGetErrorText( mmReturn,errorbuffer,MAX_PATH);
	sprintf(errorbuffer1,"RECORD: %s : %x : %s",errmsg,mmReturn,errorbuffer);
	AfxMessageBox(errorbuffer1);  
}




LRESULT RecordSound::OnStopRecording(WPARAM wp,LPARAM lp)
{
MMRESULT mmReturn = 0;

log.WriteString("\nIn the onstop recording");	
	
		if(!recording)
		return FALSE;

		mmReturn = ::waveInStop(m_hRecord);
		
		
		//To get the remaining sound data from buffer
		//Reset will call OnSoundData function	
		
		if(!mmReturn)
		{
			recording = FALSE;
			mmReturn = ::waveInReset(m_hRecord);  
		}
		
	
	/****  Code has been changed ****/	
	//	if(!mmReturn)
	//		recording = FALSE;
		
		Sleep(500); 
	
			if(!mmReturn)
			mmReturn = ::waveInClose(m_hRecord);
	
		return mmReturn;

}

//
//ON_WIM_DATA CALL BACK FUNCTION
// It will call the Display class's sendmessage fun which will send the message to server
//


LRESULT RecordSound::OnSoundData(WPARAM wParam, LPARAM lParam)
{
	log.WriteString("\nIn the onsound data");

LPWAVEHDR lpHdr = (LPWAVEHDR) lParam;

	if(lpHdr->dwBytesRecorded==0 || lpHdr==NULL)
	return ERROR_SUCCESS;

	

		//short int * lpInt = (short int*) lpHdr->lpData;
		//DWORD cbRecorded = lpHdr->dwBytesRecorded;
		//	ProcessSoundData(lpInt, cbRecorded/sizeof(short int));
		
		
	    ::waveInUnprepareHeader(m_hRecord, lpHdr, sizeof(WAVEHDR));
	
		//Send message to server
		if(lpHdr->lpData!=NULL )
		((Display*)dlg)->sendMessage(lpHdr->lpData,lpHdr->dwBytesRecorded);
		

		//Reuse this buffer for the next capture.....
		//No dynamic creation of buffers......happy running..
		if(recording)
		{
			//Reuse the old buffer
			::waveInPrepareHeader(m_hRecord,lpHdr, sizeof(WAVEHDR));
			::waveInAddBuffer(m_hRecord, lpHdr, sizeof(WAVEHDR));
		}


return ERROR_SUCCESS;

}






LRESULT RecordSound::OnEndThread(WPARAM wp,LPARAM lp)
{
	log.WriteString("\nIN the onendthread");	
	
	if(recording)
	OnStopRecording(0,0);

	::PostQuitMessage(0);
	return TRUE;
}




LPWAVEHDR  RecordSound::CreateWaveHeader()
{
	LPWAVEHDR lpHdr = new WAVEHDR;
	
	if(lpHdr==NULL)
	{
		log.WriteString("\n Unable to allocate the memory");
		return NULL;
	}
	
	ZeroMemory(lpHdr, sizeof(WAVEHDR));
	char* lpByte = new char[RECBUFFER+20];//m_WaveFormatEx.nBlockAlign*SOUNDSAMPLES)];
	
	if(lpByte==NULL)
	{
		log.WriteString("\n Unable to allocate the memory");
		return NULL;
	}
	lpHdr->lpData =  &lpByte[20];
	lpHdr->dwBufferLength =RECBUFFER;   // (m_WaveFormatEx.nBlockAlign*SOUNDSAMPLES);
	return lpHdr;

}

