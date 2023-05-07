///////////////////////////////////////////////////////////////////////////// 
// 
// MicMute.cpp: implementation of the MicMute class.
// Checks the MicMute option from playing option to prevent playback
// ( Open Volume Control (Windows) and check it....
// 
//////////////////////////////////////////////////////////////////////



#include<afxwin.h>
#include <mmsystem.h>
#include"MicMute.h"
  



void MicMute::SetMicrophone()
{

	log.Open("micmute.txt",CFile::modeCreate | CFile::modeWrite);

	if(openMixer())
	{
		if(getMicControl())
		{
			log.WriteString("\n Got the micmute control");
			selectMic(1);
		
		}
		else
		log.WriteString("\nGet mic control failed");

		closeMixer();
	}
	else
	log.WriteString("\nOpen mixer failed");

log.Close();
}


BOOL MicMute::openMixer()
{
int n;

	//Get the no of mixer devices
	n=mixerGetNumDevs();
	
	m_mixer=NULL;
	::ZeroMemory(&mixcap,sizeof(MIXERCAPS));

	if(n==0)
	return FALSE;

		//Open the first mixer device
		if(mixerOpen(&m_mixer,0,NULL,NULL,MIXER_OBJECTF_MIXER)!=MMSYSERR_NOERROR)
		return FALSE;
		
		//Get mixer properties
		if( ::mixerGetDevCaps((UINT)m_mixer,&mixcap, sizeof(MIXERCAPS) )!=MMSYSERR_NOERROR)
		return FALSE;
		
 	    return TRUE;
}

void MicMute::closeMixer()
{
	if(m_mixer!=NULL)
	::mixerClose(m_mixer);	

}



BOOL MicMute::getMicControl()
{
int mmret;
MIXERLINE mxln;
MIXERCONTROL mxcon;
MIXERLINECONTROLS mxlncon;


log.WriteString("\n In the getMicControl");

mxln.cbStruct=(sizeof(MIXERLINE));
mxln.dwComponentType=MIXERLINE_COMPONENTTYPE_DST_SPEAKERS ;


	
mmret=mixerGetLineInfo( (HMIXEROBJ)m_mixer,&mxln,MIXER_OBJECTF_HMIXER |
						   MIXER_GETLINEINFOF_COMPONENTTYPE );

	if(mmret!=MMSYSERR_NOERROR)
	{
		return FALSE;
	}

int count=mxln.cConnections;
int id=mxln.dwDestination;

	for(int j=0;j<count;j++)
	{
	mxln.cbStruct=sizeof(MIXERLINE);
	mxln.dwDestination=id;
	mxln.dwSource=j;
	
	mmret=mixerGetLineInfo((HMIXEROBJ)m_mixer,&mxln,MIXER_OBJECTF_MIXER |
						MIXER_GETLINEINFOF_SOURCE);

	if(mmret!=MMSYSERR_NOERROR)
	{
		return FALSE;
	}
	
		if(mxln.dwComponentType==MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE )
		{
			log.WriteString("\nGot the source line id");
			break;
		}
	}

	if(j==count)
	{
		log.WriteString("\n Unable to get source line");
	return FALSE;
	}
	
	
	log.WriteString("\n Got the line info");	

		//Initialize the MIXERLINECONTROLS structure
	mcontype=MIXERCONTROL_CONTROLTYPE_MUTE;
	mxlncon.cbStruct=sizeof(MIXERLINECONTROLS);
	mxlncon.dwControlType=mcontype;
	mxlncon.cControls=1;
	mxlncon.dwLineID=mxln.dwLineID;
	mxlncon.cbmxctrl=sizeof(MIXERCONTROL);
	mxlncon.pamxctrl=&mxcon;
	
	mmret=mixerGetLineControls((HMIXEROBJ)m_mixer,&mxlncon,MIXER_GETLINECONTROLSF_ONEBYTYPE | MIXER_OBJECTF_HMIXER );
	
	if(mmret!=MMSYSERR_NOERROR)
	{
	
		log.WriteString("\n Failed to get the line control");
		return FALSE;
	}
	
	mxcd.cbStruct=sizeof(MIXERCONTROLDETAILS);
	mxcd.dwControlID=mxcon.dwControlID;
	mxcd.cChannels=mxln.cChannels;

	char str[200];
	sprintf(str,"\nno of items = %d , id = %d , channel = %d",mxcon.cMultipleItems,mxcon.dwControlID,mxln.cChannels);
	log.WriteString(str);

	return TRUE;   
	
}

BOOL MicMute::selectMic(int val)
{
	if (m_mixer == NULL)
	{
		return FALSE;
	}

	log.WriteString("\n Selecting mic...");
	MIXERCONTROLDETAILS_BOOLEAN *pValue =
		new MIXERCONTROLDETAILS_BOOLEAN;

	pValue->fValue=val;
	
	mxcd.cMultipleItems=0;
	mxcd.cbDetails=sizeof(MIXERCONTROLDETAILS_BOOLEAN);
	mxcd.paDetails=pValue;
	

	

	if( mixerSetControlDetails( ( HMIXEROBJ )m_mixer, &mxcd, 
			                      MIXER_SETCONTROLDETAILSF_VALUE | MIXER_OBJECTF_HMIXER ) 
		!= MMSYSERR_NOERROR )
	{
	log.WriteString("\nError while setting value");
	return FALSE;
	}

	log.WriteString("Mic selected properly");
	return TRUE;

}







