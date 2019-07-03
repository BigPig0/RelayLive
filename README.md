# RelayLive
## 地址: 
*  https://github.com/BigPig0/RelayLive.git
*  https://gitee.com/ztwlla/RelayLive.git

## 说明:
* sip_server: sip服务器，用来与下级平台交互
* relay_live: http和直播服务器
* ipc_server: 进程间通讯的工具，sip_server和relay_live通过改程序通讯。因此需要先启动ipc，再启动另外两个程序。

## relaylive http地址
* 获取相机列表请求: http://ip:port/device/devlist
* * 查看相机列表demo: http://ip:port/devs.html
* 获取播放客户连接请求: http://ip:port/device/clients
* * 查看播放客户连接情况demo: http://ip:port/clients.html
* 播放请求地址
* * http://ip:port/live/[type]/[channel]/[code]
* * ws://ip:port/live/[type]/[channel]/[code]
* * type: h264 flv mp4
* * channel: 0-原始码流 1-缩小码流
* * code: 相机的国标编码
* 云台控制: http://ip:port/device/control/[code]?ud=[p1]&lr=[p2]&io=[p3]
* * p1: 0-停止 1-向上 2-向下
* * p2: 0-停止 1-向左 2-向右
* * p3: 0-停止 1-焦距减 2-焦距加


## 编译方法
* 平台: Windows vs2012
* 用vs打开build/RelayLive.sln,按顺序编译ThirdParty、Common、Modules、Projects下的项目。
* 在输出目录部署配置文件config.txt和脚本文件DeviceMgr.lua。(这两个文件在/Build/projects下有示例)
* 从thirdParty拷贝ocilib和ffmpeg的dll到输出目录。
* 部署数据库，数据库的操作在DeviceMgr.lua中。

## flv播放器对比:
|  | flv.js | NodePlayer.js |
| ------ | ------ | ------ |
| 许可 | 开源免费 | 不开源、免费版只能播放10分钟 |
| 性能 | 高 | 低，单独页面播放可以，页面很多js逻辑时模糊跳帧 |
| 容错 | 有时绿屏、花屏 | 不绿屏
| 容错 | chrome下解析异常会停止 | 智能跳过异常，播放画面看不出来
| 实时性 | 不断积压、延时越来越大 | 一直播放最新画面

## 第三方:
* exosip: http://savannah.nongnu.org/projects/exosip
* exosip-vs: https://github.com/BigPig0/exOsip-vs.git
* ffmpeg: http://ffmpeg.org/
* libwebsockets: https://github.com/warmcat/libwebsockets.git
* libuv: https://github.com/libuv/libuv.git
* luapp: https://github.com/ToyAuthor/luapp.git
* libcstl: https://github.com/activesys/libcstl.git
* flv.js: https://github.com/Bilibili/flv.js.git
* NodePlayer.js: https://github.com/illuspas/NodePlayer.js.git