#pragma once

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
    FLV_HEAD,
    FLV_FRAG_KEY,
    FLV_FRAG,
    MP4_HEAD,
    MP4_FRAG_KEY,
    MP4_FRAG
}AV_TYPE;

/**
 * 一帧视音频数据的内容
 */
typedef struct _AV_BUFF_ {
    AV_TYPE   eType;    //< 数据包类型
    char      *pData;   //< 数据内容
    uint32_t  nLen;     //< 数据长度
}AV_BUFF;

/** 视频回调方法 */
typedef void (*AV_CALLBACK)(AV_BUFF, void*);