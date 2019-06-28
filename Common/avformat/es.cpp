#include "stdafx.h"
#include "es.h"


CES::CES(AV_CALLBACK cb, void* handle)
    : m_pH264Buf(NULL)
    , m_nH264DataLen(0)
    , m_hUser(handle)
    , m_fCB(cb)
{
}


CES::~CES(void)
{
    if(m_pH264Buf)
        free(m_pH264Buf);
}

int CES::DeCode(AV_BUFF buff)
{
    // ES包没有es头，由h264片组成.一个es可以包含多个nalu，也有可能一个nalu分割在多个es中
    uint32_t pos = 0;
    uint32_t begin_pos = 0;
    char* begin_buff = buff.pData;
    while (pos < buff.nLen) {
        char* pPos = buff.pData + pos;
        if (pPos[0] == 0 && pPos[1] == 0 && pPos[2] == 1) {
            //if(pos > 0) Log::debug("nal3 begin width 001 pos:%d",pos);
            // 是h264报文开头
            if (pos > begin_pos) {
                CatchData(begin_buff, pos-begin_pos);
            }
            begin_pos = pos;
            begin_buff = (char*)pPos;
            pos += 3;
        } else if (pPos[0] == 0 && pPos[1] == 0 && pPos[2] == 0 && pPos[3] == 1) {
			//if(pos > 0) Log::debug("nal4 begin width 0001 pos:%d",pos);
            // 是h264报文开头
            if (pos > begin_pos) { 
                CatchData(begin_buff, pos-begin_pos);
            } 
            begin_pos = pos;
            begin_buff = (char*)pPos;
            pos += 4;
        } else {
            // 不是h264开头
            pos++;
        }
    }
    // 最后一帧
    if (buff.nLen > begin_pos) {
        CatchData(begin_buff, pos-begin_pos);
    }
    return 0;
}


void CES::CatchData(char* pBuf, uint32_t nLen) 
{
    if(m_pH264Buf == NULL)
    {
        m_pH264Buf = (char*)malloc(nLen);
        m_nH264BufLen = nLen;
    }

    bool begin = false;
    if ((pBuf[0] == 0 && pBuf[1] == 0 && pBuf[2] == 1)
        || (pBuf[0] == 0 && pBuf[1] == 0 && pBuf[2] == 0 && pBuf[3] == 1))
    {
        begin = true;
    }

    if (begin)
    {
        if(m_nH264DataLen > 0) {
            if (m_fCB != nullptr)
            {
                AV_BUFF h264 = {AV_TYPE::H264_NALU,m_pH264Buf, m_nH264DataLen};
                m_fCB(h264, m_hUser);
            }
            m_nH264DataLen = 0;
        }
    }
    else
    {
        //必须以nalu起始数据作为数据的开头
        if(m_nH264DataLen == 0) {
            Log::error("this is not nalu head");
            return;
        }
    }

    int nNewLen = m_nH264DataLen + nLen;
    if(nNewLen > m_nH264BufLen) {
        m_pH264Buf = (char*)realloc(m_pH264Buf, nNewLen);
        m_nH264BufLen = nNewLen;
    }

    memcpy(m_pH264Buf+m_nH264DataLen, pBuf, nLen);
    m_nH264DataLen += nLen;
}