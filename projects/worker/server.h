#pragma once
#include <string>
#include <stdint.h>

class ILiveSession {
public:
    virtual void AsyncSend() = 0;
};

/**
 * uri�н���������
 */
struct RequestParam {
    std::string           strPath;              // �����·��

    //��ƵԴΪ�����ַ
    std::string           strUrl;               // ԭʼ��Ƶ��ַ��������

    //��ƵԴΪGB28181
    std::string           strCode;              // �豸���룬������

    //��ƵԴΪSDK
    std::string           strHost;              //< �����豸������ip
    uint32_t              nPort;                //< �����豸�˿�
    std::string           strUsr;               //< �����豸�û���
    std::string           strPwd;               //< �����豸����
    uint32_t              nChannel;             //< ����ͨ����

    //��ʷ��Ƶ
    std::string           strBeginTime;         //< ��ʷ��Ƶ��ʼʱ��
    std::string           strEndTime;           //< ��ʷ��Ƶ����ʱ��

    //��Ƶ������Ժ�FFMPEG����
    std::string           strType;              // ý�����ͣ�Ĭ��Ϊflv
    uint32_t              nWidth;               // ��Ƶ��ȣ�Ĭ��Ϊ0����������Ƶ
    uint32_t              nHeight;              // ��Ƶ�߶ȣ�Ĭ��Ϊ0����������Ƶ
    uint32_t              nProbSize;            // ̽��PS���Ĵ�С��Ĭ��Ϊ25600
    uint32_t              nProbTime;            // ̽��PS����ʱ�䣬Ĭ��Ϊ1��
    uint32_t              nInCatch;             // ���뻺���С Ĭ��1024*16
    uint32_t              nOutCatch;            // ��������С Ĭ��1024*16
    RequestParam();
};

namespace Server {

int Init(int port);

int Cleanup();
};