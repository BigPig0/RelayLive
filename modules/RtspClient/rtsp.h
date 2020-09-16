#pragma once

enum devType {
    DEV_DAHUA = 0,
    DEV_HIK
};

struct RTSP_REQUEST
{
    string     ssid;
    string     ip;
    uint32_t   port;
    string     user_name;
    string     password;
    uint32_t   channel;     //ͨ���ţ�1��2...��
    uint32_t   stream;      //���� 0:������ 1:������
    uint32_t   rtp_port;
    devType    dev_type;    //�豸����
};

extern int set_uv(void* uv);

extern void stop_uv();

/**
 * ��������
 */
typedef void (*play_cb)(string ssid, int status);
extern int rtsp_play(RTSP_REQUEST option, play_cb cb);

/**
 * �ر�rtsp
 */
extern int rtsp_stop(string ssid);

extern char* rtsp_strerr(int status);