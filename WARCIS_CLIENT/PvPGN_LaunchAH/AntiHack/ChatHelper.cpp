#include "Antihack.h"
#include "CustomFeatures.h"

#define MAX_CHAT_MSG_LEN 128

int GetChatOffset( )
{
	int pclass = *( int* )pW3XGlobalClass;
	if ( pclass > 0 )
	{
		return *( int* )( pclass + 0x3FC );
	}

	return 0;
}




char * GetChatString( )
{
	int pChatOffset = GetChatOffset( );
	if ( pChatOffset > 0 )
	{
		pChatOffset = *( int* )( pChatOffset + 0x1E0 );
		if ( pChatOffset > 0 )
		{
			pChatOffset = *( int* )( pChatOffset + 0x1E4 );
			return ( char * )pChatOffset;
		}
	}
	return 0;
}

LPARAM lpReturnScanKeyUP = ( LPARAM )( 0xC0000001 | ( LPARAM )( MapVirtualKey( VK_RETURN, 0 ) << 16 ) );
LPARAM lpReturnScanKeyDOWN = ( LPARAM )( 0x00000001 | ( LPARAM )( MapVirtualKey( VK_RETURN, 0 ) << 16 ) );



LPARAM lpRShiftScanKeyUP = ( LPARAM )( 0xC0000001 | ( LPARAM )( MapVirtualKey( VK_RSHIFT, 0 ) << 16 ) );
LPARAM lpRShiftScanKeyDOWN = ( LPARAM )( 0x00000001 | ( LPARAM )( MapVirtualKey( VK_RSHIFT, 0 ) << 16 ) );

LPARAM lpShiftScanKeyUP = ( LPARAM )( 0xC0000001 | ( LPARAM )( MapVirtualKey( VK_SHIFT, 0 ) << 16 ) );
LPARAM lpShiftScanKeyDOWN = ( LPARAM )( 0x00000001 | ( LPARAM )( MapVirtualKey( VK_SHIFT, 0 ) << 16 ) );

LPARAM lpLShiftScanKeyUP = ( LPARAM )( 0xC0000001 | ( LPARAM )( MapVirtualKey( VK_LSHIFT, 0 ) << 16 ) );
LPARAM lpLShiftScanKeyDOWN = ( LPARAM )( 0x00000001 | ( LPARAM )( MapVirtualKey( VK_LSHIFT, 0 ) << 16 ) );






pGameChatSetState GameChatSetState;


pSetChatTargetUsers pSetChatTargetUsers_org;
pSetChatTargetUsers pSetChatTargetUsers_ptr;

BOOL UsingCustomChatTarget = FALSE;

int CustomChatTarget = 0;

int __fastcall SetChatTargetUsers_my( int chataddr, int ecx, int valtype )
{
	if ( !UsingCustomChatTarget )
		return pSetChatTargetUsers_ptr( chataddr, ecx, valtype );
	else
		return pSetChatTargetUsers_ptr( chataddr, ecx, CustomChatTarget );
}


time_t AntiSpam_LastTime = std::time( 0 );
unsigned int AntiSpam_Seconds = 4;
unsigned int AntiSpam_CurMsgCount = 0;
unsigned int AntiSpam_MsgLimit = 10;


void __stdcall SetAntiSpamLimits( unsigned int Messages, unsigned int Seconds )
{
	AntiSpam_Seconds = Seconds;
	AntiSpam_MsgLimit = Messages;
}



int __stdcall SendMessageToChat( const char * msg, BOOL toAll )
{
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif

	if ( AntiSpam_MsgLimit && AntiSpam_Seconds )
	{
		time_t AntiSpam_CurTime = std::time( 0 );
		if ( AntiSpam_CurTime - AntiSpam_LastTime > AntiSpam_Seconds )
		{
			AntiSpam_LastTime = AntiSpam_CurTime;
			AntiSpam_CurMsgCount = 0;
		}
		else
		{
			AntiSpam_CurMsgCount++;

			if ( AntiSpam_CurMsgCount > AntiSpam_MsgLimit )
			{
				return 0;
			}
		}
	}


	if ( !GetChatOffset( ) )
	{
		return FALSE;
	}

	if ( !GetChatString( ) )
	{
		return FALSE;
	}
#ifndef  ANTIHACKNODEBUG
	AddLogFunctionCall( __FUNCSIGW__, __LINE__ );
#endif
	BYTE tmpbuf[ 256 ];
	BYTE tmpbuf2[ 256 ];
	memset( tmpbuf2, 0, 256 );
	GetKeyboardState( tmpbuf );
	SetKeyboardState( tmpbuf2 );

	char * pChatString = GetChatString( );

	//char tmpdeb[ 512 ];
	//sprintf_s( tmpdeb, "%X->%s", pChatString, pChatString );
	//MessageBox( 0, tmpdeb, tmpdeb, 0 );

	if ( msg > 0 && pChatString > 0 && Warcraft3Window > 0 )
	{
		if ( IsChat( ) )
		{
			/*		BOOL NeedResetSHIFT = FALSE;
					BOOL NeedResetRSHIFT = FALSE;
					BOOL NeedResetLSHIFT = FALSE;


					if ( IsKeyPressed( VK_SHIFT ) )
					{
						NeedResetSHIFT = TRUE;
						WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYUP, VK_SHIFT, lpRShiftScanKeyUP );
					}
					if ( IsKeyPressed( VK_RSHIFT ) )
					{
						NeedResetRSHIFT = TRUE;
						WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYUP, VK_RSHIFT, lpShiftScanKeyUP );
					}
					if ( IsKeyPressed( VK_LSHIFT ) )
					{
						NeedResetLSHIFT = TRUE;
						WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYUP, VK_LSHIFT, lpLShiftScanKeyUP );
					}
		*/

			UsingCustomChatTarget = TRUE;
			if ( toAll )
			{
				//WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYDOWN, VK_RSHIFT, lpRShiftScanKeyDOWN );
				CustomChatTarget = 0;
			}
			else
			{
				CustomChatTarget = 1;
			}

			pChatString[ 0 ] = '\0';
			GameChatSetState( GetChatOffset( ), 0, 0 );
			GameChatSetState( GetChatOffset( ), 0, 1 );


			sprintf_s( pChatString, MAX_CHAT_MSG_LEN, "%.128s", msg );

			WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYDOWN, VK_RETURN, lpReturnScanKeyDOWN );
			WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYUP, VK_RETURN, lpReturnScanKeyUP );


			if ( toAll )
			{
				//WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYUP, VK_RSHIFT, lpRShiftScanKeyUP );
			}
			UsingCustomChatTarget = FALSE;


			//if ( NeedResetSHIFT )
			//{
			//	WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYDOWN, VK_SHIFT, lpRShiftScanKeyDOWN );
			//}
			//if ( NeedResetRSHIFT )
			//{
			//	WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYDOWN, VK_SHIFT, lpRShiftScanKeyDOWN );
			//}
			//if ( NeedResetLSHIFT )
			//{
			//	WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYDOWN, VK_SHIFT, lpRShiftScanKeyDOWN );
			//}


		}
		else
		{
			/*	BOOL NeedResetSHIFT = FALSE;
				BOOL NeedResetRSHIFT = FALSE;
				BOOL NeedResetLSHIFT = FALSE;


				if ( IsKeyPressed( VK_SHIFT ) )
				{
					NeedResetSHIFT = TRUE;
					WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYUP, VK_SHIFT, lpRShiftScanKeyUP );
				}
				if ( IsKeyPressed( VK_RSHIFT ) )
				{
					NeedResetRSHIFT = TRUE;
					WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYUP, VK_RSHIFT, lpShiftScanKeyUP );
				}
				if ( IsKeyPressed( VK_LSHIFT ) )
				{
					NeedResetLSHIFT = TRUE;
					WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYUP, VK_LSHIFT, lpLShiftScanKeyUP );
				}
	*/

			UsingCustomChatTarget = TRUE;

			if ( toAll )
			{
				//WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYDOWN, VK_RSHIFT, lpRShiftScanKeyDOWN );
				CustomChatTarget = 0;
			}
			else
			{
				CustomChatTarget = 1;
			}


			GameChatSetState( GetChatOffset( ), 0, 1 );

			sprintf_s( pChatString, MAX_CHAT_MSG_LEN, "%.128s", msg );

			WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYDOWN, VK_RETURN, lpReturnScanKeyDOWN );
			WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYUP, VK_RETURN, lpReturnScanKeyUP );

			if ( toAll )
			{
				//WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYUP, VK_RSHIFT, lpRShiftScanKeyUP );
			}

			UsingCustomChatTarget = FALSE;


			//if ( NeedResetSHIFT )
			//{
			//	WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYDOWN, VK_SHIFT, lpRShiftScanKeyDOWN );
			//}
			//if ( NeedResetRSHIFT )
			//{
			//	WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYDOWN, VK_SHIFT, lpRShiftScanKeyDOWN );
			//}
			//if ( NeedResetLSHIFT )
			//{
			//	WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYDOWN, VK_SHIFT, lpRShiftScanKeyDOWN );
			//}


		}

	}

	SetKeyboardState( tmpbuf );

	return 0;
}