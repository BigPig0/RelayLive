#ifndef _UTIL_CDEF_
#define _UTIL_CDEF_
#include <stdlib.h>
#include <stdio.h>

#define SAFE_MALLOC(t, p)         t *p = (t*)calloc(1,sizeof(t))
#define SAFE_MALLOC_COUNT(t, p, c) t *p = (t*)calloc(c, sizeof(t))
#define SAFE_FREE(p)              if(NULL != (p)){free (p);(p) = NULL;}

#define CHECK(p,r)                if(p){printf(#p);return r;}
#define CHECK_POINT_VOID(p)       if(NULL == (p)){printf("NULL == "#p);return;}
#define CHECK_POINT_BOOL(p)       if(NULL == (p)){printf("NULL == "#p);return false;}
#define CHECK_POINT_NULLPTR(p)    if(NULL == (p)){printf("NULL == "#p);return NULL;}
#define CHECK_POINT_INT(p,r)      if(NULL == (p)){printf("NULL == "#p);return (r);}

#ifndef __cplusplus
#define bool uint8_t
#define true  1
#define false 0
#endif

#endif