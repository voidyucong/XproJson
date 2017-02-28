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
typedef int         xpro_Boolean;

typedef void (*ERR_FUNC)(const char*);

typedef union Value_ {
    struct {char* s; int32_t len;} s;
    xpro_Boolean b;
    xpro_Number n;
    xpro_Integer i;
} Value_;

typedef struct XJson {
    int32_t t;
    int32_t level;
    int32_t child_size;
    union Value_ v;
    struct XJson* next, *prev;
    struct XJson* child, *head;
    char* key;
} XJson;


XPRO_API XJson* xpro_parser(const char* jsonstr);
XPRO_API XJson* xpro_parserFile(const char* fileName);
XPRO_API void xpro_free(XJson* json);

XPRO_API char* xpro_print(XJson* json);

XPRO_API int32_t xpro_getArraySize(XJson* array);
XPRO_API int32_t xpro_getObjectSize(XJson* object);

XPRO_API XJson* xpro_create_null();
XPRO_API XJson* xpro_create_bool(int b);
XPRO_API XJson* xpro_create_numeral(xpro_Number n);
XPRO_API XJson* xpro_create_string(const char* str);
XPRO_API XJson* xpro_create_array();
XPRO_API XJson* xpro_create_object();

XPRO_API void xpro_addItem(XJson* parent, XJson* item);
XPRO_API void xpro_addItemToObject(XJson* parent, XJson* item, const char* key);

XPRO_API XJson* xpro_detachItemInArray(XJson* array, int index);
XPRO_API XJson* xpro_detachItemInOjbect(XJson* object, const char* key);

XPRO_API XJson* xpro_getItemInArray(XJson* array, int index);
XPRO_API XJson* xpro_getItemInObject(XJson* object, const char* key);

XPRO_API void xpro_minify(char* json);

XPRO_API void xpro_saveFile(const char* fileName, const char* json);

XPRO_API void xpro_setErrFunc(ERR_FUNC func);

long xprotime();

#endif /* defined(__XDoc__xpro__) */
