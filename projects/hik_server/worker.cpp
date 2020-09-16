
#include "uv.h"
#include "libwebsockets.h"
#include "netstream.h"
#include "live.h"
#include "hiksdk.h"
#include "worker.h"
#include "ipc.h"
#include "util.h"
#include "util_log.h"
#include <list>
#include <sstream>

#ifdef USE_FFMPEG
extern "C"
{
#define snprintf  _snprintf
#define __STDC_FORMAT_MACROS
#include "libavdevice/avdevice.h"
#include "libavcodec/avcodec.h"  
#include "libavformat/avformat.h"  
#include "libswscale/swscale.h"  
#include "libavutil/imgutils.h"
#include "libavutil/timestamp.h"
}
#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"avdevice.lib")
#pragma comment(lib,"avfilter.lib")
#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"postproc.lib")
#pragma comment(lib,"swresample.lib")
#pragma comment(lib,"swscale.lib")
#endif

namespace Server
{
    typedef struct _AV_BUFF_ {
        char       *pData;    //< ��������
        uint32_t    nLen;     //< ���ݳ���
    }AV_BUFF;

    static list<CLiveWorker*>  _listWorkers;
    static CriticalSection     _csWorkers;
	static uint32_t            _flvbufsize = 1024*16;
	static uint32_t            _psbufsize = 1024*32;

    static void destroy_ring_node(void *_msg)
    {
        AV_BUFF *msg = (AV_BUFF*)_msg;
        free(msg->pData);
        msg->pData = NULL;
        msg->nLen = 0;
    }

    static string GetClientsInfo() {
        bool first = true;
        stringstream ss;
        for (auto c : _listWorkers) {
            if(!first) {
                first = false;
                ss << ",";
            }
            ss << "{\"DeviceID\":\"" << c->m_strCode 
                << "\",\"Connect\":\"";
            if(c->m_bWebSocket)
                ss << "websocket";
            else
                ss << "http";
            ss << "\",\"Media\":\"" << c->m_strType
                << "\",\"ClientIP\":\"" << c->m_strClientIP
                << "\",\"Channel\":\"" << c->m_strHw
                << "\"}";
        }
        return ss.str();
    }

    //////////////////////////////////////////////////////////////////////////

    // �����߳�
    static void real_play(void* arg){
        CLiveWorker* pLive = (CLiveWorker*)arg;
        pLive->Play();
    }

#ifdef USE_FFMPEG
    //��ȡ�������ݵķ���
    static int fill_iobuffer(void *opaque,uint8_t *buf, int bufsize){
        CLiveWorker *lw = (CLiveWorker*)opaque;
        int len = 0;
        while (len == 0) {
            len = lw->get_ps_data((char*)buf, bufsize);
        }
        return len;
    }

    //������ݵķ���
    static int write_buffer(void *opaque, uint8_t *buf, int buf_size){
        CLiveWorker* pLive = (CLiveWorker*)opaque;
        pLive->push_flv_frame((char*)buf, buf_size);
        return buf_size;
    }
#endif

    CLiveWorker::CLiveWorker()
        : m_bWebSocket(false)
		, m_bConnect(true)
		, m_bFirstStream(true)
		, m_bCbStream(false)
    {
        m_pFlvRing  = create_ring_buff(sizeof(AV_BUFF), 100, destroy_ring_node);
        m_pPSRing   = create_ring_buff(sizeof(AV_BUFF), 1000, destroy_ring_node);
    }

    CLiveWorker::~CLiveWorker()
    {
        destroy_ring_buff(m_pFlvRing);
        destroy_ring_buff(m_pPSRing);
        Log::debug("CLiveWorker release");
    }

#ifdef USE_FFMPEG
    bool CLiveWorker::Play()
    {
        int playhandle = HikPlat::Play(this, m_strCode);
        if(playhandle < 0) {
            return false;
        }

        AVFormatContext *ifc = NULL;
        AVFormatContext *ofc = NULL;

        ifc = avformat_alloc_context();
        unsigned char * iobuffer=(unsigned char *)av_malloc(_psbufsize);
        AVIOContext *avio = avio_alloc_context(iobuffer, _psbufsize, 0, this, fill_iobuffer, NULL, NULL);
        ifc->pb = avio;
		ifc->iformat = av_find_input_format("mpeg");

        int ret = avformat_open_input(&ifc, "nothing", NULL, NULL);
        if (ret != 0) {
            char tmp[1024]={0};
            av_strerror(ret, tmp, 1024);
            Log::error("Could not open input file: %d(%s)", ret, tmp);
            goto end;
        }
		//ifc->probesize = 102400;
		ifc->max_analyze_duration = 2*AV_TIME_BASE; //̽��ֻ������ʱ2s
        ret = avformat_find_stream_info(ifc, NULL);
        if (ret < 0) {
            char tmp[1024]={0};
            av_strerror(ret, tmp, 1024);
            Log::error("Failed to retrieve input stream information %d(%s)", ret, tmp);
            goto end;
        }
        Log::debug("show input format info");
        av_dump_format(ifc, 0, "nothing", 0);

        //��� �Զ���ص�
        ret = avformat_alloc_output_context2(&ofc, NULL, m_strType.c_str(), NULL);
        if (!ofc) {
            Log::error("Could not create output context\n");
            ret = AVERROR_UNKNOWN;
            goto end;
        }

        unsigned char* outbuffer=(unsigned char*)av_malloc(_flvbufsize);
        AVIOContext *avio_out =avio_alloc_context(outbuffer, _flvbufsize,1,this,NULL,write_buffer,NULL);  
        ofc->pb = avio_out; 
        ofc->flags = AVFMT_FLAG_CUSTOM_IO;

        AVOutputFormat *ofmt = ofc->oformat;
        ofmt->flags |= AVFMT_NOFILE;

        //������������Ϣ�����������Ϣ
        int in_video_index = -1, in_audio_index = -1, in_subtitle_index = -1;
        int out_video_index = -1, out_audio_index = -1, out_subtitle_index = -1;
        for (unsigned int i = 0, j = 0; i < ifc->nb_streams; i++) {
            AVStream *is = ifc->streams[i];
            if (is->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
                in_video_index = i;
                out_video_index = j++;
                //} else if (is->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
                //    in_audio_index = i;
                //    out_audio_index = j++;
                //} else if (is->codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE) {
                //    in_subtitle_index = i;
                //    out_subtitle_index = j++;
            } else {
                continue;
            }

            AVStream *os = avformat_new_stream(ofc, NULL);
            if (!os) {
                Log::error("Failed allocating output stream\n");
                ret = AVERROR_UNKNOWN;
                goto end;
            }

            ret = avcodec_parameters_copy(os->codecpar, is->codecpar);
            if (ret < 0) {
                Log::error("Failed to copy codec parameters\n");
                goto end;
            }
			os->codecpar->codec_id = AV_CODEC_ID_H264;
        }
        Log::debug("show output format info");
        av_dump_format(ofc, 0, NULL, 1);

        //Log::debug("index:%d %d %d %d",in_video_index, in_audio_index, out_video_index, out_audio_index);

        ret = avformat_write_header(ofc, NULL);
        if (ret < 0) {
            char tmp[1024]={0};
            av_strerror(ret, tmp, 1024);
            Log::error("Error avformat_write_header %d:%s \n", ret, tmp);
            goto end;
        }

        while (m_bConnect) {
            AVStream *in_stream, *out_stream;
            AVPacket pkt;
            av_init_packet(&pkt);
            ret = av_read_frame(ifc, &pkt);
            if (ret < 0)
                break;
            //Log::debug("read_index %d",pkt.stream_index);
            in_stream  = ifc->streams[pkt.stream_index];
            if (pkt.stream_index == in_video_index) {
                pkt.stream_index = out_video_index;
                out_stream = ofc->streams[pkt.stream_index];
                pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF/*|AV_ROUND_PASS_MINMAX*/);
                pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF/*|AV_ROUND_PASS_MINMAX*/);
                pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
                pkt.pos = -1;

                int wret = av_interleaved_write_frame(ofc, &pkt);
                if (wret < 0) {
                    char tmp[1024]={0};
                    av_strerror(ret, tmp, 1024);
                    Log::error("video error muxing packet %d:%s \n", ret, tmp);
                    //break;
                }
            } //else if (pkt.stream_index == in_audio_index) {
            //    //Log::debug("audio dts %d pts %d", pkt.dts, pkt.pts);
            //    pkt.stream_index = out_audio_index;
            //    out_stream = ofc->streams[pkt.stream_index];
            //    /* copy packet */
            //    pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF/*|AV_ROUND_PASS_MINMAX*/);
            //    pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF/*|AV_ROUND_PASS_MINMAX*/);
            //    pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
            //    pkt.pos = -1;
            //    //Log::debug("audio2 dts %d pts %d", pkt.dts, pkt.pts);
            //
            //    int wret = av_interleaved_write_frame(ofc, &pkt);
            //    if (wret < 0) {
            //        char tmp[1024]={0};
            //        av_strerror(ret, tmp, 1024);
            //        Log::error("audio error muxing packet %d:%s \n", ret, tmp);
            //        //break;
            //    }
            //}
            av_packet_unref(&pkt);
        }

        av_write_trailer(ofc);
end:
        /** �ر����� */
        avformat_close_input(&ifc);

        /* �ر���� */
        if (ofc && !(ofmt->flags & AVFMT_NOFILE))
            avio_closep(&ofc->pb);
        avformat_free_context(ofc);

        /** ������ */
        if (ret < 0 /*&& ret != AVERROR_EOF*/) {
            char tmp[AV_ERROR_MAX_STRING_SIZE]={0};
            av_make_error_string(tmp,AV_ERROR_MAX_STRING_SIZE,ret);
            Log::error("Error occurred: %s\n", tmp);
        }
        HikPlat::Stop(playhandle);
        if(m_bConnect) {
            Log::debug("video source is dropped, need close the client");
            //lws_set_timeout(m_pPss->wsi, PENDING_TIMEOUT_CLOSE_SEND, LWS_TO_KILL_ASYNC);
        }
        while (m_bConnect){
            sleep(10);
        }
        Log::debug("client stop, delete live worker");
        delete this;
        return ret>=0;
    }
#else
bool CLiveWorker::Play() {
}
#endif

    void CLiveWorker::push_ps_data(char* pBuff, int nLen)
    {
        //�ڴ����ݱ�����ring-buff
		int n = (int)ring_get_count_free_elements(m_pPSRing);
		if (!n) {
			Log::error("to many ps data can't catch");
			return;
		}

		// �����ݱ�����ring buff
		char* pSaveBuff = (char*)malloc(nLen);
		memcpy(pSaveBuff, pBuff, nLen);
		AV_BUFF newTag = {pSaveBuff, nLen};
		if (!ring_insert(m_pPSRing, &newTag, 1)) {
			destroy_ring_node(&newTag);
			Log::error("dropping!");
			return;
		}
    }

    int CLiveWorker::get_ps_data(char* pBuff, int &nLen)
    {
        AV_BUFF* tag = (AV_BUFF*)ring_get_element(m_pPSRing, NULL);
		if(tag) {
			int len = tag->nLen;
			memcpy(pBuff, tag->pData, tag->nLen);
			simple_ring_cosume(m_pPSRing);
			return len;
		}
        return 0;
    }

    void CLiveWorker::push_flv_frame(char* pBuff, int nLen)
    {
        if(!m_bConnect) {
            Log::error("client has already leave");
            return;
        }
        //�ڴ����ݱ�����ring-buff
        int n = (int)ring_get_count_free_elements(m_pFlvRing);
        if (!n && m_pPss) {
            //lws_set_timeout(m_pPss->wsi, PENDING_TIMEOUT_LAGGING, LWS_TO_KILL_ASYNC);
			lws_callback_on_writable(m_pPss->wsi);
            Log::error("to many data can't send");
            return;
        }
       printf("flv_free(%d) ", n);

        // �����ݱ�����ring buff
        char* pSaveBuff = (char*)malloc(nLen + LWS_PRE);
        memcpy(pSaveBuff + LWS_PRE, pBuff, nLen);
        AV_BUFF newTag = {pSaveBuff, nLen};
        if (!ring_insert(m_pFlvRing, &newTag, 1)) {
            destroy_ring_node(&newTag);
            Log::error("dropping!");
            return;
        }

        //��ͻ��˷�������
		if(m_pPss)
			lws_callback_on_writable(m_pPss->wsi);
    }

    int CLiveWorker::get_flv_frame(char **buff)
    {
		AV_BUFF *tmp = (AV_BUFF*)simple_ring_get_element(m_pFlvRing);
		if(tmp == NULL)
			return 0;
		*buff = tmp->pData + LWS_PRE;
		return tmp->nLen;
    }

	void CLiveWorker::next_flv_frame() {
		simple_ring_cosume(m_pFlvRing);
		if (ring_get_count_waiting_elements(m_pFlvRing, NULL) && m_pPss)
			lws_callback_on_writable(m_pPss->wsi);
	}

	void CLiveWorker::close()
	{
        _csWorkers.lock();
        _listWorkers.remove(this);
        IPC::SendClients(GetClientsInfo());
        _csWorkers.unlock();
		m_bConnect = false;
		m_pPss = NULL;
	}
    
    CLiveWorker* CreatLiveWorker(string strCode, string strType, string strHw, bool isWs, pss_live *pss, string clientIP) {
        CLiveWorker *worker = new CLiveWorker();
        worker->m_strCode = strCode;
        worker->m_strType = strType;
        worker->m_strHw   = strHw;
        worker->m_bWebSocket = isWs;
        worker->m_pPss = pss;
		worker->m_strClientIP = clientIP;
        if(strType == "flv")
            worker->m_strMIME = "video/x-flv";
        else if(strType == "h264")
            worker->m_strMIME = "video/h264";
        else if(strType == "mp4")
            worker->m_strMIME = "video/mp4";
        pss->pWorker = worker;

        _csWorkers.lock();
        _listWorkers.push_back(worker);
        IPC::SendClients(GetClientsInfo());
        _csWorkers.unlock();

        uv_thread_t tid;
        uv_thread_create(&tid, real_play, (void*)worker);
        Log::debug("RealPlay ok: %s",strCode.c_str());
        return worker;
    }

#ifdef USE_FFMPEG
    static void ffmpeg_log_cb(void* ptr, int level, const char* fmt, va_list vl){
        char text[256];              //��־����
        memset(text,0,256);
        vsprintf_s(text, 256, fmt, vl);

        if(level <= AV_LOG_ERROR){
            Log::error(text);
        } else if(level == AV_LOG_WARNING) {
            Log::warning(text);
        } else {
            Log::debug(text);
        }
    }

    void InitFFmpeg(){
        //av_log_set_callback(ffmpeg_log_cb);
    }
#endif
}