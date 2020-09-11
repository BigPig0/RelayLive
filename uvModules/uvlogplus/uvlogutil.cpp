#include <string.h>
#include <stdlib.h>
#include "uv.h"

namespace uvLogPlus {
    const char* levelNote[] = {
        "ALL", "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL", "OFF"
    };

    int file_sys_check_path(const char *path) {
        int i, len;
        char *str;
        len = strlen(path);
        str = (char*)calloc(1, len+1);
        strcpy(str, path);

        for (i = 0; i < len; i++){
            char tmp = str[i];
            if (tmp == '/' || tmp == '\\'){
                str[i] = '\0';
                uv_fs_t req;
                int ret = uv_fs_access(NULL, &req, str, 0, NULL);
                uv_fs_req_cleanup(&req);
                if ( ret!= 0){
                    ret = uv_fs_mkdir(NULL, &req, str, 777, NULL);
                    uv_fs_req_cleanup(&req);
                    if(0 != ret) {
                        free(str);
                        return -1;
                    }
                }
                str[i] = tmp;
            }
        }
        free(str);
        return 0;
    }

    int file_sys_exist(const char *path) {
        uv_fs_t req;
        int ret = uv_fs_access(NULL, &req, path, 0, NULL);
        uv_fs_req_cleanup(&req);
        return ret;
    }
}