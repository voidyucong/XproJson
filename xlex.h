//
//  xlex.h
//
//  Created by yucong on 17/2/16.
//  Copyright (c) 2017年 yucong. All rights reserved.
//

#ifndef __XDoc__xlex__
#define __XDoc__xlex__

#include "xpro.h"

#define getc(ls) (((ls)->srclen--)>0 ? *((ls)->source++) : EOF)
#define FIRST_RESERVED 257
enum SYS_RESERVED {
    K_FIRST = FIRST_RESERVED, K_NULL, K_TRUE, K_FALSE, K_STRING, K_NUMERAL, K_NAME, K_EOF
};

typedef struct SemInfo {
    struct { int64_t len; char* str; } s;
    xpro_Number n;
    xpro_Boolean b;
} SemInfo;

typedef struct Token {
    int32_t token;
    SemInfo sem;
} Token;

typedef struct lexState {
    int32_t current;  // current position of parser
    int32_t level;
    int32_t n;  // buff used size
    int32_t buffsize;
    u_long srclen;  // source length
    Token t;
    XJson* json;
    XJson* curbase;
    const char* source;  // origin input
    char* buff;
} lexState;

XPRO_API void xLex_init(lexState* ls, const char* source);
XPRO_API void xLex_free(lexState* ls);
XPRO_API void parser_next(lexState* ls);

static long partime;

#endif /* defined(__XDoc__xlex__) */
