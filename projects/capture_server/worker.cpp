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

//typedef struct tagBITMAPFILEHEADER
//{
//    unsigned short bfType; //2 位图文件的类型，必须为“BM”
//    unsigned long bfSize; //4 位图文件的大小，以字节为单位
//    unsigned short bfReserved1; //2 位图文件保留字，必须为0
//    unsigned short bfReserved2; //2 位图文件保留字，必须为0
//    unsigned long bfOffBits; //4 位图数据的起始位置，以相对于位图文件头的偏移量表示，以字节为单位
//} BITMAPFILEHEADER;//该结构占据14个字节。
//
//typedef struct tagBITMAPINFOHEADER{
//    unsigned long biSize; //4 本结构所占用字节数
//    long biWidth; //4 位图的宽度，以像素为单位
//    long biHeight; //4 位图的高度，以像素为单位
//    unsigned short biPlanes; //2 目标设备的平面数不清，必须为1
//    unsigned short biBitCount;//2 每个像素所需的位数，必须是1(双色), 4(16色)，8(256色)或24(真彩色)之一
//    unsigned long biCompression; //4 位图压缩类型，必须是 0(不压缩),1(BI_RLE8压缩类型)或2(BI_RLE4压缩类型)之一
//    unsigned long biSizeImage; //4 位图的大小，以字节为单位
//    long biXPelsPerMeter; //4 位图水平分辨率，每米像素数
//    long biYPelsPerMeter; //4 位图垂直分辨率，每米像素数
//    unsigned long biClrUsed;//4 位图实际使用的颜色表中的颜色数
//    unsigned long biClrImportant;//4 位图显示过程中重要的颜色数
//} BITMAPINFOHEADER;//该结构占据40个字节。

bool CreateBmp(const char *filename, uint8_t *pRGBBuffer, int width, int height, int bpp)
{
    BITMAPFILEHEADER bmpheader;
    BITMAPINFOHEADER bmpinfo;
    FILE *fp = NULL;

    fp = fopen(filename,"wb");
    if( fp == NULL )
    {
        return false;
    }

    bmpheader.bfType = ('M' <<8)|'B';
    bmpheader.bfReserved1 = 0;
    bmpheader.bfReserved2 = 0;
    bmpheader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bmpheader.bfSize = bmpheader.bfOffBits + width*height*bpp/8;

    bmpinfo.biSize = sizeof(BITMAPINFOHEADER);
    bmpinfo.biWidth = width;
    bmpinfo.biHeight = 0 - height;
    bmpinfo.biPlanes = 1;
    bmpinfo.biBitCount = bpp;
    bmpinfo.biCompression = BI_RGB;
    bmpinfo.biSizeImage = 0;
    bmpinfo.biXPelsPerMeter = 100;
    bmpinfo.biYPelsPerMeter = 100;
    bmpinfo.biClrUsed = 0;
    bmpinfo.biClrImportant = 0;

    fwrite(&bmpheader,sizeof(BITMAPFILEHEADER),1,fp);
    fwrite(&bmpinfo,sizeof(BITMAPINFOHEADER),1,fp);
    fwrite(pRGBBuffer,width*height*bpp/8,1,fp);
    fclose(fp);
    fp = NULL;

    return true;
}

//////////////////////////////////////////////////////////////////////////

// 播放线程
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
    AVFormatContext *ifc = NULL;               //输入封装
    AVFormatContext *ofc = NULL;               //输出封装
    AVStream        *istream_video = NULL;     //输入视频码流
    AVStream        *ostream_video = NULL;     //输出视频码流
    AVCodecContext  *decode_ctx = NULL;        //输入视频解码
    //AVCodecContext  *encode_ctx = NULL;        //输出视频编码
    AVFrame         *frame = NULL;             //原始码流解码帧
    struct SwsContext *pSwsCxt = NULL;         //图片格式转换
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
	//ifc->probesize = m_pParam->nProbSize;
	//ifc->max_analyze_duration = m_pParam->nProbTime*AV_TIME_BASE; //探测允许的延时
    ret = avformat_find_stream_info(ifc, NULL);
    if (ret < 0) {
        char tmp[1024]={0};
        av_strerror(ret, tmp, 1024);
        Log::error("Failed to retrieve input stream information %d(%s)", ret, tmp);
        goto end;
    }
    Log::debug("show input format info");
    av_dump_format(ifc, 0, "nothing", 0);

    //获取输入流中的视频流
    AVCodec *dec;
    in_video_index = av_find_best_stream(ifc, AVMEDIA_TYPE_VIDEO, -1, -1, &dec, 0);
    if (in_video_index < 0) {
        Log::error( "Cannot find a video stream in the input file");
        goto end;
    }
    istream_video = ifc->streams[in_video_index];

    //需要保存图片时
	if(m_pParam->nImageNumber > 0) {
        //输入流视频解码器
        decode_ctx = avcodec_alloc_context3(dec);
        avcodec_parameters_to_context(decode_ctx, istream_video->codecpar);
        decode_ctx->time_base.num = 1;
        decode_ctx->time_base.den = 25;

        /* 打开解码器 */
        if ((ret = avcodec_open2(decode_ctx, dec, NULL)) < 0) {
            Log::error("Cannot open video decoder");
            goto end;
        }

        //输入流解码后的帧
        frame = av_frame_alloc();
        if (!frame) {
            Log::error("Could not allocate frame");
            goto end;
        }

        // 视频参数
        m_nWidth = decode_ctx->width;
        m_nHeight = decode_ctx->height;
        m_nPicLen = decode_ctx->width * decode_ctx->height * 3;

        pSwsCxt = sws_getContext(m_nWidth, m_nHeight, decode_ctx->pix_fmt,
            m_nWidth, m_nHeight, AV_PIX_FMT_BGR24, SWS_BILINEAR, NULL, NULL, NULL);
    }
    //视频转换参数
    int rgb_stride[3]   = {3 * m_nWidth, 0, 0};
    uint8_t *rgb_src[3] = {NULL, NULL, NULL};

    //需要保存视频时
    if(m_pParam->videoPath.size() > 0) {
        //打开输出流
        string videoPath = m_pParam->strSavePath+m_pParam->videoPath;
        Log::debug("save video file: %s", videoPath.c_str());
        ret = avformat_alloc_output_context2(&ofc, NULL, NULL, videoPath.c_str());
        if (!ofc) {
            Log::error("Could not create output context");
            ret = AVERROR_UNKNOWN;
            goto end;
        }

        //输出流中的视频流
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

    //开始处理
    int64_t  beginDts = -1;
    uint32_t saveVideoDuration = 0;
    while (m_bRun) {
        AVPacket pkt;
        av_init_packet(&pkt);

        ret = av_read_frame(ifc, &pkt);
        if (ret < 0)
            break;

        if (pkt.stream_index == in_video_index) {
            // 视频数据
            Log::debug("read fram dts:%lld, pts:%lld, duration:%lld",pkt.dts, pkt.pts, pkt.duration);
            if(m_pParam->videoPath.size() > 0) {
                // 将原始码流写到输出
                pkt.stream_index = out_video_index;
			    pkt.dts = av_rescale_q_rnd(pkt.dts, istream_video->time_base, ostream_video->time_base, AV_ROUND_NEAR_INF/*|AV_ROUND_PASS_MINMAX*/);
			    pkt.pts = av_rescale_q_rnd(pkt.pts, istream_video->time_base, ostream_video->time_base, AV_ROUND_NEAR_INF/*|AV_ROUND_PASS_MINMAX*/);
			    pkt.duration = av_rescale_q(pkt.duration, istream_video->time_base, ostream_video->time_base);
                Log::debug("write frame dts:%lld, pts:%lld, duration:%lld",pkt.dts, pkt.pts, pkt.duration);
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

            if(m_pParam->lstImgPath.size() > 0) {
                // 解码原始码流
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
                        break;
                    }

                    // 图片数据
                    uint8_t *rgb_data   = (uint8_t*)(av_malloc(m_nPicLen));
                    rgb_src[0] = rgb_data;
                    ret = sws_scale(pSwsCxt, frame->data, frame->linesize, 0, m_nHeight, rgb_src, rgb_stride);
                    if(ret != m_nHeight) {
                        Log::error("scale failed");
                        av_frame_unref(frame);
                        break;
                    }

                    string picFile = m_pParam->strSavePath + m_pParam->lstImgPath.front();
                    m_pParam->lstImgPath.pop_front();
                    Log::debug("save image file: %s", picFile.c_str());
                    CreateBmp(picFile.c_str(), rgb_data, decode_ctx->width, decode_ctx->height, 24);

                    av_frame_unref(frame);
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

        //判断任务是否完成
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
    sws_freeContext(pSwsCxt);
    av_frame_free(&frame);

    /** 关闭解码 */
    avcodec_close(decode_ctx);

    /* 关闭输入输出 */
    avformat_close_input(&ifc);
    if (!(ofc->oformat->flags & AVFMT_NOFILE)) {
        avio_closep(&ofc->pb);
    }
    avformat_free_context(ofc);

    /** 返回码 */
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