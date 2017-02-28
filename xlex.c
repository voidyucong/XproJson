#include "xlex.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xmem.h"
#include "xlimits.h"
#include "xobject.h"

#define TOTAL_LIMITS 3
const char* const limits[TOTAL_LIMITS] = {
    "null", "true", "false"
};

#define reset_buff(ls)  \
 (ls)->n = 0; \
 memset((ls)->buff, 0, (ls)->buffsize); \
/* if ((ls)->t.sem.s.str && (ls)->t.sem.s.len > 0) { \ */
/*     xMem_free((void*)(ls)->t.sem.s.str); \ */
/*     (ls)->t.sem.s.str = NULL; \ */
/* } \ */
/*(ls)->t.sem.s.len = 0; \ */

#define next(ls) ((ls)->current = getc(ls))
#define save_and_next(ls, c) (save((ls), c), next(ls))
#define settoken(ls, c) ((ls)->t.token = (c))
#define isdigit(c) ((c)>='0'&&(c)<='9')
#define isalphaA2Z(c) ((c)>='A'&&(c)<='Z')
#define isalphaa2z(c) ((c)>='a'&&(c)<='z')
#define isalpha(c) (isalphaA2Z(c)||isalphaa2z(c))

static void save(lexState* ls, int c) {
    if (ls->n + 1 >= ls->buffsize) {
        ls->buffsize *= 1.5f;
        ls->buff = realloc_(char, ls->buff, ls->buffsize);
    }
    ls->buff[ls->n++] = (char)c;
}

static void read_hex4(lexState* ls, unsigned* ucode) {
    for (int i = 0; i < 4; ++i) {
        if (isdigit(ls->current)) { *ucode<<=4; *ucode |= (ls->current - '0'); }
        else if (ls->current >= 'A' && ls->current <= 'F') { *ucode<<=4; *ucode |= (ls->current - 'A') + 10; }
        else if (ls->current >= 'a' && ls->current <= 'f') { *ucode<<=4; *ucode |= (ls->current - 'a') + 10; }
        else { *ucode = 0; return; }
        next(ls);
    }
}

static int handle_surrogate(lexState* ls) {
    unsigned ucode = 0;
    read_hex4(ls, &ucode);
    if (ucode == 0) return 0;
    if (ucode >= 0xDC00 && ucode <= 0xDFFF) return 0;
    if (ucode >= 0xD800 && ucode <= 0xDBFF) {  /* surrogate pair */
        if (ls->current != '\\') return 0;
        next(ls);
        if (ls->current != 'u') return 0;
        next(ls);
        unsigned ucodelow = 0;
        read_hex4(ls, &ucodelow);
        if (ucodelow < 0xDC00 || ucodelow > 0xDFFF) return 0;
        ucode = 0x10000 + (ucode - 0xD800) * 0x400 + (ucodelow - 0xDC00);
    }
    return ucode;
}

static void encodeutf8(lexState* ls, unsigned ucode) {
    if (ucode <= 0x007F) {  /* 1 byte */
        save(ls, ucode);
    }
    else if (ucode <= 0x7FF) {  /* 2 byte */
        save(ls, (ucode >> 6) | 0xC0);
        save(ls, (ucode & 0x3F) | 0x80);
    }
    else if (ucode <= 0xFFFF) {  /* 3 byte */
        save(ls, (ucode >> 12) | 0xE0);
        save(ls, ((ucode >> 6) & 0x3F) | 0x80);
        save(ls, (ucode & 0x3F) | 0x80);
    }
    else if (ucode <= 0x10FFFF) {  /* 4 byte */
        save(ls, (ucode >> 18) | 0xF0);
        save(ls, ((ucode >> 12) & 0x3F) | 0x80);
        save(ls, ((ucode >> 6) & 0x3F) | 0x80);
        save(ls, (ucode & 0x3F) | 0x80);
    }
}

static void read_string(lexState* ls, int flag) {
    next(ls);
    while (ls->current != flag) {
        switch (ls->current) {
            case '\\': {
                int c;
                next(ls);
                switch (ls->current) {
                    case 'a': c = '\a'; goto save_flag;
                    case 'b': c = '\b'; goto save_flag;
                    case 'f': c = '\f'; goto save_flag;
                    case 'n': c = '\n'; goto save_flag;
                    case 'r': c = '\r'; goto save_flag;
                    case 't': c = '\t'; goto save_flag;
                    case 'v': c = '\v'; goto save_flag;
                    case 'u': {
                        next(ls);
                        unsigned ucode = handle_surrogate(ls);
                        if (ucode == 0) goto no_save_flag;
                        encodeutf8(ls, ucode);
                        goto no_save_flag;
                    }
                    case '\n': case '\r': case '\\': case '\"': case '\'':
                        c = ls->current;
                        goto save_flag;
                    default:
                        c =ls->current;
                        goto save_flag;
                }  /* switch */
                break;
            save_flag:
                save(ls, c);
                next(ls);
                break;
            no_save_flag:
                break;
            }
            default:
                if (ls->current == EOF) error_msg("Unexpected end!");
                save_and_next(ls, ls->current);
                break;
        }  /* switch */
    }  /* while */
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
static long partime = 0;
static int allocnum = 0;
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
            
            for (int i = 0; i < TOTAL_LIMITS; ++i) {  /* reserverd string? */
                if (strcmp(limits[i], ls->buff) == 0) {
                    settoken(ls, FIRST_RESERVED + i + 1);
                    break;
                }
            }
            break;
        }
    }
    
    ensure_(char, ls->t.sem.s.str, ls->t.sem.s.len, ls->n + 1);
    if (ls->t.sem.s.len < ls->n + 1) ls->t.sem.s.len = ls->n + 1;
    memcpy(ls->t.sem.s.str, ls->buff, ls->n + 1);
}

void xLex_init(lexState* ls, const char* source) {
    ls->current = 0;
    ls->level = 0;
    ls->srclen = strlen(source);
    ls->source = source;
    ls->t.token = 0;
    ls->t.sem.s.len = XPRO_MINBUFFER;
    ls->t.sem.s.str = realloc_(char, NULL, XPRO_MINBUFFER);
    ls->n = 0;
    ls->buffsize = XPRO_MINBUFFER;
    ls->buff = realloc_(char, NULL, XPRO_MINBUFFER);
    ls->curbase = NULL;
    ls->json = NULL;
}

void xLex_free(lexState* ls) {
    if (ls->buff) {
        xMem_free((void*)ls->buff);
        ls->buff = NULL;
    }
    if (ls->t.sem.s.str) {
        xMem_free((void*)ls->t.sem.s.str);
        ls->t.sem.s.str = NULL;
    }
    ls->source = NULL;
    ls->json = NULL;
    ls->buff = NULL;
    ls->curbase = NULL;
}