#include "xparser.h"
#include <string.h>
#include "xlex.h"
#include "xlimits.h"
#include "xobject.h"
#include "xmem.h"

#define changebase(ls, b) (b?(ls)->curbase=(b):0)

static void expr(lexState* ls);


static void check(lexState* ls, int c) {
    error_check(ls->t.token == c, "Missing character \"%c\"", c);
}

static void check_next(lexState* ls, int c) {
    parser_next(ls);
    check(ls, c);
}

static void checkAddItem(lexState* ls, XJson* item) {
    if (!ls->json) {
        error_check(item->t == XPRO_TARRAY || item->t == XPRO_TOBJECT, "JSON payload should be an object or array.");
        ls->json = ls->curbase = item;
    }
    else addItem(ls->curbase, item);
}

static void statnull(lexState* ls) {
    XJson* value = create_null();
    checkAddItem(ls, value);
    parser_next(ls);
}

static void statboolean(lexState* ls) {
    XJson* value = create_bool(strcmp(ls->t.sem.s.str, "true") == 0);
    checkAddItem(ls, value);
    parser_next(ls);
}

static void statstring(lexState* ls) {
    XJson* value = create_string(ls->t.sem.s.str);
    checkAddItem(ls, value);
    parser_next(ls);
}

static void statnumeral(lexState* ls) {
    XJson* value = create_double(ls->t.sem.n);
    checkAddItem(ls, value);
    parser_next(ls);
}

static void statarray(lexState* ls) {
    XJson* parent = ls->curbase;
    XJson* value = create_array();
    checkAddItem(ls, value);
    changebase(ls, value);
    do {
        parser_next(ls);
        if (ls->t.token == K_EOF)
            error_msg("Expecting 'string', 'number', 'null', 'true', 'false', '{', '['");
        if (ls->t.token == ']') {
            error_check(value->nchild == 0, "Expecting 'string', 'number', 'null', 'true', 'false', '{', '[' after ','");
            break; /* empty array */
        }
        expr(ls);
    } while (ls->t.token == ',');
    
    changebase(ls, parent);
    check(ls, ']');
    parser_next(ls);
}

static void statobject(lexState* ls) {
    XJson* parent = ls->curbase;
    XJson* value = create_object();
    checkAddItem(ls, value);
    changebase(ls, value);
    do {
        parser_next(ls);
        if (ls->t.token == K_EOF)
            error_msg("Expecting 'string' after ','");
        if (ls->t.token == '}') {
            error_check(value->nchild == 0, "Expecting 'string' after ','");
            break; /* empty object */
        }
        if (ls->t.token != K_STRING) error_msg("keys must be string.");  /* key can only be 'string' */
        
        char* key = realloc_(char, NULL, strlen(ls->t.sem.s.str) + 1);  /* save key */
        memcpy(key, ls->t.sem.s.str, strlen(ls->t.sem.s.str) + 1);
        check_next(ls, ':');
        parser_next(ls);  /* skip ':' */
        expr(ls);
        ls->curbase->top->key = key;  /* set last child's key */
    } while (ls->t.token == ',');
    
    changebase(ls, parent);
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
        default: error_check(0, "Unexpected expression \"%s\"", ls->t.sem.s.str);
    }
}

static void expr(lexState* ls) {
    switch (ls->t.token) {
//        case '}': case ']': break;
        default: statement(ls); break;
    }
}

XJson* main_parser(const char* jsonstr) {
    lexState ls;
    xLex_init(&ls, jsonstr);
    ls.current = getc(&ls);
    parser_next(&ls);
    statement(&ls);
    error_check(ls.t.token == K_EOF, "Expecting 'EOF'");
    XJson* json = ls.json;
    xLex_free(&ls);
    
    return json;
}