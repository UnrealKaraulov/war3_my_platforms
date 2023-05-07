////////////////////////////////////////////////////////////////////
// Stores the sound to disk...Paul Cheffers code...directly used here
//
// 
// WriteSound.cpp: implementation of the WriteSound class.
//
//////////////////////////////////////////////////////////////////////
#include<afxwin.h>
#include<mmsystem.h>
#include "WriteSound.h"


IMPLEMENT_DYNCREATE(WriteSound, CWinThread)


BEGIN_MESSAGE_MAP(WriteSound,CWinThread)
ON_THREAD_MESSAGE(WM_WRITESOUND_CREATEFILE,OnCreateFile)
ON_THREAD_MESSAGE(WM_WRITESOUND_WRITEDATA,OnWriteData)
ON_THREAD_MESSAGE(WM_WRITESOUND_CLOSEFILE,OnCloseFile)
ON_THREAD_MESSAGE(WM_WRITESOUND_ENDTHREAD,OnEndThread)
END_MESSAGE_MAP()

WriteSound::WriteSound()
{
	log.Open("write.txt",CFile::modeCreate | CFile::modeWrite);
	log.WriteString("\n Starting writing thread");
	
	m_hwrite=NULL;

	//set up the WAVEFORMATEX structure
	waveformat.cbSize=0;
	waveformat.wFormatTag=WAVE_FORMAT_PCM;
	waveformat.nChannels=1;
	waveformat.wBitsPerSample=8;
	waveformat.nAvgBytesPerSec=SAMPLEWSEC;
	waveformat.nSamplesPerSec=SAMPLEWSEC;
	waveformat.nBlockAlign=1;



}

WriteSound::~WriteSound()
{
log.Close();
}


BOOL WriteSound::InitInstance()
{
return TRUE;
}


int WriteSound::ExitInstance()
{
	return CWinThread::ExitInstance();
}


LRESULT WriteSound::OnCreateFile(WPARAM wParam, LPARAM lParam)
{
int cbsize;
MMRESULT mmret;
char *filename=(char*)lParam;
char str[200];
sprintf(str,"filename is %s ",filename);

log.WriteString(str);

log.WriteString("\n Creating new file ");


m_hwrite=::mmioOpen(filename,NULL,MMIO_CREATE |MMIO_WRITE | MMIO_EXCLUSIVE | MMIO_ALLOCBUF);
	
	if(filename)
	delete filename;


	if(m_hwrite==NULL)
	{
	log.WriteString("Unable to create the specified file");
	return FALSE;
	}
	//AfxMessageBox("File created ");
	log.WriteString("\n File created successfully");

	ZeroMemory(&riffblock,sizeof(MMCKINFO));
	ZeroMemory(&fmtblock,sizeof(MMCKINFO));
	ZeroMemory(&datablock,sizeof(MMCKINFO));

	riffblock.fccType=mmioFOURCC('W','A','V','E');
	mmret=mmioCreateChunk(m_hwrite,&riffblock,MMIO_CREATERIFF);
	
	if(mmret!=MMSYSERR_NOERROR)
	{
	log.WriteString("\n Riff format writing error");
	return FALSE;
	}


	fmtblock.ckid=mmioFOURCC('f','m','t',' ');
	cbsize=sizeof(WAVEFORMATEX)-2;
	fmtblock.cksize=cbsize;
	mmret=mmioCreateChunk(m_hwrite,&fmtblock,0);
	
	if(mmret!=MMSYSERR_NOERROR)
	{
	log.WriteString("\n fmt format writing error");
	return FALSE;
	}
	
	::mmioWrite(m_hwrite,(const char*)&waveformat,cbsize);

	datablock.ckid=mmioFOURCC('d','a','t','a');
	mmret=mmioCreateChunk(m_hwrite,&datablock,0);
	
	if(mmret!=MMSYSERR_NOERROR)
	{
	log.WriteString("\n data format writing error");
	return FALSE;
	}

	log.WriteString("\n Format details written successfully");
return TRUE;
}
	

LRESULT WriteSound::OnWriteData(WPARAM wParam, LPARAM lParam)
{
LPWAVEHDR lphdr=(LPWAVEHDR)lParam;
int length=lphdr->dwBufferLength;

	if(m_hwrite==NULL)
	return FALSE;

	log.WriteString("\n Writing the data");

	if(lphdr->lpData && length>0)
	mmioWrite(m_hwrite,lphdr->lpData,length);


return TRUE;
}


LRESULT WriteSound::OnCloseFile(WPARAM wParam, LPARAM lParam)
{

log.WriteString("\n Closing the file");


	if(m_hwrite)
	{
		::mmioAscend(m_hwrite,&datablock,0);
		::mmioAscend(m_hwrite,&riffblock,0);
		::mmioClose(m_hwrite,0);
	}

log.WriteString("\n Successfully closed");

return TRUE;

}

LRESULT WriteSound::OnEndThread(WPARAM wParam, LPARAM lParam)
{
log.WriteString("\n In the OnEndThread fun");

OnCloseFile(0,0);
::PostQuitMessage(0);

return TRUE;

}

