#include "xobject.h"
#include <stdlib.h>
#include <stdarg.h>
#include "xmem.h"
#include "xlimits.h"

static errState es = {NULL};

XJson* create_json() {
    XJson* json = realloc_(XJson, NULL, sizeof(XJson));
    json->t = XPRO_TNULL;
    json->child = NULL;
    json->head = NULL;
    json->prev = NULL;
    json->next = NULL;
    json->key = NULL;
    json->level = 0;
    json->child_size = 0;
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
    value->v.s.len = (int32_t)strlen(str) + 1;
    value->v.s.s = realloc_(char, NULL, (unsigned long)value->v.s.len + 1);
    memcpy(value->v.s.s, str, strlen(str) + 1);
    return value;
}
inline XJson* create_numeral(xpro_Number n) {
    XJson* value = create_json();
    value->t = XPRO_TNUMBER;
    value->v.n = n;
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
    parent->child_size++;
    if (parent->child == NULL) {
        parent->child = item;
        parent->head = item;
    }
    else {
        XJson* cur = parent->child;
        parent->child = item;
        cur->next = item;
        item->prev = cur;
    }
}

void xpro_addItemToObject(XJson* parent, XJson* item, const char* key) {
    item->key = realloc_(char, NULL, strlen(key) + 1);
    strcpy(item->key, key);
    addItem(parent, item);
}


/* print */

typedef struct printState  {
    char* buff;
    int32_t n;
    int32_t size;
} printState;

static long __times = 0;
static void formatJson(printState* ps, char* str, xpro_Number n, char* key, int depth, int type) {
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
    if (type == XPRO_TNUMBER) {char nstr[32];sprintf(nstr, "%.17g", n);strncpy(ctemp, nstr, strlen(nstr));ctemp+=strlen(nstr);}
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
    formatJson(ps, str, 0, key, depth, type);
}

static void saveNumeral(printState* ps, xpro_Number n, char* key, int depth, int type) {
    formatJson(ps, NULL, n, key, depth, type);
}

static void print_value(XJson* v, printState* ps);

static void print_null(XJson* v, printState* ps) {
    saveString(ps, "null", v->key, v->level, v->t);
}

static void print_bool(XJson* v, printState* ps) {
    saveString(ps, valueboolean(v) == 1 ? "true" : "false", v->key, v->level, v->t);
}

static void print_numeral(XJson* v, printState* ps) {
    saveNumeral(ps, valuenumeral(v), v->key, v->level, v->t);
}

static void print_string(XJson* v, printState* ps) {
    saveString(ps, valuestring(v), v->key, v->level, v->t);
}

static void print_array(XJson* v, printState* ps) {
    XJson* child = v->head;
    saveString(ps, "[\n", v->key, v->level, v->t);
    while (child) {
        print_value(child, ps);
        child = child->next;
        saveString(ps, child?",\n":"\n", NULL, 0, XPRO_TNULL);
    }
    saveString(ps, "]", NULL, v->level, v->t);
}

static void print_object(XJson* v, printState* ps) {
    XJson* child = v->head;
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
        case XPRO_TNUMBER: print_numeral(v, ps); break;
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

void error_msg(const char* fmt, ...) {
    va_list argp;
    va_start(argp, fmt);
    char szBuf[MAX_LOG_LEN + 1] = {0};
    vsnprintf(szBuf, MAX_LOG_LEN, fmt, argp);
    va_end(argp);
    
//    printf("%s\n", szBuf);
    if (es.errfunc) {
        es.errfunc(szBuf);
    }
    xpro_assert(0);
}