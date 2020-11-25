#pragma once
#include "util.h"
#include "utilc.h"
#include <string>
#include <list>

struct RequestParam;

struct FramePic {
    uint32_t              nDevice;              // 设备ID
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
    RequestParam         *m_pParam;         // 用户请求的参数
    int                   m_nWidth;
    int                   m_nHeight;
    int                   m_nPicLen;

private:
	bool                  m_bRun;           //< 执行状态
};

/** 直播 */
CLiveWorker* CreatLiveWorker(RequestParam param);

void InitFFmpeg();
