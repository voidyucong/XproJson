#ifndef __XDoc__xobject__
#define __XDoc__xobject__

#include "xproJson.h"

#define valueboolean(v) ((v)->v.b)
#define valuestring(v) ((v)->v.s.s)
#define valuestringlen(v) ((v)->v.s.len)
#define valuedouble(v) ((v)->v.n)
#define valueinteger(v) ((v)->v.i)


XPRO_API XJson* create_json();
XPRO_API XJson* create_null();
XPRO_API XJson* create_bool(int b);
XPRO_API XJson* create_double(xpro_Number n);
XPRO_API XJson* create_integer(xpro_Integer i);
XPRO_API XJson* create_string(const char* str);
XPRO_API XJson* create_array();
XPRO_API XJson* create_object();

XPRO_API void free_json(XJson* value);

XPRO_API void addItem(XJson* parent, XJson* item);

XPRO_API char* print_json(XJson* json);

XPRO_API void setErrFunc(ERR_FUNC func);
XPRO_API void error_msg(const char* msg);
XPRO_API void error_check(int cond, const char* fmt, ...);

#endif /* defined(__XDoc__xobject__) */
