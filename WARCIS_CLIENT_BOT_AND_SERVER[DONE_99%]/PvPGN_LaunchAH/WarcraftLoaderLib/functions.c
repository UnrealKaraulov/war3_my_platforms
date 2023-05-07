#if !defined(USE_SRP3)

#include "functions.h"
#include <Windows.h>
/*
Password hashing functions here are adapted from bnethash.cpp (part of pvpgn):
https://svn.berlios.de/wsvn/pvpgn/trunk/pvpgn/src/common/bnethash.cpp?rev=257&sc=0
*/

/*
 * Fill 16 elements of the array of 32 bit values with the bytes from
 * dst up to count in little endian order. Fill left over space with
 * zeros
 */
//
///* by Rupan, 8/23/2008 -- reserved for future use
//   this implements logon_proof_hash
//   Not sure about the argument types here.....
//*/
//DLLEXPORT signed int w3l_logon_proof_hash(char *arg1, char *arg2, char *arg3) {
//	int i;
//	char buf[5*4+64], *tmp;
//
//	tmp = buf + (5*4);
//	w3l_hash_init((uint32_t *)buf);
//	memset(tmp, 0, 64);
//	memcpy(tmp, (const void *)(arg1 + 32), 16);
//
//	for(i=0; i<16; i++) {
//		if(tmp[i] >= 65 && tmp[i] <= 90) {
//				tmp[i] |= 0x20;
//		}
//	}
//
//	w3l_do_hash(arg1, (bnet_hash_ctx *)buf);
//	memcpy((arg1+168), (const void *)buf, 20);
//
//	return 1;
//}
//
///*
// * This function wraps logon_proof_hash.  It can be found at address
// * 0x6F6B2870 in Game.dll v1.24.
// * Fairly sure about the argument counts herein.
//*/
//DLLEXPORT int __fastcall w3l_lph_checked(int *a1, int *a2, void *a3, void *a4)
//{
//  int result;
//
//  result = w3l_logon_proof_hash(a2, a3, a4);
//  if ( result )
//  {
//    *(a1 + 0) = *(a2 + 42); /* a2 + 168 bytes */
//    *(a1 + 1) = *(a2 + 43); /* a2 + 172 bytes */
//    *(a1 + 2) = *(a2 + 44); /* a2 + 176 bytes */
//    *(a1 + 3) = *(a2 + 45); /* a2 + 180 bytes */
//    *(a1 + 4) = *(a2 + 46); /* a2 + 184 bytes */
//  }
//  return result;
//}

#endif
