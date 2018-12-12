/*!
 * \file rtsp_hik.h
 * \date 2018/11/28 17:59
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
#pragma once
#include "rtsp_client.h"

class CRtspHik : public CRtspClient
{
public:
    CRtspHik(RTSP_REQUEST option);
    ~CRtspHik();

    virtual string make_uri();
private:

};

