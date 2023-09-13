#if !defined(_FUNCTIONS_H)
#define _FUNCTIONS_H


#if !defined(USE_SRP3)

#include <stdint.h>

typedef struct {
	uint32_t a, b, c, d, e;
} bnet_hash_ctx;

#define ROTL(x,n,w) (((x)<<((n)&(w-1))) | ((x)>>(((-(n))&(w-1)))))
#define ROTL32(x,n) ROTL((x),(n),32)
#define ROTL16(x,n) ROTL((x),(n),16)
#define DLLEXPORT __declspec(dllexport)


extern  void hash_set_16( uint32_t * dst, unsigned char const * src, unsigned int count );/*
extern DLLEXPORT void w3l_do_hash( char *username, bnet_hash_ctx *ctx );
extern DLLEXPORT void __fastcall w3l_hash_init( uint32_t *data );*/



//DLLEXPORT void w3l_do_hash(uint32_t hash[5], int32_t *tmp);

#endif
#endif /* _FUNCTIONS_H */
