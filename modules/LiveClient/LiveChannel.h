/*!
 * \file LiveChannel.h
 * \date 2019/06/21 16:45
 *
 * \author wlla
 * Contact: user@company.com
 *
 * \brief 
 *
 * TODO: ��ͨ����Ƶ����
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
        /** ԭʼͨ������ */
        CLiveChannel();
        /** ����ͨ������ */
        CLiveChannel(int channel, uint32_t w, uint32_t h);
        ~CLiveChannel();

#ifdef EXTEND_CHANNELS
        /**
         * h264������,�������ݲ���
         */
        void SetDecoder(IDecoder *decoder);
#endif

        /**
         * ��ͨ����Ӳ��ſͻ���
         * @param h �ͻ���ʵ��
         * @param t ��������
         */
        bool AddHandle(ILiveHandle* h, HandleType t);

        /**
         * �Ƴ�һ�����ſͻ���
         * @param h �ͻ���ʵ��
         * return true:���пͻ��˶����Ƴ� false:��Ȼ���ڿͻ���
         */
        bool RemoveHandle(ILiveHandle* h);

        /** ͨ�����Ƿ���ڲ��ſͻ��� */
        bool Empty();

        /**
         * ����Դ���ݣ�ԭʼͨ������ѹ���õ�����������ͨ�����ս�����yuv����
         */
        void ReceiveStream(AV_BUFF buff);


        /** ��ȡ�ͻ�����Ϣ */
        vector<ClientInfo> GetClientInfo();

        void stop();

    private:

        vector<ILiveHandle*>     m_vecHandle;   // ����ʵ�� 
        CriticalSection          m_csHandle;

#ifdef EXTEND_CHANNELS
        IEncoder                *m_pEncoder;    // YUV����Ϊh264
#endif

        int                      m_nChannel;    // ͨ����
        uint32_t                 m_nWidth;      // ��Ƶͼ��Ŀ��
        uint32_t                 m_nHeight;     // ��Ƶͼ��ĸ߶�
    };

}