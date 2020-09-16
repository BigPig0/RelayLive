/*!
 * \file RtspLiveServer.h
 * \date 2019/06/10 15:46
 *
 * \author wlla
 * Contact: user@company.com
 *
 * \brief �Զ���ص�����
 *
 * TODO: ʵ��һ���ص�����������ҵ����ص��߼���������ص���������ʵ��
 *
 * \note
*/

#pragma once
#include "RtspSocket.h"
#include "ring_buff.h"

namespace RtspServer
{
    class CRtspWorker;
    
    /**
     * per session structure
     */
    typedef struct _pss_rtsp_client_ {
        struct _pss_rtsp_client_  *pss_next;
        char                path[128];             //���Ŷ������ַ
        char                clientName[50];        //���Ŷ˵�����
        char                clientIP[50];          //���Ŷ˵�ip
        char                code[50];              //�豸����
        uint32_t            channel;               //ͨ���ţ�ָʾ��ͬ��С������
        char                strErrInfo[128];       //���ܲ���ʱ�Ĵ�����Ϣ
        ring_buff_t*        ring;             //�������ݻ�����
        uint32_t            tail;              //ringbuff�е�λ��
        bool                culled;
        CRtspWorker*        m_pWorker;
        CRtspSocket*        rtspClient;
        bool                playing;           //�Ƿ��ڲ���
    } pss_rtsp_client;

    extern int callback_live_rtsp(CRtspSocket *client, RTSP_REASON reason, void *user);

}