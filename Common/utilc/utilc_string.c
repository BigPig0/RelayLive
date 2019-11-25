#include <string.h>
#include <ctype.h>

int htoi(const char *s) {
    int i;
    int ret = 0;
    int len = strlen(s);
    for (i = 0; i<len; ++i) {
        if (s[i] >= 'a' && s[i] <= 'f') {
            ret = 16 * ret + (10 + tolower(s[i]) - 'a');
        } else if (s[i] >= 'A' && s[i] <= 'F') {
            ret = 16 * ret + (10 + tolower(s[i]) - 'A');
        } else if (s[i] >= '0' && s[i] <= '9') {
            ret = 16 * ret + (tolower(s[i]) - '0');
        } 
    }
    return ret;
}