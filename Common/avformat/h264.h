#pragma once
#include "avtypes.h"
#include "NetStreamMaker.h"

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
	reserve_end = 23,
	STAP_A = 24,  //24 单一时间的组合包
	STAP_B,       //25 单一时间的组合包
	MTAP16,       //26 多个时间的组合包
	MTAP24,       //27 多个时间的组合包
	FU_A,         //28 分片的单元
	FU_B,         //29 分片的单元
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

typedef struct nal_unit_header_rtp {
	unsigned char ex_type : 5;      // 4-8位  这个NALU单元的类型 FU_A
    unsigned char nal_ref_idc : 2;  // 2-3位  nal_ref_idc：取00~11，似乎指示这个NALU的重要性, 如00的NALU解码器可以丢弃它而不影响图像的回放。不过一般情况下不太关心这个属性
    unsigned char for_bit : 1;      // 第一位 forbidden_zero_bit：在H.264规范中规定了这一位必须为0。

	unsigned char nal_type : 5;     // 4-8位  这个NALU单元的类型 NalType
	unsigned char R : 1;            //
	unsigned char E : 1;            //
	unsigned char S : 1;
}nal_unit_header_rtp_t;

bool inline is_h264_slice(char* buff) {
	nal_unit_header* pNalUnit = (nal_unit_header*)buff;
	if(pNalUnit->nal_type == NalType::FU_A) {
		return true;
	}
	return false;
}

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

NalType inline h264_naltype(char* buff) {
	nal_unit_header* pNalUnit = (nal_unit_header*)buff;
	NalType nal = (NalType)pNalUnit->nal_type;
	if(NalType::FU_A == pNalUnit->nal_type) {
		nal_unit_header_rtp_t* pNalUnit2 = (nal_unit_header_rtp_t*)buff;
		nal = (NalType)pNalUnit2->nal_type;
	}
	return nal;
}

typedef void (*H264SPS_CALLBACK)(uint32_t, uint32_t, double, void*);

class CH264
{
public:
    CH264(H264SPS_CALLBACK spscb, AV_CALLBACK cb, void* handle=NULL);
    ~CH264();

    /**
     * 设置数据内容
     */
    int InputBuffer(char *pBuf, uint32_t nLen);

    /** 获取类型 */
    NalType NaluType(){return m_eNaluType;}

    /** 获取数据内容位置(去除掉001或0001) */
    char* DataBuff(uint32_t& nLen){nLen=m_nDataLen;return m_pDataBuff;}

    /** 获取sps解析得到的配置信息 */
    uint32_t Width(){return m_nWidth;}
    uint32_t Height(){return m_nHeight;}
    double Fps(){return m_nFps;}

private:
    /**
     * 解析数据
     */
    void ParseNalu();

    /**
     * 解码SPS,获取视频图像宽、高信息 
     * @return 成功则返回true , 失败则返回false
     */ 
    bool DecodeSps();

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

    /**
     * 数据还原，内容中的0031、00031还原位001、0001
     */
    void de_emulation_prevention(uchar* buf,uint32_t* buf_size);

private:
    char*       m_pNaluBuff;    //< 数据内容
    uint32_t    m_nBuffLen;     //< 数据长度
    char*       m_pDataBuff;    //< 去除掉001或0001后的内容
    uint32_t    m_nDataLen;     //< 内容数据的长度
    NalType     m_eNaluType;    //< 类型

    CNetStreamMaker    *m_pSPS;            // 缓存SPS
    CNetStreamMaker    *m_pPPS;            // 缓存PPS
    CNetStreamMaker    *m_pFullBuff;       // 缓存h264数据 7 8 5 1 1 1 1 ... 1
    bool               m_bFirstKey;        // 已经处理第一个关键帧
    bool               m_bDecode;          // 是否已经解析sps
    
    void*             m_hUser;                  // 回调处理对象
    H264SPS_CALLBACK  m_fCBSPS;
    AV_CALLBACK       m_fCB;

    /** sps中的数据 */
    int32_t     m_nWidth;
    int32_t     m_nHeight;
    double      m_nFps;
};