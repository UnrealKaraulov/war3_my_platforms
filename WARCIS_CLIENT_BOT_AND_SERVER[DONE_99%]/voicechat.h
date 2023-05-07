//录音端server_cb_rt3.c，双向实时传输，使用回调
#include <stdio.h>
#include <stdlib.h>
#include "portaudio.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>

/* #define SAMPLE_RATE  (17932) // Test failure to open with this value. */
#define SAMPLE_RATE  (44100)
#define FRAMES_PER_BUFFER (512)
#define NUM_SECONDS     (0.5)
#define NUM_CHANNELS    (2)
/* #define DITHER_FLAG     (paDitherOff) */
#define DITHER_FLAG     (0) /**/
/** Set to 1 if you want to capture the recording to a file. */
#define WRITE_TO_FILE   (1)

/* Select sample format. */
/*
#define PA_SAMPLE_TYPE  paInt16
typedef short SAMPLE;
#define SAMPLE_SILENCE  (0)
#define PRINTF_S_FORMAT "%d"
*/

#define PA_SAMPLE_TYPE  paFloat32
typedef float SAMPLE;
#define SAMPLE_SILENCE  (0.0f)
#define PRINTF_S_FORMAT "%.8f"

#define PORT 6666
#define TCP_PORT 7777

char CLIENT_IP[ 20 ];
char SERVER_IP[ 20 ];
char buffer[ 15 ];
int bytes;
typedef struct
{
	int          frameIndex;  /* Index into sample array. */
	int          maxFrameIndex;
	SAMPLE      *recordedSamples;
}
paTestData;

int sockfd_r;        //用于接收音频数据的套接字
int sockfd_s;        //用于发送音频数据的套接字
int rflag, sflag;        //接收、发送标志位
int flag_call, flag_becall;
struct sockaddr_in server_addr;        //服务器地址结构，指本机
struct sockaddr_in client_addr;        //客户端地址结构，指对端

int server_sockfd, client_sockfd, client_sockfd_c;
struct sockaddr_in Tserver_addr, Tclient_addr, TZ_addr;

pthread_t thread_rp, thread_rs, thread_beijiao, thread_zhujiao;
void *value;
static int recordCallback( const void *inputBuffer, void *outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void *userData )
{
	paTestData *data = ( paTestData* )userData;
	const SAMPLE *rptr = ( const SAMPLE* )inputBuffer;
	SAMPLE *wptr = &data->recordedSamples[ data->frameIndex * NUM_CHANNELS ];
	long framesToCalc;
	long i;
	int finished;
	unsigned long framesLeft = data->maxFrameIndex - data->frameIndex;

	( void )outputBuffer; /* Prevent unused variable warnings. */
	( void )timeInfo;
	( void )statusFlags;
	( void )userData;

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

	if ( inputBuffer == NULL )
	{
		for ( i = 0; i < framesToCalc; i++ )
		{
			*wptr++ = SAMPLE_SILENCE;  /* left */
			if ( NUM_CHANNELS == 2 ) *wptr++ = SAMPLE_SILENCE;  /* right */
		}
	}
	else
	{
		for ( i = 0; i < framesToCalc; i++ )
		{
			*wptr++ = *rptr++;  /* left */
			if ( NUM_CHANNELS == 2 ) *wptr++ = *rptr++;  /* right */
		}
	}
	data->frameIndex += framesToCalc;
	return finished;
}


static int playCallback( const void *inputBuffer, void *outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void *userData )
{
	paTestData *data = ( paTestData* )userData;
	SAMPLE *rptr = &data->recordedSamples[ data->frameIndex * NUM_CHANNELS ];
	SAMPLE *wptr = ( SAMPLE* )outputBuffer;
	unsigned int i;
	int finished;
	unsigned int framesLeft = data->maxFrameIndex - data->frameIndex;

	( void )inputBuffer; /* Prevent unused variable warnings. */
	( void )timeInfo;
	( void )statusFlags;
	( void )userData;

	if ( framesLeft < framesPerBuffer )
	{
		/* final buffer... */
		for ( i = 0; i < framesLeft; i++ )
		{
			*wptr++ = *rptr++;  /* left */
			if ( NUM_CHANNELS == 2 ) *wptr++ = *rptr++;  /* right */
		}
		for ( ; i < framesPerBuffer; i++ )
		{
			*wptr++ = 0;  /* left */
			if ( NUM_CHANNELS == 2 ) *wptr++ = 0;  /* right */
		}
		data->frameIndex += framesLeft;
		finished = paComplete;
	}
	else
	{
		for ( i = 0; i < framesPerBuffer; i++ )
		{
			*wptr++ = *rptr++;  /* left */
			if ( NUM_CHANNELS == 2 ) *wptr++ = *rptr++;  /* right */
		}
		data->frameIndex += framesPerBuffer;
		finished = paContinue;
	}
	return finished;
}
//获取设备
int get_device( int device )
{
	if ( device < 0 ) {
		return 0;
	}
	else {
		return device;
	}
}

/*******************声明函数********************/
void Create_Server_To_Client_Socket( void );//监听
void Print_cmd( void );//显示界面
void Get_Clientip_From_Server( void );//call之后输入对方的IP进行链接
void Create_Client_To_Server_Socket( void );
/*******************函数定义********************/
void Get_Clientip_From_Server( void )//输入对方的IP地址
{
	printf( "主叫\n" );
	int bytes;

	printf( "请输入对方的IP\n" );
	scanf( "%s", SERVER_IP );

	Create_Client_To_Server_Socket( );//与对方建立连接

}

void Create_Client_To_Server_Socket( void )//与对方建立链接
{

	client_sockfd_c = socket( AF_INET, SOCK_STREAM, 0 );
	if ( client_sockfd_c < 0 )
	{
		perror( "client_socket\n" );
		exit( 1 );
	}
	//printf("client_sockfd %d\n",client_sockfd_c);//
	TZ_addr.sin_family = AF_INET;
	TZ_addr.sin_port = htons( TCP_PORT );
	TZ_addr.sin_addr.s_addr = inet_addr( SERVER_IP );

	if ( connect( client_sockfd_c, ( struct sockaddr * )&TZ_addr, sizeof( struct sockaddr ) ) < 0 )
	{
		perror( "connect\n" );
		exit( 1 );
	}
	else
	{
		printf( "connected!\n" );
	}
}

void Print_cmd( void )
{

	printf( "\n请选择\n" );
	printf( "******************************************\n" );
	printf( "(1)call            (2)hangup\n" );
	printf( "(3)break\n" );
	printf( "******************************************\n" );

}

void Create_Server_To_Client_Socket( void )//监听
{
	if ( ( server_sockfd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
	{
		perror( "socket\n" );
		exit( 1 );
	}

	Tserver_addr.sin_family = AF_INET;
	Tserver_addr.sin_port = htons( TCP_PORT );
	Tserver_addr.sin_addr.s_addr = INADDR_ANY;

	const char *optval = "qqqqq";
	setsockopt( server_sockfd, SOL_SOCKET, SO_REUSEADDR, optval, sizeof( optval ) );//端口重利用
	if ( bind( server_sockfd, ( struct sockaddr * )&Tserver_addr, sizeof( struct sockaddr ) ) < 0 )
	{
		perror( "bind\n" );
		exit( 1 );
	}
	if ( listen( server_sockfd, 10 ) == -1 )
	{
		perror( "listen\n" );
		exit( 1 );
	}

}
//录音发送线程
void *thread_record_send( void *remote )
{
	printf( "已创建发送线程\n" );
	PaStreamParameters  inputParameters,
		outputParameters;
	PaStream*           stream;
	PaError             err = paNoError;
	paTestData          data;
	int                 i;
	int                 totalFrames;
	int                 numSamples;
	int                 numBytes;
	int bytes;
	struct sockaddr_in server_addr1;

	// printf("record_send thread!\n");//可打印

	/*************************************对端的IP 端口号赋值***********/
	int rel, optval = 0;
	int optlen = sizeof( int );
	//创建udp套接字
	if ( ( sockfd_s = socket( AF_INET, SOCK_DGRAM, 0 ) ) == -1 )
	{
		perror( "socket" );
		//                return 1;
	}

	server_addr1.sin_family = AF_INET;
	server_addr1.sin_port = htons( PORT );
	server_addr1.sin_addr.s_addr = inet_addr( ( char * )remote );
	optval = 131071;
	rel = setsockopt( sockfd_s, SOL_SOCKET, SO_SNDBUF, ( const void* )&optval, optlen );
	if ( rel < 0 )
		perror( "setsockopt" );

	rel = getsockopt( sockfd_s, SOL_SOCKET, SO_SNDBUF, ( void* )&optval, &optlen );
	if ( rel < 0 )
		perror( "getsockopt" );
	printf( "SO_SNDBUF = %d\n", optval );
	data.maxFrameIndex = totalFrames = NUM_SECONDS * SAMPLE_RATE; /* Record for a few seconds. */
	numSamples = totalFrames * NUM_CHANNELS;
	//printf("numSamples = %d\n", numSamples);//
	numBytes = numSamples * sizeof( SAMPLE );
	//printf("numBytes = %d\n", numBytes);//
	data.recordedSamples = ( SAMPLE * )malloc( numBytes ); /* From now on, recordedSamples is initialised. */
														   //    printf("data.recordedSamples = %d\n", sizeof(SAMPLE)*numBytes);
	if ( data.recordedSamples == NULL )
	{
		printf( "Could not allocate record array.\n" );
		goto done;
	}
	for ( i = 0; i < numSamples; i++ ) data.recordedSamples[ i ] = 0;

	//    err = Pa_Initialize();
	//    if( err != paNoError ) goto done;

	inputParameters.device = get_device( Pa_GetDefaultInputDevice( ) ); /* default input device */
	if ( inputParameters.device == paNoDevice ) {
		fprintf( stderr, "RecordSend:Error: No default input device.\n" );
		goto done;
	}
	inputParameters.channelCount = 2;                    /* stereo input */
	inputParameters.sampleFormat = PA_SAMPLE_TYPE;
	inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultLowInputLatency;
	inputParameters.hostApiSpecificStreamInfo = NULL;

	/* Record some audio. -------------------------------------------- */
	err = Pa_OpenStream(
		&stream,
		&inputParameters,
		NULL,                  /* &outputParameters, */
		SAMPLE_RATE,
		FRAMES_PER_BUFFER,
		paClipOff,      /* we won't output out of range samples so don't bother clipping them */
		recordCallback,
		&data );
	if ( err != paNoError ) goto done;

	while ( !sflag );        //若sflag标志位为0，不发送数据
	while ( 1 )        //循环录音发送
	{
		rflag = 1;
		//printf("start...录音\n");//可打印
		data.frameIndex = 0;
		err = Pa_StartStream( stream );
		if ( err != paNoError ) goto done;
		//                printf("\n=== Now recording!! Please speak into the microphone. ===\n"); fflush(stdout);

		while ( ( err = Pa_IsStreamActive( stream ) ) == 1 )
		{
			Pa_Sleep( 100 );
			//printf("sndindex = %d\n", data.frameIndex ); fflush(stdout);//可打印
		}
		if ( err < 0 ) goto done;

		int n = 0, overframe = 0;
		i = 0;
		while ( n < numSamples / 256 )
		{
			//printf("准备发送\n");//可打印
			bytes = sendto( sockfd_s, &data.recordedSamples[ i ], sizeof( SAMPLE ) * 256, 0, ( struct sockaddr * )&server_addr1, sizeof( struct sockaddr ) );
			//printf("bytes =%d\n",bytes);//可打印
			if ( bytes < 0 )
			{
				perror( "sendto" );
				return 0;
			}
			n++;
			i += 256;
			//rflag = 1;
		}
		/*发送剩余的帪数据*/
		overframe = numSamples - ( numSamples / 256 ) * 256;
		bytes = sendto( sockfd_s, &data.recordedSamples[ i ], sizeof( SAMPLE )*overframe, 0, ( struct sockaddr * )&server_addr1, sizeof( struct sockaddr ) );
		if ( bytes < 0 )
		{
			perror( "sendto" );
			return 0;
		}
		rflag = 1;
		//printf("发送完成\n");//可打印
		err = Pa_StopStream( stream );
		if ( err != paNoError ) goto done;
	}
done:
	err = Pa_CloseStream( stream );
	if ( err != paNoError ) goto done;
	Pa_Terminate( );
	if ( data.recordedSamples )       /* Sure it is NULL or valid. */
		free( data.recordedSamples );
	if ( err != paNoError )
	{
		fprintf( stderr, "An error occured while using the portaudio stream\n" );
		fprintf( stderr, "Error number: %d\n", err );
		fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
		err = 1;          /* Always return 0 or 1, but no other return codes. */
	}
	//    return err;
}
//接收播放线程
void *thread_recive_play( )
{
	PaStreamParameters  inputParameters,
		outputParameters;
	PaStream*           stream;
	PaError             err = paNoError;
	paTestData          data;
	int                 i;
	int                 totalFrames;
	int                 numSamples;
	int                 numBytes;
	int bytes;
	int addr_len;

	//printf("recive_play thread!\n");//可打印
	data.maxFrameIndex = totalFrames = NUM_SECONDS * SAMPLE_RATE; /* Record for a few seconds. */
	data.frameIndex = 0;
	numSamples = totalFrames * NUM_CHANNELS;
	//printf("numSamples = %d\n", numSamples);//
	numBytes = numSamples * sizeof( SAMPLE );
	//printf("numBytes = %d\n", numBytes);//
	data.recordedSamples = ( SAMPLE * )malloc( numBytes ); /* From now on, recordedSamples is initialised. */
														   //    printf("data.recordedSamples = %d\n", sizeof(SAMPLE)*numBytes);
	if ( data.recordedSamples == NULL )
	{
		printf( "Could not allocate record array.\n" );
		goto done;
	}
	for ( i = 0; i < numSamples; i++ ) data.recordedSamples[ i ] = 0;

	//    err = Pa_Initialize();
	//    if( err != paNoError ) goto done;

	/* Playback recorded data.  -------------------------------------------- */
	outputParameters.device = get_device( Pa_GetDefaultOutputDevice( ) ); /* default output device */
	if ( outputParameters.device == paNoDevice ) {
		fprintf( stderr, "RecivePlay:Error: No default output device.\n" );
		goto done;
	}
	printf( "outputParameters.device = %d\n", outputParameters.device );
	outputParameters.channelCount = 2;                     /* stereo output */
	outputParameters.sampleFormat = PA_SAMPLE_TYPE;
	outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
	outputParameters.hostApiSpecificStreamInfo = NULL;

	printf( "\n=== Now playing back. ===\n" ); fflush( stdout );
	err = Pa_OpenStream(
		&	,
		NULL, /* no input */
		&outputParameters,
		SAMPLE_RATE,
		FRAMES_PER_BUFFER,
		paClipOff,      /* we won't output out of range samples so don't bother clipping them */
		playCallback,
		&data );
	if ( err != paNoError ) goto done;

	addr_len = sizeof( client_addr );
	if ( stream )
	{
		while ( rflag )
		{
			int n = 0, overframe = 0;
			i = 0;

			while ( n < numSamples / 256 )
			{
				//printf("准备接收\n");//可打印
				data.frameIndex = i;
				bytes = recvfrom( sockfd_r, &data.recordedSamples[ i ], sizeof( SAMPLE ) * 256, 0, ( struct sockaddr* )&client_addr, &addr_len );
				//printf("recbytes = %d\n",bytes);//可打印
				if ( bytes < 0 )
				{
					perror( "recvfrom" );
					//                                        return 1;
				}
				n++;
				i += 256;
				sflag = 1;
			}
			/*接收剩余的帪数据*/
			overframe = numSamples - ( numSamples / 256 ) * 256;
			bytes = recvfrom( sockfd_r, &data.recordedSamples[ i ], sizeof( SAMPLE )*overframe, 0, ( struct sockaddr* )&client_addr, &addr_len );
			if ( bytes < 0 )
			{
				perror( "recvfrom" );
				//                                return 1;
			}
			data.frameIndex = 0;
			err = Pa_StartStream( stream );
			if ( err != paNoError ) goto done;

			//                        printf("Waiting for playback to finish.\n"); fflush(stdout);

			while ( ( err = Pa_IsStreamActive( stream ) ) == 1 )
			{
				//printf("ok!\n");
				Pa_Sleep( 100 );
				//printf("recindex = %d\n", data.frameIndex ); fflush(stdout);//可打印
			}
			if ( err < 0 ) goto done;

			err = Pa_StopStream( stream );
			if ( err != paNoError ) goto done;

			//                        printf("Done.\n"); fflush(stdout);
		}
	}

done:
	err = Pa_CloseStream( stream );
	if ( err != paNoError ) goto done;
	Pa_Terminate( );
	if ( data.recordedSamples )       /* Sure it is NULL or valid. */
		free( data.recordedSamples );
	if ( err != paNoError )
	{
		fprintf( stderr, "An error occured while using the portaudio stream\n" );
		fprintf( stderr, "Error number: %d\n", err );
		fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
		err = 1;          /* Always return 0 or 1, but no other return codes. */
	}

}

//创建主叫线程
void *thread_call( )
{
	Get_Clientip_From_Server( );

	char buff[ 20 ];
	int bytes;
	int ret;
	memset( buff, 0, sizeof( buff ) );
	scanf( "%s", buff );
	if ( strcmp( buff, "call" ) == 0 )
	{
		if ( ( bytes = send( client_sockfd_c, buff, strlen( buff ) + 1, 0 ) ) < 0 )
		{
			perror( "send\n" );
		}
		else
		{
			printf( "send to client:%s\n", buff );
		}
		memset( buff, 0, sizeof( buff ) );
		if ( ( bytes = recv( client_sockfd_c, buff, sizeof( buff ), 0 ) ) < 0 )
		{
			perror( "recv\n" );
		}
		else
		{
			printf( "recv form client:%s\n", buff );
		}
		if ( strcmp( buff, "answer" ) == 0 )
		{
			pthread_create( &thread_rp, NULL, thread_recive_play, NULL );
			pthread_create( &thread_rs, NULL, thread_record_send, ( void * )SERVER_IP );

			memset( buff, 0, sizeof( buff ) );
			strcpy( buff, "star" );
			if ( strcmp( buff, "star" ) == 0 )
			{
				if ( ( bytes = send( client_sockfd_c, buff, sizeof( buff ), 0 ) ) < 0 )
				{
					perror( "send\n" );
				}
				else
				{
					printf( "send to client:%s\n", buff );
				}
			}
			while ( 1 )
			{
				fd_set readfd_call;
				FD_ZERO( &readfd_call );//清空描述符集
				FD_SET( 0, &readfd_call );//添加描述符集
				FD_SET( client_sockfd_c, &readfd_call );
				ret = select( client_sockfd_c + 1, &readfd_call, 0, 0, 0 );//检测TCP通信套接字

				if ( ret == -1 )
				{
					perror( "select\n" );
				}
				else if ( ret == 0 )
				{
					//printf("time out\n");
				}

				if ( FD_ISSET( 0, &readfd_call ) )
				{
					memset( buff, 0, sizeof( buff ) );
					scanf( "%s", buff );
					if ( strcmp( buff, "hangup" ) == 0 )
					{
						if ( ( bytes = send( client_sockfd_c, buff, strlen( buff ) + 1, 0 ) ) < 0 )
						{
							perror( "send\n" );
						}
						else
						{
							printf( "send to client:%s\n", buff );
							sleep( 1 );
						}

						flag_call = 1;
						pthread_cancel( thread_rs );
						pthread_cancel( thread_rp );
						pthread_exit( NULL );

					}
				}
				else if ( FD_ISSET( client_sockfd_c, &readfd_call ) )
				{
					memset( buff, 0, sizeof( buff ) );
					if ( ( bytes = recv( client_sockfd_c, buff, sizeof( buff ), 0 ) ) < 0 )
					{
						perror( "recv\n" );
					}
					else
					{
						printf( "recv form client:%s\n", buff );
						if ( strcmp( buff, "hangup" ) == 0 )
						{
							flag_call = 1;
							pthread_cancel( thread_rs );
							pthread_cancel( thread_rp );
							pthread_exit( NULL );
						}
					}

				}
			}

			pthread_join( thread_rp, &value );
			pthread_join( thread_rs, &value );

		}
		else if ( strcmp( buff, "break" ) == 0 )
		{
			printf( "对方暂时不方便接听\n" );
			flag_call = 1;
			pthread_exit( NULL );
		}
	}

}
//被叫线程
void *thread_becall( )
{
	int ret;
	memset( buffer, 0, sizeof( buffer ) );

	if ( ( bytes = recv( client_sockfd, buffer, sizeof( buffer ), 0 ) ) < 0 )
	{
		perror( "recv\n" );
	}
	else
	{
		printf( "recv form clinet %s\n", buffer );
	}
	if ( strcmp( buffer, "call" ) == 0 )
	{
		printf( "%s  来电\n", inet_ntoa( Tclient_addr.sin_addr.s_addr ) );

		memset( buffer, 0, sizeof( buffer ) );
		//strcpy(buffer,"answer");
		scanf( "%s", buffer );
		if ( strcmp( buffer, "answer" ) == 0 )
		{
			if ( ( bytes = send( client_sockfd, buffer, sizeof( buffer ), 0 ) ) < 0 )
			{
				perror( "send\n" );
			}
			else
			{
				pthread_create( &thread_rp, NULL, thread_recive_play, NULL );
				printf( "send to server %s\n", buffer );
			}

			memset( buffer, 0, sizeof( buffer ) );
			if ( ( bytes = recv( client_sockfd, buffer, sizeof( buffer ), 0 ) ) < 0 )
			{
				perror( "recv\n" );
			}
			else
			{
				printf( "recv form clinet %s\n", buffer );
			}
			if ( strcmp( buffer, "star" ) == 0 )
			{
				printf( "准备创建发送线程\n" );
				pthread_create( &thread_rs, NULL, thread_record_send, ( void * )CLIENT_IP );
			}
		}

		while ( 1 )
		{
			fd_set readfd_becall;
			FD_ZERO( &readfd_becall );//清空描述符集
			FD_SET( 0, &readfd_becall );//添加描述符集
			FD_SET( client_sockfd, &readfd_becall );
			ret = select( client_sockfd + 1, &readfd_becall, 0, 0, 0 );//检测TCP通信套接字
			if ( ret == -1 )
			{
				perror( "select\n" );
			}
			else if ( ret == 0 )
			{
				//printf("time out\n");
			}

			if ( FD_ISSET( 0, &readfd_becall ) )
			{

				memset( buffer, 0, sizeof( buffer ) );
				scanf( "%s", buffer );
				if ( strcmp( buffer, "hangup" ) == 0 )
				{
					if ( ( bytes = send( client_sockfd, buffer, strlen( buffer ) + 1, 0 ) ) < 0 )
					{
						perror( "send\n" );
					}
					else
					{
						printf( "send to client:%s\n", buffer );
						sleep( 3 );
					}

					flag_becall = 1;
					pthread_cancel( thread_rs );
					pthread_cancel( thread_rp );
					pthread_exit( NULL );


				}
			}
			else if ( FD_ISSET( client_sockfd, &readfd_becall ) )
			{
				memset( buffer, 0, sizeof( buffer ) );
				printf( "client_sockfd%d\n", client_sockfd );
				if ( ( bytes = recv( client_sockfd, buffer, sizeof( buffer ), 0 ) ) < 0 )
				{
					perror( "recv\n" );
				}
				else
				{
					printf( "recv form client:%s\n", buffer );
					if ( strcmp( buffer, "hangup" ) == 0 )
					{
						flag_becall = 1;
						//flag_call = 1;
						pthread_cancel( thread_rs );
						pthread_cancel( thread_rp );
						pthread_exit( NULL );

					}
				}
			}
		}

		pthread_join( thread_rp, &value );
		pthread_join( thread_rs, &value );
	}

	else if ( strcmp( buffer, "break" ) == 0 )
	{
		printf( "拒接\n" );
		flag_becall = 1;
		if ( ( bytes = send( client_sockfd, buffer, sizeof( buffer ), 0 ) ) < 0 )
		{
			perror( "send\n" );
		}
		else
		{
			printf( "send to server %s\n", buffer );
			pthread_exit( NULL );
		}
	}
}

//接收套接字初始化
void init_sockfd_r( )
{
	int rel, optval = 0;
	int optlen = sizeof( int );
	//创建udp套接字
	if ( ( sockfd_r = socket( AF_INET, SOCK_DGRAM, 0 ) ) == -1 )
	{
		perror( "socket" );
		//                return 1;
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons( PORT );
	server_addr.sin_addr.s_addr = INADDR_ANY;
	printf( "sockfd_r = %d\n", sockfd_r );

	const char *optval1 = "qqqqq";
	setsockopt( sockfd_r, SOL_SOCKET, SO_REUSEADDR, optval1, sizeof( optval1 ) );//端口重利用
	if ( bind( sockfd_r, ( struct sockaddr * )&server_addr, sizeof( struct sockaddr ) ) == -1 )
	{
		perror( "bind" );
		//                return 1;
	}

	optval = 131071;
	rel = setsockopt( sockfd_r, SOL_SOCKET, SO_RCVBUF, ( const void* )&optval, optlen );
	if ( rel < 0 )
		perror( "setsockopt" );

	rel = getsockopt( sockfd_r, SOL_SOCKET, SO_RCVBUF, ( void* )&optval, &optlen );
	if ( rel < 0 )
		perror( "getsockopt" );
	//printf("SO_RCVBUF = %d\n", optval);
}

void init_sockfd_s( char *remote )
{
	int rel, optval = 0;
	int optlen = sizeof( int );
	//创建udp套接字
	if ( ( sockfd_s = socket( AF_INET, SOCK_DGRAM, 0 ) ) == -1 )
	{
		perror( "socket" );
		//                return 1;
	}

	client_addr.sin_family = AF_INET;
	client_addr.sin_port = htons( PORT );
	client_addr.sin_addr.s_addr = inet_addr( remote );

	optval = 131071;
	rel = setsockopt( sockfd_s, SOL_SOCKET, SO_SNDBUF, ( const void* )&optval, optlen );
	if ( rel < 0 )
		perror( "setsockopt" );

	rel = getsockopt( sockfd_s, SOL_SOCKET, SO_SNDBUF, ( void* )&optval, &optlen );
	if ( rel < 0 )
		perror( "getsockopt" );
	printf( "SO_SNDBUF = %d\n", optval );
}
/*******************************************************************/
int main( );
int main( )
{
	fd_set readfd;
	int ret;
	int addr_size;

	while ( 1 )
	{
		open( sockfd_r );
		open( server_sockfd );
		init_sockfd_r( );
		Pa_Initialize( );

		Print_cmd( );
		Create_Server_To_Client_Socket( );//监听等待对方来电

		flag_call = 0;
		flag_becall = 0;

		FD_ZERO( &readfd );//清空描述符集
		FD_SET( 0, &readfd );//添加描述符集
		FD_SET( server_sockfd, &readfd );

		ret = select( server_sockfd + 1, &readfd, 0, 0, 0 );
		if ( ret == -1 )
		{
			perror( "select\n" );
		}
		else if ( ret == 0 )
		{
			//printf("time out\n");
		}
		else if ( FD_ISSET( 0, &readfd ) )
		{
			memset( buffer, 0, sizeof( buffer ) );
			scanf( "%s", buffer );
			if ( strcmp( buffer, "call" ) == 0 )
			{
				//printf("创建主叫线程\n");
				sflag = 1;
				rflag = 1;
				pthread_create( &thread_zhujiao, NULL, thread_call, NULL );
				pthread_join( thread_zhujiao, &value );

				if ( flag_call == 1 )
				{
					close( sockfd_r );
					close( server_sockfd );
					continue;
				}
			}

		}
		else if ( FD_ISSET( server_sockfd, &readfd ) )
		{
			printf( "创建被叫\n" );
			rflag = 1;
			sflag = 0;
			addr_size = sizeof( struct sockaddr );
			if ( ( client_sockfd = accept( server_sockfd, ( struct sockaddr * )&Tclient_addr, &addr_size ) ) == -1 )
			{
				perror( "accept\n" );
				exit( 1 );
			}
			//printf("clinet_addr = %s\n",inet_ntoa(Tclient_addr.sin_addr.s_addr));
			strcpy( CLIENT_IP, inet_ntoa( Tclient_addr.sin_addr.s_addr ) );//得到对方的IP地址

			printf( "CLIENT_IP:%s\n", CLIENT_IP );
			pthread_create( &thread_beijiao, NULL, thread_becall, NULL );
			pthread_join( thread_beijiao, &value );

			if ( flag_becall == 1 )
			{
				close( sockfd_r );
				close( server_sockfd );
				continue;
			}
		}
	}

}