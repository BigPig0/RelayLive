#pragma once
#include <stdint.h>

typedef enum _AV_TYPE_
{
    NONE,
    RTP,
    RTCP,
    PS,
    TS,
    PES,
    ES,
    H264_NALU,
	H264_IDR,
	H264_NDR,
    YUV,
    FLV_HEAD,
    FLV_FRAG_KEY,
    FLV_FRAG,
    MP4_HEAD,
    MP4_FRAG_KEY,
    MP4_FRAG
}AV_TYPE;

/**
 * һ֡����Ƶ���ݵ�����
 */
typedef struct _AV_BUFF_ {
    AV_TYPE     eType;    //< ���ݰ�����
    char       *pData;    //< ��������
    uint32_t    nLen;     //< ���ݳ���
    uint64_t    m_pts;    //< ��ʾʱ���
    uint64_t    m_dts;    //< ����ʱ���
}AV_BUFF;

/** ��Ƶ�ص����� */
typedef void (*AV_CALLBACK)(AV_BUFF, void*);