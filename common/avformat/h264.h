#pragma once
#include "avtypes.h"
#include "NetStreamMaker.h"

/**
 * H264�Ľṹ��
 * startcode(00 00 00 01/00 00 01)->nal(1bytes)->slice->���->�˶�����������
 * ���h264��body�г�����startcode����00 00 00 01/00 00 01��Ϊ00 03 00 00 01/00 03 00 01.
 *
 * h264�����ַ�װ��
 * һ����annexbģʽ����ͳģʽ����startcode(0001/001)��SPS��PPS����ES��
 * һ����mp4ģʽ��һ��mp4 mkv���У�û��startcode(0001/001)��SPS��PPS�Լ�������Ϣ����װ��container�У�ÿһ��frameǰ�������frame�ĳ���
 */

/** H264ƬԪ���� */
enum NalType
{
    unknow  = 0,  // ����H264����
    b_Nal   = 1,  // B Slice,�ǹؼ�֡
    dpa_Nal = 2,
    dpb_Nal = 3,
    pdc_Nal = 4,
    idr_Nal = 5,  // IDR ,�ؼ�֡
    sei_Nal = 6,  // SEI,������ǿ��Ϣ
    sps_Nal = 7,  // SPS,���в�����
    pps_Nal = 8,  // PPS,ͼ�������
    aud_Nal = 9,
    filler_Nal = 12,
    other,        // ��������
	reserve_end = 23,
	STAP_A = 24,  //24 ��һʱ�����ϰ�
	STAP_B,       //25 ��һʱ�����ϰ�
	MTAP16,       //26 ���ʱ�����ϰ�
	MTAP24,       //27 ���ʱ�����ϰ�
	FU_A,         //28 ��Ƭ�ĵ�Ԫ
	FU_B,         //29 ��Ƭ�ĵ�Ԫ
};

/**
 * Nal��Ԫͷ�ṹ 1���ֽ�
 */
typedef struct nal_unit_header
{
    unsigned char nal_type : 5;     // 4-8λ  ���NALU��Ԫ������ NalType
    unsigned char nal_ref_idc : 2;  // 2-3λ  nal_ref_idc��ȡ00~11���ƺ�ָʾ���NALU����Ҫ��, ��00��NALU���������Զ���������Ӱ��ͼ��Ļطš�����һ������²�̫�����������
    unsigned char for_bit : 1;      // ��һλ forbidden_zero_bit����H.264�淶�й涨����һλ����Ϊ0��

}nal_unit_header_t;

/**
 *Nal��Ƭ��Ԫ�ṹ 2���ֽ�
 */
typedef struct nal_unit_header_rtp {
	unsigned char ex_type : 5;      // 4-8λ  ���NALU��Ԫ������ FU_A
    unsigned char nal_ref_idc : 2;  // 2-3λ  nal_ref_idc��ȡ00~11���ƺ�ָʾ���NALU����Ҫ��, ��00��NALU���������Զ���������Ӱ��ͼ��Ļطš�����һ������²�̫�����������
    unsigned char for_bit : 1;      // ��һλ forbidden_zero_bit����H.264�淶�й涨����һλ����Ϊ0��

	unsigned char nal_type : 5;     // 4-8λ  ���NALU��Ԫ������ NalType
	unsigned char R : 1;            //
	unsigned char E : 1;            //
	unsigned char S : 1;
}nal_unit_header_rtp_t;

/**
 * ��ȡnaluλ�ã����������startcode�����ȥ��startcode���λ��
 */
bool inline h264_nalu_data(char *buff, char **nalu) {
	if(buff[0]==0 && buff[1]==0 && buff[2]==0 && buff[3]==1) {
		*nalu = buff+4;
		return true;
	} else if (buff[0]==0 && buff[1]==0 && buff[2]==1) {
		*nalu = buff+3;
		return true;
	} 
	*nalu = buff;
	return false;
}

bool inline h264_nalu_data2(char *buff, uint32_t len, char **nalu, uint32_t *nalu_len) {
	if(len > 3 &&buff[0]==0 && buff[1]==0 && buff[2]==0 && buff[3]==1) {
		*nalu = buff+4;
		*nalu_len = len-4;
		return true;
	} else if (len > 2 && buff[0]==0 && buff[1]==0 && buff[2]==1) {
		*nalu = buff+3;
		*nalu_len = len-3;
		return true;
	} 
	*nalu = buff;
	*nalu_len = len;
	return false;
}

/**
 * �ж��Ƿ���Nal��Ƭ��Ԫ
 * @param buff ������startcode(0001/001)��nalu
 */
bool inline is_h264_slice(char* buff) {
	nal_unit_header* pNalUnit = (nal_unit_header*)buff;
	if(pNalUnit->nal_type == NalType::FU_A) {
		return true;
	}
	return false;
}

/**
 *�ж��Ƿ���h264����
 * @param buff ������startcode(0001/001)��nalu
 * @note ����Ƿ�Ƭ��Ԫ��ֻ����ʼ��Ƭ�ŷ���true���Ƿ�Ƭ����true
 */
bool inline is_h264_header(char* buff) {
	nal_unit_header* pNalUnit = (nal_unit_header*)buff;
	bool s = 1;
	NalType nal = (NalType)pNalUnit->nal_type;
	if(NalType::FU_A == pNalUnit->nal_type) {
		s = 0;
		nal_unit_header_rtp_t* pNalUnit2 = (nal_unit_header_rtp_t*)buff;
		nal = (NalType)pNalUnit2->nal_type;
		s = pNalUnit2->S;
	}
    if ( NalType::sps_Nal == nal
      || NalType::pps_Nal == nal
      || NalType::sei_Nal == nal
      || NalType::idr_Nal == nal
      || NalType::b_Nal   == nal) {
		  if(s)
			return true;
	}
	return false;
}

/**
 * �ж��Ƿ���h264����Ƭ
 * @param buff ������startcode(0001/001)��nalu
 * @note ����Ƿ�Ƭ��Ԫ��ֻ�н�����Ƭ�ŷ���true���Ƿ�Ƭ����true
 */
bool inline is_h264_end(char* buff) {
	nal_unit_header* pNalUnit = (nal_unit_header*)buff;
	bool e = 1;
	NalType nal = (NalType)pNalUnit->nal_type;
	if(NalType::FU_A == pNalUnit->nal_type) {
		e = 0;
		nal_unit_header_rtp_t* pNalUnit2 = (nal_unit_header_rtp_t*)buff;
		nal = (NalType)pNalUnit2->nal_type;
		e = pNalUnit2->E;
	}
    if ( NalType::sps_Nal == nal
      || NalType::pps_Nal == nal
      || NalType::sei_Nal == nal
      || NalType::idr_Nal == nal
      || NalType::b_Nal   == nal) {
		  if(e)
			return true;
	}
	return false;
}

/**
 * ��ȡnalu������
 * @param buff ������startcode(0001/001)��nalu
 */
NalType inline h264_naltype(char* buff) {
	nal_unit_header* pNalUnit = (nal_unit_header*)buff;
	NalType nal = (NalType)pNalUnit->nal_type;
	if(NalType::FU_A == pNalUnit->nal_type) {
		nal_unit_header_rtp_t* pNalUnit2 = (nal_unit_header_rtp_t*)buff;
		nal = (NalType)pNalUnit2->nal_type;
	}
	return nal;
}

/**
 * ����sps�õ�������һЩ��Ϣ
 * @param buff ����sps nalu
 * @param len ����sps nalu�ĳ���
 * @param width ��������Ϣ
 * @param height ����߶���Ϣ
 * @param fps ���fps
 */
bool h264_sps_info(char *buff, uint32_t len, uint32_t *width, uint32_t *height, double *fps);

class CH264
{
public:
    CH264(AV_CALLBACK cb, void* handle=NULL);
    ~CH264();

    /**
     * ������������
     */
    int Code(AV_BUFF buff);

    void SetNodelay(uint32_t nodelay){m_nNodelay = nodelay;};

private:
    /**
     * ����һ����Ƶ�ϲ�����
     */
    bool MakeVideo(char *data,int size,int bIsKeyFrame);

	/* ���ɹؼ�֡ */
	bool MakeKeyVideo();

private:
    CNetStreamMaker    *m_pSPS;            // ����SPS
    CNetStreamMaker    *m_pPPS;            // ����PPS
    CNetStreamMaker    *m_pKeyFrame;       // ����ؼ�֡��sps��pps�п����ں���
    CNetStreamMaker    *m_pData;           // ����h264���� 7 8 5 1 1 1 1 ... 1

    uint32_t           m_nNodelay;         // �Ƿ���������
    void*              m_hUser;             // �ص��������
    AV_CALLBACK        m_fCB;

    bool               m_bFirstKey;        // �Ѿ������һ���ؼ�֡
    bool               m_bGotSPS;
    bool               m_bGotPPS;
    
};