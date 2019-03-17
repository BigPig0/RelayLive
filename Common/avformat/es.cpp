#include "stdafx.h"
#include "es.h"


CES::CES(void* handle)
    : m_pH264Buf(NULL)
    , m_nH264DataLen(0)
    , m_hUser(handle)
    , m_fCB(nullptr)
{
}


CES::~CES(void)
{
}

int CES::InputBuffer(char* pBuf, uint32_t nLen)
{
    // ES包没有es头，由h264片组成
    //nal_unit_header_t* nalUnit = nullptr;
    //uint8_t nNalType = 0;
    uint32_t pos = 0;
    uint32_t begin_pos = 0;
    char* begin_buff = pBuf;
    while (pos < nLen)
    {
        char* pPos = pBuf + pos;
        if (pPos[0] == 0 && pPos[1] == 0 && pPos[2] == 1)
        {
            //if(pos > 0) Log::debug("nal3 begin width 001 pos:%d",pos);
            // 是h264报文开头
            if (pos > begin_pos)
            {
                // 回调处理H264帧
                //if (m_pObj != nullptr)
                //{
                //    m_pObj->ESParseCb(begin_buff, pos-begin_pos/*, nNalType*/);
                //}
                CatchData(begin_buff, pos-begin_pos);
            }
            //nalUnit = (nal_unit_header_t*)(pPos+3);
            //nNalType = nalUnit->nal_type;
            begin_pos = pos;
            begin_buff = (char*)pPos;
            pos += 3;
        }
        else if (pPos[0] == 0 && pPos[1] == 0 && pPos[2] == 0 && pPos[3] == 1)
        {
			//if(pos > 0) Log::debug("nal4 begin width 0001 pos:%d",pos);
            // 是h264报文开头
            if (pos > begin_pos)
            {
                // 回调处理H264帧
                //if (m_pObj != nullptr)
                //{
                //    m_pObj->ESParseCb(begin_buff, pos-begin_pos/*, nNalType*/);
                //}
                CatchData(begin_buff, pos-begin_pos);
            }
            //nalUnit = (nal_unit_header_t*)(pPos+4);
            //nNalType = nalUnit->nal_type;
            begin_pos = pos;
            begin_buff = (char*)pPos;
            pos += 4;
        }
        else
        {
            // 不是h264开头
            pos++;
        }
    }
    // 最后一帧
    if (nLen > begin_pos)
    {
        // 回调处理H264帧
        //if (m_pObj != nullptr)
        //{
        //    m_pObj->ESParseCb(begin_buff, nLen-begin_pos/*, nNalType*/);
        //}
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
                m_fCB(m_pH264Buf, m_nH264DataLen, m_hUser);
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