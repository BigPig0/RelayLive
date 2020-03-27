# RelayLive
## 地址: 
*  https://github.com/BigPig0/RelayLive.git
*  https://gitee.com/ztwlla/RelayLive.git

## 说明:
* ipc_server: 进程间通讯和进程保护的工具
  + pm.json 配置文件，需要启动的程序信息。

### 对接海康SDK
* hik_server: 使用海康sdk播放视频并转为ws-flv
* hikctrl_server: 查看平台设备信息、客户端信息
  + hikctrl.lua 数据库脚本
  + config.txt 海康平台登陆配置
* 配置示例在Build/projects/config_hik_XXX
* 客户端请求格式为 http(ws)://IP:port/live/flv/0/[code]
* sdk只有32位的，导致整个项目工具层都增加了32位的编译。只验证海康相机上抛的是标准PS流，大华相机的是其私有码流无法识别。
* 这个对接的实际意义不是很大，国标对接延时上来后，不太需要这种方法了。 不再更新。

### 对接视频流
* relay_server: 将视频流转为ws-flv，流可以为rtsp、rtmp、hls等（理论上可以，只使用过rtsp）
* relayctrl_server: 查看客户端信息
* 配置示例在Build/projects/config_relay_XXX
* 客户端请求格式为 http(ws)://IP:port/live?url=[rtsp地址]&hw=[960*480]&type=[flv]&probsize=102400&probtime=2 
  + url 原始视频地址
  + 其他参数见下

### 对接国标gb28181平台
* sip_server: sip服务器，用来与下级平台交互
* live_server: 将下级推送的基于PS的rtp流转为ws-flv
* livectrl_server: 查看设备信息、客户端信息、设备控制
* 配置示例在Build/projects/config_gb28181_XXX
* 客户端请求格式为 http(ws)://IP:port/live?code=[code]&hw=[960*480]&type=[flv]&probsize=102400&probtime=2 
  * code 相机的国标编码,必填项
  * hw 可选参数, 用来缩放视频大小, 默认不进行缩放. 用来缩小视频,只有填写的值小于视频原始值才生效
  * type 可选参数, 指定媒体封装格式，默认为flv. 可以是mp4、h264等, 另找播放方式
  * probsize 可选参数, 传递给ffmpeg探测流信息的缓冲大小，不指定时使用配置文件里的值，配置文件也未指定时默认25600. ffmpeg默认值为5000000
  * probtime 可选参数, 传递给ffmpeg探测流信息的缓冲时间，不指定时使用配置文件里的值，配置文件也未指定时默认1秒. ffmpeg默认值为0, 不同格式时间不一，ps流是7秒
* 云台控制 http://ip:port/device/control?code=[code]&ud=[p1]&lr=[p2]&io=[p3]
  * p1: 0-停止 1-向上 2-向下
  * p2: 0-停止 1-向左 2-向右
  * p3: 0-停止 1-焦距减 2-焦距加



## 编译方法
* 平台: Windows vs2012
* 用vs打开build/RelayLive.sln,按顺序编译ThirdParty、Common、Modules、Projects下的项目。
* 编译前需要在属性管理器中将Microsoft.Cpp.x64.user(或Microsoft.Cpp.Win32.user)中修改以下：(或者在每个项目属性中都修改一次)
  * 常规-输出目录：`$(SolutionDir)..\out\$(Platform)_$(Configuration)\`
  * 常规-中间目录：`$(SolutionDir)..\out\Temp\$(Platform)_$(Configuration)\$(ProjectName)\`
  * 链接器-常规-附加库目录：`$(OutDir);%(AdditionalLibraryDirectories)`
* 在输出目录部署配置文件pm.json,config.txt和脚本文件 XXX.lua。(文件在/Build/projects下有示例)
* 从thirdParty拷贝ffmpeg的dll到输出目录。
* 最好用64位进行编译使用，如需32位，可能需要一些修改
* 部署数据库，数据库的操作在XXX.lua中。
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
* libwebsockets: https://github.com/warmcat/libwebsockets.git [确实有一些问题，把这个库去掉后稳定多了]
* libuv: https://github.com/libuv/libuv.git
* luapp: https://github.com/ToyAuthor/luapp.git
* libcstl: https://github.com/activesys/libcstl.git
* flv.js: https://github.com/Bilibili/flv.js.git
* NodePlayer.js: https://github.com/illuspas/NodePlayer.js.git