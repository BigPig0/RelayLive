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
    std::string           strHost;        //< �����豸������ip
    uint32_t              nPort;          //< �����豸�˿�
    std::string           strUsr;         //< �����豸�û���
    std::string           strPwd;         //< �����豸����
    uint32_t              nChannel;       //< ����ͨ����

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