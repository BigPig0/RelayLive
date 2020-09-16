#pragma once
#include "commonDefine.h"
#include "avtypes.h"
#include "NetStreamMaker.h"
#include "h264.h"

/** ֱ��ʱ�õ�fmp4��ʽ
---------------fmp4-head-----------------------------
ftyp
moov    mvhd
        trak    tkhd
                media   mdhd
                        hdlr
                        minf    head    vmhd
                                        smhd
                                        hmhd
                                        nmhd
                                dinf    dref    url
                                                urn
                                stbl    stsd
                                        stts
                                        stsz
                                        stsc
                                        stss
                                        stco
        trak...
        mvex    trex
--------------fmp4-fragment--------------------------
moof    mfhd
        traf    tfhd
                tfdt
                trun
mdat
moof
mdat...
 */

class CMP4
{
public:
    CMP4(AV_CALLBACK cb, void* handle=NULL);
    ~CMP4();

    int Code(AV_BUFF buff);

    void SetSps(uint32_t nWidth, uint32_t nHeight, double fFps);

    void SetNodelay(uint32_t nodelay){m_nNodelay = nodelay;};

private:
    /**
     * ����mp4�ļ�ͷ��Ϣ������
     */
    bool MakeHeader();

    /**
     * ����mp4Ƭ��
     */
    bool MakeMP4Frag(bool bIsKeyFrame);

    /**
     * ����һ����Ƶ�ϲ�����
     */
    bool MakeVideo(char *data, uint32_t size, bool bIsKeyFrame);

    /* ���ɹؼ�֡ */
    bool MakeKeyVideo();

private:
    CNetStreamMaker    *m_pSPS;            // ����SPS
    CNetStreamMaker    *m_pPPS;            // ����PPS
    CNetStreamMaker    *m_pKeyFrame;       // ����ؼ�֡��sps��pps�п����ں���
    CNetStreamMaker    *m_pMdat;           // ����h264����
    CNetStreamMaker    *m_pSamplePos;      // ����ÿ��sample��λ�ƣ�trun��sample_composition_time_offset
    uint32_t           m_nSampleNum;       // �����sample��������������sps��pps��nalu����(ÿ��naluΪһ֡)
    CNetStreamMaker    *m_pHeader;         // �������ɵ�MP4ͷ
    CNetStreamMaker    *m_pFragment;       // �������ɵ�MP4����

    uint32_t           m_timestamp;       // ʱ���
    uint32_t           m_tick_gap;        // ��֡��ļ��
    uint32_t           m_nNodelay;        // �Ƿ���������

    void*              m_hUser;           // �ص��������
    AV_CALLBACK        m_fCB;

    // SPS����������Ϣ
    uint32_t           m_nWidth;          // ��Ƶ��
    uint32_t           m_nHeight;         // ��Ƶ��
    uint32_t           m_nHorizresolution;// ˮƽdpi
    uint32_t           m_mVertresolution; // ��ֱdpi
    double             m_nfps;            // ��Ƶ֡��

    // ״̬
    bool               m_bMakeHeader;     // ������MP4ͷ
    bool               m_bFirstKey;       // �Ѿ������һ���ؼ�֡
    bool               m_bRun;            // ִ��״̬
    bool               m_bGotSPS;
    bool               m_bGotPPS;
    uint32_t           m_nSeq;            // MP4Ƭ�����
};