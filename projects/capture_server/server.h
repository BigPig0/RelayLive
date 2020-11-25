#pragma once
#include <string>
#include <list>
#include <stdint.h>


/**
 * uri中解析出参数
 */
struct RequestParam {
    std::string           strUrl;               // 原始视频地址，必填项
    std::string           strType;              // 保存的视频类型，默认为mp4
    std::string           strImgType;           // 保存的图片类型，默认为jpg
    uint32_t              nWidth;               // 视频宽度，默认为0，不缩放视频
    uint32_t              nHeight;              // 视频高度，默认为0，不缩放视频
    uint32_t              nProbSize;            // 探测PS流的大小，默认为25600
    uint32_t              nProbTime;            // 探测PS流的时间，默认为1秒
    uint32_t              nInCatch;             // 输入缓存大小 默认1024*16
    uint32_t              nOutCatch;            // 输出缓存大小 默认1024*16
    uint32_t              nImageNumber;         // 截取图片的数量，默认0，大于0时需要截图
    uint32_t              nVideoDuration;       // 截取的视频文件的时长，默认0，大于0时需要截视频
    std::list<std::string> lstImgPath;           // 生成的图片存储路径
    std::string           videoPath;            // 生成的视频存储路径
    std::string           strSavePath;          // 图片和视频存储的位置
    RequestParam();
};

namespace Server {

int Init(int port);

int Cleanup();
};