# RelayLive
## 地址: 
*  https://github.com/BigPig0/RelayLive.git
*  https://gitee.com/ztwlla/RelayLive.git

## 说明:
* sip_server: sip服务器，用来与下级平台交互
* relay_live: http和直播服务器
* ipc_server: 进程间通讯的工具，sip_server和relay_live通过改程序通讯。因此需要先启动ipc，再启动另外两个程序。

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
* flv.js: https://github.com/Bilibili/flv.js.git
* NodePlayer.js: https://github.com/illuspas/NodePlayer.js.git