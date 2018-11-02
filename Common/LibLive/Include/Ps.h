/**
 * 输入PS帧
 * 输出PES包
 */
#pragma once
#include "LiveInstance.h"

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
    unsigned char header_length[2];                     // 该字段后的系统标题的字节长度
    /** 8*3 */
    uint32_t marker_bit1 : 1;                           //1  bslbf
    uint32_t rate_bound : 22;                           // 速率界限字段
    uint32_t marker_bit2 : 1;                           //1 bslbf
    /** 8 */
    uint32_t audio_bound : 6;                           // 音频界限字段
    uint32_t fixed_flag : 1;                            // 置'1'时表示比特率恒定的操作；置'0'时，表示操作的比特率可变。
    uint32_t CSPS_flag : 1;                             // 置'1'时，节目流符合2.7.9中定义的限制
    /** 8 */
    uint16_t system_audio_lock_flag : 1;                // 系统音频锁定标志字段
    uint16_t system_video_lock_flag : 1;                // 系统视频锁定标志字段
    uint16_t marker_bit3 : 1;                           // bslbf
    uint16_t video_bound : 5;                           // 视频界限字段
    /** 8 */
    uint16_t packet_rate_restriction_flag : 1;          // 分组速率限制标志字段
    uint16_t reserved_bits : 7;                         // '1111111'
    /** 8*6 不定长，stream_id P-STD_buffer_bound_scale P-STD_buffer_size_bound */
    unsigned char reserved[6];
}sh_header_t; //18

//program stream map header
typedef struct psm_header{
    /** 8*4 */
    unsigned char promgram_stream_map_start_code[4];   // '0x000001BC'
    /** 8*2 */
    unsigned char program_stream_map_length[2];        // 紧跟在该字段后的program_stream_map中的字节数。该字段的最大值为1018(0x3FA)
    /** 8 */
    unsigned char program_stream_map_version : 5;      // 节目流映射版本字段
    unsigned char reserved1 : 2;                       // 保留
    unsigned char current_next_indicator : 1;          // 置'1'时表示传送的节目流映射当前是可用的。置'0'时表示传送的节目流映射还不可用，但它将是下一个生效的表
    /** 8 */
    unsigned char marker_bit : 1;
    unsigned char reserved2 : 7;
    /** 8*2 */
    unsigned char program_stream_info_length[2];       // 节目流信息长度字段
    /** 8*2 */
    unsigned char elementary_stream_map_length[2];     // 基本流映射长度字段
    /** 8*4 这几个为一组，由elementary_stream_map_length确定有几组 */
    unsigned char stream_type;
    unsigned char elementary_stream_id;
    unsigned char elementary_stream_info_length[2];
    /** 8*4 */
    unsigned char CRC_32[4];
    /** 8*16 */
    unsigned char reserved[16];
}psm_header_t; //36
#pragma pack()

/** 判断数据是否是PS头 */
bool inline is_ps_header(ps_header_t* ps)
{
    if (ps->pack_start_code[0] == 0 && ps->pack_start_code[1] == 0 && ps->pack_start_code[2] == 1 && ps->pack_start_code[3] == 0xBA)
        return true;
    return false;
}

/** 判断数据是否是system header */
bool inline is_sh_header(sh_header_t* sh)
{
    if (sh->system_header_start_code[0] == 0 && sh->system_header_start_code[1] == 0 && sh->system_header_start_code[2] == 1 && sh->system_header_start_code[3] == 0xBB)
        return true;
    return false;
}

/** 判断数据是否是program stream map header */
bool inline is_psm_header(psm_header_t* psm)
{
    if (psm->promgram_stream_map_start_code[0] == 0 && psm->promgram_stream_map_start_code[1] == 0 && psm->promgram_stream_map_start_code[2] == 1 && psm->promgram_stream_map_start_code[3] == 0xBC)
        return true;
    return false;
}

class CPs : public IAnalyzer
{
public:
    CPs(CLiveInstance* pObj);
    ~CPs(void);

    /**
     * 插入一个PS帧
     * @param[in] pBuf PS帧
     * @param[in] nLen PS帧长度
     * @return 0成功 -1失败
     */
    int InputBuffer(char* pBuf, long nLen);

private:
    /**
     * 解析报文PS头部
     * @param[in] pBuf PS帧
     * @param[in] nLen PS帧长度
     * @param[out] nHeadLen 头的长度
     * @return 0成功 -1失败
     */
    int ParseHeader(char* pBuf, long nLen, long& nHeadLen);

    /**
     * 从报文中解析PES
     * @param[in] pBuf 跳过头部的数据位置
     * @param[in] nLen 除去头部以后的长度
     * @return 0成功 -1失败
     */
    int ParsePES(char* pBuf, long nLen);

private:
    CLiveInstance*   m_pObj;                  // 回调处理对象
};

