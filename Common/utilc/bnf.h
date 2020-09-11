#ifndef _UTIL_BNF_
#define _UTIL_BNF_

#include "utilc_api.h"
#include "utilc_def.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _bnf_ bnf_t;

bnf_t* create_bnf(const char *data, uint32_t len);

void destory_bnf(bnf_t *h);

bool bnf_line(bnf_t *h, char **line);

#ifdef __cplusplus
}
#endif
#endif