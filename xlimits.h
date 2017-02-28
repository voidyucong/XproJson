//
//  xlimits.h
//
//  Created by yucong on 17/2/16.
//  Copyright (c) 2017年 yucong. All rights reserved.
//

#ifndef __XDoc__xlimits__
#define __XDoc__xlimits__
#include <assert.h>

#define XPRO_MINBUFFER	32
#define MAX_LOG_LEN 16*1024

#if DEBUG == 1
#define xpro_assert(cond) assert(cond)
#else
#define xpro_assert(cond) (void)0
#endif

#endif /* defined(__XDoc__xlimits__) */
