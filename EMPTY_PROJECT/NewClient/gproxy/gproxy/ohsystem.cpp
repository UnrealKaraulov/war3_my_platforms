#include "ohsystem.h"
#include "dirent.h"
#include "gproxy.h"
#include "bnet.h"
#include "bnetprotocol.h"

#include <string>         // std::string
#include <iostream>       // std::cout
#include <signal.h>
#include <stdlib.h>
#ifdef WIN32
#include <windows.h>
#include <winsock.h>
#endif
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <istream>
//#include <thread>  
#include <wininet.h>
#include "gdiplus.h"

#include <time.h>

#ifndef WIN32
 #include <sys/time.h>
#endif

#ifdef __APPLE__
 #include <mach/mach_time.h>
#endif

using namespace std;

std::wstring ToStringW( const std::string& strText )
{
  std::wstring      wstrResult;

  wstrResult.resize( strText.length() );

  typedef std::codecvt<wchar_t, char, mbstate_t> widecvt;

  std::locale     locGlob;

  std::locale::global( locGlob );

  const widecvt& cvt( std::use_facet<widecvt>( locGlob ) );

  mbstate_t   State;

  const char*       cTemp;
  wchar_t*    wTemp;

  cvt.in( State,
          &strText[0], &strText[0] + strText.length(), cTemp,
          (wchar_t*)&wstrResult[0], &wstrResult[0] + wstrResult.length(), wTemp );
                
  return wstrResult;
}

string LongToString(long i)
{
    long l = i;
    string x;
    stringstream y;
    y << l;
    x = y.str();
    return x;
}


string GetFileSize( string input )
{
	FILE * pFile;
	long size = 0;
	pFile = fopen (input.c_str(), "rb");
	if (pFile==NULL)
    perror ("Error opening file.");
	else
	{
		fseek (pFile, 0, SEEK_END);
		size=ftell (pFile);
		fclose (pFile);
	}
	return LongToString(size);
}

