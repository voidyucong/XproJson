//
//  xlex.h
//
//  Created by yucong on 17/2/16.
//  Copyright (c) 2017年 yucong. All rights reserved.
//

#ifndef __XDoc__xlex__
#define __XDoc__xlex__

#include "xpro.h"

#define getc(ls) (((ls)->len--)>0 ? *((ls)->source++) : EOF)
#define FIRST_RESERVED 257
enum SYS_RESERVED {
    K_FIRST = FIRST_RESERVED, K_NULL, K_TRUE, K_FALSE, K_STRING, K_NUMERAL, K_NAME, K_EOF
};

typedef union SemInfo {
    struct {
        char* str;
        int32_t len;
    } s;
    xpro_Number n;
    xpro_Boolean b;
} SemInfo;

typedef struct Token {
    int32_t token;
    SemInfo sem;
} Token;

typedef struct lexState {
    int32_t current;  // 当前解析string的位置
    int32_t level;  // 当前层级深度
    int32_t n;  // str的已使用大小
    int32_t buffsize;  // str的长度
    u_long len;  // 当前string的长度
    Token t;
    XJson* json;
    XJson* curbase;
    const char* source;  // 原始输入
    char* buff;
} lexState;

XPRO_API void xLex_init(lexState* ls, const char* source);
XPRO_API void xLex_free(lexState* ls);
XPRO_API void parser_next(lexState* ls);

static long partime;

#endif /* defined(__XDoc__xlex__) */
