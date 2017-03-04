#ifndef __XDoc__xlex__
#define __XDoc__xlex__

#include "xproJson.h"

#define getc(ls) (((ls)->srclen--)>0 ? *((ls)->source++) : EOF)
#define FIRST_RESERVED 257
enum SYS_RESERVED {
    LEX_FIRST = FIRST_RESERVED, LEX_NULL, LEX_TRUE, LEX_FALSE, LEX_STRING, LEX_DOUBLE, LEX_INTEGER, LEX_NAME, LEX_EOF
};

typedef struct SemInfo {
    struct { int len; char* str; } s;
    xpro_Number n;
    xpro_Integer i;
    xpro_Boolean b;
} SemInfo;

typedef struct Token {
    int token;
    SemInfo sem;
} Token;

typedef struct lexState {
    int current;  /* current position of parser */
    int level;
    int n;  /* buff used size */
    int buffsize;
    unsigned long srclen;  /* source length */
    Token t;
    XJson* json;
    XJson* curbase;
    const char* source;  /* origin input */
    char* buff;
} lexState;

XPRO_API void xLex_init(lexState* ls, const char* source);
XPRO_API void xLex_free(lexState* ls);
XPRO_API void parser_next(lexState* ls);


#endif /* defined(__XDoc__xlex__) */
