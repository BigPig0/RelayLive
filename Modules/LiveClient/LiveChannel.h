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


        /** 获取客户端信息 */
        vector<ClientInfo> GetClientInfo();

        void stop();

    private:

        vector<ILiveHandle*>     m_vecHandle;   // 播放实例 
        CriticalSection          m_csHandle;

#ifdef USE_FFMPEG
        IEncoder                *m_pEncoder;    // YUV编码为h264
#endif

        int                      m_nChannel;    // 通道号
        uint32_t                 m_nWidth;      // 视频图像的宽度
        uint32_t                 m_nHeight;     // 视频图像的高度
    };

}