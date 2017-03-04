#include "xobject.h"
#include <stdlib.h>
#include <stdarg.h>
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

static long __times = 0;
static void formatJson(printState* ps, char* str, xpro_Number n, xpro_Integer i, char* key, int depth, int type) {
    /* space */
    char* space = NULL;
    if (depth > 0) {
        depth <<= 1;
        space = realloc_(char, NULL, depth + 1);
        memset(space, ' ', depth);
    }
    /* mark */
    const char* mark = type == XPRO_TSTRING ? "\"" : NULL;
    /* key */
    const char* addkey = key ? key : NULL;
    
    size_t totalLen = 0;
    totalLen += space?strlen(space):0;
    totalLen += addkey?strlen(addkey)+3:0;
    totalLen += mark?strlen(mark)*2:0;
    totalLen += str?strlen(str):32;
    
    if (ps->n + totalLen >= ps->size) {
        ps->size *= 1.5;
        if (ps->size < ps->n + totalLen)
            ps->size = (ps->n + totalLen) * 1.5;
        ps->buff = realloc_(char, ps->buff, ps->size);
    }
    ps->buff[ps->n] = '\0';
    
    char* obuff = ps->buff;
    ps->buff += ps->n;  /* to the tail */
    
    char temp[totalLen];
    char* ctemp = temp;
    if (space) {strncpy(ctemp, space, strlen(space));ctemp+=depth;}
    if (addkey) {sprintf(ctemp, "\"%s\":", addkey);ctemp+=strlen(addkey)+3;}
    if (mark) {strncpy(ctemp, mark, strlen(mark));ctemp+=strlen(mark);}
    if (str) {strncpy(ctemp, str, strlen(str));ctemp+=strlen(str);}
    if (type == XPRO_TDOUBLE) {char nstr[32];sprintf(nstr, "%.17g", n);strncpy(ctemp, nstr, strlen(nstr));ctemp+=strlen(nstr);}
    if (type == XPRO_TINTEGER) {char nstr[32];sprintf(nstr, "%d", i);strncpy(ctemp, nstr, strlen(nstr));ctemp+=strlen(nstr);}
    if (mark) {strncpy(ctemp, mark, strlen(mark));ctemp+=strlen(mark);}
    *ctemp = '\0';
    ctemp = temp;  /* back to head */
    
    ps->n += strlen(ctemp);
    memcpy(ps->buff, temp, strlen(ctemp));
    
    ps->buff = obuff;  /* back to head */
    ps->buff[ps->n] = '\0';
    if (depth > 0) free(space); space = NULL;
}

static void saveString(printState* ps, char* str, char* key, int depth, int type) {
    formatJson(ps, str, 0, 0, key, depth, type);
}

static void saveDouble(printState* ps, xpro_Number n, char* key, int depth, int type) {
    formatJson(ps, NULL, n, 0, key, depth, type);
}

static void saveInteger(printState* ps, xpro_Integer i, char* key, int depth, int type) {
    formatJson(ps, NULL, 0, i, key, depth, type);
}

static void print_value(XJson* v, printState* ps);

static void print_null(XJson* v, printState* ps) {
    saveString(ps, "null", v->key, v->level, v->t);
}

static void print_bool(XJson* v, printState* ps) {
    saveString(ps, valueboolean(v) == 1 ? "true" : "false", v->key, v->level, v->t);
}

static void print_double(XJson* v, printState* ps) {
    saveDouble(ps, valuedouble(v), v->key, v->level, v->t);
}

static void print_integer(XJson* v, printState* ps) {
    saveInteger(ps, valueinteger(v), v->key, v->level, v->t);
}

static void print_string(XJson* v, printState* ps) {
    saveString(ps, valuestring(v), v->key, v->level, v->t);
}

static void print_array(XJson* v, printState* ps) {
    XJson* child = v->stack;
    saveString(ps, "[\n", v->key, v->level, v->t);
    while (child) {
        print_value(child, ps);
        child = child->next;
        saveString(ps, child?",\n":"\n", NULL, 0, XPRO_TNULL);
    }
    saveString(ps, "]", NULL, v->level, v->t);
}

static void print_object(XJson* v, printState* ps) {
    XJson* child = v->stack;
    saveString(ps, "{\n", v->key, v->level, v->t);
    while (child) {
        print_value(child, ps);
        child = child->next;
        saveString(ps, child?",\n":"\n", NULL, 0, XPRO_TNULL);
    }
    saveString(ps, "}", NULL, v->level, v->t);
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