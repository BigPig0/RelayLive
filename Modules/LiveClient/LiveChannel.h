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
#include "Recode.h"

namespace LiveClient
{
    class CLiveChannel
    {
    public:
        /** 原始通道构造 */
        CLiveChannel();
        /** 缩放通道构造 */
        CLiveChannel(int channel, uint32_t w, uint32_t h);
        ~CLiveChannel();

#ifdef USE_FFMPEG
        /**
         * h264解码器,用来传递参数
         */
        void SetDecoder(IDecoder *decoder);
#endif

        /**
         * 向通道添加播放客户端
         * @param h 客户端实例
         * @param t 播放类型
         */
        bool AddHandle(ILiveHandle* h, HandleType t);

        /**
         * 移除一个播放客户端
         * @param h 客户端实例
         * return true:所有客户端都被移除 false:依然存在客户端
         */
        bool RemoveHandle(ILiveHandle* h);

        /** 通道中是否存在播放客户端 */
        bool Empty();

        /**
         * 接收源数据，原始通道接收压缩好的码流，其他通道接收解码后的yuv数据
         */
        void ReceiveStream(AV_BUFF buff);

        /** 获取文件头 */
        AV_BUFF GetHeader(HandleType t);

        /** 获取客户端信息 */
        vector<ClientInfo> GetClientInfo();

        /** H264中sps解析回调 */
        void set_h264_param(uint32_t nWidth, uint32_t nHeight, double fFps);

        /**
        * 从源过来的视频数据，单线程输入 
        * 以下方法由rtp接收所在的loop线程调用
        * 类中其他方法包括构造、析构都由http所在的loop线程调用
        */
        void FlvCb (AV_BUFF buff);
        void H264Cb(AV_BUFF buff);
        void TsCb  (AV_BUFF buff);
        void Mp4Cb (AV_BUFF buff);
        void RtpCb (AV_BUFF buff);
        void RtcpCb(AV_BUFF buff);
        void stop();

        bool m_bFlv;
        bool m_bMp4;
        bool m_bH264;
        bool m_bTs;
        bool m_bRtp;
        AV_BUFF               m_stFlvHead;    //flv头，内容存储在CFlv里面
        AV_BUFF               m_stMp4Head;    //mp4头，内容存储在CMP4里面

    private:
        void Init();
        void push_h264_stream(AV_BUFF buff);

        bool                     m_bDecodeSps;  // 是否已解析SPS,只有原始通道需要

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
        CH264                   *m_pH264;       // H264组包类
        CTS                     *m_pTs;         // TS组包类
        CFlv                    *m_pFlv;        // FLV组包类
        CMP4                    *m_pMp4;        // MP4组包类

#ifdef USE_FFMPEG
        IEncoder                *m_pEncoder;    // YUV编码为h264
#endif

        uint32_t                 m_nWidth;      // 视频图像的宽度
        uint32_t                 m_nHeight;     // 视频图像的高度
    };

}