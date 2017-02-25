//
//  xmem.h
//
//  Created by yucong on 17/2/16.
//  Copyright (c) 2017年 yucong. All rights reserved.
//

#ifndef __XDoc__xmem__
#define __XDoc__xmem__

#include "xpro.h"
#include <stdlib.h>
#include <string.h>

#define realloc_(t, b, n) ((t*)(xMem_realloc(b, n)))

XPRO_API void* xMem_realloc(void* block, size_t size);
XPRO_API void xMem_free(void* block);

#endif /* defined(__XDoc__xmem__) */
