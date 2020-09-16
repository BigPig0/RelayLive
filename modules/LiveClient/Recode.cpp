#include "common.h"
#include "Recode.h"
#ifdef EXTEND_CHANNELS
#include "utilc.h"
#include "es.h"
#include "h264.h"
#include "flv.h"

extern "C"
{
#define __STDC_FORMAT_MACROS
#define snprintf  _snprintf
//#include "libavdevice/avdevice.h"
#include "libavcodec/avcodec.h"  
//#include "libavformat/avformat.h"  
#include "libswscale/swscale.h"  
#include "libavutil/imgutils.h"
//#include "libavutil/timestamp.h"
}
#pragma comment(lib,"avcodec.lib")
//#pragma comment(lib,"avdevice.lib")
//#pragma comment(lib,"avfilter.lib")
//#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"postproc.lib")
//#pragma comment(lib,"swresample.lib")
#pragma comment(lib,"swscale.lib")

namespace LiveClient
{

    /**
    * h264����
    * ��������������parser������ÿ��������һ֡���ݴ�ŵ�pkt��
    * ��pkt�����ݽ���decode��yuv������������frame
    */
    class CDecoder : public IDecoder {
    public:
        AVCodec             *m_codec;      //h264�������
        AVCodecContext      *m_context;    //����������
        //AVCodecParserContext *parser;    //h264������
        AVFrame             *m_pFrame;     //�����yuv����
        bool                 m_ok;         //�Ƿ�ɹ���ʼ��ffmpeg

        CNetStreamMaker     *m_pSPS;            // ����SPS���ݵĻ���
        CNetStreamMaker     *m_pPPS;            // ����PPS���ݵĻ���
        CNetStreamMaker     *m_pKeyFrame;       // ����ؼ�֡��sps��pps�п����ں���
        int64_t              m_nKeyPts;         // ����ؼ�֡��pts
        int64_t              m_nKeyDts;         // ����ؼ�֡��dts
        bool                 m_bFirstKey;       // �Ѿ������һ���ؼ�֡
        bool                 m_bGotSPS;         // ��ǽ����Ƿ������sps
        bool                 m_bGotPPS;         // ��ǽ����Ƿ������sps
        uint32_t             m_timestamp;       // ʱ���
        uint32_t             m_tick_gap;        // ��֡��ļ��

        AV_CALLBACK          m_funCB;      //�ص�����
        void                *m_user;       //�ص�����

        CDecoder();
        ~CDecoder();
        bool DecodeVideo(AV_BUFF buff);
        bool DecodeKeyVideo();
        int Decode(AV_BUFF buff);
    };

    IDecoder* IDecoder::Create(AV_CALLBACK cb, void* handle/*=NULL*/) {
        CDecoder* ret = new CDecoder();
        ret->m_funCB = cb;
        ret->m_user = handle;
        return ret;
    }
    
    CDecoder::CDecoder() 
		: m_ok(false)
		, m_pSPS(nullptr)
		, m_pPPS(nullptr)
        , m_bFirstKey(false)
        , m_bGotSPS(false)
        , m_bGotPPS(false)
		, m_pKeyFrame(nullptr)
        , m_timestamp(0)
        , m_tick_gap(3600)
	{
		m_pSPS = new CNetStreamMaker();
		m_pPPS = new CNetStreamMaker();
		m_pKeyFrame = new CNetStreamMaker();

        //�������ʵ��
        m_codec = avcodec_find_decoder(AV_CODEC_ID_H264); 
        if (!m_codec){
            Log::error("Codec not found\n");
            return;
        }

        //������������ģ�ָ���˱����Ĳ���
        m_context = avcodec_alloc_context3(m_codec); //����AVCodecContextʵ��
        if (!m_context) {
            Log::error("Could not allocate video codec context\n");
            return;
        }

        //�������������������н�ȡ������һ��NAL Unit����
        //parser = av_parser_init(AV_CODEC_ID_H264);
        //if (!parser){
        //    Log::error("Could not allocate video parser context\n");
        //    return;
        //}

        //��AVCodec����
        int ret = avcodec_open2(m_context, m_codec, NULL);
        if (ret < 0){
            char tmp[1024]={0};
            av_strerror(ret, tmp, 1024);
            Log::error("Could not open codec %d:%s", ret, tmp);
            return;
        }

        //��װͼ�����ָ��
        m_pFrame = av_frame_alloc();
        if (!m_pFrame) {
            Log::error("Could not allocate video frame");
            return;
        }

        m_ok = true;
        //f = fopen("test.yuv", "wb");
    }

    CDecoder::~CDecoder(){
        avcodec_close(m_context);
        //av_parser_close(parser);
        av_free(m_context);
        av_frame_free(&m_pFrame);
    }

    bool CDecoder::DecodeVideo(AV_BUFF buff)
    {
        //��װ��������ʵ��
        AVPacket pkt;        //���������h264����
        av_init_packet(&pkt);
        pkt.data = (uint8_t*)buff.pData;
        pkt.size = buff.nLen;
        pkt.pts  = m_timestamp;
        pkt.dts  = m_timestamp;

        /*
        m_decode->pkt.data = NULL;
        m_decode->pkt.size = 0;
        int len = av_parser_parse2(m_decode->parser, m_decode->c, 
        &(m_decode->pkt.data), &(m_decode->pkt.size), 
        (uint8_t*)buff.pData, buff.nLen, 
        AV_NOPTS_VALUE, AV_NOPTS_VALUE, AV_NOPTS_VALUE);

        if(m_decode->pkt.size==0){    // h264һ��֡û������
        //Log::error("not full");
        return 0;
        }
        */
        int ret = avcodec_send_packet(m_context, &pkt);
        if (ret != 0) {
            char tmp[1024]={0};
            av_strerror(ret, tmp, 1024);
            Log::error("Decode Error %d:%s", ret, tmp);
            return false;
        }

        while(!ret){
            //Log::debug("receive frame");
            //ԭʼ���ݽ�����yuvͼƬ
            ret = avcodec_receive_frame(m_context, m_pFrame);
            if(ret == 0){
                //Log::debug("src width:%d, height:%d", m_decode->frame->width, m_decode->frame->height);
                //Log::debug("context %d %d %lld %d %d", m_decode->c->width, m_decode->c->height, m_decode->c->bit_rate, m_decode->c->time_base.den, m_decode->c->time_base.num);
                //Log::debug("yuv pts: %lld  dts: %lld", m_decode->frame->pts, m_decode->frame->pkt_dts);
                //
                //
                //int size = frame->width * frame->height;
                //fwrite(m_decode->frame->data[0], 1, size, m_decode->f);
                //fwrite(m_decode->frame->data[1], 1, size/4, m_decode->f);
                //fwrite(m_decode->frame->data[2], 1, size/4, m_decode->f);
                //fflush(m_decode->f);
                //break;
                if(m_funCB) {
					AV_BUFF yuv = {AV_TYPE::YUV, (char*)m_pFrame, m_timestamp, m_timestamp};
                    m_funCB(yuv, m_user);
                }
            }
			av_frame_unref(m_pFrame);
        } //while

        return true;
    }

    bool CDecoder::DecodeKeyVideo()
    {
        if(m_pSPS->size() && m_pPPS->size() && m_pKeyFrame->size()) {
            int bufsize = m_pSPS->size() + m_pPPS->size() + m_pKeyFrame->size();
            char* buff = (char*)malloc(bufsize);
            int pos = 0;
            memcpy(buff+pos, m_pSPS->get(), m_pSPS->size());
            pos += m_pSPS->size();
            memcpy(buff+pos, m_pPPS->get(), m_pPPS->size());
            pos += m_pPPS->size();
            memcpy(buff+pos, m_pKeyFrame->get(), m_pKeyFrame->size());
            pos += m_pKeyFrame->size();

            //Log::debug("send key frame");
            AV_BUFF avbuff = {AV_TYPE::H264_IDR, buff, bufsize, m_nKeyPts, m_nKeyDts};
            DecodeVideo(avbuff);
            free(buff);

            //Log::debug("send sps");
            //MakeVideo(m_pSPS->get(),m_pSPS->size(),1);
            //Log::debug("send pps");
            //MakeVideo(m_pPPS->get(),m_pPPS->size(),1);
            //Log::debug("send key frame");
            //MakeVideo(m_pKeyFrame->get(),m_pKeyFrame->size(),1);

            m_pKeyFrame->clear();
            m_bGotSPS = false;
            m_bGotPPS = false;

			m_timestamp += m_tick_gap;
            m_bFirstKey = true;

            return true;
        }
        return false;
    }

    int CDecoder::Decode(AV_BUFF buff){
		char *nalu = NULL;
		h264_nalu_data(buff.pData, &nalu);
        NalType t = h264_naltype(nalu);
        switch (t)
        {
        case b_Nal:  // �ǹؼ�֡
            {
                if(!m_bFirstKey)
                    break;
                //������ڹؼ�֡û�д�����Ҫ�ȷ��͹ؼ�֡��pps�����ڹؼ�֡������յ���
                //sps��pps��ʧ������ʹ����һ�εġ�
                DecodeKeyVideo(); 
                //Log::debug("send frame");
                DecodeVideo(buff);
				m_timestamp += m_tick_gap;
            }
            break;
        case idr_Nal: // �ؼ�֡
            {
                m_pKeyFrame->clear();
                m_pKeyFrame->append_data(buff.pData, buff.nLen);
                m_nKeyPts = buff.m_pts;
                m_nKeyDts = buff.m_dts;
                //һ��sps��pps���ڹؼ�֡ǰ�棬����ʱ���ڹؼ�֡���档
                if(m_bGotPPS && m_bGotSPS)
                    DecodeKeyVideo();
            }
            break;
        case sei_Nal:
            break;
        case sps_Nal:
            {
                //Log::debug("save sps size:%d",nLen);
                CHECK_POINT_INT(m_pSPS,-1);
                m_pSPS->clear();
                m_pSPS->append_data(buff.pData, buff.nLen);
                m_bGotSPS = true;
            }
            break;
        case pps_Nal:
            {
                //Log::debug("save pps size:%d",nLen);
                CHECK_POINT_INT(m_pPPS,-1);
                m_pPPS->clear();
                m_pPPS->append_data(buff.pData, buff.nLen);
                m_bGotPPS = true;
            }
            break;
        case other:
        case unknow:
        default:
            //Log::warning("h264 nal type: %d", t);
            break;
        }

        return 0;
    }

//////////////////////////////////////////////////////////////////////////

    /**
    * h264����
    */
    class CEncoder : public IEncoder{
    public:
        CDecoder        *m_decoder;
        SwsContext      *m_swsc;      //����������
        AVCodec         *m_codec;     //h264�������
        AVCodecContext  *m_context;   //����������
        AVFrame         *m_frame;     //���ź��yuvͼƬ
        CES             *m_pEs;       // ES�����������nalu

        AV_CALLBACK      m_funCB;
		void            *m_user;
        bool             m_ok;
        uint32_t         m_timestamp;       // ʱ���
        uint32_t         m_tick_gap;        // ��֡��ļ��

        CEncoder();
        ~CEncoder();

        void Start();
        int Code(AV_BUFF buff);
        void SetDecoder(IDecoder* dec);
        void PushH264(AV_BUFF buff);
    };

    static void ParseEsCb(AV_BUFF buff, void* pUser) {
        CEncoder *coder = (CEncoder*)pUser;
        coder->PushH264(buff);
    }

    IEncoder* IEncoder::Create(AV_CALLBACK cb, void* handle/*=NULL*/) {
        CEncoder* ret = new CEncoder();
        ret->m_funCB = cb;
        ret->m_user = handle;
        return ret; 
    }

    CEncoder::CEncoder()
        : m_ok(false)
        , m_timestamp(0)
        , m_tick_gap(3600)
    {
        m_pEs = new CES(ParseEsCb, this);

        //��װͼ�����ָ��
        m_frame = av_frame_alloc();
        if (!m_frame) {
            Log::error("Could not allocate video frame");
            return;
        }
    }

    CEncoder::~CEncoder(){
		if(m_ok) {
			avcodec_close(m_context);
			av_free(m_context);
			sws_freeContext(m_swsc);
		}
        av_frame_free(&m_frame);
        SAFE_DELETE(m_pEs);
    }

    void CEncoder::Start(){
        //����������
        m_swsc = sws_getContext(m_decoder->m_pFrame->width
            , m_decoder->m_pFrame->height
            , (AVPixelFormat)m_decoder->m_pFrame->format
            , m_width, m_height
            , (AVPixelFormat)m_decoder->m_pFrame->format
            , SWS_BICUBIC , NULL, NULL, NULL);

        if(m_swsc == NULL){
            Log::error("sws_getContext failed");
            return;
        }

        //�������ʵ��ָ��
        m_codec = avcodec_find_encoder(AV_CODEC_ID_H264); 
        if (!m_codec){
            Log::error("Codec not found\n");
            return;
        }

        //������������ģ�ָ���˱����Ĳ���
        m_context = avcodec_alloc_context3(m_codec); //����AVCodecContextʵ��
        if (!m_context) {
            Log::error("Could not allocate video codec context\n");
            return;
        }
        m_context->bit_rate = m_decoder->m_context->bit_rate; //4000000;
        m_context->width    = m_width;
        m_context->height   = m_height;
        m_context->time_base.num = 1;
        m_context->time_base.den = 25;
        m_context->pix_fmt  = (enum AVPixelFormat)m_decoder->m_pFrame->format;
        m_context->gop_size = 15;
        m_context->has_b_frames = 0;
        m_context->max_b_frames = 0;
        m_context->qmin = 34;
        m_context->qmax = 50;
        m_frame->format = (enum AVPixelFormat)m_decoder->m_pFrame->format;
        m_frame->width = m_width;
        m_frame->height = m_height;

        // �����������룬����ʱ
        AVDictionary *options = NULL;
        //-preset fast/faster/verfast/superfast/ultrafast
        av_dict_set(&options, "preset", "ultrafast",   0);
        //av_dict_set(&options, "tune",   "zerolatency", 0);

        //��AVCodec����
        if (avcodec_open2(m_context, m_codec, &options) < 0){
            Log::error("Could not open codec");
            return;
        }

        int size = av_image_alloc(m_frame->data, m_frame->linesize, m_width, m_height, (enum AVPixelFormat)(m_decoder->m_pFrame->format), 16);
        Log::debug("image size %d", size);

        m_ok= true;
    }

    int CEncoder::Code(AV_BUFF buff){
        AVFrame *Srcframe = (AVFrame *)buff.pData;

        if(!m_ok){
            Log::debug("first codec obj");
            Start();
        }
        if(!m_ok){
            Log::error("codec is not ok");
            return -1;
        }

        //����ͼ��
        int h = sws_scale(m_swsc, Srcframe->data, Srcframe->linesize, 0, Srcframe->height
            , m_frame->data, m_frame->linesize);
        m_frame->pts = buff.m_pts;
        m_frame->pkt_dts = buff.m_dts;

        //����
        int cret = avcodec_send_frame(m_context, m_frame);

        if (cret == 0){
            AVPacket pkt;        //������h264����
            av_init_packet(&pkt);
            pkt.data = NULL;
            pkt.size = 0;

            cret = avcodec_receive_packet(m_context, &pkt);
            if (cret != 0) {
                char tmp[1024]={0};
                av_strerror(cret, tmp, 1024);
                Log::error("avcodec_receive_packet error: %s", tmp);
                return -1;
            }
            //Log::debug("recode finish size:%d, pts:%d, dts:%d", m_codec->pkt.size, m_codec->pkt.pts, m_codec->pkt.dts);

            //fwrite(m_codec->pkt.data, 1, m_codec->pkt.size, m_codec->f);
            //fflush(m_codec->f);
            AV_BUFF ESBUFF = {AV_TYPE::ES, (char*)pkt.data, pkt.size};
            m_pEs->DeCode(ESBUFF);

            av_packet_unref(&pkt);
        }
        return 0;
    }

    void CEncoder::SetDecoder(IDecoder* dec){
        m_decoder = (CDecoder*)dec;
    }

    void CEncoder::PushH264(AV_BUFF buff){
        if(m_funCB){
            m_funCB(buff, m_user);
        }
    }


}
#endif