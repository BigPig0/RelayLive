#pragma once
#include "util.h"
#include "utilc.h"
#include <string>
#include <list>

struct RequestParam;

struct FramePic {
    uint32_t              nDevice;              // �豸ID
    uint32_t              nWidth;
    uint32_t              nHeight;
    uint8_t              *pBuff;
};
void freePic(FramePic p);

class CLiveWorker
{
public:
    CLiveWorker();
    ~CLiveWorker();

    bool Play();

	void close();


public:
    RequestParam         *m_pParam;         // �û�����Ĳ���
    int                   m_nWidth;
    int                   m_nHeight;
    int                   m_nPicLen;

private:
	bool                  m_bRun;           //< ִ��״̬
};

/** ֱ�� */
CLiveWorker* CreatLiveWorker(RequestParam param);

void InitFFmpeg();
