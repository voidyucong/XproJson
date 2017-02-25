//
//  xlex.c
//
//  Created by yucong on 17/2/16.
//  Copyright (c) 2017年 yucong. All rights reserved.
//

#include "xlex.h"
#include "xmem.h"
#include "xlimits.h"
#include "xobject.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TOTAL_LIMITS 3
const char* const limits[TOTAL_LIMITS] = {
    "null", "true", "false"
};

#define reset_buff(ls)  \
 (ls)->n = 0; \
 memset((ls)->buff, 0, (ls)->buffsize); \
 if ((ls)->t.sem.s.str && (ls)->t.sem.s.len > 0) { \
     xMem_free((void*)(ls)->t.sem.s.str); \
     (ls)->t.sem.s.str = NULL; \
 } \
(ls)->t.sem.s.len = 0; \

#define next(ls) ((ls)->current = getc(ls))
#define save_and_next(ls, c) (save((ls), c), next(ls))
#define settoken(ls, c) ((ls)->t.token = (c))
#define isdigit(c) ((c)>='0'&&(c)<='9')
#define isalpha(c) (((c)>='a'&&(c)<='z')||((c)>='A'&&(c)<='Z'))

static void save(lexState* ls, int c) {
    if (ls->n + 1 >= ls->buffsize) {
        ls->buffsize *= 1.5f;
        ls->buff = realloc_(char, ls->buff, ls->buffsize);
    }
    ls->buff[ls->n++] = (char)c;
}

static void read_string(lexState* ls, int flag) {
    next(ls);
    while (ls->current != flag) {
        
        
        switch (ls->current) {
            case '\\': {
                int c;
                save_and_next(ls, ls->current);
                switch (ls->current) {
                    case 'a': c = '\a'; goto save_flag;
                    case 'b': c = '\b'; goto save_flag;
                    case 'f': c = '\f'; goto save_flag;
                    case 'n': c = '\n'; goto save_flag;
                    case 'r': c = '\r'; goto save_flag;
                    case 't': c = '\t'; goto save_flag;
                    case 'v': c = '\v'; goto save_flag;
//                    case 'x': c = readhexaesc(ls); goto read_save;
//                    case 'u': utf8esc(ls);  goto no_save;
                    case '\n': case '\r': case '\\': case '\"': case '\'':
                        c = ls->current;
                        goto save_flag;
                    default:
                        c =ls->current;
                        goto save_flag;
                }  // switch
                break;
            save_flag:
                save(ls, c);
                next(ls);
                break;
            }
            default:
                if (ls->current == EOF) xpro_assert(0);
                save_and_next(ls, ls->current);
                break;
        }  // switch
    }  // while
    next(ls);
    
    settoken(ls, K_STRING);
}

/*
* number = [ "-" ] int [ frac ] [ exp ]
* int = "0" / digit1-9 *digit
* frac = "." 1*digit
* exp = ("e" / "E") ["-" / "+"] 1*digit
*/
static void read_number(lexState* ls) {
    do {
        save_and_next(ls, ls->current);
    } while (isdigit(ls->current) || ls->current == '.' || ls->current == 'e' || ls->current == 'E' || ls->current == '+' || ls->current == '-');
    char* endstr;
    double numeral = strtod(ls->buff, &endstr);
    xpro_assert(strcmp(endstr, "") == 0);
    ls->t.sem.n = numeral;
    settoken(ls, K_NUMERAL);
}

void parser_next(lexState* ls) {
    reset_buff(ls);
renext:
    switch (ls->current) {
        case '\n': case '\r': case ' ': case '\f': case '\t': case '\v': {  /* 换行、空格 */
            next(ls);
            goto renext;
        }
        case '"': {
            read_string(ls, ls->current);
            break;
        }
        case '[': case ']': case '{': case '}': case ',': case ':': {
            settoken(ls, ls->current);
            save_and_next(ls, ls->current);
            break;
        }
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
        case '-': {
            read_number(ls);
            return;
        }
        case EOF: {
            settoken(ls, K_EOF);
            return;
        }
        default: {
            do {
                save_and_next(ls, ls->current);
            } while (isdigit(ls->current) || isalpha(ls->current));
            settoken(ls, K_NAME);
            
            for (int i = 0; i < TOTAL_LIMITS; ++i) {  // 是否保留字符串
                if (strcmp(limits[i], ls->buff) == 0) {
                    settoken(ls, FIRST_RESERVED + i + 1);
                    break;
                }
            }
            break;
        }
    }
    ls->t.sem.s.len = ls->n + 1;
    ls->t.sem.s.str = realloc_(char, NULL, ls->n + 1);
    memcpy(ls->t.sem.s.str, ls->buff, ls->n + 1);
}

void xLex_init(lexState* ls, const char* source) {
    ls->current = 0;
    ls->level = 0;
    ls->len = strlen(source);
    ls->source = source;
    ls->t.token = 0;
    ls->t.sem.s.len = 0;
    ls->t.sem.s.str = NULL;
    ls->n = 0;
    ls->buffsize = XPRO_MINBUFFER;
    ls->buff = realloc_(char, NULL, ls->buffsize);
    ls->curbase = NULL;
    ls->json = NULL;
}

void xLex_free(lexState* ls) {
    if (ls->buff) {
        xMem_free((void*)ls->buff);
        ls->buff = NULL;
    }
    if (ls->t.sem.s.str && ls->t.sem.s.len > 0) {
        xMem_free((void*)ls->t.sem.s.str);
        ls->t.sem.s.str = NULL;
    }
    ls->source = NULL;
    ls->json = NULL;
    ls->buff = NULL;
    ls->curbase = NULL;
}