# RelayLive
## 地址: 
*  https://github.com/BigPig0/RelayLive.git
*  https://gitee.com/ztwlla/RelayLive.git

## 说明:
* ipc_server: 进程间通讯和进程保护的工具
  + pm.json 配置文件，需要启动的程序信息。

### 对接视频流
* relay_server: 将视频流转为ws-flv，流可以为rtsp、rtmp、hls等（理论上可以，只使用过rtsp）
  * 客户端请求格式为(不包含[]) http(ws)://IP:port/relay?url=[rtsp地址]&hw=[960*480]&type=[flv]&probsize=[102400]&probtime=[2]&outcatch=[16384]
    + url 原始视频地址
    + 其他参数见下
* relayctrl_server: 查看客户端信息
  * 查看客户端信息 http://IP:port/connect/clients

### 对接国标gb28181平台
* sip_server: sip服务器，用来与下级平台交互
* live_server: 将下级推送的基于PS的rtp流转为ws-flv
  * 客户端请求格式为(不包含[]) http(ws)://IP:port/live?code=[code]&hw=[960*480]&type=[flv]&probsize=[102400]&probtime=[2]&incatch=[16384]&outcatch=[16384]
    + code 相机的国标编码,必填项
    + hw 可选, 用来缩放视频大小, 默认不进行缩放. 用来缩小视频,只有填写的值小于视频原始值才生效
    + type 可选, 指定媒体封装格式，默认为flv. 可以是mp4、h264等, 另找播放方式
    + probsize 可选, ffmpeg探测流信息的缓冲大小，不指定时使用配置文件里的值，配置文件也未指定时默认25600. ffmpeg默认值为5000000
    + probtime 可选, ffmpeg探测流信息的缓冲时间，不指定时使用配置文件里的值，配置文件也未指定时默认1秒. ffmpeg默认值为0, 不同格式时间不一，ps流是7秒
    + incatch 可选, ffmpeg读取内存中ps流数据的缓存大小，不指定时使用配置文件里的值，配置文件也未指定时默认1024*16.
    + outcatch 可选, ffmpeg输出转换后的数据的缓存大小，不指定时使用配置文件里的值，配置文件也未指定时默认1024*16.
* livectrl_server: 查看设备信息、客户端信息、设备控制
  * 云台控制 http://ip:port/device/control?code=[code]&ud=[p1]&lr=[p2]&io=[p3]
    + code 相机的国标编码
    + p1: 0-停止 1-向上 2-向下
    + p2: 0-停止 1-向左 2-向右
    + p3: 0-停止 1-焦距减 2-焦距加
  * 订阅设备 http://ip:port/device/refresh
  * 查看设备列表 http://ip:port/device/devlist
  * 查看客户端信息 http://ip:port/device/clients

## 编译方法
* 平台: Windows vs2012
* 用vs打开RelayLive.sln,按顺序编译ThirdParty、Common、Modules、Projects下的项目。
* 在输出目录部署配置文件pm.json,config.txt和脚本文件 XXX.lua。
* 从thirdparty/libffmpeg拷贝对应的dll到输出目录。
* 使用ipc_server启动
* http-flv和websocket-flv都可以，但最好使用websocket-flv，免的出现跨域问题
* 只保留了视频，音频丢弃了。

## nginx
* 页面demo静态文件通过nginx来访问
* 信息查询和设备控制等http请求通过nginx转发到XXXctrl_server
* 视频播放请求通过nginx进行负载均衡，转发到对应的视频服务

## 第三方:
* exosip: http://savannah.nongnu.org/projects/exosip
* exosip-vs: https://github.com/BigPig0/exOsip-vs.git
* ffmpeg: http://ffmpeg.org/
* libuv: https://github.com/libuv/libuv.git
* luapp: https://github.com/ToyAuthor/luapp.git
* flv.js: https://github.com/Bilibili/flv.js.git