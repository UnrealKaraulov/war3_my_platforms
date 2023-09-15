#include "BlpReadWrite.h"
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <math.h>


//#include "jpgwrapper.h"
#include <fstream>
#include  <cstdlib>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

unsigned char FixBounds(int i)
{
	if (i > 0xFF)
		return 0xFF;
	else if (i < 0x00)
		return 0x00;
	return (unsigned char)i;
}

unsigned char FixBounds(float i)
{
	if (i > (double)0xFF)
		return 0xFF;
	else if (i < (double)0x00)
		return 0x00;
	return (unsigned char)i;
}

unsigned char FixBounds(double i)
{
	if (i > (double)0xFF)
		return 0xFF;
	else if (i < (double)0x00)
		return 0x00;
	return (unsigned char)i;
}



bool IsPowerOfTwo( const long i )
{
	long t = i;
	while ( t % 2 == 0 )
		t >>= 1;
	return ( t == 1 );
}


bool GetFirstBytes( const char* filename, char* buffer, unsigned long length )
{
	FILE* file;
	fopen_s( &file, filename, "rb" );
	if ( !file )
		return false;
	if ( fread( buffer, 1, length, file ) < length )
	{
		fclose( file );
		return false;
	}
	fclose( file );
	return true;
}


bool MaskOk( unsigned char *mask, int expectedWidth, int expectedHeight, int expectedBpp, long &offset, const char *maskFile )
{
	TGAHeader* header = ( TGAHeader* )mask;
	if ( header->colorMapType != 0 || header->imageType != 2 || header->width == 0 || header->height == 0 )
		return false;

	if ( header->width != expectedWidth || header->height != expectedHeight )
		return false;

	if ( header->bpp / 8 != expectedBpp )
		return false;

	offset = ( long )( sizeof( TGAHeader ) + header->imageIDLength );
	return true;
}

template<typename T>
inline void AssignWeightedPixel( double *target, T *source, double weight, int bytespp, bool add )
{
	if ( !add )
	{
		target[ 0 ] = ( ( double )source[ 0 ] ) * weight;
		target[ 1 ] = ( ( double )source[ 1 ] ) * weight;
		target[ 2 ] = ( ( double )source[ 2 ] ) * weight;
		if ( bytespp == 4 )
			target[ 3 ] = ( ( double )source[ 3 ] ) * weight;
	}
	else
	{
		target[ 0 ] += ( ( double )source[ 0 ] ) * weight;
		target[ 1 ] += ( ( double )source[ 1 ] ) * weight;
		target[ 2 ] += ( ( double )source[ 2 ] ) * weight;
		if ( bytespp == 4 )
			target[ 3 ] += ( ( double )source[ 3 ] ) * weight;
	}
}

inline unsigned char NormalizeComponent( double val )
{
	if ( val < 0.0 )
		return 0;
	if ( val > ( double )0xFF )
		return 0xFF;
	return ( unsigned char )val;
}


static int* g_px1a = NULL;
static int  g_px1a_w = 0;
static int* g_px1ab = NULL;
static int  g_px1ab_w = 0;


void Resize_HQ_4ch( unsigned char* src, int w1, int h1,
	int w2, int h2, StormBuffer& outdest )
{
	unsigned char * dest = ( unsigned char * )malloc( w2*h2 * 4 );

	// Both buffers must be in ARGB format, and a scanline should be w*4 bytes.

	// If pQuitFlag is non-NULL, then at the end of each scanline, it will check
	//    the value at *pQuitFlag; if it's set to 'true', this function will abort.
	// (This is handy if you're background-loading an image, and decide to cancel.)

	// NOTE: THIS WILL OVERFLOW for really major downsizing (2800x2800 to 1x1 or more) 
	// (2800 ~ sqrt(2^23)) - for a lazy fix, just call this in two passes.

	/*assert( src );
	assert( dest );
	assert( w1 >= 1 );
	assert( h1 >= 1 );
	assert( w2 >= 1 );
	assert( h2 >= 1 );
*/
// check for MMX (one time only)


	if ( w2 * 2 == w1 && h2 * 2 == h1 )
	{
		// perfect 2x2:1 case - faster code
		// (especially important because this is common for generating low (large) mip levels!)
		unsigned int *dsrc = ( unsigned int* )src;
		unsigned int *ddest = ( unsigned int* )dest;


		unsigned int remainder = 0;
		int i = 0;
		for ( int y2 = 0; y2 < h2; y2++ )
		{
			int y1 = y2 * 2;
			unsigned int* temp_src = &dsrc[ y1*w1 ];
			for ( int x2 = 0; x2 < w2; x2++ )
			{
				unsigned int xUL = temp_src[ 0 ];
				unsigned int xUR = temp_src[ 1 ];
				unsigned int xLL = temp_src[ w1 ];
				unsigned int xLR = temp_src[ w1 + 1 ];
				// note: unsigned int packing is 0xAARRGGBB

				unsigned int redblue = ( xUL & 0x00FF00FF ) + ( xUR & 0x00FF00FF ) + ( xLL & 0x00FF00FF ) + ( xLR & 0x00FF00FF ) + ( remainder & 0x00FF00FF );
				unsigned int green = ( xUL & 0x0000FF00 ) + ( xUR & 0x0000FF00 ) + ( xLL & 0x0000FF00 ) + ( xLR & 0x0000FF00 ) + ( remainder & 0x0000FF00 );
				// redblue = 000000rr rrrrrrrr 000000bb bbbbbbbb
				// green   = xxxxxx00 000000gg gggggggg 00000000
				remainder = ( redblue & 0x00030003 ) | ( green & 0x00000300 );
				ddest[ i++ ] = ( ( redblue & 0x03FC03FC ) | ( green & 0x0003FC00 ) ) >> 2;

				temp_src += 2;
			}

		}

	}
	else
	{
		// arbitrary resize.
		unsigned int *dsrc = ( unsigned int * )src;
		unsigned int *ddest = ( unsigned int * )dest;

		bool bUpsampleX = ( w1 < w2 );
		bool bUpsampleY = ( h1 < h2 );

		// If too many input pixels map to one output pixel, our 32-bit accumulation values
		// could overflow - so, if we have huge mappings like that, cut down the weights:
		//    256 max color value
		//   *256 weight_x
		//   *256 weight_y
		//   *256 (16*16) maximum # of input pixels (x,y) - unless we cut the weights down...
		int weight_shift = 0;
		float source_texels_per_out_pixel = ( ( w1 / ( float )w2 + 1 )
			* ( h1 / ( float )h2 + 1 )
			);
		float weight_per_pixel = source_texels_per_out_pixel * 256 * 256;  //weight_x * weight_y
		float accum_per_pixel = weight_per_pixel * 256; //color value is 0-255
		float weight_div = accum_per_pixel / 4294967000.0f;
		if ( weight_div > 1 )
			weight_shift = ( int )ceilf( logf( ( float )weight_div ) / logf( 2.0f ) );
		weight_shift = min( 15, weight_shift );  // this could go to 15 and still be ok.

		float fh = 256 * h1 / ( float )h2;
		float fw = 256 * w1 / ( float )w2;

		if ( bUpsampleX && bUpsampleY )
		{
			// faster to just do 2x2 bilinear interp here

			// cache x1a, x1b for all the columns:
			// ...and your OS better have garbage collection on process exit :)
			if ( g_px1a_w < w2 )
			{
				if ( g_px1a ) delete[ ] g_px1a;
				g_px1a = new int[ w2 * 2 * 1 ];
				g_px1a_w = w2 * 2;
			}
			for ( int x2 = 0; x2 < w2; x2++ )
			{
				// find the x-range of input pixels that will contribute:
				int x1a = ( int )( x2*fw );
				x1a = min( x1a, 256 * ( w1 - 1 ) - 1 );
				g_px1a[ x2 ] = x1a;
			}

			// FOR EVERY OUTPUT PIXEL
			for ( int y2 = 0; y2 < h2; y2++ )
			{
				// find the y-range of input pixels that will contribute:
				int y1a = ( int )( y2*fh );
				y1a = min( y1a, 256 * ( h1 - 1 ) - 1 );
				int y1c = y1a >> 8;

				unsigned int *ddest = &( ( unsigned int * )dest )[ y2*w2 + 0 ];

				for ( int x2 = 0; x2 < w2; x2++ )
				{
					// find the x-range of input pixels that will contribute:
					int x1a = g_px1a[ x2 ];//(int)(x2*fw); 
					int x1c = x1a >> 8;

					unsigned int *dsrc2 = &dsrc[ y1c*w1 + x1c ];

					// PERFORM BILINEAR INTERPOLATION on 2x2 pixels
					unsigned int r = 0, g = 0, b = 0, a = 0;
					unsigned int weight_x = 256 - ( x1a & 0xFF );
					unsigned int weight_y = 256 - ( y1a & 0xFF );
					for ( int y = 0; y < 2; y++ )
					{
						for ( int x = 0; x < 2; x++ )
						{
							unsigned int c = dsrc2[ x + y*w1 ];
							unsigned int r_src = ( c ) & 0xFF;
							unsigned int g_src = ( c >> 8 ) & 0xFF;
							unsigned int b_src = ( c >> 16 ) & 0xFF;
							unsigned int w = ( weight_x * weight_y ) >> weight_shift;
							r += r_src * w;
							g += g_src * w;
							b += b_src * w;
							weight_x = 256 - weight_x;
						}
						weight_y = 256 - weight_y;
					}

					unsigned int c = ( ( r >> 16 ) ) | ( ( g >> 8 ) & 0xFF00 ) | ( b & 0xFF0000 );
					*ddest++ = c;//ddest[y2*w2 + x2] = c;
				}
			}
		}
		else
		{
			// cache x1a, x1b for all the columns:
			// ...and your OS better have garbage collection on process exit :)
			if ( g_px1ab_w < w2 )
			{
				if ( g_px1ab ) delete[ ] g_px1ab;
				g_px1ab = new int[ w2 * 2 * 2 ];
				g_px1ab_w = w2 * 2;
			}
			for ( int x2 = 0; x2 < w2; x2++ )
			{
				// find the x-range of input pixels that will contribute:
				int x1a = ( int )( ( x2 )*fw );
				int x1b = ( int )( ( x2 + 1 )*fw );
				if ( bUpsampleX ) // map to same pixel -> we want to interpolate between two pixels!
					x1b = x1a + 256;
				x1b = min( x1b, 256 * w1 - 1 );
				g_px1ab[ x2 * 2 + 0 ] = x1a;
				g_px1ab[ x2 * 2 + 1 ] = x1b;
			}

			// FOR EVERY OUTPUT PIXEL
			for ( int y2 = 0; y2 < h2; y2++ )
			{
				// find the y-range of input pixels that will contribute:
				int y1a = ( int )( ( y2 )*fh );
				int y1b = ( int )( ( y2 + 1 )*fh );
				if ( bUpsampleY ) // map to same pixel -> we want to interpolate between two pixels!
					y1b = y1a + 256;
				y1b = min( y1b, 256 * h1 - 1 );
				int y1c = y1a >> 8;
				int y1d = y1b >> 8;

				for ( int x2 = 0; x2 < w2; x2++ )
				{
					// find the x-range of input pixels that will contribute:
					int x1a = g_px1ab[ x2 * 2 + 0 ];    // (computed earlier)
					int x1b = g_px1ab[ x2 * 2 + 1 ];    // (computed earlier)
					int x1c = x1a >> 8;
					int x1d = x1b >> 8;

					// ADD UP ALL INPUT PIXELS CONTRIBUTING TO THIS OUTPUT PIXEL:
					unsigned int r = 0, g = 0, b = 0, a = 0;
					for ( int y = y1c; y <= y1d; y++ )
					{
						unsigned int weight_y = 256;
						if ( y1c != y1d )
						{
							if ( y == y1c )
								weight_y = 256 - ( y1a & 0xFF );
							else if ( y == y1d )
								weight_y = ( y1b & 0xFF );
						}

						unsigned int *dsrc2 = &dsrc[ y*w1 + x1c ];
						for ( int x = x1c; x <= x1d; x++ )
						{
							unsigned int weight_x = 256;
							if ( x1c != x1d )
							{
								if ( x == x1c )
									weight_x = 256 - ( x1a & 0xFF );
								else if ( x == x1d )
									weight_x = ( x1b & 0xFF );
							}

							unsigned int c = *dsrc2++;//dsrc[y*w1 + x];
							unsigned int r_src = ( c ) & 0xFF;
							unsigned int g_src = ( c >> 8 ) & 0xFF;
							unsigned int b_src = ( c >> 16 ) & 0xFF;
							unsigned int w = ( weight_x * weight_y ) >> weight_shift;
							r += r_src * w;
							g += g_src * w;
							b += b_src * w;
							a += w;
						}
					}

					// write results
					unsigned int c = ( ( r / a ) ) | ( ( g / a ) << 8 ) | ( ( b / a ) << 16 );
					*ddest++ = c;//ddest[y2*w2 + x2] = c;
				}

			}
		}
	}

	outdest.buf = ( char* )dest;
	outdest.length = w2 * h2 * 4;
}


void ScaneImageSimple( unsigned char * data, int oldW, int oldH, int newW, int newH, int bytespp, StormBuffer & target )
{
	unsigned char* newData = ( unsigned char * )malloc( newW*newH*bytespp );
	int maxpixel = oldW*oldH*bytespp - 4;

	double scaleWidth = ( double )newW / ( double )oldW;
	double scaleHeight = ( double )newH / ( double )oldH;

	int pixel = 0;
	for ( int cy = 0; cy < newH; cy++ ) {
		for ( int cx = 0; cx < newW; cx++ ) {
			int nearestMatch = ( ( int )( cy / scaleHeight )*( oldW * bytespp ) )
				+ ( ( int )( cx / scaleWidth )*bytespp );

			if ( nearestMatch > maxpixel )
			{
				nearestMatch = maxpixel;
			}

			newData[ pixel ] = data[ nearestMatch ];
			newData[ pixel + 1 ] = data[ nearestMatch + 1 ];
			newData[ pixel + 2 ] = data[ nearestMatch + 2 ];
			if ( bytespp == 4 ) newData[ pixel + 3 ] = data[ nearestMatch + 3 ];
			pixel += bytespp;
		}
	}

	target.length = newW*newH*bytespp;
	target.buf = ( char * )newData;
}

void ScaleImage( unsigned char* rawData, int oldW, int oldH, int newW, int newH, int bytespp, StormBuffer &target )
{
	ScaneImageSimple( rawData, oldW, oldH, newW, newH, bytespp, target );
	return;
	/*if ( bytespp == 4 )
	{
		Resize_HQ_4ch( rawData, oldW, oldH, newW, newH, target );
		return;
	}

	if ( oldW == newW && oldH == newH )
	{
		target.length = ( unsigned long )( newW * newH * bytespp );
		target.buf = malloc[ target.length ];
		std::memcpy( target.buf, rawData, target.length );
		return;
	}

	// scale horizontally
	double* temp = new double[ ( unsigned int )( oldH * newW * bytespp ) ];


	if ( oldW == newW )
	{
		for ( int i = 0; i < oldW * oldH * bytespp; i++ )
			temp[ i ] = ( double )rawData[ i ];
	}
	else
	{
		double sum = 0;
		double diffW = ( ( double )oldW / ( double )newW );
		for ( int i = 0; i < newW; i++ )
		{
			double newSum = sum + diffW;
			int pix = ( int )floor( sum );
			double weight = min( diffW, 1.0 - fmod( sum, 1.0 ) );
			for ( int j = 0; j < oldH; j++ )
				AssignWeightedPixel( &temp[ ( j*newW + i )*bytespp ], &rawData[ ( j*oldW + pix )*bytespp ], ( weight / diffW ), bytespp, false );
			sum += weight;
			while ( sum < newSum )
			{
				weight = min( newSum - sum, 1.0 );
				pix++;
				for ( int j = 0; j < oldH; j++ )
					AssignWeightedPixel( &temp[ ( j*newW + i )*bytespp ], &rawData[ ( j*oldW + pix )*bytespp ], ( weight / diffW ), bytespp, true );
				sum += weight;
			}
		}
	}

	// scale vertically
	target.length = ( unsigned long )( newW * newH * bytespp );
	target.buf = malloc[ target.length ];
	double* final = new double[ target.length ];


	if ( newH == oldH )
	{
		std::memcpy( final, temp, target.length * sizeof( double ) );
	}
	else
	{
		double sum = 0;
		double diffH = ( ( double )oldH / ( double )newH );
		for ( int j = 0; j < newH; j++ )
		{
			double newSum = sum + diffH;
			int pix = ( int )floor( sum );
			double weight = min( diffH, 1.0 - fmod( sum, 1.0 ) );
			for ( int i = 0; i < newW; i++ )
				AssignWeightedPixel( &final[ ( j*newW + i )*bytespp ], &temp[ ( pix*newW + i )*bytespp ], ( weight / diffH ), bytespp, false );
			sum += weight;
			while ( sum < newSum )
			{
				weight = min( newSum - sum, 1.0 );
				pix++;
				for ( int i = 0; i < newW; i++ )
					AssignWeightedPixel( &final[ ( j*newW + i )*bytespp ], &temp[ ( pix*newW + i )*bytespp ], ( weight / diffH ), bytespp, true );
				sum += weight;
			}
		}
	}
	for ( unsigned long i = 0; i < target.length; i++ )
		target.buf[ i ] = ( char )NormalizeComponent( final[ i ] );
	delete[ ] final;
	delete[ ] temp;*/
}

void SubtractColor( unsigned char &pixel, unsigned char &mask )
{
	if ( 0xFF - mask > pixel )
		pixel = 0;
	else
		pixel -= ( 0xFF - mask );
}

void DivideColor( unsigned char &pixel, unsigned char &mask )
{
	pixel = ( unsigned char )( ( double )pixel * ( ( double )mask / ( double )0xFF ) );
}

bool ApplyOverlay( unsigned char* rawData, unsigned char* mask, int width, int height, int bytespp, int maskBpp )
{
	if ( !mask )
		return false;
	for ( int i = 0; i < width * height; i++ )
	{
		DivideColor( rawData[ i*bytespp ], mask[ i*maskBpp ] );
		DivideColor( rawData[ i*bytespp + 1 ], mask[ i*maskBpp + 1 ] );
		DivideColor( rawData[ i*bytespp + 2 ], mask[ i*maskBpp + 2 ] );
		if ( bytespp == 4 && maskBpp == 4 )
			DivideColor( rawData[ i*bytespp + 3 ], mask[ i*maskBpp + 3 ] );
	}
	return true;
}

bool ApplyBorder( unsigned char* rawData, unsigned char* mask, int width, int height, int bytespp, int borderBpp )
{
	if ( !mask )
		return false;
	if ( borderBpp == 4 )
	{
		for ( int i = 0; i < width * height; i++ )
		{
			if ( mask[ i*borderBpp + 3 ] == 0xFF )
			{ // no transparence
				rawData[ i*bytespp ] = mask[ i*borderBpp ];
				rawData[ i*bytespp + 1 ] = mask[ i*borderBpp + 1 ];
				rawData[ i*bytespp + 2 ] = mask[ i*borderBpp + 2 ];
				if ( bytespp == 4 )
					rawData[ i*bytespp + 3 ] = mask[ i*borderBpp + 3 ];
			}
			else if ( mask[ i*borderBpp + 3 ] == 0x00 )
			{ // full transparence
				rawData[ i*bytespp ] = rawData[ i*bytespp + 1 ] = rawData[ i*bytespp + 2 ] = 0x00;
				if ( bytespp == 4 )
					rawData[ i*bytespp + 3 ] = 0x00;
			}
		}
	}
	else
	{
		for ( int i = 0; i < width * height; i++ )
		{
			if ( mask[ i*borderBpp ] != 0xFF || mask[ i*borderBpp + 1 ] != 0xFF || mask[ i*borderBpp + 2 ] != 0xFF )
			{ // not white
				rawData[ i*bytespp ] = mask[ i*borderBpp ];
				rawData[ i*bytespp + 1 ] = mask[ i*borderBpp + 1 ];
				rawData[ i*bytespp + 2 ] = mask[ i*borderBpp + 2 ];
			}
		}
	}
	return true;
}


int GetRequiredMipMaps( int width, int height )
{
	int mips = 0;
	while ( width > 0 && height > 0 )
	{
		mips++;
		width = width / 2;
		height = height / 2;
	}
	return mips;
}

void * Compress( const void *source,
	int width,
	int height,
	int bpp,
	int &len,
	int quality )
{
	// not supported here
	return NULL;
}


bool CreateJpgBLP( StormBuffer rawData, StormBuffer &output, int quality, char const *, int width, int height, int bytespp, int alphaflag, int &maxmipmaps )
{
	// not supported
	return NULL;
}

bool CreatePalettedBLP( StormBuffer rawData, StormBuffer &output, int colors, char const *, int width, int height, int bytespp, int alphaflag, int &maxmipmaps )
{
    // not supported
	return NULL;
}

bool TGA2Raw( StormBuffer input, StormBuffer &output, int &width, int &height, int &bpp, const char* filename )
{
	bpp = 4;
	stbi__result_info ri;
	stbi__context s;
	stbi__start_mem(&s, (const stbi_uc*)input.buf, input.length);
	output.buf = (char*)stbi__tga_load(&s, &width, &height, &bpp, 4, &ri);
	output.length = width * height * bpp;
	bpp = 4;
	return output.buf != NULL;
}

void WriteMegaPNG(void* context, void* data, int size)
{
	StormBuffer* output = (StormBuffer*)context;
	output->buf = (char*)malloc(size);
	memcpy(output->buf, data, size);
	output->length = size;
}
void WriteMegaJPG(void* context, void* data, int size)
{
	StormBuffer* output = (StormBuffer*)context;
	output->buf = (char*)malloc(size);
	memcpy(output->buf, data, size);
	output->length = size;
}

bool RAW2Jpg(StormBuffer input, StormBuffer& output, int width, int height, int bpp, const char* filename)
{
	stbi_write_jpg_to_func(WriteMegaJPG, &output, width, height, bpp, input.buf, 89);
	return true;
}

bool RAW2Png(StormBuffer input, StormBuffer& output, int width, int height, int bpp, const char* filename)
{
	stbi_write_png_compression_level = 50;
	stbi_write_png_to_func(WriteMegaPNG, &output, width, height, bpp, input.buf, width * bpp);
	return true;
}

bool BMP2Raw( StormBuffer input, StormBuffer &output, int &width, int &height, int &bpp, char const *filename )
{
//	BITMAPFILEHEADER *FileHeader = ( BITMAPFILEHEADER* )input.buf;
//	BITMAPINFOHEADER *InfoHeader = ( BITMAPINFOHEADER* )( FileHeader + 1 );
//	if ( FileHeader->bfType != 0x4D42 )
//		return false;
//
//	if ( !IsPowerOfTwo( InfoHeader->biWidth ) || !IsPowerOfTwo( InfoHeader->biHeight ) )
//		return false;
//
//	if ( InfoHeader->biBitCount != 32 && InfoHeader->biBitCount != 24 )
//		return false;
//
//	bpp = 4;
//	if ( InfoHeader->biBitCount < 32 )
//		bpp = 3;
//	width = InfoHeader->biWidth;
//	height = InfoHeader->biHeight;
//
//	output.length = ( unsigned long )( width*height*bpp );
//	output.buf = ( char * )malloc( output.length );
//	std::memcpy( output.buf, input.buf + FileHeader->bfOffBits, output.length );
//
//	if ( bpp == 4 ) // invert alpha
//		for ( int i = 0; i < width*height; i++ )
//			output.buf[ i*bpp + 3 ] = 0xFF - output.buf[ i*bpp + 3 ];
	// notsupported
	return true;
}
#define _blp_swap_int16(x) (x)
#define _blp_swap_int32(x) (x)


void SwapBLPHeader( BLPHeader *header )
{
	header->compress = ( unsigned long )_blp_swap_int32( header->compress );
	header->IsAlpha = ( unsigned long )_blp_swap_int32( header->IsAlpha );
	header->sizex = ( unsigned long )_blp_swap_int32( header->sizex );
	header->sizey = ( unsigned long )_blp_swap_int32( header->sizey );
	header->alphaEncoding = ( unsigned long )_blp_swap_int32( header->alphaEncoding );
	header->flags2 = ( unsigned long )_blp_swap_int32( header->flags2 );

	int i = 0;
	for ( ; i < 16; i++ )
	{
		( header->poffs )[ i ] = ( unsigned long )_blp_swap_int32( ( header->poffs )[ i ] );
		( header->psize )[ i ] = ( unsigned long )_blp_swap_int32( ( header->psize )[ i ] );
	}
}


void textureInvertRBInPlace( RGBAPix *bufsrc, unsigned long srcsize )
{
	for ( unsigned long i = 0; i < ( srcsize / 4 ); i++ )
	{
		unsigned char red = bufsrc[ i ].B;
		bufsrc[ i ].B = bufsrc[ i ].R;
		bufsrc[ i ].R = red;
	}
}


void flip_vertically(unsigned char *pixels, size_t width, size_t height, size_t bytes_per_pixel)
{
	if (!pixels)
		return;
	const size_t stride = width * bytes_per_pixel;
	unsigned char *row = (unsigned char *)malloc(stride);
	if (!row)
		return;
	unsigned char *low = pixels;
	unsigned char *high = &pixels[(height - 1) * stride];

	for (; low < high; low += stride, high -= stride)
	{
		std::memcpy(row, low, stride);
		std::memcpy(low, high, stride);
		std::memcpy(high, row, stride);
	}
	free(row);
}



unsigned char * Scale_WithoutResize( unsigned char *pixels, size_t width, size_t height, size_t newwidth, size_t newheight,  size_t bytes_per_pixel )
{

	if ( newwidth < width )
		return pixels;
	if ( newheight < height )
		return pixels;

	unsigned char * outimage = ( unsigned char * )malloc( newwidth * newheight * bytes_per_pixel );

	memset( outimage, 0, newwidth * newheight * bytes_per_pixel );


	size_t y = 0;

	for ( ; y < height; y++ )
	{

		unsigned char * offset1 = &pixels[ y * width * bytes_per_pixel ];
		unsigned char * offset2 = &outimage[ y * newwidth * bytes_per_pixel ];
		size_t copysize = width * bytes_per_pixel;

		std::memcpy( offset2, offset1, copysize );

	}

	free( pixels );
	return outimage;
}


tBGRAPixel * blp1_convert_paletted_separated_alpha_BGRA( uint8_t* pSrc, tBGRAPixel* pInfos, unsigned int width, unsigned int height, bool invertAlpha )
{
	tBGRAPixel* pBuffer = ( tBGRAPixel* )malloc( width * height * sizeof( tBGRAPixel ) );
	tBGRAPixel* pDst = pBuffer;

	uint8_t* pIndices = pSrc;
	uint8_t* pAlpha = pSrc + width * height;

	for ( unsigned int y = 0; y < height; y++ )
	{
		for ( unsigned int x = 0; x < width; x++ )
		{
			*pDst = pInfos[ *pIndices ];

			if ( invertAlpha )
				pDst->a = ( uint8_t )( 0xFF - *pAlpha );
			else
				pDst->a = *pAlpha;

			++pIndices;
			++pAlpha;
			++pDst;
		}
	}
	flip_vertically( ( unsigned char* )pBuffer, width, height, 4 );

	return pBuffer;
}

RGBAPix * blp1_convert_paletted_separated_alpha( uint8_t* pSrc, RGBAPix* pInfos, unsigned int width, unsigned int height, bool invertAlpha )
{
	RGBAPix * outrgba = ( RGBAPix * )blp1_convert_paletted_separated_alpha_BGRA( pSrc, ( tBGRAPixel * )pInfos, width, height, invertAlpha );

	return outrgba;
}


tBGRAPixel* blp1_convert_paletted_alpha_BGRA( uint8_t* pSrc, tBGRAPixel* pInfos, unsigned int width, unsigned int height )
{
	tBGRAPixel* pBuffer = ( tBGRAPixel* )malloc( width * height * sizeof( tBGRAPixel ) );
	tBGRAPixel* pDst = pBuffer;

	uint8_t* pIndices = pSrc;

	for ( unsigned int y = 0; y < height; ++y )
	{
		for ( unsigned int x = 0; x < width; ++x )
		{
			*pDst = pInfos[ *pIndices ];
			pDst->a = ( uint8_t )( 0xFF - pDst->a );

			++pIndices;
			++pDst;
		}
	}

	return pBuffer;
}

RGBAPix* blp1_convert_paletted_alpha( uint8_t* pSrc, RGBAPix* pInfos, unsigned int width, unsigned int height )
{
	RGBAPix * outrgba = ( RGBAPix * )blp1_convert_paletted_alpha_BGRA( pSrc, ( tBGRAPixel* )pInfos, width, height );
	return outrgba;
}

tBGRAPixel* blp1_convert_paletted_no_alpha_BGRA( uint8_t* pSrc, tBGRAPixel* pInfos, unsigned int width, unsigned int height )
{
	tBGRAPixel* pBuffer = ( tBGRAPixel * )malloc( width * height * sizeof( tBGRAPixel ) );
	tBGRAPixel* pDst = pBuffer;

	uint8_t* pIndices = pSrc;

	for ( unsigned int y = 0; y < height; ++y )
	{
		for ( unsigned int x = 0; x < width; ++x )
		{
			*pDst = pInfos[ *pIndices ];
			pDst->a = 0xFF;

			++pIndices;
			++pDst;
		}
	}
	flip_vertically( ( unsigned char* )pBuffer, width, height, 4 );

	return pBuffer;
}

RGBAPix* blp1_convert_paletted_no_alpha( uint8_t* pSrc, RGBAPix* pInfos, unsigned int width, unsigned int height )
{
	RGBAPix * outrgba = ( RGBAPix * )blp1_convert_paletted_no_alpha_BGRA( pSrc, ( tBGRAPixel* )pInfos, width, height );
	return outrgba;
}

unsigned long Blp2Raw( StormBuffer input, StormBuffer &output, int &width, int &height, int &bpp, int &mipmaps, int & alphaflag, int & compresstype, int & pictype, char const *filename )
{
	BLPHeader blph;
	bpp = 4;
	width = 0;
	height = 0;
	unsigned long curpos = 0;
	unsigned long textureSize = 0;
	if ( input.buf == NULL || input.length == NULL || input.length < sizeof( BLPHeader ) )
		return 0;


	std::memcpy( &blph, input.buf, sizeof( BLPHeader ) );

	if ( memcmp( blph.ident, "BLP1", 4 ) != 0 )
		return 0;

	mipmaps = 0;
	for ( int i = 0; i < 15; i++ )
	{
		if ( blph.poffs[ i ] > 0 )
		{
			mipmaps++;
		}
	}


	alphaflag = ( int )blph.IsAlpha;


	curpos += sizeof( BLPHeader );
	textureSize = blph.sizex * blph.sizey * 4;
	compresstype = ( int )blph.compress;

	pictype = ( int )blph.alphaEncoding;


	if ( blph.compress == 1 )
	{

		if ( input.length < curpos + 256 * 4 )
		{
			return 0;
		}

		RGBAPix Pal[ 256 ];
		std::memcpy( Pal, input.buf + curpos, 256 * 4 );
		curpos += 256 * 4;

		int offset = ( int )blph.poffs[ 0 ];
		int size = ( int )blph.psize[ 0 ];

		if ( alphaflag > 0 && ( blph.alphaEncoding == 4 || blph.alphaEncoding == 3 ) )
		{

			if ( input.length < curpos + blph.sizex * blph.sizey * 2 )
				return 0;


			uint8_t* tdata = ( uint8_t * )malloc( ( unsigned int )size );
			std::memcpy( tdata, input.buf + offset, ( size_t )size );

			RGBAPix *pic = blp1_convert_paletted_separated_alpha( ( uint8_t* )tdata, Pal, blph.sizex, blph.sizey, 0 );

			free( tdata );


			output.length = textureSize;
			output.buf = ( char * )malloc( textureSize );
			std::memcpy( output.buf, pic, textureSize );
			free( pic );

			width = ( int )blph.sizex;
			height = ( int )blph.sizey;


			return textureSize;
		}
		else if ( alphaflag > 0 && blph.alphaEncoding == 5 )
		{

			if ( input.length < curpos + blph.sizex*blph.sizey )
				return 0;



			uint8_t* tdata = ( uint8_t * )malloc( ( unsigned int )size );
			std::memcpy( tdata, input.buf + offset, ( size_t )size );
			RGBAPix *pic = blp1_convert_paletted_alpha( ( uint8_t* )tdata, Pal, blph.sizex, blph.sizey );

			free( tdata );
			output.length = textureSize;
			output.buf = ( char * )malloc( textureSize );
			std::memcpy( output.buf, pic, textureSize );
			free( pic );
			width = ( int )blph.sizex;
			height = ( int )blph.sizey;

			return textureSize;
		}
		else
		{

			if ( input.length < curpos + blph.sizex * blph.sizey )
				return 0;


			uint8_t* tdata = ( uint8_t * )malloc( ( unsigned int )size );
			std::memcpy( tdata, input.buf + offset, ( size_t )size );
			RGBAPix *pic = blp1_convert_paletted_no_alpha( ( uint8_t* )tdata, Pal, blph.sizex, blph.sizey );

			free( tdata );
			output.length = textureSize;
			output.buf = ( char * )malloc( textureSize );
			std::memcpy( output.buf, pic, textureSize );
			free( pic );
			width = ( int )blph.sizex;
			height = ( int )blph.sizey;


			return textureSize;
		}

	}
	// JPEG compressed
	else if ( blph.compress == 0 )
	{
		unsigned long JPEGHeaderSize;
		std::memcpy(&JPEGHeaderSize, input.buf + curpos, 4);
		JPEGHeaderSize = _blp_swap_int32(JPEGHeaderSize);

		StormBuffer tempdata;
		tempdata.length = blph.psize[0] + JPEGHeaderSize;
		tempdata.buf = (char*)malloc(blph.psize[0] + JPEGHeaderSize);
		std::memcpy(tempdata.buf, input.buf + curpos + 4, (size_t)JPEGHeaderSize);


		width = (int)blph.sizex;
		height = (int)blph.sizey;

		curpos = blph.poffs[0];
		std::memcpy((tempdata.buf + JPEGHeaderSize), input.buf + curpos, blph.psize[0]);

		//FILE* f;
		//fopen_s(&f, "test66.jpg", "wb");
		//if (f)
		//{
		//	fwrite(tempdata.buf, 1, tempdata.length, f);
		//	fclose(f);
		//}

		StormBuffer tmpout;
		if ( !JPG2Raw( tempdata, tmpout, width, height, bpp, filename ) )
		{
			tmpout.Clear( );
			tempdata.Clear( );
			width = 0;
			height = 0;
			return  0;
		}

		RGBAPix* tmp2 = (RGBAPix*)tmpout.buf;
		for (int x = 0; x < width * height; x++)
		{
			tmp2[x].ToBRGAPix();
		}
		tempdata.Clear( );
		// Output should be RGBA, BLPs use BGRA
		//textureInvertRBInPlace( ( RGBAPix* )output.buf, output.length );
		output = tmpout;

		width = ( int )blph.sizex;
		height = ( int )blph.sizey;

		return textureSize;
	}

	return 0;
}

bool JPG2Raw( StormBuffer input, StormBuffer & output, int & width, int & height, int &bpp, char const *filename )
{
	stbi__result_info ri;
	stbi__context s;
	stbi__start_mem(&s, (const stbi_uc* )input.buf, input.length);
	output.buf = (char*)stbi__jpeg_load(&s, &width, &height, &bpp, 4, &ri);
	output.length = width * height * 4;
	bpp = 4;
	return output.buf != NULL;
}



int ArrayXYtoId( int width, int x, int y )
{
	return  width * y + x;
}