/**
 * ����PES֡
 * ���ES��
 */
#pragma once
#include "avtypes.h"

/** PES������ */
enum PESType
{
    pes_unkown,     //δ֪�������ݰ��쳣
    pes_video,      //��ƵPES��
    pes_audio,      //��ƵPES��
    pes_jump        //��Ҫ������pes�������ǵ�ҵ����Ҫ������Щ����
};

#pragma pack(1)
/** pes header */
typedef struct pes_header
{
    unsigned char pes_start_code_prefix[3];       // "000001"
    unsigned char stream_id;                      // pes������
    unsigned char PES_packet_length[2];           // ���ڸ��ֶκ���ֽ���Ŀ
}pes_header_t; //6

/** pesѡ�������ʱ��������pes header���� */
typedef struct optional_pes_header{
    unsigned char original_or_copy : 1;
    unsigned char copyright : 1;
    unsigned char data_alignment_indicator : 1;
    unsigned char PES_priority : 1;
    unsigned char PES_scrambling_control : 2;
    unsigned char fix_bit : 2;

    unsigned char PES_extension_flag : 1;
    unsigned char PES_CRC_flag : 1;
    unsigned char additional_copy_info_flag : 1;
    unsigned char DSM_trick_mode_flag : 1;
    unsigned char ES_rate_flag : 1;
    unsigned char ESCR_flag : 1;
    unsigned char PTS_DTS_flags : 2;

    unsigned char PES_header_data_length;
}optional_pes_header_t; // 3
#pragma pack()

typedef struct pes_status_data{
    bool pesTrued;
    PESType pesType;
    pes_header_t *pesPtr;
}pes_status_data_t;

/**
 * �ж������Ƿ���PES��
 */
bool inline is_pes_header(pes_header_t* pes)
{
    if (pes->pes_start_code_prefix[0] == 0 && pes->pes_start_code_prefix[1] == 0 && pes->pes_start_code_prefix[2] == 1)
    {
        if (pes->stream_id == 0xC0 || pes->stream_id == 0xE0
			|| pes->stream_id == 0xBC || pes->stream_id == 0xBD || pes->stream_id == 0xBE || pes->stream_id == 0xBF 
			|| pes->stream_id == 0xF0 || pes->stream_id == 0xF1 || pes->stream_id == 0xF2 || pes->stream_id == 0xF8)
        {
            return true;
        }
    }
    return false;
}


/**
 * PES��������
 */
class CPes
{
public:
    CPes(AV_CALLBACK cb, void* handle=NULL);
    ~CPes(void);

    /**
     * PES����
     * @param[in] buff.pData PES֡
     * @param[in] buff.nLen PES֡����
     * @return 0�ɹ� -1ʧ��
     */
    int Decode(AV_BUFF buff);

private:
    void*             m_hUser;                  // �ص��������
    AV_CALLBACK       m_fCB;
};

