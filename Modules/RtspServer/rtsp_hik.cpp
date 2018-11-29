/*!
 * \file rtsp_hik.c
 * \date 2018/11/28 17:49
 *
 * \author wlla
 * Contact: user@company.com
 *
 * \brief 
 *
 * 通过rtsp访问海康摄像机获取视频流
 *
 * \note
*/
#include "stdafx.h"
#include "rtsp_hik.h"

CRtspHik::CRtspHik(RTSP_REQUEST option)
    : CRtspClient(option)
{
}

CRtspHik::~CRtspHik()
{
}

string CRtspHik::make_uri()
{
    stringstream ss;
    ss << "rtsp://" << _option.ip << ":" << _option.port << "/MPEG-4/ch" << _option.channel;
    if(0 == _option.stream) {
        ss << "/main/av_stream";
    } else {
        ss << "/sub/av_stream";
    }
    _uri = ss.str();
    return _uri;
}