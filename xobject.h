//
//  xobject.h
//
//  Created by yucong on 17/2/17.
//  Copyright (c) 2017年 yucong. All rights reserved.
//

#ifndef __XDoc__xobject__
#define __XDoc__xobject__

#include "xpro.h"

#define valueboolean(v) ((v)->v.b)
#define valuestring(v) ((v)->v.s.s)
#define valuestringlen(v) ((v)->v.s.len)
#define valuenumeral(v) ((v)->v.n)

XPRO_API XJson* create_json();
XPRO_API XJson* create_null();
XPRO_API XJson* create_bool(int b);
XPRO_API XJson* create_numeral(xpro_Number n);
XPRO_API XJson* create_string(const char* str);
XPRO_API XJson* create_array();
XPRO_API XJson* create_object();

XPRO_API void addItem(XJson* parent, XJson* item);

//XPRO_API void print_value(XJson* v);
XPRO_API char* print_json(XJson* json);

static int valuenum = 0;

#endif /* defined(__XDoc__xobject__) */
