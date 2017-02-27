//
//  xparser.c
//
//  Created by yucong on 17/2/14.
//  Copyright (c) 2017年 yucong. All rights reserved.
//

#include "xparser.h"
#include "xlex.h"
#include "xlimits.h"
#include "xobject.h"
#include "xmem.h"
#include <string.h>

#define copystr(s, str) \
    if ((s) == NULL) {s = realloc_(char, NULL, strlen(str) + 1);} \
    else if (strlen(s) < strlen(str)) {s = realloc_(char, s, strlen(str) + 1);} \
    memcpy(s, str, strlen(str) + 1); \

static void expr(lexState* ls);


static void check(lexState* ls, int c) {
    if (ls->t.token != c) {
        error_msg("Missing \"%c\"\n", c);
    }
}

static void check_next(lexState* ls, int c) {
    parser_next(ls);
    if (ls->t.token != c) {
        error_msg("Missing \"%c\"\n", c);
    }
}

static void statnull(lexState* ls) {
    long b=xprotime();
    XJson* value = create_null();
    if (!ls->json) ls->json = ls->curbase = value;
    else addItem(ls->curbase, value);
    parser_next(ls);
    long e = xprotime()-b;
    if (e > 0) printf("%s %ld\n", __FUNCTION__, e);
}

static void statboolean(lexState* ls) {
    XJson* value = create_bool(strcmp(ls->t.sem.s.str, "true") == 0);
    if (!ls->json) ls->json = ls->curbase = value;
    else addItem(ls->curbase, value);
    parser_next(ls);
}

static void statstring(lexState* ls) {
    XJson* value = create_string(ls->t.sem.s.str);
    if (!ls->json) ls->json = ls->curbase = value;
    else addItem(ls->curbase, value);
    parser_next(ls);
}

static void statnumeral(lexState* ls) {
    XJson* value = create_numeral(ls->t.sem.n);
    if (!ls->json) ls->json = ls->curbase = value;
    else addItem(ls->curbase, value);
    parser_next(ls);
}

static void statarray(lexState* ls) {
    XJson* parent = ls->curbase;
    XJson* value = create_array();
    if (!ls->json) ls->json = ls->curbase = parent = value;
    else addItem(ls->curbase, value);
    ls->curbase = value;
    do {
        parser_next(ls);
        expr(ls);
    } while (ls->t.token == ',');
    
    ls->curbase = parent;
    check(ls, ']');
    parser_next(ls);
}

static void statobject(lexState* ls) {
    XJson* parent = ls->curbase;
    XJson* value = create_object();
    if (!ls->json) ls->json = ls->curbase = parent = value;
    else addItem(ls->curbase, value);
    ls->curbase = value;
    do {
        parser_next(ls);
        if (ls->t.token == '}')  // empty object
            break;
        xpro_assert(ls->t.token == K_STRING);  // key can only be 'string'
        const char* key = NULL;  // save key
        copystr(key, ls->t.sem.s.str);
        check_next(ls, ':');
        parser_next(ls);  // skip ':'
        
        expr(ls);
        XJson* v = ls->curbase->child;
        v->key = key;
    } while (ls->t.token == ',');
    
    ls->curbase = parent;
    check(ls, '}');
    parser_next(ls);
}

static void statement(lexState* ls) {
    switch (ls->t.token) {
        case '[': statarray(ls); break;
        case '{': statobject(ls); break;
        case K_NULL: statnull(ls); break;
        case K_TRUE: case K_FALSE: statboolean(ls); break;
        case K_STRING: statstring(ls); break;
        case K_NUMERAL: statnumeral(ls); break;
        default: error_msg("Unexpected expression \"%s\"\n", ls->t.sem.s.str);
    }
}

static void expr(lexState* ls) {
    switch (ls->t.token) {
        case '}': case ']': break;
        default: statement(ls); break;
    }
}

XJson* main_parser(const char* jsonstr) {
    lexState ls;
    xLex_init(&ls, jsonstr);
    ls.current = getc(&ls);
    
    parser_next(&ls);
    while (ls.t.token != K_EOF) {
        statement(&ls);
    }
    XJson* json = ls.json;
    xLex_free(&ls);
    
    return json;
}