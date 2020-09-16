#ifndef _UTIL_CDEF_
#define _UTIL_CDEF_

#include <stdlib.h>

#define SAFE_MALLOC(t, p)         t *p = (t*)calloc(1,sizeof(t))
#define SAFE_MALLOC_COUNT(t, p, c) t *p = (t*)calloc(c, sizeof(t))
#define SAFE_FREE(p)              if(NULL != (p)){free((p));(p) = NULL;}

#define CHECK(p,r)                if(p){printf(#p);return r;}
#define CHECK_POINT_VOID(p)       if(NULL == (p)){printf("NULL == "#p);return;}
#define CHECK_POINT_BOOL(p)       if(NULL == (p)){printf("NULL == "#p);return false;}
#define CHECK_POINT_NULLPTR(p)    if(NULL == (p)){printf("NULL == "#p);return NULL;}
#define CHECK_POINT_INT(p,r)      if(NULL == (p)){printf("NULL == "#p);return (r);}

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#ifndef INFINITE
#define INFINITE 0xFFFFFFFF 
#endif

#ifndef __cplusplus
#define bool int
#define true  1
#define false 0
#endif

#endif