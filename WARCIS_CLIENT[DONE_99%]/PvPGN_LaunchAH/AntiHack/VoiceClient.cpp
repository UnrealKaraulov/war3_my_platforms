// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "Antihack.h"
#include "CustomFeatures.h"
#include "warcis_reconnector.h"
#include "WarcraftFrameHelper.h"

/** @file paex_record.c
@ingroup examples_src
@brief Record input into an array; Save array to a file; Playback recorded data.
@author Phil Burk  http://www.softsynth.com
*/
/*
* $Id$
*
* This program uses the PortAudio Portable Audio Library.
* For more information see: http://www.portaudio.com
* Copyright (c) 1999-2000 Ross Bencina and Phil Burk
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files
* (the "Software"), to deal in the Software without restriction,
* including without limitation the rights to use, copy, modify, merge,
* publish, distribute, sublicense, and/or sell copies of the Software,
* and to permit persons to whom the Software is furnished to do so,
* subject to the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
* ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
* CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
* WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/*
* The text above constitutes the entire PortAudio license; however,
* the PortAudio community also makes the following non-binding requests:
*
* Any person wishing to distribute modifications to the Software is
* requested to send the modifications to the original developer so that
* they can be incorporated into the canonical version. It is also
* requested that these non-binding requests be included along with the
* license above.
*/

#include <stdio.h>
#include <stdlib.h>
#include "Antihack.h"
#include "CustomFeatures.h"


#include <concurrent_vector.h>
#define safevector concurrency::concurrent_vector

struct paTestData
{
	unsigned int          frameIndex;  /* Index into sample array. */
	//unsigned int          recordedSamples.size( );
	safevector<SAMPLE>	  recordedSamples;
	std::string PlayerName;
	bool				  PlayStopped;
	bool				  FirstPlay;
	bool				  Muted;

	unsigned int		  LastPlayTime;
	paTestData( )
	{
		frameIndex = /*recordedSamples.size( ) = */0;
		PlayStopped = false;
		FirstPlay = false;
		LastPlayTime = 0;
		Muted = false;
	}
};


safevector< paTestData > paTestDataPlayList;
safevector< paTestData > paTestDataSendList;


/*


Поток воспроизведения звука:
{
	Создать поток с именем игрока
	Читать данные из paTestData до тех пор пока они не закончатся
	Установить PlayStopped в true
	Стереть recordedSamples
	Установить frameIndex/recordedSamples.size( ) в 0
}


Прием данных:
{
	Найти paTestData с нужным именем
	Добавить данные в recordedSamples и увеличить максимальное число кадров
	Если PlayStopped установлен в true, установить в false и создать новый поток воспроизведения
}

*/

safevector< HANDLE > paThreads;


/* #define SAMPLE_RATE  (17932) // Test failure to open with this value. */
#define SAMPLE_RATE  ( 8000.0 )
#define FRAMES_PER_BUFFER (512)
#define NUM_SECONDS     (0.2)
//#define DITHER_FLAG     (paDitherOff) 
/** Set to 1 if you want to capture the recording to a file. */
#define WRITE_TO_FILE   (1)

/* Select sample format. */


/* This routine will be called by the PortAudio engine when audio is needed.
** It may be called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/
int frameid = 0;
static int recordCallback( const void *inputBuffer, void *outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void *userData )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__);
#endif
	paTestData *data = ( paTestData* )userData;
	SAMPLE *rptr = ( SAMPLE* )inputBuffer;
	SAMPLE *wptr = &data->recordedSamples[ data->frameIndex ];
	long framesToCalc;
	long i;
	int finished;
	unsigned long framesLeft = data->recordedSamples.size( ) - data->frameIndex;

	( void )outputBuffer; /* Prevent unused variable warnings. */
	( void )timeInfo;
	( void )statusFlags;
	( void )userData;


	/*CONSOLE_Print( "[RECORD] Frames left: " + to_string( framesLeft ) +
		". FrameIndex:" + to_string( data->frameIndex ) +
		". recordedSamples.size( ):" + to_string( data->recordedSamples.size( ) ) );*/


	if ( framesLeft < framesPerBuffer )
	{
		framesToCalc = framesLeft;
		finished = paComplete;
	}
	else
	{
		framesToCalc = framesPerBuffer;
		finished = paContinue;
	}

	if ( IsKeyPressed( VK_OEM_3 ) )
	{
		data->recordedSamples.resize( data->recordedSamples.size( ) + framesPerBuffer );
	}

	//AddNewPaTestData( data->frameIndex, framesToCalc, rptr, "LocalPlayer" );


	if ( wc3classgproxy && ( current_menu == Wc3Menu::GAME_LOBBY || IsGame( ) ) )
	{
		CWC3 * gproxy_wc3 = ( CWC3* )wc3classgproxy;
		//for ( auto s : paTestDataSendList )
		//{
		gproxy_wc3->SendVoicePacket( data->frameIndex, framesToCalc, rptr );
		//}
	}
	else
		finished = paComplete;
	/*if ( inputBuffer == NULL )
	{
		for ( i = 0; i < framesToCalc; i++ )
		{
			*wptr++ = SAMPLE_SILENCE;
		}
	}
	else
	{
		for ( i = 0; i < framesToCalc; i++ )
		{
			*wptr++ = *rptr++;
		}
	}

	*/
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__);
#endif
	data->frameIndex += framesToCalc;
	return finished;
}

/* This routine will be called by the PortAudio engine when audio is needed.
** It may be called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/


static int playCallback( const void *inputBuffer, void *outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void *userData )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__);
#endif
	//CONSOLE_Print( "Play data ( " + to_string( GetTickCount( ) ) + " )" );

	int bufferid = ( int )userData;

	paTestData & data = paTestDataPlayList[ bufferid ];


	if ( data.Muted )
		return paComplete;

	SAMPLE *rptr = &data.recordedSamples[ data.frameIndex ];
	SAMPLE *wptr = ( SAMPLE* )outputBuffer;
	unsigned int i;
	int finished;
	unsigned int framesLeft = data.recordedSamples.size( ) - data.frameIndex;


	( void )inputBuffer; /* Prevent unused variable warnings. */
	( void )timeInfo;
	( void )statusFlags;
	( void )userData;

	/*CONSOLE_Print( "[PLAY] Frames left: " + to_string( framesLeft ) +
		". FrameIndex:" + to_string( data.frameIndex ) +
		". recordedSamples.size( ):" + to_string( data.recordedSamples.size( ) ) );
*/
	if ( framesLeft < framesPerBuffer )
	{
		//CONSOLE_Print( "playCallback 3" );

		/* final buffer... */
		for ( i = 0; i < framesLeft; i++ )
		{
			*wptr++ = *rptr++;  /* left */
		}
		for ( ; i < framesPerBuffer; i++ )
		{
			*wptr++ = 0;  /* left */
		}
		data.frameIndex += framesLeft;
		data.recordedSamples.clear( );/*
		data.recordedSamples.size( ) = 0;*/
		data.frameIndex = 0;
		data.PlayStopped = true;
		finished = paComplete;
		//CONSOLE_Print( "Buffer end!" );

	}
	else
	{
		//CONSOLE_Print( "playCallback 33" );

		for ( i = 0; i < framesPerBuffer; i++ )
		{
			*wptr++ = *rptr++;  /* left */
		}
		data.frameIndex += framesPerBuffer;
		finished = paContinue;
		//CONSOLE_Print( "playCallback 44" );

	}
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__);
#endif
	return finished;
}

/*******************************************************************/


time_t LastDeviceCheck_input = time( 0 );
PaDeviceIndex LastDevice_input = paNoDevice;

paTestData RecordMessage( void )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	PaStream*           recordstream = nullptr;

	//CONSOLE_Print( "Recording... 1 ( " + to_string( GetTickCount( ) ) + " )" );

	PaStreamParameters  inputParameters;

	PaError             err = paNoError;

	paTestData          data;
	int                 i;
	int                 totalFrames;
	int                 numSamples;
	int                 numBytes;
	data.PlayerName = "localplayer";
	/*data.recordedSamples.size( ) = */
	totalFrames = ( int )( NUM_SECONDS * SAMPLE_RATE ); /* Record for a few seconds. */
	data.frameIndex = 0;
	numSamples = totalFrames;
	numBytes = numSamples * sizeof( SAMPLE );
	data.recordedSamples = safevector<SAMPLE>( numSamples, 0 ); /* From now on, recordedSamples is initialised. */

	//if ( data.recordedSamples == NULL )
	//{
	//	return data;
	//}
	//for ( i = 0; i < numSamples; i++ ) data.recordedSamples[ i ] = 0;

	//if ( LastDevice_input == paNoDevice || time( 0 ) > LastDeviceCheck_input )
	//{
	inputParameters.device = Pa_GetDefaultInputDevice( ); /* default input device */
	if ( inputParameters.device == paNoDevice ) {
		CONSOLE_Print( "Error: No default input device.\n" );
		Sleep( 2000 );/*
		Pa_Terminate( );
		Pa_Initialize( );*/
		return paTestData( );
	}
	inputParameters.channelCount = 1;                    /* stereo input */
	inputParameters.sampleFormat = PA_SAMPLE_TYPE;
	inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultLowInputLatency;
	inputParameters.hostApiSpecificStreamInfo = NULL;
	LastDeviceCheck_input = time( 0 );
	//}

//	CONSOLE_Print( "Recording... 2 ( " + to_string( GetTickCount( ) ) + " )" );

	/* Record some audio. -------------------------------------------- */
	//if ( recordstream == nullptr )
	err = Pa_OpenStream(
		&recordstream,
		&inputParameters,
		NULL,                  /* &outputParameters, */
		SAMPLE_RATE,
		FRAMES_PER_BUFFER,
		paClipOff | paDitherOff,      /* we won't output out of range samples so don't bother clipping them */
		recordCallback,
		&data );


	//CONSOLE_Print( "Recording... 3 ( " + to_string( GetTickCount( ) ) + " )" );

	if ( err != paNoError || recordstream == nullptr )
	{
		return paTestData( );
	}

	//CONSOLE_Print( "Recording... 4 ( " + to_string( GetTickCount( ) ) + " )" );

	err = Pa_StartStream( recordstream );

	if ( err != paNoError )
	{
		recordstream = nullptr;
		return paTestData( );
	}
	//CONSOLE_Print( "Recording... 5 ( " + to_string( GetTickCount( ) ) + " )" );

	while ( Pa_IsStreamActive( recordstream ) && IsKeyPressed( VK_OEM_3 ) )
	{
		Pa_Sleep( 5 );
	}

	//CONSOLE_Print( "Recording... 6 ( " + to_string( GetTickCount( ) ) + " )" );

	if ( !IsKeyPressed( VK_OEM_3 ) )
	{
		data.recordedSamples.resize( data.frameIndex );
		Pa_AbortStream( recordstream );
	}
	else
		Pa_CloseStream( recordstream );


	return data;
}

time_t LastDeviceCheck = time( 0 );
PaDeviceIndex LastDevice = paNoDevice;

DWORD WINAPI PLAYDATAPROCESSTHREAD( LPVOID lpdata )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	//CONSOLE_Print( "PLAYDATAPROCESSTHREAD ( " + to_string( GetTickCount( ) ) + " )" );
	//SetThreadPriority( GetCurrentThread( ), THREAD_PRIORITY_LOWEST );

	int bufferid = ( int )lpdata;
	//CONSOLE_Print( "PLAYDATAPROCESSTHREAD 1 : " + to_string(bufferid) + " size : " + to_string( paTestDataPlayList.size( )));

	paTestData & data = paTestDataPlayList[ bufferid ];

	if ( data.FirstPlay )
	{
		data.frameIndex = 0;
		CONSOLE_Print( "FirstPlay( " + to_string( GetTickCount( ) ) + " )" );
		Sleep( 500 );
		data.FirstPlay = false;
	}

	//CONSOLE_Print( "PLAYDATAPROCESSTHREAD 2" );
	PaError             err = paNoError;
	PaStreamParameters  outputParameters;

	PaStream*           stream;
	/* Playback recorded data.  -------------------------------------------- */
	//data.frameIndex = 0;
	//if ( LastDevice == paNoDevice || time( 0 ) > LastDeviceCheck )
	//{
	LastDeviceCheck = time( 0 );
	outputParameters.device = Pa_GetDefaultOutputDevice( ); /* default output device */
	//CONSOLE_Print( "PLAYDATAPROCESSTHREAD 3" );
	outputParameters.channelCount = 1;                     /* stereo output */
	outputParameters.sampleFormat = PA_SAMPLE_TYPE;
	outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
	outputParameters.hostApiSpecificStreamInfo = NULL;
	//}

	if ( outputParameters.device == paNoDevice ) {
		CONSOLE_Print( "Error: No default output device.\n" );
		Sleep( 2000 );
		//Pa_Terminate( );
		//Pa_Initialize( );
		return 0;
	}
	//CONSOLE_Print( "PLAYDATAPROCESSTHREAD 5" );

	err = Pa_OpenStream(
		&stream,
		NULL, /* no input */
		&outputParameters,
		SAMPLE_RATE,
		FRAMES_PER_BUFFER,
		paClipOff | paDitherOff,      /* we won't output out of range samples so don't bother clipping them */
		playCallback,
		( LPVOID )bufferid );

	//CONSOLE_Print( "PLAYDATAPROCESSTHREAD Start play ok!.\n" );
	if ( err != paNoError )
	{
		CONSOLE_Print( "Error: Pa_OpenStream.\n" );
		return 0;
	}

	if ( stream )
	{
		err = Pa_StartStream( stream );
		if ( err != paNoError )
		{
			CONSOLE_Print( "Error: Pa_StartStream.\n" );
			return 0;
		}

		/*err = Pa_WriteStream( stream, data.recordedSamples, data.recordedSamples.size( ) );
		if ( err != paNoError )
		{
			if ( data.recordedSamples )
				free( data.recordedSamples );
			delete _data;
			CONSOLE_Print( "Error: Pa_WriteStream.\n" );
			return 0;
		}
		*/
		while ( ( err = Pa_IsStreamActive( stream ) ) == 1 )
			Pa_Sleep( 50 );
		if ( err < 0 )
		{
			CONSOLE_Print( "Error: Pa_IsStreamActive.\n" );
			return 0;
		}

		err = Pa_CloseStream( stream );
		if ( err != paNoError )
		{
			CONSOLE_Print( "Error: Pa_CloseStream.\n" );
			return 0;
		}

	}

	if ( err != paNoError )
	{
		CONSOLE_Print( "An error occured while using the portaudio stream\n" );
		err = 1;          /* Always return 0 or 1, but no other return codes. */
	}
	return 0;
}

// ^\w+[\s+\w+]+\(.*?\)[\n\r]*\{[\n\r]* 
// $1#ifndef  ANTIHACKNODEBUG\n AddLogFunctionCall( __FUNCSIGW__  );\n#endif\n

void AddNewPaTestData( unsigned int frameidx, unsigned int maxframeidx, SAMPLE * samples, const std::string & playername )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	//CONSOLE_Print( "AddNewPaTestData" );

	//std::transform( playername.begin( ), playername.end( ), playername.begin( ), ::tolower );
	//CONSOLE_Print( "AddNewPaTestData 2" );

	if ( current_menu == Wc3Menu::GAME_LOBBY )
	{
		if ( !IsPlayerSlotNotEnemy( playername ) )
		{
			//CONSOLE_Print( "Player " + playername + " is enemy?" );
			return;
		}
	}

	int playdataidx = 0;
	for ( auto & s : paTestDataPlayList )
	{
		bool playernamesame = playername == s.PlayerName;
		if ( playernamesame )
		{
			s.LastPlayTime = GetTickCount( );

			if ( s.Muted )
				return;

			if ( !s.PlayStopped && playernamesame )
			{
				//CONSOLE_Print( "Append data time(" + to_string( GetTickCount( ) ) + ")" );
				for ( unsigned int i = 0; i < maxframeidx; i++ )
				{
					s.recordedSamples.push_back( samples[ i ] );
				}
				//s.recordedSamples.insert( s.recordedSamples.end( ), &samples[0], &samples[ maxframeidx ] );
				//s.recordedSamples.size( ) += maxframeidx;
				return;
			}
			else
			{
				CONSOLE_Print( "Create data(" + to_string( GetTickCount( ) ) + ")" );
				s.FirstPlay = true;
				s.frameIndex = 0;
				//s.recordedSamples.size( ) = maxframeidx;
				s.recordedSamples.clear( );
				s.recordedSamples = safevector<SAMPLE>( maxframeidx );

				memcpy( &s.recordedSamples[ 0 ], samples, maxframeidx * sizeof( SAMPLE ) );
				//memcpy( &s.recordedSamples[0], samples, maxframeidx * sizeof( SAMPLE ) );
				s.PlayStopped = false;
				DWORD thread_id;
				paThreads.push_back( CreateThread( NULL, 8192, PLAYDATAPROCESSTHREAD, ( LPVOID )playdataidx,
					STACK_SIZE_PARAM_IS_A_RESERVATION, &thread_id ) );


				AddThreadInfoMy( thread_id, L" PLAYDATAPROCESSTHREAD #2 THREAD" );
				//	CONSOLE_Print( "AddNewPaTestData 44" );

				return;
			}
		}

		playdataidx++;
	}

	CONSOLE_Print( "Create new data \"" + playername + "\" (" + to_string( GetTickCount( ) ) + ")" );

	paTestData tmpNewData = paTestData( );
	tmpNewData.frameIndex = 0;
	//tmpNewData.recordedSamples.size( ) = maxframeidx;
	tmpNewData.recordedSamples = safevector<SAMPLE>( maxframeidx );
	memcpy( &tmpNewData.recordedSamples[ 0 ], samples, maxframeidx * sizeof( SAMPLE ) );

	tmpNewData.PlayerName = playername;
	tmpNewData.PlayStopped = false;
	tmpNewData.FirstPlay = true;
	paTestDataPlayList.push_back( tmpNewData );
	DWORD thread_id;
	paThreads.push_back( CreateThread( NULL, 8192, PLAYDATAPROCESSTHREAD, ( LPVOID )playdataidx,
		STACK_SIZE_PARAM_IS_A_RESERVATION, &thread_id ) );

	AddThreadInfoMy( thread_id, L" PLAYDATAPROCESSTHREAD #3 THREAD" );

	safevector<HANDLE> outpaThreads;

	for ( auto s : paThreads )
	{
		DWORD exitcode = 0;
		GetExitCodeThread( s, &exitcode );
		if ( exitcode != STILL_ACTIVE )
		{
			CloseHandle( s );
		}
		else
			outpaThreads.push_back( s );
	}

	paThreads.clear( );
	paThreads = outpaThreads;
	//paTestDataPlayList.push_back( tmppaTestData );
}


DWORD WINAPI VoiceClientRecordThread( LPVOID )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	//SetThreadPriority( GetCurrentThread( ), THREAD_PRIORITY_LOWEST );
	CONSOLE_Print( "VoiceClientRecordThread" );
	PaError             err = paNoError;
	err = Pa_Initialize( );
	//CONSOLE_Print( "VoiceClientRecordThread 2" );
	if ( err != paNoError ) {
		Pa_Terminate( );
		//	CONSOLE_Print( "VoiceClientRecordThread 3" );
		return 0;
	}
	//	CONSOLE_Print( "VoiceClientRecordThread 4" );
	while ( true )
	{
		Sleep( 100 );
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif

		while ( IsKeyPressed( VK_OEM_3 ) )
		{
			if ( wc3classgproxy && ( current_menu == Wc3Menu::GAME_LOBBY || IsGame( ) ) )
			{
				//CONSOLE_Print( "VoiceClientRecordThread 5" );
				//CONSOLE_Print( "Send data start recording: ( " + to_string( GetTickCount( ) ) + " )" );
				/*paTestData data =*/
				RecordMessage( );
				//	CONSOLE_Print( "VoiceClientRecordThread 6" );
				/*if ( data.recordedSamples.size( ) )
				{
					CONSOLE_Print( "Send data end recording: ( " + to_string( GetTickCount( ) ) + " )" );
					AddNewPaTestData( data.frameIndex, data.recordedSamples.size( ), &data.recordedSamples[ 0 ], "LocalPlayer" );
					Sleep( 1000 );
					AddNewPaTestData( data.frameIndex, data.recordedSamples.size( ), &data.recordedSamples[ 0 ], "LocalPlayer" );
					Sleep( 1000 );
					AddNewPaTestData( data.frameIndex, data.recordedSamples.size( ), &data.recordedSamples[ 0 ], "LocalPlayer" );
					Sleep( 1000 );
					AddNewPaTestData( data.frameIndex, data.recordedSamples.size( ), &data.recordedSamples[ 0 ], "LocalPlayer" );
					Sleep( 1000 );*/
					//if ( wc3classgproxy )
					//{
					//	CWC3 * gproxy_wc3 = ( CWC3* )wc3classgproxy;
					//	//for ( auto s : paTestDataSendList )
					//	//{
					//		gproxy_wc3->SendVoicePacket( data.frameIndex, data.recordedSamples.size( ), ( unsigned char* )data.recordedSamples, data.SampleLen );
					//	//}
					//}
					//paTestDataSendList.push_back( data );
					//CONSOLE_Print( "VoiceClientRecordThread 7" );
					//PlayData( data );
				/*}
				data.recordedSamples.clear( );
				data.FirstPlay = true;
				data.frameIndex = 0;*/
				//CONSOLE_Print( "VoiceClientRecordThread 8" );
			}
			else
				break;
		}
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif

		//recordstream = nullptr;
	}

	return 0;
}

DWORD WINAPI VoiceClientPlayThread( LPVOID )
{

#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	return 0;
}

void SendVoicePackets( CWC3 * gproxy_wc3 )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	if ( !gproxy_wc3 )
		return;

	for ( auto s : paTestDataSendList )
	{
		gproxy_wc3->SendVoicePacket( s.frameIndex, s.recordedSamples.size( ), &s.recordedSamples[ 0 ] );
	}

}

void InitVoiceClientThread( )
{

#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	DWORD thread_id;
	CreateThread( 0, 0, VoiceClientRecordThread, 0, 0, &thread_id );
	AddThreadInfoMy( thread_id, L" VOICE CLIENT THREAD " );

}

void UninitializeVoiceClient( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	Pa_Terminate( );
}

using namespace NWar3Frame;



std::vector<CWar3Frame * > VoicePlayerFrameList[ 2 ];


int VoicePlayerCallback( CWar3Frame*frame, int FrameAddr, uint32_t ClickType )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	CONSOLE_Print( "Press voice frame" );

	if ( !frame || frame->GetFrameCustomValue( 1 ) != 'MUTE' || frame->GetFrameCustomValue( 2 ) < 0 || frame->GetFrameCustomValue( 2 ) >= paTestDataPlayList.size( ) )
		return 0;



	if ( IsGame( ) )
		CWar3Frame::FocusFrame( 0 );

	paTestData & tmpPaTestData = paTestDataPlayList[ frame->GetFrameCustomValue( 2 ) ];

	tmpPaTestData.Muted = !tmpPaTestData.Muted;


	CWar3Frame backimg = CWar3Frame( );
	backimg.CWar3FrameFromAddress( frame->GetFrameBackdropAddress( CFrameBackdropType::ControlBackdrop ) );
	backimg.SetFrameType( CFrameType::FRAMETYPE_BACKDROP );
	backimg.SetTexture( tmpPaTestData.Muted ? "WarcisPictures\\UserCantSpeak.blp" : "WarcisPictures\\UserWantToSpeak.blp" );

	if ( tmpPaTestData.Muted )
		frame->SetText( ( "Unmute " + tmpPaTestData.PlayerName ).c_str( ) );
	else
		frame->SetText( ( "Mute " + tmpPaTestData.PlayerName ).c_str( ) );


	CONSOLE_Print( "Player:" + tmpPaTestData.PlayerName + ( tmpPaTestData.Muted ? " muted." : " can speak" ) );

	return 0;
}

void Build10FramesIfNeed( )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__ );
#endif
	CWar3Frame CheckIsWarcisSpeakItemExists = CWar3Frame( );
	if ( !CheckIsWarcisSpeakItemExists.Load( "WarcisSpeakItem" ) || VoicePlayerFrameList[ 0 ].size( ) == 0 )
	{
		for ( auto  s : VoicePlayerFrameList[ 0 ] )
		{
			s->DestroyThisFrame( );
			delete s;
		}
		for ( auto  s : VoicePlayerFrameList[ 1 ] )
		{
			s->DestroyThisFrame( );
			delete s;
		}
		VoicePlayerFrameList[ 0 ].clear( );
		VoicePlayerFrameList[ 1 ].clear( );
		float StartVoiceShowOffset = 0.55;

		for ( int i = 0; i < 12; i++ )
		{
			CWar3Frame * CurVoiceFrame = new CWar3Frame( "WarcisSpeakItem", i, true);
			CurVoiceFrame->SetFrameType( CFrameType::FRAMETYPE_FRAME );
			CurVoiceFrame->SetFrameAbsolutePosition( CFramePosition::BOTTOM_LEFT, 0.035f, StartVoiceShowOffset );

			CWar3Frame * CurVoiceFrameButton = new CWar3Frame( );
			VoicePlayerFrameList[ 0 ].push_back( CurVoiceFrame );
			VoicePlayerFrameList[ 1 ].push_back( CurVoiceFrameButton );
		
			StartVoiceShowOffset -= 0.034;
		}

		CONSOLE_Print( __FUNCSIGW__ + to_wstring( __LINE__ ) );
	}
	
}

BOOL VoiceChangeMenuCallbackInitialized = FALSE;

void VoiceChangeMenuCallback( )
{
	//CONSOLE_Print( __FUNCTION__ + to_string( __LINE__ ) );

	//CONSOLE_Print( "Need delete!" );
	for ( auto s : VoicePlayerFrameList[ 0 ] )
	{
		s->DestroyThisFrame( );
		//s->~CWar3Frame( );
		delete s;
	}
	for ( auto s : VoicePlayerFrameList[ 1 ] )
	{
		s->DestroyThisFrame( );
		delete s;
	}

	VoicePlayerFrameList[ 0 ].clear( );
	VoicePlayerFrameList[ 1 ].clear( );

	//CONSOLE_Print( "Delete ok!" );
}


int FirstJoinLobby = 10;


void UpdateVoicePlayerList( )
{

	if ( !VoiceChangeMenuCallbackInitialized )
	{
		CWar3Frame::SetChangeMenuEventCallback( VoiceChangeMenuCallback );
		VoiceChangeMenuCallbackInitialized = TRUE;
	}
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
//	CONSOLE_Print( __FUNCTION__ + to_string(__LINE__) );
	unsigned int CurTicks = GetTickCount( );

	if ( FirstJoinLobby )
	{
		FirstJoinLobby--;
		VoiceChangeMenuCallback( );
		return;
	}

	//CONSOLE_Print( __FUNCTION__ + to_string( __LINE__ ) );

	if ( current_menu != Wc3Menu::GAME_LOBBY && !IsGame( ) )
	{
	//	CONSOLE_Print( __FUNCTION__ + to_string( __LINE__ ) );

#ifndef  ANTIHACKNODEBUG
		AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
		
		for ( auto & s : paTestDataPlayList )
		{
			if ( s.LastPlayTime + 4000 > CurTicks )
			{
				return;
			}
		}



		for ( auto s : VoicePlayerFrameList[ 0 ] )
		{
			s->DestroyThisFrame( );
			delete s;
		}
		for ( auto s : VoicePlayerFrameList[ 1 ] )
		{
			s->DestroyThisFrame( );
			delete s;
		}

		VoicePlayerFrameList->clear( );
		//paTestDataPlayList.clear( );
		//CONSOLE_Print( "Delete ok!" );
#ifndef  ANTIHACKNODEBUG
		AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
		FirstJoinLobby = 10;
		return;
	}


//	CONSOLE_Print( __FUNCTION__ + to_string( __LINE__ ) );

#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif

	Build10FramesIfNeed( );

	//for ( auto & s : VoicePlayerFrameList[ 0 ] )
	//{
	//	s->DestroyThisFrame( );
	//	s->~CWar3Frame( );
	//	delete s;
	//}
	//for ( auto & s : VoicePlayerFrameList[ 1 ] )
	//{
	//	s->DestroyThisFrame( );
	//	s->~CWar3Frame( );
	//	delete s;
	//}
	//CONSOLE_Print( __FUNCTION__ + to_string( __LINE__ ) );

#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	int CurrentFrameId = 0;


	for ( auto & s : paTestDataPlayList )
	{
		if ( CurrentFrameId < VoicePlayerFrameList[ 0 ].size( ) )
			if ( s.LastPlayTime + 1000 > CurTicks )
			{
			//	CONSOLE_Print( __FUNCTION__ + to_string( __LINE__ ) );

				CWar3Frame CheckIsWarcisSpeakItemExists = CWar3Frame( );
				if ( CheckIsWarcisSpeakItemExists.Load( "WarcisSpeakItem", CurrentFrameId ) )
				{
				//	CONSOLE_Print( __FUNCTION__ + to_string( __LINE__ ) );

					CWar3Frame * CurVoiceFrame = VoicePlayerFrameList[ 0 ][ CurrentFrameId ];
					CWar3Frame * CurVoiceFrameButton = VoicePlayerFrameList[ 1 ][ CurrentFrameId ];
					//
#ifndef  ANTIHACKNODEBUG
					AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
					CurVoiceFrame->Show( true );

		
					CWar3Frame CurVoicePlayerName = CWar3Frame( );
					CurVoicePlayerName.Load( "WarcisSpeakLoginText", CurrentFrameId );
					CurVoicePlayerName.SetFrameType( CFrameType::FRAMETYPE_ITEM );
					CurVoicePlayerName.SetText( s.PlayerName.c_str( ) );
					CurVoicePlayerName.~CWar3Frame( );


				//	CONSOLE_Print( __FUNCTION__ + to_string( __LINE__ ) );

					CurVoiceFrameButton->Load( "WarcisSpeakFrameImg", CurrentFrameId );
					CurVoiceFrameButton->SetFrameType( CFrameType::FRAMETYPE_BUTTON );

					if ( IsGame( ) )
					{
						if ( s.Muted )
							CurVoiceFrameButton->SetText( ( "Unmute " + s.PlayerName ).c_str( ) );
						else
							CurVoiceFrameButton->SetText( ( "Mute " + s.PlayerName ).c_str( ) );
					}


					CurVoiceFrameButton->SetCallbackFunc( VoicePlayerCallback );
					CurVoiceFrameButton->RegisterEventCallback( CFrameEvents::FRAME_EVENT_PRESSED );
					CurVoiceFrameButton->SetFrameCustomValue( 'MUTE', 1 );
					CurVoiceFrameButton->SetFrameCustomValue( CurrentFrameId, 2 );


					CWar3Frame backimg = CWar3Frame( );
					backimg.CWar3FrameFromAddress( CurVoiceFrameButton->GetFrameBackdropAddress( CFrameBackdropType::ControlBackdrop ) );
					backimg.SetFrameType( CFrameType::FRAMETYPE_BACKDROP );
					backimg.SetTexture( s.Muted ? "WarcisPictures\\UserCantSpeak.blp" : "WarcisPictures\\UserWantToSpeak.blp" );
#ifndef  ANTIHACKNODEBUG
					AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
					//CONSOLE_Print( __FUNCTION__ + to_string( __LINE__ ) );

					CurrentFrameId++;

				}
				else break;
			}

	}



#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
//	CONSOLE_Print( __FUNCTION__ + to_string( __LINE__ ) );

	for ( auto & s : paTestDataPlayList )
	{
		if ( CurrentFrameId < VoicePlayerFrameList[ 0 ].size( ) )
			if ( !( s.LastPlayTime + 1000 > CurTicks ) && IsKeyPressed( VK_LMENU ) )
			{
				CWar3Frame CheckIsWarcisSpeakItemExists = CWar3Frame( );
				if ( CheckIsWarcisSpeakItemExists.Load( "WarcisSpeakItem", CurrentFrameId ) )
				{
			//		CONSOLE_Print( __FUNCTION__ + to_string( __LINE__ ) );

					CWar3Frame * CurVoiceFrame = VoicePlayerFrameList[ 0 ][ CurrentFrameId ];
					CWar3Frame * CurVoiceFrameButton = VoicePlayerFrameList[ 1 ][ CurrentFrameId ];
					//
#ifndef  ANTIHACKNODEBUG
					AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
					CurVoiceFrame->Show( true );

					CWar3Frame CurVoicePlayerName = CWar3Frame( );
					CurVoicePlayerName.Load( "WarcisSpeakLoginText", CurrentFrameId );
					CurVoicePlayerName.SetFrameType( CFrameType::FRAMETYPE_ITEM );
					CurVoicePlayerName.SetText( s.PlayerName.c_str( ) );
					CurVoicePlayerName.~CWar3Frame( );

					CurVoiceFrameButton->Load( "WarcisSpeakFrameImg", CurrentFrameId );
					CurVoiceFrameButton->SetFrameType( CFrameType::FRAMETYPE_BUTTON );
			//		CONSOLE_Print( __FUNCTION__ + to_string( __LINE__ ) );
					if ( IsGame( ) )
					{
						if ( s.Muted )
							CurVoiceFrameButton->SetText( ( "Unmute " + s.PlayerName ).c_str( ) );
						else
							CurVoiceFrameButton->SetText( ( "Mute " + s.PlayerName ).c_str( ) );
					}
					CurVoiceFrameButton->SetCallbackFunc( VoicePlayerCallback );
					CurVoiceFrameButton->RegisterEventCallback( CFrameEvents::FRAME_EVENT_PRESSED );
					CurVoiceFrameButton->SetFrameCustomValue( 'MUTE', 1 );
					CurVoiceFrameButton->SetFrameCustomValue( CurrentFrameId, 2 );

					CWar3Frame backimg = CWar3Frame( );
					backimg.CWar3FrameFromAddress( CurVoiceFrameButton->GetFrameBackdropAddress( CFrameBackdropType::ControlBackdrop ) );
					backimg.SetFrameType( CFrameType::FRAMETYPE_BACKDROP );
					backimg.SetTexture( s.Muted ? "WarcisPictures\\UserCantSpeak.blp" : "WarcisPictures\\UserWantToSpeak.blp" );
#ifndef  ANTIHACKNODEBUG
					AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
					CurrentFrameId++;
				//	CONSOLE_Print( __FUNCTION__ + to_string( __LINE__ ) );

				}
				else break;
			}

	}
//	CONSOLE_Print( __FUNCTION__ + to_string( __LINE__ ) );

#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif

	for ( int i = CurrentFrameId; i < 12; i++ )
	{
		CWar3Frame CheckIsWarcisSpeakItemExists = CWar3Frame( );
		if ( CheckIsWarcisSpeakItemExists.Load( "WarcisSpeakItem", i ) )
		{
#ifndef  ANTIHACKNODEBUG
			AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
			CheckIsWarcisSpeakItemExists.SetFrameType( CFrameType::FRAMETYPE_FRAME );
			CheckIsWarcisSpeakItemExists.Show( false );
		}
		else break;
	}
//	CONSOLE_Print( __FUNCTION__ + to_string( __LINE__ ) );

#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
}