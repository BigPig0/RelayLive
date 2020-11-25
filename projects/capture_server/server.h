#pragma once
#include <string>
#include <list>
#include <stdint.h>


/**
 * uri�н���������
 */
struct RequestParam {
    std::string           strUrl;               // ԭʼ��Ƶ��ַ��������
    std::string           strType;              // �������Ƶ���ͣ�Ĭ��Ϊmp4
    std::string           strImgType;           // �����ͼƬ���ͣ�Ĭ��Ϊjpg
    uint32_t              nWidth;               // ��Ƶ��ȣ�Ĭ��Ϊ0����������Ƶ
    uint32_t              nHeight;              // ��Ƶ�߶ȣ�Ĭ��Ϊ0����������Ƶ
    uint32_t              nProbSize;            // ̽��PS���Ĵ�С��Ĭ��Ϊ25600
    uint32_t              nProbTime;            // ̽��PS����ʱ�䣬Ĭ��Ϊ1��
    uint32_t              nInCatch;             // ���뻺���С Ĭ��1024*16
    uint32_t              nOutCatch;            // ��������С Ĭ��1024*16
    uint32_t              nImageNumber;         // ��ȡͼƬ��������Ĭ��0������0ʱ��Ҫ��ͼ
    uint32_t              nVideoDuration;       // ��ȡ����Ƶ�ļ���ʱ����Ĭ��0������0ʱ��Ҫ����Ƶ
    std::list<std::string> lstImgPath;           // ���ɵ�ͼƬ�洢·��
    std::string           videoPath;            // ���ɵ���Ƶ�洢·��
    std::string           strSavePath;          // ͼƬ����Ƶ�洢��λ��
    RequestParam();
};

namespace Server {

int Init(int port);

int Cleanup();
};