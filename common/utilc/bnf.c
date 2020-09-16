#include "bnf.h"
#include "utilc_def.h"

typedef struct _bnf_ {
    char *data;
    uint32_t len;

    char *line;
} bnf_t;

bnf_t* create_bnf(const char *data, uint32_t len) {
    SAFE_MALLOC(bnf_t, ret);
    ret->data = (char*)data;
    ret->len = len;
    ret->line = (char*)data;
    return ret;
}

void destory_bnf(bnf_t *h) {
    SAFE_FREE(h);
}

/**
 * ��ȡ��һ���ı�
 * @param line ���뵱ǰ�е�λ��
 * @param line_length �����ǰ�еĳ���
 * @param next_line �����һ�е�λ��
 */
static bool parse_line(const char *line, uint32_t* line_length, char const** next_line) {
    const char *ptr = line;
    *next_line = NULL;
    *line_length = 0;
    for (; *ptr != '\0'; ++ptr, ++(*line_length)) {
        if(*ptr == '\r' || *ptr == '\n') {
            ++ptr;
            while (*ptr == '\r' || *ptr == '\n') 
                ++ptr;
            *next_line = ptr;
            if (next_line[0] == '\0') 
                *next_line = NULL; // special case for end
            break;
        }
    }

    if (line[0] == '\r' || line[0] == '\n') 
        return true;


    return true;
}

bool bnf_line(bnf_t *h, char **line) {
    char *next_line;
    uint32_t line_len;

    if (h->line == NULL)
        return false;

    *line = h->line;

    parse_line(h->line, &line_len, (char const**)&next_line);
    h->line = next_line;

    return true;
}