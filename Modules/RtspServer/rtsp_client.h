/*!
 * \file rtsp_client.h
 * \date 2018/11/28 18:01
 *
 * \author wlla
 * Contact: user@company.com
 *
 * \brief 
 *
 * rtsp客户端
 *
 * \note
*/
#pragma once
#include "rtsp.h"
#include "uv.h"

#define SOCKET_RECV_BUFF_LEN 1024

enum RTSP_ERR_NUM
{
    RTSP_ERR_SUCESS = 0,
    RTSP_ERR_CONNECT_FAILED = -1000,
    RTSP_ERR_TCP_RECV_FAILED,
    RTSP_ERR_IP4_ADDR_FAILED,
    RTSP_ERR_TCP_SEND_FAILED,
    RTSP_ERR_OPTION_FAILED,
    RTSP_ERR_DESCRIBE_FAILED,
    RTSP_ERR_SETUP_FAILED,
    RTSP_ERR_PLAY_FAILED,
    RTSP_ERR_TEARDOWN_FAILED
};

enum RTSP_STEP
{
    RTSP_STEP_NONE,
    RTSP_STEP_OPTION,
    RTSP_STEP_DESCRIBE,
    RTSP_STEP_SETUP,
    RTSP_STEP_PLAY,
    RTSP_STEP_TEARDOWN
};

enum RTSP_STEP_STATE
{
    RTSP_STEP_BEGIN = 0, //开始发起请求
    RTSP_STEP_SUCESS,    //请求成功
    RTSP_STEP_FAILED,    //请求失败
};

enum RTSP_CONNECT_STATE
{
    RTSP_CONNECT_INIT = 0,
    RTSP_CONNECT_CONNECTED,
    RTSP_CONNECT_SHUTDOWN,
    RTSP_CONNECT_CLOSE
};

class CRtspClient
{
public:
    CRtspClient(RTSP_REQUEST option);
    ~CRtspClient();

    int play(play_cb cb);
    int stop();

    virtual int send_options();
    virtual int send_describe();
    virtual int send_setup();
    virtual int send_play();
    virtual int send_teardown();

    virtual int parse_options();
    virtual int parse_describe();
    virtual int parse_setup();
    virtual int parse_play();
    virtual int parse_teardown();

    /** 根据option值生成uri */
    virtual string make_uri() = 0;

public:
    char*               _send_buff;
    char*               _recv_buff;
    uint32_t            _recv_len;

    uv_tcp_t            _tcp;
    uv_connect_t        _conn;
    uv_write_t          _write;
    uv_shutdown_t       _shutdown;

    RTSP_REQUEST        _option;
    RTSP_STEP           _step;
    RTSP_STEP_STATE     _step_state;
    RTSP_CONNECT_STATE  _conn_state;

    play_cb             _play_cb;
    bool                _need_auth;
    string              _uri;
    string              _realm;
    string              _nonce;
    string              _session;
    string              _auth_md5;
};
