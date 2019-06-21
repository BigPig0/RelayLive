/*!
 * \file LiveChannel.h
 * \date 2019/06/21 16:45
 *
 * \author wlla
 * Contact: user@company.com
 *
 * \brief 
 *
 * TODO: 子通道视频处理
 *
 * \note
*/

#pragma once
#include "LiveClient.h"
#include "avtypes.h"
#include "ts.h"
#include "flv.h"
#include "mp4.h"
#include "h264.h"
#include "uv.h"

namespace LiveClient
{
    class CLiveChannel
    {
    public:
        CLiveChannel(int channel = 0);
        ~CLiveChannel();

        void SetParam(uint32_t w, uint32_t h){
            m_nWidth = w;
            m_nHeight = h;
        }

        bool AddHandle(ILiveHandle* h, HandleType t);
        bool RemoveHandle(ILiveHandle* h);
        bool Empty();

        AV_BUFF GetHeader(HandleType t);

        /** 获取客户端信息 */
        string GetClientInfo();

        /** H264中sps解析回调 */
        void set_h264_param(uint32_t nWidth, uint32_t nHeight, double fFps);
        /**
        * 从源过来的视频数据，单线程输入 
        * 以下方法由rtp接收所在的loop线程调用
        * 类中其他方法包括构造、析构都由http所在的loop线程调用
        */
        void push_flv_stream (AV_BUFF buff);
        void push_h264_stream(AV_BUFF buff);
        void push_ts_stream  (AV_BUFF buff);
        void push_fmp4_stream(AV_BUFF buff);
        void push_rtp_stream (AV_BUFF buff);
        void push_rtcp_stream(AV_BUFF buff);
        void stop();

        bool m_bFlv;
        bool m_bMp4;
        bool m_bH264;
        bool m_bTs;
        bool m_bRtp;
        AV_BUFF               m_stFlvHead;    //flv头，内容存储在CFlv里面
        AV_BUFF               m_stMp4Head;    //mp4头，内容存储在CMP4里面

    private:
        vector<ILiveHandle*>     m_vecLiveFlv;  // 播放实例 
        CriticalSection          m_csFlv;
        vector<ILiveHandle*>     m_vecLiveMp4;  // 播放实例 
        CriticalSection          m_csMp4;
        vector<ILiveHandle*>     m_vecLiveH264; // 播放实例 
        CriticalSection          m_csH264;
        vector<ILiveHandle*>     m_vecLiveTs;   // 播放实例 
        CriticalSection          m_csTs;
        vector<ILiveHandleRtp*>  m_vecLiveRtp;  // 播放实例 
        CriticalSection          m_csRtp;

        int                      m_nChannel;    // 通道号
        CH264                   *m_pH264;            // H264组包类
        CTS                     *m_pTs;              // TS组包类
        CFlv                    *m_pFlv;             // FLV组包类
        CMP4                    *m_pMp4;             // MP4组包类

        uint32_t                 m_nWidth;
        uint32_t                 m_nHeight;
    };

}