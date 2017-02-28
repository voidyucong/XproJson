#ifndef __XDoc__xobject__
#define __XDoc__xobject__

#include "xpro.h"

#define valueboolean(v) ((v)->v.b)
#define valuestring(v) ((v)->v.s.s)
#define valuestringlen(v) ((v)->v.s.len)
#define valuenumeral(v) ((v)->v.n)

static ERR_FUNC errfunc = NULL;

XPRO_API XJson* create_json();
XPRO_API XJson* create_null();
XPRO_API XJson* create_bool(int b);
XPRO_API XJson* create_numeral(xpro_Number n);
XPRO_API XJson* create_string(const char* str);
XPRO_API XJson* create_array();
XPRO_API XJson* create_object();

XPRO_API void addItem(XJson* parent, XJson* item);

XPRO_API char* print_json(XJson* json);

XPRO_API void error_msg(const char* fmt, ...);

#endif /* defined(__XDoc__xobject__) */
