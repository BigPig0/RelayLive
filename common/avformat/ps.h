/**
 * ����PS֡
 * ���PES��
 */
#pragma once
#include "avtypes.h"

#pragma pack(1)
//ps header
typedef struct ps_header{
    /** 8*4 */
    unsigned char pack_start_code[4];                    // '0x000001BA'
    /** 8 */
    unsigned char system_clock_reference_base21 : 2;     // SCR 29..28
    unsigned char marker_bit : 1;                        // '1'
    unsigned char system_clock_reference_base1 : 3;      // SCR 31..30
    unsigned char fix_bit : 2;                           // '01'
    /** 8 */
    unsigned char system_clock_reference_base22;         // SCR 27..20
    /** 8 */
    unsigned char system_clock_reference_base31 : 2;     // SRC 14..13
    unsigned char marker_bit1 : 1;                       // '1'
    unsigned char system_clock_reference_base23 : 5;     // SRC 19..15
    /** 8 */
    unsigned char system_clock_reference_base32;         // SRC 12..5
    /** 8 */
    unsigned char system_clock_reference_extension1 : 2; // SRCE 8..7
    unsigned char marker_bit2 : 1;                       // '1'
    unsigned char system_clock_reference_base33 : 5;     // SRC 4..0
    /** 8 */
    unsigned char marker_bit3 : 1;                       // '1'
    unsigned char system_clock_reference_extension2 : 7; // SRCE 6..0
    /** 8 */
    unsigned char program_mux_rate1;                     // PMR 21..14
    /** 8 */
    unsigned char program_mux_rate2;                     // PMR 13..6
    unsigned char marker_bit5 : 1;
    unsigned char marker_bit4 : 1;
    unsigned char program_mux_rate3 : 6;                 // PMR 0..5
    /** 8 */
    unsigned char pack_stuffing_length : 3;
    unsigned char reserved : 5;
}ps_header_t;//14

//system header
typedef struct sh_header
{
    /** 8*4 */
    unsigned char system_header_start_code[4];          // '0x000001BB'
    /** 8*2 */
    unsigned char header_length[2];                     // ���ֶκ��ϵͳ������ֽڳ���
    /** 8*3 */
    uint32_t marker_bit1 : 1;                           //1  bslbf
    uint32_t rate_bound : 22;                           // ���ʽ����ֶ�
    uint32_t marker_bit2 : 1;                           //1 bslbf
    /** 8 */
    uint32_t audio_bound : 6;                           // ��Ƶ�����ֶ�
    uint32_t fixed_flag : 1;                            // ��'1'ʱ��ʾ�����ʺ㶨�Ĳ�������'0'ʱ����ʾ�����ı����ʿɱ䡣
    uint32_t CSPS_flag : 1;                             // ��'1'ʱ����Ŀ������2.7.9�ж��������
    /** 8 */
    uint16_t system_audio_lock_flag : 1;                // ϵͳ��Ƶ������־�ֶ�
    uint16_t system_video_lock_flag : 1;                // ϵͳ��Ƶ������־�ֶ�
    uint16_t marker_bit3 : 1;                           // bslbf
    uint16_t video_bound : 5;                           // ��Ƶ�����ֶ�
    /** 8 */
    uint16_t packet_rate_restriction_flag : 1;          // �����������Ʊ�־�ֶ�
    uint16_t reserved_bits : 7;                         // '1111111'
    /** 8*6 ��������stream_id P-STD_buffer_bound_scale P-STD_buffer_size_bound */
    unsigned char reserved[6];
}sh_header_t; //18

//program stream map header
typedef struct psm_header{
    /** 8*4 */
    unsigned char promgram_stream_map_start_code[4];   // '0x000001BC'
    /** 8*2 */
    unsigned char program_stream_map_length[2];        // �����ڸ��ֶκ��program_stream_map�е��ֽ��������ֶε����ֵΪ1018(0x3FA)
    /** 8 */
    unsigned char program_stream_map_version : 5;      // ��Ŀ��ӳ��汾�ֶ�
    unsigned char reserved1 : 2;                       // ����
    unsigned char current_next_indicator : 1;          // ��'1'ʱ��ʾ���͵Ľ�Ŀ��ӳ�䵱ǰ�ǿ��õġ���'0'ʱ��ʾ���͵Ľ�Ŀ��ӳ�仹�����ã�����������һ����Ч�ı�
    /** 8 */
    unsigned char marker_bit : 1;
    unsigned char reserved2 : 7;
    /** 8*2 */
    unsigned char program_stream_info_length[2];       // ��Ŀ����Ϣ�����ֶ�
    /** 8*2 */
    unsigned char elementary_stream_map_length[2];     // ������ӳ�䳤���ֶ�
    /** 8*4 �⼸��Ϊһ�飬��elementary_stream_map_lengthȷ���м��� */
    unsigned char stream_type;
    unsigned char elementary_stream_id;
    unsigned char elementary_stream_info_length[2];
    /** 8*4 */
    unsigned char CRC_32[4];
    /** 8*16 */
    unsigned char reserved[16];
}psm_header_t; //36
#pragma pack()

/** �ж������Ƿ���PSͷ */
bool inline is_ps_header(ps_header_t* ps)
{
    if (ps->pack_start_code[0] == 0 && ps->pack_start_code[1] == 0 && ps->pack_start_code[2] == 1 && ps->pack_start_code[3] == 0xBA)
        return true;
    return false;
}

/** �ж������Ƿ���system header */
bool inline is_sh_header(sh_header_t* sh)
{
    if (sh->system_header_start_code[0] == 0 && sh->system_header_start_code[1] == 0 && sh->system_header_start_code[2] == 1 && sh->system_header_start_code[3] == 0xBB)
        return true;
    return false;
}

/** �ж������Ƿ���program stream map header */
bool inline is_psm_header(psm_header_t* psm)
{
    if (psm->promgram_stream_map_start_code[0] == 0 && psm->promgram_stream_map_start_code[1] == 0 && psm->promgram_stream_map_start_code[2] == 1 && psm->promgram_stream_map_start_code[3] == 0xBC)
        return true;
    return false;
}

class CPs
{
public:
    CPs(AV_CALLBACK cb, void* handle=NULL);
    ~CPs(void);

    /**
     * ����һ��PS֡
     * @param[in] buff.pBuf PS֡
     * @param[in] buff.nLen PS֡����
     * @return 0�ɹ� -1ʧ��
     */
    int DeCode(AV_BUFF buff);

private:
    /**
     * ��������PSͷ��
     * @param[in] pBuf PS֡
     * @param[in] nLen PS֡����
     * @param[out] nHeadLen ͷ�ĳ���
     * @return 0�ɹ� -1ʧ��
     */
    int ParseHeader(char* pBuf, uint32_t nLen, uint32_t& nHeadLen);

    /**
     * �ӱ����н���PES
     * @param[in] pBuf ����ͷ��������λ��
     * @param[in] nLen ��ȥͷ���Ժ�ĳ���
     * @return 0�ɹ� -1ʧ��
     */
    int ParsePES(char* pBuf, uint32_t nLen);

private:
    void*             m_hUser;                  // �ص��������
    AV_CALLBACK       m_fCB;
};

