#include "common.h"
#include "rtsp.h"
#include "utilc.h"
#include "cstl_easy.h"

char* response_status[] = {
    "100 Continue(all 100 range)",
    "110 Connect Timeout",
    "200 OK",
    "201 Created",
    "250 Low on Storage Space",
    "300 Multiple Choices",
    "301 Moved Permanently",
    "302 Moved Temporarily",
    "303 See Other",
    "304 Not Modified",
    "305 Use Proxy",
    "350 Going Away",
    "351 Load Balancing",
    "400 Bad Request",
    "401 Unauthorized",
    "402 Payment Required",
    "403 Forbidden",
    "404 Not Found",
    "405 Method Not Allowed",
    "406 Not Acceptable",
    "407 Proxy Authentication Required",
    "408 Request Time-out",
    "410 Gone",
    "411 Length Required",
    "412 Precondition Failed",
    "413 Request Entity Too Large",
    "414 Request-URI Too Large",
    "415 Unsupported Media Type",
    "451 Parameter Not Understood",
    "452 reserved",
    "453 Not Enough Bandwidth",
    "454 Session Not Found",
    "455 Method Not Valid in This State",
    "456 Header Field Not Valid for Resource",
    "457 Invalid Range",
    "458 Parameter Is Read-Only",
    "459 Aggregate operation not allowed",
    "460 Only aggregate operation allowed",
    "461 Unsupported transport",
    "462 Destination unreachable",
    "500 Internal Server Error",
    "501 Not Implemented",
    "502 Bad Gateway",
    "503 Service Unavailable",
    "504 Gateway Time-out",
    "505 RTSP Version not supported",
    "551 Option not supported"
};

/**
 * ��������ṹ��
 */
static void destory_rtsp_ruquest(rtsp_ruquest_t *req) {
    SAFE_FREE(req->uri);
    HASH_MAP_DESTORY(req->headers, string_t*, string_t*, string_destroy, string_destroy);
    free(req);
}

/**
 * ��һ�������л�ȡ\r\n֮ǰ��һ���֣�����һ��
 * @param line �������ݵ���ʼλ��
 * @param len ���ݵĳ��ȣ�����ʼλ�ÿ�ʼ�����
 * @param next_line �����һ�е�λ��
 * @return ��һ�еĳ���
 */
static int rtsp_get_line(char *line, int len, char **next_line) {
    *next_line = NULL;
    int line_length = 0;
    for (char const* ptr = line; line_length < len; ++ptr, ++line_length) {
        if(*ptr == '\r' || *ptr == '\n') {
            ++ptr;
            ++line_length;
            while (*ptr == '\r' || *ptr == '\n') {
                ++ptr;
                ++line_length;
            }
            *next_line = (char*)ptr;
            if (line_length == len) 
                *next_line = NULL; // special case for end
            break;
        }
    }

    return line_length;
}

typedef struct _rtsp_ {
    void               *user;       //�û��Զ�������
    net_stream_maker_t *sm;         //����δ��������
    rtsp_callback      cb;          //rtsp������ɻص�
} rtsp;

rtsp* create_rtsp(void *user, rtsp_callback cb) {
    SAFE_MALLOC(rtsp, ret);
    ret->user = user;
    ret->cb = cb;
    ret->sm = create_net_stream_maker();
    return ret;
}

void destory_rtsp(rtsp *h) {
    destory_net_stream_maker(h->sm);
    SAFE_FREE(h);
}

void rtsp_handle_request(rtsp *h, char *data, int len) {
    char *tmpbuf = data;    //���ܵ������������ָ������Ŀ�ʼλ�á������Ƕ�����ݰ���϶���
    int tmplen   = len;     //��������ĳ���   
    int usedlen  = len;     //��ǰ���ݰ������������λ�õĳ���
    int finish   = 0;       //��ǰ���ݰ��Ƿ����������
    rtsp_ruquest_t *req;      //����������������

    // ��ѯ��\r\n\r\n����һ���������
    for(int i=3; i<len; ++i) {
        if(data[i-3]=='\r' && data[i-2]=='\n' && data[i-1]=='\r' && data[i]=='\n') {
            tmpbuf = data;
            usedlen = tmplen = i+1;
            finish = 1;
            // ����л�������ݰ�����ϳ�һ������
            if(get_net_stream_len(h->sm) > 0) {
                net_stream_append_data(h->sm, data, usedlen);
                tmpbuf = get_net_stream_data(h->sm);
                tmplen = get_net_stream_len(h->sm);
            }
            break;
        }
    }

     // û�л�ȡ�����������󣬻�������
    if (!finish) {
        if(len > 0) {
            net_stream_append_data(h->sm, data, len);
        }
        return;
    }

    // ��ȡ������������
    req = (rtsp_ruquest_t*)calloc(1, sizeof(rtsp_ruquest_t));
    do {
        char *next_line;
        char *buff   = tmpbuf;
        int buff_len = tmplen;
        int error = 0;
        int ret = 0;

        // ������һ��
        char *method = NULL;
        char *uri = NULL;
        int line_len = rtsp_get_line(buff, tmplen, &next_line);
        if(!line_len) {
            error = 1;
            break;
        }

        method = (char*)calloc(1, line_len);
        uri    = (char*)calloc(1, line_len);
        ret = sscanf(buff, "%[^ ] %[^ ] %*[rtspRTSP]/1.0", method, uri);
        if(ret <= 0) {
            error = 1;
            break;
        }
        if(!strcasecmp(method, "OPTIONS"))
            req->method = RTSP_OPTIONS;
        else if(!strcasecmp(method, "DESCRIBE"))
            req->method = RTSP_DESCRIBE;
        else if(!strcasecmp(method, "SETUP"))
            req->method = RTSP_SETUP;
        else if(!strcasecmp(method, "PLAY"))
            req->method = RTSP_PLAY;
        else if(!strcasecmp(method, "PAUSE"))
            req->method = RTSP_PAUSE;
        else if(!strcasecmp(method, "TEARDOWN"))
            req->method = RTSP_TEARDOWN;
        else {
            error = 1;
            break;
        }
        SAFE_FREE(method);
        req->uri = uri;

        // ����ʣ����
        buff     = next_line;
        buff_len = buff_len - line_len;
        while(buff) {
            char *key, *value;
            string_t *strKey, *strValue;
            line_len = rtsp_get_line(buff, buff_len, &next_line);
            key   = (char*)calloc(1, line_len);
            value = (char*)calloc(1, line_len);
            ret = sscanf(buff, "%[^:]: %[^\r\n]", key, value);
            buff     = next_line;
            buff_len = buff_len - line_len;
            if(ret <= 0) {
                free(key);
                free(value);
                continue;
            }
            if(!strcasecmp(key, "CSeq")) {
                req->CSeq = atoi(value);
			} else {
				//������rtp�˿�
				if(!strcasecmp(key, "Transport")) {
					char *cp = strstr(value, "client_port=");
					if(cp) {
						int p1=0, p2=0;
						sscanf(cp, "client_port=%d-%d", &p1, &p2);
						req->rtp_port = p1;
						req->rtcp_port = p2;
					}
				}
                if(!req->headers) {
                    req->headers = create_hash_map(void*, void*);
                    hash_map_init_ex(req->headers, 0, string_map_hash, string_map_compare);
                }
                strKey = create_string();
                string_init_cstr(strKey, key);
                strValue = create_string();
                string_init_cstr(strValue, value);
                hash_map_insert_easy(req->headers, strKey, strValue);
            }
            free(key);
            free(value);
        }
    } while (0);

    //�ص�����
    if(h->cb) {
        h->cb(h->user, req);
    }
    destory_rtsp_ruquest(req);


    //�������µı���
    if(len > tmplen) {
        rtsp_handle_request(h, data+tmplen, len-tmplen);
    }

}

void rtsp_handle_answer(rtsp *h, char *data, int len) {

}

const char* rtsp_request_get_header(rtsp_ruquest_t *req, char *data){
    string_t* str_h = (string_t*)hash_map_find_easy_str(req->headers, data);
    if(str_h)
        return string_c_str(str_h);

    return NULL;
}