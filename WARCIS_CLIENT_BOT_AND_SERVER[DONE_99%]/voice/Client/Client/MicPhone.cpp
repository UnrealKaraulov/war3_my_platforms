// MicPhone.cpp -- To select the microphone from the recording option
//
//////////////////////////////////////////////////////////////////////


#include<afxwin.h>
#include <mmsystem.h>
#include"MicPhone.h"

/*******************  MIXER Information ************************/
/*   Sequence of Events...                                     */
/*   * Open Mixer											   */
/*       * Get no of audio mixer devices					   */	
/*       * Open the first mixer (0)							   */				
/*       * Get Properties of mixer							   */
/*	* GetMicControl											   */  	
/*		 * Get the destination line(Input) property.(Line id)  */ 
/*		 * Get the mixer control for that destination line.    */
/*       * Detect which item of mixer control controls the     */
/*		   microphone source line                              */
/*			                                                   */
/*  * SelectMic(int value)                                     */
/*		 * Get the value for all items of mixer control        */
/*       * Set the value(0 or 1) for the item with the         */ 
/*         perticular index (microphone)                       */
/*  * Close Mixer											   */	
/*       * Close the Mixer									   */	
/*															   */
/***************************************************************/
	
	  



void MicPhone::SetMicrophone()
{

	log.Open("mic.txt",CFile::modeCreate | CFile::modeWrite);

	if(openMixer())
	{
		if(getMicControl())
		{
			selectMic(1);
		
		}
		else
		log.WriteString("\nGet mic control failed");

		closeMixer();
	}
	else
	log.WriteString("\nOpen mixer failed");
}


BOOL MicPhone::openMixer()
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

void MicPhone::closeMixer()
{
	if(m_mixer!=NULL)
	::mixerClose(m_mixer);	
}



BOOL MicPhone::getMicControl()
{
int mmret;
MIXERLINE mxln;
MIXERCONTROL mxcon;
MIXERLINECONTROLS mxlncon;


mxln.cbStruct=(sizeof(MIXERLINE));
mxln.dwComponentType=MIXERLINE_COMPONENTTYPE_DST_WAVEIN;


	
mmret=mixerGetLineInfo( (HMIXEROBJ)m_mixer,&mxln,MIXER_OBJECTF_HMIXER |
						   MIXER_GETLINEINFOF_COMPONENTTYPE );

	if(mmret!=MMSYSERR_NOERROR)
	{
		return FALSE;
	}

	log.WriteString("\n Got the line info");	

		//Initialize the MIXERLINECONTROLS structure
	mcontype=MIXERCONTROL_CONTROLTYPE_MIXER;
	
	mxlncon.cbStruct=sizeof(MIXERLINECONTROLS);
	mxlncon.dwControlType=mcontype;
	mxlncon.cControls=1;
	mxlncon.dwLineID=mxln.dwLineID;
	mxlncon.cbmxctrl=sizeof(MIXERCONTROL);
	mxlncon.pamxctrl=&mxcon;
	
	mmret=mixerGetLineControls((HMIXEROBJ)m_mixer,&mxlncon,MIXER_GETLINECONTROLSF_ONEBYTYPE | MIXER_OBJECTF_HMIXER );
	
	


	log.WriteString("\n Got the line control info");

	if(mmret!=MMSYSERR_NOERROR)
	{
		// no mixer, try MUX
		log.WriteString("\n Once again find.. line control");

		mcontype = MIXERCONTROL_CONTROLTYPE_MUX;
		mxlncon.cbStruct = sizeof(MIXERLINECONTROLS);
		mxlncon.dwLineID = mxln.dwLineID;
		mxlncon.dwControlType = mcontype;
		mxlncon.cControls = 1;
		mxlncon.cbmxctrl = sizeof(MIXERCONTROL);
		mxlncon.pamxctrl = &mxcon;
		if (::mixerGetLineControls(reinterpret_cast<HMIXEROBJ>(m_mixer),
								   &mxlncon,
								   MIXER_OBJECTF_HMIXER |
								   MIXER_GETLINECONTROLSF_ONEBYTYPE)
			!= MMSYSERR_NOERROR)
		{
		return FALSE;
		}
	}
	
	

	mitems=mxcon.cMultipleItems;
	conname=mxcon.szName;
	mcontrolid=mxcon.dwControlID;


	



	if(mitems==0)   //SHOULD BE GREATER THAN 2
	{
		return FALSE;
	}
	log.WriteString("\n Finding src for microphone");
	
	// get the index of the Microphone Select control
	MIXERCONTROLDETAILS_LISTTEXT *pmxcdSelectText =
		new MIXERCONTROLDETAILS_LISTTEXT[mitems];

	if (pmxcdSelectText != NULL)
	{
		MIXERCONTROLDETAILS mxcd;
		mxcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
		mxcd.dwControlID = mxcon.dwControlID;
		mxcd.cChannels = 1;
		mxcd.cMultipleItems = mitems;
		mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_LISTTEXT);
		mxcd.paDetails = pmxcdSelectText;
		if (::mixerGetControlDetails(reinterpret_cast<HMIXEROBJ>(m_mixer),
									 &mxcd,
									 MIXER_OBJECTF_HMIXER |
									 MIXER_GETCONTROLDETAILSF_LISTTEXT)
			== MMSYSERR_NOERROR)
		{
			// determine which controls the Microphone source line
			for (DWORD dwi = 0; (signed)dwi < mitems; dwi++)
			{
				// get the line information
				MIXERLINE mxl;
				mxl.cbStruct = sizeof(MIXERLINE);
				mxl.dwLineID = pmxcdSelectText[dwi].dwParam1;
				if (::mixerGetLineInfo(reinterpret_cast<HMIXEROBJ>(m_mixer),
									   &mxl,
									   MIXER_OBJECTF_HMIXER |
									   MIXER_GETLINEINFOF_LINEID)
					== MMSYSERR_NOERROR &&
					mxl.dwComponentType == MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE)
				{
					// found, dwi is the index.
					mindex = dwi;
					micname = pmxcdSelectText[dwi].szName;
					break;
				}
			}

			if ((signed)dwi >= (signed)mitems)
			{
				// could not find it using line IDs, some mixer drivers have
				// different meaning for MIXERCONTROLDETAILS_LISTTEXT.dwParam1.
				// let's try comparing the item names.
				for (dwi = 0; (signed)dwi < (signed)mitems; dwi++)
				{
					if (::lstrcmp(pmxcdSelectText[dwi].szName,
								  _T("Microphone")) == 0)
					{
						// found, dwi is the index.
						log.WriteString("\n Got the Source");
						mindex = dwi;
						micname = pmxcdSelectText[dwi].szName;
						break;
					}
				}
			}
		}

		delete []pmxcdSelectText;
	}
	

	return((unsigned) mindex <(unsigned) mitems);


}

BOOL MicPhone::selectMic(int val)
{
	if (m_mixer == NULL ||
		mitems == 0 ||
		mindex >= mitems)
	{
		return FALSE;
	}

	BOOL bRetVal = FALSE;

  log.WriteString("\n Setting value");
	// get all the values first
	MIXERCONTROLDETAILS_BOOLEAN *pmxcdSelectValue =
		new MIXERCONTROLDETAILS_BOOLEAN[mitems];

	if (pmxcdSelectValue != NULL)
	{
		MIXERCONTROLDETAILS mxcd;
		mxcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
		mxcd.dwControlID = mcontrolid;
		mxcd.cChannels = 1;
		mxcd.cMultipleItems = mitems;
		mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
		mxcd.paDetails = pmxcdSelectValue;
		
		if (::mixerGetControlDetails(reinterpret_cast<HMIXEROBJ>(m_mixer),
									 &mxcd,
									 MIXER_OBJECTF_HMIXER |
									 MIXER_GETCONTROLDETAILSF_VALUE)
			== MMSYSERR_NOERROR)
		{
	
			// MUX restricts the line selection to one source line at a time.
			if (val != 0 && mcontype == MIXERCONTROL_CONTROLTYPE_MUX)
			{
				::ZeroMemory(pmxcdSelectValue,
							 mitems * sizeof(MIXERCONTROLDETAILS_BOOLEAN));
			}

			// set the Microphone value
			pmxcdSelectValue[mindex].fValue = val;

			mxcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
			mxcd.dwControlID = mcontrolid;
			mxcd.cChannels = 1;
			mxcd.cMultipleItems = mitems;
			mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
			mxcd.paDetails = pmxcdSelectValue;
			if (::mixerSetControlDetails(reinterpret_cast<HMIXEROBJ>(m_mixer),
										 &mxcd,
										 MIXER_OBJECTF_HMIXER |
										 MIXER_SETCONTROLDETAILSF_VALUE)
				== MMSYSERR_NOERROR)
			{
				bRetVal = TRUE;
			}
		}

		delete []pmxcdSelectValue;
	}

	return bRetVal;
}





