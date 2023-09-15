#include "Antihack.h"
#include "gproxy.h"
#include "crc32.h"
#include <Windows.h>
unsigned int Antihack_magic_value = 0xFFFFFFFF;



int ScanResult( )
{
	return ANTIHACK_VERSION;
}


unsigned int __stdcall GetFileChecksum( const char * filepath)
{
	std::ifstream file(filepath, std::ios::binary | std::ios::ate);
	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<char> buffer(size);
	if (file.read(buffer.data(), size))
	{
		unsigned int retval = crc32_16bytes_prefetch(&buffer[0], size);
		return retval;
	}

	return 0;
}