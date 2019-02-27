/**
 * 输入ES包
 * 输出H264片(帧)
 */
#pragma once
#include "liveObj.h"
#include "h264.h"


/**
 * ES包解析类
 */
class CES
{
public:
    CES(CLiveObj* pObj);
    ~CES(void);

    /**
     * 插入一个PES包
     * @param[in] pBuf PES帧
     * @param[in] nLen PES帧长度
     * @return 0成功 -1失败
     */
    int InputBuffer(char* pBuf, uint32_t nLen);

private:
    /**
     * 将pes解析出来的h264数据缓存，可能一个nalu存放在多个pes里面
     * @param[in] pBuf 数据内容
     * @param[in] nLen 数据长度 
     */
    void CatchData(char* pBuf, uint32_t nLen);

private:
    CLiveObj*   m_pObj;                  // 回调处理对象
    char*       m_pH264Buf;
    uint32_t    m_nH264BufLen;
    int         m_nH264DataLen;
};

