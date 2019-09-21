#include "common.h"
#include "sdp.h"
#include "utilc.h"

static char* strDup(char const* str) {
    if (str == NULL) return NULL;
    size_t len = strlen(str) + 1;
    char* copy = (char*)calloc(1, len);

    if (copy != NULL) {
        memcpy(copy, str, len);
    }
    return copy;
}

static char* strDupSize(char const* str, size_t& resultBufSize) {
    if (str == NULL) {
        resultBufSize = 0;
        return NULL;
    }

    resultBufSize = strlen(str) + 1;
    char* copy = (char*)calloc(1,resultBufSize);
    return copy;
}

static char* strDupSize(char const* str) {
    size_t dummy;
    return strDupSize(str, dummy);
}

static bool parse_sdp_line(char const* line, uint32_t* line_length, char const* next_line) {
    next_line = NULL;
    *line_length = 0;
    for (char const* ptr = line; ptr != '\0'; ++ptr, ++line_length) {
        if(*ptr == '\r' || *ptr == '\n') {
            ++ptr;
            while (*ptr == '\r' || *ptr == '\n') 
                ++ptr;
            next_line = ptr;
            if (next_line[0] == '\0') 
                next_line = NULL; // special case for end
            break;
        }
    }

    if (line[0] == '\r' || line[0] == '\n') 
        return true;
    if (strlen(line) < 2 || line[1] != '='
        || line[0] < 'a' || line[0] > 'z') {
            Log::error("Invalid SDP line: ", line);
            return false;
    }

    return true;
}

static bool parse_sdp_line_a(sdp_t *sdp, char const *line, uint32_t line_length){
    char *attribute = (char*)calloc(1, line_length);
    sscanf(line, "a=%[^:\r\n]", attribute);
    if (!strcmp(attribute, "cat")) {
    } else if(!strcmp(attribute, "keywds")){
        sdp->attribute_keywds = (char*)calloc(1, line_length);
        sscanf(line, "a=keywds:%[^\r\n]", sdp->attribute_keywds);
    } else if(!strcmp(attribute, "tool")){
        sdp->attribute_keywds = (char*)calloc(1, line_length);
        sscanf(line, "a=keywds:%[^\r\n]", sdp->attribute_keywds);
    } else if(!strcmp(attribute, "recvonly")){
        sdp->attribute_sendrecv = (char*)calloc(1, line_length);
        memcpy(sdp->attribute_sendrecv, "recvonly", 8);
    } else if(!strcmp(attribute, "sendrecv")){
        sdp->attribute_sendrecv = (char*)calloc(1, line_length);
        memcpy(sdp->attribute_sendrecv, "sendrecv", 8);
    } else if(!strcmp(attribute, "sendonly")){
        sdp->attribute_sendrecv = (char*)calloc(1, line_length);
        memcpy(sdp->attribute_sendrecv, "sendonly", 8);
    } else if(!strcmp(attribute, "type")){
        sdp->attribute_type = (char*)calloc(1, line_length);
        sscanf(line, "a=type:%[^\r\n]", sdp->attribute_type);
    } else if(!strcmp(attribute, "charset")){
        sdp->attribute_charset = (char*)calloc(1, line_length);
        sscanf(line, "a=charset:%[^\r\n]", sdp->attribute_charset);
    } else if(!strcmp(attribute, "sdplang")){
        sdp->attribute_sdplang = (char*)calloc(1, line_length);
        sscanf(line, "a=sdplang:%[^\r\n]", sdp->attribute_sdplang);
    } else if(!strcmp(attribute, "lang")){
        sdp->attribute_lang = (char*)calloc(1, line_length);
        sscanf(line, "a=lang:%[^\r\n]", sdp->attribute_lang);
    } else if(!strcmp(attribute, "framerate")){
        sscanf(line, "a=framerate:%d", &sdp->attribute_framerate);
    } else if(!strcmp(attribute, "quality")){
        sdp->attribute_lang = (char*)calloc(1, line_length);
        sscanf(line, "a=lang:%[^\r\n]", sdp->attribute_lang);
    } else if(!strcmp(attribute, "fmtp")){
        sdp->attribute_fmtp = (char*)calloc(1, line_length);
        sscanf(line, "a=fmtp:%[^\r\n]", sdp->attribute_fmtp);
    } else if(!strcmp(attribute, "ptime")){
        sdp->attribute_ptime = (char*)calloc(1, line_length);
        sscanf(line, "a=ptime:%[^\r\n]", sdp->attribute_ptime);
    } else if(!strcmp(attribute, "orient")){
        sdp->attribute_orient = (char*)calloc(1, line_length);
        sscanf(line, "a=orient:%[^\r\n]", sdp->attribute_orient);
    } else if(!strcmp(attribute, "rtpmap")){
        SAFE_MALLOC(rtpmap_t, rtpmap);
        sscanf(line, "a=rtpmap:%d %[^/]/%d", &rtpmap->number, rtpmap->name, &rtpmap->clock);
        rtpmap->next = sdp->attribute_rtpmap;
        sdp->attribute_rtpmap = rtpmap;
    }/* else if(!strcmp(attribute, "")){

     } else if(!strcmp(attribute, "")){

     } else if(!strcmp(attribute, "")){

     }*/

    return true;
}

sdp_t* create_sdp(char const *content /*= NULL*/) {
    SAFE_MALLOC(sdp_t, sdp);
    if(!content)
        return sdp;

    char const* sdp_line = content;
    char const* next_line = NULL;
    uint32_t    line_len = 0;
    while (sdp_line) {
        if(!parse_sdp_line(sdp_line, &line_len, next_line))
            return sdp;
        switch (sdp_line[0]) {
        case 'v':
            sscanf(sdp_line, "s=%d", &sdp->version);
            break;
        case 'o': 
            sdp->origin_username = (char*)calloc(1, line_len);
            sdp->origin_nettype  = (char*)calloc(1, line_len);
            sdp->origin_address  = (char*)calloc(1, line_len);
            sscanf(sdp_line, "o=%[^ ] %d %d %[^ ] IP%d %[^\r\n]"
                , sdp->origin_username
                , &sdp->origin_sess_id
                , &sdp->origin_sess_version
                , sdp->origin_nettype
                , &sdp->origin_addrtype
                , sdp->origin_address);
            break;
        case 's':
            sdp->session_name = (char*)calloc(1, line_len);
            sscanf(sdp_line, "s=%[^\r\n]", sdp->session_name);
            break;
        case 'i':
            sdp->session_information = (char*)calloc(1, line_len);
            sscanf(sdp_line, "i=%[^\r\n]", sdp->session_information);
            break;
        case 'u':
            sdp->uri = (char*)calloc(1, line_len);
            sscanf(sdp_line, "u=%[^\r\n]", sdp->uri);
            break;
        case 'e':
            sdp->email = (char*)calloc(1, line_len);
            sscanf(sdp_line, "e=%[^\r\n]", sdp->email);
            break;
        case 'p':
            sdp->phone = (char*)calloc(1, line_len);
            sscanf(sdp_line, "p=%[^\r\n]", sdp->phone);
            break;
        case 'c':
            sdp->connect_nettype = (char*)calloc(1, line_len);
            sdp->connect_address = (char*)calloc(1, line_len);
            sscanf(sdp_line, "c=%[^ ] IP%d %[^/\r\n]\/%[^\r\n]"
                , sdp->connect_nettype
                , &sdp->connect_addrtype
                , sdp->connect_address
                , &sdp->connect_ttl);
            break;
        case 'b':
            sdp->bandwidth_type = (char*)calloc(1, line_len);
            sscanf(sdp_line, "b=%[^:]:%d", sdp->bandwidth_type, sdp->bandwidth);
            break;
        case 't':
            sscanf(sdp_line, "t=%d %d", &sdp->time_start, &sdp->time_stop);
            break;
        case 'r':
            break;
        case 'z':
            sdp->timezone_adjustments = (char*)calloc(1, line_len);
            sscanf(sdp_line, "z=%[^\r\n]", sdp->timezone_adjustments);
            break;
        case 'k':
            sdp->encryption_method = (char*)calloc(1, line_len);
            sdp->encryption_key    = (char*)calloc(1, line_len);
            sscanf(sdp_line, "k=%[^:]:%[^\r\n]", sdp->encryption_method, sdp->encryption_key);
            break;
        case 'a':
            parse_sdp_line_a(sdp, sdp_line, line_len);
            break;
        case 'm':
            sdp->media          = (char*)calloc(1, line_len);
            sdp->media_protocol = (char*)calloc(1, line_len);
            sdp->media_format   = (char*)calloc(1, line_len);
            sscanf(sdp_line, "m=%[^ ] %d %[^ \r\n] %[^\r\n]"
                , sdp->media
                , &sdp->media_port
                , sdp->media_protocol
                , sdp->media_format);
            break;
        default:
            break;
        }
        sdp_line = next_line;
    }
    return sdp;
}

void destory_sdp(sdp_t* sdp) {
    SAFE_FREE(sdp->origin_username);
    SAFE_FREE(sdp->origin_nettype);
    SAFE_FREE(sdp->origin_address);
    SAFE_FREE(sdp->session_name);
    SAFE_FREE(sdp->session_information);
    SAFE_FREE(sdp->uri);
    SAFE_FREE(sdp->email);
    SAFE_FREE(sdp->phone);
    SAFE_FREE(sdp->connect_nettype);
    SAFE_FREE(sdp->connect_address);
    SAFE_FREE(sdp->bandwidth_type);
    SAFE_FREE(sdp->bandwidth_type);
    SAFE_FREE(sdp->encryption_method);
    SAFE_FREE(sdp->encryption_key);
    SAFE_FREE(sdp->attribute_cat);
    SAFE_FREE(sdp->attribute_keywds);
    SAFE_FREE(sdp->attribute_tool);
    SAFE_FREE(sdp->attribute_sendrecv);
    SAFE_FREE(sdp->attribute_type);
    SAFE_FREE(sdp->attribute_charset);
    SAFE_FREE(sdp->attribute_sdplang);
    SAFE_FREE(sdp->attribute_lang);
    SAFE_FREE(sdp->attribute_quality);
    SAFE_FREE(sdp->attribute_fmtp);
    SAFE_FREE(sdp->attribute_ptime);
    SAFE_FREE(sdp->attribute_orient);
    SAFE_FREE(sdp->media);
    SAFE_FREE(sdp->media_protocol);
    SAFE_FREE(sdp->media_format);
    rtpmap_t* rtpmap = sdp->attribute_rtpmap;
    rtpmap_t* rtpmap_tmp;
    while (rtpmap) {
        rtpmap_tmp = rtpmap;
        rtpmap = rtpmap->next;
        SAFE_FREE(rtpmap_tmp);
    }
    SAFE_FREE(sdp);
}