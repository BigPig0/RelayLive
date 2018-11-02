#include "stdafx.h"
#include "Flv.h"
#include "H264.h"

/* offsets for packed values */
#define FLV_AUDIO_SAMPLESSIZE_OFFSET 1
#define FLV_AUDIO_SAMPLERATE_OFFSET  2
#define FLV_AUDIO_CODECID_OFFSET     4

#define FLV_VIDEO_FRAMETYPE_OFFSET   4

/* bitmasks to isolate specific values */
#define FLV_AUDIO_CHANNEL_MASK    0x01
#define FLV_AUDIO_SAMPLESIZE_MASK 0x02
#define FLV_AUDIO_SAMPLERATE_MASK 0x0c
#define FLV_AUDIO_CODECID_MASK    0xf0

#define FLV_VIDEO_CODECID_MASK    0x0f
#define FLV_VIDEO_FRAMETYPE_MASK  0xf0

#define AMF_END_OF_OBJECT         0x09

enum
{
    FLV_HEADER_FLAG_HASVIDEO = 1,
    FLV_HEADER_FLAG_HASAUDIO = 4,
};

enum
{
    FLV_TAG_TYPE_AUDIO = 0x08,
    FLV_TAG_TYPE_VIDEO = 0x09,
    FLV_TAG_TYPE_META  = 0x12,
};

enum
{
    FLV_MONO   = 0,
    FLV_STEREO = 1,
};

enum
{
    FLV_SAMPLESSIZE_8BIT  = 0,
    FLV_SAMPLESSIZE_16BIT = 1 << FLV_AUDIO_SAMPLESSIZE_OFFSET,
};

enum
{
    FLV_SAMPLERATE_SPECIAL = 0, /**< signifies 5512Hz and 8000Hz in the case of NELLYMOSER */
    FLV_SAMPLERATE_11025HZ = 1 << FLV_AUDIO_SAMPLERATE_OFFSET,
    FLV_SAMPLERATE_22050HZ = 2 << FLV_AUDIO_SAMPLERATE_OFFSET,
    FLV_SAMPLERATE_44100HZ = 3 << FLV_AUDIO_SAMPLERATE_OFFSET,
};

enum
{
    FLV_CODECID_MP3 = 2 << FLV_AUDIO_CODECID_OFFSET,
    FLV_CODECID_AAC = 10<< FLV_AUDIO_CODECID_OFFSET,
};

enum
{
    FLV_CODECID_H264 = 7,
};

enum
{
    FLV_FRAME_KEY   = 1 << FLV_VIDEO_FRAMETYPE_OFFSET | 7,
    FLV_FRAME_INTER = 2 << FLV_VIDEO_FRAMETYPE_OFFSET | 7,
};

typedef enum
{
    AMF_DATA_TYPE_NUMBER      = 0x00,
    AMF_DATA_TYPE_BOOL        = 0x01,
    AMF_DATA_TYPE_STRING      = 0x02,
    AMF_DATA_TYPE_OBJECT      = 0x03,
    AMF_DATA_TYPE_NULL        = 0x05,
    AMF_DATA_TYPE_UNDEFINED   = 0x06,
    AMF_DATA_TYPE_REFERENCE   = 0x07,
    AMF_DATA_TYPE_MIXEDARRAY  = 0x08,
    AMF_DATA_TYPE_OBJECT_END  = 0x09,
    AMF_DATA_TYPE_ARRAY       = 0x0a,
    AMF_DATA_TYPE_DATE        = 0x0b,
    AMF_DATA_TYPE_LONG_STRING = 0x0c,
    AMF_DATA_TYPE_UNSUPPORTED = 0x0d,
} AMFDataType;


/* Put functions  */

void flv_buffer::append_amf_string(const char *str)
{
    uint16_t len = strlen( str );
    append_be16( len );
    append_data( (char*)str, len );
}

void flv_buffer::append_amf_double(double d)
{
    append_byte( AMF_DATA_TYPE_NUMBER );
    append_double(d);
}

CFlv::CFlv(void)
    : m_pSPS(nullptr)
    , m_pPPS(nullptr)
    , m_pData(nullptr)
    , m_timestamp(0)
    , m_tick_gap(0)
    , m_funCallBack(nullptr)
    , m_nWidth(0)
    , m_nHeight(0)
    , m_nfps(25)
    , m_bMakeScript(false)
    , m_bFirstKey(false)
{
    m_pSPS = new flv_buffer();
    m_pPPS = new flv_buffer();
    m_pData = new flv_buffer();
    m_bRun = true;
}

CFlv::~CFlv(void)
{
    m_bRun = false;
    SAFE_DELETE(m_pSPS);
    SAFE_DELETE(m_pPPS);
    SAFE_DELETE(m_pData);
}

int CFlv::InputBuffer(char* pBuf, long nLen)
{
    if(!m_bRun)
    {
        Log::error("already stop");
        return false;
    }
    MutexLock lock(&m_cs);

    CH264 nalu;
    nalu.SetBuff(pBuf, nLen);
    NalType type = nalu.NaluType();
    uint32_t nDataLen = 0;
    char* pData = nalu.DataBuff(nDataLen);
    switch (type)
    {
    case b_Nal:
        // 发送非关键帧
        if(!m_bFirstKey)
            break;
        //Log::debug("send frame");
        MakeVideoH264Tag(pData,nDataLen,0);
        m_timestamp += m_tick_gap;
        break;
    case idr_Nal:
        // 第一个tag是scriptTag
        //Log::debug("send scriptTag");
        if(!m_bMakeScript)
        {    
            // Script Tag
            if(!MakeScriptTag())
                break;
            // AVCDecoderConfigurationRecord, this is the first video tag
            if(!MakeVideoH264HeaderTag())
                break;

            m_bMakeScript = true;
        }
        // 发送关键帧
        Log::debug("send key frame");
        MakeVideoH264Tag(pData,nDataLen,1);
        m_timestamp += m_tick_gap;
        m_bFirstKey = true;
        break;
    case sei_Nal:
        break;
    case sps_Nal:
        {
            if(m_pSPS == nullptr)
            {
                Log::error("m_pSPS is null");
                return false;
            }

            //Log::debug("save sps size:%d",nLen);
            m_pSPS->clear();
            m_pSPS->append_data(pBuf, nLen);
        }
        break;
    case pps_Nal:
        {
            if (m_pPPS == nullptr)
            {
                Log::error("m_pPPS is null");
                return false;
            }

            //Log::debug("save pps size:%d",nLen);
            m_pPPS->clear();
            m_pPPS->append_data(pBuf, nLen);
        }
        break;
    case other:
    case unknow:
    default:
		Log::warning("h264 nal type: %d", type);
        break;
    }
    return true;
}

void CFlv::SetCallBack(cbFunc cb)
{
    m_funCallBack = cb;
}

bool CFlv::MakeHeader(char** ppBuff, int* pLen)
{
    flv_buffer* c = new flv_buffer();
    c->append_string("FLV"); // Signature
    c->append_byte(1);    // Version
    c->append_byte(1);    // Video Only
    c->append_be32(9);    // DataOffset
    c->append_be32(0);    // PreviousTagSize0

    *ppBuff = (char*)malloc(c->size());
    if (ppBuff != nullptr)
    {
        *pLen = c->size();
        memcpy(*ppBuff, c->get(), *pLen);

        SAFE_DELETE(c);
        return true;
    }

    SAFE_DELETE(c);
    return false;
}

bool CFlv::MakeScriptTag()
{
    flv_buffer* c = (flv_buffer*)m_pData;
    flv_buffer* sps = (flv_buffer*)m_pSPS;
    if (c == nullptr || sps == nullptr || sps->get() == nullptr)
    {
        Log::error("m_pData:%d,m_pSPS:%d",m_pData,m_pSPS);
        return false;
    }
    if (m_funCallBack == nullptr)
    {
        Log::error("m_funCallBack is null");
        return false;
    }

    if(!m_bMakeScript)
    {
        CH264 nalu;
        nalu.SetBuff(sps->get(), sps->size());
        if(false == nalu.DecodeSps(m_nWidth,m_nHeight,m_nfps))
        {
            Log::error("解码sps失败");
            return false;
        }
        m_tick_gap = 1000/(m_nfps>0?m_nfps:25);
        Log::debug("width = %d,height = %d, fps= %lf, tickgap= %d",m_nWidth,m_nHeight,m_nfps,m_tick_gap);
    }

    c->clear();
    c->append_byte( FLV_TAG_TYPE_META ); // Tag Type "script data"

    int nDataLenPos = c->size();
    c->append_be24( 0 ); // data length
    c->append_be24( 0 ); // timestamp 对于脚本型的tag总是0
    c->append_be32( 0 ); // reserved (timestampextended streamID)

    int nDataLenBegin = c->size();
    c->append_byte( AMF_DATA_TYPE_STRING );
    c->append_amf_string( "onMetaData" );

    c->append_byte( AMF_DATA_TYPE_MIXEDARRAY ); //meta元素个数，下面填写几个，这里就是几
    c->append_be32( 8 );

    c->append_amf_string( "duration" );
    c->append_amf_double( 0 ); // 时长

    c->append_amf_string( "width" );
    c->append_amf_double( m_nWidth );

    c->append_amf_string( "height" );
    c->append_amf_double( m_nHeight );

    c->append_amf_string( "videodatarate" );
    c->append_amf_double( 0 ); // 码率

    c->append_amf_string( "framerate" );
    c->append_amf_double( m_nfps );

    c->append_amf_string( "videocodecid" );
    c->append_amf_double( FLV_CODECID_H264 );

    c->append_amf_string( "encoder" );
    c->append_byte( AMF_DATA_TYPE_STRING );
    c->append_amf_string( "Lavf55.37.102" );

    c->append_amf_string( "filesize" );
    c->append_amf_double( 0 ); // 文件大小

    c->append_amf_string( "" );
    c->append_byte( AMF_END_OF_OBJECT );

    unsigned nDataLen = c->size() - nDataLenBegin;
    c->rewrite_be24( nDataLenPos, nDataLen ); // 重写 data length

    c->append_be32( nDataLen + 11 ); // PreviousTagSize

    //上抛scriptTag
    //m_funCallBack->popflv(IFlvCallBack::callback_script_tag, (char*)c->get(), c->size());
    m_funCallBack(flv_tag_type::callback_script_tag, c->get(), c->size());


    // flv写文件
    //char* pheader = nullptr;
    //int headlen = 0;
    //MakeHeader(&pheader, &headlen);
    //fwrite(pheader, headlen, 1, fpflv);
    //fwrite(c->data, c->d_cur, 1, fpflv);
    //fflush(fpflv);
    return true;
}

bool CFlv::MakeVideoH264HeaderTag()
{
    flv_buffer* c = (flv_buffer*)m_pData;
    flv_buffer* sps = (flv_buffer*)m_pSPS;
    flv_buffer* pps = (flv_buffer*)m_pPPS;
    if (c == nullptr || c->get() == nullptr 
        || sps == nullptr || sps->get() == nullptr
        || pps == nullptr )
    {
        Log::error("m_pData:%d, m_pSPS:%d, m_pPPS:%d",m_pData,m_pSPS,m_pPPS);
        return false;
    }
    if (m_funCallBack == nullptr)
    {
        Log::error("m_funCallBack is null");
        return false;
    }

    CH264 naluSps;
    naluSps.SetBuff(sps->get(), sps->size());
    uint32_t nSpsLen = 0;
    char* pSpsData = naluSps.DataBuff(nSpsLen);
    CH264 naluPps;
    naluPps.SetBuff(pps->get(), pps->size());
    uint32_t nPpsLen = 0;
    char* pPpsData = naluPps.DataBuff(nPpsLen);

    c->clear();
    c->append_byte( FLV_TAG_TYPE_VIDEO );// Tag Type
    int nDataLenPos = c->size();
    c->append_be24( 0 );                 // body size  rewrite later
    c->append_be24( m_timestamp );       // timestamp
    c->append_byte( m_timestamp >> 24 ); // timestamp extended
    c->append_be24( 0 );                 // StreamID - Always 0

    int nDataLenBegin = c->size();          // needed for overwriting length

    c->append_byte( 7 | FLV_FRAME_KEY ); // Frametype and CodecID:0x17
    c->append_byte( 0 );                 // AV packet type: AVC sequence header
    c->append_be24( 0 );                 // composition time

    c->append_byte( 1 );                 // version
    c->append_byte( pSpsData[1] );      // profile
    c->append_byte( pSpsData[2] );      // profile
    c->append_byte( pSpsData[3] );      // level
    c->append_byte( 0xff );              // 6 bits reserved (111111) + 2 bits nal size length - 1 (11)

    // SPS
    c->append_byte( 0xe1 );              // 3 bits reserved (111) + 5 bits number of sps (00001)
    c->append_be16( nSpsLen );
    c->append_data( pSpsData, nSpsLen );

    // PPS
    c->append_byte( 1 );                 // number of pps
    c->append_be16( nPpsLen );
    c->append_data( pPpsData, nPpsLen );

    // rewrite data length info
    unsigned length = c->size() - nDataLenBegin;
    c->rewrite_be24( nDataLenPos, length );

    c->append_be32( length + 11 );      // PreviousTagSize

    m_funCallBack(flv_tag_type::callback_video_spspps_tag, (char*)c->get(), c->size());

    Log::debug("get sps size:%d,get pps size:%d",sps->size(),pps->size());
    // h264写文件
    //fwrite(sps->data, sps->d_cur, 1, fp);
    //fwrite(pps->data, pps->d_cur, 1, fp);
    //fflush(fp);
    //// flv写文件
    //fwrite(c->data, c->d_cur, 1, fpflv);
    //fflush(fpflv);
    return true;
}

bool CFlv::MakeVideoH264Tag(char *data,int size,int bIsKeyFrame)
{
    flv_buffer* c = (flv_buffer*)m_pData;
    if (c == nullptr)
    {
        Log::error("m_pData is null");
        return false;
    }
    if (m_funCallBack == nullptr)
    {
        Log::error("m_funCallBack is null");
        return false;
    }

    c->clear();
    c->append_byte( FLV_TAG_TYPE_VIDEO );       // Tag Type
    int nDataLenPos = c->size();
    c->append_be24( 0 );                        // body size  rewrite later
    c->append_be24( m_timestamp );              // timestamp
    c->append_byte( m_timestamp >> 24 );        // timestamp extended
    c->append_be24( 0 );                        // StreamID - Always 0

    int nDataLenBegin = c->size();                 // needed for overwriting length
    if(bIsKeyFrame)
        c->append_byte( FLV_FRAME_KEY);         // Frametype and CodecID:0x17
    else 
        c->append_byte( FLV_FRAME_INTER);       // Frametype and CodecID:0x27

    c->append_byte( 1 );                        // AV packet type: AVC NALU
    c->append_be24( m_tick_gap );               // composition time
    if(bIsKeyFrame)
    {
        flv_buffer* sps = (flv_buffer*)m_pSPS;
        flv_buffer* pps = (flv_buffer*)m_pPPS;
        if (c != nullptr && c->get() != nullptr 
            && sps != nullptr && sps->get() != nullptr
            && pps != nullptr )
        {
            CH264 naluSps;
            naluSps.SetBuff(sps->get(), sps->size());
            uint32_t nSpsLen = 0;
            char* pSpsData = naluSps.DataBuff(nSpsLen);
            CH264 naluPps;
            naluPps.SetBuff(pps->get(), pps->size());
            uint32_t nPpsLen = 0;
            char* pPpsData = naluPps.DataBuff(nPpsLen);
            //写入sps NALU
            c->append_be32( nSpsLen );           // SPS NALU Length
            c->append_data( pSpsData, nSpsLen);  // NALU Data
            //写入pps Nalu
            c->append_be32( nPpsLen );           // SPS NALU Length
            c->append_data( pPpsData, nPpsLen);  // NALU Data
        }
    }
    //写入Nalu
    c->append_be32( size);                       // NALU Length
    c->append_data( data, size);                 // NALU Data

    // rewrite data length info
    unsigned length = c->size() - nDataLenBegin;
    c->rewrite_be24( nDataLenPos, length );

    c->append_be32( 11 + length );               // PreviousTagSize

    if(bIsKeyFrame)
        m_funCallBack(flv_tag_type::callback_key_video_tag, (char*)c->get(), c->size());
    else
        m_funCallBack(flv_tag_type::callback_video_tag, (char*)c->get(), c->size());

    //// h264写文件
    //fwrite(data, size, 1, fp);
    //fflush(fp);
    //// flv写文件
    //fwrite(c->data, c->d_cur, 1, fpflv);
    //fflush(fpflv);
    return true;
}