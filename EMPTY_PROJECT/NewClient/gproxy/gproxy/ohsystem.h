// standard integer sizes for 64 bit compatibility

//#ifdef WIN32
// #include "ms_stdint.h"
//#else
// #include <stdint.h>
//#endif

// STL
#include <windows.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <vector>

using namespace std;

typedef vector<unsigned char> BYTEARRAY;

string LongToString(long value);
string GetFileSize( string input );
