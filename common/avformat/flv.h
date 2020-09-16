#pragma once
#include "avtypes.h"
#include "NetStreamMaker.h"
#include "h264.h"

//enum flv_tag_type
//{
//    callback_flv_header = 0,
//    callback_script_tag,
//    callback_video_spspps_tag,
//    callback_key_video_tag,
//    callback_video_tag
//};

class CFlvStreamMaker : public CNetStreamMaker
{
public:
    void append_amf_string(const char *str );
    void append_amf_double(double d );
} ;

class CFlv
{
public:
    CFlv(AV_CALLBACK cb, void* handle=NULL);
    ~CFlv(void);

    int Code(AV_BUFF buff);

    void SetSps(uint32_t nWidth, uint32_t nHeight, double fFps);

    void SetNodelay(uint32_t nodelay){m_nNodelay = nodelay;};

private:
    /**
     * ����flv�ļ�ͷ��Ϣ������
     */
    bool MakeHeader();

    /**
     * ����һ����Ƶ�ϲ�����
     */
    bool MakeVideo(char *data,int size,int bIsKeyFrame);

	/* ���ɹؼ�֡ */
	bool MakeKeyVideo();

private:
    CFlvStreamMaker*        m_pSPS;            // ����SPS���ݵĻ���
    CFlvStreamMaker*        m_pPPS;            // ����PPS���ݵĻ���
    CFlvStreamMaker*        m_pKeyFrame;       // ����ؼ�֡��sps��pps�п����ں���
    CFlvStreamMaker*        m_pHeader;         // flvͷ����
    CFlvStreamMaker*        m_pData;           // ������ɵ�flv����

    uint32_t           m_timestamp;       // ʱ���
    uint32_t           m_tick_gap;        // ��֡��ļ��
    uint32_t           m_nNodelay;        // �Ƿ���������

    void*             m_hUser;                  // �ص��������
    AV_CALLBACK       m_fCB;

    // SPS����������Ϣ
    uint32_t           m_nWidth;          // ��Ƶ��
    uint32_t           m_nHeight;         // ��Ƶ��
    double             m_nfps;            // ��Ƶ֡��

    // ״̬
    bool               m_bMakeScript;     // ������scriptTag
    bool               m_bFirstKey;       // �Ѿ������һ���ؼ�֡
    bool               m_bRun;            // ִ��״̬
    bool               m_bGotSPS;
    bool               m_bGotPPS;

    CriticalSection    m_cs;    //InputBuffer�̲߳���ȫ�������ܱ�֤�ǵ��̵߳��ã���˲���Ҫ����
};

