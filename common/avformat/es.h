/**
 * ����ES��
 * ���H264Ƭ(֡)
 */
#pragma once
#include "avtypes.h"
#include "h264.h"

/**
 * ES��������
 */
class CES
{
public:
    CES(AV_CALLBACK cb, void* handle=NULL);
    ~CES(void);

    /**
     * PES������
     * @param[in] buff.pData PES֡
     * @param[in] buff.nLen PES֡����
     * @return 0�ɹ� -1ʧ��
     */
    int DeCode(AV_BUFF buff);

private:
    /**
     * ��pes����������h264���ݻ��棬����һ��nalu����ڶ��pes����
     * @param[in] pBuf ��������
     * @param[in] nLen ���ݳ��� 
     */
    void CatchData(char* pBuf, uint32_t nLen);

private:
    void*             m_hUser;                  // �ص��������
    AV_CALLBACK       m_fCB;

    char*       m_pH264Buf;
    uint32_t    m_nH264BufLen;
    int         m_nH264DataLen;
};

