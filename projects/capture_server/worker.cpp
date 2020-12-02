#include "uv.h"
#include "server.h"
#include "worker.h"
#include "util.h"
#include "utilc.h"
#include <list>
#include <sstream>

using namespace util;

extern "C"
{
#define snprintf  _snprintf
#define __STDC_FORMAT_MACROS
#include "libavdevice/avdevice.h"
#include "libavcodec/avcodec.h"  
#include "libavformat/avformat.h"  
#include "libavfilter/avfilter.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include "libswscale/swscale.h"  
#include "libavutil/imgutils.h"
#include "libavutil/timestamp.h"
#include "libavutil/opt.h"
}
#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"avdevice.lib")
#pragma comment(lib,"avfilter.lib")
#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"postproc.lib")
#pragma comment(lib,"swresample.lib")
#pragma comment(lib,"swscale.lib")

extern list<FramePic> g_framePics;
extern uv_mutex_t     g_framePicLock;

void freePic(FramePic p) {
    av_freep(&p.pBuff);
}

int img_savePicture(AVFrame *pFrame, const char *out_filename) {//���뱣��ͼƬ

    int width = pFrame->width;
    int height = pFrame->height;
    AVCodecContext *pCodeCtx = NULL;
    AVFormatContext *pFormatCtx = NULL;
    AVStream *pAVStream = NULL;

    do{
        pFormatCtx = avformat_alloc_context();
        // ��������ļ���ʽ
        pFormatCtx->oformat = av_guess_format("mjpeg", NULL, NULL);

        // ��������ʼ�����AVIOContext
        if (avio_open(&pFormatCtx->pb, out_filename, AVIO_FLAG_READ_WRITE) < 0) {
            Log::error("Couldn't open output file.");
            break;
        }

        // ����һ����stream
        pAVStream = avformat_new_stream(pFormatCtx, 0);
        if (pAVStream == NULL) {
            break;
        }

        AVCodecParameters *parameters = pAVStream->codecpar;
        parameters->codec_id = pFormatCtx->oformat->video_codec;
        parameters->codec_type = AVMEDIA_TYPE_VIDEO;
        parameters->format = AV_PIX_FMT_YUVJ420P;
        parameters->width = pFrame->width;
        parameters->height = pFrame->height;

        AVCodec *pCodec = NULL;
        pCodec = avcodec_find_encoder(pAVStream->codecpar->codec_id);
        if (!pCodec) {
            Log::error("Could not find encoder");
            break;
        }

        pCodeCtx = avcodec_alloc_context3(pCodec);
        if (!pCodeCtx) {
            Log::error("Could not allocate video codec context");
            break;
        }

        if ((avcodec_parameters_to_context(pCodeCtx, pAVStream->codecpar)) < 0) {
            Log::error("Failed to copy %s codec parameters to decoder context", av_get_media_type_string(AVMEDIA_TYPE_VIDEO));
            break;
        }
        pCodeCtx->time_base.num = 1;
        pCodeCtx->time_base.den = 25;

        if (avcodec_open2(pCodeCtx, pCodec, NULL) < 0) {
            Log::error("Could not open codec.");
            break;
        }

        int ret = avformat_write_header(pFormatCtx, NULL);
        if (ret < 0) {
            Log::error("write_header fail\n");
            break;
        }

        int y_size = width * height;

        //Encode
        // ��AVPacket�����㹻��Ŀռ�
        AVPacket pkt;
        av_new_packet(&pkt, y_size * 3);

        // ��������
        ret = avcodec_send_frame(pCodeCtx, pFrame);
        if (ret < 0) {
            Log::error("Could not avcodec_send_frame.");
            break;
        }

        // �õ����������
        ret = avcodec_receive_packet(pCodeCtx, &pkt);
        if (ret < 0) {
            Log::error("Could not avcodec_receive_packet");
            break;
        }
        ret = av_write_frame(pFormatCtx, &pkt);
        if (ret < 0) {
            Log::error("Could not av_write_frame");
            break;
        }
        av_packet_unref(&pkt);

        //Write Trailer
        av_write_trailer(pFormatCtx);
    }
    while(0);

    if(pCodeCtx)
        avcodec_close(pCodeCtx);
    if(pFormatCtx && pFormatCtx->pb)
        avio_close(pFormatCtx->pb);
    if(pFormatCtx)
        avformat_free_context(pFormatCtx);

    return 0;
}

//////////////////////////////////////////////////////////////////////////

// �����߳�
static void real_play(void* arg){
    CLiveWorker* pLive = (CLiveWorker*)arg;
    pLive->Play();
}

CLiveWorker::CLiveWorker()
    : m_nWidth(0)
    , m_nHeight(0)
    , m_nPicLen(0)
    , m_bRun(true)
{
    m_pParam = new RequestParam();
}

CLiveWorker::~CLiveWorker()
{
    SAFE_DELETE(m_pParam);
    Log::debug("~CLiveWorker()");
}

bool CLiveWorker::Play()
{
    AVFormatContext *ifc = NULL;               //�����װ
    AVFormatContext *ofc = NULL;               //�����װ
    AVStream        *istream_video = NULL;     //������Ƶ����
    AVStream        *ostream_video = NULL;     //�����Ƶ����
    AVCodecContext  *decode_ctx = NULL;        //������Ƶ����
    //AVCodecContext  *encode_ctx = NULL;        //�����Ƶ����
    AVFrame         *frame = NULL;             //ԭʼ��������֡
    //struct SwsContext *pSwsCxt = NULL;         //ͼƬ��ʽת��
    int             in_video_index = -1;
    int             out_video_index = 0;

    //��������
    int ret = avformat_open_input(&ifc, m_pParam->strUrl.c_str(), NULL, NULL);
    if (ret != 0) {
        char tmp[1024]={0};
        av_strerror(ret, tmp, 1024);
        Log::error("Could not open input file: %d(%s)", ret, tmp);
        goto end;
    }
	ifc->probesize = m_pParam->nProbSize;
	ifc->max_analyze_duration = m_pParam->nProbTime*AV_TIME_BASE; //̽���������ʱ
    ret = avformat_find_stream_info(ifc, NULL);
    if (ret < 0) {
        char tmp[1024]={0};
        av_strerror(ret, tmp, 1024);
        Log::error("Failed to retrieve input stream information %d(%s)", ret, tmp);
        goto end;
    }
    Log::debug("show input format info");
    av_dump_format(ifc, 0, "nothing", 0);

    //��ȡ�������е���Ƶ��
    AVCodec *dec;
    in_video_index = av_find_best_stream(ifc, AVMEDIA_TYPE_VIDEO, -1, -1, &dec, 0);
    if (in_video_index < 0) {
        Log::error( "Cannot find a video stream in the input file");
        goto end;
    }
    istream_video = ifc->streams[in_video_index];

    //��Ҫ����ͼƬʱ
	if(m_pParam->nImageNumber > 0) {
        //��������Ƶ������
        decode_ctx = avcodec_alloc_context3(dec);
        avcodec_parameters_to_context(decode_ctx, istream_video->codecpar);
        decode_ctx->time_base.num = 1;
        decode_ctx->time_base.den = 25;

        /* �򿪽����� */
        if ((ret = avcodec_open2(decode_ctx, dec, NULL)) < 0) {
            Log::error("Cannot open video decoder");
            goto end;
        }

        //������������֡
        frame = av_frame_alloc();
        if (!frame) {
            Log::error("Could not allocate frame");
            goto end;
        }
    }

    //��Ҫ������Ƶʱ
    if(m_pParam->videoPath.size() > 0) {
        //�������
        string videoPath = m_pParam->strSavePath+m_pParam->videoPath;
        Log::debug("save video file: %s", videoPath.c_str());
        ret = avformat_alloc_output_context2(&ofc, NULL, NULL, videoPath.c_str());
        if (!ofc) {
            Log::error("Could not create output context");
            ret = AVERROR_UNKNOWN;
            goto end;
        }

        //������е���Ƶ��
        ostream_video = avformat_new_stream(ofc, NULL);
        if (!ostream_video) {
            Log::error("Failed allocating output stream");
            ret = AVERROR_UNKNOWN;
            goto end;
        }
        ostream_video->id = ofc->nb_streams-1;

        ret = avcodec_parameters_copy(ostream_video->codecpar, istream_video->codecpar);
        if (ret < 0) {
            Log::error("Failed to copy codec parameters");
            goto end;
        }
        ostream_video->codecpar->codec_tag = 0;
        ostream_video->time_base = istream_video->time_base;

        Log::debug("show output format info");
        av_dump_format(ofc, 0, videoPath.c_str(), 1);

        if (!(ofc->oformat->flags & AVFMT_NOFILE)) {
            ret = avio_open(&ofc->pb, videoPath.c_str(), AVIO_FLAG_WRITE);
            if (ret < 0) {
                fprintf(stderr, "Could not open output file '%s'", videoPath.c_str());
                goto end;
            }
        }

        ret = avformat_write_header(ofc, NULL);
        if (ret < 0) {
            char tmp[1024]={0};
            av_strerror(ret, tmp, 1024);
            Log::error("Error avformat_write_header %d:%s", ret, tmp);
            goto end;
        }
    }

    //��ʼ����
    int64_t  beginDts = -1;
    uint32_t saveVideoDuration = 0;
    while (m_bRun) {
        AVPacket pkt;
        av_init_packet(&pkt);

        ret = av_read_frame(ifc, &pkt);
        if (ret < 0)
            break;

        if (pkt.stream_index == in_video_index) { // ��Ƶ����
            //����ͼ���ļ�
            if(m_pParam->lstImgPath.size() > 0) {
                // ����ԭʼ����
                ret = avcodec_send_packet(decode_ctx, &pkt);
                if (ret < 0) {
                    Log::error("decoding video stream failed");
                    goto end;
                }

                while (true && !m_pParam->lstImgPath.empty()) {
                    ret = avcodec_receive_frame(decode_ctx, frame);
                    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                        break;
                    } else if (ret < 0) {
                        Log::error("Error while receiving a frame from the decoder");
                        break;
                    }
                    if(frame->key_frame == 0) {
                        av_frame_unref(frame);
                        continue;
                    }

                    // ����ͼƬ
                    string picFile = m_pParam->strSavePath + m_pParam->lstImgPath.front();
                    m_pParam->lstImgPath.pop_front();
                    Log::debug("save image file: %s", picFile.c_str());
                    img_savePicture(frame, picFile.c_str());

                    av_frame_unref(frame);
                }
            }

            // ������Ƶ�ļ� ��Ƶ���ں��棬��Ϊav_interleaved_write_frame�Ὣpkt�е������ͷ�
            //Log::debug("read fram dts:%lld, pts:%lld, duration:%lld",pkt.dts, pkt.pts, pkt.duration);
            if(m_pParam->videoPath.size() > 0 && pkt.dts>=0 && pkt.pts>=0) {
                // ��ԭʼ����д�����
                pkt.stream_index = out_video_index;
			    pkt.dts = av_rescale_q_rnd(pkt.dts, istream_video->time_base, ostream_video->time_base, AV_ROUND_NEAR_INF/*|AV_ROUND_PASS_MINMAX*/);
			    pkt.pts = av_rescale_q_rnd(pkt.pts, istream_video->time_base, ostream_video->time_base, AV_ROUND_NEAR_INF/*|AV_ROUND_PASS_MINMAX*/);
			    pkt.duration = av_rescale_q(pkt.duration, istream_video->time_base, ostream_video->time_base);
                //Log::debug("write frame dts:%lld, pts:%lld, duration:%lld",pkt.dts, pkt.pts, pkt.duration);
			    pkt.pos = -1;
                if(beginDts == -1) {
                    beginDts = pkt.dts;
                    if(beginDts < 0)
                        beginDts = 0;
                }
                saveVideoDuration = (pkt.dts-beginDts)*ostream_video->time_base.num/ostream_video->time_base.den;
                ret = av_interleaved_write_frame(ofc, &pkt);
                if (ret < 0) {
                    char tmp[1024]={0};
                    av_strerror(ret, tmp, 1024);
                    Log::error("video error muxing packet %d:%s", ret, tmp);
                    //break;
                }
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

        //�ж������Ƿ����
        if(!m_pParam->lstImgPath.empty())
            m_bRun = true;
        else if(m_pParam->nVideoDuration > 0 && m_pParam->nVideoDuration > saveVideoDuration)
            m_bRun = true;
        else
            m_bRun = false;
    }

    if(m_pParam->nVideoDuration > 0) {
        av_write_trailer(ofc);
    }
end:
    //sws_freeContext(pSwsCxt);
    av_frame_free(&frame);

    /** �رս��� */
    avcodec_close(decode_ctx);

    /* �ر�������� */
    avformat_close_input(&ifc);
    if (ofc && ofc->oformat && ofc->pb && !(ofc->oformat->flags & AVFMT_NOFILE)) {
        avio_closep(&ofc->pb);
    }
    if(ofc)
        avformat_free_context(ofc);

    /** ������ */
    if (ret < 0 ) {
        char tmp[AV_ERROR_MAX_STRING_SIZE]={0};
        av_make_error_string(tmp,AV_ERROR_MAX_STRING_SIZE,ret);
        Log::error("Error occurred: %s", tmp);
    }

    Log::debug("finish, delete live worker");
    delete this;
    return ret>=0;
}

void CLiveWorker::close()
{
	m_bRun = false;
}
    
CLiveWorker* CreatLiveWorker(RequestParam param) {
    CLiveWorker *worker = new CLiveWorker();
    *worker->m_pParam = param;

    uv_thread_t tid;
    uv_thread_create(&tid, real_play, (void*)worker);
    Log::debug("RealPlay ok: %s",param.strUrl.c_str());
    return worker;
}

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