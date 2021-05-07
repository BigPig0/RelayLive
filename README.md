# RelayLive
## 地址: 
*  https://github.com/BigPig0/RelayLive.git
*  https://gitee.com/ztwlla/RelayLive.git

##说明:
* 本程序的主要功能是将各种网络视频方式转为网页播放方式。
* 视频来源可以分为：流媒体（rtsp，rtmp，hls等）、GB28281平台、海康SDK连接相机
* 提供给网页播放的方式为http-flv、ws-flv(通过flv.js、NodePlayer.js)，http-fmp4(video标签直接可用)，ws-mp4(自己写msi扩展)。

## 模块:
* ipc_server: 进程间通讯，在windows平台还有进程保护的作用，通过pm.json配置启动其他程序
* control_server: 查看客户端信息，控制相机，以及国标下查看设备信息
* sip_server: [gb28181] sip服务器，用来与下级平台交互
* relay_server: 将流媒体转为网页播放
* live_server: [gb28181]将下级推送的基于PS的rtp流转为网页播放
* capture_server: 将流媒体截取一些图片及指定时长的视频文件保存

## 前端请求格式
  * 视频请求有http和websocket两种格式，其他请求都是http
  * 流媒体转为网页播放: http(ws)://IP:port/relay?url=[rtsp地址]
    + url 原始视频地址
  * gb28181设备视频转网页播放: http(ws)://IP:port/live?code=[code]
    + code 相机的国标编码,必填项
  * 海康sdk设备视频转网页播放:http(ws)://IP:port/hik?host=[host]&port=[8000]&user=[user]&pwd=[pwd]&channel=[1]
    + host 相机ip地址或域名
    + port 相机开放的端口
    + user 相机用户名
    + pwd 相机登陆密码
    + channel 指定的播放通道
  * 以上视频请求可选参数说明：
    + hw 用来缩放视频大小, 默认不进行缩放. 用来缩小视频,只有填写的值小于视频原始值才生效。&hw=960*480
    + type 指定媒体封装格式，默认为flv。 可以是mp4[使用fmp4]、h264等。&type=mp4
    + probsize ffmpeg探测流信息的缓冲大小，不指定时使用配置文件里的值，配置文件也未指定时默认25600。&probsize=102400
    + probtime ffmpeg探测流信息的缓冲时间，单位毫秒，不指定时使用配置文件里的值，配置文件也未指定时默认1秒。&probtime=2000
    + incatch ffmpeg读取内存中ps流数据的缓存大小[流媒体无效]，不指定时使用配置文件里的值，配置文件也未指定时默认1024*16。&incatch=16384
    + outcatch ffmpeg输出转换后的数据的缓存大小，不指定时使用配置文件里的值，配置文件也未指定时默认1024*16。&outcatch=16384
    + begintime 播放历史视频，指定开始时间[流媒体无效]。&begintime=20201230235959
    + endtime 播放历史视频，指定结束时间[流媒体无效]。&endtime=20210101090000
  * 查看客户端信息 http://IP:port/device/clients
  * [gb28181]云台控制 http://ip:port/device/control?code=[code]&ud=[p1]&lr=[p2]&io=[p3]
    + code 相机的国标编码
    + p1: 0-停止 1-向上 2-向下
    + p2: 0-停止 1-向左 2-向右
    + p3: 0-停止 1-焦距减 2-焦距加
  * [gb28181]订阅设备 http://ip:port/device/refresh
  * [gb28181]查看设备列表 http://ip:port/device/devlist
  * [gb28181]查看客户端信息 http://ip:port/device/clients

## 编译方法
* Windows vs2012
  * 用vs打开RelayLive.sln,按顺序编译ThirdParty、Common、Modules、Projects下的项目。
  * 输出目录为./out/x64_debug(release)。
  * 在可执行文件同一目录部署配置文件pm.json,config.txt[和脚本文件XXX.lua]。
  * 从thirdparty/libffmpeg拷贝对应的dll到输出目录。
  * 使用ipc_server启动
  * 日志位于可执行文件目录的log文件夹下
* Linux
  * 首先下载ffmpeg，x264，x265的源码编译安装，生成动态库。ffmpeg需要开启x264、x265。有需要可以增加其他编码库。
  * 进入代码根目录，执行make
  * 生成的文件在./out/linux64
  * 将可执行文件拷贝到bin目录。
  * 在/etc/relaylive部署配置文件config.txt[和脚本文件XXX.lua]。
  * 使用命令行启动或使用systemd管理
  * 日志位于/var/log/relaylive
* 注意事项
  * 只保留了视频，音频丢弃了。需要音频可以在worker中添加代码。
  * 编译配置都是64位，需要32位的需要修改vs项目文件或makefile。
  * 根目录的config.h可以配置需要哪些数据库功能，数据库只有sip_server的lua脚本中需要使用。

## 配置文件
* [IPC]
* * name ipc使用的管道名称，默认为ipcsvr
* * ctrl control_server使用的ipc客户端名称，默认为ctrlsvr
* [SipSever]
* * Code 本机平台编码
* * IP 本机IP，发送给下级平台的IP
* * Port sip服务监听端口
* * RegAuthor 是否需要鉴权，默认0
* [PlatFormInfo]
* * Code 对方下级平台的编码
* * IP 对方下级平台的IP
* * Port 对方下级平台的端口
* * SubscribeStatus 是否订阅设备状态
* * SubscribePos 是否订阅设备位置，订阅整个平台
* * SubscribePosDev 是否订阅指定部门的设备位置[特殊定制]
* * SubscribePosDepart 订阅设备所在的部门，多个部门用','分隔[特殊定制]
* [RtpClient]
* * IP live_server服务所在的ip
* * BeginPort rtp接收起始端口
* * PortNum rtp端口数量
* * outtime rtp队列超时,ms
* [Script]
* * use 是否使用lua脚本，yes、no[默认]
* * path lua脚本路径
* [Capture]
* * save 截图保存位置
* * name 图片、视频文件链接地址前缀，给nginx访问文件时匹配地址
* * clean 超出天数的文件自动清理
* [FFMPEG]
* * probsize ffmpeg探测流信息的缓冲大小，默认25600
* * probtime ffmpeg探测流信息的缓冲时间，单位毫秒，默认1000
* * incatch ffmpeg读取内存中ps流数据的缓存大小，默认1024*16
* * outcatch ffmpeg输出转换后的数据的缓存大小，默认1024*16。

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