/**
 * 输入PES帧
 * 输出ES包
 */
#pragma once
#include "liveObj.h"

/** PES包类型 */
enum PESType
{
    pes_unkown,     //未知，此数据包异常
    pes_video,      //视频PES包
    pes_audio,      //音频PES包
    pes_jump        //需要跳过的pes包，我们的业务不需要处理这些类型
};

#pragma pack(1)
/** pes header */
typedef struct pes_header
{
    unsigned char pes_start_code_prefix[3];       // "000001"
    unsigned char stream_id;                      // pes包类型
    unsigned char PES_packet_length[2];           // 跟在该字段后的字节数目
}pes_header_t; //6

/** pes选项，若存在时，紧跟在pes header后面 */
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
 * 判断数据是否是PES包
 */
bool inline is_pes_header(pes_header_t* pes)
{
    if (pes->pes_start_code_prefix[0] == 0 && pes->pes_start_code_prefix[1] == 0 && pes->pes_start_code_prefix[2] == 1)
    {
        if (pes->stream_id == 0xC0 || pes->stream_id == 0xE0)
        {
            return true;
        }
    }
    return false;
}

typedef void(*PES_CALLBACK)(char*, long, uint64_t, uint64_t, void*);

/**
 * PES包解析类
 */
class CPes
{
public:
    CPes(void* handle);
    ~CPes(void);

    /**
     * 插入一个PES包
     * @param[in] pBuf PES帧
     * @param[in] nLen PES帧长度
     * @return 0成功 -1失败
     */
    int InputBuffer(char* pBuf, uint32_t nLen);

private:
    void*             m_hUser;                  // 回调处理对象
    PES_CALLBACK      m_fCB;
};

