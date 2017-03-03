#ifndef __XDoc__xpro__
#define __XDoc__xpro__
#include "xconf.h"

/*
** basic types
*/
#define XPRO_TNULL          0
#define XPRO_TBOOLEAN       1
#define XPRO_TNUMBER		2
#define XPRO_TINTEGER       3  /* TODO */
#define XPRO_TSTRING		4
#define XPRO_TARRAY         5
#define XPRO_TOBJECT		6

#define XPRO_NUMTAGS		7

typedef double      xpro_Number;
typedef long long   xpro_Integer;  /* TODO */
typedef char        xpro_Boolean;

typedef void (*ERR_FUNC)(const char*);

typedef union Value_ {
    struct {char* s; int len;} s;
    xpro_Boolean b;
    xpro_Number n;
//    xpro_Integer i;
} Value_;

typedef struct XJson {
    int t;
    int level;
    int nchild;
    union Value_ v;
    struct XJson* next, *prev;
    struct XJson* top;
    struct XJson* stack;
    char* key;
} XJson;


XPRO_API XJson* xpro_parser(const char* jsonstr);
XPRO_API XJson* xpro_parserFile(const char* fileName);
XPRO_API void xpro_free(XJson* json);

XPRO_API char* xpro_print(XJson* json);

XPRO_API int xpro_getArraySize(XJson* array);
XPRO_API int xpro_getObjectSize(XJson* object);

XPRO_API XJson* xpro_create_null();
XPRO_API XJson* xpro_create_bool(int b);
XPRO_API XJson* xpro_create_int(xpro_Integer i);  /* TODO */
XPRO_API XJson* xpro_create_double(xpro_Number n);
XPRO_API XJson* xpro_create_string(const char* str);
XPRO_API XJson* xpro_create_array();
XPRO_API XJson* xpro_create_object();

XPRO_API void xpro_addItemToArray(XJson* parent, XJson* item);
XPRO_API void xpro_addItemToObject(XJson* parent, XJson* item, const char* key);

XPRO_API XJson* xpro_detachItemInArray(XJson* array, int index);
XPRO_API XJson* xpro_detachItemInOjbect(XJson* object, const char* key);

XPRO_API void xpro_deleteItemInArray(XJson* array, int index);
XPRO_API void xpro_deleteItemInOjbect(XJson* object, const char* key);

XPRO_API XJson* xpro_getItemInArray(XJson* array, int index);
XPRO_API XJson* xpro_getItemInObject(XJson* object, const char* key);

XPRO_API void xpro_minify(char* json);
XPRO_API void xpro_dump(const char* fileName, const char* json);

XPRO_API void xpro_setErrFunc(ERR_FUNC func);

long xprotime();

#define xpro_addNullToArray(parent) (xpro_addItemToArray((parent), xpro_create_null()))
#define xpro_addTrueToArray(parent) (xpro_addItemToArray((parent), xpro_create_bool(1)))
#define xpro_addFalseToArray(parent) (xpro_addItemToArray((parent), xpro_create_bool(0)))
#define xpro_addDoubleToArray(parent, n) (xpro_addItemToArray((parent), xpro_create_double(n)))
#define xpro_addStringToArray(parent, str) (xpro_addItemToArray((parent), xpro_create_string(str)))

#define xpro_addNullToObject(parent, key) (xpro_addItemToObject((parent), xpro_create_null(), (key)))
#define xpro_addTrueToObject(parent, key) (xpro_addItemToObject((parent), xpro_create_bool(1), (key)))
#define xpro_addFalseToObject(parent, key) (xpro_addItemToObject((parent), xpro_create_bool(0), (key)))
#define xpro_addDoubleToObject(parent, n, key) (xpro_addItemToObject((parent), xpro_create_double(n), (key)))
#define xpro_addStringToObject(parent, str, key) (xpro_addItemToObject((parent), xpro_create_string(str), (key)))

#endif /* defined(__XDoc__xpro__) */
