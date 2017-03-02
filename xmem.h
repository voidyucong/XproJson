#ifndef __XDoc__xmem__
#define __XDoc__xmem__

#include <stdlib.h>
#include <string.h>
#include "xproJson.h"

#define realloc_(t, b, n) ((t*)(xMem_realloc(b, n)))
#define ensure_(t, b, o, n) (b = (t*)xMem_ensure(b, o, n))

XPRO_API void* xMem_realloc(void* block, size_t size);
XPRO_API void xMem_free(void* block);
XPRO_API void* xMem_ensure(void* block, size_t oz, size_t nz);

#endif /* defined(__XDoc__xmem__) */
