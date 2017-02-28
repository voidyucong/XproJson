//
//  xmem.c
//
//  Created by yucong on 17/2/16.
//  Copyright (c) 2017年 yucong. All rights reserved.
//

#include "xmem.h"
#include "xlimits.h"
#include "xobject.h"

void* xMem_realloc(void* block, size_t size) {
    void* newblock = NULL;
    if (block == NULL) {
        newblock = malloc(size);
    }else {
        newblock = realloc(block, size);
    }
    if (newblock == NULL) {
        error_msg("Memory alloc failed!");
    }
    return newblock;
}

void xMem_free(void* block) {
    if (block) {
//        printf("free memory %x\n", block);
        free(block);
        block = NULL;
    }
}

void* xMem_ensure(void* block, size_t oz, size_t nz) {
    if (oz < nz) {
        block = xMem_realloc(block, nz);
    }
    return block;
}