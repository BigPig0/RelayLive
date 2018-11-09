#pragma once

/**
 * H264的结构：
 * 00 00 00 01/00 00 01->nal(1bytes)->slice->宏块->运动估计向量。
 * 如果h264的body中出现了前缀则由00 00 00 01/00 00 01变为00 03 00 00 01/00 03 00 01.
 */

/** H264片元类型 */
enum NalType
{
    unknow  = 0,  // 不是H264数据
    b_Nal   = 1,  // B Slice,非关键帧
    dpa_Nal = 2,
    dpb_Nal = 3,
    pdc_Nal = 4,
    idr_Nal = 5,  // IDR ,关键帧
    sei_Nal = 6,  // SEI,补充增强信息
    sps_Nal = 7,  // SPS,序列参数集
    pps_Nal = 8,  // PPS,图像参数集
    aud_Nal = 9,
    filler_Nal = 12,
    other,        // 其他类型
};

/**
 * Nal单元头结构 1个字节
 */
typedef struct nal_unit_header
{
    unsigned char nal_type : 5;     // 4-8位  这个NALU单元的类型 NalType
    unsigned char nal_ref_idc : 2;  // 2-3位  nal_ref_idc：取00~11，似乎指示这个NALU的重要性, 如00的NALU解码器可以丢弃它而不影响图像的回放。不过一般情况下不太关心这个属性
    unsigned char for_bit : 1;      // 第一位 forbidden_zero_bit：在H.264规范中规定了这一位必须为0。

}nal_unit_header_t;

class CH264
{
public:
    CH264();
    ~CH264();

    /**
     * 设置数据内容
     */
    void SetBuff(char *nal_str, uint32_t nLen);

    /** 获取类型 */
    NalType NaluType(){return m_eNaluType;}

    /** 获取数据内容位置(去除掉001或0001) */
    char* DataBuff(uint32_t& nLen){nLen=m_nDataLen;return m_pDataBuff;}

    /**
     * 解码SPS,获取视频图像宽、高信息 
     * @param width 图像宽度
     * @param height 图像高度
     * @return 成功则返回true , 失败则返回false
     */ 
    bool DecodeSps(uint32_t &width,uint32_t &height,double &fps);

private:
    /**
     * 解析数据
     */
    void ParseNalu();

    uint32_t Ue(uchar *pBuff, uint32_t nLen, uint32_t &nStartBit);

    int Se(uchar *pBuff, uint32_t nLen, uint32_t &nStartBit);

    /**
     * 按位从数据流中获取值
     * @param buf[in] 数据流
     * @param nStartBit[inout] 起始的位,计算结束移动到下个区域
     * @param BitCount[in] 值占用的位数
     * @return 指定位的数值
     */
    uint32_t u(uint32_t BitCount,uchar* buf,uint32_t &nStartBit);

    void de_emulation_prevention(uchar* buf,uint32_t* buf_size);

private:
    char*       m_pNaluBuff;    //< 数据内容
    uint32_t    m_nBuffLen;     //< 数据长度
    char*       m_pDataBuff;    //< 去除掉001或0001后的内容
    uint32_t    m_nDataLen;     //< 内容数据的长度
    NalType     m_eNaluType;    //< 类型

    /** sps中的数据 */
    int32_t     m_nWidth;
    int32_t     m_nHeight;
    double      m_nFps;
};