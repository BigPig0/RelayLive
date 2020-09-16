#include "common.h"
#include "flv.h"
#include "h264.h"

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

void CFlvStreamMaker::append_amf_string(const char *str)
{
    uint16_t len = (uint16_t)strlen( str );
    append_be16( len );
    append_data( (char*)str, len );
}

void CFlvStreamMaker::append_amf_double(double d)
{
    append_byte( AMF_DATA_TYPE_NUMBER );
    append_double(d);
}

CFlv::CFlv(AV_CALLBACK cb, void* handle)
    : m_pSPS(nullptr)
    , m_pPPS(nullptr)
    , m_pData(nullptr)
    , m_timestamp(0)
    , m_tick_gap(0)
    , m_nNodelay(0)
    , m_nWidth(0)
    , m_nHeight(0)
    , m_nfps(25)
    , m_bMakeScript(false)
    , m_bFirstKey(false)
    , m_bGotSPS(false)
    , m_bGotPPS(false)
    , m_hUser(handle)
    , m_fCB(cb)
{
    m_pSPS = new CFlvStreamMaker();
    m_pPPS = new CFlvStreamMaker();
    m_pKeyFrame = new CFlvStreamMaker();
    m_pHeader = new CFlvStreamMaker();
    m_pData = new CFlvStreamMaker();
    m_bRun = true;
}

CFlv::~CFlv(void)
{
    m_bRun = false;
    SAFE_DELETE(m_pSPS);
    SAFE_DELETE(m_pPPS);
    SAFE_DELETE(m_pKeyFrame);
    SAFE_DELETE(m_pHeader);
    SAFE_DELETE(m_pData);
}

int CFlv::Code(AV_BUFF buff)
{
    if(!m_bRun)
    {
        Log::error("already stop");
        return false;
    }
    // MutexLock lock(&m_cs); //�����ܱ�֤�ǵ��̵߳��ã���˲���Ҫ����

    char* pBuf;
    uint32_t nLen = 0;
    h264_nalu_data2(buff.pData, buff.nLen, &pBuf, &nLen);
    NalType eType = h264_naltype(pBuf);

    if(eType == sps_Nal && m_nWidth==0){
        double fps;
        h264_sps_info(pBuf, nLen, &m_nWidth, &m_nHeight, &fps);
		m_tick_gap = 1000/(m_nfps>0?m_nfps:25);
		Log::debug("width = %d,height = %d, fps= %lf, tickgap= %d",m_nWidth,m_nHeight,m_nfps,m_tick_gap);
    }

    switch (eType)
    {
    case b_Nal:  // �ǹؼ�֡
        {
            if(!m_bFirstKey)
                break;
            //������ڹؼ�֡û�д�����Ҫ�ȷ��͹ؼ�֡��pps�����ڹؼ�֡������յ���
            //sps��pps��ʧ������ʹ����һ�εġ�
            MakeKeyVideo(); 
            //Log::debug("send frame");
            MakeVideo(pBuf,nLen,0);
            m_timestamp += m_tick_gap;
        }
        break;
    case idr_Nal: // �ؼ�֡
		{
			m_pKeyFrame->clear();
			m_pKeyFrame->append_data(pBuf, nLen);
            //һ��sps��pps���ڹؼ�֡ǰ�棬����ʱ���ڹؼ�֡���档
            if(m_bGotPPS && m_bGotSPS)
			    MakeKeyVideo();
		}
        break;
    case sei_Nal:
        break;
    case sps_Nal:
        {
            //Log::debug("save sps size:%d",nLen);
            CHECKPOINT_INT(m_pSPS,-1);
            m_pSPS->clear();
            m_pSPS->append_data(pBuf, nLen);
            m_bGotSPS = true;
        }
        break;
    case pps_Nal:
        {
            //Log::debug("save pps size:%d",nLen);
            CHECKPOINT_INT(m_pPPS,-1);
            m_pPPS->clear();
            m_pPPS->append_data(pBuf, nLen);
            m_bGotPPS = true;
        }
        break;
    case other:
    case unknow:
    default:
		//Log::warning("h264 nal type: %d", eType);
        break;
    }
    return true;
}

void CFlv::SetSps(uint32_t nWidth, uint32_t nHeight, double fFps) 
{
    m_nWidth = nWidth;
    m_nHeight = nHeight;
    m_nfps = fFps;
    m_tick_gap = 1000/(m_nfps>0?m_nfps:25);
    Log::debug("width = %d,height = %d, fps= %lf, tickgap= %d",m_nWidth,m_nHeight,m_nfps,m_tick_gap);
}

bool CFlv::MakeHeader()
{
    CHECKPOINT_BOOL(m_pHeader);
    CHECKPOINT_BOOL(m_pSPS);
    CHECKPOINT_BOOL(m_pPPS);
    uint16_t nSpsLen = (uint16_t)m_pSPS->size();
    char* pSpsData = m_pSPS->get(); 
    uint16_t nPpsLen = (uint16_t)m_pPPS->size();
    char* pPpsData = m_pPPS->get();

    /** FLV header */
    m_pHeader->append_string("FLV"); // Signature
    m_pHeader->append_byte(1);    // Version
    m_pHeader->append_byte(1);    // Video Only
    m_pHeader->append_be32(9);    // DataOffset
    m_pHeader->append_be32(0);    // PreviousTagSize0

    /** Script tag */
    m_pHeader->append_byte( FLV_TAG_TYPE_META ); // Tag Type "script data"

    int nDataLenPos = m_pHeader->size();
    m_pHeader->append_be24( 0 ); // data length
    m_pHeader->append_be24( 0 ); // timestamp ���ڽű��͵�tag����0
    m_pHeader->append_be32( 0 ); // reserved (timestampextended streamID)

    int nDataLenBegin = m_pHeader->size();
    m_pHeader->append_byte( AMF_DATA_TYPE_STRING );
    m_pHeader->append_amf_string( "onMetaData" );

    m_pHeader->append_byte( AMF_DATA_TYPE_MIXEDARRAY ); //metaԪ�ظ�����������д������������Ǽ�
    m_pHeader->append_be32( 8 );

    m_pHeader->append_amf_string( "duration" );
    m_pHeader->append_amf_double( 0 ); // ʱ��

    m_pHeader->append_amf_string( "width" );
    m_pHeader->append_amf_double( m_nWidth );

    m_pHeader->append_amf_string( "height" );
    m_pHeader->append_amf_double( m_nHeight );

    m_pHeader->append_amf_string( "videodatarate" );
    m_pHeader->append_amf_double( 0 ); // ����

    m_pHeader->append_amf_string( "framerate" );
    m_pHeader->append_amf_double( m_nfps );

    m_pHeader->append_amf_string( "videocodecid" );
    m_pHeader->append_amf_double( FLV_CODECID_H264 );

    m_pHeader->append_amf_string( "encoder" );
    m_pHeader->append_byte( AMF_DATA_TYPE_STRING );
    m_pHeader->append_amf_string( "Lavf55.37.102" );

    m_pHeader->append_amf_string( "filesize" );
    m_pHeader->append_amf_double( 0 ); // �ļ���С

    m_pHeader->append_amf_string( "" );
    m_pHeader->append_byte( AMF_END_OF_OBJECT );

    unsigned nDataLen = m_pHeader->size() - nDataLenBegin;
    m_pHeader->rewrite_be24( nDataLenPos, nDataLen ); // ��д data length

    m_pHeader->append_be32( nDataLen + 11 ); // PreviousTagSize

    /** AVCDecoderConfigurationRecord, this is the first video tag */
    m_pHeader->append_byte( FLV_TAG_TYPE_VIDEO );// Tag Type
    nDataLenPos = m_pHeader->size();
    m_pHeader->append_be24( 0 );                 // body size  rewrite later
    m_pHeader->append_be24( m_timestamp );       // timestamp
    m_pHeader->append_byte( m_timestamp >> 24 ); // timestamp extended
    m_pHeader->append_be24( 0 );                 // StreamID - Always 0

    nDataLenBegin = m_pHeader->size();          // needed for overwriting length

    m_pHeader->append_byte( 7 | FLV_FRAME_KEY ); // Frametype and CodecID:0x17
    m_pHeader->append_byte( 0 );                 // AV packet type: AVC sequence header
    m_pHeader->append_be24( 0 );                 // composition time

    m_pHeader->append_byte( 1 );                 // version
    m_pHeader->append_byte( pSpsData[1] );      // profile
    m_pHeader->append_byte( pSpsData[2] );      // profile
    m_pHeader->append_byte( pSpsData[3] );      // level
    m_pHeader->append_byte( 0xff );              // 6 bits reserved (111111) + 2 bits nal size length - 1 (11)

    // SPS
    m_pHeader->append_byte( 0xe1 );              // 3 bits reserved (111) + 5 bits number of sps (00001)
    m_pHeader->append_be16( nSpsLen );
    m_pHeader->append_data( pSpsData, nSpsLen );

    // PPS
    m_pHeader->append_byte( 1 );                 // number of pps
    m_pHeader->append_be16( nPpsLen );
    m_pHeader->append_data( pPpsData, nPpsLen );

    // rewrite data length info
    unsigned length = m_pHeader->size() - nDataLenBegin;
    m_pHeader->rewrite_be24( nDataLenPos, length );

    m_pHeader->append_be32( length + 11 );      // PreviousTagSize

    /** call back */
    if(m_fCB != nullptr){
        AV_BUFF buff = {FLV_HEAD, m_pHeader->get(), m_pHeader->size()};
        m_fCB(buff, m_hUser);
    }
    return true;
}

bool CFlv::MakeVideo(char *data,int size,int bIsKeyFrame)
{
    CHECKPOINT_BOOL(m_pData);

    // ��ʱ����ģʽ������һ�����ݣ�ÿ���յ��ؼ�֡����֮ǰ���������һ������
    if(m_nNodelay == 0 && bIsKeyFrame && m_pData->size() > 0) {
        if(m_fCB != nullptr){
            AV_BUFF buff = {FLV_FRAG_KEY, m_pData->get(), m_pData->size()};
            m_fCB(buff, m_hUser);
        }
        m_pData->clear();
    }

    m_pData->append_byte( FLV_TAG_TYPE_VIDEO );       // Tag Type
    int nDataLenPos = m_pData->size();
    m_pData->append_be24( 0 );                        // body size  rewrite later
    m_pData->append_be24( m_timestamp );              // timestamp
    m_pData->append_byte( m_timestamp >> 24 );        // timestamp extended
    m_pData->append_be24( 0 );                        // StreamID - Always 0

    int nDataLenBegin = m_pData->size();                 // needed for overwriting length
    if(bIsKeyFrame)
        m_pData->append_byte( FLV_FRAME_KEY);         // Frametype and CodecID:0x17
    else 
        m_pData->append_byte( FLV_FRAME_INTER);       // Frametype and CodecID:0x27

    m_pData->append_byte( 1 );                        // AV packet type: AVC NALU
    m_pData->append_be24( m_tick_gap );               // composition time
    if(bIsKeyFrame)
    {
        if (m_pSPS != nullptr && m_pSPS->size() > 0
            && m_pPPS != nullptr && m_pPPS->size() > 0)
        {
            //д��sps NALU
            m_pData->append_be32( m_pSPS->size() );           // SPS NALU Length
            m_pData->append_data( m_pSPS->get(), m_pSPS->size());  // NALU Data
            //д��pps Nalu
            m_pData->append_be32( m_pPPS->size() );           // PPS NALU Length
            m_pData->append_data( m_pPPS->get(), m_pPPS->size());  // NALU Data
        }
    }
    //д��Nalu
    m_pData->append_be32( size);                       // NALU Length
    m_pData->append_data( data, size);                 // NALU Data

    // rewrite data length info
    unsigned length = m_pData->size() - nDataLenBegin;
    m_pData->rewrite_be24( nDataLenPos, length );

    m_pData->append_be32( 11 + length );               // PreviousTagSize

    //��������ģʽ��ÿ�յ�һ֡���ݾ��������׷���
    if(m_nNodelay != 0 && m_pData->size() > 0) {
        if(m_fCB != nullptr){
            AV_BUFF buff = {bIsKeyFrame?FLV_FRAG_KEY:FLV_FRAG, m_pData->get(), m_pData->size()};
            m_fCB(buff, m_hUser);
        }
        m_pData->clear();
    }
    //// h264д�ļ�
    //fwrite(data, size, 1, fp);
    //fflush(fp);
    //// flvд�ļ�
    //fwrite(c->data, c->d_cur, 1, fpflv);
    //fflush(fpflv);
    return true;
}

bool CFlv::MakeKeyVideo()
{
	if(m_pSPS->size() && m_pPPS->size() && m_pKeyFrame->size()) {
		// ��һ��tag��scriptTag
		//Log::debug("send scriptTag");
		if(!m_bMakeScript)
		{    
			if(!MakeHeader())
				return false;

			m_bMakeScript = true;
		}

		// ���͹ؼ�֡
		static uint64_t num = 0;
		Log::debug("send key frame %lld", num++);
		MakeVideo(m_pKeyFrame->get(),m_pKeyFrame->size(),1);

		m_pKeyFrame->clear();
        m_bGotSPS = false;
        m_bGotPPS = false;

		m_timestamp += m_tick_gap;
		m_bFirstKey = true;

		return true;
	}
	return false;
}