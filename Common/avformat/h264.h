#pragma once
#include "avtypes.h"
#include "NetStreamMaker.h"

/**
 * H264的结构：
 * startcode(00 00 00 01/00 00 01)->nal(1bytes)->slice->宏块->运动估计向量。
 * 如果h264的body中出现了startcode则由00 00 00 01/00 00 01变为00 03 00 00 01/00 03 00 01.
 *
 * h264有两种封装，
 * 一种是annexb模式，传统模式，有startcode(0001/001)，SPS和PPS是在ES中
 * 一种是mp4模式，一般mp4 mkv会有，没有startcode(0001/001)，SPS和PPS以及其它信息被封装在container中，每一个frame前面是这个frame的长度
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

/**
 *Nal分片单元结构 2个字节
 */
typedef struct nal_unit_header_rtp {
	unsigned char ex_type : 5;      // 4-8位  这个NALU单元的类型 FU_A
    unsigned char nal_ref_idc : 2;  // 2-3位  nal_ref_idc：取00~11，似乎指示这个NALU的重要性, 如00的NALU解码器可以丢弃它而不影响图像的回放。不过一般情况下不太关心这个属性
    unsigned char for_bit : 1;      // 第一位 forbidden_zero_bit：在H.264规范中规定了这一位必须为0。

	unsigned char nal_type : 5;     // 4-8位  这个NALU单元的类型 NalType
	unsigned char R : 1;            //
	unsigned char E : 1;            //
	unsigned char S : 1;
}nal_unit_header_rtp_t;

/**
 * 获取nalu位置，即如果存在startcode，输出去除startcode后的位置
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
 * 判断是否是Nal分片单元
 * @param buff 不包含startcode(0001/001)的nalu
 */
bool inline is_h264_slice(char* buff) {
	nal_unit_header* pNalUnit = (nal_unit_header*)buff;
	if(pNalUnit->nal_type == NalType::FU_A) {
		return true;
	}
	return false;
}

/**
 *判断是否是h264码流
 * @param buff 不包含startcode(0001/001)的nalu
 * @note 如果是分片单元，只有起始分片才返回true。非分片返回true
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
 * 判断是否是h264最后分片
 * @param buff 不包含startcode(0001/001)的nalu
 * @note 如果是分片单元，只有结束分片才返回true。非分片返回true
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
 * 获取nalu的类型
 * @param buff 不包含startcode(0001/001)的nalu
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
 * 解析sps得到码流的一些信息
 * @param buff 输入sps nalu
 * @param len 输入sps nalu的长度
 * @param width 输出宽度信息
 * @param height 输出高度信息
 * @param fps 输出fps
 */
bool h264_sps_info(char *buff, uint32_t len, uint32_t *width, uint32_t *height, double *fps);

class CH264
{
public:
    CH264(AV_CALLBACK cb, void* handle=NULL);
    ~CH264();

    /**
     * 设置数据内容
     */
    int Code(AV_BUFF buff);

    /** 获取类型 */
    NalType NaluType(){return m_eNaluType;}

    /** 获取数据内容位置(去除掉001或0001) */
    char* DataBuff(uint32_t& nLen){nLen=m_nDataLen;return m_pDataBuff;}

    void SetNodelay(uint32_t nodelay){m_nNodelay = nodelay;};

private:
    /**
     * 生成一个视频断并上抛
     */
    bool MakeVideo(char *data,int size,int bIsKeyFrame);

	/* 生成关键帧 */
	bool MakeKeyVideo();

private:
    CNetStreamMaker    *m_pSPS;            // 缓存SPS
    CNetStreamMaker    *m_pPPS;            // 缓存PPS
    CNetStreamMaker    *m_pKeyFrame;       // 缓存关键帧，sps和pps有可能在后面
    CNetStreamMaker    *m_pData;           // 缓存h264数据 7 8 5 1 1 1 1 ... 1

    uint32_t           m_nNodelay;         // 是否立即发送
    void*              m_hUser;             // 回调处理对象
    AV_CALLBACK        m_fCB;

    bool               m_bFirstKey;        // 已经处理第一个关键帧
    bool               m_bGotSPS;
    bool               m_bGotPPS;
    
};