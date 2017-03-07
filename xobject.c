#include "xobject.h"
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <float.h>
#include "xmem.h"
#include "xlimits.h"

static errState es = {NULL};

XJson* create_json() {
    XJson* json = realloc_(XJson, NULL, sizeof(XJson));
    json->t = XPRO_TNULL;
    json->top = NULL;
    json->stack = NULL;
    json->prev = NULL;
    json->next = NULL;
    json->key = NULL;
    json->level = 0;
    json->nchild = 0;
    json->v.s.len = 0;
    json->v.s.s = NULL;
    return json;
}

inline XJson* create_null() {
    XJson* value = create_json();
    value->t = XPRO_TNULL;
    return value;
}
inline XJson* create_bool(int b) {
    XJson* value = create_json();
    value->t = XPRO_TBOOLEAN;
    value->v.b = b;
    return value;
}
inline XJson* create_string(const char* str) {
    XJson* value = create_json();
    value->t = XPRO_TSTRING;
    value->v.s.len = (int)strlen(str) + 1;
    value->v.s.s = realloc_(char, NULL, (unsigned long)value->v.s.len + 1);
    memcpy(value->v.s.s, str, strlen(str) + 1);
    return value;
}
inline XJson* create_double(xpro_Number n) {
    XJson* value = create_json();
    value->t = XPRO_TDOUBLE;
    value->v.n = n;
    return value;
}

inline XJson* create_integer(xpro_Integer i) {
    XJson* value = create_json();
    value->t = XPRO_TINTEGER;
    value->v.i = i;
    return value;
}

inline XJson* create_array() {
    XJson* value = create_json();
    value->t = XPRO_TARRAY;
    return value;
}
inline XJson* create_object() {
    XJson* value = create_json();
    value->t = XPRO_TOBJECT;
    return value;
}

void addItem(XJson* parent, XJson* item) {
    xpro_assert(parent->t == XPRO_TARRAY || parent->t == XPRO_TOBJECT);
    
    item->level = parent->level + 1;
    parent->nchild++;
    if (parent->stack == NULL) {
        parent->top = item;
        parent->stack = item;
    }
    else {
        XJson* cur = parent->top;
        parent->top = item;
        cur->next = item;
        item->prev = cur;
    }
}


/* print */

typedef struct printState  {
    char* buff;
    int n;
    int size;
} printState;

static void saves(printState* ps, char* str) {
    size_t len = strlen(str);
    size_t origlen = ps->n;
    if (ps->n + len >= ps->size) {
        ps->size = (ps->n + len) * 1.5;
        ps->buff = realloc_(char, ps->buff, ps->size);
    }
    ps->n += len;
    ps->buff += origlen;
    memcpy(ps->buff, str, len);
    ps->buff -= origlen;
    ps->buff[ps->n] = '\0';
}

static void savec(printState* ps, int c) {
    if (ps->n + 1 >= ps->size) {
        ps->size *= 1.5f;  /* resize */
        ps->buff = realloc_(char, ps->buff, ps->size);
    }
    ps->buff[ps->n++] = (char)c;
    ps->buff[ps->n] = '\0';
}

static long __times = 0;

static void print_retract(printState* ps, int depth) {
    /* space */
    if (depth <= 0) return;
    depth <<= 1;
    char* space = realloc_(char, NULL, depth + 1);
    memset(space, ' ', depth);
    space[depth] = '\0';
    saves(ps, space);
    xMem_free(space);
}

static void print_str(printState* ps, char* str) {
    if (!str)
    {
        saves(ps, "\"\"");
        return;
    }
    char* pstr = str;
    saves(ps, "\"");
    while (*pstr) {
        if (*pstr > 31 && *pstr != '\"' && *pstr != '\\') savec(ps, *pstr++);
        else {
            switch (*pstr) {
                case '\\': saves(ps, "\\\\"); break;
                case '\"': saves(ps, "\\\""); break;
                case '\a': saves(ps, "\\a"); break;
                case '\b': saves(ps, "\\b"); break;
                case '\f': saves(ps, "\\f"); break;
                case '\n': saves(ps, "\\n"); break;
                case '\r': saves(ps, "\\r"); break;
                case '\t': saves(ps, "\\t"); break;
                case '\v': saves(ps, "\\v"); break;
                default: { char arr[5]; sprintf(arr,"u%04x", *pstr); saves(ps, arr); break; }
            }
            pstr++;
        }
    }
    saves(ps, "\"");
}

static void print_key(printState* ps, char* key) {
    if (!key) return;
    print_str(ps, key);
    savec(ps, ':');
}

static void print_value(XJson* v, printState* ps);

static void print_null(XJson* v, printState* ps) {
    print_retract(ps, v->level);
    print_key(ps, v->key);
    saves(ps, "null");
}

static void print_bool(XJson* v, printState* ps) {
    print_retract(ps, v->level);
    print_key(ps, v->key);
    saves(ps, valueboolean(v) == 1 ? "true" : "false");
}

static void print_double(XJson* v, printState* ps) {
    print_retract(ps, v->level);
    print_key(ps, v->key);
    
    xpro_Number n = valuedouble(v);
    char nstr[64];
    if (fabs(floor(n)-n) <= DBL_EPSILON && fabs(n) < 1.0e60)    sprintf(nstr,"%.0f",n);
    else if (fabs(n) < 1.0e-6 || fabs(n) > 1.0e9)               sprintf(nstr,"%e",n);
    else                                                    sprintf(nstr,"%f",n);
    saves(ps, nstr);
}

static void print_integer(XJson* v, printState* ps) {
    print_retract(ps, v->level);
    print_key(ps, v->key);
    
    xpro_Integer i = valueinteger(v);
    char nstr[21];
    sprintf(nstr, "%lld", i);
    saves(ps, nstr);
}

static void print_string(XJson* v, printState* ps) {
    print_retract(ps, v->level);
    print_key(ps, v->key);
    print_str(ps, valuestring(v));
}

static void print_array(XJson* v, printState* ps) {
    XJson* child = v->stack;
    print_retract(ps, v->level);
    print_key(ps, v->key);
    saves(ps, "[\n");
    while (child) {
        print_value(child, ps);
        child = child->next;
        saves(ps, child?",\n":"\n");
    }
    print_retract(ps, v->level);
    saves(ps, "]");
}

static void print_object(XJson* v, printState* ps) {
    XJson* child = v->stack;
    print_retract(ps, v->level);
    print_key(ps, v->key);
    saves(ps, "{\n");
    while (child) {
        print_value(child, ps);
        child = child->next;
        saves(ps, child?",\n":"\n");
    }
    print_retract(ps, v->level);
    saves(ps, "}");
}

static void print_value(XJson* v, printState* ps) {
    if (!v) return;
    switch (v->t) {
        case XPRO_TBOOLEAN: print_bool(v, ps); break;
        case XPRO_TSTRING: print_string(v, ps); break;
        case XPRO_TNULL: print_null(v, ps); break;
        case XPRO_TDOUBLE: print_double(v, ps); break;
        case XPRO_TINTEGER: print_integer(v, ps); break;
        case XPRO_TARRAY: print_array(v, ps); break;
        case XPRO_TOBJECT: print_object(v, ps); break;
        default: break;
    }
}

char* print_json(XJson* json) {
    printState ps;
    ps.n = 0;
    ps.size = 32;
    ps.buff = realloc_(char, NULL, ps.size);
    memset(ps.buff, 0, ps.size);
    print_value(json, &ps);
/*    printf("use %ldms\n", __times); */
    return ps.buff;
}

void setErrFunc(ERR_FUNC func) {
    es.errfunc = func;
}

void error_msg(const char* msg) {
    if (es.errfunc) es.errfunc(msg);
    xpro_assert(0);
}

void error_check(int cond, const char* fmt, ...) {
    if (!cond) {
        va_list argp;
        va_start(argp, fmt);
        char szBuf[MAX_LOG_LEN + 1] = {0};
        vsnprintf(szBuf, MAX_LOG_LEN, fmt, argp);
        va_end(argp);
        
        error_msg(szBuf);
    }
}