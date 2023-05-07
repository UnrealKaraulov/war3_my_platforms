#include <stdio.h>
#pragma comment (lib,"War3.lib")

extern unsigned long __stdcall timeGetTime();


int main()
{
	return timeGetTime();
}