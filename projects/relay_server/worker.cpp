#include "uv.h"
#include "server.h"
#include "utilc_api.h"
#include "worker.h"
#include "ipc.h"
#include "util.h"
#include "easylog.h"
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

typedef struct _AV_BUFF_ {
    char       *pData;    //< 数据内容
    uint32_t    nLen;     //< 数据长度
}AV_BUFF;

static list<CLiveWorker*>  _listWorkers;
static CriticalSection     _csWorkers;
static uint32_t            _flvbufsize = 1024*32;

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
            ss << ",";
        } else {
            first = false;
		}
        ss << "{\"URL\":\"" << c->m_pParam->strUrl
            << "\",\"Connect\":\"";
        if(c->m_bWebSocket)
            ss << "websocket";
        else
            ss << "http";
        ss << "\",\"Media\":\"" << c->m_pParam->strType
            << "\",\"ClientIP\":\"" << c->m_strClientIP
            << "\",\"Channel\":\"" << c->m_pParam->nWidth << "*" << c->m_pParam->nHeight
            << "\"}";
    }
    return ss.str();
}

//////////////////////////////////////////////////////////////////////////

// 播放线程
static void real_play(void* arg){
    CLiveWorker* pLive = (CLiveWorker*)arg;
    pLive->Play();
}


//输出数据的方法
static int write_buffer(void *opaque, uint8_t *buf, int buf_size){
    CLiveWorker* pLive = (CLiveWorker*)opaque;
    pLive->push_flv_frame((char*)buf, buf_size);
    return buf_size;
}

CLiveWorker::CLiveWorker()
    : m_bWebSocket(false)
	, m_bConnect(true)
{
    m_pParam = new RequestParam();
    m_pFlvRing  = create_ring_buff(sizeof(AV_BUFF), 1000, destroy_ring_node);
}

CLiveWorker::~CLiveWorker()
{
    SAFE_DELETE(m_pParam);
    destroy_ring_buff(m_pFlvRing);
    Log::debug("~CLiveWorker()");
}

bool CLiveWorker::Play()
{
    AVFormatContext *ifc = NULL;               //输入封装
    AVFormatContext *ofc = NULL;               //输出封装
    AVStream        *istream_video = NULL;     //输入视频码流
    AVStream        *ostream_video = NULL;     //输出视频码流
    AVCodecContext  *decode_ctx = NULL;        //输入视频解码
    AVCodecContext  *encode_ctx = NULL;        //输出视频编码
    AVFrame         *frame = NULL;             //原始码流解码帧
    AVFrame         *filt_frame = NULL;        //filter缩放后的帧
    AVFilterGraph   *filter_graph = NULL;      //filter对象
    AVFilterContext *buffersink_ctx = NULL;    //输入filter
    AVFilterContext *buffersrc_ctx = NULL;     //输出filter
    bool            recodec_video = false;     //视频码流是否需要重新解编码
    bool            scale_video = false;       //视频图像是否需要缩放
    int             in_video_index = -1;
    int             out_video_index = 0;

    //打开输入流
    int ret = avformat_open_input(&ifc, m_pParam->strUrl.c_str(), NULL, NULL);
    if (ret != 0) {
        char tmp[1024]={0};
        av_strerror(ret, tmp, 1024);
        Log::error("Could not open input file: %d(%s)", ret, tmp);
        goto end;
    }
	ifc->probesize = m_pParam->nProbSize;
	ifc->max_analyze_duration = m_pParam->nProbTime*AV_TIME_BASE; //探测允许的延时
    ret = avformat_find_stream_info(ifc, NULL);
    if (ret < 0) {
        char tmp[1024]={0};
        av_strerror(ret, tmp, 1024);
        Log::error("Failed to retrieve input stream information %d(%s)", ret, tmp);
        goto end;
    }
    Log::debug("show input format info");
    av_dump_format(ifc, 0, "nothing", 0);

    AVCodec *dec;
    in_video_index = av_find_best_stream(ifc, AVMEDIA_TYPE_VIDEO, -1, -1, &dec, 0);
    if (in_video_index < 0) {
        Log::error( "Cannot find a video stream in the input file");
        goto end;
    }
    istream_video = ifc->streams[in_video_index];

    if (istream_video->codecpar->codec_id != AV_CODEC_ID_H264)
        recodec_video = true;
    if ( (m_pParam->nWidth > 0  && istream_video->codecpar->width > m_pParam->nWidth)
        || (m_pParam->nHeight > 0 && istream_video->codecpar->height > m_pParam->nHeight)
    ) {
        recodec_video = true;
        scale_video = true;
    }

    if(recodec_video){
        //需要重新编解码
        decode_ctx = avcodec_alloc_context3(dec);
        avcodec_parameters_to_context(decode_ctx, istream_video->codecpar);
        decode_ctx->time_base.num = 1;
        decode_ctx->time_base.den = 25;

        /* init the video decoder */
        if ((ret = avcodec_open2(decode_ctx, dec, NULL)) < 0) {
            Log::error("Cannot open video decoder");
            return ret;
        }

        frame = av_frame_alloc();
        if (!frame) {
            Log::error("Could not allocate frame");
            goto end;
        }
        if(scale_video) {
            filt_frame = av_frame_alloc();
            if (!filt_frame) {
                Log::error("Could not allocate filter_frame");
                goto end;
            }
        }
    }


    //打开输出流
    ret = avformat_alloc_output_context2(&ofc, NULL, m_pParam->strType.c_str(), NULL);
    if (!ofc) {
        Log::error("Could not create output context");
        ret = AVERROR_UNKNOWN;
        goto end;
    }

    unsigned char* outbuffer=(unsigned char*)av_malloc(m_pParam->nOutCatch);
    AVIOContext *avio_out =avio_alloc_context(outbuffer, m_pParam->nOutCatch,1,this,NULL,write_buffer,NULL);  
    ofc->pb = avio_out; 
    ofc->flags = AVFMT_FLAG_CUSTOM_IO;

    AVOutputFormat *ofmt = ofc->oformat;
    ofmt->flags |= AVFMT_NOFILE;

    //输出流中的视频流
    ostream_video = avformat_new_stream(ofc, NULL);
    if (!ostream_video) {
        Log::error("Failed allocating output stream");
        ret = AVERROR_UNKNOWN;
        goto end;
    }
    ostream_video->id = ofc->nb_streams-1;

    if(recodec_video) {
        // 视频重新编码为h264
        AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_H264);
        encode_ctx = avcodec_alloc_context3(codec);
		encode_ctx->bit_rate = 400000;
		encode_ctx->width = scale_video?m_pParam->nWidth:decode_ctx->width;
		encode_ctx->height = scale_video?m_pParam->nHeight:decode_ctx->height;
		encode_ctx->time_base.num = 1;
		encode_ctx->time_base.den = 25;
		encode_ctx->framerate.num = 25;
		encode_ctx->framerate.den = 1;
		encode_ctx->gop_size = 15;
		encode_ctx->has_b_frames = 0;
		encode_ctx->max_b_frames = 0;
		encode_ctx->qmin = 34;
		encode_ctx->qmax = 50;
		encode_ctx->pix_fmt = decode_ctx->pix_fmt;
		encode_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

        av_opt_set(encode_ctx->priv_data, "preset",  "superfast", 0);
        av_opt_set(encode_ctx->priv_data, "tune",    "zerolatency", 0);
        av_opt_set(encode_ctx->priv_data, "profile", "baseline", 0);

        ret = avcodec_open2(encode_ctx, codec, NULL);
        if (ret < 0) {
            Log::error("can not open encoder");
            goto end;
        }

        ret = avcodec_parameters_from_context(ostream_video->codecpar, encode_ctx);
        if (ret < 0) {
            Log::error("Failed to get codec parameters from context");
            goto end;
        }

        ostream_video->time_base = encode_ctx->time_base;
    } else {
        ret = avcodec_parameters_copy(ostream_video->codecpar, istream_video->codecpar);
        if (ret < 0) {
            Log::error("Failed to copy codec parameters");
            goto end;
        }
        ostream_video->time_base = istream_video->time_base;
    }

    Log::debug("show output format info");
    av_dump_format(ofc, 0, NULL, 1);

    ret = avformat_write_header(ofc, NULL);
    if (ret < 0) {
        char tmp[1024]={0};
        av_strerror(ret, tmp, 1024);
        Log::error("Error avformat_write_header %d:%s", ret, tmp);
        goto end;
    }

    if(scale_video) {
        //初始化缩放视频的filter
        filter_graph = avfilter_graph_alloc();

        //源filter
        char args[512]={0};
        sprintf(args, "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
            decode_ctx->width, decode_ctx->height, decode_ctx->pix_fmt,
            istream_video->time_base.num, istream_video->time_base.den,
            decode_ctx->sample_aspect_ratio.num, decode_ctx->sample_aspect_ratio.den);
        const AVFilter *buffersrc  = avfilter_get_by_name("buffer");
        ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in", args, NULL, filter_graph);
        if (ret < 0) {
            Log::error("Cannot create buffer source");
            goto end;
        }

        //sink filter
        const AVFilter *buffersink = avfilter_get_by_name("buffersink");
        AVBufferSinkParams *buffersink_params = av_buffersink_params_alloc();
        enum AVPixelFormat pix_fmts[] = { encode_ctx->pix_fmt, AV_PIX_FMT_NONE };
        buffersink_params->pixel_fmts = pix_fmts;
        ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out", NULL, buffersink_params, filter_graph);
        av_free(buffersink_params);
        if (ret < 0) {
            Log::error("Cannot create buffer sink");
            goto end;
        }

        //Endpoints for the filter graph
        AVFilterInOut *outputs = avfilter_inout_alloc();
        outputs->name       = av_strdup("in");
        outputs->filter_ctx = buffersrc_ctx;
        outputs->pad_idx    = 0;
        outputs->next       = NULL;

        AVFilterInOut *inputs  = avfilter_inout_alloc();
        inputs->name       = av_strdup("out");
        inputs->filter_ctx = buffersink_ctx;
        inputs->pad_idx    = 0;
        inputs->next       = NULL;

        char filter_descr[20]={0};
        sprintf(filter_descr, "scale=%d:%d", m_pParam->nWidth, m_pParam->nHeight);
        ret = avfilter_graph_parse_ptr(filter_graph, filter_descr, &inputs, &outputs, NULL);
        if (ret < 0){
            Log::error("Cannot filter graph parse ptr");
            goto end;
        }

        ret = avfilter_graph_config(filter_graph, NULL);
        if (ret < 0){
            Log::error("Cannot filter graph config");
            goto end;
        }
    }

    while (m_bConnect) {
        AVPacket pkt;
        av_init_packet(&pkt);

        ret = av_read_frame(ifc, &pkt);
        if (ret < 0)
            break;

        if (pkt.stream_index == in_video_index) {
            // 视频数据
            //Log::debug("read fram dts:%lld, pts:%lld, duration:%lld",pkt.dts, pkt.pts, pkt.duration);
            if(pkt.pts < 0) {
                Log::error("error: ts:%lld, pts:%lld, duration:%lld",pkt.dts, pkt.pts, pkt.duration);
                av_packet_unref(&pkt);
                continue;
            }
            if(pkt.dts < 0) {
                Log::warning("warning: ts:%lld, pts:%lld, duration:%lld",pkt.dts, pkt.pts, pkt.duration);
                pkt.dts = pkt.pts;
            }
            if(recodec_video) {
                // 解码原始码流
                ret = avcodec_send_packet(decode_ctx, &pkt);
                if (ret < 0) {
                    Log::error("decoding video stream failed");
                    goto end;
                }

                while (true) {
                    ret = avcodec_receive_frame(decode_ctx, frame);
                    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                        break;
                    } else if (ret < 0) {
                        Log::error("Error while receiving a frame from the decoder\n");
                        break;
                    }

                    frame->pts = frame->best_effort_timestamp;

                    if(scale_video) {
                        // 需要通过filter缩放帧
                        ret = av_buffersrc_add_frame_flags(buffersrc_ctx, frame, AV_BUFFERSRC_FLAG_KEEP_REF);
                        if (ret < 0) {
                            Log::error("Error while feeding the filtergraph");
                            goto end;
                        }
                        while (true) {
                            ret = av_buffersink_get_frame(buffersink_ctx, filt_frame);
                            if (ret < 0) {
                                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                                    ret = 0;
                                }
                                break;
                            }

                            //缩放后的视频帧编码并写入输出
                            AVPacket enc_pkt;
                            av_init_packet(&enc_pkt);
                            ret = avcodec_send_frame(encode_ctx, filt_frame);
                            if (ret < 0)
                                break;

                            while (true) {
                                ret = avcodec_receive_packet(encode_ctx, &enc_pkt);
                                if (ret < 0)
                                    break;


                                enc_pkt.stream_index = out_video_index;
								enc_pkt.pts = av_rescale_q_rnd(pkt.pts, istream_video->time_base, ostream_video->time_base, AV_ROUND_NEAR_INF/*|AV_ROUND_PASS_MINMAX*/);
								enc_pkt.dts = av_rescale_q_rnd(pkt.dts, istream_video->time_base, ostream_video->time_base, AV_ROUND_NEAR_INF/*|AV_ROUND_PASS_MINMAX*/);
								enc_pkt.duration = av_rescale_q(pkt.duration, istream_video->time_base, ostream_video->time_base);
								enc_pkt.pos = -1;
                                ret = av_interleaved_write_frame(ofc, &enc_pkt);
								if (ret < 0) {
									char tmp[1024]={0};
									av_strerror(ret, tmp, 1024);
									Log::error("video error muxing packet %d:%s", ret, tmp);
									//break;
								}

                                av_packet_unref(&enc_pkt);
                            }
                            av_frame_unref(filt_frame);
                        }
                    } else {
                        // 不需要缩放，直接重编码并写入输出
                        AVPacket enc_pkt;
                        av_init_packet(&enc_pkt);
                        ret = avcodec_send_frame(encode_ctx, frame);
                        if (ret < 0)
                            break;

                        while (true) {
                            ret = avcodec_receive_packet(encode_ctx, &enc_pkt);
                            if (ret < 0)
                                break;

                            enc_pkt.stream_index = out_video_index;
							enc_pkt.pts = av_rescale_q_rnd(pkt.pts, istream_video->time_base, ostream_video->time_base, AV_ROUND_NEAR_INF/*|AV_ROUND_PASS_MINMAX*/);
							enc_pkt.dts = av_rescale_q_rnd(pkt.dts, istream_video->time_base, ostream_video->time_base, AV_ROUND_NEAR_INF/*|AV_ROUND_PASS_MINMAX*/);
							enc_pkt.duration = av_rescale_q(pkt.duration, istream_video->time_base, ostream_video->time_base);
							enc_pkt.pos = -1;
                            ret = av_interleaved_write_frame(ofc, &enc_pkt);
							if (ret < 0) {
								char tmp[1024]={0};
								av_strerror(ret, tmp, 1024);
								Log::error("video error muxing packet %d:%s", ret, tmp);
								//break;
							}

                            av_packet_unref(&enc_pkt);
                        }
                    }

                    av_frame_unref(frame);
                }
            } else {
                // 将原始码流写到输出
                pkt.stream_index = out_video_index;
				pkt.pts = av_rescale_q_rnd(pkt.pts, istream_video->time_base, ostream_video->time_base, AV_ROUND_NEAR_INF/*|AV_ROUND_PASS_MINMAX*/);
				pkt.dts = av_rescale_q_rnd(pkt.dts, istream_video->time_base, ostream_video->time_base, AV_ROUND_NEAR_INF/*|AV_ROUND_PASS_MINMAX*/);
				pkt.duration = av_rescale_q(pkt.duration, istream_video->time_base, ostream_video->time_base);
				pkt.pos = -1;
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
    }

    av_write_trailer(ofc);
end:
    /** 清理filter */
    av_frame_free(&frame);
    av_frame_free(&filt_frame);
    avfilter_graph_free(&filter_graph);

    /** 关闭解码 */
    avcodec_close(decode_ctx);
    avcodec_close(encode_ctx);

    /* 关闭输入输出 */
    avformat_close_input(&ifc);
    if (ofc && !(ofmt->flags & AVFMT_NOFILE))
        avio_closep(&ofc->pb);
    avformat_free_context(ofc);

    /** 返回码 */
    if (ret < 0 /*&& ret != AVERROR_EOF*/) {
        char tmp[AV_ERROR_MAX_STRING_SIZE]={0};
        av_make_error_string(tmp,AV_ERROR_MAX_STRING_SIZE,ret);
        Log::error("Error occurred: %s", tmp);
    }

    if(m_bConnect) {
        Log::debug("video source is dropped, need close the client");
    }
    while (m_bConnect){
        sleep(10);
    }
    Log::debug("client stop, delete live worker");
    delete this;
    return ret>=0;
}


void CLiveWorker::push_flv_frame(char* pBuff, int nLen)
{
    if(!m_bConnect) {
        Log::error("client has already leave");
        return;
    }

    //内存数据保存至ring-buff
    int n = (int)ring_get_count_free_elements(m_pFlvRing);
    if (!n && m_pSession) {
        Log::error("to many data can't send");
        return;
    }
    printf("flv%d ", n);

    // 将数据保存在ring buff
    char* pSaveBuff = (char*)malloc(nLen);
    memcpy(pSaveBuff, pBuff, nLen);
    AV_BUFF newTag = {pSaveBuff, nLen};
    if (!ring_insert(m_pFlvRing, &newTag, 1)) {
        destroy_ring_node(&newTag);
        Log::error("dropping!");
        return;
    }

    //向客户端发送数据
	if(m_pSession && m_bConnect)
        m_pSession->AsyncSend();
}

int CLiveWorker::get_flv_frame(char **buff)
{
	AV_BUFF *tmp = (AV_BUFF*)simple_ring_get_element(m_pFlvRing);
	if(tmp == NULL)
		return 0;
	*buff = tmp->pData;
	return tmp->nLen;
}

void CLiveWorker::next_flv_frame() {
	simple_ring_cosume(m_pFlvRing);
}

void CLiveWorker::close()
{
    _csWorkers.lock();
    _listWorkers.remove(this);
    IPC::SendClients(GetClientsInfo());
    _csWorkers.unlock();
	m_bConnect = false;
	m_pSession = NULL;
}
    
CLiveWorker* CreatLiveWorker(RequestParam param, bool isWs, ILiveSession *pSession, string clientIP) {
    CLiveWorker *worker = new CLiveWorker();
    *worker->m_pParam = param;
    worker->m_bWebSocket = isWs;
    worker->m_pSession = pSession;
	worker->m_strClientIP = clientIP;

    _csWorkers.lock();
    _listWorkers.push_back(worker);
    IPC::SendClients(GetClientsInfo());
    _csWorkers.unlock();

    uv_thread_t tid;
    uv_thread_create(&tid, real_play, (void*)worker);
    Log::debug("RealPlay ok: %s",param.strUrl.c_str());
    return worker;
}

static void ffmpeg_log_cb(void* ptr, int level, const char* fmt, va_list vl){
    char text[256];              //日志内容
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