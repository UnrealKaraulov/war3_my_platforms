#pragma once
#include <stdint.h>
#include <string>
#include <algorithm>
using namespace std;

enum IconType
{
	Active,
	Passive,
	Autocast
};

enum InfocardIconType
{
	Normal,
	Level
};

enum PictureType
{
	bmp,
	tga,
	jpg,
	blp
};
#include <pshpack1.h>

struct BLPHeader
{
	char     ident[ 4 ];       // Always 'BLP1'
	uint32_t    compress;           // 0: JPEG, 1: palette
	uint32_t    IsAlpha;          // 8: Alpha
	uint32_t    sizex;          // In pixels, power-of-two
	uint32_t    sizey;
	uint32_t    alphaEncoding;  // 3, 4: Alpha list, 5: Alpha from palette
	uint32_t    flags2;         // Unused
	uint32_t    poffs[ 16 ];
	uint32_t    psize[ 16 ];
};


struct TGAHeader {
	char  imageIDLength;
	char  colorMapType;
	char  imageType;
	short int colourmaporigin;
	short int colourmaplength;
	char  colourmapdepth;
	short int x_origin;
	short int y_origin;
	short width;
	short height;
	char  bpp;
	char  imagedescriptor;
};



struct tBGRAPixel
{
	uint8_t b;
	uint8_t g;
	uint8_t r;
	uint8_t a;
};


unsigned char FixBounds(int i);
unsigned char FixBounds(double i);
unsigned char FixBounds(float i);

struct RGBAPix
{
	unsigned char R;
	unsigned char G;
	unsigned char B;
	unsigned char A;

	RGBAPix( )
	{
		this->R = 0;
		this->G = 0;
		this->B = 0;
		this->A = 0;
	}

	RGBAPix( unsigned char R, unsigned char G, unsigned char B, unsigned char A )
	{
		this->R = R;
		this->G = G;
		this->B = B;
		this->A = A;
	}

	RGBAPix RGBAPixWar3( unsigned char R, unsigned char G, unsigned char B, unsigned char A )
	{
		this->R = B;
		this->G = G;
		this->B = R;
		this->A = A;
		return *this;
	}


	unsigned int RGBAPixWar3_u( unsigned char R, unsigned char G, unsigned char B, unsigned char A )
	{
		return ( unsigned int )( this->A << 24 | this->R << 16 | this->G << 8 | this->B << 0 );
	}

	unsigned int ToUINT( )
	{
		return ( unsigned int )( this->A << 24 | this->R << 16 | this->G << 8 | this->B << 0 );
	}

	unsigned int FromUINT( unsigned int color )
	{
		return ( unsigned int )( this->A << 24 | this->R << 16 | this->G << 8 | this->B << 0 );
	}

	RGBAPix FromString( const char * text )
	{
		if ( text && strlen( text ) == 8 )
		{
			char colorstr[ 11 ];
			colorstr[ 0 ] = '0';// text[ i + 2 ];
			colorstr[ 1 ] = 'x';//text[ i + 3 ];
			//A
			colorstr[ 2 ] = text[ 0 ];
			colorstr[ 3 ] = text[ 1 ];
			//R
			colorstr[ 4 ] = text[ 6 ];
			colorstr[ 5 ] = text[ 7 ];
			//G
			colorstr[ 6 ] = text[ 4 ];
			colorstr[ 7 ] = text[ 5 ];
			//B
			colorstr[ 8 ] = text[ 2 ];
			colorstr[ 9 ] = text[ 3 ];
			colorstr[ 10 ] = '\0';

			// Смысла от прозрачного текста нет так что считаем что FF это 0 прозрачность
			this->FromUINT( strtoul( colorstr, NULL, 0 ) );
		}
		else
		{
			this->R = 0;
			this->G = 0;
			this->B = 0;
			this->A = 0;
		}
		return *this;
	}

	RGBAPix operator * ( RGBAPix pix )
	{
		if ( pix.A > 0 && this->A > 0 )
			this->A = FixBounds( 255.f / ( this->A / pix.A ) );
		else
			this->A = 0;
		if ( pix.R > 0 && this->R > 0 )
			this->R = FixBounds( 255.f / ( this->R / pix.R ) );
		else
			this->R = 0;
		if ( pix.G > 0 && this->G > 0 )
			this->G = FixBounds( 255.f / ( this->G / pix.G ) );
		else
			this->G = 0;
		if ( pix.B > 0 && this->B > 0 )
			this->B = FixBounds( 255.f / ( this->B / pix.B ) );
		else
			this->B = 0;


		return ( *this );
	}
	RGBAPix operator + ( RGBAPix pix )
	{
		this->A = FixBounds( this->A + pix.A );
		this->R = FixBounds( this->R + pix.R );
		this->G = FixBounds( this->G + pix.G );
		this->B = FixBounds( this->B + pix.B );

		return ( *this );
	}

	RGBAPix ToBRGAPix()
	{
		RGBAPix tmp = RGBAPix();
		tmp.R = this->B;
		tmp.G = this->G;
		tmp.B = this->R;
		tmp.A = this->A;
		this->R = tmp.R;
		this->G = tmp.G;
		this->B = tmp.B;
		this->A = tmp.A;
		return tmp;
	}


	RGBAPix operator / ( RGBAPix pix )
	{
		if ( pix.A > 0 && this->A > 0 )
			this->A = FixBounds( 255.f / ( pix.A / this->A ) );
		else
			this->A = 255;
		if ( pix.R > 0 && this->R > 0 )
			this->R = FixBounds( 255.f / ( pix.R / this->R ) );
		else
			this->R = 255;
		if ( pix.G > 0 && this->G > 0 )
			this->G = FixBounds( 255.f / ( pix.G / this->G ) );
		else
			this->G = 255;
		if ( pix.B > 0 && this->B > 0 )
			this->B = FixBounds( 255.f / ( pix.B / this->B ) );
		else
			this->B = 255;


		return ( *this );
	}

	RGBAPix operator - ( RGBAPix pix )
	{
		this->A = FixBounds( this->A - pix.A );
		this->R = FixBounds( this->R - pix.R );
		this->G = FixBounds( this->G - pix.G );
		this->B = FixBounds( this->B - pix.B );

		return ( *this );
	}


};
typedef struct RGBAPix RGBAPix;

struct RGBPix
{
	unsigned char R;
	unsigned char G;
	unsigned char B;

	RGBPix( )
	{
		this->R = 0;
		this->G = 0;
		this->B = 0;
	}

	RGBPix( unsigned char R, unsigned char G, unsigned char B, unsigned char A )
	{
		this->R = R;
		this->G = G;
		this->B = B;
	}

	RGBPix RGBPixWar3( unsigned char R, unsigned char G, unsigned char B, unsigned char A )
	{
		this->R = B;
		this->G = G;
		this->B = R;
		return *this;
	}



	unsigned int RGBPixWar3_u( unsigned char R, unsigned char G, unsigned char B, unsigned char A )
	{
		return ( unsigned int )( 0 << 24 | this->R << 16 | this->G << 8 | this->B << 0 );
	}

	unsigned int ToUINT( )
	{
		return ( unsigned int )( 0 << 24 | this->R << 16 | this->G << 8 | this->B << 0 );
	}

	RGBAPix ToRGBAPix()
	{
		RGBAPix tmp = RGBAPix();
		tmp.R = this->R;
		tmp.G = this->G;
		tmp.B = this->B;
		tmp.A = 255;
		return tmp;
	}

	RGBAPix ToGBRAPix()
	{
		RGBAPix tmp = RGBAPix();
		tmp.R = this->G;
		tmp.G = this->B;
		tmp.B = this->R;
		tmp.A = 255;
		return tmp;
	}

	RGBAPix ToBGRAPix()
	{
		RGBAPix tmp = RGBAPix();
		tmp.R = this->B;
		tmp.G = this->G;
		tmp.B = this->R;
		tmp.A = 255;
		return tmp;
	}

	RGBAPix ToBRGAPix()
	{
		RGBAPix tmp = RGBAPix();
		tmp.R = this->B;
		tmp.G = this->R;
		tmp.B = this->G;
		tmp.A = 255;
		return tmp;
	}



	RGBAPix ToGRBAPix()
	{
		RGBAPix tmp = RGBAPix();
		tmp.R = this->G;
		tmp.G = this->R;
		tmp.B = this->B;
		tmp.A = 255;
		return tmp;
	}


};
typedef struct RGBPix RGBPix;



struct PAPix
{
	unsigned char i;
	unsigned char A;
};
typedef struct PAPix PAPix;

struct PPix
{
	unsigned char i;
};
typedef struct PPix PPix;
#include <poppack.h>


#pragma pack(push,1)


//extern int memoryleakcheck;

void  __stdcall AddNewLineToJassLog(const char* s);
void __stdcall  AddNewLineToDotaChatLog(const char* s);
void __stdcall  AddNewLineToDotaHelperLog(const char* s, int line);//( const char * s, int line );
void __stdcall  AddNewLineToJassNativesLog(const char* s);
void __stdcall EnableErrorHandler(int);
void __stdcall DisableErrorHandler(int);


//extern int memoryleakcheck;

class StormBuffer
{
private:

public:
	char* buf;
	unsigned long length;
	bool NeedClear = false;
	/*~StormBuffer( )
	{
	Clear( );
	}*/
	StormBuffer()
	{

		buf = 0;
		length = 0;
	}
	StormBuffer(unsigned long l)
	{
		//	memoryleakcheck++;
		length = l;
		buf = (char*)malloc(l + 1);
		NeedClear = true;
		buf[l] = '\0';
	}
	StormBuffer(char* b, unsigned long l)
	{

		buf = b;
		length = l;
	}
	void Resize(unsigned long l)
	{

		Clear();
		buf = (char*)malloc(l + 1);
		NeedClear = true;
		buf[l] = '\0';
		length = l;

	}

	char* GetData()
	{

		return buf;
	}
	char* GetData(int offset)
	{

		return buf + offset;
	}

	unsigned long GetSize()
	{

		return length;
	}

	void Clear()
	{
		length = 0;
		if (buf != NULL)
		{
			free(buf);
		}
		buf = NULL;
	}

	StormBuffer& Clone(StormBuffer& CopyObject)
	{
		Resize(CopyObject.length);
		std::memcpy(buf, CopyObject.GetData(), length);
		return (*this);
	}

	StormBuffer& operator =(StormBuffer& CopyObject)
	{

		/*Resize( CopyObject.length );
		std::memcpy( buf, CopyObject.GetData( ), length );*/
		length = CopyObject.length;
		buf = CopyObject.buf;
		return (*this);
	}
	StormBuffer& operator =(std::string& CopyString)
	{

		Resize(static_cast<int>(CopyString.size()));
		std::memcpy(buf, CopyString.c_str(), length);
		return (*this);
	}

	char& operator [](int Index)
	{
		return buf[Index];
	}


};

typedef struct StormBufferList
{
	char** buf;
	unsigned long length;
	StormBufferList()
	{

		buf = 0;
		length = 0;
	}
	StormBufferList(unsigned long l)
	{

		buf = (char**)malloc(l);
		length = l;
	}
	StormBufferList(char** b, unsigned long l)
	{

		buf = b;
		length = l;
	}
} StormBufferList;


#pragma pack(pop)

typedef RGBAPix palette[ 256 ];
unsigned char * Scale_WithoutResize( unsigned char *pixels, size_t width, size_t height, size_t newwidth, size_t newheight, size_t bytes_per_pixel );
unsigned long Blp2Raw( StormBuffer input, StormBuffer &output, int &width, int &height, int &bpp, int &mipmaps, int & alphaflag, int & compresstype, int & pictype, char const *filename );
bool TGA2Raw( StormBuffer input, StormBuffer &output, int &width, int &height, int &bpp, const char* filename );
bool BMP2Raw( StormBuffer input, StormBuffer &output, int &width, int &height, int &bpp, const char* filename );
bool JPG2Raw( StormBuffer input, StormBuffer &output, int &width, int &height, int &bpp, const char* filename );
bool CreatePalettedBLP( StormBuffer rawData, StormBuffer &output, int colors, char const *filename, int width, int height, int bytespp, int  alphaflag, int &maxmipmaps );
bool RAW2Jpg( StormBuffer input, StormBuffer &output, int width, int height, int bpp, const char* filename );
bool RAW2Png(StormBuffer input, StormBuffer& output, int width, int height, int bpp, const char* filename);
bool CreateJpgBLP( StormBuffer rawData, StormBuffer &output, int quality, char const *filename, int width, int height, int bytespp, int  alphaflag, int &maxmipmaps );
void textureInvertRBInPlace( RGBAPix *bufsrc, unsigned long srcsize );
void ScaleImage( unsigned char* rawData, int oldW, int oldH, int newW, int newH, int bytespp, StormBuffer &target );
bool ApplyOverlay( unsigned char* rawData, unsigned char* mask, int width, int height, int bytespp, int maskBpp );
bool ApplyBorder( unsigned char* rawData, unsigned char* mask, int width, int height, int bytespp, int borderBpp );
void flip_vertically(unsigned char *pixels, size_t width, size_t height, size_t bytes_per_pixel);
int ArrayXYtoId( int width, int x, int y );
